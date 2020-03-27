#include<stdio.h> 
#include<stdlib.h>
#include<sys/wait.h> 
#include<unistd.h> 
#include<string.h>

#define MAXLINE 1024

void print_Current_Dir(void);
void changeDirectory(char *path);

int main() 
{ 
    int stat;
    if (fork()== 0) {
        printf("HC: hello from child\n"); 
        char currDir[MAXLINE];
         
         if (getcwd(currDir, sizeof(currDir)) != NULL) {
            printf("%s\n", currDir);
            printf("len = %d\n", strlen(currDir));
         } else {
            perror("getcwd() failed");
         }
    }
    else
    { 
        printf("HP: hello from parent\n"); 
        wait(&stat); 
        printf("Exit status: %d\n", WEXITSTATUS(stat)); 
        printf("CT: child has terminated\n"); 
    } 
    //changeDirectory("hi");
    printf("Bye\n"); 
    return 0; 
} 

void print_Current_Dir(void) {
   char currDir[MAXLINE];
         
   if (getcwd(currDir, sizeof(currDir)) != NULL) {
      printf("%s\n", currDir);
   } else {
      perror("getcwd() failed");
   }
}

void changeDirectory(char *path) {
   // If only 'cd', go to home directory
   if (path == NULL) {
      chdir(getenv("HOME"));
   } else {
      if (chdir(path) == -1) {
         printf("%s: No such file or directory\n", path);
         return;
      } 
   }
   print_Current_Dir();
}
