#!/usr/bin/env bash
# Shared QEMU UI helpers. Set QEMU_GUI=1 for windowed VGA + PS/2 keyboard.
# QEMU_GUI=0 (default): headless serial console (-nographic / pty).

qemu_gui_enabled() {
	[ "${QEMU_GUI:-0}" = "1" ]
}

# Common QEMU args for run/debug (GUI: VGA + PS/2; headless: serial on terminal).
qemu_run_extra_args() {
	printf '%s\n' -boot order=c,menu=off
	printf '%s\n' -nic none
	if qemu_gui_enabled; then
		# Single VGA window; Shell I/O is on the VGA screen via PS/2 keyboard.
		# No serial mirror (-serial none) so only ONE window appears.
		if [ "$(uname -s)" = "Darwin" ]; then
			if [ "${QEMU_FULLSCREEN:-0}" = "1" ]; then
				printf '%s\n' -display 'cocoa,show-cursor=on,zoom-to-fit=on,full-screen=on'
			else
				printf '%s\n' -display 'cocoa,show-cursor=on,zoom-to-fit=on'
			fi
		fi
		printf '%s\n' -vga std
		printf '%s\n' -serial none
	else
		printf '%s\n' -nographic
	fi
}

# Same as above plus debugcon file (for make boot-trace diagnostics).
qemu_boot_trace_args() {
	local trace="${1:-bin/boot-debugcon.log}"
	printf '%s\n' -debugcon "file:${trace}"
	qemu_run_extra_args
}

qemu_gui_display_args() {
	printf '%s\n' -boot order=c,menu=off
	printf '%s\n' -nic none
	if qemu_gui_enabled; then
		if [ "$(uname -s)" = "Darwin" ]; then
			if [ "${QEMU_FULLSCREEN:-0}" = "1" ]; then
				printf '%s\n' -display 'cocoa,show-cursor=on,zoom-to-fit=on,full-screen=on'
			else
				printf '%s\n' -display 'cocoa,show-cursor=on,zoom-to-fit=on'
			fi
		fi
		printf '%s\n' -vga std
		printf '%s\n' -serial none
	else
		printf '%s\n' -display none
	fi
}

qemu_ui_hint_run() {
	if qemu_gui_enabled; then
		cat <<'EOF'
QEMU GUI: Shell runs inside the QEMU window (VGA text). Click the window and type.
EOF
	else
		cat <<'EOF'
Headless mode: Shell I/O is on this terminal (-nographic serial console).
EOF
	fi
}

qemu_ui_hint_debug() {
	if qemu_gui_enabled; then
		cat <<'EOF'
QEMU GUI mode: after (gdb) c, Shell REPL is in the QEMU VGA window.
Click the QEMU window and type with PS/2 keyboard.
EOF
	else
		local pty_path="${1:-}"
		cat <<'EOF'
Headless debug: boot trace in bin/qemu-serial.log (debugcon).
EOF
		if [ -n "$pty_path" ] && [ -e "$pty_path" ]; then
			echo "Serial Shell console: screen ${pty_path}"
			echo "  (or: cu -l ${pty_path} -s 115200)"
		fi
		echo "Tip: make debug-tmux splits serial log + GDB."
	fi
}

qemu_capture_pty_path() {
	local stderr_file="$1"
	local out_file="$2"
	local pty=""

	pty=$(sed -n 's/.*char device redirected to \([^ (]*\).*/\1/p' "$stderr_file" | head -1)
	if [ -n "$pty" ]; then
		printf '%s' "$pty" >"$out_file"
	fi
}
