#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

KERNEL_ELF="build/kernel.elf"
DISK_OUT="bin/aitos-grub.img"
DISK_SIZE_MB=64
PART_SIZE_MB=32
MNT=""

cleanup() {
	if [ -n "$MNT" ] && mountpoint -q "$MNT" 2>/dev/null; then
		umount "$MNT" || true
	fi
	[ -n "$MNT" ] && rmdir "$MNT" 2>/dev/null || true
}
trap cleanup EXIT

if [ "$(id -u)" -ne 0 ]; then
	echo "error: build-grub-disk.sh requires root (loop mount + grub-install)" >&2
	exit 1
fi

if [ ! -f "$KERNEL_ELF" ]; then
	echo "error: $KERNEL_ELF not found; run make build first" >&2
	exit 1
fi

for cmd in parted mkfs.vfat grub-install; do
	if ! command -v "$cmd" >/dev/null 2>&1; then
		echo "error: $cmd not found" >&2
		exit 1
	fi
done

mkdir -p bin
dd if=/dev/zero of="$DISK_OUT" bs=1M count="$DISK_SIZE_MB" status=none
parted -s "$DISK_OUT" mklabel msdos
parted -s "$DISK_OUT" mkpart primary fat32 1MiB "${PART_SIZE_MB}MiB"
parted -s "$DISK_OUT" set 1 boot on

LOOP=$(losetup --find --show --partscan "$DISK_OUT")
mkfs.vfat -F 32 "${LOOP}p1"

MNT=$(mktemp -d)
mount "${LOOP}p1" "$MNT"
mkdir -p "$MNT/boot/grub"
cp "$KERNEL_ELF" "$MNT/boot/kernel.elf"
cp grub/grub.cfg "$MNT/boot/grub/grub.cfg"
grub-install --target=i386-pc \
	--boot-directory="$MNT/boot" \
	--modules="multiboot2" \
	--force "$LOOP"

sync
umount "$MNT"
rmdir "$MNT"
MNT=""
losetup -d "$LOOP"

echo "grub-disk: $DISK_OUT"
