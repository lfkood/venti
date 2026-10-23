#ifndef VENTI_IRQ_H
#define VENTI_IRQ_H

#include "pico/types.h"

void gpio_irq_handler(uint gpio, uint32_t event_mask);

#endif