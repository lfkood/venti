#!/usr/bin/env bash
# Used by the VS Code "Start RTT (flash + OpenOCD)" task.
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
openocd_dir="${HOME}/.pico-sdk/openocd/0.12.0+dev"
cd -- "$project_dir"

if [[ ! -f build/venti.elf ]]; then
    echo "Missing build/venti.elf. Run the Compile Project task first." >&2
    exit 1
fi

# Keep OpenOCD running for the RTT console and the external GDB attach profile.
# Search RP2350 main SRAM for the control block after the firmware initializes it.
exec "$openocd_dir/openocd" \
    -s "$openocd_dir/scripts" \
    -f interface/cmsis-dap.cfg \
    -f target/rp2350.cfg \
    -c "bindto 127.0.0.1" \
    -c "adapter speed 5000" \
    -c "program build/venti.elf verify reset" \
    -c "targets rp2350.cm0" \
    -c "sleep 500" \
    -c "rtt setup 0x20000000 0x80000 {SEGGER RTT}" \
    -c "rtt polling_interval 10" \
    -c "rtt start" \
    -c "rtt server start 9090 0"
