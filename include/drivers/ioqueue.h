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

#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H
#include <lib/stdint.h>
#include <sched/sync.h>
#include <sched/thread.h>

#define bufsize 64

// circular queue
struct ioqueue
{
    struct lock         lock;
    struct task_struct* producer;
    struct task_struct* consumer;
    char                buf[bufsize];
    int32_t             head; // head of queue: write data
    int32_t             tail; // tail of queue: read data
};

void ioqueue_init(struct ioqueue* ioq);
bool ioq_full(struct ioqueue* ioq);
bool ioq_empty(struct ioqueue* ioq);
char ioq_getchar(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq, char target);

#endif
