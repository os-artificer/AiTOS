#!/usr/bin/env bash
# Auto-retry GRUB boot verification until shell prompt appears (or max attempts).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

MAX="${MAX:-5}"
ATTEMPT=1

echo "auto-verify-grub: up to ${MAX} attempts..."

while [ "$ATTEMPT" -le "$MAX" ]; do
	echo "--- attempt ${ATTEMPT}/${MAX} ---"
	if SKIP_BUILD="$([ "$ATTEMPT" -eq 1 ] && echo 0 || echo 1)" bash scripts/verify-grub-shell.sh; then
		echo "auto-verify-grub: OK on attempt ${ATTEMPT}"
		exit 0
	fi
	ATTEMPT=$((ATTEMPT + 1))
	pkill -9 qemu-system-x86_64 2>/dev/null || true
	sleep 1
done

echo "auto-verify-grub: FAIL after ${MAX} attempts" >&2
exit 1
