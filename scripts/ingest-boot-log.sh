#!/usr/bin/env bash
# Ingest boot debugcon trace into NDJSON debug log.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TRACE="${1:-$ROOT/bin/boot-debugcon.log}"
LOG="${2:-$ROOT/.cursor/debug-56499b.log}"
SESSION="${DEBUG_SESSION_ID:-56499b}"
RUN_ID="${RUN_ID:-post-fix}"

[ -f "$TRACE" ] || exit 0

ts=$(date +%s000)
chars=$(tr -d '\n' <"$TRACE")
len=${#chars}

python3 - "$LOG" "$SESSION" "$RUN_ID" "$ts" "$chars" "$len" <<'PY'
import json, sys
log, session, run_id, ts, chars, length = sys.argv[1:7]
stages = {
    "M": "mbr_after_logo",
    "B": "loader_start",
    "P": "pm_start",
    "C": "before_long_mode",
    "L": "long_mode_entry",
    "J": "before_jump_kernel",
    "S": "kernel_start",
    "K": "kmain",
    "S": "shell_ready",
    "R": "before_shell_run",
}
seen = []
for ch in chars:
    seen.append({"char": ch, "stage": stages.get(ch, "unknown")})
entry = {
    "sessionId": session,
    "runId": run_id,
    "hypothesisId": "boot-trace",
    "location": "scripts/ingest-boot-log.sh",
    "message": "boot debugcon trace",
    "data": {"trace": chars, "length": int(length), "stages": seen},
    "timestamp": int(ts),
}
with open(log, "a", encoding="utf-8") as f:
    f.write(json.dumps(entry, ensure_ascii=False) + "\n")
PY
