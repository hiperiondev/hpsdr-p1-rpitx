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

#ifndef HPSDR_FUNCTIONS_H_
#define HPSDR_FUNCTIONS_H_

// special protocol
void hpsdr_program(uint8_t *buffer);
void hpsdr_erase_packet(uint8_t *buffer);
void hpsdr_set_ip(uint8_t *buffer);

///////////////

// ep2 functions
void ep2_adc1preamp(uint8_t *frame, int* reg, int val, char* str);
void ep2_adc2preamp(uint8_t *frame, int* reg, int val, char* str);
void ep2_adc3preamp(uint8_t *frame, int* reg, int val, char* str);
void ep2_adc4preamp(uint8_t *frame, int* reg, int val, char* str);
void ep2_alex6mlna(uint8_t *frame, int* reg, int val, char* str);
void ep2_alexbyphpfs(uint8_t *frame, int* reg, int val, char* str);
void ep2_alexhpf(uint8_t *frame, int* reg, int val, char* str);
void ep2_alexlpf(uint8_t *frame, int* reg, int val, char* str);
void ep2_alexmanhpflpf(uint8_t *frame, int* reg, int val, char* str);
void ep2_alextrdis(uint8_t *frame, int* reg, int val, char* str);
void ep2_c25extboarddata(uint8_t *frame, int* reg, int val, char* str);
void ep2_commonmercuryfreq(uint8_t *frame, int* reg, int val, char* str);
void ep2_cwdelay(uint8_t *frame, int* reg, int val, char* str);
void ep2_cwhang(uint8_t *frame, int* reg, int val, char* str);
void ep2_cwint(uint8_t *frame, int* reg, int val, char* str);
void ep2_cw_mode(uint8_t *frame, int* reg, int val, char* str);
void ep2_cwrev(uint8_t *frame, int* reg, int val, char* str);
void ep2_cwspacing(uint8_t *frame, int* reg, int val, char* str);
void ep2_cwspeed(uint8_t *frame, int* reg, int val, char* str);
void ep2_cwweight(uint8_t *frame, int* reg, int val, char* str);
void ep2_hermesconfig(uint8_t *frame, int* reg, int val, char* str);
void ep2_linegain(uint8_t *frame, int* reg, int val, char* str);
void ep2_mercuryattontx0(uint8_t *frame, int* reg, int val, char* str);
void ep2_mercuryattontx1(uint8_t *frame, int* reg, int val, char* str);
void ep2_metisdb9(uint8_t *frame, int* reg, int val, char* str);
void ep2_micbias(uint8_t *frame, int* reg, int val, char* str);
void ep2_micptt(uint8_t *frame, int* reg, int val, char* str);
void ep2_micsrc(uint8_t *frame, int* reg, int val, char* str);
void ep2_opencollector(uint8_t *frame, int* reg, int val, char* str);
void ep2_penelopeselect(uint8_t *frame, int* reg, int val, char* str);
void ep2_pm_config(uint8_t *frame, int* reg, int val, char* str);
void ep2_ptt(uint8_t *frame, int* reg, int val, char* str);
void ep2_puresignal(uint8_t *frame, int* reg, int val, char* str);
void ep2_receivers(uint8_t *frame, int* reg, int val, char* str);
void ep2_ref10mhz(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx1adc(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx1attenable(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx1att(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx1hlattgain(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx2adc(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx2att(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx3adc(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx4adc(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx5adc(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx6adc(uint8_t *frame, int* reg, int val, char* str);
void ep2_rx7adc(uint8_t *frame, int* reg, int val, char* str);
void ep2_rxfreq1(uint8_t *frame, long* reg, int val, char* str);
void ep2_rxfreq2(uint8_t *frame, long* reg, int val, char* str);
void ep2_rxfreq3(uint8_t *frame, long* reg, int val, char* str);
void ep2_rxfreq4(uint8_t *frame, long* reg, int val, char* str);
void ep2_rxfreq5(uint8_t *frame, long* reg, int val, char* str);
void ep2_rxfreq6(uint8_t *frame, long* reg, int val, char* str);
void ep2_rxfreq7(uint8_t *frame, long* reg, int val, char* str);
void ep2_samplerate(uint8_t *frame, int* reg, int val, char* str);
void ep2_sidetonefreq(uint8_t *frame, int* reg, int val, char* str);
void ep2_sidetonevolume(uint8_t *frame, int* reg, int val, char* str);
void ep2_src122mhz(uint8_t *frame, int* reg, int val, char* str);
void ep2_timestampmic(uint8_t *frame, int* reg, int val, char* str);
void ep2_tipring(uint8_t *frame, int* reg, int val, char* str);
void ep2_txatt(uint8_t *frame, int* reg, int val, char* str);
void ep2_txclasse(uint8_t *frame, int* reg, int val, char* str);
void ep2_txdrive(uint8_t *frame, int* reg, int val, char* str);
void ep2_txfreq(uint8_t *frame, long* reg, int val, char* str);
void ep2_vnamode(uint8_t *frame, int* reg, int val, char* str);

#endif
