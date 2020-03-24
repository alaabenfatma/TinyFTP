#include <dirent.h>
#include "csapp.h"
/* ----------------------- The super global directory ----------------------- */
DIR *current_directory;
/* -------------- Universal stuff I wrote while eating cookies -------------- */
#define buffSize 256    //must be >= 2
#define messageSize 512 //must be >= 2
#define NPROC 5         //must be >= 2
pid_t child_processes[NPROC];
#define FOLDER "downloads/"
char EOF_BUFF;
/* ------------------------------ PROGRESS BAR ------------------------------ */

void printProgress(ssize_t downloaded, ssize_t size);
double percentage(double size, double downloaded);

/* --------------------------------- COLORS --------------------------------- */
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"
typedef int bool;
#define true 1
#define false 0



/* ---------------------------------- utils --------------------------------- */
int is_file(char *path);
/* -------------------------- Server-side funtions -------------------------- */

struct stat fileProperties(char *filename);
void cmd(int connfd);
void echo(char *msg);
void get(char *msg);
void resume();
char *strremove(char *str, const char *sub);
char *nameOfCrashedFile();
long sizeOfCrashedFile();

/* --------------------------- Shell-like commands -------------------------- */
void ls();


/* ---------------------------------- CMDS ---------------------------------- */
char *getFirstArgument(char cmd[]);
int StartsWith(const char *a, const char *b);