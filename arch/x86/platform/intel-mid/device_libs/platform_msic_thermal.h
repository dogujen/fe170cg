/*
 * platform_msic_thermal.h: msic_thermal platform data header file
 *
 * (C) Copyright 2008 Intel Corporation
 * Author:
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */
#ifndef _PLATFORM_MSIC_THERMAL_H_
#define _PLATFORM_MSIC_THERMAL_H_

#define MSIC_THERM_DEV_NAME "msic_thermal"

extern void __init *msic_thermal_platform_data(void *info)
			__attribute__((weak));
enum {
	mfld_thermal,
	ctp_thermal,
	lex_thermal,
	vb_thermal,
};

enum {
		fe170cg_thermal,
		fe170c_thermal,
};
#endif
