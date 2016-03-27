/**
 * @file	isu_mmu.c
 * @author	Kris Hall
 * @date	3/29/2015 - created
 * @date	3/26/2016 - updated and simplified
 * @brief	source file of isu_mmu.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "isu_mmu.h"
#include "llist/isu_llist.h"
#include "common/isu_error.h"
#include "common/isu_color.h"

/// defines for the delays for memory access, in nanoseconds
#define NUM_PAGES 8

/*****************************
 *Prototypes
 *****************************/
int isu_mmu_page_check(isu_mmu_t mem, int p);
int isu_mmu_page_rep_fifo(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t);
int isu_mmu_page_rep_lru(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t);
int isu_mmu_page_rep_clock(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t);
int isu_mmu_page_rep_second_chance(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t);

struct ISU_MEM_PAGE_STRUCT{

	/// reference bit
	char ref;

	/// the page number
	int page;

	/// the time of last access
	unsigned long long access_time;

	/// the time of placement
	unsigned long long placement_time;
};

typedef struct ISU_MEM_PAGE_STRUCT *isu_mem_page_t;

struct ISU_MMU_STRUCT{
	/// array representing the working set
	isu_mem_page_t *pages;

	/// the mode of operation for page replacement
	int rep_mode;

	/// the position of the hand for the clock algorithm
	int hand;
};

isu_mmu_t isu_mmu_create(int mode){
	int i;
	isu_mmu_t mmu;
	mmu = calloc(1, sizeof(struct ISU_MMU_STRUCT));
	if(mmu == NULL){
		isu_print(PRINT_ERROR, "calloc returned NULL");
		return NULL;
	}
	mmu->pages = malloc(NUM_PAGES * sizeof(isu_mem_page_t));

	for(i = 0; i < NUM_PAGES; i++){
		mmu->pages[i] = calloc(1, sizeof(struct ISU_MEM_PAGE_STRUCT));
		mmu->pages[i]->page = -1;
		mmu->pages[i]->placement_time = 0;
		mmu->pages[i]->access_time = 0;
		mmu->pages[i]->ref = 0;
	}
	mmu->rep_mode = mode;
	mmu->hand = 0;
	isu_print(PRINT_DEBUG, "created new MMU");
	return mmu;
}

void isu_mmu_destroy(isu_mmu_t mem){
	free(mem->pages);
	mem->pages = 0;
	free(mem);
	mem = 0;
}

int isu_mmu_handle_req(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t){
	// return value
	int ret;

	/// book keeping where we set the time of the request being started
	isu_mem_req_set_req_time(req, *t);
	// run certain replacement algorithms based on the value of mode
	switch(mem->rep_mode){
	case 1: ret = isu_mmu_page_rep_lru(mem, req, t);
		break;
	case 2: ret = isu_mmu_page_rep_clock(mem, req, t);
		break;
	case 3: /// This is left here for the second chance algorithm
		break;
	default: ret = isu_mmu_page_rep_fifo(mem, req, t);
		break;
	}

	/// book keeping where we set the time of the request being handled
	isu_mem_req_set_handle_time(req, *t);

	return ret;
}

int isu_mmu_ref_clear(isu_mmu_t mem){
	int i;
	for(i = 0; i < NUM_PAGES; i++){
		mem->pages[i]->ref = 0;
	}
}

/// checks if the page `page` already exists
/// returns 0 if it is not
/// returns 1 if it is
int isu_mmu_page_check(isu_mmu_t mem, int p){
	if(p < 0){
		isu_print(PRINT_ERROR, "Error in value of p: value not applicable");
	}

	int i;
	for(i = 0; i < NUM_PAGES; i++){
		if(mem->pages[i]->page == p){
			mem->pages[i]->ref = 1;
			return 1;
		}
	}
	// can't find it
	return 0;
}

int isu_mmu_page_rep_fifo(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t){
	int ret;
	int new;
	int old;
	int i;

	/// book keeping purposes, copying all the pages in L1 cache
	for(i = 0; i < NUM_PAGES; i++){
		isu_mem_req_add_page(req, (mem->pages[i]->page));
	}
	
	//first, get the page that is being requested
	int page = isu_mem_req_get_page(req);

	//once we have the page number, check if it is in memory
	//if the page is in the working set, it is a hit, otherwise it is a miss
	//	if it is a miss, but it is still in memory, we swap it out
	//	if it is not in memory, we have to go fetch it
	char hit = (char)isu_mmu_page_check(mem, page);
	/// if `hit` is 1, it is a hit, and that the memory page is already in the working set
	if(hit){
		isu_mem_req_set_access_hit(req, 1);
		for(i = 0; i < NUM_PAGES; i++){
			if(mem->pages[i]->page == page){
				mem->pages[i]->access_time = *t;
				mem->pages[i]->ref = 1;
			}
		}
		ret = 0;
	/// if `hit` is 0 then `page` is not in the working set, and we need to replace it
	}else{
		/// the new page to be put in the working set is `page`
		new = page;

		/// now we figure out which one is to be replaced
		/// first find the position of the page with the earliest placement time
		unsigned long long temp = mem->pages[0]->placement_time;
		old = 0;
		for(i = 0; i < NUM_PAGES; i++){
			if(temp > mem->pages[i]->placement_time){
				temp = mem->pages[i]->placement_time;
				old = i;
			}
		}
		
		/// once the loop is complete, we know the `old` page to be replaced with
		/// the `new` page
		mem->pages[old]->placement_time = *t;
		mem->pages[old]->access_time = *t;
		mem->pages[old]->page = new;
		mem->pages[old]->ref = 1;
		ret = 0;
	}

	/// increment the time, t
	(*t)++;

	return ret;
}

int isu_mmu_page_rep_lru(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t){
	/// TODO implement the LRU page replacement algorithm here
	/// First we want to do some book keeping, copy the elements in L1
	/// to the memory request class
	#warning TODO implement the LRU page replacement algorithm
}

int isu_mmu_page_rep_clock(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t){
	/// TODO implement the clock page replacement algorithm here
	/// First we want to do some book keeping, copy the elements in L1
	/// to the memory request class
	#warning TODO implement the clock page replacement algorithm
}

int isu_mmu_page_rep_second_chance(isu_mmu_t mem, isu_mem_req_t req, unsigned long long *t){
	/// TODO implement the clock page replacement algorithm here
	/// 
}
