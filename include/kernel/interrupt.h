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

#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include <lib/stdint.h>
typedef void* intr_handler;
void          register_handler(uint8_t vector_no, intr_handler function);
void          idt_init(void);

// define the two status of interrupt
// INTR_OFF = 0 ,turn off interrupt
// INTR_ON = 1 , turn on interrupt
enum intr_status
{
    INTR_OFF,
    INTR_ON
};
enum intr_status intr_get_status(void);
enum intr_status intr_set_status(enum intr_status);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);

#endif
