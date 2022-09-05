#include "barber_shop.h"

static bool *barber_stat;

int main(int argc, char *argv[]){
   int   ret      =  SUCCESS;
   int   shm_fd   =  -1;

   _SHMBUR *shmp  = NULL;

   if(argc > 1){
      barber_num = atoi(argv[1]);
      if(!barber_num){
         barber_num = 3;
      }
   }
   pthread_t pth[barber_num];
   barber_stat = (bool*)calloc(barber_num, sizeof(bool));
   if(!barber_stat){
    #ifdef _DEBUG
      printf("barber_stat malloc ERROR : %s .\n", strerror(errno));
    #endif

      goto ERROR;
   }

   umask(S_IWOTH);
   shm_fd = shm_open(SHM_FILE, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IROTH);

   if(shm_fd == -1){
    #ifdef _DEBUG
      printf("shm_open ERROR : %s .\n", strerror(errno));
    #endif

      goto  ERROR;
   }

   if (ftruncate(shm_fd, sizeof(_SHMBUR)) == -1){
    #ifdef _DEBUG
      printf("ftruncate ERROR : %s .\n", strerror(errno));
    #endif

      goto ERROR;
   }

   shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
   if (shmp == MAP_FAILED){
    #ifdef _DEBUG
      printf("mmap ERROR : %s .\n", strerror(errno));
    #endif

      goto ERROR;
   }

   if (sem_init(&(shmp->process_sem), 1, 0) == -1){
    #ifdef _DEBUG
      printf("sem_init process_sem ERROR : %s .\n", strerror(errno));
    #endif

      goto ERROR;
   }
   if (sem_init(&(shmp->thread_sem), 0, 0) == -1){
    #ifdef _DEBUG
      printf("sem_init thread_sem ERROR : %s .\n", strerror(errno));
    #endif

      goto ERROR;
   }

   for(int i = 0; i < barber_num; i++){
      if(ret = pthread_create(&pth[i], NULL, thread_func, NULL)){
       #ifdef _DEBUG
         printf("pthread_create ERROR : %s .\n", strerror(errno));
       #endif

         goto ERROR;
      }
   }

   int   guest_num   =  0;
   int   loop        =  true;
   while(loop){
    #ifdef _DEBUG
      printf("wait...\n");
    #endif

      if(sem_wait(&(shmp->process_sem)) == -1){
       #ifdef _DEBUG
         printf("sem_init thread_sem loop ERROR : %s .\n", strerror(errno));
       #endif

         goto ERROR;
      }
      guest_num++;
      printf("received guest %d, ", guest_num);

    #ifdef _DEBUG
      printf("get!\n");
    #endif

      if(shmp->cmd == 'c'){
       #ifdef _DEBUG
         printf("close!\n");
       #endif

         loop = false;
      }
      else{
         int val = 0;
         if(sem_getvalue(&(shmp->thread_sem), &val) == -1){
          #ifdef _DEBUG
            printf("sem_getvalue ERROR : %s .\n", strerror(errno));
          #endif

            goto ERROR;
         }

         if(val > BENCH_LIMIT){
          #ifdef _DEBUG
          #endif

            printf("FULL! guest %d leave!\n", guest_num);
         }
         else{
            bool  busy  =  true;
            if(val == 0){
               for(int i = 0; i < barber_num; i++){
                  if(!barber_stat[i]){
                   #ifdef _DEBUG
                     printf("%d is not busy!\n", i);
                   #endif
                     busy = false;

                     break;
                  }
               }
            }

            if(busy){
               printf("guest %d waiting in the bench.\n", guest_num);
            }
         }

       #ifdef _DEBUG
         printf("\n----------\nval : %d\n----------\n", val);
       #endif
      }

      if(sem_post(&(shmp->thread_sem)) == -1){
       #ifdef _DEBUG
         printf("sem_post thread_sem ERROR : %s .\n", strerror(errno));
       #endif

         goto ERROR;
      }
   }

   for(int i = 0; i < barber_num; i++){
      if(pthread_join(pth[i], NULL)){
       #ifdef _DEBUG
         printf("pthread_join ERROR : %s .\n", strerror(errno));
       #endif

         goto ERROR;
      }
   }


 ERROR:
   if(ret || errno){
      printf("ret : %d\nerrno : %s .\n", ret, strerror(errno));
   }
   if(shm_fd != -1){
      close(shm_fd);
      shm_unlink(SHM_FILE);
   }
   if(barber_stat){
      free(barber_stat);
   }

   if(shmp){
      if(ret = munmap(shmp, sizeof(*shmp)) == -1){
         printf("mummap ERROR : %s .\n", strerror(errno));
         exit(errno);
      }
   }

   return ret ? ret : errno;
}

void *thread_func(void *data){




   pthread_exit(NULL);
}
