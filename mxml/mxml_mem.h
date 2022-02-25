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
