#include "vmem.h"
#include "vmem_common.h"
#include <sys/types.h>

vmem_t *heap_arena;      /* Primordial kernel vmem arena*/
vmem_t *vmem_seg_arena;  /* This arena backs virtual pages with physical pages,
                            so this is an arena of mapped pages, (this is the
                            specific arena to which the slab allocator directly
                            communicates with)*/
vmem_t *vmem_vmem_arena; /* arena for vmem structures, self explanatory  */

static vmem_t vmem0[VMEM_INITIAL];
static vmem_seg_t vmem_seg0[VMEM_SEG_INITIAL];

static uint32_t vmem_id;
vmem_lock_t vmem_id_lock;

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
  vmem_lock_obtain(&vmem_id_lock);

  if (vmem_vmem_arena == NULL) { /* Arena still in bootstrapping process */
    vmp = &vmem0[id - 1];
  } else {
    vmem_alloc(vmem_vmem_arena, sizeof(vmem_t), vmflag);
  }

  // TODO : strcpy name to  name to vmp->vm_name;

  if (vmp == NULL) {
    return (NULL);
  }

  vmp->vm_quantumn = quantum;
  vmem_lock_init(&vmp->vm_lock);

  /* Init the free list */
  for (int i = 0; i < VMEM_FREELIST_LEN; i++) {
    vsp = vmp->vm_freelist[i];
    vsp->start = 0;
    vsp->end = 0;
    vsp->knext = (vsp + 1);
    vsp->kprev = (vsp - 1);
  }
  vmp->vm_freelist[0]->prev = NULL;
  vmp->vm_freelist[VMEM_FREELIST_LEN - 1]->knext = NULL;

  // TODO: quantum cache shenanigans
  //
  // TODO: add initial span, [base, base+size) to the arena

  return vmp;
}

void vmem_destroy(vmem_t *vmp);

void vmem_free(vmem_t *vmp, void *addr, size_t size);

void vmem_xfree(vmem_t *vmp, void *addr, size_t size);

void *vmem_add(vmem_t *vmp, void *addr, size_t size, int vmflag);

void vmem_init(void *base, size_t size) {
  vmem_lock_init(&vmem_id_lock);
  heap_arena = vmem_create("heap", base, size, PAGE_SIZE, NULL, NULL, NULL, 0,
                           VM_SLEEP); // Kernel virtual address arena

  vmem_seg_arena =
      vmem_create("vmem_seg", NULL, 0, PAGE_SIZE, NULL, NULL, heap_arena, 0,
                  VM_SLEEP); // arena of mapped pages,

  vmem_vmem_arena = vmem_create("vmem_vmem", vmem0, sizeof(vmem0), 1, NULL,
                                NULL, vmem_seg_arena, 0, VM_SLEEP);
}
