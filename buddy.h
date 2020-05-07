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
#ifndef __BUDDY_H__
#define __BUDDY_H__

int alloc_pages(unsigned int *page, unsigned int order);
int free_pages(unsigned int page, unsigned int order);

double get_unusable_index(unsigned int order);
void print_free_pages(const unsigned int order);

int init_buddy(unsigned int nr_pages_in_order);

void fini_buddy(void);
#endif
