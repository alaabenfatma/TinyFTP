#include <ftw.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <crypt.h>
#include "csapp.h"

/* -------------------------------------------------------------------------- */
/*                         The super global directory                         */
/* -------------------------------------------------------------------------- */

char global_path[FILENAME_MAX];
DIR *current_directory;

/* -------------------------------------------------------------------------- */
/*                             Global definitions                             */
/* -------------------------------------------------------------------------- */

#define buffSize 256    //must be >= 2
#define messageSize 512 //must be >= 2
#define NPROC 4         //must be >= 2
pid_t child_processes[NPROC];
#define FOLDER "downloads/"
//boolean type
typedef int bool;
#define true 1
#define false 0

/* -------------------------------------------------------------------------- */
/*                               PROGRESS LABEL                               */
/* -------------------------------------------------------------------------- */

struct timeval stop, start;
bool downloading;
void printProgress(char *msg,ssize_t downloaded, ssize_t size);
double percentage(double size, double downloaded);

/* -------------------------------------------------------------------------- */
/*                                   COLORS                                   */
/* -------------------------------------------------------------------------- */

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

/* -------------------------------------------------------------------------- */
/*                                    utils                                   */
/* -------------------------------------------------------------------------- */

char *getFirstArgument(char cmd[]);
bool StartsWith(const char *a, const char *b);
char *strremove(char *str, const char *sub);
char *nameOfCrashedFile();
long sizeOfCrashedFile();
struct stat fileProperties(char *filename);
int is_file(char *path);
char *fileBaseName(char const *path);

/* -------------------------------------------------------------------------- */
/*                            Server-side funtions                            */
/* -------------------------------------------------------------------------- */

void s_cmd(int connfd);
void s_get(char *msg);
void s_resume();
void s_ls();
void s_pwd();
void s_cd(char *path);
void s_mkdir(char *fname);
void s_rm(char *fname);
void s_rmdir(char *fname);
void s_put(char *fname);
bool s_removeDirectory(char *fname);

/* -------------------------------------------------------------------------- */
/*                            Client-side funtions                            */
/* -------------------------------------------------------------------------- */
void welcome();
void help();
void clearClientScreen();
void c_get(char *msg);
void c_resume();
void c_ls();
void c_pwd();
void c_cd();
void c_mkdir();
void c_rm();
void c_rmdir();
void c_put(char *fname);

/* -------------------------------------------------------------------------- */
/*                         Accounts managing functions                        */
/* -------------------------------------------------------------------------- */
char *readpassword();
bool createAccount(char *username,char *password);
bool loginAccount(char *username,char *password);
char **getAccountInfo();

/* -------------------------------------------------------------------------- */
/*                                   CSV IO                                   */
/* -------------------------------------------------------------------------- */
FILE *initfield();
void setfield(int el, char num, FILE *p);
int getfield(int el, FILE *p);




/*Please note that this function forces the client to try connecting multiple times!*/
int forceConnect(char *ip,int port,int timeout);