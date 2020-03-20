#include "../libs/csapp.h"
#include "../libs/utils.h"

struct stat fileProperties(char *filename){
      struct stat st;
      stat(filename, &st);
      return st;
}