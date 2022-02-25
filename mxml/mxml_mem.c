/*
 * Copyright 2021 Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/hpsdr-p1-rpitx *
 *
 * This is based on other projects:
 *    librpitx (https://github.com/F5OEO/librpitx)
 *    HPSDR simulator (https://github.com/g0orx/pihpsdr)
 *    small-memory XML config database library (https://github.com/dleonard0/mxml)
 *
 *    please contact their authors for more information.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "mxml_mem.h"

unsigned int totalAlloc = 0;
void *ptrs[1000];
//char mxml_tags[1000][256];

int ptr_cnt = 0;
bool mem_init = false;

void _mxml_mem_init(void) {
    for (int n = 0; n < 1000; n++)
        ptrs[n] = 0;
    mem_init = true;
}

void* _mxml_malloc(size_t size) {
    if (!mem_init)
        _mxml_mem_init();

    void *p;
    totalAlloc += size;

    p = malloc(size + sizeof(int));
    *(int*) p = size;
    ptrs[ptr_cnt] = p;
    //strcpy(mxml_tags[ptr_cnt++], _mxml_tag);

    //printf("_mxml_malloc(total: %d)\n", totalAlloc);
    return (void*) (((int*) p) + 1);
}

void* _mxml_calloc(size_t count, size_t size) {
    if (!mem_init)
        _mxml_mem_init();
    void *p;
    totalAlloc += size * count;

    p = calloc(count, size + sizeof(int));
    *(int*) p = size;
    ptrs[ptr_cnt] = p;
    //strcpy(mxml_tags[ptr_cnt++], _mxml_tag);

    //printf("_mxml_calloc(total: %d)\n", totalAlloc);
    return (void*) (((int*) p) + 1);
}

void _mxml_free(void *ptr) {
    if (!mem_init)
        _mxml_mem_init();
    int ptr_id = -1;
    ptr = (void*) (((int*) ptr) - 1);
    totalAlloc -= *(int*) ptr;
    for (int n = 0; n < ptr_cnt; n++)
        if (ptrs[n] == ptr) {
            ptr_id = n;
            break;
        }

    free(ptr);
    if (ptr_id != -1)
        ptrs[ptr_id] = 0;
    //printf("_mxml_free(total: %d/%d)\n", totalAlloc, ptr_cnt);
}

void mxml_free_all(void) {
    int cnt = 0;
    for (int n = 0; n < 1000; n++)
        if (ptrs[n] != 0) {
            //printf("_mxml_free_all: %s\n", mxml_tags[n]);
            free(ptrs[n]);
            ++cnt;
        }
    //if (cnt != 0)
    //    printf("WARNING!!! ");
    //printf("_mxml_free_all: %d\n", cnt);
}
