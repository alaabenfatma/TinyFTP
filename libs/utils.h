#include <dirent.h>
#include "csapp.h"
/* ----------------------- The super global directory ----------------------- */
char global_path[FILENAME_MAX];
DIR *current_directory;
/* -------------- Universal stuff I wrote while eating cookies -------------- */
#define buffSize 256    //must be >= 2
#define messageSize 512 //must be >= 2
#define NPROC 5         //must be >= 2
pid_t child_processes[NPROC];
#define FOLDER "downloads/"

/* ------------------------------ PROGRESS BAR ------------------------------ */
void printProgress(char *msg,ssize_t downloaded, ssize_t size);
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
char *getFirstArgument(char cmd[]);
int StartsWith(const char *a, const char *b);
char *strremove(char *str, const char *sub);
char *nameOfCrashedFile();
long sizeOfCrashedFile();
struct stat fileProperties(char *filename);
int is_file(char *path);
/* -------------------------- Server-side funtions -------------------------- */

void s_cmd(int connfd);
void s_get(char *msg);
void s_resume();
void s_ls();
void s_pwd();
void s_cd(char *path);

/* ----------------------- Client-side funtions ----------------------- */
void c_get(char *msg);
void c_resume();
void c_ls();
void c_pwd();
void c_cd();


