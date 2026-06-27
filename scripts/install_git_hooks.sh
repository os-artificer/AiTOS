#!/usr/bin/env bash
# Install repo-local git hooks (prepare-commit-msg strips Cursor co-author trailers).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/scripts/git-hooks/prepare-commit-msg"
DST="$ROOT/.git/hooks/prepare-commit-msg"

if [[ ! -d "$ROOT/.git/hooks" ]]; then
	echo "error: .git/hooks not found (not a git repository?)" >&2
	exit 1
fi

install -m 755 "$SRC" "$DST"
echo "Installed $DST"
