/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>

#include "mm.h"

/* Proposed data structure elements */

typedef struct header {
    struct header * next;     // Bit 0 is used to indicate free block
    uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (BlockHeader *)((uintptr_t)(p->next) & ~0x1)    /* Mask out free flag */
#define SET_NEXT(p,n)  p->next = (BlockHeader *)(((uintptr_t)n & ~0x1) | ((uintptr_t)p->next & 0x1))  /* Preserve free flag */
#define GET_FREE(p)    (uint8_t) (((uintptr_t)(p->next) & 0x1))   /* Get the free flag */
#define SET_FREE(p,f)  p->next = (BlockHeader *)(((uintptr_t)GET_NEXT(p)) | ((f) ? 0x1 : 0x0))   /* Set free bit */
#define SIZE(p)        ((size_t)((uintptr_t)GET_NEXT(p) - (uintptr_t)(p) - sizeof(BlockHeader)))  /* Calculate block size */

#define MIN_SIZE     (8)   // A block should have at least 8 bytes available for the user

extern const uintptr_t memory_start, memory_end;

static BlockHeader * first = NULL;
static BlockHeader * current = NULL;
static BlockHeader *last = NULL;

/**
 * @name    align_up
 * @brief   Aligns a given address upwards to the nearest multiple of alignment.
 *
 * @param   size_t x The address or size to align.
 * @param   size_t align The alignment boundary.
 * @retval  Aligned value.
 */
static inline size_t align_up(size_t x, size_t align) {
return (x + (align - 1)) & ~(align - 1);
}

void simple_init() {
    uintptr_t aligned_memory_start = memory_start + (memory_start % 64);
    uintptr_t aligned_memory_end   = memory_end - (memory_end % 64);

    if (first == NULL) {
        if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            first = (BlockHeader *) aligned_memory_start;
            last = (BlockHeader *) aligned_memory_end;
            SET_FREE(first, 1);
            SET_NEXT(first, last);
            SET_NEXT(last, first);
            SET_FREE(last, 1);
            current = first;
        }
    }
}

void* simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }
    //Pad the requested size to a multiple of 8 bytes
    size_t aligned_size = align_up(size, sizeof(uintptr_t));
    BlockHeader * search_start = current;

    do {
        if (GET_FREE(current)) {
            if (SIZE(current) >= aligned_size) {
                //The current block is large enough to contain the requested block
                if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    SET_FREE(current, 0);
                } else {
                    BlockHeader * next = (GET_NEXT(current));
                    if ((GET_FREE(next)) && ((SIZE(current)+SIZE(next)+ 8) >= aligned_size)){
                        //coalesce current block and next block
                        SET_NEXT(current, GET_NEXT(next));
                        SET_FREE(current,0);
                    }
                    BlockHeader * new_block = (BlockHeader *)((uintptr_t)current + sizeof(BlockHeader) + aligned_size);
                    SET_NEXT(new_block, GET_NEXT(current));
                    SET_FREE(new_block, 1);
                    SET_NEXT(current, new_block);
                    SET_FREE(current, 0);
                }
                void *currAdd = (void *)((uintptr_t)current + sizeof(BlockHeader));
                current = GET_NEXT(current);
                return currAdd;
            }
        }
        current = GET_NEXT(current);
    } while (current != search_start);

    return NULL;
}

void simple_free(void * ptr) {
    if (ptr == NULL) return;

    BlockHeader * block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));
    if (GET_FREE(block)) {
        return;
    }

    SET_FREE(block, 1);
/*
    BlockHeader * next_block = GET_NEXT(block);
    if (GET_FREE(next_block)) {
        SET_NEXT(block, GET_NEXT(next_block));
    }
    current = first;
    */
}

/* Include test routines */

#include "mm_aux.c"
