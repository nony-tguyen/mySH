#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glob.h>

// (hint: use GLOB_NOCHECK|GLOB_TILDE as the second parameter of the glob() function)
char **tokenise(char *str, char *sep);

int main(void) {
    glob_t globList;
    
    int flags, glob_return;
    flags = GLOB_NOCHECK|GLOB_TILDE;
    char str[1024] = "wc -l *.c";
    char **tokens;
    tokens = tokenise(str, " ");
    
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
    //char temp[1024];
    for (i = 0; i < globList.gl_pathc; i++) {
        if (i == 0) {
            strcpy(str, globList.gl_pathv[i]);
        } else {
            strcat(str, " ");
            strcat(str, globList.gl_pathv[i]);
        }
      //printf("%s\n", globList.gl_pathv[i]);
    }
        
        
   // printf("%s\n", str);
    char **newTokens;
    
    newTokens = tokenise(str, " "); 
    for (i = 0; i < globList.gl_pathc; i++)
        printf("%s\n", newTokens[i]);
    
    
    return 0;
}

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
   //assert(strings != NULL);
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
