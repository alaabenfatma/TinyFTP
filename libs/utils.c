#include "../libs/csapp.h"
#include "../libs/utils.h"

struct stat fileProperties(char *filename){
      struct stat st;
      stat(filename, &st);
      return st;
}
char *strremove(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}
char *nameOfCrashedFile(){
      FILE *f;
      f = fopen("crash.log","r");
      char * fname = malloc(messageSize);
      fscanf(f,"%s",fname);
      fname = strtok(fname,",");
      fclose(f);
      return fname;
}
