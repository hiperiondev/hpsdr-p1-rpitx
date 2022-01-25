/*
 * Copyright 2021 Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/hpsdr-p1-rpitx *
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

#ifndef _HPSDR_MAIN_H_
#define _HPSDR_MAIN_H_

#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <complex.h>

#include "librpitx.h"

extern pthread_t iqsender_tx_id;

struct Configs_T {
    struct {
        bool debug;
         int iqburst;
        char *emulation;
         int emulation_id;
    } global;
    struct {
          bool enabled;
          char *type;
           int *gpio_pins_lpf;
        size_t gpio_pins_lpf_length;
           int *gpio_pins_hpf;
        size_t gpio_pins_hpf_length;
          char **band_str;
           int *band_start;
           int *band_end;
           int *gpio_lpf;
           int *gpio_hpf;
        size_t band_str_length;
        size_t band_start_length;
        size_t band_end_length;
        size_t gpio_lpf_length;
        size_t gpio_hpf_length;
    } filters;
};
struct Configs_T *confs;

iqdmasync_t *iqsender;
extern int enable_thread;
extern int active_thread;
extern int device_emulation;

// RTXLEN must be an sixteen-fold multiple of 63
// because we have 63 samples per 512-byte METIS packet,
// and two METIS packets per TCP/UDP packet,
// and two/four/eight-fold up-sampling if the TX sample
// rate is 96000/192000/384000
#define TXLEN 10 // tx buffer len = TXLEN * iqburst

extern      int tx_iq_ptr;
extern     bool tx_init;
extern uint32_t last_seqnum;
extern uint32_t seqnum;

typedef struct tx_args_st {
    float _Complex *iq_buffer;
    iqdmasync_t *iqsender;
} tx_args_t;
tx_args_t tx_arg;

// Constants for conversion of TX power
extern double c1, c2;

// Using clock_nanosleep of librt
extern int clock_nanosleep(clockid_t __clock_id, int __flags, __const struct timespec *__req, struct timespec *__rem);

#endif
