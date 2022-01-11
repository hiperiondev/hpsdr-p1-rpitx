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

#ifndef HPSDR_OLDPROTOCOL_H_
#define HPSDR_OLDPROTOCOL_H_

// These variables store the state of the "old protocol" SDR.
// When every they are changed, this is reported.
struct protocol_t {
     int AlexTXrel;               //
     int alexRXout;               //
     int alexRXant;               //
     int MicTS;                   //
     int duplex;                  //
     int receivers;               //
     int rate;                    //
     int preamp;                  //
     int LTdither;                //
     int LTrandom;                //
     int ref10;                   //
     int src122;                  //
     int PMconfig;                //
     int MicSrc;                  //
     int txdrive;                 //
     int txatt;                   //
     int sidetone_volume;         //
     int cw_internal;             //
     int rx_att[2];               //
     int rx1_attE;                //
     int rx_preamp[4];            //
     int MerTxATT0;               //
     int MerTxATT1;               //
     int MetisDB9;                //
     int PeneSel;                 //
     int PureSignal;              //
     int LineGain;                //
     int MicPTT;                  //
     int tip_ring;                //
     int MicBias;                 //
     int ptt;                     //
     int AlexAtt;                 //
     int TX_class_E;              //
     int OpenCollectorOutputs;    //
    long tx_freq;                 //
    long rx_freq[7];              //
     int hermes_config;           //
     int alex_lpf;                //
     int alex_hpf;                //
     int alex_manual;             //
     int alex_bypass;             //
     int lna6m;                   //
     int alexTRdisable;           //
     int vna;                     //
     int c25_ext_board_i2c_data;  //
     int rx_adc[7];               //
     int cw_hang;                 //
     int cw_reversed;             //
     int cw_speed;                //
     int cw_mode;                 //
     int cw_weight;               //
     int cw_spacing;              //
     int cw_delay;                //
     int CommonMercuryFreq;       //
     int freq;                    //
};

extern struct protocol_t settings;
extern double txatt_dbl;
extern double rxatt_dbl[4];
extern double txdrv_dbl;

#endif
