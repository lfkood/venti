#include "tachometer.h"

static QueueHandle_t tach_queue;

void tachometer_init(void) {
  gpio_init(TACH1_PIN);
  gpio_add_raw_irq_handler(TACH1_PIN, tach_irq);
  gpio_set_irq_enabled(TACH1_PIN, GPIO_IRQ_EDGE_FALL, true);
  tach_queue = xQueueCreate(10, sizeof(uint64_t));
}

#define TACH_BUF_LENGTH 32

typedef struct {
  uint64_t timestamps_us[TACH_BUF_LENGTH];
} tach_event_t;

static void tach_irq() {
  uint64_t timestamp_us = time_us_64();

  BaseType_t task_woken = pdFALSE;

  if (xQueueSendFromISR(tach_queue, &timestamp_us, &task_woken) != pdPASS) {
    record_tach_overflow_from_isr(); /* Must be ISR-safe. */
  }

  portYIELD_FROM_ISR(task_woken);
}

static void tach_monitor() {
  
}