#include "tachometer.h"

#include "pico/time.h"
#include "pico/stdio.h"
#include "stdatomic.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "board_definitions.h"
#include <stdio.h>

#define TACH_QUEUE_LENGTH 32
static QueueHandle_t tach_queue;

typedef enum {
  VENTI_TACH_IRQ = 0,
  VENTI_USER_INPUT_IRQ,
} venti_irq_type_t;

typedef struct {
  uint32_t event_mask;
  venti_irq_type_t type;
} venti_gpio_irq_t;

static venti_gpio_irq_t VENTI_IRQ_TABLE[MAX_INTERRUPT_PIN];

// faketodo: move outside tachometer.c
static void gpio_irq_handler(uint gpio, uint32_t event_mask) {
  gpio_acknowledge_irq(gpio, event_mask);
  switch (VENTI_IRQ_TABLE[gpio].type) {
    case VENTI_TACH_IRQ:
      // Handle tachometer interrupt
      uint64_t timestamp_us = time_us_64();
      BaseType_t task_woken = pdFALSE;

      if (xQueueSendFromISR(tach_queue, &timestamp_us, &task_woken) == pdPASS) {
        stdio_puts("Tachometer queue full.\n");
      }

      portYIELD_FROM_ISR(task_woken);
      break;

    case VENTI_USER_INPUT_IRQ:
      // Handle user input interrupt
      break;
    default:
      stdio_puts("GPIO interrupt without a type found.\n");
  }
  // tachometer interrupt handler
  
  
}

void tach_monitor(void *parameter) {
  tach_queue = xQueueCreate(TACH_QUEUE_LENGTH, sizeof(uint64_t));

  gpio_init(TACH1_PIN);
  gpio_set_irq_enabled_with_callback(TACH1_PIN, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);

  for (;;) {
    uint64_t timestamp_us;
    if (xQueueReceive(tach_queue, &timestamp_us, pdMS_TO_TICKS(5000)) == pdPASS) {

    }
  }
}
