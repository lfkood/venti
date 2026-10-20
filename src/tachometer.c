#include "tachometer.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "stdatomic.h"

#include "SEGGER_SYSVIEW_FreeRTOS.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "board_definitions.h"
#include <pico/sem.h>
#include <stdatomic.h>

typedef enum {
  VENTI_TACH_IRQ = 0,
  VENTI_USER_INPUT_IRQ,
} venti_irq_type_t;

typedef struct {
  uint32_t event_mask;
  venti_irq_type_t type;
  size_t fan_index;
} venti_gpio_irq_t;

// faketodo: move to board def
static const venti_gpio_irq_t VENTI_IRQ_TABLE[FAN_COUNT] = {
  {
    .event_mask = GPIO_IRQ_EDGE_FALL,
    .type = VENTI_TACH_IRQ,
    .fan_index = 0,
  },
  {
    .event_mask = GPIO_IRQ_EDGE_FALL,
    .type = VENTI_TACH_IRQ,
    .fan_index = 1,
  },
  {
    .event_mask = GPIO_IRQ_EDGE_FALL,
    .type = VENTI_TACH_IRQ,
    .fan_index =  2,
  },
};

#define TACH_QUEUE_LENGTH 32
static QueueHandle_t tach_queue[FAN_COUNT];

// faketodo: move outside tachometer.c
static void gpio_irq_handler(uint gpio, uint32_t event_mask) {
  gpio_acknowledge_irq(gpio, event_mask);

  traceISR_ENTER();
  BaseType_t task_woken = pdFALSE;

  switch (VENTI_IRQ_TABLE[gpio].type) {
    // Handle tachometer interrupt
    case VENTI_TACH_IRQ:
      uint64_t timestamp_us = time_us_64();

      if (xQueueSendFromISR(tach_queue[VENTI_IRQ_TABLE[gpio].fan_index], &timestamp_us, &task_woken) == pdPASS) {
        stdio_puts("Tachometer queue full.\n");
      }

    // Handle user input interrupt
    case VENTI_USER_INPUT_IRQ:
      stdio_printf("Received user input interrupt on pin %d.\n", gpio);

    default:
      stdio_puts("GPIO interrupt without a type found.\n");
  }
  portYIELD_FROM_ISR(task_woken);
}

void tach_monitor(void *parameter) {
  // faketodo: make it a parameter
  size_t fan_index = 1;

  tach_queue[fan_index] = xQueueCreate(TACH_QUEUE_LENGTH, sizeof(uint64_t));


  gpio_init(TACH1_PIN);
  gpio_set_irq_enabled_with_callback(TACH1_PIN, GPIO_IRQ_EDGE_FALL, true,
                                     gpio_irq_handler);

  uint64_t last_timestamp_us = 0; // 0 is used to indicate that no tachometer signal has been received yet
  size_t interval_index = 0;

  static uint32_t fan_interval[FAN_COUNT][8];
  for (;;) {
    uint64_t new_timestamp_us;
    if (xQueueReceive(tach_queue[fan_index], &new_timestamp_us, pdMS_TO_TICKS(5000)) ==
        pdPASS) {
      if (last_timestamp_us) {
        fan_interval[fan_index][interval_index] = new_timestamp_us - last_timestamp_us;
      }
      last_timestamp_us = new_timestamp_us;
    } else {
      fan_interval[fan_index][interval_index] = 0;
    }
    interval_index = (interval_index + 1) % sizeof(fan_interval) / sizeof(fan_interval[0]);
  }
}
