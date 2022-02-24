/*
 * mxml_mem.c
 *
 *  Created on: 24 feb. 2022
 *      Author: egonzalez
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

unsigned int totalAlloc = 0;
void *ptrs[1000];
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
    ptrs[ptr_cnt++] = p;
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
    ptrs[ptr_cnt++] = p;
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

void _mxml_free_all(void) {
    int cnt = 0;
    for (int n = 0; n < 1000; n++)
        if (ptrs[n] != 0) {
            free(ptrs[n]);
            ++cnt;
        }
    //if (cnt != 0)
    //    printf("WARNING!!! ");
    //printf("_mxml_free_all: %d\n", cnt);
}
