//Parametres et defines utiles..
#include "csapp.h"
#include <dirent.h> //Pour la manipulation des dossiers
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <crypt.h> //pour la partie 8

#define buffSize 256 //must be >= 2 
#define MAX_NAME_LEN 256
#define NPROC 3
#define MAXUSERS 50
#define CREDLENGTH 20

DIR *dir; //Gestion du dossier courant 

long cr; //position courante dans le transfert, utile pour la partie 3

int get(int connfd);
int get2(int connfd, long pos);
int ls(DIR *dir);
int remove_directory(const char *path);
void get_cred(int connfd, char** str);
int verify_login(char *username, char *password);

//utilisateurs
char* users[MAXUSERS][2];