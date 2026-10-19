#include "venti.h"

#include "pico/stdio.h"
#include "hardware/gpio.h"


#include "board_definitions.h"

#include "freertos_init.h"

int main() {
  stdio_init_all();

  stdio_puts("venti: RTT ready; starting FreeRTOS");
  
  freeRTOS_init();

  // Reached if the scheduler cannot allocate its startup tasks.
  panic("FreeRTOS scheduler failed to start");
}
