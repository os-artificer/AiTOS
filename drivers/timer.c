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

#include <drivers/timer.h>
#include <kernel/debug.h>
#include <kernel/global.h>
#include <kernel/interrupt.h>
#include <lib/kernel/io.h>
#include <lib/kernel/print.h>
#include <lib/stdint.h>
#include <sched/thread.h>

#define IRQ0_FREQUENCY 30000
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY // the initial value of counter 0
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0 // No.1 counter
#define COUNTER0_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

#define mil_seconds_per_intr 10

uint32_t ticks;

// write the control word into the control word register and set the initial value
static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl, uint8_t counter_mode, uint16_t counter_value)
{
    // write into control word register
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    // write the low bit of counter_value
    outb(counter_port, (uint8_t)counter_value);
    // writhe the hight bit of counter_value
    outb(counter_port, (uint8_t)(counter_value >> 8)); //!!!!! attention: heare is diffrent from book !!!!!
}

// timer interrupt handler
static void intr_timer_handler(void)
{
    struct task_struct* cur_thread = running_thread();
    ASSERT(cur_thread->stack_magic == 0x12345678);
    ++cur_thread->elapsed_ticks; // record time of current thread  had run in cpu
    ++ticks;                     // from first time interrupt to now

    if (cur_thread->ticks == 0)
    {
        schedule();
    }
    else
    {
        --cur_thread->ticks;
    }
}

// initial PIT8253
void timer_init()
{
    put_str("timer_init start \n");
    // set the frequency of counter
    frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER0_MODE, COUNTER0_VALUE);
    register_handler(0x20, intr_timer_handler);
    put_str("timer_init done \n");
}

// sleep in ticks
static void ticks_to_sleep(uint32_t sleep_ticks)
{
    uint32_t start_ticks = ticks;
    while (ticks - start_ticks < sleep_ticks)
    {
        thread_yield();
    }
}

// sleep in millisecond (1 second = 1000 millisecond)
void mtime_sleep(uint32_t m_seconds)
{
    uint32_t sleep_ticks = DIV_ROUND_UP(m_seconds, mil_seconds_per_intr);
    ASSERT(sleep_ticks > 0);
    ticks_to_sleep(sleep_ticks);
}
