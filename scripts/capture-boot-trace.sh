#!/usr/bin/env bash
# Capture boot trace (debugcon) for diagnosis; used by debug mode.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# shellcheck source=scripts/load-env.sh
source "$ROOT/scripts/load-env.sh"
load_env_rc

QEMU="${QEMU:-qemu-system-x86_64}"
QEMU_GUI="${QEMU_GUI:-1}"
RUN_ID="${RUN_ID:-pre-fix}"
export RUN_ID

# shellcheck source=scripts/qemu-ui.sh
source "$ROOT/scripts/qemu-ui.sh"

make image
: >bin/boot-debugcon.log

ARGS=()
while IFS= read -r line; do
	ARGS+=("$line")
done < <(qemu_boot_trace_args bin/boot-debugcon.log)

echo "Capturing boot trace (QEMU_GUI=${QEMU_GUI}, 12s)..."
"$QEMU" \
	-drive file=bin/hd60M.img,format=raw,if=ide,index=0,media=disk \
	-drive file=bin/hd80M.img,format=raw,if=ide,index=1,media=disk \
	-m 128 \
	-cpu qemu64 \
	-no-reboot \
	"${ARGS[@]}" \
	-device isa-debug-exit,iobase=0xf4,iosize=0x04 &
QPID=$!
sleep 12
kill "$QPID" 2>/dev/null || true
wait "$QPID" 2>/dev/null || true

echo "boot-debugcon: $(tr -d '\n' <bin/boot-debugcon.log)"
bash scripts/ingest-boot-log.sh
