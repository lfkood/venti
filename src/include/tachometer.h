#ifndef TACHOMETER_H
#define TACHOMETER_H
#include "FreeRTOS.h"
#include "pico/types.h"
#include "stdint.h"
#include "task.h"
#include "venti_board_defs.h"
#include "venti_defs.h"

typedef struct {
  size_t fan_index;
  TaskHandle_t fan_controller;
  size_t recipient_count;
  TaskHandle_t recipients[2];
} tach_monitor_parameters_t;

typedef struct {
    uint16_t temp_celsius_x10[FAN_CURVE_MAX_LENGTH];
    uint16_t duty_cycle[FAN_CURVE_MAX_LENGTH];
    size_t length;
} fan_curve_t;

typedef struct {
  fan_control_pins_t control_pins;
  fan_curve_t fan_curve;
} fan_control_parameters_t;


typedef enum : uint8_t {
  SPEED_SELECT_PID,
  SPEED_SELECT_PWM,
  SPEED_SELECT_AUTO,
  SPEED_SELECT_OFF,
} fan_control_select_t;
typedef enum : uint8_t { 
  CONTROL_SET_PWM=0,
  CONTOL_SET_VOLTAGE=1,
  CONTROL_UNCHANGED,
} fan_control_type_t;

typedef struct {
  uint16_t speed_value;
  fan_control_select_t control_select;
  fan_control_type_t control_type;
} fan_control_command_t;

typedef union {
  fan_control_command_t as_command;
  uint32_t as_notification;
} fan_control_notification_t;

_Static_assert(sizeof(fan_control_notification_t) == 4,
               "Fan notification must be 4 bytes");

void tach_monitor(void* pvParameters);
void fan_control(void* pvParameters);

#endif
