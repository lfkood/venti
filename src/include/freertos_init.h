#ifndef VENTI_FREERTOS_INIT_H
#define VENTI_FREERTOS_INIT_H
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "venti_board_defs.h"
#include "venti_defs.h"

typedef enum {
    PID_FAN_SPEED_TOO_LOW,
} user_error_t;

extern QueueHandle_t user_error_queue;
extern TaskHandle_t TACH_MONITOR[FAN_COUNT];
extern TaskHandle_t FAN_CONTROL[FAN_COUNT];

void freeRTOS_init(void);


#endif