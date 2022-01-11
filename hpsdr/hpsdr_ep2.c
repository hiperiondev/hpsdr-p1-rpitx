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

#include <hpsdr_protocol.h>
#include <stdint.h>
#include <math.h>

#include "hpsdr_debug.h"
#include "hpsdr_definitions.h"
#include "hpsdr_functions.h"
#include "hpsdr_ep2.h"
#include "hpsdr_main.h"

// floating-point represents of tx att, rx att, and rx preamp settings
double txatt_dbl = 1.0;
double rxatt_dbl[4] = { 1.0, 1.0, 1.0, 1.0 };  // this reflects both att and preamp
double txdrv_dbl = 0.99;

struct protocol_t settings = {
                     .AlexTXrel = -1,
                     .alexRXout = -1,
                     .alexRXant = -1,
                         .MicTS = -1,
                        .duplex = -1,
                     .receivers = -1,
                          .rate = -1,
                        .preamp = -1,
                      .LTdither = -1,
                      .LTrandom = -1,
                         .ref10 = -1,
                        .src122 = -1,
                      .PMconfig = -1,
                        .MicSrc = -1,
                       .txdrive = 255,
                         .txatt = 0,
               .sidetone_volume = -1,
                   .cw_internal = -1,
                        .rx_att = { -1, -1 },
                      .rx1_attE = -1,
                     .rx_preamp = { -1, -1, -1, -1 },
                     .MerTxATT0 = -1,
                     .MerTxATT1 = -1,
                      .MetisDB9 = -1,
                       .PeneSel = -1,
                    .PureSignal = -1,
                      .LineGain = -1,
                        .MicPTT = -1,
                      .tip_ring = -1,
                       .MicBias = -1,
                           .ptt = 0,
                       .AlexAtt = -1,
                    .TX_class_E = -1,
          .OpenCollectorOutputs = -1,
                       .tx_freq = -1,
                       .rx_freq = { -1, -1, -1, -1, -1, -1, -1 },
                 .hermes_config = -1,
                      .alex_lpf = -1,
                      .alex_hpf = -1,
                   .alex_manual = -1,
                   .alex_bypass = -1,
                         .lna6m = -1,
                 .alexTRdisable = -1,
                           .vna = -1,
        .c25_ext_board_i2c_data = -1,
                        .rx_adc = { -1, -1, -1, -1, -1, -1, -1 },
                       .cw_hang = -1,
                   .cw_reversed = -1,
                      .cw_speed = -1,
                       .cw_mode = -1,
                     .cw_weight = -1,
                    .cw_spacing = -1,
                      .cw_delay = -1,
             .CommonMercuryFreq = -1,
                          .freq = -1,
};

#define chk_data(a,b,c,FUNCTION) if ((a) != b) {FUNCTION(frame, &b, a, c);}

void ep2_handler(uint8_t *frame) {
    uint16_t data;
    int rc;
    int mod;

    chk_data(EP2_PTT(frame), settings.ptt, "PTT", ep2_ptt);
    switch (frame[0]) {
    case 0:
    case 1:
        chk_data(EP2_RATE(frame)                , settings.rate                , "SampleRate"             , ep2_samplerate);
        chk_data(EP2_REF10(frame)               , settings.ref10               , "Ref10MHz"               , ep2_ref10mhz);
        chk_data(EP2_SRC122(frame)              , settings.src122              , "Source122MHz"           , ep2_src122mhz);
        chk_data(EP2_PMCONFIG(frame)            , settings.PMconfig            , "Penelope/Mercury config", ep2_pm_config);
        chk_data(EP2_MICSRC(frame)              , settings.MicSrc              , "MicSource"              , ep2_micsrc);
        chk_data(EP2_TXCLASSC(frame)            , settings.TX_class_E          , "TX CLASS-E"             , ep2_txclasse);
        chk_data(EP2_OPENCOLLECTOROUTPUTS(frame), settings.OpenCollectorOutputs, "OpenCollector"          , ep2_opencollector);
        chk_data(EP2_RECEIVERS(frame)           , settings.receivers           , "RECEIVERS"              , ep2_receivers);
        chk_data(EP2_MICTS(frame)               , settings.MicTS               , "TimeStampMic"           , ep2_timestampmic);
        chk_data(EP2_COMMONMERCURYFREQ(frame)   , settings.CommonMercuryFreq   , "Common Mercury Freq"    , ep2_commonmercuryfreq);

        mod = 0;
        rc = EP2_ALEXATT(frame);
        if (rc != settings.AlexAtt) {
            mod = 1;
            settings.AlexAtt = rc;
        }
        rc = EP2_PREAMP(frame);
        if (rc != settings.preamp) {
            mod = 1;
            settings.preamp = rc;
        }
        rc = EP2_LTDITHER(frame);
        if (rc != settings.LTdither) {
            mod = 1;
            settings.LTdither = rc;
        }
        rc = EP2_LTRANDOM(frame);
        if (rc != settings.LTrandom) {
            mod = 1;
            settings.LTrandom = rc;
        }
        if (mod)
            hpsdr_dbg_printf(1, "AlexAtt=%d Preamp=%d Dither=%d Random=%d\n", settings.AlexAtt, settings.preamp, settings.LTdither, settings.LTrandom);

        mod = 0;
        rc = EP2_ALEXRXANT(frame);
        if (rc != settings.alexRXant) {
            mod = 1;
            settings.alexRXant = rc;
        }
        rc = EP2_ALEXXRXOUT(frame);
        if (rc != settings.alexRXout) {
            mod = 1;
            settings.alexRXout = rc;
        }
        rc = EP2_ALETXREL(frame);
        if (rc != settings.AlexTXrel) {
            mod = 1;
            settings.AlexTXrel = rc;
        }
        rc = EP2_DUPLEX(frame);
        if (rc != settings.duplex) {
            mod = 1;
            settings.duplex = rc;
        }
        if (mod)
            hpsdr_dbg_printf(1, "RXout=%d RXant=%d TXrel=%d Duplex=%d\n", settings.alexRXout, settings.alexRXant, settings.AlexTXrel, settings.duplex);

        if (device_emulation == DEVICE_C25) {
            // charly25: has two 18-dB preamps that are switched with "preamp" and "dither"
            //           and two attenuators encoded in alex-att
            //           both only applies to rx1!
            rxatt_dbl[0] = pow(10.0, -0.05 * (12 * settings.AlexAtt - 18 * settings.LTdither - 18 * settings.preamp));
            rxatt_dbl[1] = 1.0;
        } else {
            // assume that it has alex attenuators in addition to the step attenuators
            rxatt_dbl[0] = pow(10.0, -0.05 * (10 * settings.AlexAtt + settings.rx_att[0]));
            rxatt_dbl[1] = 1.0;
        }
        break;

    case 2:
    case 3:
        chk_data(EP2_TXFREQ(frame), settings.tx_freq   , "TX FREQ", ep2_txfreq);
        break;

    case 4:
    case 5:
        chk_data(EP2_RXFREQ(frame), settings.rx_freq[0], "RX FREQ1", ep2_rxfreq1);
        break;

    case 6:
    case 7:
        chk_data(EP2_RXFREQ(frame), settings.rx_freq[1], "RX FREQ2", ep2_rxfreq2);
        break;

    case 8:
    case 9:
        chk_data(EP2_RXFREQ(frame), settings.rx_freq[2], "RX FREQ3", ep2_rxfreq3);
        break;

    case 10:
    case 11:
        chk_data(EP2_RXFREQ(frame), settings.rx_freq[3], "RX FREQ4", ep2_rxfreq4);
        break;

    case 12:
    case 13:
        chk_data(EP2_RXFREQ(frame), settings.rx_freq[4], "RX FREQ5", ep2_rxfreq5);
        break;

    case 14:
    case 15:
        chk_data(EP2_RXFREQ(frame), settings.rx_freq[5], "RX FREQ6", ep2_rxfreq6);
        break;

    case 16:
    case 17:
        chk_data(EP2_RXFREQ(frame), settings.rx_freq[6], "RX FREQ7", ep2_rxfreq7);
        break;

    case 18:
    case 19:
        chk_data(EP2_TXDRIVE(frame)         , settings.txdrive       , "TX DRIVE"           , ep2_txdrive);
        chk_data(EP2_HERMESCONFIG(frame)    , settings.hermes_config , "HERMES CONFIG"      , ep2_hermesconfig);
        chk_data(EP2_ALEXMANUALHPFLPF(frame), settings.alex_manual   , "ALEX manual HPF/LPF", ep2_alexmanhpflpf);
        chk_data(EP2_VNA(frame)             , settings.vna           , "VNA mode"           , ep2_vnamode);
        chk_data(EP2_ALEXHPF(frame)         , settings.alex_hpf      , "ALEX HPF"           , ep2_alexhpf);
        chk_data(EP2_ALEXBYPASS(frame)      , settings.alex_bypass   , "ALEX Bypass HPFs"   , ep2_alexbyphpfs);
        chk_data(EP2_LNA6M(frame)           , settings.lna6m         , "ALEX 6m LNA"        , ep2_alex6mlna);
        chk_data(EP2_ALEXTRDISABLE(frame)   , settings.alexTRdisable , "ALEX T/R disable"   , ep2_alextrdis);
        chk_data(EP2_ALEXLPF(frame)         , settings.alex_lpf      , "ALEX LPF", ep2_alexlpf);
        // reset tx level. leave a little head-room for noise
        txdrv_dbl = (double) settings.txdrive * 0.00390625;  // div. by. 256
        break;

    case 20:
    case 21:
        chk_data(EP2_ADC1PREAMP(frame), settings.rx_preamp[0], "ADC1 preamp"        , ep2_adc1preamp);
        chk_data(EP2_ADC2PREAMP(frame), settings.rx_preamp[1], "ADC2 preamp"        , ep2_adc2preamp);
        chk_data(EP2_ADC3PREAMP(frame), settings.rx_preamp[2], "ADC3 preamp"        , ep2_adc3preamp);
        chk_data(EP2_ADC4PREAMP(frame), settings.rx_preamp[3], "ADC4 preamp"        , ep2_adc4preamp);
        chk_data(EP2_TIPRING(frame)   , settings.tip_ring    , "TIP/Ring"           , ep2_tipring);
        chk_data(EP2_MICBIAS(frame)   , settings.MicBias     , "MicBias"            , ep2_micbias);
        chk_data(EP2_MICPTT(frame)    , settings.MicPTT      , "MicPTT"             , ep2_micptt);
        chk_data(EP2_LINEGAIN(frame)  , settings.LineGain    , "LineGain"           , ep2_linegain);
        chk_data(EP2_MERTXATT0(frame) , settings.MerTxATT0   , "Mercury Att on TX/0", ep2_mercuryattontx0);
        chk_data(EP2_PURESIGNAL(frame), settings.PureSignal  , "PureSignal"         , ep2_puresignal);
        chk_data(EP2_PENESEL(frame)   , settings.PeneSel     , "PenelopeSelect"     , ep2_penelopeselect);
        chk_data(EP2_METISDB9(frame)  , settings.MetisDB9    , "MetisDB9"           , ep2_metisdb9);
        chk_data(EP2_MERTXATT1(frame) , settings.MerTxATT1   , "Mercury Att on TX/1", ep2_mercuryattontx1);

        if (frame[4] & 0x40) {
            // some firmware/emulators use bit6 to indicate a 6-bit format
            // for a combined attenuator/preamplifier with the AD9866 chip.
            // the value is between 0 and 60 and formally corresponds to
            // to an rx gain of -12 to +48 dB. however, we set here that
            // a value of +16 (that is, 28 on the 0-60 scale) corresponds to
            // "zero attenuation"
            chk_data(EP2_RX1HLATTGAIN(frame), settings.rx_att[0], "RX1 HL ATT/GAIN", ep2_rx1hlattgain);
        } else {
            chk_data(EP2_RX1ATT(frame)      , settings.rx_att[0], "RX1 ATT"        , ep2_rx1att);
            chk_data(EP2_RX1ATTENABLE(frame), settings.rx1_attE , "RX1 ATT enable" , ep2_rx1attenable);
            //
            // some hardware emulates "switching off att and preamp" by setting ATT
            // to 20 dB, because the preamp cannot be switched.
            // if (!rx1_attE) rx_att[0]=20;
        }
        if (device_emulation != DEVICE_C25) {
            // set rx amplification factors. no switchable preamps available normally.
            rxatt_dbl[0] = pow(10.0, -0.05 * (10 * settings.AlexAtt + settings.rx_att[0]));
            rxatt_dbl[1] = pow(10.0, -0.05 * (settings.rx_att[1]));
            rxatt_dbl[2] = 1.0;
            rxatt_dbl[3] = 1.0;
        }
        break;

    case 22:
    case 23:
        chk_data(EP2_RX2ATT(frame)   , settings.rx_att[1]  , "RX2 ATT"   , ep2_rx2att);
        chk_data(EP2_CWREV(frame)    , settings.cw_reversed, "CW REV"    , ep2_cwrev);
        chk_data(EP2_CWSPEED(frame)  , settings.cw_speed   , "CW SPEED"  , ep2_cwspeed);
        chk_data(EP2_CWMODE(frame)   , settings.cw_mode    , "CW MODE"   , ep2_cw_mode);
        chk_data(EP2_CWWEIGHT(frame) , settings.cw_weight  , "CW WEIGHT" , ep2_cwweight);
        chk_data(EP2_CWSPACING(frame), settings.cw_spacing , "CW SPACING", ep2_cwspacing);

        // set rx amplification factors.
        rxatt_dbl[1] = pow(10.0, -0.05 * (settings.rx_att[1]));
        break;

    case 24:
    case 25:
        data = frame[1];
        data |= frame[2] << 8;
        chk_data(EP2_C25EXTBOARDDATA(frame), settings.c25_ext_board_i2c_data, "C25 EXT BOARD DATA", ep2_c25extboarddata);
        break;

    case 28:
    case 29:
        chk_data(EP2_RX1ADC(frame), settings.rx_adc[0], "RX1 ADC", ep2_rx1adc);
        chk_data(EP2_RX2ADC(frame), settings.rx_adc[1], "RX2 ADC", ep2_rx2adc);
        chk_data(EP2_RX3ADC(frame), settings.rx_adc[2], "RX3 ADC", ep2_rx3adc);
        chk_data(EP2_RX4ADC(frame), settings.rx_adc[3], "RX4 ADC", ep2_rx4adc);
        chk_data(EP2_RX5ADC(frame), settings.rx_adc[4], "RX5 ADC", ep2_rx5adc);
        chk_data(EP2_RX6ADC(frame), settings.rx_adc[5], "RX6 ADC", ep2_rx6adc);
        chk_data(EP2_RX7ADC(frame), settings.rx_adc[6], "RX7 ADC", ep2_rx7adc);
        chk_data(EP2_TXATT(frame) , settings.txatt    , "TX ATT" , ep2_txatt);
        txatt_dbl = pow(10.0, -0.05 * (double) settings.txatt);
        if (device_emulation == DEVICE_C25) {
            // redpitaya: hard-wired adc settings.
            settings.rx_adc[0] = 0;
            settings.rx_adc[1] = 1;
            settings.rx_adc[2] = 1;
        }
        break;

    case 30:
    case 31:
        chk_data(EP2_CWINT(frame)         , settings.cw_internal    , "CW INT"          , ep2_cwint);
        chk_data(EP2_SIDETONEVOLUME(frame), settings.sidetone_volume, "SIDE TONE VOLUME", ep2_sidetonevolume);
        chk_data(EP2_CWDELAY(frame)       , settings.cw_delay       , "CW DELAY"        , ep2_cwdelay);
        settings.cw_delay = frame[3];
        break;

    case 32:
    case 33:
        chk_data(EP2_CWHANG(frame)      , settings.cw_hang, "CW HANG"       , ep2_cwhang);
        chk_data(EP2_SIDETONEFREQ(frame), settings.freq   , "SIDE TONE FREQ", ep2_sidetonefreq);
        break;
    }
}
