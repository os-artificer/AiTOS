#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# shellcheck source=scripts/load-env.sh
source "$ROOT/scripts/load-env.sh"
load_env_rc

QEMU="${QEMU:-qemu-system-x86_64}"
QEMU_GUI="${QEMU_GUI:-0}"
TRACE="bin/boot-debugcon.log"
DEBUG_LOG="$ROOT/.cursor/debug-56499b.log"

# shellcheck source=scripts/qemu-ui.sh
source "$ROOT/scripts/qemu-ui.sh"
# shellcheck source=scripts/qemu-mac-resize.sh
source "$ROOT/scripts/qemu-mac-resize.sh"

: >"$TRACE"
UI_ARGS=()
while IFS= read -r line; do
	UI_ARGS+=("$line")
done < <(qemu_boot_trace_args "$TRACE")

echo "Starting QEMU (QEMU_GUI=${QEMU_GUI})..."
qemu_ui_hint_run
if qemu_gui_enabled && [ "$(uname -s)" = "Darwin" ]; then
	echo "macOS: auto-resize 1280x960 + zoom-to-fit (needs Accessibility permission)."
	echo "Tip: QEMU_FULLSCREEN=1 make run-qemu-gui for full-screen."
fi
echo "Quit: close QEMU window, or Ctrl-C in this terminal."

cleanup() {
	RUN_ID=gui-fix3 bash scripts/ingest-boot-log.sh "$TRACE" "$DEBUG_LOG" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

if qemu_gui_enabled && [ "$(uname -s)" = "Darwin" ]; then
	qemu_mac_resize_window 1280 960 "" "$DEBUG_LOG" &
	exec "$QEMU" \
		-drive file=bin/hd60M.img,format=raw,if=ide,index=0,media=disk \
		-drive file=bin/hd80M.img,format=raw,if=ide,index=1,media=disk \
		-m 128 \
		-cpu qemu64 \
		-no-reboot \
		"${UI_ARGS[@]}" \
		-device isa-debug-exit,iobase=0xf4,iosize=0x04
else
	exec "$QEMU" \
		-drive file=bin/hd60M.img,format=raw,if=ide,index=0,media=disk \
		-drive file=bin/hd80M.img,format=raw,if=ide,index=1,media=disk \
		-m 128 \
		-cpu qemu64 \
		-no-reboot \
		"${UI_ARGS[@]}" \
		-device isa-debug-exit,iobase=0xf4,iosize=0x04
fi
