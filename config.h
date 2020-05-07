/**********************************************************************
 * Copyright (c) 2018
 *	Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/
#ifndef _CONFIG_H_

#define true 1
#define false 0
typedef int bool;

#define DEFAULT_NR_PAGES_IN_ORDER	12
#define NR_ORDERS (MAX_ORDER + 1)

extern int verbose;

#define PRINTF(...) \
	if (verbose > 0) fprintf(stderr, __VA_ARGS__)
#define LOG(...) \
	if (verbose > 1) fprintf(stderr, __VA_ARGS__)
#endif
