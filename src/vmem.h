#ifndef _VMEM_H
#include "vmem_common.h"
#include <stddef.h>
#include <stdint.h>

#define VM_SLEEP 0x10
#define VM_NOSLEEP 0x01

typedef struct vmem vmem_t;

struct vmem {
  char vm_name[VMEM_NAMELEN]; /* arena name */
  vmem_lock_t vm_lock;        /* arena lock, defined in vmem_common.h */
  uint32_t vm_id;             /* arena id
                               * ( XXX: unused for now)
                               * */
  vmem_t *vm_source;          /*arena vmem source for importing resources*/
                              // TODO: add KSTAT;
  void *(*vm_source_alloc)(vmem_t *, size_t, int);
  void (*vm_source_free)(vmem_t *, void *, size_t);
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

void vmem_init(void *base, size_t size); // Base and size of the kernal heap

#endif // !_VMEM_H
