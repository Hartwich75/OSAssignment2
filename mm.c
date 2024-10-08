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

/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
    uintptr_t aligned_memory_start = memory_start + (memory_start % 64);  /* Align start */
    uintptr_t aligned_memory_end   = memory_end - (memory_end % 64);   /* Align end */

    /* Already initialized ? */
    if (first == NULL) {
        /* Check that we have room for at least one free block and an end header */
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

/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation.
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */
void* simple_malloc(size_t size) {

    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }

    size_t aligned_size = align_up(size, sizeof(uintptr_t));  /* Align size to word boundary */

    /* Search for a free block */
    BlockHeader * search_start = current;
    do {
        if (GET_FREE(current)) {
            /* Possibly coalesce consecutive free blocks here */

            /* Check if free block is large enough */
            if (SIZE(current) >= aligned_size) {
                /* Will the remainder be large enough for a new block? */
                if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    /* Use block as is, marking it non-free */
                    SET_FREE(current, 0);  /* Mark block as allocated */
                } else {
                    /* Carve aligned_size from block and allocate new free block for the rest */
                    BlockHeader * new_block = (BlockHeader *)((uintptr_t)current + sizeof(BlockHeader) + aligned_size);
                    SET_NEXT(new_block, GET_NEXT(current));
                    SET_FREE(new_block, 1);  /* New block is free */
                    SET_NEXT(current, new_block);  /* Update current's next */
                    SET_FREE(current, 0);  /* Mark current block as allocated */
                }
                void *currAdd = (void *)((uintptr_t)current + sizeof(BlockHeader));
                current = GET_NEXT(current);  /* Advance current */
                return currAdd;
            }
        }

        current = GET_NEXT(current);
    } while (current != search_start);

    /* None found */
    return NULL;
}

/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation.
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
void simple_free(void * ptr) {
    if (ptr == NULL) return;  /* Null pointer check */

    BlockHeader * block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader)); /* Find the block corresponding to ptr */
    if (GET_FREE(block)) {
        /* Block is not in use -- probably an error */
        return;
    }

    /* Free the block */
    SET_FREE(block, 1);

    /* Possibly coalesce consecutive free blocks here */
    BlockHeader * next_block = GET_NEXT(block);
    if(GET_FREE(next_block)) {
        SET_NEXT(block, GET_NEXT(next_block));
    }
    current = first;
}

/* Include test routines */

#include "mm_aux.c"
