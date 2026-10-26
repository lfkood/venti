#ifndef VENTI_BOARD_DEFINES_H
#define VENTI_BOARD_DEFINES_H

#include "stdint.h"
#include "hardware/gpio.h"

#define FAN_COUNT 3
enum {
  TACH_PIN_0 = 4,
  TACH_PIN_1 = 6,
  TACH_PIN_2 = 8,
};

static const uint TACH_PIN[FAN_COUNT] = {
  TACH_PIN_0,
  TACH_PIN_1,
  TACH_PIN_2,
};
#define MAX_INTERRUPT_PIN 16 // controls interrupt handler lookup size

typedef struct {
    uint16_t voltage_control;
    uint16_t pwm_control;
} fan_control_pins_t;

static const fan_control_pins_t FAN0_CONTROL_PINS = {.voltage_control = 1, .pwm_control = 5};
static const fan_control_pins_t FAN1_CONTROL_PINS = {.voltage_control = 2, .pwm_control = 7};
static const fan_control_pins_t FAN2_CONTROL_PINS = {.voltage_control = 3, .pwm_control = 9};

typedef enum {
  VENTI_TACH_IRQ = 0,
  VENTI_USER_INPUT_IRQ,
} venti_irq_type_t;

typedef struct {
  uint32_t event_mask;
  venti_irq_type_t type;
  size_t fan_index;
} venti_gpio_irq_t;

static venti_gpio_irq_t VENTI_IRQ_TABLE[MAX_INTERRUPT_PIN] = {
    [TACH_PIN_0] = {
        .event_mask = GPIO_IRQ_EDGE_FALL,
        .type = VENTI_TACH_IRQ,
        .fan_index = 0,
    },
    [TACH_PIN_1] = {
        .event_mask = GPIO_IRQ_EDGE_FALL,
        .type = VENTI_TACH_IRQ,
        .fan_index = 1,
    },
    [TACH_PIN_2] = {
        .event_mask = GPIO_IRQ_EDGE_FALL,
        .type = VENTI_TACH_IRQ,
        .fan_index = 2,
    },
};


#endif
