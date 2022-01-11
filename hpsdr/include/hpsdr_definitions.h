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

#ifndef HPSDR_DEFINITIONS_H_
#define HPSDR_DEFINITIONS_H_

// devices
#define DEVICE_METIS            0
#define DEVICE_HERMES           1
#define DEVICE_GRIFFIN          2
#define DEVICE_ANGELIA          4
#define DEVICE_ORION            5
#define DEVICE_HERMES_LITE      6
#define DEVICE_HERMES_LITE2     1006
#define DEVICE_ORION2           10
#define DEVICE_C25              100

// port definitions from host
#define GENERAL_REGISTERS_FROM_HOST_PORT 1024
#define PROGRAMMING_FROM_HOST_PORT 1024
#define RECEIVER_SPECIFIC_REGISTERS_FROM_HOST_PORT 1025
#define TRANSMITTER_SPECIFIC_REGISTERS_FROM_HOST_PORT 1026
#define HIGH_PRIORITY_FROM_HOST_PORT 1027
#define AUDIO_FROM_HOST_PORT 1028
#define TX_IQ_FROM_HOST_PORT 1029

// port definitions to host
#define COMMAND_RESPONCE_TO_HOST_PORT 1024
#define HIGH_PRIORITY_TO_HOST_PORT 1025
#define MIC_LINE_TO_HOST_PORT 1026
#define WIDE_BAND_TO_HOST_PORT 1027
#define RX_IQ_TO_HOST_PORT_0 1035
#define RX_IQ_TO_HOST_PORT_1 1036
#define RX_IQ_TO_HOST_PORT_2 1037
#define RX_IQ_TO_HOST_PORT_3 1038
#define RX_IQ_TO_HOST_PORT_4 1039
#define RX_IQ_TO_HOST_PORT_5 1040
#define RX_IQ_TO_HOST_PORT_6 1041
#define RX_IQ_TO_HOST_PORT_7 1042

#endif /* HPSDR_DEFINITIONS_H_ */
