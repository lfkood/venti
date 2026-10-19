#ifndef TACHOMETER_H
#define TACHOMETER_H
#include <stdint.h>
#include "pico/types.h"

void tach_monitor(void *parameter);
static void gpio_irq_handler(uint gpio, uint32_t event_mask);

#endif
