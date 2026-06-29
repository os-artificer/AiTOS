#!/usr/bin/env bash
# Run GRUB boot, send help+version via PTY, extract DBG markers to NDJSON log.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

DEBUG_LOG="${DEBUG_LOG:-.cursor/debug-536064.log}"
TRACE="bin/boot-debugcon.log"
QEMU="${QEMU:-qemu-system-x86_64}"
PTY="$(mktemp -u /tmp/aitos-pty.XXXXXX)"

cleanup() {
	rm -f "$PTY"
	pkill -9 qemu-system-x86_64 2>/dev/null || true
}
trap cleanup EXIT INT TERM

make build grub-iso >/dev/null
pkill -9 qemu-system-x86_64 2>/dev/null || true
sleep 0.2
: >"$TRACE"
: >"$DEBUG_LOG"

socat -d -d "pty,link=$PTY,raw,echo=0" "pty,raw,echo=0" &
SOCAT_PID=$!
sleep 0.3

"$QEMU" \
	-drive file=bin/aitos.iso,format=raw,if=ide,index=0,media=disk \
	-boot order=c,menu=off \
	-m 128 -cpu qemu64 -no-reboot \
	-debugcon "file:${TRACE}" \
	-serial "$PTY" \
	-display none -nic none \
	-device isa-debug-exit,iobase=0xf4,iosize=0x04 &
QPID=$!

# Wait for shell prompt in debugcon, then send commands via PTY.
for _ in $(seq 1 80); do
	if grep -aq 'aitos@localhost' "$TRACE" 2>/dev/null; then
		break
	fi
	sleep 0.25
done

sleep 0.5
printf 'help\n' >"$PTY"
sleep 2
printf 'version\n' >"$PTY"
sleep 2
printf 'echo hello\n' >"$PTY"
sleep 2

kill "$QPID" 2>/dev/null || true
wait "$QPID" 2>/dev/null || true
kill "$SOCAT_PID" 2>/dev/null || true

python3 - "$TRACE" "$DEBUG_LOG" <<'PY'
import re, sys, json, time
trace_path, log_path = sys.argv[1], sys.argv[2]
text = open(trace_path, 'rb').read().decode('utf-8', 'replace')
pat = re.compile(r'DBG\|536064\|([^|]+)\|([^|]+)\|([^|]+)\|([0-9a-f]+)')
with open(log_path, 'w') as out:
    for m in pat.finditer(text):
        out.write(json.dumps({
            "sessionId": "536064",
            "hypothesisId": m.group(1),
            "location": m.group(2),
            "message": m.group(3),
            "data": {"value": int(m.group(4), 16)},
            "timestamp": int(time.time() * 1000),
            "runId": "grub-shell-test"
        }) + "\n")
print("debugcon markers:", len(pat.findall(text)))
print("has Available:", "Available commands" in text)
print("has version:", "0.1.0-mvp" in text)
print("has echo:", "hello" in text)
PY

echo "--- debugcon tail ---"
tr -d '\r' <"$TRACE" | tail -25
