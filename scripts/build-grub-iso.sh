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

if ! command -v grub-mkrescue >/dev/null 2>&1; then
	echo "error: grub-mkrescue not found; install grub-pc-bin / grub-common" >&2
	exit 1
fi

mkdir -p "$ISO_DIR/boot/grub"
cp "$KERNEL_ELF" "$ISO_DIR/boot/kernel.elf"
cp grub/grub.cfg "$ISO_DIR/boot/grub/grub.cfg"
mkdir -p bin
grub-mkrescue -o "$ISO_OUT" "$ISO_DIR"
echo "grub-iso: $ISO_OUT"
