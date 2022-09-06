#include "barber_shop.h"



int main(int argc, char *argv[]){
   int   ret      =  SUCCESS;
   int   shm_fd   =  -1;

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

   shmp->queue = (int*)malloc(BENCH_LIMIT * sizeof(int));
   if(!shmp->queue){
    #ifdef _DEBUG
      printf("shmp->queue malloc ERROR : %s .\n", strerror(errno));
    #endif

      goto ERROR;
   }
   shmp->head = 0;
   shmp->tail = 0;

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

   pthread_mutex_init(&mutex, NULL);

   for(int i = 0; i < barber_num; i++){
      if(ret = pthread_create(&pth[i], NULL, thread_func, &barber_id)){
       #ifdef _DEBUG
         printf("pthread_create ERROR : %s .\n", strerror(errno));
       #endif

         goto ERROR;
      }
   }

   int   guest_num   =  0;
   int   loop        =  true;
   int   val         =  0;
   while(loop){
    #ifdef _DEBUG
      printf("wait...\n");
    #endif

      if(sem_wait(&(shmp->process_sem)) == -1){
       #ifdef _DEBUG
         printf("sem_wait process_sem loop ERROR : %s .\n", strerror(errno));
       #endif

         goto ERROR;
      }

      if(shmp->cmd == 'c'){
       #ifdef _DEBUG
         printf("close!\n");
       #endif

         if(shmp->head != shmp->tail || val == BENCH_LIMIT){
            printf("guest ");
            if(val == BENCH_LIMIT){
               printf("%d ", shmp->queue[shmp->head]);
               shmp->head++;
            }
            while(shmp->head != shmp->tail){
               printf("%d ", shmp->queue[shmp->head]);
               shmp->head++;

               if(shmp->head >= BENCH_LIMIT){
                  shmp->head = 0;
               }
            }
            printf("leaves the bench!\n");
         }
         loop = false;
      }
      else{
         guest_num++;
         printf("received guest %d, ", guest_num);

       #ifdef _DEBUG
         printf("get!\n");
       #endif

         if(sem_getvalue(&(shmp->thread_sem), &val) == -1){
          #ifdef _DEBUG
            printf("sem_getvalue ERROR : %s .\n", strerror(errno));
          #endif

            goto ERROR;
         }
 #ifdef _DEBUG
   printf("\n----------\nval : %d\n----------\n", val);
 #endif
         if(val >= BENCH_LIMIT){
          #ifdef _DEBUG
          #endif

            printf("FULL! guest %d leave!\n", guest_num);
            continue;
         }
         else{
            pthread_mutex_lock(&mutex);
            shmp->queue[(shmp->tail)++] = guest_num;

            if(shmp->tail >= BENCH_LIMIT){
               shmp->tail = 0;
            }

            bool  busy  =  true;
            for(int i = 0; i < barber_num && !val; i++){
               if(!barber_stat[i]){
                  #ifdef _DEBUG
                  printf("%d is not busy!\n", i);
                  #endif
                  busy = false;

                  break;
               }
            }
            pthread_mutex_unlock(&mutex);

            if(busy){
               printf("guest %d waiting in the bench.\n", guest_num);
            }
         }
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
   pthread_mutex_destroy(&mutex);
   sem_destroy(&shmp->process_sem);
   sem_destroy(&shmp->thread_sem);
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
   if(shmp->queue){
      free(shmp->queue);
   }
   if(shmp){
      if(ret = munmap(shmp, sizeof(*shmp)) == -1){
         printf("mummap ERROR : %s .\n", strerror(errno));
         exit(errno);
      }
   }

   return ret ? ret : errno;
}

void *thread_func(void *arg){
   pthread_mutex_lock(&mutex);
   int   index =  (*(int*)arg)++;
   pthread_mutex_unlock(&mutex);

   while(1){
      pthread_mutex_lock(&mutex);
      barber_stat[index] = STAT_SLEEP;
      pthread_mutex_unlock(&mutex);

      printf("barber number %d is sleep...\n", index);

      if(sem_wait(&(shmp->thread_sem)) == -1){
       #ifdef _DEBUG
         printf("sem_wait thread_sem loop thread_func ERROR : %s .\n", strerror(errno));
       #endif

         pthread_exit(NULL);
      }

      if(shmp->cmd == 'c'){
       #ifdef _DEBUG
         printf("barber %d is exit!\n", index);
       #endif

         if(sem_post(&(shmp->thread_sem)) == -1){
          #ifdef _DEBUG
            printf("sem_post thread_sem ERROR : %s .\n", strerror(errno));
          #endif

            exit(0);
         }
         break;
      }
      else{
         pthread_mutex_lock(&mutex);
         int guest_num = shmp->queue[(shmp->head)++];

         if(shmp->head >= BENCH_LIMIT){
            shmp->head = 0;
         }

         barber_stat[index] = STAT_DOING;
         pthread_mutex_unlock(&mutex);

         printf("barber %d is serving guest %d .\n", index, guest_num);

         //do something
       //#ifdef _DEBUG
         #include <time.h>
         srand(time(NULL));
         for(int i = 0; i <= 100;){
            printf("barber %d is %3d%% complete...\n", index, i);
            i += 10;
            sleep(rand() % 5);
         }
       //#endif
      }
   }


   pthread_exit(NULL);
}
