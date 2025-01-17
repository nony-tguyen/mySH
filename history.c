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
   int len;                                // Added to track array len                      
   HistoryEntry commands[MAXHIST];
} HistoryList;

HistoryList CommandHistory;

// initCommandHistory()
// - initialise the data structure
// - read from .history if it exists

int initCommandHistory()
{
   // TODO 
   char *h = getenv("HOME");
   char path[MAXSTR], buffer[MAXSTR];
   int lastNo = 0;
   
   // Get HISTFILE from the path and open it
   sprintf(path,"%s/%s", h, HISTFILE);
   FILE *list = fopen(path, "r");
   // Create file if it does not exist 
   if (list == NULL) {
      list = fopen(path, "w+");
   }
   
   // Read from file and store into struct 
   int i = 0;
   while (i < MAXHIST && fgets(buffer, MAXSTR, list) != NULL) {
      // Need delimiters to copy string with spaces
      sscanf(buffer, "%d  %[^\n]", &lastNo, buffer);
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
      // Array full, slide array down
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
