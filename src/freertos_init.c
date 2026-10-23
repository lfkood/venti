#include "freertos_init.h"

#include "FreeRTOS.h"  // IWYU pragma: keep
#include "pico/stdio.h"
#include "task.h"
#include "queue.h"

#define VENTI_SYSVIEW_IMPLEMENTATION
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_Conf.h"
#include "venti_board_defs.h"
#include "gpio_interface.h"
#include "misc.h"
#include "tachometer.h"
#include "usb_interface.h"

void tach_parameter_setup(TaskHandle_t usb_interface_handle, TaskHandle_t gpio_interface_handle, size_t fan_index);

#define tskBLINK_PRIORITY tskIDLE_PRIORITY + 1
#define tskINTERFACE_PRIORITY tskIDLE_PRIORITY + 2
#define tskFAN_PRIORITY tskIDLE_PRIORITY + 3

static tach_monitor_parameters_t tach_parameters[FAN_COUNT];
static fan_control_parameters_t fan_control_parameters[FAN_COUNT];
TaskHandle_t TACH_MONITORS[FAN_COUNT];
QueueHandle_t user_error_queue;

void freeRTOS_init() {
  SEGGER_SYSVIEW_Conf();
  SEGGER_SYSVIEW_Start();

  if (xTaskCreate(blink_task, "blink", 512, NULL, tskBLINK_PRIORITY, NULL) !=
      pdPASS) {
    stdio_printf("Failed to create blink task");
  }

  BaseType_t result = pdPASS;
  TaskHandle_t usb_interface_handle = NULL;
  result += xTaskCreate(usb_interface, "usb_interface", 512, NULL,
                        tskINTERFACE_PRIORITY, &usb_interface_handle);
  TaskHandle_t gpio_interface_handle = NULL;
  result += xTaskCreate(gpio_interface, "gpio_interface", 512, NULL,
                        tskINTERFACE_PRIORITY, &gpio_interface_handle);

  fan_curve_t* fan_curve = (fan_curve_t*)pvPortMalloc(sizeof(fan_curve_t));

  tach_parameter_setup(usb_interface_handle, gpio_interface_handle, 0);

  fan_control_parameters[0].control_pins = FAN1_CONTROL_PINS;
  fan_control_parameters[0].fan_curve = *fan_curve;

  result += xTaskCreate(fan_control, "fan_control_0", 512, &fan_control_parameters[0],
                        tskFAN_PRIORITY, &tach_parameters[0].fan_controller);

  result += xTaskCreate(tach_monitor, "tach_monitor_0", 512, &tach_parameters[0],
                        tskFAN_PRIORITY, &TACH_MONITORS[0]);

  tach_parameter_setup(usb_interface_handle, gpio_interface_handle, 1);

  fan_control_parameters[1].control_pins = FAN1_CONTROL_PINS;
  fan_control_parameters[1].fan_curve = *fan_curve;

  result += xTaskCreate(fan_control, "fan_control_1", 512, &fan_control_parameters[1],
                        tskFAN_PRIORITY, &tach_parameters[1].fan_controller);

  result += xTaskCreate(tach_monitor, "tach_monitor_1", 512, &tach_parameters[1],
                        tskIDLE_PRIORITY + 1, &TACH_MONITORS[1]);

  tach_parameter_setup(usb_interface_handle, gpio_interface_handle, 2);

  fan_control_parameters[2].control_pins = FAN2_CONTROL_PINS;
  fan_control_parameters[2].fan_curve = *fan_curve;
  
  result += xTaskCreate(fan_control, "fan_control_2", 512, &fan_control_parameters[2],
                        tskFAN_PRIORITY, &tach_parameters[2].fan_controller);

  result += xTaskCreate(tach_monitor, "tach_monitor_2", 512, &tach_parameters[2],
                        tskIDLE_PRIORITY + 1, &TACH_MONITORS[2]);

  if (result == pdFAIL) {
    stdio_printf("Failed to create tach monitor task");
  }
  user_error_queue = xQueueCreate(USER_ERROR_QUEUE_LENGTH, sizeof(user_error_t));
  if (user_error_queue == NULL) {
    stdio_printf("Failed to create user error queue");
  }
  vPortFree(fan_curve);
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
