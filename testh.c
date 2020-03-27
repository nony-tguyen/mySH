// COMP1521 18s2 mysh ... command history
// Implements an abstract data object

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

int initCommandHistory();
void addToCommandHistory(char *cmdLine, int seqNo);
void showCommandHistory(FILE *histFile);
char *getCommandFromHistory(int cmdNo);
void saveCommandHistory();
void cleanCommandHistory();
void trim(char *str);

int main (void) {
   int entry;
   char buff[MAXSTR];
   char *buffer = NULL;
   
   entry = initCommandHistory();
   while(fgets(buff,MAXSTR,stdin) != NULL) {
      trim(buff);
      if (buff[0] == '!') {
         // '!!' re-executes last command = cmdNo - 1
         if (buff[1] == '!') {
            buffer = getCommandFromHistory(entry-1);
            strcpy(buff,buffer);
            printf("execute last command: %s\n",buff);
         } else {           
            int seqNum;
            char *end;
            seqNum = (int) strtol(&buff[1], &end, 10);
            if (end == &buff[1] || seqNum < 0) {
               printf("Invalid history substitution\n");
               continue;
            } else {
               buffer = getCommandFromHistory(seqNum);
               if (buffer == NULL) {
                  printf("No command #%d\n", seqNum);
                  continue;
               }   
               strcpy(buff,buffer);
               printf("execute command No.: %d %s\n",seqNum,buff);
            }
            
            /*// String passed in from index 1, atoi returns 0 if its not 
            // a number
            int seqNum;
            seqNum = atoi(&buff[1]);
            if (seqNum != 0 ) {
               if ((buffer = getCommandFromHistory(seqNum)) == NULL) {
                  printf("No command #%d\n", seqNum);
                  continue;
               } else {
               
                  strcpy(buff,buffer);
                  printf("Execute command numer: %d %s\n", seqNum, buff);
               }
            } else {
               printf("Invalid history substitution\n");
               continue;
            }*/
            
         }   
      }
      if (strcmp(buff, "h") == 0 || strcmp(buff, "history") == 0) {
         showCommandHistory(stdout);
      }  
      addToCommandHistory(buff, entry);
      entry++;
   }
   printf("End:\n");
   
   saveCommandHistory();
   cleanCommandHistory();

   return 0;
}


// initCommandHistory()
// - initialise the data structure
// - read from .history if it exists

int initCommandHistory()
{
   // TODO //CHECK THIS IMPLEMENTATION BEFORE RUNNING //NOTE: HISTORY STARTS FROM 1
   char buffer[MAXSTR];
   int lastNo = 0;

   FILE *list = fopen("h.txt", "r");
   
   int i = 0;
   while (i < MAXHIST && fgets(buffer, MAXSTR, list) != NULL) {
      // Careful of whitespaces
      // delimiters
      sscanf(buffer, "%d  %[^\t\n]", &lastNo, buffer);
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
      // We scanned in the '\n' character
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
   FILE *list = fopen("h.txt", "w");
   
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
