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

#include <stdbool.h>
#include <complex.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "hpsdr_debug.h"
#include "hpsdr_iq_tx.h"
#include "hpsdr_main.h"
#include "hpsdr_protocol.h"

#include "librpitx.h"

#define Harmonic 1

        bool tx_init = false;
        bool sender_init = false;
        bool tx_sending = false;
unsigned int tx_block = 0;
 static long last_freq = 0;
   pthread_t iqsender_tx_id;

void iqsender_init(uint64_t TuneFrequency) {
    if (sender_init || (TuneFrequency < 1000)) {
        printf("avoid init!\n");
        return;
    }

    float ppmpll = 0.0;

    iqdmasync_init(&(tx_arg.iqsender), TuneFrequency, 48000, 14, iqburst * 4, MODE_IQ);
    iqdmasync_set_ppm(&(tx_arg.iqsender), ppmpll);

    tx_init = true;

    hpsdr_dbg_printf(0, "Start rpitx iq send\n");
}

void iqsender_deinit(void) {
    if (tx_arg.iqsender != NULL) {
        hpsdr_dbg_printf(0, "iqsender_deinit\n");
        tx_init = false;
        usleep(50000);
        iqdmasync_deinit(&(tx_arg.iqsender));
    } else {
        hpsdr_dbg_printf(0, "ERROR: iqsender NULL\n");
    }

    hpsdr_dbg_printf(0, "Stop rpitx iq send\n");
}

void iqsender_set(void) {
    if (!tx_init) {
        if (settings.tx_freq < 1000000 || settings.tx_freq > 500000000) {
            hpsdr_dbg_printf(0, "(init) Freq OUT OF RANGE (1000000 - 500000000) : %d\n", (int) settings.tx_freq);
            return;
        }

        hpsdr_dbg_printf(1, "Starting TX at Freq %ld (fifosize = %d)\n", settings.tx_freq, iqburst * 4);
        iqsender_init(settings.tx_freq);
        last_freq = settings.tx_freq;
        hpsdr_dbg_printf(0, "FTX at %ld\n", settings.tx_freq);
        return;
    }

    if (settings.tx_freq != last_freq) {
        if (settings.tx_freq < 1000000 || settings.tx_freq > 500000000) {
            hpsdr_dbg_printf(0, "(chg frq) Freq OUT OF RANGE (1000000 - 500000000) : %d\n", (int) settings.tx_freq);
            return;
        }

        hpsdr_dbg_printf(0, "Changing TX frequency\n");
        iqsender_deinit();
        iqsender_init(settings.tx_freq);

        hpsdr_dbg_printf(0, "TX frequency changed: %d->%d\n", last_freq, settings.tx_freq);
        last_freq = settings.tx_freq;
    }
}

void iqsender_clear_buffer(void) {
    memset(tx_arg.iq_buffer, 0, iqburst * TXLEN * sizeof(float _Complex));
}

void* iqsender_tx(void *data) {
    hpsdr_dbg_printf(0, "START SENDER THREAD\n");
    int buffer_offset = 0;

    if (tx_arg.iq_buffer == NULL) {
        hpsdr_dbg_printf(0, "ERROR: tx buffer not allocated\n");
        return NULL;
    }

    while (1) {
        if (tx_arg.iqsender == NULL || !tx_init) {
            usleep(100);
            continue;
        }

        buffer_offset = tx_block * iqburst;
        iqdmasync_set_iq_samples(&(tx_arg.iqsender), tx_arg.iq_buffer + buffer_offset, iqburst, Harmonic);

        ++tx_block;
        if (tx_block > TXLEN - 1)
            tx_block = 0;
    }

    hpsdr_dbg_printf(0, "STOP SENDER THREAD\n");
    return NULL;
}
