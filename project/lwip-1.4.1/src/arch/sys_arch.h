#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__


#include "os_cpu.h"
#include "os_cfg.h"
#include "ucos_ii.h"
#include "cc.h"


/* HANDLE is used for sys_sem_t but we won't include windows.h */
struct _sys_sem {
  void *sem;
};
typedef struct _sys_sem sys_sem_t;
#define SYS_SEM_NULL NULL
#define sys_sem_valid(sema) (((sema) != NULL) && ((sema)->sem != NULL))
#define sys_sem_set_invalid(sema) ((sema)->sem = NULL)

/* let sys.h use binary semaphores for mutexes */
#define LWIP_COMPAT_MUTEX 1

#ifndef MAX_QUEUE_ENTRIES
#define MAX_QUEUE_ENTRIES 100
#endif
struct lwip_mbox {
  void* sem;
  void* q_mem[MAX_QUEUE_ENTRIES];
  unsigned int head, tail;
  unsigned int msg_num; 
};
typedef struct lwip_mbox sys_mbox_t;
#define SYS_MBOX_NULL NULL
#define sys_mbox_valid(mbox) ((mbox != NULL) && ((mbox)->sem != NULL))
#define sys_mbox_set_invalid(mbox) ((mbox)->sem = NULL)

/* DWORD (thread id) is used for sys_thread_t but we won't include windows.h */
typedef INT8U sys_thread_t;

#endif
