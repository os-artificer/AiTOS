<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/image/logo-dark.svg">
    <img src="docs/image/logo.svg" alt="AiTOS Logo" width="280">
  </picture>
</p>

# AiTOS

AiTOS 是一个 基于x86-64 架构`实验性质`的操作系统，毕竟学习内核技术实践还是很重要的。

本项目使用 NASM + GCC 构建，通过硬盘镜像在 QEMU 中启动与调试。

## 开发环境（一键配置）

macOS 与 Linux 均可使用项目自带脚本安装依赖、生成 `env.rc` 并验证构建：

```bash
./scripts/install_devenv.sh
```

脚本会完成：

1. 安装构建与调试工具（见下方「依赖」）
2. 根据平台生成 `env.rc`（macOS 自动配置 `x86_64-elf-*` 交叉工具链）
3. 执行 `make` 验证环境

**可选参数：**


| 参数               | 说明                          |
| ---------------- | --------------------------- |
| `--skip-install` | 跳过包安装，仅生成 `env.rc` 并验证构建    |
| `--no-verify`    | 安装依赖并生成 `env.rc`，不执行 `make` |
| `--force-env`    | 覆盖已有 `env.rc`               |


也可手动配置：

```bash
cp env.rc.example env.rc
# 按需修改工具链或 QEMU / GDB 路径
```



## 依赖


| 工具                     | 用途                                           |
| ---------------------- | -------------------------------------------- |
| `nasm`                 | 汇编引导程序                                       |
| `gcc`, `ld`, `objcopy` | 编译与链接内核（Linux 使用系统原生工具链）                     |
| `x86_64-elf-gcc` 等     | **仅 macOS 需要**：Apple Clang 无法生成裸机 ELF，需交叉工具链 |
| `qemu-system-x86_64`   | 运行与 GDB 远程调试                                 |
| `gdb`                  | 源码级调试                                        |
| `tmux`                 | 可选，`make debug-tmux` 分屏调试                    |
| `screen`               | 可选，无头调试时连接串口 pty（`make debug-tmux` 自动使用）     |


**macOS**（需先安装 [Homebrew](https://brew.sh)）：

```bash
brew install x86_64-elf-gcc x86_64-elf-binutils nasm qemu gdb tmux
```

**Debian/Ubuntu**：

```bash
sudo apt install build-essential nasm gcc gdb qemu-system-x86 tmux
```

其他 Linux 发行版（Fedora `dnf`、Arch `pacman`、Alpine `apk` 等）由 `install_devenv.sh` 自动识别并安装对应包。

## 环境配置

机器相关路径通过 `env.rc` 配置，项目内磁盘镜像等一律使用相对路径。`env.rc` 会被 Makefile 包含（工具链 `CC :=` 等）；调试脚本通过 `scripts/load-env.sh` 安全加载其中的 `export` 行。

**macOS** 示例（由 `install_devenv.sh` 自动生成）：

```makefile
CC := x86_64-elf-gcc
LD := x86_64-elf-ld
OBJCOPY := x86_64-elf-objcopy
```

**Linux** 通常无需覆盖工具链，使用 Makefile 默认的 `gcc` / `ld` 即可。


| 变量                      | 默认值                          | 说明                                                |
| ----------------------- | ---------------------------- | ------------------------------------------------- |
| `CC` / `LD` / `OBJCOPY` | `gcc` / `ld` / `objcopy`     | Makefile 工具链（macOS 在 `env.rc` 中改为 `x86_64-elf-*`） |
| `QEMU` / `GDB`          | `qemu-system-x86_64` / `gdb` | 在 `env.rc` 中用 `export` 覆盖（供调试脚本使用）                |
| `GDB_PORT`              | `1234`                       | QEMU GDB stub 端口                                  |
| `QEMU_GUI`              | `0`                          | `1` 启用 QEMU 图形窗口（VGA + PS/2 键盘）；`0` 为无头串口模式       |
| `DEBUG`                 | `0`                          | `make DEBUG=1` 启用调试构建（`-g -O0`）                   |


macOS 上若 `gdb` 报权限错误，需按 [GDB Darwin 权限说明](https://sourceware.org/gdb/wiki/PermissionsDarwin) 做一次 codesign。

## 快速开始

```bash
./scripts/install_devenv.sh   # 首次使用：安装依赖并生成 env.rc

make                          # 编译，生成 bin/hd60M.img、bin/hd80M.img
```

**本地有图形界面（推荐）：**

```bash
make run-qemu-gui             # QEMU 窗口（VGA 文本），在窗口内 PS/2 键盘交互
make debug-gui                # GDB 调试，Shell 在 QEMU 窗口输入
```

GUI 模式：Shell **在启动 QEMU 的终端里交互**（串口镜像，可靠）；QEMU 窗口为可选 VGA 预览。macOS 自动 1280x960 + zoom-to-fit；仍太小可试 `QEMU_FULLSCREEN=1 make run-qemu-gui`。

**SSH / 无图形环境：**

```bash
make run-qemu                 # 当前终端串口交互（-nographic）
make debug-tmux               # tmux 分屏：串口 pty + GDB + 启动日志
```



## 开发与调试

项目支持 **GUI** 与 **无 GUI** 两种 QEMU 模式，均可进入 Shell 交互界面：


| 模式                     | QEMU 显示      | Shell 输入                       | 适用场景             |
| ---------------------- | ------------ | ------------------------------ | ---------------- |
| 无 GUI（`QEMU_GUI=0`，默认） | 无窗口，串口控制台    | 当前终端（`run-qemu`）或 pty（`debug`） | SSH、CI、无 DISPLAY |
| GUI（`QEMU_GUI=1`）      | QEMU 窗口（VGA） | QEMU 窗口内 PS/2 键盘               | 本地桌面开发           |


也可通过变量切换：`make run-qemu GUI=1`、`make debug GUI=1`（等价于 `*-gui` 目标）。

本地 macOS/Linux 与 SSH 远程 Linux 开发机均可使用相同命令：

```bash
cd AiTOS
./scripts/install_devenv.sh   # 首次配置

make                  # 编译

# --- 运行（无调试）---
make run-qemu         # 无头运行
make run-qemu-gui     # GUI 运行

# --- 调试 ---
make debug            # 无头：GDB 在本终端，串口见 bin/qemu-serial.pty
make debug-gui        # GUI：GDB 在本终端，Shell 在 QEMU 窗口
make debug-tmux       # 无头 tmux 三分屏（串口 + GDB + debugcon 日志）
make debug-tmux-gui   # GUI tmux 双分屏（日志 + GDB）
make stop             # 停止 QEMU 调试实例
```



### 命令速查


| 命令                                  | 模式    | 说明                                           |
| ----------------------------------- | ----- | -------------------------------------------- |
| `make run-qemu`                     | 无 GUI | 前台运行（`-nographic`，串口在当前终端）                   |
| `make run-qemu-gui`                 | GUI   | 前台运行（QEMU 窗口 + VGA 输出）                       |
| `make debug`                        | 无 GUI | DEBUG=1 编译 → QEMU 调试桩 → GDB（输入 `c` 停在 kmain） |
| `make debug-gui`                    | GUI   | 同上，Shell 键盘输入在 QEMU 窗口                       |
| `make debug-tmux`                   | 无 GUI | tmux：串口 pty + GDB + `bin/qemu-serial.log`    |
| `make debug-tmux-gui`               | GUI   | tmux：debugcon 日志 + GDB（键盘在 QEMU 窗口）          |
| `make debug-qemu`                   | 无 GUI | 仅启动 QEMU 调试桩                                 |
| `make debug-qemu-gui`               | GUI   | 仅启动带窗口的 QEMU 调试桩                             |
| `make gdb`                          | —     | 连接已运行的 QEMU（需先 `make debug-qemu`）            |
| `make stop`                         | —     | 停止 QEMU 及关联 GDB（请在**另一终端**执行）                |
| `tail -f bin/qemu-serial.log`       | —     | 查看 debugcon 启动日志                             |
| `screen $(cat bin/qemu-serial.pty)` | 无 GUI | 连接调试串口（`make debug` 后）                       |


退出 GDB 后，`make debug` 会自动清理 QEMU 进程。若卡在 GDB 提示符，可在另一终端执行 `make stop`，或在 GDB 中输入 `quit`。

### 进入 Shell 命令行

AiTOS **没有 SSH 登录**。`127.0.0.1:1234` 是 **GDB 调试端口**，不能用 `ssh` 连接。


| 目标           | 无 GUI                                                   | GUI                             |
| ------------ | ------------------------------------------------------- | ------------------------------- |
| 运行并进入 Shell  | `make run-qemu`，在当前终端输入                                 | `make run-qemu-gui`，在 QEMU 窗口输入 |
| GDB 调试内核     | `make debug` → GDB 输入 `c`                               | `make debug-gui` → GDB 输入 `c`   |
| 调试时 Shell 输入 | `screen $(cat bin/qemu-serial.pty)` 或 `make debug-tmux` | 点击 QEMU 窗口后输入                   |
| 查看启动日志       | `tail -f bin/qemu-serial.log`                           | 同左，或观察 QEMU 窗口                  |


`kmain` 完成初始化后会调用 `shell_run()` 进入 REPL（提示符 `[aitos@localhost]$`）。使用 `make debug` / `make debug-gui` 时，须先在 GDB 输入 `c`，内核才会继续运行。

### 双终端调试

```bash
# 终端 1（无 GUI 或 GUI 均可）
make debug-qemu        # 或 make debug-qemu-gui

# 终端 2
make gdb
```



### 从本机 GDB 连远程开发机（可选）

```bash
ssh -L 1234:127.0.0.1:1234 user@devhost
# 在远程执行 make debug-qemu 后，本机 gdb 连接 localhost:1234
```



## 调试



### 方式一：命令行 GDB

**本地桌面（有图形界面）：**

```bash
make debug-gui
# QEMU 窗口弹出后，在 (gdb) 输入 c；Shell 在 QEMU 窗口操作
```

**SSH / 无图形环境：**

```bash
make debug
# (gdb) 输入 c 后，另开终端：screen $(cat bin/qemu-serial.pty)
# 或使用 make debug-tmux 自动分屏
```

GDB 常用命令：

```text
(gdb) break kmain
(gdb) continue
(gdb) next
(gdb) print variable_name
(gdb) bt
```



### 方式二：Cursor / VS Code

1. 安装 C/C++ 扩展（`ms-vscode.cpptools`）
2. 打开项目（远程开发可用 Remote SSH）
3. 选择 **Debug AiTOS (QEMU + GDB)**，按 F5

若 QEMU 已在运行，可选用 **Attach GDB (QEMU already running)**。内核为 x86_64，若断点异常可在 GDB 中执行 `set architecture i386:x86-64`。

## 构建选项


| 命令                    | 说明                            |
| --------------------- | ----------------------------- |
| `make` / `make image` | 编译并写入磁盘镜像                     |
| `make DEBUG=1`        | 带调试符号编译                       |
| `make gdbscripts`     | 生成 `build/gdb/*.gdb`（调试前自动执行） |
| `make clean`          | 清理 `build/`                   |
| `make clean-all`      | 清理 `build/` 与 `bin/`          |




## 目录结构

```text
arch/x86_64/
  boot/          MBR、loader、boot.inc、gdt.inc
  kernel.asm     中断入口、键盘中断
  print.asm      底层打印
  linker.ld      高半区链接脚本
arch/x86/        旧 ia32 源码（参考，不参与当前构建）
kernel/          kmain、IRQ、printk、boot_banner
mm/              bootstrap.c（MVP 桩）；memory.c 等旧代码保留未链接
drivers/         console、keyboard（MVP）；ide、timer 等未链接
sched/           core.c（MVP 桩）；thread.c 等旧代码保留未链接
proc/            syscall-stub.c（MVP 桩）；fork/exec 等未链接
fs/              fs-stub.c（MVP 桩）；dir/file/inode 等未链接
shell/           REPL 与内置命令（cmd.c、repl.c）
lib/             运行时库（string.c 等；kernel/、user/ 子目录为旧代码）
include/         公共头文件（aitos/、drivers/ 等）
tests/           独立测试 main（未接入 Makefile）
build/           编译产物（.o、kernel.elf、kernel.bin）
bin/             磁盘镜像（hd60M.img、hd80M.img）
scripts/         环境安装、QEMU 运行/调试（install_devenv.sh、run-qemu.sh、qemu-ui.sh）
build/gdb/       编译生成的 GDB 初始化脚本
env.rc.example   机器环境配置模板
.vscode/         IDE 调试配置
```

头文件统一放在 `include/`，使用 `#include <子系统/头文件.h>`，例如：

```c
#include <aitos/init.h>
#include <aitos/console.h>
#include <aitos/shell.h>
```



## 常见问题

`**make` 报错 `bin/hd60M.img: No such file or directory**`  
`make` 会自动创建 `bin/` 目录；若仍失败请执行 `make clean-all && make`。

`**make debug` 后直接回到 shell、没有 `(gdb)` 提示符**  
使用 `ssh -t user@host` 登录以分配伪终端；或执行 `make debug-tmux`。

**GDB 连接失败**  
确认 QEMU 已启动（Linux：`ss -tlnp | grep 1234`；macOS：`lsof -iTCP:1234 -sTCP:LISTEN`）；残留进程执行 `make stop` 后重试。

**GDB 无源码或断点不准**  
确认以 `DEBUG=1` 编译：`make debug` 会自动处理。

`**make debug-tmux` 提示无 tmux**  
安装 `tmux` 或使用 `make debug` / `make debug-gui`。

**无头调试时 Shell 无法输入**  
`make debug` 启动后会写入 `bin/qemu-serial.pty`，执行 `screen $(cat bin/qemu-serial.pty)` 连接串口；或直接使用 `make debug-tmux`。

**本地开发更推荐 GUI 模式**  
`make run-qemu-gui` / `make debug-gui` 在 QEMU 窗口中直接键盘交互，无需配置串口 pty。

**macOS 上** `make` **报 Mach-O / section 相关错误**  
未配置交叉工具链。执行 `./scripts/install_devenv.sh`，或手动在 `env.rc` 中设置 `x86_64-elf-gcc` / `x86_64-elf-ld` / `x86_64-elf-objcopy`。