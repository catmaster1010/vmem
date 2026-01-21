#include "vmem.h"
#include "vmem_common.h"
#include <stdlib.h>

int main() {
  void *base =
      malloc(PAGE_SIZE *
             500); /* Pretend this is the mapped virtual adress range
                      that has yet to be backed by physical adresses, to which
                      it will be backed through calls to segkmem_alloc*/
  vmem_init(base, PAGE_SIZE * 500);
  vmem_debug("Vmem initialized");

  return 0;
}
