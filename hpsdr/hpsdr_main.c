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

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "librpitx.h"
#include "hpsdr_main.h"
#include "hpsdr_debug.h"
#include "hpsdr_functions.h"
#include "hpsdr_definitions.h"
#include "hpsdr_iq_tx.h"
#include "hpsdr_network.h"

int DEVICE_EMULATION;
int enable_thread;
int active_thread;
double c1, c2;
int IQBURST = 100;

char exit_signal[33][17] = {
        "NOSIGNAL",
        "SIGHUP",
        "SIGINT",
        "SIGQUIT",
        "SIGILL",
        "SIGTRAP",
        "SIGABRT",
        "SIGIOT",
        "SIGBUS",
        "SIGFPE",
        "SIGKILL",
        "SIGUSR1",
        "SIGSEGV",
        "SIGUSR2",
        "SIGPIPE",
        "SIGALRM",
        "SIGTERM",
        "SIGSTKFLT",
        "SIGCHLD",
        "SIGCONT",
        "SIGSTOP",
        "SIGTSTP",
        "SIGTTIN",
        "SIGTTOU",
        "SIGURG",
        "SIGXCPU",
        "SIGXFSZ",
        "SIGVTALRM",
        "SIGPROF",
        "SIGWINCH",
        "SIGIO",
        "SIGPWR",
        "SIGSYS/SIGUNUSED",
};

static void terminate(int num) {
    fprintf(stderr, "Caught signal - Terminating 0x%x/%d(%s)\n", num, num, exit_signal[num]);
    iqsender_deinit();
    exit(1);
}

int main(int argc, char *argv[]) {
    int i;

    for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

    // examples for METIS:   ATLAS bus with Mercury/Penelope boards
    // examples for HERMES:  ANAN10, ANAN100
    // examples for ANGELIA: ANAN100D
    // examples for ORION:   ANAN200D
    // examples for ORION2:  ANAN7000, ANAN8000
    // examples for C25:     RedPitaya based boards with fixed ADC connections
    DEVICE_EMULATION = DEVICE_HERMES_LITE;

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-atlas", 6)) {
            DEVICE_EMULATION = DEVICE_METIS;
        }

        if (!strncmp(argv[i], "-hermes", 7)) {
            hpsdr_dbg_printf(1, "HERMES\n");
            DEVICE_EMULATION = DEVICE_HERMES;
        }

        if (!strncmp(argv[i], "-griffin", 8)) {
            DEVICE_EMULATION = DEVICE_GRIFFIN;
        }

        if (!strncmp(argv[i], "-angelia", 8)) {
            DEVICE_EMULATION = DEVICE_ANGELIA;
        }

        if (!strncmp(argv[i], "-orion", 6)) {
            DEVICE_EMULATION = DEVICE_ORION;
        }

        if (!strncmp(argv[i], "-orion2", 7)) {
            DEVICE_EMULATION = DEVICE_ORION2;
        }

        if (!strncmp(argv[i], "-hermeslite", 11)) {
            DEVICE_EMULATION = DEVICE_HERMES_LITE;
        }

        if (!strncmp(argv[i], "-hermeslite2", 12)) {
            DEVICE_EMULATION = DEVICE_HERMES_LITE2;
        }

        if (!strncmp(argv[i], "-c25", 4)) {
            DEVICE_EMULATION = DEVICE_C25;
        }

        if (!strncmp(argv[i], "-debug", 6)) {
            librpitx_dbg_setlevel(1);
            hpsdr_dbg_setlevel(1);
        }

        if (!strncmp(argv[i], "-iqburst", 8)) {
            IQBURST = atoi(argv[i + 1]);
        }
    }

    switch (DEVICE_EMULATION) {

        case DEVICE_METIS:
            hpsdr_dbg_printf(1, "DEVICE is METIS\n");
            c1 = 3.3;
            c2 = 0.090;
            break;

        case DEVICE_HERMES:
            hpsdr_dbg_printf(1, "DEVICE is HERMES\n");
            c1 = 3.3;
            c2 = 0.095;
            break;

        case DEVICE_GRIFFIN:
            hpsdr_dbg_printf(1, "DEVICE is GRIFFIN\n");
            c1 = 3.3;
            c2 = 0.095;
            break;

        case DEVICE_ANGELIA:
            hpsdr_dbg_printf(1, "DEVICE is ANGELIA\n");
            c1 = 3.3;
            c2 = 0.095;
            break;

        case DEVICE_HERMES_LITE:
            hpsdr_dbg_printf(1, "DEVICE is HermesLite V1\n");
            c1 = 3.3;
            c2 = 0.095;
            break;

        case DEVICE_HERMES_LITE2:
            hpsdr_dbg_printf(1, "DEVICE is HermesLite V2\n");
            c1 = 3.3;
            c2 = 0.095;
            break;

        case DEVICE_ORION:
            hpsdr_dbg_printf(1, "DEVICE is ORION\n");
            c1 = 5.0;
            c2 = 0.108;
            break;

        case DEVICE_ORION2:
            hpsdr_dbg_printf(1, "DEVICE is ORION MkII\n");
            c1 = 5.0;
            c2 = 0.108;
            break;

        case DEVICE_C25:
            hpsdr_dbg_printf(1, "DEVICE is STEMlab/C25\n");
            c1 = 3.3;
            c2 = 0.090;
            break;
    }

    tx_arg.iq_buffer = (float _Complex*) malloc(IQBURST * TXLEN * sizeof(float _Complex));
    tx_arg.iqsender = NULL;

    pthread_create(&iqsender_tx_id, NULL, &iqsender_tx, (void*) &tx_arg);
    pthread_detach(iqsender_tx_id);

    hpsdr_network_init();
    while (1) {
        if (hpsdr_network_process() != EXIT_SUCCESS)
            break;
    }
    hpsdr_network_deinit();

    return EXIT_SUCCESS;
}
