#include "vmem.h"
#include "vmem_common.h"
#include <stdlib.h>

int main() {
  void *base = malloc(PAGE_SIZE * 500);
  vmem_init(base, PAGE_SIZE * 500);
  vmem_debug("Vmem initialized");

  return 0;
}
