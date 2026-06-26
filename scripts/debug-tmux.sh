#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# shellcheck source=scripts/load-env.sh
source "$ROOT/scripts/load-env.sh"
load_env_rc

GDB_PORT="${GDB_PORT:-1234}"
GDB="${GDB:-gdb}"
QEMU_GUI="${QEMU_GUI:-0}"
SESSION="aitos-debug"
PTY_FILE="bin/qemu-serial.pty"

# shellcheck source=scripts/qemu-ui.sh
source "$ROOT/scripts/qemu-ui.sh"

if ! command -v tmux >/dev/null 2>&1; then
	echo "tmux not found; falling back to: make debug"
	exec make debug
fi

bash scripts/debug-qemu.sh stop 2>/dev/null || true
tmux has-session -t "$SESSION" 2>/dev/null && tmux kill-session -t "$SESSION"

bash scripts/debug-qemu.sh start

if qemu_gui_enabled; then
	# GUI: serial log pane + GDB; Shell keyboard goes to QEMU window.
	tmux new-session -d -s "$SESSION" -n debug \
		"cd '$ROOT' && tail -f bin/qemu-serial.log"
	tmux split-window -h -t "$SESSION:0" \
		"cd '$ROOT' && bash scripts/gdb-console.sh"
	tmux select-pane -t "$SESSION:0.1"
	tmux attach -t "$SESSION"
	exit 0
fi

pty_path=""
if [ -f "$PTY_FILE" ]; then
	pty_path=$(cat "$PTY_FILE")
fi

if [ -n "$pty_path" ] && [ -e "$pty_path" ] && command -v screen >/dev/null 2>&1; then
	# Headless: serial Shell console + GDB + debugcon log.
	tmux new-session -d -s "$SESSION" -n debug \
		"cd '$ROOT' && screen '${pty_path}'"
	tmux split-window -v -t "$SESSION:0" \
		"cd '$ROOT' && bash scripts/gdb-console.sh"
	tmux split-window -h -t "$SESSION:0.0" \
		"cd '$ROOT' && tail -f bin/qemu-serial.log"
	tmux select-pane -t "$SESSION:0.1"
else
	# Fallback when screen/pty unavailable.
	tmux new-session -d -s "$SESSION" -n debug \
		"cd '$ROOT' && tail -f bin/qemu-serial.log"
	tmux split-window -h -t "$SESSION:0" \
		"cd '$ROOT' && bash scripts/gdb-console.sh"
	tmux select-pane -t "$SESSION:0.1"
	if [ -n "$pty_path" ]; then
		echo "Serial console: screen ${pty_path}" >&2
	fi
fi

tmux attach -t "$SESSION"
