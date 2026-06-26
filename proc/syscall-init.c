/**
 * Copyright 2026 AiTOS authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include <proc/syscall-init.h>
#include <drivers/console.h>
#include <proc/exec.h>
#include <proc/fork.h>
#include <fs/fs.h>
#include <lib/kernel/print.h>
#include <lib/string.h>
#include <lib/user/syscall.h>
#include <sched/thread.h>

#define syscall_nr 32
typedef void* syscall;
syscall       syscall_table[syscall_nr]; // the member of array is function pointer

// return the pid of current id
uint32_t sys_getpid(void)
{
    return running_thread()->pid;
}

void sys_putchar(char char_ascii)
{
    console_put_char(char_ascii);
}

void syscall_init(void)
{
    put_str("syscall_init start\n");

    syscall_table[SYS_GETPID]    = sys_getpid;
    syscall_table[SYS_WRITE]     = sys_write;
    syscall_table[SYS_MALLOC]    = sys_malloc;
    syscall_table[SYS_FREE]      = sys_free;
    syscall_table[SYS_FORK]      = sys_fork;
    syscall_table[SYS_READ]      = sys_read;
    syscall_table[SYS_PUTCHAR]   = sys_putchar;
    syscall_table[SYS_CLEAR]     = cls_screen;
    syscall_table[SYS_GETCWD]    = sys_getcwd;
    syscall_table[SYS_OPEN]      = sys_open;
    syscall_table[SYS_CLOSE]     = sys_close;
    syscall_table[SYS_LSEEK]     = sys_lseek;
    syscall_table[SYS_UNLINK]    = sys_unlink;
    syscall_table[SYS_MKDIR]     = sys_mkdir;
    syscall_table[SYS_OPENDIR]   = sys_opendir;
    syscall_table[SYS_CLOSEDIR]  = sys_closedir;
    syscall_table[SYS_RMDIR]     = sys_rmdir;
    syscall_table[SYS_READDIR]   = sys_readdir;
    syscall_table[SYS_REWINDDIR] = sys_rewinddir;
    syscall_table[SYS_STAT]      = sys_stat;
    syscall_table[SYS_CHDIR]     = sys_chdir;
    syscall_table[SYS_PS]        = sys_ps;
    syscall_table[SYS_EXECV]     = sys_execv;

    put_str("syscall_init done\n");
}

