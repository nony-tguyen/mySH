#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char **tokenise(char *str, char *sep);

int main(void) {
    char str[100] = "hello tony";
    
   /* char **buff;
    char **temp;
    buff = tokenise(str, " ");
    int i = 0;
    temp = buff;
    while (temp[i] != NULL) {
        buff[i+1] = strdup("hi");
        printf("%s\n", buff[i]);
        printf("%s\n", buff[i+1]);
        i++;
    }
    free(buff);*/
    char new[100] = "-add me";
    sprintf(str, "%s %s", str, new);
    printf("%s\n", str);
    printf("len = %d\n", strlen(str));
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
