#!/usr/bin/env bash
# Load shell-safe variables from env.rc (ignore Makefile-only "VAR :=" lines).

load_env_rc() {
	local file="${1:-env.rc}"

	[ -f "$file" ] || return 0

	while IFS= read -r line || [ -n "$line" ]; do
		case "$line" in
			'' | \#*) continue ;;
			export\ *) eval "$line" ;;
			*[[:space:]]:=*) continue ;;
			*=*) export "$line" ;;
		esac
	done <"$file"
}
