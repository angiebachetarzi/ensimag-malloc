/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}

void * 
get_available_block(int k) {
    //case 1 : no more available blocks
    //need to reallocate
    if (k >= FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant) {
        mem_realloc_medium();
    }

    //case 2 : find an available block
    //need to remove it from the list and return it
    void *curr = arena.TZL[k];
    if (curr != NULL){
        arena.TZL[k] = *(void**)curr;
        return curr;
    }

    //case 3 : no blocks available
    //need to get a bigger block
    //divide a block number k+1 (recursively)
    void *ptr = get_available_block(k + 1);
    //get buddy and put it in the list
    void *buddy = (void *)((uintptr_t)ptr ^ (1 << k));
    *(void **) buddy = 0;
    arena.TZL[k] = buddy;

    //return the first half of the block
    return ptr;
}

void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    
    unsigned long new_size = size + 32;

    unsigned int k = puiss2(new_size);

    //get available block (size == 2^k)
    //eventually, divide blocks to obtain it
    void *p = get_available_block(k);
    
    //call marking function for the block
    return mark_memarea_and_get_user_ptr(p, new_size, MEDIUM_KIND);
}


void efree_medium(Alloc a) {
    /* ecrire votre code ici */
}


