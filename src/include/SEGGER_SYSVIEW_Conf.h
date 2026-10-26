/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
*                                                                    *
*         SEGGER SystemView  * Real-time application analysis        *
*              https://github.com/SEGGERMicro/SystemView             *
*                                                                    *
**********************************************************************

---------------------------END-OF-HEADER------------------------------

Purpose : SEGGER SystemView configuration file.
          Set defines which deviate from the defaults (see SEGGER_SYSVIEW_ConfDefaults.h) here.          

Additional information:
  Required defines which must be set are:
    SEGGER_SYSVIEW_GET_TIMESTAMP
    SEGGER_SYSVIEW_GET_INTERRUPT_ID
  For known compilers and cores, these might be set to good defaults
  in SEGGER_SYSVIEW_ConfDefaults.h.
  
  SystemView needs a (nestable) locking mechanism.
  If not defined, the RTT locking mechanism is used,
  which then needs to be properly configured.
*/

#ifndef SEGGER_SYSVIEW_CONF_H
#define SEGGER_SYSVIEW_CONF_H

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

/*********************************************************************
*
*       Define: SEGGER_SYSVIEW_SECTION
*
*  Description
*    Section to place the SystemView RTT Buffer into.
*  Default
*    undefined: Do not place into a specific section.
*  Notes
*    If SEGGER_RTT_SECTION is defined, the default changes to use
*    this section for the SystemView RTT Buffer, too.
*/
#if !(defined SEGGER_SYSVIEW_SECTION) && (defined SEGGER_RTT_BUFFER_SECTION)
  #define SEGGER_SYSVIEW_SECTION                  SEGGER_RTT_BUFFER_SECTION
#endif


/*********************************************************************
* TODO: Add your defines here.                                       *
**********************************************************************
*/

#define SEGGER_SYSVIEW_RTT_CHANNEL 1
// Allow for bursts and polling/forwarding latency through the IP probe.
#define SEGGER_SYSVIEW_RTT_BUFFER_SIZE (128u * 1024u)
// Start on the recorder's command, not before a host is draining the buffer.
#define SEGGER_SYSVIEW_START_ON_INIT 0

// Use the Pico hardware timer instead of the debugger-dependent DWT counter.
// This timer is already running before main(), including before the scheduler.
#define SEGGER_SYSVIEW_GET_TIMESTAMP() SEGGER_SYSVIEW_X_GetTimestamp()
#define SEGGER_SYSVIEW_TIMESTAMP_BITS 32
#define VENTI_SYSVIEW_TIMESTAMP_FREQ 1'000'000u
#define VENTI_SYSVIEW_CPU_FREQ clock_get_hz(clk_sys)
#define VENTI_SYSVIEW_RAM_BASE SRAM_BASE
#define VENTI_SYSVIEW_SYSTEM_DESC "N=venti,D=RP2350 Cortex-M33,O=FreeRTOS"


#endif  // SEGGER_SYSVIEW_CONF_H

// In exactly one source file, include SEGGER_SYSVIEW.h first, then define
// VENTI_SYSVIEW_IMPLEMENTATION and include this header again. This section is
// outside the configuration guard because SEGGER includes the settings before
// declaring the API types used below.
#if defined(VENTI_SYSVIEW_IMPLEMENTATION) && !defined(VENTI_SYSVIEW_IMPLEMENTED)
#define VENTI_SYSVIEW_IMPLEMENTED

#include "hardware/clocks.h"
#include "hardware/regs/addressmap.h"
#include "hardware/timer.h"

extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

static void venti_sysview_send_system_description(void) {
  SEGGER_SYSVIEW_SendSysDesc(VENTI_SYSVIEW_SYSTEM_DESC);
  SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
}

U32 SEGGER_SYSVIEW_X_GetTimestamp(void) {
  return time_us_32();
}

void SEGGER_SYSVIEW_Conf(void) {
  SEGGER_SYSVIEW_Init(VENTI_SYSVIEW_TIMESTAMP_FREQ, VENTI_SYSVIEW_CPU_FREQ,
                     &SYSVIEW_X_OS_TraceAPI, venti_sysview_send_system_description);
  SEGGER_SYSVIEW_SetRAMBase(VENTI_SYSVIEW_RAM_BASE);

#if SEGGER_SYSVIEW_START_ON_INIT
  SEGGER_SYSVIEW_Start();
#endif
}

#endif  // VENTI_SYSVIEW_IMPLEMENTATION && !VENTI_SYSVIEW_IMPLEMENTED

/*************************** End of file ****************************/
