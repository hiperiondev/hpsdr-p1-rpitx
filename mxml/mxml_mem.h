/*
 * mxml_mem.h
 *
 *  Created on: 24 feb. 2022
 *      Author: egonzalez
 */

#ifndef MXML_MEM_H_
#define MXML_MEM_H_

//char _mxml_tag[256];

void* _mxml_malloc(size_t size);
void* _mxml_calloc(size_t count, size_t size);
void _mxml_free(void *ptr);
void mxml_free_all(void);

int memcpy_s(void *dst, size_t sizeInBytes, const void *src, size_t count);
int memmove_s(void *dst, size_t sizeInBytes, const void *src, size_t count);

#endif /* MXML_MEM_H_ */
