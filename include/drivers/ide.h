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

#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H
#include <lib/kernel/bitmap.h>
#include <kernel/global.h>
#include <lib/kernel/list.h>
#include <lib/stdint.h>
#include <sched/sync.h>

// the struct of partition
struct partition
{
    uint32_t            start_lba;    // the start sector
    uint32_t            sec_cnt;      // the number of sector
    struct disk*        my_disk;      // the disk of which this partition belong to
    struct list_elem    part_tag;     // the tag of list
    char                name[8];      // the name of partition
    struct super_block* sb;           // the super block
    struct bitmap       block_bitmap; // block bitmap
    struct bitmap       inode_bitmap; // inode bitmap
    struct list         open_inode;   // the inode list of this partition
};

// the struct of hard disk
struct disk
{
    char                name[8];        // the name of hard disk
    struct ide_channel* my_channel;     // the channel if which this disk belong to
    uint8_t             dev_no;         // this hard disk is main 0 or slave 1
    struct partition    prim_parts[4];  // only 4 primary partition
    struct partition    logic_parts[8]; // we set 8 logic partition (but limitless actually)
};

// the struct of ata channel(ide channel)
struct ide_channel
{
    char             name[8];        // the name of this ata channel
    uint16_t         port_base;      // the start of port
    uint8_t          irq_no;         // the interrupt number of this ata channel
    struct lock      lock;           // lock
    bool             expecting_intr; // mean this channel is waiting for the interrupt of hard disk
    struct semaphore disk_done;      // used for block and wake up driver
    struct disk      devices[2];     // a channel have two disk(main and slave)
};

void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void ide_write(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void intr_hd_handler(uint8_t irq_no);
void ide_init(void);
#endif
