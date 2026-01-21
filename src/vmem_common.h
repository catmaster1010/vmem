#ifndef _VMEM_COMMON_H
#define _VMEM_COMMON_H
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

#define PAGE_SIZE                                                              \
  sysconf(_SC_PAGE_SIZE) /*The size of a page defined on the system */

#define VMEM_NAMELEN                                                           \
  32 /* Maximum length of a name for an arena, this should be sufficient for   \
        most implementations  */

#define vmem_lock_init(l)                                                      \
  mtx_init(l, mtx_plain) /* A function taking a pointer to a vmem_lock_t which \
                            then initializes the lock*/

#define vmem_lock_obtain(l)                                                    \
  mtx_lock(l) /* A function taking a pointer to an initialized vmem_lock_t     \
      which then obains the lock, if the lock is already held, it is up to the \
      implementation to define such behaviour, (spinning, blocking the current \
      thread, etc.) */

#define vmem_lock_release(l)                                                   \
  mtx_unlock(                                                                  \
      l) /* A function taking a pointer to an initialized, obtained,           \
                                                                        \      \
            vmem_lock_t which then releases the lock */
#define vmem_lock_destroy(l)                                                   \
  mtx_destroy(l) /* A function taking a pointer to a vmem_lock_t */

typedef mtx_t vmem_lock_t; /* We will use a mutex from the threads.h library */

#define vmem_debug                                                             \
  printf /* optional print function used for debbuging purposes */

#define vmem_frame_alloc                                                       \
  malloc(PAGE_SIZE) /* Allocate a frame of PHYSICAL memmory, used by           \
      segkmem_alloc when it invokes vmem_alloc() on                            \
      heap_arena to get a virtual adress and then backs                        \
      it with physical pages, how you choose to get a physical page is up to   \
      you, eg. first fit, best fit, worst fit   */

#define vmem_frame_map(                                                        \
    addr, frame) /* Provide a function to which addr is mapped to frame */

#endif
