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
#include <complex.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "hpsdr_debug.h"
#include "hpsdr_iq_tx.h"
#include "hpsdr_main.h"
#include "hpsdr_protocol.h"

#include "librpitx.h"

bool tx_init = false;
static long last_freq = 0;
pthread_mutex_t lock;

void iqsender_init(uint64_t TuneFrequency, int fifosize) {
    float ppmpll = 0.0;

    pthread_mutex_lock(&lock);
    iqdmasync_init(&iqsender, TuneFrequency, 48000, 14, fifosize, MODE_IQ);
    iqdmasync_Setppm(&iqsender, ppmpll);
    pthread_mutex_unlock(&lock);
    tx_init = true;

    hpsdr_dbg_printf(0, "Start rpitx iq send\n");
}

void iqsender_deinit(void) {
    if (iqsender != NULL) {
        tx_init = false;
        pthread_mutex_lock(&lock);
        iqdmasync_deinit(&iqsender);
        pthread_mutex_unlock(&lock);
    }

    hpsdr_dbg_printf(0, "Stop rpitx iq send\n");
}

void iqsender_clearBuffer(void) {
    float _Complex blankBuffer[IQBURST * 4];
    memset(blankBuffer, 0, IQBURST * 4 * sizeof(float _Complex));
    memset(tx_iq_buffer, TXLEN, sizeof(float _Complex));
    if (iqsender != NULL) {
        pthread_mutex_lock(&lock);
        iqdmasync_SetIQSamples(&iqsender, blankBuffer, (IQBURST * 4), 0);
        pthread_mutex_unlock(&lock);
    }

    hpsdr_dbg_printf(0, "-- Clear DMA buffer --\n");
}

void iqsender_set(void) {
    if (!tx_init) {
        if (settings.tx_freq < 1000000 || settings.tx_freq > 500000000) {
            hpsdr_dbg_printf(0, "Freq OUT OF RANGE\n");
            return;
        }

        hpsdr_dbg_printf(1, "Starting TX at Freq %ld\n", settings.tx_freq);
        iqsender_init(settings.tx_freq, IQBURST * 4);
        last_freq = settings.tx_freq;
        return;
    }

    if (settings.tx_freq != last_freq) {
        if (settings.tx_freq < 1000000 || settings.tx_freq > 500000000) {
            hpsdr_dbg_printf(0, "Freq OUT OF RANGE\n");
            return;
        }
        pthread_mutex_lock(&lock);
        iqsender_deinit();
        iqsender_init(settings.tx_freq, IQBURST * 4);
        pthread_mutex_unlock(&lock);
        last_freq = settings.tx_freq;
        hpsdr_dbg_printf(0, "TX frequency changed: %d\n", settings.tx_freq);
    }
}

void* iqsender_tx(void *data) {
    int CplxSampleNumber = 0;
    int ptr = 0;
    int Harmonic = 1;
    float _Complex CIQBuffer[IQBURST];
    memset(CIQBuffer, IQBURST, sizeof(float _Complex));

    while (1) {
        while (iqsender == NULL)
            usleep(500);

        CIQBuffer[CplxSampleNumber++] = tx_iq_buffer[ptr];
        tx_iq_buffer[ptr++] = 0;
        if (ptr > TXLEN)
            ptr = 0;

        if (CplxSampleNumber == IQBURST) {
            if (iqsender != NULL) {
                pthread_mutex_lock(&lock);
                iqdmasync_SetIQSamples(&iqsender, CIQBuffer, CplxSampleNumber, Harmonic);
                pthread_mutex_unlock(&lock);
            }
            CplxSampleNumber = 0;
        }

    }

    return NULL;
}
