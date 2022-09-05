#ifndef _BARBER_SHOP_H_
#define _BARBER_SHOP_H_

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

#define SUCCESS            0
#define ERR_BASE           0

#define SHM_FILE           "barber_shop_shm"
#define CMD_SIZE           (0xff)


typedef struct _SHMBUR {
   sem_t    process_sem;
   sem_t    thread_sem;
   char     cmd;
   int a;
} _SHMBUR;



#endif