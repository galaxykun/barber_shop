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
#include <pthread.h>

#define SUCCESS            0
#define ERR_BASE           0

#define SHM_FILE           "barber_shop_shm"
#define STAT_SLEEP         false
#define STAT_DOING         true
#define CMD_SIZE           (0xff)
#define BENCH_LIMIT        (10)


typedef struct _SHMBUR {
   sem_t    process_sem;
   sem_t    thread_sem;
   char     cmd;
   int      *queue;
   int      head;
   int      tail;
} _SHMBUR;

int barber_num = 3;
_SHMBUR *shmp  = NULL;
int barber_id  = 0;
static bool *barber_stat;
pthread_mutex_t mutex;

void *thread_func (void *data);

#endif