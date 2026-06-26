# AiTOS x86-64 基础架构迁移 — 实现方案

> 修订日期：2026-06-26  
> 对应设计：[设计方案](../design/x86_64-migration.md)

## 1. 实施进度总览

| 阶段 | 内容 | 状态 |
|------|------|------|
| 1 | 构建系统、`linker.ld`、基础头文件 | ✅ 已完成 |
| 2 | MBR/loader 引导链、`boot_info`、Logo | 🟡 进行中（PM 已通，Long Mode 未通） |
| 3 | 64 位入口、printk、IDT/键盘、控制台 | 🟡 代码已写，运行时未验证 |
| 4 | mm/sched/syscall/fs 骨架 | ✅ 桩代码已链接 |
| 5 | Shell 五命令 + QEMU 调试 | ⏳ 待引导链打通 |

**当前 debugcon 输出**（`-debugcon stdio`）：`B P E S C`

| 标记 | 含义 |
|------|------|
| B | `loader_start` 实模式入口 |
| P | 进入 32 位保护模式（`pm_start`） |
| E | 保护模式早期代码继续执行 |
| S | 段寄存器与栈（`esp=0x90000`）设置完成 |
| C | 即将进入 `setup_pae_paging` |

**阻塞点**：`jmp setup_pae_paging` 之后尚未到达 `G`/`L`/`6` 及 `kmain`。

## 2. 目录与文件映射

### 2.1 新增/迁移的核心路径

```text
arch/x86_64/
  boot/
    mbr.asm          # MBR：Logo、双次 PIO 加载、jmp LOADER_ENTRY
    loader.asm       # E820、GDT、PM、PAE、Long Mode
    boot.inc         # 物理地址常量、LOADER_ENTRY
    gdt.inc          # GDT 描述符宏、段选择子
    boot_logo.inc    # VGA 文本 Logo
  kernel.asm         # 中断入口、键盘中断
  print.asm          # 底层打印
  linker.ld          # 高半区链接脚本

include/aitos/       # types, compiler, init, printk, errno, boot_info, console, shell, mm, sched, ...

mm/bootstrap.c       # mm 桩
sched/core.c         # sched 桩
proc/syscall-stub.c  # syscall 桩
fs/fs-stub.c         # fs 桩
shell/cmd.c          # 五内置命令
shell/repl.c         # REPL 主循环
kernel/main.c        # kmain → aitos_init → shell_run
kernel/boot_banner.c # 内核 banner
```

### 2.2 保留但不参与 MVP 链接

```text
arch/x86/            # 旧 ia32 源码（参考）
mm/memory.c
sched/thread.c
proc/{fork,exec,process}.c
fs/{dir,file,inode,fs}.c
drivers/{ide,timer}.c
```

## 3. 构建与运行

### 3.1 工具链

```bash
# Debian/Ubuntu
sudo apt install build-essential nasm gcc gdb qemu-system-x86
```

### 3.2 构建目标

```bash
make build    # mbr.bin, loader.bin, kernel.elf, kernel.bin
make image    # bin/hd60M.img, bin/hd80M.img
make run-qemu # qemu-system-x86_64, -debugcon stdio
```

**Makefile 要点**：

| 项 | 值 |
|----|-----|
| 编译 | `gcc -m64 -mcmodel=kernel -mno-red-zone` |
| 链接 | `ld -m elf_x86_64 -T arch/x86_64/linker.ld` |
| QEMU | `qemu-system-x86_64 -cpu qemu64` |
| 磁盘 | `hd60M.img`：MBR + loader@扇区2 + kernel.bin@扇区6 |
| DEBUG | `DEBUG=1` 时 loader 含 `loader_debug_gate` 供 GDB 停住 |

### 3.3 调试

```bash
make debug          # 编译 DEBUG=1 → 启动 QEMU GDB stub → 进入 GDB
make debug-qemu     # 仅启动 QEMU（端口 1234）
make gdb            # 连接已运行的 QEMU
make stop           # 停止 QEMU 与关联 GDB
```

GDB 脚本：`build/gdb/aitos.gdb`、`build/gdb/aitos-debug.gdb`（由 `scripts/gdb/*.gdb.in` 生成）。

## 4. 引导链实现细节

### 4.1 关键常量（`arch/x86_64/boot/boot.inc`）

```text
LOADER_BASE_ADDR     = 0x900
LOADER_ENTRY         = 0xa26    # loader_start 物理地址（紧凑布局）
KERNEL_BIN_BASE_ADDR = 0x70000
KERNEL_START_SECTOR  = 6
KERNEL_SECTOR_COUNT  = 40
PAGE_TABLE_BASE      = 0x90000
BOOT_INFO_ADDR       = 0xb00
STACK_TOP            = 0xffffffff80009f000
```

### 4.2 MBR 实现要点

- **单 section** `[org 0x7c00]`，避免多 section 导致 MBR 超过 512 字节
- `boot_logo.inc` 放在 `times 510-($-$$)` **之前**
- `rd_disk_m_16`：设置扇区数 → LBA → `READ SECTORS (0x20)` → 等待 DRQ → 循环读 256 字
- 两次加载：loader → `0x900`；kernel → `ds:0`（`ds=0x7000`）即物理 `0x70000`
- 两次加载之间 `wait_bsy` 轮询 `0x1f7`

### 4.3 Loader 布局优化

**问题**：原 `times 0x300` 填充使 `loader_start` 位于 `0xc00`，超出单扇区 PIO 可靠加载范围。

**解决**：移除 0x300 填充，`loader_start` 紧随 GDT/`ards_buf` 之后（约 `0xa26`），MBR 使用 `LOADER_ENTRY` 跳转。

### 4.4 保护模式入口

**问题**：`[bits 16]` 下 `mov eax, cr0` 在 QEMU 中不能正确读 CR0。

**解决**：

```nasm
mov eax, 0x11          ; ET + PE
mov cr0, eax
jmp dword SELECTOR_CODE32:pm_start
```

必须使用 **`jmp dword`**（32 位偏移远跳转），不能用 16 位远跳转。

### 4.5 段寄存器设置

设置 `ds/es/fs/gs/ss` 时，**不可在 `mov ax, SELECTOR_DATA` 之后用 `mov al, imm` 破坏 ax**，否则后续 `mov es, ax` 等会使用错误选择子。

### 4.6 内核镜像

当前镜像写入 **`kernel.bin`**（`objcopy -O binary`），引导阶段硬编码入口：

```nasm
mov rax, 0xffffffff80000b54
jmp rax
```

（与 `readelf -h kernel.elf` 中 `Entry point` 一致。）

后续可恢复 `kernel_init_elf64` 解析 ELF Program Header，在完整多扇区加载稳定后启用。

### 4.7 分页与 Long Mode（设计实现，待调通）

`setup_pae_paging`（32 位）：

1. PML4[0] → PDPT@+0x1000 → PD@+0x2000：identity 2MB
2. PML4[511] → PDPT@+0x3000 → PD@+0x4000：映射 `KERNEL_BIN_BASE_ADDR` → `0xffffffff80000000`

`enter_long_mode`：

1. `CR4.PAE`
2. `WRMSR(EFER, LME)`
3. `CR3 = PAGE_TABLE_BASE`
4. `CR0.PG`
5. `jmp dword SELECTOR_CODE64:long_mode_entry`

## 5. 内核初始化链

```c
// kernel/main.c
void kmain(void)
{
    boot_show_banner();
    aitos_init();      // console, irq, keyboard, mm, sched, syscall, fs stubs
    intr_enable();
    shell_run();       // REPL
}
```

## 6. Shell 实现

- `shell/cmd.c`：`help`、`clear`、`version`、`echo`、`uname`
- `shell/repl.c`：提示符 `[aitos@localhost]$`，读键盘、解析、调用 `shell_dispatch()`
- Shell **直接调用内核函数**（如 `console_clear()`），不经 syscall

## 7. 已发现的问题与修复记录

| 问题 | 根因 | 修复/对策 |
|------|------|----------|
| BIOS 不引导 | `boot_logo.inc` 在 `section MBR` 前，MBR 604B，签名不在 510 | 单 section + 正确 padding |
| 仅输出 B | `loader_start` 在 0xc00，PIO 只可靠加载首扇区 | 紧凑布局 + `LOADER_ENTRY` |
| 无法进入 PM | `mov eax,cr0` 在实模式无效 | `mov eax,0x11` |
| 远跳转失败 | 16 位 `jmp` 到 32 位代码段 | `jmp dword` |
| 段设置后崩溃 | `mov al` 破坏选择子 | 先完成全部 `mov r, ax` 再调试输出 |
| kernel 加载挂起 | 40 扇区单次 PIO 读不完整 | 改用 `kernel.bin`；多扇区 DRQ 待完善 |
| `LOADER_ENTRY` 漂移 | 增删 loader 数据区改变偏移 | `boot.inc` 显式定义；Makefile 依赖 `boot.inc` |

## 8. 待办（实现侧）

1. **打通 Long Mode**：内联或修复 `setup_pae_paging`/`enter_long_mode` 跳转；确认 GDT 64 位代码段（`L=1`）
2. **完善 MBR 多扇区 PIO**：每扇区独立等待 DRQ，确保 `kernel.bin` 完整载入 `0x70000`
3. **移除 debugcon 调试字母**（`B/P/E/...`），恢复干净启动输出
4. **端到端验证**：`kmain` banner → `aitos_init` 日志 → Shell 五命令
5. **更新根目录 `README.md`**：反映 x86-64、`qemu-system-x86_64`、当前调试流程

## 9. 参考命令

```bash
# 检查 MBR 签名
xxd -s 510 -l 2 build/mbr.bin    # 应显示 55 aa

# 检查 loader 入口偏移
python3 -c "d=open('build/loader.bin','rb').read(); print(hex(d.find(bytes([0x31,0xc0,0x8e,0xd8]))))"

# 检查内核入口
readelf -h build/kernel.elf | grep Entry

# 快速启动测试
make image && make run-qemu
```
