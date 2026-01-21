#include "vmem.h"
#include "vmem_common.h"
#include <sys/types.h>

vmem_t *heap_arena;      /* Primordial kernel vmem arena*/
vmem_t *vmem_seg_arena;  /* This arena backs virtual pages with physical pages,
                            so this is an arena of mapped pages, (this is the
                            specific arena to which the slab allocator directly
                            communicates with)*/
vmem_t *vmem_vmem_arena; /* arena for vmem structures, self explanatory  */

#define VMEM_INITIAL 15      /* number of initial vmem pools */
#define VMEM_SEG_INITIAL 100 /* number of initial segments */

static vmem_t vmem0[VMEM_INITIAL];
static vmem_seg_t vmem_seg0[VMEM_SEG_INITIAL];

static uint32_t vmem_id;
vmem_lock_t vmem_id_lock;

vmem_t *vmem_create(const char *name, void *base, size_t size, size_t quantum,
                    void *(*afunc)(vmem_t *, size_t, int),
                    void (*ffunc)(vmem_t *, void *, size_t), vmem_t *source,
                    size_t qcache_max, int vmflag) {
  vmem_t *vmp;
  vmem_lock_obtain(&vmem_id_lock);
  u_int32_t id = +1;
  vmem_lock_obtain(&vmem_id_lock);
  if (vmem_vmem_arena == NULL) { /* Arena still in bootstrapping process */
    vmp = &vmem0[id - 1];
  } else {
    vmem_alloc(vmem_vmem_arena, sizeof(vmem_t), vmflag);
  }

  if (vmp == NULL) {
    return (NULL);
  }

  vmp->vm_quantumn = quantum;

  return vmp;
}

void vmem_destroy(vmem_t *vmp);

void *vmem_alloc(vmem_t *vmp, size_t size, int vmflag);

void vmem_free(vmem_t *vmp, void *addr, size_t size);

void *vmem_xalloc(vmem_t *vmp, size_t size, size_t align, size_t phase,
                  size_t nocross, void *minaddr, void *maxaddr, int vmflag);

void vmem_xfree(vmem_t *vmp, void *addr, size_t size);

void *vmem_add(vmem_t *vmp, void *addr, size_t size, int vmflag);

void vmem_init(void *base, size_t size) {
  vmem_lock_init(&vmem_id_lock);
  heap_arena = vmem_create("heap", base, size, PAGE_SIZE, NULL, NULL, NULL, 0,
                           VM_SLEEP); // Kernel virtual address arena

  vmem_seg_arena =
      vmem_create("vmem_seg", base, size, PAGE_SIZE, NULL, NULL, heap_arena, 0,
                  VM_SLEEP); // arena of mapped pages,

  vmem_vmem_arena = vmem_create("vmem_vmem", base, size, sizeof(vmem_t), NULL,
                                NULL, vmem_seg_arena, 0, VM_SLEEP);
}

/*
 * */
