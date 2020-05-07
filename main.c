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
#include <ctype.h>
#include <getopt.h>

#include "config.h"
#include "buddy.h"
#include "checker.h"

int verbose = 1;
static int phase = 0;
static unsigned int nr_pages_in_order = DEFAULT_NR_PAGES_IN_ORDER;

static int __parse_operation(char *line, char *argv[])
{
	int argc = 0;
	char *c = line;
	char *start;
	bool str = false;

	while (c && *c != '\r' && *c != '\n') {
		if (*c == '\t' || *c == ' ') {
			*c = '\0';
			c++;
			if (str) argv[argc++] = start;
			str = false;
			continue;
		}

		if (!str) {
			start = c;
		}
		str = true;
		c++;
	}
	if (c && (*c == '\r' || *c == '\n') && str) {
		*c = '\0';
		argv[argc++] = start;
	}
	return argc;
}

static bool __check_operation(const int expect, const int argc, const char *name)
{
	if (expect != argc) {
		fprintf(stderr, "Wrong %s operation (argc %d != %d)\n",
				name, expect, argc);
		return true;
	}
	return false;
}

/**
 * Process CLI inputs
 */
static void __process_operations(void)
{
	bool keep_running = true;
	int argc;
	char *argv[60] = {NULL};
	char line[120];
	unsigned int order;
	unsigned int page;

	PRINTF(">> ");
	while (keep_running && fgets(line, sizeof(line), stdin)) {
		argc = __parse_operation(line, argv);
		if (argc <= 0) {
			break;
		}
		if (strlen(argv[0]) != 1) {
			printf("Unknown operation '%s'\n", argv[0]);
			break;
		}
		switch(tolower(argv[0][0])) {
		case '#': /* comment */
			break;
		case '-': /* Draw a horizontal line */
			fprintf(stderr, "%d ---------------\n", phase++);
			break;
		case 'a': /* allocation */
			if (__check_operation(2, argc, "alloc")) continue;
			order = atoi(argv[1]);
			if (alloc_pages(&page, order) == 0) {
				mark_alloc_pages(page, order);
			} else {
				fprintf(stderr, "Cannot allocate order-%d page\n", order);
			}
			break;
		case 'd': /* free / deallocation */
		case 'f':
			if (__check_operation(2, argc, "free")) continue;
			if (clear_alloc_pages(atoi(argv[1]), &page, &order) == 0) {
				free_pages(page, order);
			}
			break;
		case 's': /* show the chunk list */
			if (__check_operation(2, argc, "print")) continue;
			print_free_pages(atoi(argv[1]));
			break;
		case 'p': /* print chunk lists */
			if (__check_operation(1, argc, "print all")) continue;
			order = NR_ORDERS - 1;
			do {
				print_free_pages(order);
			} while (order-- != 0);
			fprintf(stderr, "%d ---------------\n", phase++);
			break;
		case 'l': /* list operations */
			if (__check_operation(1, argc, "list")) continue;
			list_alloc_pages();
			break;
		case 'u': /* check unusable index */
			if (__check_operation(2, argc, "unusable index")) continue;
			order = atoi(argv[1]);
			PRINTF("Unusable index for order %d is ", order);
			fprintf(stderr, "    %.4f\n", get_unusable_index(order));
			break;
		case 'h': /* Help!! */
			printf("  a <order> : allocate a 2^@order chunk\n");
			printf("  d <id>    : free the chunk whose allocation id is @id\n");
			printf("  f <id>    : same to 'd'\n");
			printf("  l         : show the allocated results\n");
			printf("  p         : print all free chunks\n");
			printf("  s <order> : print order-@order free chunks\n");
			printf("  u <order> : print the unusable index of order-@order\n");
			break;
		case 'q': /* I'm done. Good bye*/
			keep_running = false;
			break;
		case '0':
			verbose = 0;
			break;
		case '1':
			verbose = 1;
			break;
		case '2':
			verbose = 2;
			break;
		default:
			fprintf(stderr, "Unknown operation '%c'\n", argv[0][0]);
		}
		PRINTF(">> ");
	}
}


static int __parse_options(int argc, char *argv[]) {
	char opt;
	while ((opt = getopt(argc, argv, "h?vqn:a:")) != -1) {
		switch(opt) {
		case 'n': /* set the number of pages to manage */
			nr_pages_in_order = atol(optarg);
			if (nr_pages_in_order < MAX_ORDER) {
				fprintf(stderr, "nr_pages_in_order should be >="
						"or equal to MAX_ORDER which is currently %d\n",
						MAX_ORDER);
				return -EINVAL;
			}
			break;
		case 'v':
			verbose++;
			break;
		case 'q':
			verbose--;
			break;
		case 'h':
		case '?':
		default:
			fprintf(stderr, "Usage: %s\n", argv[0]);
			fprintf(stderr, "  -v : verbose\n");
			fprintf(stderr, "  -q : be quiet\n");
			fprintf(stderr, "  -n <nr_pages in order-n notation>\n");

			return -EBUSY;
			break;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (__parse_options(argc, argv))
		return EXIT_FAILURE;

	PRINTF("Starting the buddy system with 2^%u pages. Have fun!!\n",
			nr_pages_in_order);

	if (init_checker()) return EXIT_FAILURE;

	if (init_buddy(nr_pages_in_order))
		goto out_checker;

	__process_operations();

	fini_buddy();

out_checker:
	fini_checker();

	return EXIT_SUCCESS;
}
