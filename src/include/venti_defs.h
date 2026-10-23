#ifndef VENTI_TYPES
#define VENTI_TYPES

// faketodo: move other defines here
#define VENTI_WAIT_MASK (1 << 0)
#define VENTI_READ_MASK (1 << 1)
#define VENTI_WRITE_MASK (1 << 2 | VENTI_READ_MASK | VENTI_WAIT_MASK)

// User interface uses just fan index as speed update notification (0..FAN_COUNT-1)
#define FAN_CONTROL_SPEED_NOTIFICATION_INDEX 0
#define FAN_CONTROL_COMMAND_INDEX 1
#define FAN_CONTROL_CURVE_UPDATE_INDEX 1


// freertos_init.c
#define USER_ERROR_QUEUE_LENGTH 16

// tachometer.c
#define TACH_QUEUE_LENGTH 32
#define FAN_INTERVAL_LENGTH 8
#define FAN_CURVE_MAX_LENGTH 16

#define PID_DT_MS  10
#define PID_DT_F   (PID_DT_MS / 1000.0f)
#define PID_DT_US  (PID_DT_MS * 1000)

#endif