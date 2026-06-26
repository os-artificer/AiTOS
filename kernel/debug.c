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

#include <kernel/debug.h>
#include <kernel/interrupt.h>
#include <lib/kernel/print.h>

// print the filename ,line ,function name ,condition
void panic_spin(char*       filename,
                int         line,
                const char* func,
                const char* condition)
{
    // close interrupt
    intr_disable();
    put_str("\n\n\n!!!!! error !!!!!\n");
    put_str("filename:");
    put_str(filename);
    put_str("\n");
    put_str("line:0x");
    put_int(line);
    put_str("\n");
    put_str("function:");
    put_str((char*)func);
    put_str("\n");
    put_str("condition:");
    put_str((char*)condition);
    put_str("\n");
    while (1);
}
