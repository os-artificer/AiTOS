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

#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include <fs/dir.h>
#include <fs/fs.h>
#include <lib/stdint.h>
#include <sched/thread.h>
enum SYSCALL_NR
{
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE,
    SYS_FORK,
    SYS_READ,
    SYS_PUTCHAR,
    SYS_CLEAR,
    SYS_GETCWD,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_LSEEK,
    SYS_UNLINK,
    SYS_MKDIR,
    SYS_OPENDIR,
    SYS_CLOSEDIR,
    SYS_CHDIR,
    SYS_RMDIR,
    SYS_READDIR,
    SYS_REWINDDIR,
    SYS_STAT,
    SYS_PS,
    SYS_EXECV
};

uint32_t          getpid(void);
uint32_t          write(int32_t fd, const void* buf, uint32_t count);
void*             malloc(uint32_t size);
void              free(void* ptr);
pid_t             fork(void);
int32_t           read(int32_t fd, void* buf, uint32_t count);
void              putchar(char char_ascii);
void              clear(void);
char*             getcwd(char* buf, uint32_t size);
int32_t           open(char* pathname, uint8_t flag);
int32_t           close(int32_t fd);
int32_t           lseek(int32_t fd, int32_t offset, uint8_t whence);
int32_t           unlink(const char* pathname);
int32_t           mkdir(const char* pathname);
struct dir*       opendir(const char* name);
int32_t           closedir(struct dir* dir);
int32_t           rmdir(const char* pathname);
struct dir_entry* readdir(struct dir* dir);
void              rewinddir(struct dir* dir);
int32_t           stat(const char* path, struct stat* buf);
int32_t           chdir(const char* path);
void              ps(void);
int32_t           execv(const char* path, const char* argv[]);

#endif
