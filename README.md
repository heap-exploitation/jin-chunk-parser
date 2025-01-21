# glibc 2.26

### env

**`Pwngdb`**: for heapinfo

### chunk

```c
struct malloc_chunk {

  INTERNAL_SIZE_T      mchunk_prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      mchunk_size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;

  /* Only used for large blocks: pointer to next larger size.  */
  struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
  struct malloc_chunk* bk_nextsize;
};
```

- `prev_size`: size of the previous chunk when it was freed
- `size`: chunk size + flag (the least significant 3 bits)
    
    Because the CPU allocates Heap Chunks in multiples of 8 bytes or 16 bytes, the least significant 3 bits are not used. So the system uses these bits as flag bits.
    
    - PREV_INUSE
    - IS_MMAPPED
    - NON_MAIN_ARENA
- `fd`: actual start of the data area, and it is not used when the current chunk is allocated. When it is freed, fd points to the forward chunk.
- `bk`: When the current chunk is freed, bk points to the backward chunk.
- `fd_nextsize`: a pointer used in the large bin, pointing to the address of a heap chunk smaller than the current heap chunk.
- `bk_next_size`: a pointer used in the large bin, pointing to the address of a heap chunk larger than the current heap chunk.

### bin & tcache

`Fastbin`: 32 ~ 128 bytes

`Smallbin`: < 1024 bytes

`Largebin`: ≥ 1024 bytes

`Tcache`: 32 ~ 1032 bytes

### main_arena

```c
struct malloc_state
{
  /* Serialize access.  */
  __libc_lock_define (, mutex);

  /* Flags (formerly in max_fast).  */
  int flags;

  /* Fastbins */
  mfastbinptr fastbinsY[NFASTBINS];

  /* Base of the topmost chunk -- not otherwise kept in a bin */
  mchunkptr top;

  /* The remainder from the most recent split of a small request */
  mchunkptr last_remainder;

  /* Normal bins packed as described above */
  mchunkptr bins[NBINS * 2 - 2];

  /* Bitmap of bins */
  unsigned int binmap[BINMAPSIZE];

  /* Linked list */
  struct malloc_state *next;

  /* Linked list for free arenas.  Access to this field is serialized
     by free_list_lock in arena.c.  */
  struct malloc_state *next_free;

  /* Number of threads attached to this arena.  0 if the arena is on
     the free list.  Access to this field is serialized by
     free_list_lock in arena.c.  */
  INTERNAL_SIZE_T attached_threads;

  /* Memory allocated from the system in this arena.  */
  INTERNAL_SIZE_T system_mem;
  INTERNAL_SIZE_T max_system_mem;
};
```
