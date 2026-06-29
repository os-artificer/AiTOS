#!/usr/bin/env bash
# Non-interactive check: GRUB boot reaches shell prompt (and optionally runs help).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# shellcheck source=scripts/load-env.sh
source "$ROOT/scripts/load-env.sh"
load_env_rc

QEMU="${QEMU:-qemu-system-x86_64}"
TRACE="${TRACE:-bin/boot-debugcon.log}"
BOOT_SECS="${BOOT_SECS:-14}"
RUN_CMD="${RUN_CMD:-0}"
if [ "$RUN_CMD" = "1" ]; then
	BOOT_SECS="${BOOT_SECS:-20}"
fi
SKIP_BUILD="${SKIP_BUILD:-0}"

if [ "$SKIP_BUILD" != "1" ]; then
	if [ "$RUN_CMD" = "1" ]; then
		make build grub-iso CFLAGS_EXTRA="-DGRUB_SHELL_SELFTEST" >/dev/null
	else
		make build grub-iso >/dev/null
	fi
fi
pkill -9 qemu-system-x86_64 2>/dev/null || true
sleep 0.2

: >"$TRACE"

QEMU_ARGS=(
	-drive file=bin/aitos.iso,format=raw,if=ide,index=0,media=disk
	-boot order=c,menu=off
	-m 128
	-cpu qemu64
	-no-reboot
	-debugcon "file:${TRACE}"
	-nographic
	-nic none
	-device isa-debug-exit,iobase=0xf4,iosize=0x04
)

if [ "$RUN_CMD" = "1" ]; then
	# Kernel self-test runs help/version/echo at shell entry (no serial input needed).
	timeout "$BOOT_SECS" "$QEMU" "${QEMU_ARGS[@]}" >/dev/null 2>&1 || true
else
	timeout "$BOOT_SECS" "$QEMU" "${QEMU_ARGS[@]}" >/dev/null 2>&1 || true
fi

if ! grep -aq 'aitos@localhost' "$TRACE"; then
	echo "verify-grub-shell: FAIL — shell prompt not found in ${TRACE}" >&2
	echo "--- debugcon tail ---" >&2
	tr -d '\r' <"$TRACE" | tail -20 >&2
	exit 1
fi

if [ "$RUN_CMD" = "1" ]; then
	if ! grep -aq 'Available commands' "$TRACE"; then
		echo "verify-grub-shell: FAIL — help output not found in ${TRACE}" >&2
		tr -d '\r' <"$TRACE" | tail -30 >&2
		exit 1
	fi
	echo "verify-grub-shell: OK (shell prompt + help/version/echo self-test)"
else
	echo "verify-grub-shell: OK (shell prompt)"
fi
