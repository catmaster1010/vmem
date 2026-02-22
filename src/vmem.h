#ifndef _VMEM_H
#include "vmem_common.h"
#include <stddef.h>
#include <stdint.h>

#define VM_SLEEP 0x10
#define VM_NOSLEEP 0x01

#define VMEM_HASHTABLE_LENGTH                                                  \
  64                         /* number of buckets for allocated hash table */
#define VMEM_INITIAL 15      /* number of initial vmem pools */
#define VMEM_SEG_INITIAL 100 /* number of initial segments */

#define VMEM_ALLOC 0x01 /* Allocated segment */
#define VMEM_FREE 0x02  /* Free segment */
#define VMEM_SPAN 0x03  /* Span segment */

typedef struct vmem vmem_t;
typedef struct vmem_seg vmem_seg_t;

#define VMEM_FREELIST_LEN (sizeof(uintptr_t) * 8)

struct vmem {
  char vm_name[VMEM_NAMELEN]; /* arena name */
  vmem_lock_t vm_lock;        /* arena lock, defined in vmem_common.h */
  uint32_t vm_id;             /* arena id */
  vmem_t *vm_source;          /*arena vmem source for importing resources*/
  size_t
      vm_quantumn; /* quote from solaris8: "Most commonly the quantum is either
                    * 1 or PAGESIZE, but any power of 2 is legal.  All vmem
                    * allocations are guaranteed to be quantum-aligned.""*/
  void *(*vm_source_alloc)(vmem_t *, size_t, int);
  void (*vm_source_free)(vmem_t *, void *, size_t);
  vmem_seg_t *vm_freelist[VMEM_FREELIST_LEN + 1];
  vmem_seg_t *vm_hashtable[VMEM_HASHTABLE_LENGTH]; /* initial hash table */
  size_t vm_nsegfree;     /* # of free vmem_seg_t structures*/
  vmem_seg_t *vm_segfree; /* freelist of `vmem_seg_t` structures */
  vmem_seg_t *vm_sp;      /* seg pointer to the dll of segment structures */
};

struct vmem_seg {
  uintptr_t vs_start;   /* start of the segment */
  uintptr_t vs_end;     /* end of the segment */
  vmem_seg_t *vs_knext; /* next segment of the same kin/type (span, free
                        segment, allocated segment) */
  vmem_seg_t *vs_kprev; /* previous segment of the same kin/type  */

  vmem_seg_t *vs_anext; /* next segment in the arena */
  vmem_seg_t *vs_prev;  /* previous segment in the arena */
};

vmem_t *
vmem_create(const char *name,                        /* descriptive name */
            void *base,                              /* start of initial span */
            size_t size,                             /* size of initial span */
            size_t quantum,                          /* unit of currency */
            void *(*afunc)(vmem_t *, size_t, int),   /* import alloc function */
            void (*ffunc)(vmem_t *, void *, size_t), /* import free function */
            vmem_t *source,                          /* import source arena */
            size_t qcache_max,                       /* maximum size to cache */
            int vmflag); /* VM_SLEEP or VM_NOSLEEP */

void vmem_destroy(vmem_t *vmp);

void *vmem_alloc(vmem_t *vmp, size_t size, int vmflag);

void vmem_free(vmem_t *vmp, void *addr, size_t size);

void *vmem_xalloc(vmem_t *vmp, size_t size, size_t align, size_t phase,
                  size_t nocross, void *minaddr, void *maxaddr, int vmflag);

void vmem_xfree(vmem_t *vmp, void *addr, size_t size);

void *vmem_add(vmem_t *vmp, void *addr, size_t size, int vmflag);

void vmem_init(
    void *base,
    size_t size); // Virtual adress of the base and size of the kernel heap.

#endif // !_VMEM_H
