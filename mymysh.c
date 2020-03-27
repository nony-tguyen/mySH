// mysh.c ... a small shell
// Started by John Shepherd, September 2018
// Completed by <<YOU>>, September/October 2018

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glob.h>
#include <assert.h>
#include <fcntl.h>
#include "history.h"

// This is defined in string.h
// BUT ONLY if you use -std=gnu99
//extern char *strdup(char *);

// Function forward references

void trim(char *);
int strContains(char *, char *);
char **tokenise(char *, char *);
char **fileNameExpand(char **);
void freeTokens(char **);
char *findExecutable(char *, char **);
int isExecutable(char *);
void prompt(void);

// Functions I added
void print_Current_Dir(void);
void changeDirectory(char *path);
int checkRedirections(char **tokens, char *direct);
int findTokenLength(char **tokens);

// Global Constants

#define MAXLINE 200

// Global Data

/* none ... unless you want some */
// Glob struct - added
glob_t globList;

// Main program
// Set up enviroment and then run main loop
// - read command, execute command, repeat

int main(int argc, char *argv[], char *envp[])
{
   pid_t pid;   // pid of child process
   int stat;    // return status of child
   char **path; // array of directory names
   int cmdNo;   // command number
   int i;       // generic index

   // set up command PATH from environment variable
   for (i = 0; envp[i] != NULL; i++) {
      if (strncmp(envp[i], "PATH=", 5) == 0) break;
   }
   if (envp[i] == NULL)
      path = tokenise("/bin:/usr/bin",":");
   else
      // &envp[i][5] skips over "PATH=" prefix
      path = tokenise(&envp[i][5],":");
#ifdef DBUG
   for (i = 0; path[i] != NULL;i++)
      printf("path[%d] = %s\n",i,path[i]);
#endif

   // initialise command history
   // - use content of ~/.mymysh_history file if it exists

   cmdNo = initCommandHistory();

   // main loop: print prompt, read line, execute command

   char line[MAXLINE];
   prompt();
   while (fgets(line, MAXLINE, stdin) != NULL) {
      trim(line); // remove leading/trailing space

      // TODO
      // Code to implement mainloop goes here
      // Uses
      // - addToCommandHistory()
      // - showCommandHistory()
      // - and many other functions
      // TODO
      
      if (strcmp(line,"") == 0) { printf("mysh$ "); continue; }
      
      // History substitution
      char *buff;
      if (line[0] == '!') {
         // '!!' re-executes last command = cmdNo - 1
         if (line[1] == '!') {
            buff = getCommandFromHistory(cmdNo-1);
            // Special case if '!!' used when theres no commands in file
            if (buff == NULL) {
               printf("No command #%d\n", cmdNo-1);
               prompt();
               continue;
            }
            strcpy(line,buff);
         } else {
            int seqNum;
            char *end;
            
            // Using strtol to convert string to int '!SeqNo'
            // atoi has no error checking i.e. if !0 was entered
            seqNum = (int) strtol(&line[1], &end, 10);
            if (end == &line[1] || line[1] == ' ' || seqNum < 0) {
                printf("Invalid history substitution\n");
                prompt();
                continue;
            } else {
               buff = getCommandFromHistory(seqNum);
               if (buff == NULL) {
                  printf("No command #%d\n", seqNum);
                  prompt();
                  continue;
               }
               strcpy(line,buff);
            }
         }   
      }
      
      // Tokenise
      char **args, *pathName;
      args = tokenise(line," ");
      
      // Expand filename wildcards
      args = fileNameExpand(args);
      
      // Handle shell built-ins 
      if (strcmp(args[0], "exit") == 0) {
         break;
         
      } else if (strcmp(args[0], "h") == 0 || 
                 strcmp(args[0], "history") == 0) {
      
         showCommandHistory(stdout);
         
      } else if (strcmp(args[0], "pwd") == 0) {
         print_Current_Dir(); 
         
      } else if (strcmp(args[0], "cd") == 0) {
         changeDirectory(args[1]);
         
      } 
      
      // Otherwise, it is a command
      else {
         // Checking input/output redirections
         int in = 0, out = 0;
         int fd, index;
         index = findTokenLength(args) - 1;
         if (checkRedirections(args, "<") == -1 ||
             checkRedirections(args, ">") == -1) {
            
            fprintf(stderr, "Invalid i/o redirection\n");
            prompt();
            continue;
         } else if (checkRedirections(args, "<") == 1) {
            in = 1;
            // Obtain fd of file for input redirection
            fd = open(args[index], O_RDONLY);
            if (fd < 0) {
               fprintf(stderr,"Input redirection: "
                              "No such file or directory\n");
               prompt();
               continue;
            }
         } else if (checkRedirections(args, ">") == 1) {
            out = 1;
            // Open flag to write to file, permissions = 0664            
            fd = open(args[index], O_WRONLY | O_TRUNC | O_CREAT  
                                 , S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            if (fd < 0) {
               fprintf(stderr,"Output redirection: "
                              "No such file or directory\n");
               prompt();
               continue;
            }
            
         }  
         
         // Check if command exists
         pathName = findExecutable(args[0],path);
         if (pathName == NULL) {
            printf("%s: Command not found\n", args[0]);
            prompt();
            continue;
         }
         
         // Set up format
         printf("Running %s ...\n", pathName);     
         printf("--------------------\n");
          
         // Execute the file
         pid = fork();
         if (pid == -1) {
            perror("Fork failed");
            exit(1);
         } else if (pid == 0) {
            // Handle any I/O redirections by dup2'ing the new fd to
            // stdin (0) or stdout (1)
            if (in) {
               // Set to NULL so '<' and the file does not get execve'd
               args[index-1] = NULL;
               args[index] = NULL;
               dup2(fd, 0);  
               close(fd);
            }
            if (out) {  
               args[index-1] = NULL;
               args[index] = NULL;
               dup2(fd, 1);  
               close(fd);
            }
            execve(pathName,args,envp);
            
            // If it reaches here, execve did not work
            fprintf(stderr,"%s: unknown type of executable\n",pathName);
            exit(255);
         } else {
            wait(&stat);
            printf("--------------------\n");
            printf("Returns %d\n", WEXITSTATUS(stat));            
         }
      }
      
      // cmdLine/built-in exists, can be saved to history
      addToCommandHistory(line,cmdNo);  
      cmdNo++;   
      // Free items before next command 
      globfree(&globList);
      freeTokens(args);
      prompt();
   }
   saveCommandHistory();
   cleanCommandHistory();
   printf("\n");
   return(EXIT_SUCCESS);
}

// fileNameExpand: expand any wildcards in command-line args
// - returns a possibly larger set of tokens
char **fileNameExpand(char **tokens)
{
   // TODO
   int flags, glob_return;
   flags = GLOB_NOCHECK|GLOB_TILDE;
    
   int i = 0;
   while (tokens[i] != NULL) {
      if (i > 0) {
         flags = GLOB_NOCHECK|GLOB_TILDE|GLOB_APPEND;
      }
      // Store each token onto the glob struct and expand any wildcards
      glob_return = glob(tokens[i], flags, NULL, &globList);     
      if (glob_return != 0) {
         fprintf(stderr,"glob failed!\n");
         exit(1);
      }
      i++;        
   }
   
   // Iterate through and copy from the glob struct into a buffer
   char str[MAXLINE*10];
   for (i = 0; i < globList.gl_pathc; i++) {
      if (i == 0) {
         strcpy(str, globList.gl_pathv[i]);
      } else {
         strcat(str, " ");
         strcat(str, globList.gl_pathv[i]);
      }
    }
    // Tokenise this new line           
    char **newTokens;    
    newTokens = tokenise(str, " "); 
    
    //No longer using these tokens
    free(tokens);

    return newTokens;
}

// findExecutable: look for executable in PATH
char *findExecutable(char *cmd, char **path)
{
      char executable[MAXLINE];
      executable[0] = '\0';
      if (cmd[0] == '/' || cmd[0] == '.') {
         strcpy(executable, cmd);
         if (!isExecutable(executable))
            executable[0] = '\0';
      }
      else {
         int i;
         for (i = 0; path[i] != NULL; i++) {
            sprintf(executable, "%s/%s", path[i], cmd);
            if (isExecutable(executable)) break;
         }
         if (path[i] == NULL) executable[0] = '\0';
      }
      if (executable[0] == '\0')
         return NULL;
      else
         return strdup(executable);
}

// isExecutable: check whether this process can execute a file
int isExecutable(char *cmd)
{
   struct stat s;
   // must be accessible
   if (stat(cmd, &s) < 0)
      return 0;
   // must be a regular file
   //if (!(s.st_mode & S_IFREG))
   if (!S_ISREG(s.st_mode))
      return 0;
   // if it's owner executable by us, ok
   if (s.st_uid == getuid() && s.st_mode & S_IXUSR)
      return 1;
   // if it's group executable by us, ok
   if (s.st_gid == getgid() && s.st_mode & S_IXGRP)
      return 1;
   // if it's other executable by us, ok
   if (s.st_mode & S_IXOTH)
      return 1;
   return 0;
}

// tokenise: split a string around a set of separators
// create an array of separate strings
// final array element contains NULL
char **tokenise(char *str, char *sep)
{
   // temp copy of string, because strtok() mangles it
   char *tmp;
   // count tokens
   tmp = strdup(str);
   int n = 0;
   strtok(tmp, sep); n++;
   while (strtok(NULL, sep) != NULL) n++;
   free(tmp);
   // allocate array for argv strings
   char **strings = malloc((n+1)*sizeof(char *));
   assert(strings != NULL);
   // now tokenise and fill array
   tmp = strdup(str);
   char *next; int i = 0;
   next = strtok(tmp, sep);
   strings[i++] = strdup(next);
   while ((next = strtok(NULL,sep)) != NULL)
      strings[i++] = strdup(next);
   strings[i] = NULL;
   free(tmp);
   return strings;
}

// freeTokens: free memory associated with array of tokens
void freeTokens(char **toks)
{
   for (int i = 0; toks[i] != NULL; i++)
      free(toks[i]);
   free(toks);
}

// trim: remove leading/trailing spaces from a string
void trim(char *str)
{
   int first, last;
   first = 0;
   while (isspace(str[first])) first++;
   last  = strlen(str)-1;
   while (isspace(str[last])) last--;
   int i, j = 0;
   for (i = first; i <= last; i++) str[j++] = str[i];
   str[j] = '\0';
}

// strContains: does the first string contain any char from 2nd string?
int strContains(char *str, char *chars)
{
   for (char *s = str; *s != '\0'; s++) {
      for (char *c = chars; *c != '\0'; c++) {
         if (*s == *c) return 1;
      }
   }
   return 0;
}

// prompt: print a shell prompt
// done as a function to allow switching to $PS1
void prompt(void)
{
   printf("mymysh$ ");
}

// prints the current directory for the 'pwd' command
void print_Current_Dir(void) {
   char currDir[MAXLINE];
         
   if (getcwd(currDir, sizeof(currDir)) != NULL) {
      printf("%s\n", currDir);
   } else {
      perror("getcwd() failed");
   }
}

// Check if path exists and changes to that directory: cd [PATH] 
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

// Check tokens for any input or output redirections '<' or '>'
// If found, makes sure it is the second last token to be valid command
// and set up token to be executed 
int checkRedirections(char **tokens, char *direct) {
   int i, tokLen;
   for (i = 0; tokens[i] != NULL; i++) {
      if (strcmp(tokens[i], direct) == 0) break;
   }  
   tokLen = findTokenLength(tokens);
   
   // If it doesn't exist in the tokens at all 
   if (i == tokLen) {
      return 0;     
   }
   // If '>' or '<' is not the second last token: invalid
   else if (i < 1 || strcmp(tokens[i], tokens[tokLen-2]) != 0) {
      return -1;
   }
   // Otherwise, remove '>' or '<' from the tokens
   else {
      //tokens[i] = tokens[i+1];
      //tokens[i+1] = NULL;
      return 1; 
   }
}

// Helper function to find length of tokens
int findTokenLength(char **tokens) {
   int i;
   for (i = 0; tokens[i] != NULL; i++);

   return i;
}
