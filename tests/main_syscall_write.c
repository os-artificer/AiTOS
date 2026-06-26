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

#include <drivers/console.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <drivers/ioqueue.h>
#include <drivers/keyboard.h>
#include <lib/kernel/print.h>
#include <proc/process.h>
#include <lib/stdio.h>
#include <proc/syscall-init.h>
#include <lib/user/syscall.h>
#include <sched/thread.h>

void k_thread_a(void*);
void k_thread_b(void*);
void u_proc_a(void);
void u_proc_b(void);
int  prog_a_pid = 0, prog_b_pid = 0;

int main(void)
{
    put_str("kernel starting!\n");
    init_all();

    process_execute(u_proc_a, "user_proc_a");
    process_execute(u_proc_b, "user_proc_b");

    intr_enable();
    console_put_str(" main_pid:0x");
    console_put_int(sys_getpid());
    console_put_char('\n');

    thread_start("k_thread_a", 31, k_thread_a, "argA ");
    thread_start("k_thread_b", 31, k_thread_b, "argB  ");

    while (1);
    return 0;
}

void k_thread_a(void* arg)
{
    char* param = arg;
    console_put_str(" thread_a_pid:0x");
    console_put_int(sys_getpid());
    console_put_char('\n');

    while (1);
}
void k_thread_b(void* arg)
{
    char* param = arg;
    console_put_str(" thread_b_pid:0x");
    console_put_int(sys_getpid());
    console_put_char('\n');

    while (1);
}
void u_proc_a(void)
{
    char* name = "prog_a";
    printf(" I am %s , my pid : %d%c", name, getpid(), '\n');
    while (1);
}

void u_proc_b(void)
{
    char* name     = "prog_b";
    char  buf[100] = {0};
    printf(" I am %s , my pid : %d%c", name, getpid(), '\n');
    sprintf(buf, " test sprintf!!!!%c", '\n');
    write(buf);
    while (1);
}
