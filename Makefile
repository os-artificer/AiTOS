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

GDB_DIR		= $(BUILD_DIR)/gdb
GDB_INIT	= $(GDB_DIR)/aitos.gdb
GDB_DEBUG	= $(GDB_DIR)/aitos-debug.gdb
LOADER_DEBUG_GATE = loader_debug_gate

INC		= -I include/
CFLAGS		= -m64 -mcmodel=medium -mno-red-zone -fno-pic -fno-pie \
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

OBJS		= \
	$(BUILD_DIR)/arch/x86_64/kernel.o \
	$(BUILD_DIR)/arch/x86_64/print.o \
	$(BUILD_DIR)/kernel/main.o \
	$(BUILD_DIR)/kernel/irq.o \
	$(BUILD_DIR)/kernel/printk.o \
	$(BUILD_DIR)/kernel/boot_banner.o \
	$(BUILD_DIR)/drivers/console.o \
	$(BUILD_DIR)/drivers/vga.o \
	$(BUILD_DIR)/drivers/keyboard.o \
	$(BUILD_DIR)/mm/bootstrap.o \
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

export QEMU GDB GDB_PORT QEMU_GUI

QEMU_DRIVE	= -drive file=$(HD60_IMG),format=raw,if=ide,index=0,media=disk \
		  -drive file=$(HD80_IMG),format=raw,if=ide,index=1,media=disk

.PHONY: all build image clean clean-all \
	run-qemu run-qemu-gui boot-trace \
	debug debug-gui debug-tmux debug-tmux-gui \
	debug-qemu debug-qemu-gui gdb stop gdbscripts

default: all

all: image
	@echo "build success: $(HD60_IMG)"

build: $(BOOT_BIN) $(LOADER_BIN) $(KERNEL_ELF)

image: build $(HD60_IMG) $(HD80_IMG) $(KERNEL_BIN)

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

gdb:
	$(MAKE) DEBUG=1 image gdbscripts
	$(GDB) -q -x $(GDB_INIT) -ex "target remote 127.0.0.1:$(GDB_PORT)" \
		-x $(GDB_DEBUG) $(KERNEL_ELF)

clean:
	rm -rf $(BUILD_DIR)/*
	rm -f bin/qemu-debug.pid bin/qemu-serial.log bin/qemu-serial.pty bin/qemu-debug.stderr

clean-all: clean
	rm -rf $(BIN_DIR)
