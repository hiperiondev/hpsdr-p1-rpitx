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

#ifndef HPSDR_EP2_H_
#define HPSDR_EP2_H_

#define EP2_PTT(fr)                  (fr[0] & 1)
#define EP2_RATE(fr)                 ((fr[1] & 0x03) >> 0)
#define EP2_REF10(fr)                ((fr[1] & 0x0C) >> 3)
#define EP2_SRC122(fr)               ((fr[1] & 0x10) >> 4)
#define EP2_PMCONFIG(fr)             ((fr[1] & 0x60) >> 5)
#define EP2_MICSRC(fr)               ((fr[1] & 0x80) >> 7)
#define EP2_TXCLASSC(fr)             (fr[2] & 1)
#define EP2_OPENCOLLECTOROUTPUTS(fr) ((fr[2] & 0xfe) >> 1)
#define EP2_RECEIVERS(fr)            (((fr[4] >> 3) & 7) + 1)
#define EP2_MICTS(fr)                (((fr[4] >> 6) & 1))
#define EP2_COMMONMERCURYFREQ(fr)    ((fr[4] >> 7) & 1)
#define EP2_ALEXATT(fr)              (fr[3] & 0x03)
#define EP2_PREAMP(fr)               ((fr[3] & 0x04) >> 2)
#define EP2_LTDITHER(fr)             ((fr[3] & 0x08) >> 3)
#define EP2_LTRANDOM(fr)             ((fr[3] & 0x10) >> 4)
#define EP2_ALEXRXANT(fr)            ((fr[3] & 0x60) >> 5)
#define EP2_ALEXXRXOUT(fr)           ((fr[3] & 0x80) >> 7)
#define EP2_ALETXREL(fr)             ((fr[4] >> 0) & 3)
#define EP2_DUPLEX(fr)               ((fr[4] >> 2) & 1)
#define EP2_TXFREQ(fr)               ((fr[4] | (fr[3] << 8) | (fr[2] << 16) | (fr[1] << 24)))
#define EP2_RXFREQ(fr)               (fr[4] | (fr[3] << 8)  | (fr[2] << 16) | (fr[1] << 24))
#define EP2_TXDRIVE(fr)              (fr[1])
#define EP2_HERMESCONFIG(fr)         (fr[2] & 0x3F)
#define EP2_ALEXMANUALHPFLPF(fr)     ((fr[2] >> 6) & 0x01)
#define EP2_VNA(fr)                  ((fr[2] >> 7) & 0x01)
#define EP2_ALEXHPF(fr)              (fr[3] & 0x1F)
#define EP2_ALEXBYPASS(fr)           ((fr[3] >> 5) & 0x01)
#define EP2_LNA6M(fr)                ((fr[3] >> 6) & 0x01)
#define EP2_ALEXTRDISABLE(fr)        ((fr[3] >> 7) & 0x01)
#define EP2_ALEXLPF(fr)              (fr[4])
#define EP2_ADC1PREAMP(fr)           ((fr[1] & 0x01) >> 0)
#define EP2_ADC2PREAMP(fr)           ((fr[1] & 0x02) >> 1)
#define EP2_ADC3PREAMP(fr)           ((fr[1] & 0x04) >> 2)
#define EP2_ADC4PREAMP(fr)           ((fr[1] & 0x08) >> 3)
#define EP2_TIPRING(fr)              ((fr[1] & 0x10) >> 4)
#define EP2_MICBIAS(fr)              ((fr[1] & 0x20) >> 5)
#define EP2_MICPTT(fr)               ((fr[1] & 0x40) >> 6)
#define EP2_LINEGAIN(fr)             ((fr[2] & 0x1F) >> 0)
#define EP2_MERTXATT0(fr)            ((fr[2] & 0x20) >> 5)
#define EP2_PURESIGNAL(fr)           ((fr[2] & 0x40) >> 6)
#define EP2_PENESEL(fr)              ((fr[2] & 0x80) >> 7)
#define EP2_METISDB9(fr)             ((fr[3] & 0x0F) >> 0)
#define EP2_MERTXATT1(fr)            ((fr[3] & 0x10) >> 4)
#define EP2_RX1HLATTGAIN(fr)         (37 - (fr[4] & 0x3F))
#define EP2_RX1ATT(fr)               ((fr[4] & 0x1F) >> 0x0F)
#define EP2_RX1ATTENABLE(fr)         ((fr[4] & 0x20) >> 5)
#define EP2_RX2ATT(fr)               (fr[1] & 0x1f)
#define EP2_CWREV(fr)                ((fr[2] >> 6) & 1)
#define EP2_CWSPEED(fr)              (fr[3] & 63)
#define EP2_CWMODE(fr)               ((fr[3] >> 6) & 3)
#define EP2_CWWEIGHT(fr)             (fr[4] & 127)
#define EP2_CWSPACING(fr)            ((fr[4] >> 7) & 1)
#define EP2_C25EXTBOARDDATA(fr)      ((fr[2] << 8) | fr[1])
#define EP2_RX1ADC(fr)               ((fr[1] & 0x03) >> 0)
#define EP2_RX2ADC(fr)               ((fr[1] & 0x0C) >> 2)
#define EP2_RX3ADC(fr)               ((fr[1] & 0x30) >> 4)
#define EP2_RX4ADC(fr)               ((fr[1] & 0xC0) >> 6)
#define EP2_RX5ADC(fr)               ((fr[2] & 0x03) >> 0)
#define EP2_RX6ADC(fr)               ((fr[2] & 0x0C) >> 2)
#define EP2_RX7ADC(fr)               ((fr[2] & 0x30) >> 4)
#define EP2_TXATT(fr)                ((fr[3] & 0x1f))
#define EP2_CWINT(fr)                (fr[1] & 1)
#define EP2_CWDELAY(fr)              (frame[3])
#define EP2_SIDETONEVOLUME(fr)       (fr[2])
#define EP2_CWHANG(fr)               ((fr[1] << 2) | (fr[2] & 3))
#define EP2_SIDETONEFREQ(fr)         ((fr[3] << 4) | (fr[4] & 255))

void ep2_handler(uint8_t *frame);

#endif /* HPSDR_EP2_H_ */
