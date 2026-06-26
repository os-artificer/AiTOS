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

#ifndef __LIB_STRING_H
#define __LIB_STRING_H
#include <lib/stdint.h>
void     memset(void* des_, uint8_t value, uint32_t size);
void     memcpy(void* des_, void* src_, uint32_t size);
int      memcmp(const void* a_, const void* b_, uint32_t size);
char*    strcpy(char* dst_, const char* src_);
uint32_t strlen(const char* str);
int8_t   strcmp(const char* a, const char* b);
char*    strchr(const char* str, const uint8_t ch);
char*    strrchr(const char* str, const uint8_t ch);
char*    strcat(char* dst_, const char* src_);
uint32_t strchrs(const char* str, uint8_t ch);

#endif
