#!/usr/bin/env bash
# Interactive GDB console for an already-running QEMU debug stub.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
# shellcheck source=scripts/load-env.sh
source "$ROOT/scripts/load-env.sh"
load_env_rc

GDB_PORT="${GDB_PORT:-1234}"
GDB="${GDB:-gdb}"
GDB_SESSION="${ROOT}/.cursor/gdb-session-af2b5b.gdb"

make gdbscripts

cleanup() {
	bash scripts/debug-qemu.sh stop
}
trap cleanup EXIT INT TERM

cat >"$GDB_SESSION" <<EOF
target remote 127.0.0.1:${GDB_PORT}
source build/gdb/aitos-debug.gdb
EOF

if [ -e /dev/tty ]; then
	exec "$GDB" -q -x build/gdb/aitos.gdb -x "$GDB_SESSION" build/kernel.elf </dev/tty
else
	exec "$GDB" -q -x build/gdb/aitos.gdb -x "$GDB_SESSION" build/kernel.elf
fi
