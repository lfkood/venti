#include "venti_irq.h"
#include "freertos_init.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "FreeRTOS.h"
#include "task.h"
#include "freertos_init.h"
#include "venti_board_defs.h"


// faketodo: move outside tachometer.c
void __time_critical_func(gpio_irq_handler)(uint gpio,
                                                   uint32_t event_mask) {
  traceISR_ENTER();
  BaseType_t task_woken = pdFALSE;

  switch (VENTI_IRQ_TABLE[gpio].type) {
    // Handle tachometer interrupt
    case VENTI_TACH_IRQ:
      uint32_t timestamp_us = time_us_32();

      if (xTaskNotifyFromISR(TACH_MONITOR[VENTI_IRQ_TABLE[gpio].fan_index],
                             timestamp_us, eSetValueWithOverwrite,
                             &task_woken) != pdPASS) {
        stdio_puts("Tachometer queue full.\n");
      }

    // Handle user input interrupt
    case VENTI_USER_INPUT_IRQ:
      // stdio_printf("Received user input interrupt on pin %d.\n", gpio);

    default:
      // stdio_puts("GPIO interrupt without a type found.\n");
  }
  traceISR_EXIT();
  portYIELD_FROM_ISR(task_woken);
}