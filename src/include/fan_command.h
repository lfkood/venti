#ifndef VENTI_FAN_COMMAND_H
#define VENTI_FAN_COMMAND_H

#include <stdint.h>

typedef enum : uint8_t {
  SPEED_SELECT_PID,
  SPEED_SELECT_PWM,
  SPEED_SELECT_AUTO,
  SPEED_SELECT_OFF,
} fan_control_select_t;
typedef enum : uint8_t {
  CONTROL_SET_PWM = 0,
  CONTROL_SET_VOLTAGE = 1,
  CONTROL_UNCHANGED,
} fan_control_type_t;

typedef struct {
  fan_control_select_t control_select;
  fan_control_type_t control_type;
  uint16_t speed_value;
} fan_control_command_t;

typedef struct {
  fan_control_select_t control_select;
  fan_control_type_t control_type;
  uint16_t speed_value;
} fan_control_state_t;

typedef union {
  fan_control_command_t as_command;
  uint32_t as_notification;
} fan_control_notification_t;

_Static_assert(sizeof(fan_control_notification_t) == 4,
               "Fan notification must be 4 bytes");

typedef union {
  fan_control_state_t as_state;
  uint32_t as_notification;
} fan_state_notification_t;

_Static_assert(sizeof(fan_state_notification_t) == 4,
               "Fan notification must be 4 bytes");

#endif
