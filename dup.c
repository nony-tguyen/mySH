#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(void)
{
    int fd, pid;
  
    pid = fork();
    if (pid != 0) {
        int stat;
        wait(&stat);
        printf("Parent: %d ... \n", getpid());
         


    } else {
        fd = open("h.txt", O_RDONLY);
        if (fd < 0) {
            perror("h.txt");
            exit(1);
        }
        dup2(fd, 0);  
        close(fd);
        /* cat < "file.txt */
        execlp("cat", "cat", NULL);
    }
    
    return 0;
}




/* 
int p[2], pid;
   assert(pipe(p) == 0);
   pid = fork();
   assert(pid >= 0);
   if (pid != 0) {  // parent
      printf("Parent (%d) ready ...\n",getpid());
      if (dup2(p[0], 0) < 0)
         { printf("Dup failed\n"); exit(1); }
      close(p[1]);
      char line[BUFSIZ]; int size, max = 0;
      while (fgets(line,BUFSIZ,stdin) != NULL) {
         printf("got: %s", line);
         if (strncmp(line,"total",5) == 0) continue;
         sscanf(&line[25], "%d", &size);
         if (size > max) max = size;
      }
      printf("Largest file has %d bytes\n", max);
   }
   else {           // child
      char *args[3] = { "/bin/ls", "-l", NULL };
      if (dup2(p[1], 1) < 0)
         { printf("Dup failed\n"); exit(1); }
      close(p[0]);
      printf("Child (%d) ready ...\n",getpid());
      execve("/bin/ls", args, NULL);
      printf("Exec failed\n");
   }
*/
