#include "tachometer.h"

#include "FreeRTOS.h"
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#include "freertos_init.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "hardware/adc.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "queue.h"
#include "stddef.h"
#include "stdint.h"
#include "task.h"
#include "venti_board_defs.h"
#include "venti_defs.h"
#include "venti_irq.h"
#include "venti_lib.h"
// #include "arm_math.h"
#include "pid.h"

typedef struct {
  uint32_t timestamp_us;
  size_t fan_index;
} fan_event_t;

uint32_t fan_interval[FAN_COUNT][FAN_INTERVAL_LENGTH];

inline uint16_t look_up_duty_cycle(uint16_t temp_celsius_x10, fan_curve_t* fan_curve);
inline uint16_t adc_to_celsius_x10(uint16_t adc_value);

void tach_monitor(void* pvParameters) {
  // To guarantee the interrupt is set on the same core as this task
  vTaskCoreAffinitySet(NULL, 1 << 0);

  const tach_monitor_parameters_t* parameters = pvParameters;
  const size_t fan_index = parameters->fan_index;
  const size_t recipient_count = parameters->recipient_count;
  const TaskHandle_t fan_controller = parameters->fan_controller;
  const TaskHandle_t* recipients = parameters->recipients;
  uint32_t* const intervals = fan_interval[fan_index];

  gpio_init(TACH1_PIN);
  gpio_set_irq_enabled_with_callback(TACH1_PIN, GPIO_IRQ_EDGE_FALL, true,
                                     gpio_irq_handler);

  uint32_t last_timestamp_us = 0;
  uint32_t new_timestamp_us = 0;
  size_t interval_index = 0;
  uint32_t current_speed = 0;

  for (;;) {
    xTaskNotifyIndexed(fan_controller, FAN_CONTROL_SPEED_NOTIFICATION_INDEX,
                       current_speed, eSetValueWithOverwrite);
    for (int recipient = 0; recipient < recipient_count; recipient++) {
      xTaskNotifyIndexed(recipients[recipient], fan_index, current_speed,
                         eSetValueWithOverwrite);
    }
    BaseType_t queue_result =
        xTaskNotifyWait(0, UINT32_MAX, &new_timestamp_us, pdMS_TO_TICKS(1000));

    if (queue_result == pdPASS) {
      interval_index = (interval_index + 1) % FAN_INTERVAL_LENGTH;
      uint32_t interval = new_timestamp_us - last_timestamp_us;

      if (last_timestamp_us > (UINT16_MAX >> 1)) {
        stdio_puts("fan rotation interval larger than 15 bits recorded");
      }

      intervals[interval_index] = interval;
      last_timestamp_us = new_timestamp_us;

      // Send only data only when all intervals are filled
      if (!intervals[FAN_INTERVAL_LENGTH - 1]) {
        continue;
      }
      // Notify the fan controller and other recipients(like user interfaces) of
      // update
      current_speed =
          sum8_u32(intervals) >> 3;  // find mean of current intervals

    } else {
      interval_index = 0;
      memset(intervals, 0, sizeof(intervals[0]) * FAN_INTERVAL_LENGTH);
      current_speed = 0;
    }
  }
}

void fan_control(void* pvParameters) {
  #pragma region Paramater Initialization and variable declaration
  fan_control_parameters_t* parameters = pvParameters;
  const uint vin_control_pin = parameters->control_pins.voltage_control;
  const uint pwm_control_pin = parameters->control_pins.pwm_control;
  fan_curve_t fan_curve = parameters->fan_curve;

  fan_control_notification_t command_notification;
  fan_control_command_t current_command;

  TickType_t last_wake_time = xTaskGetTickCount();
  BaseType_t command_result;
  BaseType_t speed_result;
  BaseType_t curve_update_result;
  union {
    fan_curve_t* as_curve;
    uint32_t as_notification;
  } curve_update;

  uint32_t current_speed;
  float current_speed_f;
  float target_speed_f;
  uint16_t pwm_level = 0;

  venti_pid_t* pwm_control_pid = venti_pid_init(0.1f, 0.3f, 0.02f);
  venti_pid_t* vin_control_pid = venti_pid_init(0.1f, 0.3f, 0.02f);

  #pragma region Peripheral Initialization
  uint voltage_slice = pwm_gpio_to_slice_num(vin_control_pin);
  pwm_config voltage_config = pwm_get_default_config();
  pwm_config_set_wrap(&voltage_config, 4'095);
  pwm_set_gpio_level(vin_control_pin, 4096);
  pwm_init(voltage_slice, &voltage_config, true);
  gpio_set_function(vin_control_pin, GPIO_FUNC_PWM);

  pwm_config pwm_pin_config = pwm_get_default_config();
  pwm_config_set_clkdiv(&pwm_pin_config, 1.5f);
  pwm_config_set_wrap(&pwm_pin_config, 4'095);
  pwm_set_gpio_level(pwm_control_pin, 4'095);
  uint pwm_pin_slice = pwm_gpio_to_slice_num(pwm_control_pin);
  pwm_init(pwm_pin_slice, &pwm_pin_config, true);
  gpio_set_function(pwm_control_pin, GPIO_FUNC_PWM);
  
  // Init temp sensor
  adc_init();
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);
  #pragma endregion

  uint32_t waiting_for_fan_start = 1;
  fan_control_type_t current_control_type = CONTROL_UNCHANGED;
  uint32_t pwm_pin_detected = 0;
  #pragma endregion
  for (;;) {
    #pragma region Command Handling
    // Get command for interface
    command_result =
        xTaskNotifyWaitIndexed(FAN_CONTROL_COMMAND_INDEX, 0, 0,
                               &command_notification.as_notification, 0);
    // Clear PID
    if (current_command.control_select != SPEED_SELECT_PID &&
        command_notification.as_command.control_select == SPEED_SELECT_PID) {
      venti_pid_reset(pwm_control_pid);
      venti_pid_reset(vin_control_pid);
    }
    current_command = command_notification.as_command;
    // Set up state machine for fan control type.
    switch (current_command.control_type) {
      case CONTROL_SET_PWM:
        current_control_type = CONTROL_SET_PWM;
      case CONTOL_SET_VOLTAGE:
        current_control_type = CONTOL_SET_VOLTAGE;
      case CONTROL_UNCHANGED:
        if (pwm_pin_detected) {
          current_control_type = CONTROL_SET_PWM;
        } else {
          current_control_type = CONTOL_SET_VOLTAGE;
        }
    }
    #pragma endregion
    // Get current speed from tachmonitor
    speed_result = xTaskNotifyWaitIndexed(FAN_CONTROL_SPEED_NOTIFICATION_INDEX,
                                          0, 0, &current_speed, 0);
    curve_update_result = xTaskNotifyWaitIndexed(FAN_CONTROL_CURVE_UPDATE_INDEX,
                                                0, 0, &curve_update.as_notification, 0);
    if (curve_update_result == pdPASS) {
      fan_curve = *curve_update.as_curve;
    }
    // Fan speed can go to zero on certain fans under 20 duty cycle or when we
    // manually turn it off
    if (current_speed == 0) {
      if (current_command.control_select != SPEED_SELECT_OFF ||
          !((current_command.control_select == SPEED_SELECT_AUTO ||
             current_command.control_select == SPEED_SELECT_PWM) &&
            pwm_level <= 200)) {
        venti_pid_reset(pwm_control_pid);
        venti_pid_reset(vin_control_pid);
        waiting_for_fan_start = 1;
        pwm_set_gpio_level(vin_control_pin, 4'095);
        vTaskDelay(pdMS_TO_TICKS(2000));
        gpio_set_function(vin_control_pin, GPIO_FUNC_SIO);
        pwm_pin_detected = gpio_get(pwm_control_pin);
        gpio_set_input_enabled(vin_control_pin, false);
        pwm_set_gpio_level(pwm_control_pin, 4'095);
        gpio_set_function(vin_control_pin, GPIO_FUNC_PWM);
        if (current_command.control_select == SPEED_SELECT_PID) {
          xQueueSend(user_error_queue, PID_FAN_SPEED_TOO_LOW, pdMS_TO_TICKS(10));
        }
        last_wake_time = xTaskGetTickCount();
      }
    }
    switch (current_command.control_select) {
      case SPEED_SELECT_PWM:
        pwm_level = current_command.speed_value;
        break;
      case SPEED_SELECT_PID:
        current_speed_f = (float)current_speed;
        target_speed_f = (float)current_command.speed_value;
        float error = current_speed_f - target_speed_f;
        if (current_control_type) {
          pwm_level = (uint16_t)venti_pid_step(vin_control_pid, error);
        } else {
          pwm_level = (uint16_t)venti_pid_step(pwm_control_pid, error);
        }
        stdio_printf("current speed: %d, target speed: %d, pwm level: %d\n",
                     current_speed, current_command.speed_value, pwm_level);
        break;
      case SPEED_SELECT_AUTO:
          pwm_level = look_up_duty_cycle(adc_to_celsius_x10(adc_read()), &fan_curve);
          break;
      case SPEED_SELECT_OFF:
        pwm_set_gpio_level(pwm_control_pin, 0);
        pwm_set_gpio_level(vin_control_pin, 0);
        break;
    }
    switch (current_command.control_type) {
          case CONTROL_SET_PWM:
            pwm_set_gpio_level(pwm_control_pin, pwm_level);
            break;
          case CONTOL_SET_VOLTAGE:
            pwm_set_gpio_level(vin_control_pin, pwm_level);
            break;
          case CONTROL_UNCHANGED:
            stdio_puts("invalid fan control state");
            break;
        }
    
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(PID_DT_MS));
  }
}

inline uint16_t adc_to_celsius_x10(uint16_t adc_value) {
  return 4372 - ((int32_t)adc_value * 4682 + 500) / 1000;;
}

inline uint16_t look_up_duty_cycle(uint16_t temp_celsius_x10, fan_curve_t* fan_curve) {
  size_t i = 0;
  size_t j = 0;
  for (j = 0; j < fan_curve->length; j++) {
    if (temp_celsius_x10 <= fan_curve->temp_celsius_x10[j]) {
      break;
    }
    i = j;
  }
  return ((int32_t)fan_curve->duty_cycle[i]*(1+(int32_t)fan_curve->temp_celsius_x10[j] - (int32_t)temp_celsius_x10) + fan_curve->duty_cycle[j]*(1+(int32_t)temp_celsius_x10 - (int32_t)fan_curve->temp_celsius_x10[i]))/(2+(int32_t)fan_curve->temp_celsius_x10[j] - (int32_t)fan_curve->temp_celsius_x10[i]);
}
