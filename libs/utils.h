/*
BSD 3-Clause License

Copyright (c) 2020, Alaa Ben Fatma
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <ftw.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <crypt.h>
#include <libgen.h>
#include "csapp.h"
#include <pwd.h>

/* -------------------------------------------------------------------------- */
/*                         The super global directory                         */
/* -------------------------------------------------------------------------- */

char global_path[FILENAME_MAX];
DIR *current_directory;
FILE *busy;

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
#define ftpDB ".ftpAccounts.db"
char ftpAccountsPath[messageSize];

/* -------------------------------------------------------------------------- */
/*                               PROGRESS LABEL                               */
/* -------------------------------------------------------------------------- */

struct timeval stop, start;
bool downloading;
void printProgress(char *msg, ssize_t downloaded, ssize_t size);
double percentage(double size, double downloaded);

/* -------------------------------------------------------------------------- */
/*                                   COLORS                                   */
/* -------------------------------------------------------------------------- */

#define BOLD "\e[1m"
#define INVERTED "\e[7m"
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
char *fileBaseName(char  *path);
char *currentTime();
bool isValidFD(int fd);
void clear();
void clearLine();
 char *homedir();
 char * parse_fgets(char cmd[]);
/* -------------------------------------------------------------------------- */
/*                            Server-side funtions                            */
/* -------------------------------------------------------------------------- */

void s_cmd(int connfd,int child);
void s_createAccount();
void s_loginAccount();
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
void s_bye();

/* -------------------------------------------------------------------------- */
/*                             Server-side helpers                            */
/* -------------------------------------------------------------------------- */
int busyChildren();
void connectedClients();
/* -------------------------------------------------------------------------- */
/*                            Client-side funtions                            */
/* -------------------------------------------------------------------------- */
void welcome();
void help();
void c_createAccount();
void c_loginAccount();
void c_get(char *msg);
void c_resume();
void c_ls();
void c_pwd();
void c_cd();
void c_mkdir();
void c_rm();
void c_rmdir();
void c_put(char *fname);
void c_bye(bool forced);
void disconnect();

/* -------------------------------------------------------------------------- */
/*                         Accounts managing functions                        */
/* -------------------------------------------------------------------------- */
void initDB();
char *readpassword();
typedef struct {
    char username[messageSize];
    char password[messageSize];
}account;
account getAccountInfo();
char securityBreachAttempt(char *cmd);
/* -------------------------------------------------------------------------- */
/*                                   CSV IO                                   */
/* -------------------------------------------------------------------------- */
FILE *initfield();
void setfield(int el, char num, FILE *p);
int getfield(int el, FILE *p);

/* -------------------------------------------------------------------------- */
/*                             Client-side helpers                            */
/* -------------------------------------------------------------------------- */
int establishConnection(char *ip, int port, int timeout);
int runTimeCheck(int fd,char *arg);

