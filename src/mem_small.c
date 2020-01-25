/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void *
emalloc_small(unsigned long size)
{
    // if chunkpool is empty, allocate new chunkpool
    // create new list for the chunkpool
    if (arena.chunkpool == NULL) {
        unsigned long new_chunkpool_size = mem_realloc_small();
        unsigned long chunk_number = new_chunkpool_size/CHUNKSIZE;

        //head of the list
        void *ptr = arena.chunkpool;

        for (int i = 0; i < chunk_number; i++) {
            //actual ptr points to next one
            *(void **)ptr = ptr + CHUNKSIZE;
            //move on to the next chunk
            ptr = ptr + CHUNKSIZE;
        }
    }

    //get the first chunk of the chunkpool
    void *ptr = arena.chunkpool;
    //update the chunkpool to get the first ptr
    arena.chunkpool = *(void **) ptr;

    //call function to mark the ptr
    return mark_memarea_and_get_user_ptr(ptr, CHUNKSIZE, SMALL_KIND);
    
}

void efree_small(Alloc a) {
    //get a to the head of the list
    *(void **) a.ptr = arena.chunkpool;
    //update the chunkpool
    arena.chunkpool = a.ptr;
}
