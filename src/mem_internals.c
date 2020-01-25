/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    unsigned long magic = knuth_mmix_one_round((unsigned long )ptr);
    magic = (magic & ~(0b11UL)) | k;
    *(unsigned long *)(ptr + 1) = magic;

    *(unsigned long *)ptr = size;
    
    *(unsigned long *)(ptr + sizeof(size)/8 - 2) = magic;
    *(unsigned long *)(ptr + sizeof(size)/8 - 1) = size;

    // 16 because the total of (size + magic) is 8B!!
    return ptr + 16;
}

Alloc
mark_check_and_get_alloc(void *ptr)
{
    Alloc a = {};

    unsigned long actual_size = *(unsigned long *) (ptr - 2);
    unsigned long actual_magic = *(unsigned long *)(ptr - 1);
    unsigned long expected_magic = knuth_mmix_one_round((unsigned long) (ptr - 2));

    //assert that actual_magic is correct
    assert(actual_magic == expected_magic);

    unsigned long size_end = *(unsigned long *)(ptr + actual_size/8 - 3);

    //assert that size at the end is equal to the one at the start
    assert(actual_size == size_end);

    *(unsigned long *)a.ptr = *(unsigned long *)(ptr - 2);
    a.kind = actual_magic & 0b11UL;
    a.size = actual_size;

    return a;
}


unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1 << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
