#ifndef MISC_H
#define MISC_H

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

static void blink_task(void *parameter);

#endif