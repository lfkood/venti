#include "venti.h"

#include "freertos_init.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "pico/stdio.h"
#include "venti_board_defs.h"

int main() {
  stdio_init_all();

  set_sys_clock_khz(150000, false);
  stdio_puts("stdio ready; starting FreeRTOS");

  freeRTOS_init();

  // Reached if the scheduler cannot allocate its startup tasks.
  panic("FreeRTOS scheduler failed to start");
}
