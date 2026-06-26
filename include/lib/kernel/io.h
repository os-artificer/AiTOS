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

/*-------------------------mathine_mode--------------------
    b --(QImode)low 8bit of register:[a-d]l
    w --(HImode) 2 bit of register :[a-d]x
-------------------------------------------------------------*/

#ifndef __LIB_IO_H
#define __LIB_IO_H
#include <lib/stdint.h>

// write a byte into port
static inline void outb(uint16_t port, uint8_t data)
{
    // N is represent 0~255, d represent dx,%b0 represent al , %w1 represent dx
    asm volatile("outb %b0,%w1" : : "a"(data), "Nd"(port));
}

// write word_cnt byte from the begin of addr into port
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt)
{
    // + is represent read and write ;outsw is write the 16 bit content in ds:esi into port
    asm volatile("cld;rep outsw" : "+S"(addr), "+c"(word_cnt) : "d"(port));
}

// read a byte from port
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile("inb %w1,%b0" : "=a"(data) : "Nd"(port));
    return data;
}

// read wrod_cnt byte from port into addr
static inline void insw(uint16_t port, const void* addr, uint32_t word_cnt)
{
    asm volatile("cld;rep insw" : "+D"(addr), "+c"(word_cnt) : "d"(port) : "memory");
}

#endif

