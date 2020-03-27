#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glob.h>

// (hint: use GLOB_NOCHECK|GLOB_TILDE as the second parameter of the glob() function)
char **tokenise(char *str, char *sep);

int main(void) {
    glob_t globList;
    
    int glob_return, flags;
    char **original_List, **tokens;
    //int glob_index;
    flags = GLOB_NOCHECK|GLOB_TILDE;
    
    char str[1024] = "wc -l *.c";
    tokens = tokenise(str, " ");
    original_List = tokens;
    int i = 1, len = 0, tokIndex = 1;
    // Need to switch tokens and orginal_list --> malloc a bigger size of the new list cause token is limited in size
    while (original_List[i] != NULL) {

        // Append after first call
        if (i > 1) {
            flags = flags|GLOB_APPEND;
            len = globList.gl_pathc;
        }           

        glob_return = glob(original_List[i], flags, NULL, &globList);     
        if (glob_return != 0) {
            fprintf(stderr,"glob failed!\n");
            exit(1);
        }

        // If token the same after glob --> could not be globbed
        // Otherwise, filename has been expanded
        if (strcmp(original_List[i],globList.gl_pathv[len]) == 0) {
            tokens[tokIndex] = strdup(original_List[i]);
            tokIndex++;
        } else {
            for (; len < globList.gl_pathc; len++) {
                tokens[tokIndex++] = strdup(globList.gl_pathv[len]);
                //tokIndex++;
            }
            
        }
        i++;
        
    }
    for (i = 0; tokens[i] != NULL; i++)
        printf("%s\n", tokens[i]);
    /*for (i = 0; i < globList.gl_pathc; i++) 
        printf("%s\n", globList.gl_pathv[i]);*/
    
    /* Or i can first glob everything onto the struct, then malloc a new
    string or call tokenise to create a new set of tokens by copying each
    element down from the glob struct*/
    /*
    char **buff;
    char str[1024] = "hello tony";
    buff = tokenise(str, " ");
    int result;
    result = glob("*.c", GLOB_NOCHECK|GLOB_TILDE, NULL, &globList);
    result = glob("*.tony", GLOB_NOCHECK|GLOB_TILDE|GLOB_APPEND, NULL, &globList);
    if (result != 0) {
        fprintf(stderr,"glob failed!\n");
        exit(1);
    }
    printf("how many %d\n", globList.gl_pathc);
    int i;
    for (i = 0; i < globList.gl_pathc; i++) {
        printf("%s\n", globList.gl_pathv[i]);
        if (strcmp("*.tony", globList.gl_pathv[i]) == 0) continue;
        if (i == 0) strcpy(buff[0],globList.gl_pathv[i]);
        else sprintf(buff[0], "%s %s", buff[0], globList.gl_pathv[i]);
    }
    
    printf("New string: %s\n", buff[0]);   
        
    result = glob("*.new", GLOB_NOCHECK|GLOB_TILDE|GLOB_APPEND, NULL, &globList);
    printf("how many %d\n", globList.gl_pathc);    */
        
        
    globfree(&globList);


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
