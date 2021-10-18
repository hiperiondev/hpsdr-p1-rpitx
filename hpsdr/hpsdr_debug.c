/*
 * Copyright 2021 Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/hpsdr-protocol1-rpitx *
 *
 * This is based on other projects:
 *    librpitx (https://github.com/F5OEO/librpitx)
 *    HPSDR simulator (https://github.com/g0orx/pihpsdr)
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

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "hpsdr_debug.h"

static int hpsdr_debug_level = 0;
int hpsdr_debug_id = 1;

void hpsdr_dbg_setlevel(int Level) {
    hpsdr_debug_level = Level;
}

int hpsdr_dbg_getlevel(void) {
    return hpsdr_debug_level;
}

void hpsdr_dbg_printf(int Level, const char *fmt, ...) {
    if (Level <= hpsdr_debug_level) {
        bool debug_id_m = false;
        va_list args;
        va_start(args, fmt);
        if (fmt[0] == '<') {
            --hpsdr_debug_id;
            debug_id_m = true;
            fprintf(stderr, "[hpsdr] %*c(%d)", hpsdr_debug_id, ' ', hpsdr_debug_id);
        }
        if (fmt[0] == '>') {
            debug_id_m = true;
            fprintf(stderr, "[hpsdr] %*c(%d)", hpsdr_debug_id, ' ', hpsdr_debug_id);
            ++hpsdr_debug_id;
        }
        if (!debug_id_m)
            fprintf(stderr, "[hpsdr] %*c", hpsdr_debug_id, ' ');
        vfprintf(stderr, fmt, args);
        va_end(args);
        debug_id_m = false;
    }
}
