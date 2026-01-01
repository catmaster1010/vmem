#ifndef _VMEM_COMMON_H
#define _VMEM_COMMON_H

#include <threads.h>

#define VMEM_NAMELEN 32    /* Maximum length of a name for an arena */
typedef mtx_t vmem_lock_t; /* We will use a mutex from the threads.h library */

#endif
