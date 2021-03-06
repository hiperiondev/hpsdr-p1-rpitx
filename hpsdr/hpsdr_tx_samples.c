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

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "hpsdr_main.h"
#include "hpsdr_definitions.h"
#include "hpsdr_functions.h"
#include "hpsdr_debug.h"
#include "librpitx.h"
#include "hpsdr_protocol.h"

uint8_t *bp;
int j;
int16_t samplei, sampleq;
int tx_iq_ptr = 0;
int burst_cnt = 0;
int CplxSampleNumber = 0;
float _Complex *CIQBuffer;
int ciqbuffer_ptr = 0;
int Harmonic = 1;

void samples_rcv(uint8_t *buffer) {
    // Put TX IQ samples into the ring buffer
    // In the old protocol, samples come in groups of 8 bytes L1 L0 R1 R0 I1 I0 Q1 Q0
    // Here, L1/L0 and R1/R0 are audio samples, and I1/I0 and Q1/Q0 are the TX iq samples
    // I1 contains bits 8-15 and I0 bits 0-7 of a signed 16-bit integer. We convert this
    // here to double.
    double disample, dqsample;
    bp = buffer + 16;  // skip 8 header and 8 SYNC/C&C bytes

    for (j = 0; j < 126; j++) {
        bp += 4;
        samplei = (int) ((signed char) *bp++) << 8;
        samplei |= (int) ((signed char) *bp++ & 0xFF);
        disample = samplei * 0.000030518509476;
        sampleq = (int) ((signed char) *bp++) << 8;
        sampleq |= (int) ((signed char) *bp++ & 0xFF);
        dqsample = sampleq * 0.000030518509476;
        tx_arg.iq_buffer[tx_iq_ptr++] = disample + dqsample * I;

        if (tx_iq_ptr > TXLEN * config.global.iqburst)
            tx_iq_ptr = 0;

        if ((tx_iq_ptr % config.global.iqburst) == 0)
            ++burst_cnt;

        if (j == 62)
            bp += 8;  // skip 8 SYNC/C&C bytes of second block
    }

}
