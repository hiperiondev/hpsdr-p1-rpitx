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
#include "hpsdr_config.h"

int device_emulation;
int enable_thread;
int active_thread;
double c1, c2;
//int iqburst = 1000;

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
    hpsdr_config_deinit();
    exit(1);
}

int main(int argc, char *argv[]) {
    int i;
    char filename[256] = "hpsdr_p1_rpitx.conf";

    for (int i = 0; i < 64; i++) {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = terminate;
        sigaction(i, &sa, NULL);
    }

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-config", 7)) {
            strncpy(filename, argv[i+1], 255);
            filename[255] = '\0';
        }
    }


    hpsdr_dbg_printf(0, "config file: %s\n", filename);
    hpsdr_config_init(filename);

    if (confs->global.debug) {
        librpitx_dbg_setlevel(1);
        hpsdr_dbg_setlevel(1);
    }

    switch (confs->global.emulation_id) {

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

    hpsdr_dbg_printf(0, "iqburst: %d\n", confs->global.iqburst);

    tx_arg.iq_buffer = (float _Complex*) malloc(confs->global.iqburst * TXLEN * sizeof(float _Complex));
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
