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

#ifndef HPSDR_CONFIG2_H_
#define HPSDR_CONFIG2_H_

int hpsdr_config_init(char *filename);
int hpsdr_config_get_band(double freq);

#endif /* HPSDR_CONFIG_H_ */
