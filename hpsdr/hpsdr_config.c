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

#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#include "mxml.h"
#include "mxml_mem.h"
#include "hpsdr_definitions.h"
#include "hpsdr_main.h"
#include "hpsdr_debug.h"

static int emu;
static char default_cfg[] = ""
        "<config>\n"
        "    <global>\n"
        "        <debug>     true       </debug>\n"
        "        <iqburst>   1000       </iqburst>\n"
        "        <emulation> hermeslite </emulation>\n"
        "    </global>\n"
        "\n"
        "    <filters>\n"
        "        <enabled> false </enabled>\n"
        "        <delay>   1     </delay>\n"
        "        <type>    pin   </type>\n"
        "    </filters>\n"
        "\n"
        "    <bands>\n"
        "        <total> 16 </total>\n"
        "        \n"
        "        <name0>   2200m\n"
        "            <lo>  135   </lo>\n"
        "            <hi>  138   </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name0>\n"
        "\n"
        "        <name1>   630m\n"
        "            <lo>  472   </lo>\n"
        "            <hi>  479   </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name1>\n"
        "\n"
        "        <name2>   160m\n"
        "            <lo>  1810  </lo>\n"
        "            <hi>  2000  </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name2>\n"
        "\n"
        "        <name3>   80m\n"
        "            <lo>  3500  </lo>\n"
        "            <hi>  4000  </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name3>\n"
        "\n"
        "        <name4>   60m\n"
        "            <lo>  5351  </lo>\n"
        "            <hi>  5367  </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name4>\n"
        "\n"
        "        <name5>   40m\n"
        "            <lo>  7000  </lo>\n"
        "            <hi>  7300  </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name5>\n"
        "\n"
        "        <name6>   30m\n"
        "            <lo>  10100 </lo>\n"
        "            <hi>  10150 </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name6>\n"
        "\n"
        "        <name7>   20m\n"
        "            <lo>  14000 </lo>\n"
        "            <hi>  14350 </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name7>\n"
        "\n"
        "        <name8>   17m\n"
        "            <lo>  18068 </lo>\n"
        "            <hi>  18168 </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name8>\n"
        "\n"
        "        <name9>   15m\n"
        "            <lo>  21000 </lo>\n"
        "            <hi>  21450 </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name9>\n"
        "\n"
        "        <name10>  12m\n"
        "            <lo>  24890 </lo>\n"
        "            <hi>  24990 </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name10>\n"
        "\n"
        "        <name11>  10m\n"
        "            <lo>  28000 </lo>\n"
        "            <hi>  29700 </hi>\n"
        "            <hpf> 0     </lpf>\n"
        "            <lpf> 0     </hpf>\n"
        "        </name11>\n"
        "\n"
        "        <name12>  6m\n"
        "            <lo>  50000 </lo>\n"
        "            <hi>  54000 </hi>\n"
        "            <lpf> 0     </lpf>\n"
        "            <hpf> 0     </hpf>\n"
        "        </name12>\n"
        "\n"
        "        <name13>  2m\n"
        "            <lo>  144000 </lo>\n"
        "            <hi>  148000 </hi>\n"
        "            <lpf> 0      </lpf>\n"
        "            <hpf> 0      </hpf>\n"
        "        </name13>\n"
        "\n"
        "        <name14>  1.25m\n"
        "            <lo>  220000 </lo>\n"
        "            <hi>  225000 </hi>\n"
        "            <lpf> 0      </lpf>\n"
        "            <hpf> 0      </hpf>\n"
        "        </name14>\n"
        "\n"
        "        <name15>  70cm\n"
        "            <lo>  430000 </lo>\n"
        "            <hi>  440000 </hi>\n"
        "            <lpf> 0      </lpf>\n"
        "            <hpf> 0      </hpf>\n"
        "        </name15>\n"
        "    </bands>\n"
        "\n"
        "</config>\n";

typedef enum {
    STR2INT_SUCCESS,      //
    STR2INT_OVERFLOW,     //
    STR2INT_UNDERFLOW,    //
    STR2INT_INCONVERTIBLE //
} str2int_errno;

static int devices_id[9] = {
        DEVICE_METIS,        //
        DEVICE_HERMES,       //
        DEVICE_GRIFFIN,      //
        DEVICE_ANGELIA,      //
        DEVICE_ORION,        //
        DEVICE_HERMES_LITE,  //
        DEVICE_HERMES_LITE2, //
        DEVICE_ORION2,       //
        DEVICE_C25           //
        };

static char *device_type[9] = {
        "metis",       //
        "hermes",      //
        "griffin",     //
        "angelia",     //
        "orion",       //
        "hermeslite",  //
        "hermeslite2", //
        "orion2",      //
        "c25"          //
        };

static char *filter_type[4] = {
        "pin",     //
        "binary",  //
        "pcf8575", //
        "mcp23016" //
        };

static int get_device(char *name) {
    int n;
    for (int i = 0; i < strlen(name); i++)
        name[i] = tolower(name[i]);
    for (n = 0; n < 9; n++) {

        if (strcmp(name, device_type[n]) == 0) {
            return n;
        }
    }
    return -1;
}

static int get_filter_type(char *name) {
    int n;
    for (int i = 0; i < strlen(name); i++)
        name[i] = tolower(name[i]);
    for (n = 0; n < 2; n++) {
        if (strcmp(name, filter_type[n]) == 0) {
            return n;
        }
    }
    return -1;
}

static int get_boolean(const char *string, bool *value) {
    char *t[] = { "y", "Y", "yes", "Yes", "YES", "true", "True", "TRUE", "on", "On", "ON", NULL };
    char *f[] = { "n", "N", "no", "No", "NO", "false", "False", "FALSE", "off", "Off", "OFF", NULL };
    char **p;

    for (p = t; *p; p++) {
        if (strcmp(string, *p) == 0) {
            *value = true;
            return 0;
        }
    }
    for (p = f; *p; p++) {
        if (strcmp(string, *p) == 0) {
            *value = false;
            return 0;
        }
    }
    return -1;
}

static char* ltrim(char *s) {
    while (isspace(*s))
        s++;
    return s;
}

static char* rtrim(char *s) {
    char *back;
    int len = strlen(s);

    if (len == 0)
        return (s);

    back = s + len;
    while (isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

static char* trim(char *s) {
    return rtrim(ltrim(s));
}

static str2int_errno str2int(int *out, char *s, int base) {
    char *end;
    if (s[0] == '\0' || isspace(s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    // Both checks are needed because INT_MAX == LONG_MAX is possible
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = l;
    return STR2INT_SUCCESS;
}

static void print_config(hpsdr_config_t config) {
    hpsdr_dbg_printf(0, "[CONFIGURATION]\n");
    hpsdr_dbg_printf(0, "----------------------- global --------------------------\n");
    hpsdr_dbg_printf(0, "    config.global.debug = %s\n", config.global.debug ? "true" : "false");
    hpsdr_dbg_printf(0, "  config.global.iqburst = %d\n", config.global.iqburst);
    hpsdr_dbg_printf(0, "config.global.emulation = %s\n", device_type[emu]);
    hpsdr_dbg_printf(0, "----------------------- filters -------------------------\n");
    hpsdr_dbg_printf(0, " config.filters.enabled = %s\n", config.filters.enabled ? "true" : "false");
    hpsdr_dbg_printf(0, "   config.filters.delay = %d\n", config.filters.delay);
    hpsdr_dbg_printf(0, "    config.filters.type = %s\n", filter_type[config.filters.type]);
    hpsdr_dbg_printf(0, "----------------------- bands ---------------------------\n");
    for (int n = 0; n < config.bands_len; n++) {
        hpsdr_dbg_printf(0, "--------[%02d]--------\n", n);
        hpsdr_dbg_printf(0, "name: %s\n", config.bands[n].name);
        hpsdr_dbg_printf(0, "  lo: %d kHz\n", config.bands[n].lo);
        hpsdr_dbg_printf(0, "  hi: %d kHz\n", config.bands[n].hi);
        hpsdr_dbg_printf(0, " lpf: %d\n", config.bands[n].lpf);
        hpsdr_dbg_printf(0, " hpf: %d\n", config.bands[n].hpf);
    }
    hpsdr_dbg_printf(0, "----------------\n");

    hpsdr_dbg_printf(0, "[END CONFIGURATION]\n\n");
}

#define GET_STR(db, str)        trim(mxml_get(db, str))
#define GET_INT(out, db, str)   { int err = str2int(&out, GET_STR(db, str), 10); \
		                            if (err != STR2INT_SUCCESS) { \
		                                hpsdr_dbg_printf(0, "ERROR:"); hpsdr_dbg_printf(0, str); hpsdr_dbg_printf(0, "= %s\n", GET_STR(db, str)); return 1;}}
#define GET_BOOL(out, db, str)  { bool res; int err = get_boolean(GET_STR(db, str), &res); \
		                            if (err != 0) { \
		                                hpsdr_dbg_printf(0, "ERROR:"); hpsdr_dbg_printf(0, str); hpsdr_dbg_printf(0, "= %s\n", GET_STR(db, str)); return 1;} \
		                            out = res; }

int hpsdr_config_init(char *filename) {
    char *node, tmp[1024];
    char *data = NULL;
    size_t datalen;
    struct mxml *db;

    config.bands_len = 0;

    FILE *file = NULL;

    file = fopen(filename, "rb");
    if (file == NULL) {
        hpsdr_dbg_printf(0, "error opening file\ncreating default config file: hpsdr_p1_rpitx.cfg\n");
        file = fopen("hpsdr_p1_rpitx.cfg", "w+");
        fprintf(file, default_cfg);
        fclose(file);
        file = fopen("hpsdr_p1_rpitx.cfg", "rb");
    }

    hpsdr_dbg_printf(0, "parsing config file\n");
    getdelim(&data, &datalen, 0, file);
    db = mxml_new(data, datalen);

    // global
    hpsdr_dbg_printf(0, "reading global\n");
    GET_BOOL(config.global.debug, db, "config.global.debug");
    GET_INT(config.global.iqburst, db, "config.global.iqburst");
    emu = get_device(GET_STR(db, "config.global.emulation"));
    if (emu == -1) {
        hpsdr_dbg_printf(0, "ERROR: config.global.emulation = %s\n", GET_STR(db, "config.global.emulation"));
        return 1;
    }
    config.global.emulation = devices_id[emu];

    // filters
    hpsdr_dbg_printf(0, "reading filters\n");
    GET_BOOL(config.filters.enabled, db, "config.filters.enabled");
    GET_INT(config.filters.delay, db, "config.filters.delay");
    config.filters.type = get_filter_type(GET_STR(db, "config.filters.type"));
    if (config.filters.type == -1) {
        hpsdr_dbg_printf(0, "ERROR: config.filters.type = %s\n", GET_STR(db, "config.filters.type"));
        return 1;
    }

    // bands
    hpsdr_dbg_printf(0, "reading bands\n");
    GET_INT(config.bands_len, db, "config.bands.total");
    if(config.bands_len > MAXBANDS) {
        hpsdr_dbg_printf(0, "ERROR: too many bands. allowed: %d\n", MAXBANDS);
        return 1;
    }

    for (int n = 0; n < config.bands_len; n++) {

        sprintf(tmp, "config.bands.name%d", n);
        node = GET_STR(db, tmp);
        strcpy(config.bands[n].name, node);
        config.bands[n].name[63] = '\0';

        sprintf(tmp, "config.bands.name%d.lo", n);
        GET_INT(config.bands[n].lo, db, tmp);

        sprintf(tmp, "config.bands.name%d.hi", n);
        GET_INT(config.bands[n].hi, db, tmp);

        sprintf(tmp, "config.bands.name%d.lpf", n);
        GET_INT(config.bands[n].lpf, db, tmp);

        sprintf(tmp, "config.bands.name%d.hpf", n);
        GET_INT(config.bands[n].hpf, db, tmp);
    }

    print_config(config);

    free(db);
    free(data);
    mxml_free_all();

    return 0;
}

int hpsdr_config_get_band(double freq) {
    int freq_khz = freq / 1000;
    for (int n = 0; n < config.bands_len; n++) {
        if (freq_khz >= config.bands[n].lo && freq_khz <= config.bands[n].hi)
            return n;
    }
    return -1;
}
