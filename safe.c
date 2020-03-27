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
            strcpy(line,buff);
         } else {
            // String passed in from index 1, atoi returns 0 if its not 
            // a number
            int seqNum;
            seqNum = atoi(&line[1]);
            if (seqNum != 0 ) {
               if ((buff = getCommandFromHistory(seqNum)) == NULL) {
                  printf("No command #%d\n", seqNum);
                  continue;
               } else {
                  strcpy(line,buff);
               }
            } else {
               printf("Invalid history substitution\n");
               continue;
            }
         }   
      }
      // NEED TO MAKE SURE SEQNUM DOESNT ACCESS A VALUE LESS THAN 20 IN THE FILE
      /* 
      int seqNum;
      char *end;
      seqNum = (int) strtol(&line[1], end, 10);
      if (end == &line[1]) {
         printf("Invalid history substitution\n");
         continue;
      } else {
         buff = getCommandFromHistory(seqNum);
         if (buff == NULL) {
            printf("No command #%d\n", seqNum);
            continue;
         }
         strcpy(line,buff);
      }
      */
      
      char **args, *pathName;
      args = tokenise(line," ");
      
      // Expand filename wildcards
      args = fileNameExpand(args);
      
      // Handle shell built-ins --> create function called: built_in_Handler(args)
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
         int fd;
         if (checkRedirections(args, "<") == 1) {
            in = 1;
         } else if (checkRedirections(args, ">") == 1) {
            out = 1;
         } else if (checkRedirections(args, "<") == -1 ||
                    checkRedirections(args, ">") == -1) {
            fprintf(stderr, "Invalid i/o redirection\n");
            prompt();
            continue;
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
            int index;
            index = findTokenLength(args) - 1;
            if (in) {
               fd = open(args[index], O_RDONLY);
               if (fd < 0) {
                  perror("Input Redirection");
                  exit(1);
               }
               // Set this to NULL so the file does not get execve'd
               args[index] = NULL;
               dup2(fd, 0);  
               close(fd);
            }
            if (out) {              
               fd = open(args[index], O_WRONLY | O_TRUNC | O_CREAT  
                                    , S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
               if (fd < 0) {
                  perror("Output Redirection");
                  exit(1);
               }
               args[index] = NULL;
               dup2(fd, 1);  
               close(fd);
            }
            execve(pathName,args,envp);
            perror("Execve failed");
         } else {
            wait(&stat);
            printf("--------------------\n");
            printf("Returns %d\n", WEXITSTATUS(stat));            
         }
      }
      
      // cmdLine/built-in exists, can be saved to history
      addToCommandHistory(line,cmdNo);  
      cmdNo++;   
      // Free it here?
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
            //flags = flags|GLOB_APPEND;
      }
      glob_return = glob(tokens[i], flags, NULL, &globList);     
      if (glob_return != 0) {
         fprintf(stderr,"glob failed!\n");
         exit(1);
      }
      i++;        
   }
   char str[MAXLINE*10];
   for (i = 0; i < globList.gl_pathc; i++) {
      if (i == 0) {
         strcpy(str, globList.gl_pathv[i]);
      } else {
         strcat(str, " ");
         strcat(str, globList.gl_pathv[i]);
      }
      //printf("%s\n", globList.gl_pathv[i]);
    }
                
    char **newTokens;    
    newTokens = tokenise(str, " "); 
    /*for (i = 0; i < globList.gl_pathc; i++)
        printf("%s\n", newTokens[i]);*/
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
   //printf("i = %d\n", i);
   else if (i < 2 || strcmp(tokens[i], tokens[tokLen-2]) != 0) {
      return -1;
   }
   // Otherwise, remove '>' or '<' from the tokens
   else {
      tokens[i] = tokens[i+1];
      tokens[i+1] = NULL;
      return 1; 
   }
}

// Helper function to find length of tokens
int findTokenLength(char **tokens) {
   int i;
   for (i = 0; tokens[i] != NULL; i++);

   return i;
}

/////////////////////////////////////////////////////////////////////////// HISTORY.C
// COMP1521 18s2 mysh ... command history
// Implements an abstract data object

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

// This is defined in string.h
// BUT ONLY if you use -std=gnu99
//extern char *strdup(const char *s);

// Command History
// array of command lines
// each is associated with a sequence number

#define MAXHIST 20
#define MAXSTR  200

#define HISTFILE ".mymysh_history"

typedef struct _history_entry {
   int   seqNumber;
   char *commandLine;
} HistoryEntry;

typedef struct _history_list {
   int nEntries;  
   int len;                                  // Added to track array len                      
   HistoryEntry commands[MAXHIST];
} HistoryList;

HistoryList CommandHistory;

// initCommandHistory()
// - initialise the data structure
// - read from .history if it exists

int initCommandHistory()
{
   // TODO //CHECK THIS IMPLEMENTATION BEFORE RUNNING //NOTE: HISTORY STARTS FROM 1 // Make sure to increment sequence no each time cmd is added
   char *h = getenv("HOME");
   char path[MAXSTR], buffer[MAXSTR];
   int lastNo = 0;
   
   // Get HISTFILE from the path and open it
   sprintf(path,"%s/%s", h, HISTFILE);
   FILE *list = fopen(path, "r");
   // Create file if it does not exist --> check this
   if (list == NULL) {
      list = fopen(path, "w+");
   }
   
   int i = 0;
   while (i < MAXHIST && fgets(buffer, MAXSTR, list) != NULL) {
      // Careful of whitespaces --> line is trimmed
      // dont need %3d
      sscanf(buffer, " %3d  %s", &lastNo, buffer);
      CommandHistory.commands[i].seqNumber = lastNo;
      CommandHistory.commands[i].commandLine = strdup(buffer);
      
      i++;
   }
   
   if (i == 0) {
      // Nothing in the file yet, list starts from 1
      CommandHistory.nEntries = 0;
   }  else {
      CommandHistory.nEntries = lastNo;
   }
   CommandHistory.len = i;
     
   return CommandHistory.nEntries+1;         
   
   /*
   char *bigPath = getenv("HOME");
   strcat(bigPath,/);
   strcat(bigPath, HISTFILE);
   FILE * listEntries = fopen(bigPath,"r");*/
}

// addToCommandHistory()
// - add a command line to the history list
// - overwrite oldest entry if buffer is full

void addToCommandHistory(char *cmdLine, int seqNo)
{
   // TODO
   if (CommandHistory.len == MAXHIST) {
      // Slide array down
      int i;
      for (i = 0; i < MAXHIST - 1; i++) 
         CommandHistory.commands[i] = CommandHistory.commands[i+1];
      // i = last item on array
      CommandHistory.commands[i].seqNumber = seqNo;
      CommandHistory.commands[i].commandLine = strdup(cmdLine);
      CommandHistory.len = i + 1;  
   } else {
      int index = CommandHistory.len;
      CommandHistory.commands[index].seqNumber = seqNo;
      CommandHistory.commands[index].commandLine = strdup(cmdLine);
      CommandHistory.len++;      
   }
}

// showCommandHistory()
// - display the list of 

void showCommandHistory(FILE *outf)
{
   // TODO
   /* fopen()
      strcpy(cmdLine,CommandHistory.commands[***].commandLine);
      fprintf(...)*/
   int i;
   for (i = 0; i < CommandHistory.len; i++) {
      fprintf(outf, " %3d  %s\n", CommandHistory.commands[i].seqNumber,
                                CommandHistory.commands[i].commandLine);
   }
}

// getCommandFromHistory()
// - get the command line for specified command
// - returns NULL if no command with this number

char *getCommandFromHistory(int cmdNo)
{
   // TODO
   int i;
   for (i = 0; i < MAXHIST; i++) {
      if (cmdNo == CommandHistory.commands[i].seqNumber) break;    
   }
   
   if (i == MAXHIST) {
      return NULL;
   } else {
      return CommandHistory.commands[i].commandLine;
   }
}

// saveCommandHistory()
// - write history to $HOME/.mymysh_history

void saveCommandHistory()
{
   // TODO
   char path[MAXSTR];
   char *h = getenv("HOME");
   sprintf(path,"%s/%s", h, HISTFILE);
   FILE *list = fopen(path, "w");
   
   showCommandHistory(list);
   
}

// cleanCommandHistory
// - release all data allocated to command history

void cleanCommandHistory()
{
   // TODO
   int i;
   for (i = 0; i < CommandHistory.len; i++)
      free(CommandHistory.commands[i].commandLine);
}
