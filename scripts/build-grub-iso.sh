#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

KERNEL_ELF="build/kernel.elf"
ISO_DIR="build/grub-iso"
ISO_OUT="bin/aitos.iso"

if [ ! -f "$KERNEL_ELF" ]; then
	echo "error: $KERNEL_ELF not found; run make build first" >&2
	exit 1
fi

# Locate grub-mkrescue. Linux ships it as `grub-mkrescue` (grub-pc-bin); macOS
# uses a cross build (Homebrew i686-elf-grub / x86_64-elf-grub), whose binary is
# prefixed and whose i386-pc modules live under lib/<target>/grub.
GRUB_MKRESCUE=""
for cand in grub-mkrescue grub2-mkrescue i686-elf-grub-mkrescue x86_64-elf-grub-mkrescue; do
	if command -v "$cand" >/dev/null 2>&1; then
		GRUB_MKRESCUE="$(command -v "$cand")"
		break
	fi
done

if [ -z "$GRUB_MKRESCUE" ]; then
	echo "error: grub-mkrescue not found." >&2
	echo "  Linux: install grub-pc-bin / grub-common (+ xorriso)" >&2
	echo "  macOS: brew install xorriso i686-elf-grub" >&2
	exit 1
fi

# Find the directory that contains the i386-pc platform modules so grub-mkrescue
# can build a BIOS-bootable image even with a cross GRUB in a non-default prefix.
GRUB_DIR_ARG=()
if ! "$GRUB_MKRESCUE" --help 2>/dev/null | grep -q -- '--directory'; then
	: # older grub-mkrescue without -d; rely on default module path
elif [ ! -d "$(dirname "$GRUB_MKRESCUE")/../lib/grub/i386-pc" ]; then
	pc_dir="$(find "$(dirname "$GRUB_MKRESCUE")/.." -type d -name i386-pc 2>/dev/null | head -n 1)"
	if [ -n "$pc_dir" ]; then
		GRUB_DIR_ARG=(-d "$pc_dir")
	fi
fi

if ! command -v xorriso >/dev/null 2>&1; then
	echo "error: xorriso not found (required by grub-mkrescue)." >&2
	echo "  macOS: brew install xorriso" >&2
	exit 1
fi

mkdir -p "$ISO_DIR/boot/grub"
cp "$KERNEL_ELF" "$ISO_DIR/boot/kernel.elf"
cp grub/grub.cfg "$ISO_DIR/boot/grub/grub.cfg"
mkdir -p bin
"$GRUB_MKRESCUE" "${GRUB_DIR_ARG[@]}" -o "$ISO_OUT" "$ISO_DIR"
echo "grub-iso: $ISO_OUT (via $(basename "$GRUB_MKRESCUE"))"
