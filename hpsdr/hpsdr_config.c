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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hpsdr_debug.h"
#include "hpsdr_main.h"
#include "confini.h"

char *devices[9] = {
        "metis",
        "hermes",
        "griffin",
        "angelia",
        "orion",
        "hermeslite",
        "hermeslite2",
        "orion2",
        "c25"
};

int devices_id[9] = {
        0,
        1,
        2,
        4,
        5,
        6,
        1006,
        10,
        100
};

#define CLEAN_MALLOC(DEST, SIZE, RETVAL) \
    free(DEST); \
    DEST = malloc(SIZE); \
    if (!DEST) { \
    	hpsdr_dbg_printf(0, "malloc() failed\n"); \
        return RETVAL; \
    }

#define hpsdh_config_format \
    ((IniFormat) { \
        .delimiter_symbol = INI_EQUALS, \
        .case_sensitive = false, \
        .semicolon_marker = INI_IGNORE, \
        .hash_marker = INI_IGNORE, \
        .section_paths = INI_ABSOLUTE_AND_RELATIVE, \
        .multiline_nodes = INI_MULTILINE_EVERYWHERE, \
        .no_single_quotes = false, \
        .no_double_quotes = false, \
        .no_spaces_in_names = false, \
        .implicit_is_not_empty = true, \
        .do_not_collapse_values = false, \
        .preserve_empty_quotes = false, \
        .disabled_after_space = false, \
        .disabled_can_be_implicit = true \
    })

/*  If `dest_str` is non-`NULL` free it, then `strdup(disp->value)` into it  */
int set_new_string(char **const dest_str, IniDispatch *const disp) {
    disp->d_len = ini_string_parse(disp->value, disp->format);
    CLEAN_MALLOC(*dest_str, disp->d_len, 1);
    memcpy(*dest_str, disp->value, disp->d_len + 1);
    return 0;
}

/*  If `dest_arr` is non-`NULL` free it, then parse `disp->value` into it as an array of strings  */
int set_new_strarray(char ***const dest_arr, size_t *const dest_len, const IniDispatch *const disp, const char delimiter) {
    *dest_len = ini_array_get_length(disp->value, delimiter, disp->format);
    CLEAN_MALLOC(*dest_arr, *dest_len * sizeof(char*) + disp->v_len + 1, 1);
    size_t idx = 0;
    char *token, *remnant = ((char*) *dest_arr) + (*dest_len + 1) * sizeof(char*);
    memcpy(remnant, disp->value == INI_GLOBAL_IMPLICIT_VALUE && !INI_GLOBAL_IMPLICIT_VALUE ? "" : disp->value, disp->v_len + 1);
    while ((token = ini_array_release(&remnant, delimiter, disp->format))) {
        ini_string_parse(token, disp->format);
        (*dest_arr)[idx++] = token;
    }
    return 0;
}

/*  If `dest_arr` is non-`NULL` free it, then parse `disp->value` into it as an array of integers  */
int set_new_intarray(int **const dest_arr, size_t *const dest_len, const IniDispatch *const disp, const char delimiter) {
    *dest_len = ini_array_get_length(disp->value, delimiter, disp->format);
    CLEAN_MALLOC(*dest_arr, *dest_len * sizeof(int), 1);
    const char *shifted = disp->value;
    size_t idx = 0;
    while (shifted) {
        (*dest_arr)[idx++] = ini_get_int(shifted);
        ini_array_shift(&shifted, delimiter, disp->format);
    }
    return 0;
}

static int populate_config(IniDispatch *const disp, void *const v_confs) {
#define confs ((struct Configs_T *) v_confs)

    // defaults
    confs->global.iqburst = 1000;
    confs->global.emulation_id = 6;

    if (disp->type == INI_KEY) {
        if (ini_array_match("select_bands", disp->append_to, '.', disp->format)) {
            if (ini_string_match_si("bands", disp->data, disp->format)) {
                const size_t arrlen = ini_array_get_length(disp->value, ',', disp->format);
                char *remaining = disp->value;
                if (arrlen > 0) {
                    /*  array of strings `clients.data[0]` exists  */
                    IniDispatch *dspclone = malloc(sizeof(IniDispatch));
                    if (!dspclone) {
                        hpsdr_dbg_printf(0, "malloc() failed\n");
                        return 1;
                    }
                    memcpy(dspclone, disp, sizeof(IniDispatch));
                    dspclone->value = ini_array_release(&remaining, ',', disp->format);
                    dspclone->v_len = strlen(dspclone->value);
                    set_new_strarray(&confs->filters.band_str, &confs->filters.band_str_length, dspclone, ':');
                    if (arrlen > 1) {
                        /*  array of integers `clients.data[1]` exists  */
                        dspclone->value = ini_array_release(&remaining, ',', disp->format);
                        dspclone->v_len = strlen(dspclone->value);
                        set_new_intarray(&confs->filters.band_start, &confs->filters.band_start_length, dspclone, ':');
                        set_new_intarray(&confs->filters.band_end, &confs->filters.band_end_length, dspclone, ':');
                        set_new_intarray(&confs->filters.gpio_lpf, &confs->filters.gpio_lpf_length, dspclone, ':');
                        set_new_intarray(&confs->filters.gpio_hpf, &confs->filters.gpio_hpf_length, dspclone, ':');
                    }
                    free(dspclone);
                }
            }
            if (ini_string_match_si("enabled", disp->data, disp->format)) {
                confs->filters.enabled = ini_get_bool(disp->value, 0);
            }
            if (ini_string_match_si("type", disp->data, disp->format)) {
                set_new_string(&confs->filters.type, disp);
            }
            if (ini_string_match_si("gpios", disp->data, disp->format)) {
                set_new_intarray(&confs->filters.gpio, &confs->filters.gpio_length, disp, ',');
            }
        } else if (ini_array_match("global", disp->append_to, '.', disp->format)) {
            if (ini_string_match_si("debug", disp->data, disp->format)) {
                confs->global.debug = ini_get_bool(disp->value, 0);
            }
            if (ini_string_match_si("iqburst", disp->data, disp->format)) {
                confs->global.iqburst = ini_get_int(disp->value);
            }
            if (ini_string_match_si("emulation", disp->data, disp->format)) {
                set_new_string(&confs->global.emulation, disp);
            }
        }
    }

    if (confs->global.emulation) {
        int l;
        for (int n = 0; n < 9; n++) {
            for (l = 0; l < strlen(confs->global.emulation); l++)
                confs->global.emulation[l] = tolower(confs->global.emulation[l]);

            if (!strcmp(confs->global.emulation, devices[n])) {
                confs->global.emulation_id = devices_id[n];
                break;
            }
        }
    }

    return 0;
#undef confs
}

int hpsdr_config_init(char *filename) {
    confs = calloc(1, sizeof(struct Configs_T));

    if (!confs) {
        hpsdr_dbg_printf(0, "allocate confs failed\n");
        return 1;
    }
    if (load_ini_path(
            filename,
            hpsdh_config_format,
            NULL,
            populate_config,
            confs
            )
        ) {
        hpsdr_dbg_printf(0, "Sorry, something went wrong :-(\n");
        return 1;
    }

#define PRINT_CONF_ARRAY_WITHLABEL(PATH, LABEL, FORMAT) \
        for (size_t idx = 0; idx < confs->PATH##_length; idx++) { \
            if (confs->PATH) { hpsdr_dbg_printf(0, #LABEL "[%zu] -> " FORMAT "\n", idx, confs->PATH[idx]); } \
        }
#define PRINT_CONF_ARRAY(PATH, FORMAT) \
        PRINT_CONF_ARRAY_WITHLABEL(PATH, PATH, FORMAT)
#define PRINT_CONF_SIMPLEVAL(PATH, FORMAT) \
        if (confs->PATH) { hpsdr_dbg_printf(0, #PATH " -> " FORMAT "\n", confs->PATH); }

    PRINT_CONF_SIMPLEVAL(global.debug, "%d");
    PRINT_CONF_SIMPLEVAL(global.iqburst, "%d");
    PRINT_CONF_SIMPLEVAL(global.emulation, "%s");
    hpsdr_dbg_printf(0, "global.emulation_id" " -> " "%d" "\n", confs->global.emulation_id);

#undef PRINT_CONF_SIMPLEVAL
#undef PRINT_CONF_ARRAY
#undef PRINT_CONF_ARRAY_WITHLABEL

    return 0;
}

int hpsdr_config_deinit(void) {
    free(confs->global.emulation);
    free(confs->filters.type);
    free(confs->filters.gpio);
    free(confs->filters.band_str);
    free(confs);
    return 0;
}
