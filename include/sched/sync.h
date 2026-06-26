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

#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include <lib/kernel/list.h>
#include <lib/stdint.h>
#include <sched/thread.h>

// struct of semaphore
struct semaphore
{
    uint8_t     value;
    struct list waiters;
};

// struct of lock
struct lock
{
    struct task_struct* holder;           // holder of lock
    struct semaphore    semaphore;        // mutex semaphore; semaphore = 1
    uint32_t            holder_repeat_nr; // the number of holder had apply for lock
};

void sema_init(struct semaphore* psema, uint8_t value);
void lock_init(struct lock* lock);
void sema_down(struct semaphore* psema);
void sema_up(struct semaphore* psema);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);

#endif
