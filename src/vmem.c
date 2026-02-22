#include "vmem.h"
#include "vmem_common.h"
#include <sys/types.h>

vmem_t *heap_arena;      /* Primordial kernel vmem arena*/
vmem_t *vmem_seg_arena;  /* arena for `vmem_seg_t` structures*/
vmem_t *vmem_vmem_arena; /* arena for vmem structures, self explanatory  */

static vmem_t vmem0[VMEM_INITIAL];
static vmem_seg_t vmem_seg0[VMEM_SEG_INITIAL];

static uint32_t vmem_id;
vmem_lock_t vmem_id_lock;

/*
 * Assumes the arena lock is obtained while this procedure is called
 * Put a vmem_seg_t structure on vmp's segfree list.
 */
static void vmem_put_seg(vmem_t *vmp, vmem_seg_t *vsp) {
  vsp->vs_knext = vmp->vm_segfree;
  vmp->vm_segfree = vsp;
  vmp->vm_nsegfree++;
}

/*
 * Assumes the arena lock is obtained while this procedure is called
 * Returns a pointer to a vmem_seg_t structure from vmp's segfree list, aka
 * vmp->vm_segfree, if vmp->vm_nsegfree = 0, then a NULL pointer is returned
 */
static vmem_seg_t *vmem_get_seg(vmem_t *vmp) {
  vmem_seg_t *new_seg;
  if (vmp->vm_nsegfree > 0) {
    new_seg = vmp->vm_segfree;
    vmp->vm_segfree = new_seg->vs_knext;
    vmp->vm_segfree--;
    return (new_seg);
  }
  return NULL;
}

// vm_lock is held while entering & exiting this procedure
static int vmem_populate(vmem_t *vmp, size_t nsegneeded, int vmflag) {
  vmem_seg_t *sp;
  vmem_t *smp = vmem_seg_arena;
  while (vmp->vm_nsegfree < nsegneeded) {
    vmem_lock_release(&vmp->vm_lock);
    vmem_lock_obtain(&smp->vm_lock);
    if (smp->vm_nsegfree > 0) {
      sp = vmem_get_seg(smp);
      vmem_lock_release(&smp->vm_lock);
      vmem_lock_obtain(&vmp->vm_lock);
      vmem_put_seg(vmp, sp);
      continue;
    }

    /* TODO: wuh woh, there are no segments left in the segment arena
     * so, allocate a page, then divy it up, then that is our new segments :)
     * Note; we manually allocate from the heap arena rather than a call to
     * vmem_alloc reasoning: (From solaris8 code quoted) (1) we'd immediately
     * recurse into vmem_populate(), and (2) we need precise control over how
     * 'vtemp' is used. `vtemp` is the the temporary stack allocated vmem_seg_t
     * to get around the fact that we cant allocate from heap_arena unless we
     * have `vmem_seg_t` structures
     */
  }

  if (vmp->vm_nsegfree >= nsegneeded) {
    return 1;
  }
  return 0;
}

static void *vmem_segment_create(vmem_t *vmp, void *addr, size_t size,
                                 u_int8_t type) {}

static void *vmem_span_create(vmem_t *vmp, void *addr, size_t size) {

  return NULL;
}

void *vmem_xalloc(vmem_t *vmp, size_t size, size_t align, size_t phase,
                  size_t nocross, void *minaddr, void *maxaddr, int vmflag) {
  if (size == 0) {
    vmem_panic("vmem_xalloc: size cannot be 0.");
  }
  if ((align | phase | nocross) & (vmp->vm_quantumn - 1)) {
  }
  vmem_lock_obtain(&vmp->vm_lock);
}

void *vmem_alloc(vmem_t *vmp, size_t size, int vmflag) {
  // TODO: dont default to non quantum caching allocations
  return vmem_xalloc(vmp, size, vmp->vm_quantumn, 0, 0, NULL, NULL, vmflag);
}

vmem_t *vmem_create(const char *name, void *base, size_t size, size_t quantum,
                    void *(*afunc)(vmem_t *, size_t, int),
                    void (*ffunc)(vmem_t *, void *, size_t), vmem_t *source,
                    size_t qcache_max, int vmflag) {
  vmem_t *vmp;
  vmem_seg_t *vsp;

  vmem_lock_obtain(&vmem_id_lock);
  uint32_t id = +1;
  vmem_lock_release(&vmem_id_lock);

  if (vmem_vmem_arena == NULL) { /* Arena still in bootstrapping process */
    vmp = &vmem0[id - 1];
  } else {
    vmem_alloc(vmem_vmem_arena, sizeof(vmem_t), vmflag);
  }

  strcpy(vmp->vm_name, name);

  if (vmp == NULL) {
    return (NULL);
  }

  vmp->vm_quantumn = quantum;
  vmem_lock_init(&vmp->vm_lock);

  /* Init the free list */
  for (int i = 0; i < VMEM_FREELIST_LEN; i++) {
    vsp = vmp->vm_freelist[i];
    vsp->vs_start = 0;
    vsp->vs_end = 0;
    vsp->vs_knext = (vsp + 1);
    vsp->vs_kprev = (vsp - 1);
  }
  vmp->vm_freelist[0]->vs_prev = NULL;
  vmp->vm_freelist[VMEM_FREELIST_LEN - 1]->vs_knext = NULL;

  // TODO: quantum cache shenanigans, as in, create the caches and whatnot
  //
  // TODO: add initial span, [base, base+size) to the arena

  if (vmem_add(vmp, base, size, vmflag) == NULL && (base || size)) {
    vmem_destroy(vmp);
    return NULL;
  }

  return vmp;
}

void vmem_destroy(vmem_t *vmp);

void vmem_free(vmem_t *vmp, void *addr, size_t size);

void vmem_xfree(vmem_t *vmp, void *addr, size_t size);

/* Returns addr on sucess, NULL on failure ,
 * Fails if:
 * (1) No resources available (e.g. unable to allocate a page to populate vmp)
 * AND
 * (2) vmflag is VM_NOSLEEP
 */
void *vmem_add(vmem_t *vmp, void *addr, size_t size, int vmflag) {
  vmem_lock_obtain(&vmp->vm_lock);
  /*
   * Two are needed; one for the new span marker, one for the new segment
   */
  if (vmem_populate(vmp, 2, vmflag)) {
    // create the span
    return vmem_span_create(vmp, addr, size);
  } else {
    return NULL;
  }
}

/* A bit of info on these functions: (from bonwick '01)
 * The power of importing lies in the side effects of the
 * import functions, and is best understood by example.
 * In Solaris, the function segkmem_alloc() invokes
 * vmem_alloc() to get a virtual address and then backs
 * it with physical pages. Therefore, we can create an
 * arena of mapped pages by simply importing from an
 * arena of virtual addresses using segkmem_alloc()
 * and segkmem_free().
 * */
static void segkmem_free(vmem_t *vmp, void *addr, size_t size) {}
static void *segkmem_alloc(vmem_t *vmp, size_t size, int vmflag) {
  return NULL;
}

void vmem_init(void *base, size_t size) {
  vmem_lock_init(&vmem_id_lock);
  heap_arena = vmem_create("heap", NULL, 0, PAGE_SIZE, segkmem_alloc,
                           segkmem_free, NULL, 0,
                           VM_SLEEP); // Kernel virtual address arena

  vmem_seg_arena = vmem_create("vmem_seg", NULL, 0, PAGE_SIZE, NULL, NULL,
                               heap_arena, 0, VM_SLEEP);
  vmem_add(heap_arena, base, size, VM_SLEEP);
  vmem_vmem_arena =
      vmem_create("vmem_vmem", vmem0, sizeof(vmem0), 1, segkmem_alloc,
                  segkmem_free, vmem_seg_arena, 0, VM_SLEEP);
}
