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

#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

void panic_spin(char* filename, int line, const char* func, const char* condition);

//------------------------------------ __VA_ARGS__---------------------------------
// stand for the all argument of ...
// ... is mean the argument is changalbe

#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

//------------------------------------------------------------------------------

#ifdef NDEBUG
#define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION)  \
    if (CONDITION)         \
    {                      \
    }                      \
    else                   \
    {                      \
        PANIC(#CONDITION); \
    }

#endif /*NDEBUG*/

#endif /*__KERNEL_DEBUG_H*/
