
/* Test routines to be included at the end of mm.c */


/**
 * @name    simple_macro_test
 * @brief   Makes an internal test of the given macros
 * @retval  0 if ok, otherwise a positive number indicating the error cause
 */
int simple_macro_test() {
  BlockHeader block;
  BlockHeader * p = &block;
  void * addr[2] = { (void *)  0x1234BABA, (void *) 0xFEDCBA981234BABA };
  int i;
  int ret = 0;

  /* Test separately for 32 and 64 bit addresses */
  for (i =0; i < 2; i++) {
    p->next = NULL;
    /* Check that next and free are properly separated */
    SET_NEXT(p, addr[i]);
    SET_FREE(p, 7);  /* only least bit should be used */

    if (GET_NEXT(p) != addr[i]) return 1 + i*10;  // Next pointer damaged
    if (GET_FREE(p) != 1)       return 2 + i*10;  // Free flag not set

    SET_NEXT(p, NULL);
    if (GET_FREE(p) != 1)       return 3 + i*10;  // Free flag damaged

    SET_NEXT(p, addr[i]);
    SET_FREE(p, 0);

    if (GET_FREE(p) != 0)       return 4 + i*10;  // Free flag not cleared
    if (GET_NEXT(p) != addr[i]) return 5 + i*10;  // Next pointer damaged

    /* Check size for forward next pointer */
    SET_NEXT(p, (void *) ((uintptr_t) p + sizeof(BlockHeader) + 0x100 ) );
    if (SIZE(p) !=  0x100)      return 6 + i*10;

    /* Check size for backward next pointer (dummy block) */
    SET_NEXT(p, (void *) ((uintptr_t) p + sizeof(BlockHeader) - 0x100 ) );
    if (SIZE(p) != 0 && SIZE(p) < 0x800000000000000 )   return 7 + i*10;
  
  }
  return ret;
} 


static void print_block(BlockHeader * p) {
  printf("Block at 0x%08lx next = 0x%08lx, free = %d\n",  (uintptr_t) p, (uintptr_t) GET_NEXT(p), GET_FREE(p));
}


/**
 * @name    simple_block_dump
 * @brief   Dumps the current list of blocks on standard out
 */
void simple_block_dump(void) {
  BlockHeader * p;

  if (first == NULL) {
    printf("Data structure is not initialized\n");
    return;
  }

  printf("first = 0x%08lx, current = 0x%08lx\n", (uintptr_t) first, (uintptr_t) current);

  p = first;

  do {
    if ((uintptr_t) p < memory_start || (uintptr_t) p >= memory_end) {
      printf("Block pointer 0x%08lx out of range\n", (uintptr_t) p);
      return;
    }

    print_block(p);

    p = GET_NEXT(p);
  } while (p != first);

}

