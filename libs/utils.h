//Parametres et defines utiles..

#define buffSize 200 //must be >= 2 
#define NPROC 20 //must be >= 2 
void echo(int connfd);
void get(int connfd);
struct stat fileProperties(char *filename);
