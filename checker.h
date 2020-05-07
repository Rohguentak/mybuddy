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
#ifndef _CHECKER_H_

int mark_alloc_pages(const unsigned int page, const unsigned int order);
int clear_alloc_pages(const unsigned int id, unsigned int *page, unsigned int *order);
void list_alloc_pages(void);

int init_checker(void);
void fini_checker(void);
#endif
