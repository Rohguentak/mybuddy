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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <sys/queue.h>
#include <pthread.h>

#include "config.h"

/**
 * This file contains a checker implementation. HOWEVER, YOU DON'T NEED TO 
 * SEE THE INTERNAL IMPLEMENTATION JUST LIKE THE GENERATORS AND COUNTER IN
 * THE PREVIOUS PA3.
 */

static unsigned int __id = 0;
struct op {
	unsigned int id;	/* Alloc operation id */
	unsigned int order;
	unsigned int page;
	STAILQ_ENTRY(op) list;
};
static STAILQ_HEAD(tailhead, op) ops;
pthread_mutex_t ops_lock = PTHREAD_MUTEX_INITIALIZER;

int mark_alloc_pages(const unsigned int page, const unsigned int order)
{
	struct op *op = NULL;

	pthread_mutex_lock(&ops_lock);
	STAILQ_FOREACH(op, &ops, list) {
		if (op->page <= page && page < op->page + (1 << op->order)) {
			fprintf(stderr, "ERROR: Page %u was already allocated "
					"by operation id=%u\n", page, op->id);
			pthread_mutex_unlock(&ops_lock);
			return -EINVAL;
		}
	}

	op = malloc(sizeof(*op));
	assert(op);

	op->id = __id++;
	op->page = page;
	op->order = order;
	STAILQ_INSERT_TAIL(&ops, op, list);
	pthread_mutex_unlock(&ops_lock);

	LOG(" Allocate order-%u page(s) from %u (id=%u)\n", order, page, op->id);
	return 0;
}

int clear_alloc_pages(const unsigned int id, unsigned int *page, unsigned int *order)
{
	struct op *op;

	pthread_mutex_lock(&ops_lock);
	STAILQ_FOREACH(op, &ops, list) {
		if (id == op->id) {
			STAILQ_REMOVE(&ops, op, op, list);
			LOG(" Free order-%u page(s) from %u (id=%u)\n",
					op->order, op->page, op->id);
			*page = op->page;
			*order = op->order;
			pthread_mutex_unlock(&ops_lock);

			free(op);
			return 0;
		}
	}
	pthread_mutex_unlock(&ops_lock);

	fprintf(stderr, "ERROR: No alloc operation with id=%u\n", id);
	return -ENOENT;
}

void list_alloc_pages(void)
{
	struct op *op;

	printf("%8s  %8s  %2s\n", "alloc id", "page", "order");
	printf("---------------------------\n");
	pthread_mutex_lock(&ops_lock);
	STAILQ_FOREACH(op, &ops, list) {
		printf(" %6u:  %#8x  %2u\n", op->id, op->page, op->order);
	}
	pthread_mutex_unlock(&ops_lock);
}

int init_checker(void)
{
	STAILQ_INIT(&ops);
	return 0;
}

void fini_checker(void)
{
	struct op *op;
	while (!STAILQ_EMPTY(&ops)) {
		op = STAILQ_FIRST(&ops);
		STAILQ_REMOVE(&ops, op, op, list);
		free(op);
	}
}
