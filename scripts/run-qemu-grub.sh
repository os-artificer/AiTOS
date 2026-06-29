#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# shellcheck source=scripts/load-env.sh
source "$ROOT/scripts/load-env.sh"
load_env_rc

QEMU="${QEMU:-qemu-system-x86_64}"
QEMU_GUI="${QEMU_GUI:-0}"
GRUB_BOOT="${GRUB_BOOT:-iso}"
TRACE="bin/boot-debugcon.log"

# shellcheck source=scripts/qemu-ui.sh
source "$ROOT/scripts/qemu-ui.sh"

if [ "$QEMU_GUI" = "1" ]; then
	echo "warning: GRUB path Phase 1 is headless only; forcing QEMU_GUI=0" >&2
	QEMU_GUI=0
fi

case "$GRUB_BOOT" in
iso)
	QEMU_BOOT=c
	BOOT_MEDIA=(-drive file=bin/aitos.iso,format=raw,if=ide,index=0,media=disk)
	;;
disk)
	QEMU_BOOT=c
	BOOT_MEDIA=(-drive "file=bin/aitos-grub.img,format=raw,if=ide,index=0,media=disk")
	;;
*)
	echo "error: GRUB_BOOT must be iso or disk (got: $GRUB_BOOT)" >&2
	exit 1
	;;
esac

export QEMU_BOOT

: >"$TRACE"
UI_ARGS=()
while IFS= read -r line; do
	UI_ARGS+=("$line")
done < <(qemu_boot_trace_args "$TRACE")

echo "Starting QEMU GRUB (GRUB_BOOT=${GRUB_BOOT}, QEMU_BOOT=${QEMU_BOOT})..."
qemu_ui_hint_run
echo "Kernel log: tail -f ${TRACE}   (GRUB headless output goes to debugcon)"
echo "Quit: Ctrl-C in this terminal."

DATA_DRIVE=()
if [ "$GRUB_BOOT" = "disk" ]; then
	DATA_DRIVE=(-drive file=bin/hd80M.img,format=raw,if=ide,index=1,media=disk)
fi

if [ "${QEMU_MIRROR_DEBUGCON:-1}" = "1" ] && [ -t 1 ]; then
	(
		while [ ! -s "$TRACE" ]; do
			sleep 0.2
		done
		tail -f "$TRACE"
	) &
	TAIL_PID=$!
	trap 'kill "$TAIL_PID" 2>/dev/null || true' EXIT INT TERM
fi

# Note: macOS ships bash 3.2, where "${arr[@]}" on an empty array under `set -u`
# raises "unbound variable". DATA_DRIVE is empty for the iso path, so guard the
# possibly-empty arrays with the ${arr[@]+...} form (expands to nothing if empty).
exec "$QEMU" \
	"${BOOT_MEDIA[@]}" \
	${DATA_DRIVE[@]+"${DATA_DRIVE[@]}"} \
	-m 128 \
	-cpu qemu64 \
	-no-reboot \
	${UI_ARGS[@]+"${UI_ARGS[@]}"} \
	-device isa-debug-exit,iobase=0xf4,iosize=0x04
