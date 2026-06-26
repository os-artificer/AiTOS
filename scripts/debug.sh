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
GDB_SESSION="${ROOT}/.cursor/gdb-session-af2b5b.gdb"
PTY_FILE="bin/qemu-serial.pty"

# shellcheck source=scripts/qemu-ui.sh
source "$ROOT/scripts/qemu-ui.sh"

ensure_tty() {
	if [ -t 0 ]; then
		return 0
	fi
	exec script -q -c "bash \"$0\"" /dev/null
}

ensure_tty

cleanup() {
	bash scripts/debug-qemu.sh stop
}
trap cleanup EXIT INT TERM

bash scripts/debug-qemu.sh start

cat >"$GDB_SESSION" <<EOF
target remote 127.0.0.1:${GDB_PORT}
source build/gdb/aitos-debug.gdb
EOF

cat <<'MSG'

GDB connected at CPU reset.
  Type:  c          (continue — stops at kmain after boot)

退出调试: 在 GDB 输入 quit
         或另开终端执行 make stop（会停止 QEMU 与 GDB）

MSG

qemu_ui_hint_debug "$(cat "$PTY_FILE" 2>/dev/null || true)"
echo

set +e
if [ -e /dev/tty ]; then
	"$GDB" -q -x build/gdb/aitos.gdb -x "$GDB_SESSION" build/kernel.elf </dev/tty
else
	"$GDB" -q -x build/gdb/aitos.gdb -x "$GDB_SESSION" build/kernel.elf
fi
GDB_EXIT_CODE=$?
set -e

exit "$GDB_EXIT_CODE"
