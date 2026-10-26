#ifndef TACHOMETER_H
#define TACHOMETER_H
#include "FreeRTOS.h"
#include "fan_command.h"
#include "pico/types.h"
#include "stdint.h"
#include "task.h"
#include "venti_board_defs.h"
#include "venti_defs.h"

typedef struct {
  TaskHandle_t fan_controller;
  // for sending fan speed back to interface
  size_t recipient_count;
  TaskHandle_t recipients[2];
  size_t fan_index;
} tach_monitor_parameters_t;

typedef struct {
  int16_t temp_celsius_x10[FAN_CURVE_MAX_LENGTH];
  uint16_t duty_cycle[FAN_CURVE_MAX_LENGTH];
  size_t length;
} fan_curve_t;

typedef struct {
  fan_control_pins_t control_pins;
  fan_curve_t fan_curve;
  // for sending controle type back to interface
  size_t recipient_count;
  TaskHandle_t recipients[2];
  size_t fan_index;
} fan_control_parameters_t;

void tach_monitor(void* pvParameters);
void fan_control(void* pvParameters);

extern uint32_t MAX_RPM[FAN_COUNT];

#endif
