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

int coalesceNext(BlockHeader * curr) {
    BlockHeader * next = GET_NEXT(curr);
    if (GET_FREE(next)&& GET_FREE(curr)) {
        SET_NEXT(curr, GET_NEXT(next));
        return 1;
    }
    return 0;
}

void simple_init() {
    //Check if CPU is 32 or 64 bits
    int instructionSetBits = sizeof(uintptr_t) * 8;
    uintptr_t aligned_memory_start = memory_start + (memory_start % instructionSetBits);
    uintptr_t aligned_memory_end   = memory_end - (memory_end % instructionSetBits);

    if (first == NULL) {
        if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            first = (BlockHeader *) aligned_memory_start;
            last = (BlockHeader *) aligned_memory_end-sizeof(BlockHeader);
            SET_FREE(first, 1);
            SET_NEXT(first, last);
            SET_NEXT(last, first);
            SET_FREE(last, 0);
            current = first;
        }
    }
}

void* simple_malloc(size_t size) {
    if (first == NULL) {
        //printf("simple_init() \n");
        simple_init();
        //printf("done \n");
        if (first == NULL) return NULL;
    }
//Pad the requested size to a multiple of 8 bytes
    size_t aligned_size = align_up(size, sizeof(uintptr_t));
    BlockHeader * search_start = current;
    int allocated = 0;

    do { // Search for a free block
        if (GET_FREE(current)) { //the current block is free
            if (SIZE(current) >= aligned_size) { // The current block is large enough to contain the requested block
                //printf("(SIZE(current) >= aligned_size) == true \n");
                if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    //The current block is large enough to contain only the requested block
                    //printf("SET_FREE) \n");
                    SET_FREE(current, 0);
                    allocated = 1;
                    //printf("Done \n");
                } else {
                    //The current block is large enough to contain the requested block and a new free block
                    BlockHeader * new_block = (BlockHeader *)((uintptr_t)current + sizeof(BlockHeader) + aligned_size);
                    BlockHeader * next = GET_NEXT(current);
                    SET_FREE(new_block, 1);
                    if (GET_FREE(next)){//If next is free, coalesce new block with next
                        SET_NEXT(new_block, GET_NEXT(next));
                    }
                    else{ //If next is not free, set next as next of new block
                        SET_NEXT(new_block, next);
                    }
                    SET_NEXT(current, new_block);
                    SET_FREE(current, 0);
                    allocated = 1;
                }
            } else { //The current block is not large enough to contain the requested block
                //printf("Assessing next block \n");
                BlockHeader * next = (GET_NEXT(current));
                // Check if the next block is free and the combined block is large enough
                if ((next != NULL) && (GET_FREE(next)) && ((SIZE(current) + SIZE(next) + sizeof(BlockHeader)) >= aligned_size)) {
                    allocated = coalesceNext(current);
                    if (allocated) {
                        SET_FREE(current, 0);
                  //      printf("Successfully coalesced 2 blocks\n");
                    }
                    else {
                    //    printf("Failed to coalesce 2 blocks\n");
                        return NULL;
                    }
                }
            }
            // If a block has been allocated, check if it can be split further
            if (allocated) {
                void *currAdd = (void *)((uintptr_t)current + sizeof(BlockHeader));
                current = GET_NEXT(current);
                if ((uintptr_t)current > memory_end) {
                    return NULL;
                }
                if (GET_NEXT(current)==NULL){
                    SET_NEXT(current, last);
                }

               // printf("Returning address \n");
                return currAdd; // Return the address of the allocated block
            }
        }
        current = GET_NEXT(current);
    } while (current != search_start);

/* None found */
    return NULL;
}

void simple_free(void * ptr) {
    //printf("Simple free called\n");
    if (ptr == NULL) return;

    BlockHeader * block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));
    if (GET_FREE(block)) {
        return; //block is already free
    }
    SET_FREE(block, 1);
}

/* Include test routines */

#include "mm_aux.c"
