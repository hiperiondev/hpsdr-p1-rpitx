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

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include <hpsdr_protocol.h>
#include <unistd.h>
#include <pthread.h>

#include "hpsdr_debug.h"
#include "hpsdr_definitions.h"
#include "hpsdr_main.h"
#include "hpsdr_functions.h"
#include "hpsdr_ep2.h"
#include "hpsdr_ep6.h"
#include "hpsdr_tx_samples.h"

pthread_t op_handler_ep6_id;
int sock_TCP_Server;
int sock_TCP_Client;
int sock_udp;
struct sockaddr_in addr;
struct sockaddr_in addr_udp;
socklen_t lenaddr;
struct sockaddr_in addr_from;
struct timeval tv;
uint8_t buffer[1032];
int yes = 1;
int bytes_read, bytes_left;
int size;
uint32_t last_seqnum = 0xffffffff, seqnum;  // sequence number of received packet
int udp_retries = 0;
uint32_t code;
uint32_t *code0 = (uint32_t*) buffer;  // fast access to code of first buffer

uint8_t reply[11] = {
        0xef, //
        0xfe, //
        2,    //
        0xaa, //
        0xbb, //
        0xcc, //
        0xdd, //
        0xee, //
        0xff, //
        0,    //
        1     //
        };

uint8_t id[4] = {
        0xef, //
        0xfe, //
        1,    //
        6     //
        };

int hpsdr_network_init(void) {
    sock_TCP_Server = -1;
    sock_TCP_Client = -1;

    if ((sock_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        hpsdr_dbg_printf(1, "socket");
        return EXIT_FAILURE;
    }

    setsockopt(sock_udp, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock_udp, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));

    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(sock_udp, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    memset(&addr_udp, 0, sizeof(addr_udp));
    addr_udp.sin_family = AF_INET;
    addr_udp.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_udp.sin_port = htons(1024);

    if (bind(sock_udp, (struct sockaddr*) &addr_udp, sizeof(addr_udp)) < 0) {
        hpsdr_dbg_printf(1, "bind");
        return EXIT_FAILURE;
    }

    if ((sock_TCP_Server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        hpsdr_dbg_printf(1, "socket tcp");
        return EXIT_FAILURE;
    }

    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));

    int tcpmaxseg = 1032;
    setsockopt(sock_TCP_Server, IPPROTO_TCP, TCP_MAXSEG, (const char*) &tcpmaxseg, sizeof(int));

    int sndbufsize = 65535;
    int rcvbufsize = 65535;
    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_SNDBUF, (const char*) &sndbufsize, sizeof(int));
    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVBUF, (const char*) &rcvbufsize, sizeof(int));
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    if (bind(sock_TCP_Server, (struct sockaddr*) &addr_udp, sizeof(addr_udp)) < 0) {
        hpsdr_dbg_printf(1, "ERROR: bind tcp\n");
        return EXIT_FAILURE;
    }

    listen(sock_TCP_Server, 1024);
    hpsdr_dbg_printf(1, "Listening for TCP client connection request\n");

    int flags = fcntl(sock_TCP_Server, F_GETFL, 0);
    fcntl(sock_TCP_Server, F_SETFL, flags | O_NONBLOCK);

    return EXIT_SUCCESS;
}

void hpsdr_network_deinit(void) {
    close(sock_udp);

    if (sock_TCP_Client > -1) {
        close(sock_TCP_Client);
    }

    if (sock_TCP_Server > -1) {
        close(sock_TCP_Server);
    }
}

int hpsdr_network_process(void) {
    memcpy(buffer, id, 4);

    if (sock_TCP_Client > -1) {
        // using recvmmsg with a time-out should be used for a byte-stream protocol like tcp
        // (each "packet" in the datagram may be incomplete).
        // this is especially true if the socket has a receive time-out, but this problem
        // also occurs if the is no such receive time-out.
        // therefore we read a complete packet here (1032 bytes).
        // our tcp-extension to the hpsdr protocol ensures that only 1032-byte packets may arrive here.
        bytes_read = 0;
        bytes_left = 1032;
        while (bytes_left > 0) {
            size = recvfrom(sock_TCP_Client, buffer + bytes_read, (size_t) bytes_left, 0, NULL, 0);
            if (size < 0 && errno == EAGAIN)
                continue;
            if (size < 0)
                break;
            bytes_read += size;
            bytes_left -= size;
        }

        bytes_read = size;
        if (size >= 0) {
            // 1032 bytes have successfully been read by tcp.
            // let the downstream code know that there is a single packet, and its size
            bytes_read = 1032;

            // in the case of a metis-discovery packet, change the size to 63
            if (*code0 == 0x0002feef) {
                bytes_read = 63;
            }

            // in principle, we should check on (*code0 & 0x00ffffff) == 0x0004feef,
            // then we cover all kinds of start and stop packets.
            // in the case of a metis-stop packet, change the size to 64
            if (*code0 == 0x0004feef) {
                bytes_read = 64;
            }

            // in the case of a metis-start tcp packet, change the size to 64
            // the special start code 0x11 has no function any longer, but we shall still support it.
            if (*code0 == 0x1104feef || *code0 == 0x0104feef) {
                bytes_read = 64;
            }
        }
    } else {
        lenaddr = sizeof(addr_from);
        bytes_read = recvfrom(sock_udp, buffer, 1032, 0, (struct sockaddr*) &addr_from, &lenaddr);
        if (bytes_read > 0) {
            udp_retries = 0;
        } else {
            udp_retries++;
        }
    }

    if (bytes_read < 0 && errno != EAGAIN) {
        hpsdr_dbg_printf(1, "recvfrom");
        return EXIT_FAILURE;
    }

    // if nothing has arrived via udp for some time, try to open tcp connection.
    // "for some time" means 10 subsequent un-successful udp rcvmmsg() calls
    if (sock_TCP_Client < 0 && udp_retries > 10) {
        if ((sock_TCP_Client = accept(sock_TCP_Server, NULL, NULL)) > -1) {
            hpsdr_dbg_printf(1, "sock_TCP_Client: %d connected to sock_TCP_Server: %d\n", sock_TCP_Client, sock_TCP_Server);
        }
        // this avoids firing accept() too often if it constantly fails
        udp_retries = 0;
    }
    if (bytes_read <= 0)
        return EXIT_SUCCESS;

    memcpy(&code, buffer, 4);

    hpsdr_dbg_printf(2, "-- code received: %04x (%d)\n", code, code);

    switch (code) {

        // pc to sdr transmission via process_ep2
        case 0x0201feef:
            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 1032) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, (int) bytes_read);
                break;
            }

            // sequence number check
            seqnum = ((buffer[4] & 0xFF) << 24) + ((buffer[5] & 0xFF) << 16) + ((buffer[6] & 0xFF) << 8) + (buffer[7] & 0xFF);

            if (seqnum != last_seqnum + 1) {
                hpsdr_dbg_printf(1, "SEQ ERROR: last %ld, recvd %ld\n", (long) last_seqnum, (long) seqnum);
            }

            last_seqnum = seqnum;

            ep2_handler(buffer + 11);
            ep2_handler(buffer + 523);

            if (active_thread) {
                samples_rcv(buffer);
            }
            break;

            // respond to an incoming metis detection request
        case 0x0002feef:
            hpsdr_dbg_printf(1, "Respond to an incoming Metis detection request / code: 0x%08x\n", code);

            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 63) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, (int) bytes_read);
                break;
            }
            reply[2] = 2;
            if (active_thread) {
                reply[2] = 3;
            }
            reply[9] = 31; // software version
            reply[10] = DEVICE_EMULATION;
            if (DEVICE_EMULATION == DEVICE_HERMES_LITE2) {
                // use hl1 device id and new software version
                reply[9] = 41;
                reply[10] = DEVICE_HERMES_LITE;
            }
            memset(buffer, 0, 60);
            memcpy(buffer, reply, 11);

            if (sock_TCP_Client > -1) {
                // we will get into trouble if we respond via tcp while the radio is
                // running with tcp.
                // we simply suppress the response in this (very unlikely) case.
                if (!active_thread) {
                    if (send(sock_TCP_Client, buffer, 60, 0) < 0) {
                        hpsdr_dbg_printf(1, "TCP send error occurred when responding to an incoming Metis detection request!\n");
                    }
                    // close the tcp socket which was only used for the detection
                    close(sock_TCP_Client);
                    sock_TCP_Client = -1;
                }
            } else {
                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
            }

            break;

            // stop the sdr to pc transmission via handler_ep6
        case 0x0004feef:
            hpsdr_dbg_printf(1, "STOP the transmission via handler_ep6 / code: 0x%08x\n", code);

            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 64) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, bytes_read);
                break;
            }

            enable_thread = 0;
            while (active_thread)
                usleep(1000);

            if (sock_TCP_Client > -1) {
                close(sock_TCP_Client);
                sock_TCP_Client = -1;
            }
            break;

            // start the pc-to-sdr handler thread
        case 0x0104feef:
        case 0x0204feef:
        case 0x0304feef:
            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 64) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, bytes_read);
                break;
            }
            hpsdr_dbg_printf(1, "START the PC-to-SDR handler thread / code: 0x%08x\n", code);

            enable_thread = 0;
            while (active_thread)
                usleep(1000);
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = addr_from.sin_addr.s_addr;
            addr.sin_port = addr_from.sin_port;

            enable_thread = 1;
            active_thread = 1;

            if (pthread_create(&op_handler_ep6_id, NULL, ep6_handler, NULL) < 0) {
                hpsdr_dbg_printf(1, "ERROR: create protocol thread");
                return EXIT_FAILURE;
            }
            pthread_detach(op_handler_ep6_id);

            break;

            // non standard cases
        default:
            // "program" packet
            if (bytes_read == 264 && buffer[0] == 0xEF && buffer[1] == 0xFE && buffer[2] == 0x03 && buffer[3] == 0x01) {
                static long cnt = 0;
                unsigned long blks = (buffer[4] << 24) + (buffer[5] << 16) + (buffer[6] << 8) + buffer[7];
                hpsdr_dbg_printf(1, "Program blks=%lu count=%ld\r", blks, ++cnt);

                hpsdr_program(buffer);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                if (blks == cnt)
                    hpsdr_dbg_printf(1, "\n\n Programming Done!\n");
                break;
            }

            // "erase" packet
            if (bytes_read == 64 && buffer[0] == 0xEF && buffer[1] == 0xFE && buffer[2] == 0x03 && buffer[3] == 0x02) {
                hpsdr_dbg_printf(1, "Erase packet received:\n");

                hpsdr_erase_packet(buffer);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                break;

            }

            // "set ip" packet
            if (bytes_read == 63 && buffer[0] == 0xEF && buffer[1] == 0xFE && buffer[2] == 0x03) {
                hpsdr_dbg_printf(1, "SetIP packet received:\n");
                hpsdr_dbg_printf(1, "MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n", buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
                hpsdr_dbg_printf(1, "IP  address is %03d:%03d:%03d:%03d\n", buffer[9], buffer[10], buffer[11], buffer[12]);

                hpsdr_set_ip(buffer);

                sendto(sock_udp, buffer, 63, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                break;
            }
    }

    return EXIT_SUCCESS;
}

void hpsdr_network_send(uint8_t *buffer, size_t len) {
    int counter;
    if (sock_TCP_Client > -1) {
        counter = sendto(sock_TCP_Client, buffer, 1032, 0, (struct sockaddr*) &addr, sizeof(addr));
        if (counter < 0) {
            hpsdr_dbg_printf(1, "TCP sendmsg error occurred at sequence number: %u !\n", counter);
        }
    } else {
        sendto(sock_udp, buffer, 1032, 0, (struct sockaddr*) &addr, sizeof(addr));
    }
}
