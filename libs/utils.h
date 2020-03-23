//Parametres et defines utiles..

/* --------------------------------- COLORS --------------------------------- */

#define RED   "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW   "\x1B[33m"
#define BLUE   "\x1B[34m"
#define MAGENTA   "\x1B[35m"
#define CYAN   "\x1B[36m"
#define WHITE   "\x1B[37m"
#define RESET "\x1B[0m"
typedef int bool;
#define true 1
#define false 0
#define buffSize 256 //must be >= 2 
#define messageSize 512 //must be >= 2 
#define NPROC 20 //must be >= 2 
#define FOLDER "downloads/"

pid_t child_processes[NPROC];

struct stat fileProperties(char *filename);
void cmd(int connfd);
void echo(char *msg);
void get(char *msg);
void resume();
char *strremove(char *str, const char *sub);
char *nameOfCrashedFile();
typedef struct {
    long position;
    size_t size;
    char *buff;
}data_from_file;