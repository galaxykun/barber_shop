#include "barber_shop.h"

int main(){
   int   ret      =  SUCCESS;
   int   shm_fd   =  -1;

   _SHMBUR *shmp  = NULL;

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

   while(1){
      printf("wait...\n");
      sem_wait(&(shmp->process_sem));
      printf("get!\n");
      if(shmp->cmd == 'c'){
         printf("close!\n");
         break;
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


   if(shmp){
      if(ret = munmap(shmp, sizeof(*shmp)) == -1){
         printf("mummap ERROR : %s .\n", strerror(errno));
         exit(errno);
      }
   }

   return ret ? ret : errno;
}