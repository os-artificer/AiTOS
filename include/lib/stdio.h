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

#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include <lib/stdint.h>

#define va_start(ap, v) ap = (va_list) & v // make ap point to the first argument
#define va_arg(ap, t) *((t*)(ap += 4))     // make ap point to next arg and return
#define va_end(ap) ap = NULL               // clear the ap

typedef char* va_list;
uint32_t      vsprintf(char* str, const char* format, va_list ap);
uint32_t      printf(const char* format, ...);
uint32_t      sprintf(char* buf, const char* format, ...);
#endif
