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

#include <kernel/init.h>
#include <drivers/console.h>
#include <fs/fs.h>
#include <drivers/ide.h>
#include <kernel/interrupt.h>
#include <drivers/keyboard.h>
#include <mm/memory.h>
#include <lib/kernel/print.h>
#include <proc/syscall-init.h>
#include <sched/thread.h>
#include <drivers/timer.h>
#include <proc/tss.h>

// initial all module
void init_all()
{
    put_str("init_all\n");
    idt_init();
    mem_init();
    thread_init();
    timer_init();
    console_init();
    keyboard_init();
    tss_init();
    syscall_init();
    ide_init();
    filesys_init();
}
