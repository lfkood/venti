#ifndef VENTI_FREERTOS_CONFIG_H
#define VENTI_FREERTOS_CONFIG_H
#include "board_definitions.h"

// Start with scheduling on one core.
#define configNUMBER_OF_CORES 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES FAN_COUNT

#include "FreeRTOSConfig_examples_common.h"
#include "SEGGER_SYSVIEW_FreeRTOS.h"

#endif
