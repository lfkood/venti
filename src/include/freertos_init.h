#ifndef FREERTOS_INIT_H
#define FREERTOS_INIT_H

#include "FreeRTOS.h" // IWYU pragma: keep
#include "task.h"

#include "SEGGER_SYSVIEW.h"
#define VENTI_SYSVIEW_IMPLEMENTATION
#include "SEGGER_SYSVIEW_Conf.h"

#include "misc.h"

void freeRTOS_init(void);

#endif