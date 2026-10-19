#include "venti.h"

int main() {
  stdio_init_all();
  tachometer_init();

  stdio_puts("venti: RTT ready; starting FreeRTOS");
  
  freeRTOS_init();

  // Reached if the scheduler cannot allocate its startup tasks.
  panic("FreeRTOS scheduler failed to start");
}
