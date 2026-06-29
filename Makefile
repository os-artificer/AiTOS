-include env.rc

AS		= nasm
CC		?= gcc
LD		?= ld
MKDIR		= mkdir -p
OBJCOPY		?= objcopy

BUILD_DIR	= build
BIN_DIR		= bin

KERNEL_ELF	= $(BUILD_DIR)/kernel.elf
KERNEL_BIN	= $(BUILD_DIR)/kernel.bin
BOOT_BIN	= $(BUILD_DIR)/mbr.bin
LOADER_BIN	= $(BUILD_DIR)/loader.bin
HD60_IMG	= $(BIN_DIR)/hd60M.img
HD80_IMG	= $(BIN_DIR)/hd80M.img
AITOS_ISO	= $(BIN_DIR)/aitos.iso
GRUB_DISK	= $(BIN_DIR)/aitos-grub.img

GDB_DIR		= $(BUILD_DIR)/gdb
GDB_INIT	= $(GDB_DIR)/aitos.gdb
GDB_DEBUG	= $(GDB_DIR)/aitos-debug.gdb
LOADER_DEBUG_GATE = loader_debug_gate

INC		= -I include/
CFLAGS		= -m64 -mcmodel=medium -mno-red-zone -mgeneral-regs-only -fno-pic -fno-pie \
		  -fcf-protection=none \
		  -Wall -Wextra -nostdlib -fno-builtin -fno-stack-protector \
		  $(INC) -c
ASFLAGS		= -f elf64
LDFLAGS		= -m elf_x86_64 -T arch/x86_64/linker.ld -nostdlib

ifeq ($(DEBUG),1)
CFLAGS		+= -g -O0 -DDEBUG
LDFLAGS		+= -g
else
CFLAGS		+= -O2
endif
CFLAGS		+= $(CFLAGS_EXTRA)

OBJS		= \
	$(BUILD_DIR)/arch/x86_64/kernel.o \
	$(BUILD_DIR)/arch/x86_64/multiboot2.o \
	$(BUILD_DIR)/arch/x86_64/print.o \
	$(BUILD_DIR)/kernel/main.o \
	$(BUILD_DIR)/kernel/irq.o \
	$(BUILD_DIR)/kernel/printk.o \
	$(BUILD_DIR)/kernel/boot_banner.o \
	$(BUILD_DIR)/drivers/console.o \
	$(BUILD_DIR)/drivers/vga.o \
	$(BUILD_DIR)/drivers/keyboard.o \
	$(BUILD_DIR)/mm/bootstrap.o \
	$(BUILD_DIR)/boot/early.o \
	$(BUILD_DIR)/boot/multiboot2.o \
	$(BUILD_DIR)/sched/core.o \
	$(BUILD_DIR)/proc/syscall-stub.o \
	$(BUILD_DIR)/fs/fs-stub.o \
	$(BUILD_DIR)/shell/cmd.o \
	$(BUILD_DIR)/shell/repl.o \
	$(BUILD_DIR)/lib/string.o

QEMU		?= qemu-system-x86_64
GDB		?= gdb
GDB_PORT	?= 1234
QEMU_GUI	?= 0

export QEMU GDB GDB_PORT QEMU_GUI GRUB_BOOT

QEMU_DRIVE	= -drive file=$(HD60_IMG),format=raw,if=ide,index=0,media=disk \
		  -drive file=$(HD80_IMG),format=raw,if=ide,index=1,media=disk

.PHONY: all build image clean clean-all gcc-version-check \
	run-qemu run-qemu-gui boot-trace \
	debug debug-gui debug-tmux debug-tmux-gui \
	debug-qemu debug-qemu-gui gdb stop gdbscripts \
	grub-iso grub-disk run-qemu-grub run-qemu-grub-disk \
	debug-qemu-grub debug-grub debug-grub-disk verify-grub-shell verify-grub-shell-cmd auto-verify-grub

default: all

all: image
	@echo "build success: $(HD60_IMG)"

gcc-version-check:
	@ver=$$($(CC) -dumpversion 2>/dev/null) || ver=; \
	major=$${ver%%.*}; \
	if [ -z "$$ver" ]; then \
	  echo "error: $(CC) not found or not executable"; \
	  echo "hint: run ./scripts/install_devenv.sh or configure env.rc"; \
	  exit 1; \
	fi; \
	if [ "$$major" -lt 13 ] 2>/dev/null; then \
	  echo "error: AiTOS requires GCC >= 13 (found $$ver via $(CC))"; \
	  exit 1; \
	fi; \
	if [ "$$(uname -s)" = Darwin ] && ! $(CC) -dumpmachine 2>/dev/null | grep -q elf; then \
	  echo "error: macOS needs x86_64-elf-gcc cross toolchain (found $(CC))"; \
	  echo "hint: run ./scripts/install_devenv.sh to generate env.rc"; \
	  exit 1; \
	fi

build: gcc-version-check $(BOOT_BIN) $(LOADER_BIN) $(KERNEL_ELF)

image: build $(HD60_IMG) $(HD80_IMG) $(KERNEL_BIN)

$(BUILD_DIR)/boot/%.o: boot/%.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -O0 $< -o $@

$(BUILD_DIR)/kernel/irq.o: kernel/irq.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -O0 $< -o $@

$(BUILD_DIR)/kernel/printk.o: kernel/printk.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -O0 $< -o $@

$(BUILD_DIR)/drivers/console.o: drivers/console.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -O0 $< -o $@

$(BUILD_DIR)/shell/cmd.o: shell/cmd.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -O0 $(CFLAGS_EXTRA) $< -o $@

$(BUILD_DIR)/shell/repl.o: shell/repl.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -O0 $(CFLAGS_EXTRA) $< -o $@

$(BUILD_DIR)/lib/string.o: lib/string.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -O2 $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.asm
	$(MKDIR) $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BOOT_BIN): arch/x86_64/boot/mbr.asm arch/x86_64/boot/boot.inc
	$(MKDIR) $(dir $@)
	$(AS) -f bin -I arch/x86_64/boot/ $< -o $@

$(LOADER_BIN): arch/x86_64/boot/loader.asm arch/x86_64/boot/boot.inc arch/x86_64/boot/gdt.inc
	$(MKDIR) $(dir $@)
	$(AS) -f bin -I arch/x86_64/boot/ $(if $(filter 1,$(DEBUG)),-DDEBUG_BUILD,) $< -o $@

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)

$(HD60_IMG): $(BOOT_BIN) $(LOADER_BIN) $(KERNEL_BIN) | $(BIN_DIR)
	dd if=/dev/zero of=$@ bs=1M count=60 status=none
	dd if=$(BOOT_BIN) of=$@ bs=512 count=1 conv=notrunc status=none
	dd if=$(LOADER_BIN) of=$@ bs=512 count=4 seek=2 conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$@ bs=512 count=400 seek=6 conv=notrunc status=none

$(HD80_IMG): | $(BIN_DIR)
	dd if=/dev/zero of=$@ bs=1M count=80 status=none

gdbscripts: $(GDB_INIT) $(GDB_DEBUG)

$(GDB_INIT): scripts/gdb/aitos.gdb.in
	$(MKDIR) $(GDB_DIR)
	sed 's|@KERNEL_ENTRY@|kmain|g' $< > $@

$(GDB_DEBUG): scripts/gdb/aitos-debug.gdb.in Makefile
	$(MKDIR) $(GDB_DIR)
	sed -e 's|@LOADER_DEBUG_GATE@|$(LOADER_DEBUG_GATE)|g' \
	    -e 's|@KERNEL_ENTRY@|kmain|g' $< > $@

run-qemu: image
	@QEMU_GUI=$(QEMU_GUI) bash scripts/run-qemu.sh

run-qemu-gui: image
	@QEMU_GUI=1 bash scripts/run-qemu.sh

boot-trace: image
	@chmod +x scripts/capture-boot-trace.sh scripts/ingest-boot-log.sh
	@QEMU_GUI=$(QEMU_GUI) bash scripts/capture-boot-trace.sh

debug-qemu:
	@QEMU_GUI=$(QEMU_GUI) bash scripts/debug-qemu.sh start

debug-qemu-gui:
	@QEMU_GUI=1 bash scripts/debug-qemu.sh start

debug:
	@QEMU_GUI=$(QEMU_GUI) bash scripts/debug.sh

debug-gui:
	@QEMU_GUI=1 bash scripts/debug.sh

debug-tmux:
	@QEMU_GUI=$(QEMU_GUI) bash scripts/debug-tmux.sh

debug-tmux-gui:
	@QEMU_GUI=1 bash scripts/debug-tmux.sh

stop:
	@bash scripts/debug-qemu.sh stop
	@bash scripts/debug-qemu-grub.sh stop 2>/dev/null || true

gdb:
	$(MAKE) DEBUG=1 image gdbscripts
	$(GDB) -q -x $(GDB_INIT) -ex "target remote 127.0.0.1:$(GDB_PORT)" \
		$(KERNEL_ELF)

grub-iso: $(KERNEL_ELF) $(HD80_IMG)
	@bash scripts/build-grub-iso.sh

grub-disk: $(KERNEL_ELF)
	@bash scripts/build-grub-disk.sh

run-qemu-grub: grub-iso
	@GRUB_BOOT=iso bash scripts/run-qemu-grub.sh

run-qemu-grub-disk: grub-disk
	@GRUB_BOOT=disk bash scripts/run-qemu-grub.sh

verify-grub-shell:
	@chmod +x scripts/verify-grub-shell.sh
	@bash scripts/verify-grub-shell.sh

verify-grub-shell-cmd:
	@chmod +x scripts/verify-grub-shell.sh
	@RUN_CMD=1 bash scripts/verify-grub-shell.sh

auto-verify-grub:
	@chmod +x scripts/auto-verify-grub.sh scripts/verify-grub-shell.sh
	@bash scripts/auto-verify-grub.sh

debug-qemu-grub:
	@GRUB_BOOT=$(if $(filter disk,$(GRUB_BOOT)),disk,iso) bash scripts/debug-qemu-grub.sh start

debug-grub:
	@GRUB_BOOT=$(if $(filter disk,$(GRUB_BOOT)),disk,iso) bash scripts/debug-grub.sh

debug-grub-disk:
	@GRUB_BOOT=disk bash scripts/debug-grub.sh

clean:
	rm -rf $(BUILD_DIR)/*
	rm -f bin/qemu-debug.pid bin/qemu-serial.log bin/qemu-serial.pty bin/qemu-debug.stderr
	rm -f $(AITOS_ISO) $(GRUB_DISK)

clean-all: clean
	rm -rf $(BIN_DIR)
