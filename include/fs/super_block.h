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

#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H
#include <lib/stdint.h>
// super block
struct super_block
{
    uint32_t magic; // magic number , mean different fs

    uint32_t sec_cnt;       // the all sectors in this partition
    uint32_t inode_cnt;     // the all inode in this partition
    uint32_t part_lba_base; // the start lba address of in this partition

    uint32_t block_bitmap_lba;   // the start lba address of block bitmap
    uint32_t block_bitmap_sects; // the number of sectors

    uint32_t inode_bitmap_lba;   // the start lba address of inode bitmap
    uint32_t inode_bitmap_sects; // the number of sectors

    uint32_t inode_table_lba;   // the start lba address of inode table
    uint32_t inode_table_sects; // the number of sectors

    uint32_t data_start_lba; // the start lba address of first sector in data area
    uint32_t root_inode_no;  // the inode number of root
    uint32_t dir_entry_size; // size of directory

    uint8_t pad[460]; // gather enough 512 byte (1 sector)
} __attribute__((packed));

#endif
