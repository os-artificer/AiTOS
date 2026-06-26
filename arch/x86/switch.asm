; Copyright 2026 AiTOS authors.
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

[bits 32]
section .text
global switch_to
switch_to:
    ;save context of current thread
    ;<- in stack :here is the return address
    push  esi
    push  edi
    push  ebx
    push  ebp
    mov   eax,   [esp+20]           ;get the cur(current thread)
    mov   [eax], esp                ;save the esp(stack of current thread)

    ;resume context of next thread
    mov   eax, [esp+24]
    mov   esp, [eax]                ;reload new esp(stack of next thread)
    pop   ebp
    pop   ebx
    pop   edi
    pop   esi
    ret                             ;here is return address in new stack

