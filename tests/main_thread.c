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
#include <lib/kernel/print.h>
#include <sched/thread.h>

void k_thread_a(void*);
void k_thread_b(void*);

int main(void)
{
    put_str("kernel starting!\n");
    init_all();
    thread_start("k_thread_a", 31, k_thread_a, "argA ");
    thread_start("k_thread_b", 8, k_thread_b, "argB ");

    intr_enable();
    while (1)
    {
        console_put_str("Main ");
    };
    return 0;
}

void k_thread_a(void* arg)
{
    char* param = arg;
    while (1)
    {
        console_put_str(param);
    }
}
void k_thread_b(void* arg)
{
    char* param = arg;
    while (1)
    {
        console_put_str(param);
    }
}
