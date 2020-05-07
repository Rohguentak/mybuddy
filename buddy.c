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
#include <errno.h>
#include "queue.h"
#include "config.h"
#include <stdlib.h>

/**
 * Skeleton data structures to implement the buddy system allocator
 */

/**
 * Data structure to represent an order-@order pages. To the rest of this file,
 * consecutive pages will be represented in @start:@order notation.
 * E.g., 16:3 is 8(2^3)  consecutive pages (or say order-3 page) starting from
 * page frame 16.
 */
struct chunk {
	/**
	 * TODO: Modify this structure as you need.
	 */
	TAILQ_ENTRY(chunk) next;
	unsigned int start;
	unsigned int order;
};


/**
 * Data structure to maintain order-@order free chunks.
 * NOTE that chunk_list SHOULD WORK LIKE THE QUEUE; the firstly added chunk
 * should be used first, otherwise the grading system will fail.
 */
struct chunk_list {
	/**
	 * TODO: Modify this structure as you need 
	 */
	unsigned int order;
	unsigned int numofchunkinfreelist;
	TAILQ_HEAD(chnks,chunk) freech;
};


/**
 * Data structure to realize the buddy system allocator
 */
struct buddy {
	/**
	 * TODO: Modify this example data structure as you need
	 */
	
	/**
	 * Free chunk list in the buddy system allocator.
	 *
	 * @NR_ORDERS is @MAX_ORDER + 1 (considering order-0 pages) and deifned in
	 * config.h. @MAX_ORDER is set in the Makefile. MAKE SURE your buddy
	 * implementation can handle order-0 to order-@MAX_ORDER pages.
	 */
	struct chunk_list chunks[NR_ORDERS];

	unsigned int allocated;	/* Number of pages that are allocated */
	unsigned int free;		/* Number of pages that are free */
};


/**
 * This is your buddy system allocator instance!
 */
static struct buddy buddy;


/**
 *    Your buddy system allocator should manage from order-0 to
 *  order-@MAX_ORDER. In the following example, assume your buddy system
 *  manages page 0 to 0x1F (0 -- 31, thus @nr_pages is 32) and pages from
 *  20 to 23 and 28 (0x14 - 0x17, 0x1C) are allocated by alloc_pages()
 *  by some orders.
 *  At this moment, the buddy system will split the address space into;
 *
 *      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
 * 0x00 <-------------------4-------------------------->
 * 0x10 <----2----->X  X  X  X  <-----2---->X  <0><-1-->
 *                  ^  ^  ^  ^              ^
 *                  |--|--|--|--------------|
 *                     allocated
 *
 *   Thus, the buddy system will maintain the free chunk lists like;
 *
 * Order     | Start addresses of free chunks
 * ----------------------------------------------
 * MAX_ORDER |
 *    ...    |
 *     4     | 0x00
 *     3     |
 *     2     | 0x10, 0x18
 *     1     | 0x1e
 *     0     | 0x1d
 */


/**
 * Allocate 2^@order contiguous pages.
 *
 * Description:
 *    For example, when @order=0, allocate a single page, @order=2 means
 *  to allocate 4 consecutive pages, and so forth.
 *    From the example state above, alloc_pages(2) gives 0x10 through @*page
 *  and the corresponding entry is removed from the free chunk. NOTE THAT the
 *  free chunk lists should be maintained as 'FIFO' so alloc_pages(2) returns 
 *  0x10, not 0x18. 
 *    To hanle alloc_pages(3), the order-4 chunk (0x00 -- 0x0f) should
 *  be broken into smaller chunks (say 0x00 -- 0x07, 0x08 -- 0x0f), and
 *  the LEFT BUDDY will be returned through @page whereas RIGHT BUDDY
 *  will be put into the order-3 free chunk list.
 *
 * Return:
 *   0      : On successful allocation. @*page will contain the starting
 *            page number of the allocated chunk
 *  -EINVAL : When @order < 0 or @order > MAX_ORDER
 *  -ENOMEM : When order-@order contiguous chunk is not available in the system
 */
int alloc_pages(unsigned int *page, const unsigned int order)
{
	/**
	 * Your implementation will look (but not limited to) like;
	 *
	 * Check whether a chunk is available from chunk_list of @order
	 * if (exist) {
	 *    allocate the chunk from the list; Done!
	 * } else {
	 *    Make an order-@order chunk by breaking a higher-order chunk(s)
	 *    - Find the smallest free chunk that can satisfy the request
	 *    - Break the LEFT chunk until it is small enough
	 *    - Put remainders into the free chunk list
	 *
	 *    Return the allocated chunk via @*page
	 * }
	 *
	 *----------------------------------------------------------------------
	 * Print out below message using PRINTF upon each events. Note it is
	 * possible for multiple events to be happened to handle a single
	 * alloc_pages(). Also, MAKE SURE TO USE 'PRINTF', _NOT_ printf, otherwise
	 * the grading procedure will fail.
	 *
	 * - Split an order-@x chunk starting from @page into @left and @right:
	 *   PRINTF("SPLIT 0x%x:%u -> 0x%x:%u + 0x%x:%u\n",
	 *			page, x, left, x-1, right, x-1);
	 *
	 * - Put an order-@x chunk starting from @page into the free list:
	 *   PRINTF("PUT   0x%x:%u\n", page, x);
	 *
	 * - Allocate an order-@x chunk starting from @page for serving the request:
	 *   PRINTF("ALLOC 0x%x:%x\n", page, x);
	 *
	 * Example: A order-4 chunk starting from 0 is split into 0:3 and 8:3,
	 * and 0:3 is split again to 0:2 and 4:2 to serve an order-2 allocation.
	 * And then 0:2 is allocated:
	 *
	 * SPLIT 0x0:4 -> 0x0:3 + 0x8:3
	 * PUT   0x8:3
	 * SPLIT 0x0:3 -> 0x0:2 + 0x4:2
	 * PUT   0x4:2
	 * ALLOC 0x0:2
	 *
	 *       OR
	 *
	 * SPLIT 0x0:4 -> 0x0:3 + 0x8:3
	 * SPLIT 0x0:3 -> 0x0:2 + 0x4:2
	 * PUT   0x8:3
	 * PUT   0x4:2
	 * ALLOC 0x0:2
	 *
	 *       OR
	 *
	 * SPLIT 0x0:4 -> 0x0:3 + 0x8:3
	 * SPLIT 0x0:3 -> 0x0:2 + 0x4:2
	 * PUT   0x4:2
	 * PUT   0x8:3
	 * ALLOC 0x0:2
	 *----------------------------------------------------------------------
	 */
	
	
	int cntforsplit =1; 
	int b =0;
	int hasbigchunk=0; 
	if(order > NR_ORDERS-1||order < 0){
		return EINVAL;
	}
	if (buddy.chunks[order].numofchunkinfreelist > 0) { 
		struct chunk * chunkforrmv = buddy.chunks[order].freech.tqh_first;
	    TAILQ_REMOVE(&(buddy.chunks[order].freech),chunkforrmv,next);
		(buddy.chunks[order].numofchunkinfreelist)--;
		*page = chunkforrmv->start;
		free(chunkforrmv);
	}
	else {
		if(order == NR_ORDERS-1){
			return ENOMEM;
		}
		else if(order +1 == NR_ORDERS-1){ 
			if(buddy.chunks[NR_ORDERS-1].numofchunkinfreelist>0){
				struct chunk * chunkforrmv = buddy.chunks[order+1].freech.tqh_first;
				struct chunk * chunkforins = malloc(sizeof(struct chunk));
				hasbigchunk = NR_ORDERS-1;
				TAILQ_REMOVE(&(buddy.chunks[hasbigchunk].freech),chunkforrmv,next);*page = chunkforrmv->start;
				(buddy.chunks[hasbigchunk].numofchunkinfreelist)--;
				chunkforins->start = chunkforrmv->start + (1<<chunkforrmv->order)/2;
				chunkforins->order = chunkforrmv->order-1; 
				TAILQ_INSERT_TAIL(&(buddy.chunks[order].freech),chunkforins,next);
				(buddy.chunks[order].numofchunkinfreelist)++;
				free(chunkforrmv);
			}
			else {
				return ENOMEM;
			}
		}
		else{
			for(b =order + 1;b<NR_ORDERS-1;b++){
				if(buddy.chunks[b].numofchunkinfreelist>0){
					break;
				}
			}
			if(buddy.free ==0){
				return ENOMEM;
			}
			else{
				hasbigchunk = b;
				int i = hasbigchunk; 
				struct chunk * chunkforrmv = buddy.chunks[hasbigchunk].freech.tqh_first;
				TAILQ_REMOVE(&(buddy.chunks[hasbigchunk].freech),chunkforrmv,next);
				*page = chunkforrmv->start;
				(buddy.chunks[hasbigchunk].numofchunkinfreelist)--;
				while(1) {
			 		if(cntforsplit == hasbigchunk - order+1){
						 break;
					}
					struct chunk * chunkforins = malloc(sizeof(struct chunk));
					chunkforins->start = chunkforrmv->start + (1<<(i-1));
					chunkforins->order = i-1;
					TAILQ_INSERT_TAIL(&(buddy.chunks[i-1].freech),chunkforins,next);
					(buddy.chunks[i-1].numofchunkinfreelist)++;
					i--;
					cntforsplit++;
				}
				
				free(chunkforrmv);
			}
		}
	}
	buddy.allocated += (1 << order);
	buddy.free -= (1 << order);
	return 0;
}


/**
 * Free @page which are contiguous for 2^@order pages
 *
 * Description:
 *    Assume @page was allocated by alloc_pages(@order) above. 
 *  WARNING: When handling free chunks, put them into the free chunk list
 *  carefully so that free chunk lists work in FIFO.
 */
int Buddyornot(int rgt_or_lft,int order,int page){
	int rvale= 0; int abc = buddy.chunks[order].numofchunkinfreelist;
	
	if(order == NR_ORDERS -1){
		return 0;
	}
	else if(rgt_or_lft == 0 && buddy.chunks[order].numofchunkinfreelist !=0){
		for(int a =0; a<abc;a++){
			
			struct chunk * forins = malloc(sizeof(struct chunk));
			struct chunk * chforfre = buddy.chunks[order].freech.tqh_first;
			TAILQ_REMOVE(&(buddy.chunks[order].freech),chforfre,next);
			buddy.chunks[order].numofchunkinfreelist =buddy.chunks[order].numofchunkinfreelist-1;
			if(chforfre->start == page +(1<<order)){
				rvale =1;
				free(forins);
				free(chforfre);
			}else{
				forins->start = chforfre->start; forins->order = chforfre->order;
				TAILQ_INSERT_TAIL(&(buddy.chunks[order].freech),forins,next);
				buddy.chunks[order].numofchunkinfreelist =buddy.chunks[order].numofchunkinfreelist+1;
				free(chforfre);
			}
		}
	}
	else if(rgt_or_lft != 0 && buddy.chunks[order].numofchunkinfreelist !=0){
			for(int a =0; a<abc;a++){
			struct chunk * forins = malloc(sizeof(struct chunk));
			struct chunk * chforfre = buddy.chunks[order].freech.tqh_first;
			TAILQ_REMOVE(&(buddy.chunks[order].freech),chforfre,next);
			//(buddy.chunks[order].numofchunkinfreelist)--;
			buddy.chunks[order].numofchunkinfreelist =buddy.chunks[order].numofchunkinfreelist-1;
			if(chforfre->start == page -(1<<order)){
				rvale =1;
				free(chforfre); free(forins);
			}else{
				forins->start = chforfre->start; forins->order = chforfre->order;
				TAILQ_INSERT_TAIL(&(buddy.chunks[order].freech),forins,next);
				buddy.chunks[order].numofchunkinfreelist =buddy.chunks[order].numofchunkinfreelist+1;
				free(chforfre);
			}
		}
	
	}//printf("rvalue is %d\n",rvale);
	
	return rvale;
}
void free_pages(unsigned int page, const unsigned int order)
{
	/**
	 * Your implementation will look (but not limited to) like;
	 *
	 * Find the buddy chunk from this @order.
	 * if (buddy does not exist in this order-@order free list) {
	 *    put into the TAIL of this chunk list. Problem solved!!!
	 * } else {
	 *    Merge with the buddy
	 *    Promote the merged chunk into the higher-order chunk list
	 *
	 *    Consider the cascading case as well; in the higher-order list, there
	 *    might exist its buddy again, and again, again, ....
	 * }
	 *
	 *----------------------------------------------------------------------
	 * Similar to alloc_pages() above, print following messages using PRINTF
	 * when the event happens;
	 *
	 * - Merge order-$x buddies starting from $left and $right:
	 *   PRINTF("MERGE : 0x%x:%u + 0x%x:%u -> 0x%x:%u\n",
	 *			left, x, right, x, left, x+1);
	 *
	 * - Put an order-@x chunk starting from @page into the free list:
	 *   PRINTF("PUT  : 0x%x:%u\n", page, x);
	 *
	 * Example: Two buddies 0:2 and 4:2 (0:2 indicates an order-2 chunk
	 * starting from 0) are merged to 0:3, and it is merged again with 8:3,
	 * producing 0:4. And then finally the chunk is put into the order-4 free
	 * chunk list:
	 *
	 * MERGE : 0x0:2 + 0x4:2 -> 0x0:3
	 * MERGE : 0x0:3 + 0x8:3 -> 0x0:4
	 * PUT   : 0x0:4
	 *----------------------------------------------------------------------
	 */
	int od = order; int startadd = page;
	struct chunk * inset = malloc(sizeof(struct chunk));
	int rgt_or_lft = startadd%(1<<(od+1));
	while(1){
		if(od==(NR_ORDERS-1)){
			break;
		}
		else if(Buddyornot(rgt_or_lft,od,startadd)==0){ //printf("kkkk\n"); //there is no budy
			break;
		}
		else{
			if(rgt_or_lft == 0){
				od = od+1; rgt_or_lft = startadd%(1<<(od+1));
			}
			else {
				startadd = startadd - (1<<od);
				od = od+1;rgt_or_lft = startadd%(1<<(od+1));
			}
		}
	}
	
	inset->start = startadd; inset->order = od; //printf("put at 0x%x od is %d\n",inset->start,inset->order);
	TAILQ_INSERT_TAIL(&(buddy.chunks[od].freech),inset,next);
	buddy.chunks[od].numofchunkinfreelist = buddy.chunks[od].numofchunkinfreelist+1;//printf("when free %d\n",buddy.chunks[od].numofchunkinfreelist);
	
	buddy.allocated -= (1 << order);
	buddy.free += (1 << order);
}


/**
 * Print out the order-@order free chunk list
 *
 *  In the example above, print_free_pages(0) will print out:
 *  0x1d:0
 *
 *  print_free_pages(2):
 *    0x10:2
 *    0x18:2
 */
void print_free_pages(const unsigned int order)
{
	int num = buddy.chunks[order].numofchunkinfreelist; 
	if(buddy.chunks[order].numofchunkinfreelist>0){
		while(num>0){
			struct chunk * chunkforrm = buddy.chunks[order].freech.tqh_first;
			TAILQ_REMOVE(&(buddy.chunks[order].freech),chunkforrm,next);//printf("fisrts order is %d\n",order);
			(buddy.chunks[order].numofchunkinfreelist)--;
			fprintf(stderr, "    0x%x:%u\n", chunkforrm->start, order);
			TAILQ_INSERT_TAIL(&(buddy.chunks[order].freech),chunkforrm,next);// printf("fisrts order is %d\n",order);
			(buddy.chunks[order].numofchunkinfreelist)++;
			num--;
		}
	}
	/**
	 * Your implementation should print out each free chunk from the beginning
	 * in the following format.
	 * WARNING: USE fprintf(stderr) NOT printf, otherwise the grading
	 * system will evaluate your implementation wrong.
	 */
	//fprintf(stderr, "    0x%x:%u\n", starting_page, order);

}


/**
 * Return the unusable index(UI) of order-@order.
 *
 * Description:
 *    Return the unusable index of @order. In the above example, we have 27 free
 *  pages;
 *  # of free pages =
 *    sum(i = 0 to @MAX_ORDER){ (1 << i) * # of order-i free chunks }
 *
 *    and
 *
 *  UI(0) = 0 / 27 = 0.0 (UI of 0 is always 0 in fact).
 *  UI(1) = 1 (for 0x1d) / 27 = 0.037
 *  UI(2) = (1 (0x1d) + 2 (0x1e-0x1f)) / 27 = 0.111
 *  UI(3) = (1 (0x1d) + 2 (0x1e-0x1f) + 4 (0x10-0x13) + 4 (0x18-0x1b)) / 27
 *        = 0.407
 *  ...
 */
double get_unusable_index(unsigned int order)
{
	double sumofunusa = 0;
	if(order == 0){
		return 0; //printf("hallung\n");
	}
	else{
		for(int a=0; a<order ; a++){
			sumofunusa = sumofunusa+(buddy.chunks[a].numofchunkinfreelist)*(1<<a); 
		}
		return (sumofunusa/buddy.free);
	}
}


/**
 * Initialize your buddy system.
 *
 * @nr_pages_in_order: number of pages in order-n notation to manage.
 * For instance, if @nr_pages_in_order = 13, the system should be able to
 * manage 8192 pages. You can set @nr_pages_in_order by using -n option while
 * launching the program;
 * ./pa4 -n 13       <-- will initiate the system with 2^13 pages.
 *
 * Return:
 *   0      : On successful initialization
 *  -EINVAL : Invalid arguments or when something goes wrong
 */
int init_buddy(unsigned int nr_pages_in_order)
{
	int i;
	struct chunk* chtoins;// chtoins ->order = 0; chtoins ->start =0;
	chtoins = malloc(sizeof(struct chunk));
	chtoins->order =0; chtoins->start = 0;
	buddy.allocated = 0;
	buddy.free = 1 << nr_pages_in_order; 

	for (i = 0; i < NR_ORDERS; i++) {
		buddy.chunks[i].order = i;
		buddy.chunks[i].numofchunkinfreelist = 0;
		TAILQ_INIT(&(buddy.chunks[i].freech));
	}
	if(nr_pages_in_order<NR_ORDERS-1){
		return EINVAL;
	}
	if(nr_pages_in_order == NR_ORDERS-1){
		chtoins->start = 0; chtoins->order = nr_pages_in_order;
		TAILQ_INSERT_TAIL(&(buddy.chunks[nr_pages_in_order].freech),chtoins,next); //first put chunk which is start at 0
		(buddy.chunks[nr_pages_in_order].numofchunkinfreelist)++;
	}
	else if(nr_pages_in_order> NR_ORDERS-1){ //if nrpio > MAX order spilt page until nrpio == maxorder 
		chtoins->start = 0; chtoins->order = NR_ORDERS-1; unsigned int add=0;
		TAILQ_INSERT_TAIL(&(buddy.chunks[NR_ORDERS-1].freech),chtoins,next);//printf("%d max order is%d\n",nr_pages_in_order,NR_ORDERS-1);
		(buddy.chunks[NR_ORDERS-1].numofchunkinfreelist)++;
		for(int a = 1; a<(1<<(nr_pages_in_order-NR_ORDERS+1));a++){ //printf("2\n");
			struct chunk * chforthis = malloc(sizeof(struct chunk));
			add = a*(1<<(nr_pages_in_order-1))/(1<<(nr_pages_in_order-NR_ORDERS));//printf("start 0x%x nr is %d\n",add,a);
			chforthis->start = add; 
			chforthis->order = NR_ORDERS-1;	//printf("order %d\n",chforthis->order);
			TAILQ_INSERT_TAIL(&(buddy.chunks[NR_ORDERS-1].freech),chforthis,next);
			(buddy.chunks[NR_ORDERS-1].numofchunkinfreelist)++;
		}	
	}//printf("",buddy.chunks[NR_ORDERS-1].)
	/**
	 * TODO: Don't forget to initiate the free chunk list with
	 * order-@MAX_ORDER chunks. Note you might add multiple chunks if
	 * @nr_pages_in_order > @MAX_ORDER. For instance, when
	 * @nr_pages_in_order = 10 and @MAX_ORDER = 9, the initial free chunk
	 * lists will have two chunks; 0x0:9, 0x200:9. 1
	 * (1~numof split)/(num of split+1)* (1<<nrpio) is address of first free chunk
	 */

	return 0;
}


/**
 * Return resources that your buddy system has been allocated. No other
 * function will be called after calling this function.
 */
void fini_buddy(void)
{
	/**
	 * TODO: Do your finalization if needed, and don't forget to release
	 * the initial chunks that you put in init_buddy().
	 */
	int abcd;
	for(int a = 0; a<NR_ORDERS;a++){
		abcd =buddy.chunks[a].numofchunkinfreelist;
		if(abcd>0){
			for(int b=0 ; b<abcd;b++){
				struct chunk * rmv = buddy.chunks[a].freech.tqh_first;
				TAILQ_REMOVE(&(buddy.chunks[a].freech),rmv,next);
				(buddy.chunks[a].numofchunkinfreelist)--;
				free(rmv);
			}
		}
	}
}
