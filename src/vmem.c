#include "vmem.h"

vmem_t *vmem_create(const char *name, void *base, size_t size, size_t quantum,
                    void *(*afunc)(vmem_t *, size_t, int),
                    void (*ffunc)(vmem_t *, void *, size_t), vmem_t *source,
                    size_t qcache_max, int vmflag) {
  // TODO: make vmem vmem arena
}

void vmem_destroy(vmem_t *vmp);

void *vmem_alloc(vmem_t *vmp, size_t size, int vmflag);

void vmem_free(vmem_t *vmp, void *addr, size_t size);

void *vmem_xalloc(vmem_t *vmp, size_t size, size_t align, size_t phase,
                  size_t nocross, void *minaddr, void *maxaddr, int vmflag);

void vmem_xfree(vmem_t *vmp, void *addr, size_t size);

void *vmem_add(vmem_t *vmp, void *addr, size_t size, int vmflag);
