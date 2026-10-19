#ifndef TACHOMETER_H
#define TACHOMETER_H

#include "hardware/gpio.h"
#include "pico/time.h"
#include "hardware/irq.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "board_definitions.h"

static void tach_irq(void);

#endif