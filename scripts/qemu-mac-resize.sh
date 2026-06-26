#!/usr/bin/env bash
# Enlarge QEMU cocoa window on macOS.
qemu_mac_resize_window() {
	local width="${1:-1280}"
	local height="${2:-960}"
	local pid="${3:-}"

	[ "$(uname -s)" = "Darwin" ] || return 0

	osascript - "$width" "$height" "$pid" <<'APPLESCRIPT' >/dev/null 2>&1 || true
on run argv
	set winW to (item 1 of argv) as integer
	set winH to (item 2 of argv) as integer
	set qpid to item 3 of argv
	set winX to 80
	set winY to 60
	repeat with attempt from 1 to 10
		delay 1
		tell application "System Events"
			set targets to {}
			if qpid is not "" then
				try
					set targets to every process whose unix id is (qpid as integer)
				end try
			end if
			if (count of targets) is 0 then
				set targets to every process whose name contains "qemu"
			end if
			repeat with p in targets
				if (count of windows of p) > 0 then
					set frontmost of p to true
					tell window 1 of p
						set position to {winX, winY}
						set size to {winW, winH}
					end tell
					return "resized:" & (name of p) & ":attempt" & attempt
				end if
			end repeat
		end tell
	end repeat
	return "no_window"
end run
APPLESCRIPT
}
