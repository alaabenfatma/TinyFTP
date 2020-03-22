//Parametres et defines utiles..

#define buffSize 256 //must be >= 2 
#define NPROC 20 //must be >= 2 
pid_t child_processes[NPROC];

struct stat fileProperties(char *filename);
void cmd(int connfd);
void echo(char *msg);
void get(char *msg);
