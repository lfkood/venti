#ifndef VENTI_TYPES
#define VENTI_TYPES

// faketodo: move other defines here
#define VENTI_WAIT_MASK (1 << 0)
#define VENTI_READ_MASK (1 << 1)
#define VENTI_WRITE_MASK (1 << 2 | VENTI_READ_MASK | VENTI_WAIT_MASK)

// User interface uses just fan index as speed update notification (0..FAN_COUNT-1)
#define FAN_CONTROL_SPEED_NOTIFICATION_INDEX 0
#define FAN_CONTROL_COMMAND_INDEX 1
#define FAN_CONTROL_CURVE_UPDATE_INDEX 2
// Interface state slots follow the per-fan speed slots.
#define FAN_INTERFACE_STATE_INDEX(fan_index) (FAN_COUNT + (fan_index))

#define tskBLINK_PRIORITY tskIDLE_PRIORITY + 1
#define tskINTERFACE_PRIORITY tskIDLE_PRIORITY + 2
#define tskFAN_PRIORITY tskIDLE_PRIORITY + 3

// freertos_init.c
#define USER_ERROR_QUEUE_LENGTH 16
#define USER_COMMAND_QUEUE_LENGTH 16

// tachometer.c
#define TACH_QUEUE_LENGTH 32
#define FAN_INTERVAL_LENGTH 8
#define FAN_CURVE_MAX_LENGTH 16

#define PID_DT_MS 10

// usb_interface.c
#define RX_LINE_LENGTH 128
#define STAT_COLUMN_COUNT 6
#define OUTPUT_BUFFER_SIZE 1024

#endif
