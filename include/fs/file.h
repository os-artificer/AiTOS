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

#ifndef __FS_FILE_H
#define __FS_FILE_H
#include <fs/dir.h>
#include <drivers/ide.h>
#include <lib/stdint.h>

#define MAX_FILE_OPEN 32 // max file can open

// file struct
struct file
{
    uint32_t      fd_pos; // the operate position of file
    uint32_t      fd_flag;
    struct inode* fd_inode;
};

// stand file descriptor
enum std_fd
{
    stdin_no,
    stdout_no,
    stderr_no
};

// bitmap type
enum bitmap_type
{
    INODE_BITMAP,
    BLOCK_BITMAP
};

int32_t get_free_slot_in_global(void);
int32_t pcb_fd_install(int32_t global_fd_index);
int32_t inode_bitmap_alloc(struct partition* part);
int32_t block_bitmap_alloc(struct partition* part);
void    bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp);
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag);
int32_t file_open(uint32_t inode_no, uint8_t flag);
int32_t file_close(struct file* file);
int32_t file_write(struct file* file, const void* buf, uint32_t count);
int32_t file_read(struct file* file, void* buf, uint32_t count);

#endif
