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

#include <lib/user/assert.h>

#include <lib/kernel/print.h>
#include <lib/stdio.h>

// print the filename ,line ,function name ,condition
void user_spin(char* filename, int line, const char* func,
               const char* condition)
{
    put_str("\n\n\n!!!!! error !!!!!\n");
    printf("filename: %s\nline: %d\nfunction: %s\ncondition: %s\n", filename,
           line, func, condition);
    while (1);
}

