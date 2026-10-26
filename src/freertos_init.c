#include "freertos_init.h"

#include "FreeRTOS.h"  // IWYU pragma: keep
#include "pico/stdio.h"
#include "task.h"
#include "queue.h"

#define VENTI_SYSVIEW_IMPLEMENTATION
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_Conf.h"
#include "venti_board_defs.h"
#include "venti_defs.h"
#include "gpio_interface.h"
#include "misc.h"
#include "tachometer.h"
#include "usb_interface.h"

void tach_parameter_setup(TaskHandle_t usb_interface_handle, TaskHandle_t gpio_interface_handle, size_t fan_index);

static tach_monitor_parameters_t tach_parameters[FAN_COUNT];
static fan_control_parameters_t fan_control_parameters[FAN_COUNT];
TaskHandle_t TACH_MONITOR[FAN_COUNT];
TaskHandle_t FAN_CONTROL[FAN_COUNT];
QueueHandle_t user_error_queue;

void freeRTOS_init() {
  // Let the recorder start tracing after the probe is ready to drain RTT.
  SEGGER_SYSVIEW_Conf();

  if (xTaskCreate(blink_task, "blink", 512, NULL, tskBLINK_PRIORITY, NULL) !=
      pdPASS) {
    stdio_printf("Failed to create blink task");
  }

  BaseType_t result = pdPASS;
  TaskHandle_t usb_interface_handle = NULL;
  result += xTaskCreate(stdio_interface, "stdio_interface", 2048, NULL,
                        tskINTERFACE_PRIORITY, &usb_interface_handle);
  TaskHandle_t gpio_interface_handle = NULL;
  result += xTaskCreate(gpio_interface, "gpio_interface", 2048, NULL,
                        tskINTERFACE_PRIORITY, &gpio_interface_handle);

  fan_curve_t fan_curve = {
    .temp_celsius_x10 = { 200, 300 },
    .duty_cycle = { 0, 100 },
    .length = 2
  };

  tach_parameter_setup(usb_interface_handle, gpio_interface_handle, 0);

  fan_control_parameters[0].control_pins = FAN1_CONTROL_PINS;
  fan_control_parameters[0].fan_curve = fan_curve;
  fan_control_parameters[0].recipient_count = tach_parameters[0].recipient_count;
  for (int i = 0; i < tach_parameters[0].recipient_count; i++) {
    fan_control_parameters[0].recipients[i] = tach_parameters[0].recipients[i];
  }
  fan_control_parameters[0].fan_index = tach_parameters[0].fan_index;


  result += xTaskCreate(fan_control, "fan_control_0", 2048, &fan_control_parameters[0],
                        tskFAN_PRIORITY, &FAN_CONTROL[0]);

  tach_parameters[0].fan_controller = FAN_CONTROL[0];

  result += xTaskCreate(tach_monitor, "tach_monitor_0", 2048, &tach_parameters[0],
                        tskFAN_PRIORITY, &TACH_MONITOR[0]);

  tach_parameter_setup(usb_interface_handle, gpio_interface_handle, 1);

  fan_control_parameters[1].control_pins = FAN1_CONTROL_PINS;
  fan_control_parameters[1].fan_curve = fan_curve;
  fan_control_parameters[1].recipient_count = tach_parameters[1].recipient_count;
  for (int i = 0; i < tach_parameters[1].recipient_count; i++) {
    fan_control_parameters[1].recipients[i] = tach_parameters[1].recipients[i];
  }
  fan_control_parameters[1].fan_index = tach_parameters[1].fan_index;

  result += xTaskCreate(fan_control, "fan_control_1", 2048, &fan_control_parameters[1],
                        tskFAN_PRIORITY, &FAN_CONTROL[1]);

  tach_parameters[1].fan_controller = FAN_CONTROL[1];
  

  result += xTaskCreate(tach_monitor, "tach_monitor_1", 2048, &tach_parameters[1],
                        tskIDLE_PRIORITY + 1, &TACH_MONITOR[1]);

  tach_parameter_setup(usb_interface_handle, gpio_interface_handle, 2);

  fan_control_parameters[2].control_pins = FAN2_CONTROL_PINS;
  fan_control_parameters[2].fan_curve = fan_curve;
  fan_control_parameters[2].recipient_count = tach_parameters[2].recipient_count;
  for (int i = 0; i < tach_parameters[2].recipient_count; i++) {
    fan_control_parameters[2].recipients[i] = tach_parameters[2].recipients[i];
  }
  fan_control_parameters[2].fan_index = tach_parameters[2].fan_index;
  
  result += xTaskCreate(fan_control, "fan_control_2", 2048, &fan_control_parameters[2],
                        tskFAN_PRIORITY, &FAN_CONTROL[2]);

  tach_parameters[2].fan_controller = FAN_CONTROL[2];

  result += xTaskCreate(tach_monitor, "tach_monitor_2", 2048, &tach_parameters[2],
                        tskIDLE_PRIORITY + 1, &TACH_MONITOR[2]);

  if (result == pdFAIL) {
    stdio_printf("Failed to create tach monitor task");
  }
  user_error_queue = xQueueCreate(USER_ERROR_QUEUE_LENGTH, sizeof(user_error_t));
  if (user_error_queue == NULL) {
    stdio_printf("Failed to create user error queue");
  }
  vTaskStartScheduler();
}

void tach_parameter_setup(TaskHandle_t usb_interface_handle,
    TaskHandle_t gpio_interface_handle, size_t fan_index) {
  tach_monitor_parameters_t* tach_params = &tach_parameters[fan_index];
  tach_params->fan_index = fan_index;
  tach_params->recipient_count = 2;
  tach_params->recipients[0] = usb_interface_handle;
  tach_params->recipients[1] = gpio_interface_handle;
  tach_params->fan_controller = NULL;
}
