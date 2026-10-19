#!/usr/bin/env bash
set -euo pipefail

# Windows must already be running OpenOCD for the RP2350, for example:
# openocd.exe -c "bindto 0.0.0.0" -f interface/cmsis-dap.cfg \
#   -f target/rp2350.cfg -c "adapter speed 25000"
# RTT commands: https://openocd.org/doc/html/General-Commands.html#Real-Time-Transfer-_0028RTT_0029
gdb=${1:?Expected the WSL GDB executable}
elf=${2:?Expected the firmware ELF path}

if [[ ! -x "$gdb" || ! -f "$elf" ]]; then
    printf 'GDB executable or firmware ELF missing:\n  %s\n  %s\n' "$gdb" "$elf" >&2
    exit 1
fi

host=${OPENOCD_HOST:-}
if [[ -z "$host" ]]; then
    if command -v wslinfo >/dev/null && [[ $(wslinfo --networking-mode 2>/dev/null) == mirrored ]]; then
        host=127.0.0.1
    else
        host=$(ip -4 route show default | awk '/default/ {print $3; exit}')
    fi
fi
if [[ -z "$host" ]]; then
    printf 'Cannot detect the Windows host. Set OPENOCD_HOST to its IP address.\n' >&2
    exit 1
fi

monitor_pid=
cleanup() {
    local status=$?
    trap - EXIT
    trap '' INT TERM HUP
    if [[ -n "$monitor_pid" ]]; then
        kill "$monitor_pid" 2>/dev/null || true
        wait "$monitor_pid" 2>/dev/null || true
    fi
    exec 3>&-

    printf '\nLocal session ended.\n'
    exit "$status"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
trap 'exit 129' HUP

printf '\nFlashing %s through Windows OpenOCD at %s:3333.\n' "$elf" "$host"
# GDB reads the ELF in WSL and transfers it over the existing GDB connection.
# Stop any previous RTT session before resetting/reflashing; on the first run
# there may be no RTT listener yet. Reconfigure RTT after the firmware starts
# so a previous firmware's control-block address is not reused.
"$gdb" --quiet --nx --batch \
    -ex 'set confirm off' \
    -ex 'set remotetimeout 10' \
    -ex "target extended-remote $host:3333" \
    -ex 'monitor adapter speed 25000' \
    -ex 'monitor targets rp2350.cm0' \
    -ex 'monitor catch {rtt stop}' \
    -ex 'monitor catch {rtt server stop 9090}' \
    -ex 'monitor reset init' \
    -ex 'load' \
    -ex 'monitor reset run' \
    -ex 'monitor sleep 500' \
    -ex 'monitor targets rp2350.cm0' \
    -ex 'monitor rtt setup 0x20000000 0x80000 {SEGGER RTT}' \
    -ex 'monitor rtt polling_interval 10' \
    -ex 'monitor rtt start' \
    -ex 'monitor rtt server start 9090 0' \
    -ex 'detach' \
    --se="$elf"

printf '\nMonitoring RTT channel 0 at %s:9090.\n' "$host"
if ! exec 3<>"/dev/tcp/$host/9090"; then
    printf 'Cannot connect to RTT. Check the Windows OpenOCD bind address and firewall for port 9090.\n' >&2
    exit 1
fi
# Waiting for a background reader lets Bash handle signals immediately.
cat <&3 &
monitor_pid=$!
wait "$monitor_pid"
monitor_pid=
