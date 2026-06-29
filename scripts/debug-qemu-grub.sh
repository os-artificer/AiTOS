#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# shellcheck source=scripts/load-env.sh
source "$ROOT/scripts/load-env.sh"
load_env_rc

GDB_PORT="${GDB_PORT:-1234}"
QEMU="${QEMU:-qemu-system-x86_64}"
QEMU_GUI="${QEMU_GUI:-0}"
GRUB_BOOT="${GRUB_BOOT:-iso}"
PID_FILE="bin/qemu-debug.pid"
SERIAL_LOG="bin/qemu-serial.log"
PTY_FILE="bin/qemu-serial.pty"
QEMU_STDERR="bin/qemu-debug.stderr"

# shellcheck source=scripts/qemu-ui.sh
source "$ROOT/scripts/qemu-ui.sh"

free_gdb_port() {
	local pid=""

	if command -v ss >/dev/null 2>&1; then
		pid=$(ss -tlnp 2>/dev/null | grep ":${GDB_PORT} " | sed -n 's/.*pid=\([0-9]*\).*/\1/p' | head -1 || true)
	elif command -v lsof >/dev/null 2>&1; then
		pid=$(lsof -tiTCP:"${GDB_PORT}" -sTCP:LISTEN 2>/dev/null | head -1 || true)
	fi

	if [ -n "$pid" ] && { [ ! -f "$PID_FILE" ] || [ "$(cat "$PID_FILE" 2>/dev/null)" != "$pid" ]; }; then
		kill "$pid" 2>/dev/null || true
		sleep 0.2
	fi
}

stop_qemu() {
	if [ -f "$PID_FILE" ]; then
		local pid
		pid=$(cat "$PID_FILE")
		if kill -0 "$pid" 2>/dev/null; then
			kill "$pid" 2>/dev/null || true
			sleep 0.2
			kill -0 "$pid" 2>/dev/null && kill -9 "$pid" 2>/dev/null || true
		fi
		rm -f "$PID_FILE"
	fi
	rm -f "$PTY_FILE" "$QEMU_STDERR"
}

stop_gdb_sessions() {
	local killed=0
	local pid cmd cwd

	if [ ! -d /proc ]; then
		return 0
	fi

	for pid in $(pgrep -x gdb 2>/dev/null || true); do
		cmd=$(tr '\0' ' ' <"/proc/${pid}/cmdline" 2>/dev/null || true)
		case "$cmd" in
			*build/kernel.elf* | *build/kernel.bin*) ;;
			*) continue ;;
		esac
		cwd=$(readlink -f "/proc/${pid}/cwd" 2>/dev/null || true)
		[ "$cwd" = "$ROOT" ] || continue
		kill -TERM "$pid" 2>/dev/null || true
		killed=$((killed + 1))
	done
	if [ "$killed" -gt 0 ]; then
		sleep 0.3
		for pid in $(pgrep -x gdb 2>/dev/null || true); do
			cmd=$(tr '\0' ' ' <"/proc/${pid}/cmdline" 2>/dev/null || true)
			case "$cmd" in
				*build/kernel.elf* | *build/kernel.bin*) ;;
				*) continue ;;
			esac
			cwd=$(readlink -f "/proc/${pid}/cwd" 2>/dev/null || true)
			[ "$cwd" = "$ROOT" ] || continue
			kill -0 "$pid" 2>/dev/null && kill -9 "$pid" 2>/dev/null || true
		done
		echo "Stopped ${killed} GDB session(s)."
	fi
}

stop_debug_wrappers() {
	local pid cmd

	if [ ! -d /proc ]; then
		return 0
	fi

	for pid in $(pgrep -x script 2>/dev/null || true); do
		cmd=$(tr '\0' ' ' <"/proc/${pid}/cmdline" 2>/dev/null || true)
		case "$cmd" in
			*scripts/debug.sh* | *scripts/debug-grub.sh* | *build/gdb/aitos.gdb*)
				kill -TERM "$pid" 2>/dev/null || true
				;;
		esac
	done
}

stop_all() {
	stop_qemu
	free_gdb_port
	stop_gdb_sessions
	stop_debug_wrappers
	echo "Debug QEMU (GRUB) stopped."
}

grub_boot_media() {
	case "$GRUB_BOOT" in
	iso)
		export QEMU_BOOT=c
		BOOT_MEDIA=(-drive file=bin/aitos.iso,format=raw,if=ide,index=0,media=disk)
		;;
	disk)
		export QEMU_BOOT=c
		BOOT_MEDIA=(-drive "file=bin/aitos-grub.img,format=raw,if=ide,index=0,media=disk")
		;;
	*)
		echo "error: GRUB_BOOT must be iso or disk" >&2
		exit 1
		;;
	esac
}

start_qemu() {
	mkdir -p bin
	stop_qemu
	free_gdb_port

	if [ "$GRUB_BOOT" = "iso" ]; then
		make DEBUG=1 build grub-iso gdbscripts
	else
		make DEBUG=1 build grub-disk gdbscripts
	fi

	grub_boot_media

	: >"$SERIAL_LOG"
	rm -f "$PTY_FILE"

	UI_ARGS=(-nic none)
	while IFS= read -r line; do
		UI_ARGS+=("$line")
	done < <(qemu_gui_display_args)

	if ! qemu_gui_enabled; then
		UI_ARGS+=(-serial pty)
	fi

	DATA_DRIVE=()
	if [ "$GRUB_BOOT" = "disk" ]; then
		DATA_DRIVE=(-drive file=bin/hd80M.img,format=raw,if=ide,index=1,media=disk)
	fi

	# bash 3.2 (macOS): guard possibly-empty arrays so `set -u` does not abort.
	"$QEMU" \
		"${BOOT_MEDIA[@]}" \
		${DATA_DRIVE[@]+"${DATA_DRIVE[@]}"} \
		-m 128 \
		-cpu qemu64 \
		-no-reboot \
		${UI_ARGS[@]+"${UI_ARGS[@]}"} \
		-debugcon "file:${SERIAL_LOG}" \
		-S \
		-gdb "tcp:127.0.0.1:${GDB_PORT}" \
		>"$QEMU_STDERR" 2>&1 &
	echo $! >"$PID_FILE"

	sleep 0.5

	if ! qemu_gui_enabled; then
		qemu_capture_pty_path "$QEMU_STDERR" "$PTY_FILE"
		if [ ! -s "$PTY_FILE" ] && [ -f "$SERIAL_LOG" ]; then
			qemu_capture_pty_path "$SERIAL_LOG" "$PTY_FILE"
		fi
	fi

	echo "QEMU GRUB debug stub started (GRUB_BOOT=${GRUB_BOOT}, QEMU_GUI=${QEMU_GUI})"
	echo "GDB target: 127.0.0.1:${GDB_PORT}"
	qemu_ui_hint_debug "$(cat "$PTY_FILE" 2>/dev/null || true)"
	echo "Run: make debug-grub"
}

case "${1:-start}" in
start) start_qemu ;;
stop) stop_all ;;
*)
	echo "Usage: $0 {start|stop}" >&2
	exit 1
	;;
esac
