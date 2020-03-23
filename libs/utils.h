//Parametres et defines utiles..
typedef int bool;
#define true 1
#define false 0
#define buffSize 2 //must be >= 2 
#define messageSize 512 //must be >= 2 
#define NPROC 20 //must be >= 2 
#define FOLDER "downloads/"
pid_t child_processes[NPROC];

struct stat fileProperties(char *filename);
void cmd(int connfd);
void echo(char *msg);
void get(char *msg);
