#ifndef VENTI_FREERTOS_CONFIG_H
#define VENTI_FREERTOS_CONFIG_H

// Start with scheduling on one core.
#define FREE_RTOS_KERNEL_SMP 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 8

#include "FreeRTOSConfig_examples_common.h"
#include "SEGGER_SYSVIEW_FreeRTOS.h"

#endif
