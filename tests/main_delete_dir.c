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

#include <drivers/console.h>
#include <fs/dir.h>
#include <fs/fs.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <drivers/ioqueue.h>
#include <drivers/keyboard.h>
#include <mm/memory.h>
#include <lib/kernel/print.h>
#include <proc/process.h>
#include <lib/stdio.h>
#include <lib/string.h>
#include <proc/syscall-init.h>
#include <lib/user/syscall.h>
#include <sched/thread.h>

int main(void)
{
    put_str("kernel starting!\n");
    init_all();
    intr_enable();

    printf("/dir1 content before delete /dir1/subdir1:\n");
    struct dir*       dir   = sys_opendir("/dir1/");
    char*             type  = NULL;
    struct dir_entry* dir_e = NULL;
    while ((dir_e = sys_readdir(dir)))
    {
        if (dir_e->f_type == FT_REGULAR)
        {
            type = "regular";
        }
        else
        {
            type = "directory";
        }
        printf("   %s    %s\n", type, dir_e->filename);
    }

    printf("try to delete nonempty directory /dir1/subdir1 \n");
    if (sys_rmdir("/dir1/subdir1") == -1)
    {
        printf("sys_rmdir: /dir1/subdir1 failed\n");
    }
    if (sys_unlink("/dir1/subdir1/file2") == -1)
    {
        printf("sys_unlink: /dir1/subdir1 failed\n");
    }
    printf("try to delete nonempty directory /dir1/subdir1 again \n");
    if (sys_rmdir("/dir1/subdir1") == -1)
    {
        printf("sys_rmdir: /dir1/subdir1 failed\n");
    }

    printf("/dir1 content after delete /dir1/subdir1:\n");
    sys_rewinddir(dir);
    while ((dir_e = sys_readdir(dir)))
    {
        if (dir_e->f_type == FT_REGULAR)
        {
            type = "regular";
        }
        else
        {
            type = "directory";
        }
        printf("   %s    %s\n", type, dir_e->filename);
    }
    while (1);
    return 0;
}

