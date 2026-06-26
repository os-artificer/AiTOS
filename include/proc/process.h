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

#ifndef __USERPROG_PROCESS_H
#define __USERPROG_PROCESS_H
#include <sched/thread.h>

// bottom of stack is 0xc0000000 , but we should point to the start of this page(0x1000 = 4096)
#define USER_STACK3_VADDR (0xc0000000 - 0x1000)

// the start of user process virtual address
#define USER_VADDR_START 0x8048000

#define default_prio 31

void      start_process(void* filename_);
void      page_dir_activate(struct task_struct* p_thread);
void      process_activate(struct task_struct* p_thread);
uint32_t* create_page_dir(void);
void      create_user_vaddr_bitmap(struct task_struct* user_prog);
void      process_execute(void* filename, char* name);

#endif
