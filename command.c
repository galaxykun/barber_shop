#include "barber_shop.h"


int main () {
   int   ret      =  SUCCESS;
   int   shm_fd   =  -1;
   int   loop     =  true;

   shm_fd = shm_open(SHM_FILE, O_RDWR, 0);
   if (shm_fd == -1) {
    #ifdef _DEBUG
      printf("shm_open ERROR : %s .\n", strerror(errno));
    #endif

      goto  ERROR;
   }

   shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
   if (shmp == MAP_FAILED) {
    #ifdef _DEBUG
      printf("mmap ERROR : %s .\n", strerror(errno));
    #endif

      goto ERROR;
   }

   printf("Please enter the command. (guest : g | close : c)\n");
   char cmd[CMD_SIZE];
   while (loop) {
      printf("cmd : ");scanf("%s", cmd);
      cmd[CMD_SIZE - 1] = 0;

      if (!strcmp(cmd, "guest") || !strcmp(cmd, "g")) {
         shmp->cmd = 'g';
      }
      else if (!strcmp(cmd, "close") || !strcmp(cmd, "c")) {
         shmp->cmd = 'c';
         loop = false;
      }
      else{
       #ifdef _DEBUG
         printf("cmd is error!\n");
       #endif

         printf("Unknown command! Please try again!\n");
         continue;
      }

      if (sem_post(&(shmp->process_sem)) == -1) {
       #ifdef _DEBUG
         printf("sem_post ERROR : %s .\n", strerror(errno));
       #endif

         goto ERROR;
      }

       #ifdef _DEBUG
         printf("cmd is : %c\n", shmp->cmd);
       #endif
   }


 ERROR:
   if (ret || errno) {
      printf("ret : %d\nerrno : %s .\n", ret, strerror(errno));
   }
   if (shm_fd != -1) {
      close(shm_fd);
      shm_unlink(SHM_FILE);
   }
   if (shmp) {
      if (ret = munmap(shmp, sizeof(*shmp)) == -1) {
         printf("mummap ERROR : %s .\n", strerror(errno));
         exit(errno);
      }
   }


   return ret ? ret : errno;
}