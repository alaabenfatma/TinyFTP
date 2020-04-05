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
#include "../libs/csapp.h"
#include "../libs/utils.h"
/* -------------------------------------------------------------------------- */
/*            Get the name of the file that we failed to download.            */
/* -------------------------------------------------------------------------- */
char *nameOfCrashedFile()
{
    FILE *f;
    f = fopen("crash.log", "r");
    char *fname = malloc(messageSize);
    fscanf(f, "%s", fname);
    fname = strtok(fname, ",");
    fclose(f);
    return fname;
}

/* -------------------------------------------------------------------------- */
/*      Get the size (post-crash) of the file that we failed to download.     */
/* -------------------------------------------------------------------------- */
long sizeOfCrashedFile()
{
    FILE *f;
    f = fopen("crash.log", "r");
    char *fname = malloc(messageSize);
    fscanf(f, "%s", fname);
    fname = strtok(fname, ",");
    fname = strtok(NULL, ",");
    fclose(f);
    char **err = malloc(sizeof(10));
    return (long)strtol(fname, err, 10);
}

/* -------------------------------------------------------------------------- */
/*    A cool progress bar to help us keep track of the transfer operations.   */
/* -------------------------------------------------------------------------- */

void printProgress(char *msg, ssize_t downloaded, ssize_t size)
{
    int _percentage = (int)percentage((double)size, (double)downloaded);
    printf("%s : %ld/%ld (%d%%)", msg, downloaded, size, _percentage);
    printf("\r");
    printf(RESET);
    if (_percentage == 100)
    {
        //remove console line after download is finished.
        clearLine();
    }
    fflush(stdout);
}
double percentage(double size, double downloaded)
{
    return (double)downloaded / size * 100.0;
}

/* -------------------------------------------------------------------------- */
/*                Return true if string a starts with string b                */
/* -------------------------------------------------------------------------- */
bool StartsWith(const char *a, const char *b)
{
    if (strncmp(a, b, strlen(b)) == 0)
        return true;
    return false;
}

/* -------------------------------------------------------------------------- */
/*          Return the first arg from a cmd .i.e "rm file" => "file"          */
/* -------------------------------------------------------------------------- */
char *getFirstArgument(char cmd[])
{
    char *argument = malloc(strlen(cmd) + 1);
    int i, j = 0;
    if (StartsWith(cmd, "rm -r"))
    {
        for (i = 6; i < strlen(cmd); i++)
        {
            argument[j] = cmd[i];
            argument[j + 1] = '\0';
            j++;
        }
        return argument;
    }

    for (i = 0; i < strlen(cmd); i++)
    {
        if (cmd[i] == ' ')
        {
            break;
        }
    }
    for (i = i + 1; i < strlen(cmd); i++)
    {
        argument[j] = cmd[i];
        argument[j + 1] = '\0';
        j++;
    }

    return argument;
}

/* -------------------------------------------------------------------------- */
/*                      Remove a substring from a string                      */
/* -------------------------------------------------------------------------- */
char *strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

/* -------------------------------------------------------------------------- */
/*                                 Parse query                                */
/* -------------------------------------------------------------------------- */
char *parse_fgets(char cmd[])
{
    size_t len = strlen(cmd);

    if (len > 0 && cmd[len - 1] == '\n')
    {
        cmd[len - 1] = '\0';
    }
    return cmd;
}

/* ------------------------ return false if directory ----------------------- */
int is_file(char *path)
{
    return S_ISREG(fileProperties(path).st_mode);
}

/* -------------------------------------------------------------------------- */
/*        We can use this hack to clear the screen and return ftp> only       */
/* -------------------------------------------------------------------------- */
void clear()
{
    /* --- Reference : https://www.geeksforgeeks.org/clear-console-c-language/ -- */
    const char *CLEAR_SCREEN_ANSI = "\n\e[1;1H\e[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}
void clearLine()
{
    printf("\r%c[2K", 27);
}

/* -------------------------------------------------------------------------- */
/*    Returns true if directory gets completely deleted. false, otherwose.    */
/* ------------------------ **fname** = folder name. ------------------------ */
/* -------------------------------------------------------------------------- */
bool s_removeDirectory(char *fname)
{
    printf("root : %s\n", fname);

    DIR *dir;
    struct dirent *current;
    char file_to_delete[1024];
    if (!(dir = opendir(fname)))
        return false;

    while ((current = readdir(dir)) != NULL)
    {
        if (!is_file(current->d_name))
        {
            if (strcmp(current->d_name, ".") == 0 || strcmp(current->d_name, "..") == 0)
                continue;

            snprintf(file_to_delete, sizeof(file_to_delete), "%s/%s", fname, current->d_name);
            s_removeDirectory(file_to_delete);
            remove(file_to_delete);
        }
        else
        {
            remove(current->d_name);
        }
    }
    /* ------------------------- We delete the root dir ------------------------- */
    if (remove(fname) != 0)
        return false;
    closedir(dir);
    return true;
}

/* -------------------------------------------------------------------------- */
/*                           Get filename from path                           */
/* -------------------------------------------------------------------------- */
char *fileBaseName(char *path)
{
    return basename(path);
}

/* -------------------------------------------------------------------------- */
/*                    Get the properties of a certain file                    */
/* -------------------------------------------------------------------------- */
struct stat fileProperties(char *filename)
{
    struct stat st;
    stat(filename, &st);
    return st;
}
bool directoryExists(char *path)
{
    DIR *dir = opendir(path);
    return (dir != NULL);
}
char *homedir()
{
    char *dir = malloc(messageSize);
    if (!(dir = getenv("HOME")))
    {
        dir = getpwuid(getuid())->pw_dir;
    }
    strcat(dir, "/");
    return dir;
}
void initDB()
{

    char path[messageSize];
    char temp[messageSize];
    strcpy(path, homedir());

    strcat(path, ftpDB);
    if (access(path, F_OK) != -1)
    {
        strcpy(ftpAccountsPath, path);
        printf("Accouts are contained in : %s\n", ftpAccountsPath);
        FILE *db = fopen(path, "r+");
        if (fgets(temp, messageSize, db) == 0)
        {
            fprintf(db, "%s", "username|pwd|date\n");
        }
        fclose(db);

        return;
    }
    strcpy(ftpAccountsPath, path);
    printf("Accouts are contained in : %s\n", ftpAccountsPath);

    FILE *db = fopen(path, "w+");
    fprintf(db, "%s", "username|pwd|date\n");
    fclose(db);
    fflush(stdout);
    return;
}
/* -------------------------------------------------------------------------- */
/*                           Securely read password                           */
/* -------------------------------------------------------------------------- */
char *readpassword()
{
    return crypt(getpass("Password : "), "ftp");
    ;
}

/* -------------------------------------------------------------------------- */
/*                               Welcome screen                               */
/* -------------------------------------------------------------------------- */
void welcome()
{
    printf("The server is " BOLD "read-only" RESET ". Please login if you want to make any modifications.\nType "BOLD"help"RESET" for details.\n");
}

/* -------------------------------------------------------------------------- */
/*                         Gather login/register data                         */
/* -------------------------------------------------------------------------- */
account getAccountInfo()
{
    account acc;
    printf("username : ");
    Fgets(acc.username, messageSize, stdin);
    strcpy(acc.username, parse_fgets(acc.username));
    strcpy(acc.password, parse_fgets(readpassword()));
    return acc;
}

/* -------------------------------------------------------------------------- */
/*                              Create a CSV file                             */
/* -------------------------------------------------------------------------- */
FILE *initfield()
{
    FILE *stream = fopen("busy.log", "w+");
    int i;
    for (i = 0; i < NPROC - 1; i++)
    {
        fputc('0', stream);
        fputc(';', stream);
    }
    fputc('0', stream);
    fflush(stream);
    return stream;
}

/* -------------------------------------------------------------------------- */
/*                       Modify one line of the CSV file                      */
/* -------------------------------------------------------------------------- */
//modifie la colone el par num dans p
void setfield(int el, char num, FILE *p)
{
    rewind(p);
    if (el == 0)
    {
        fputc(num, p);
    }
    else
    {
        char ch;
        int nbv = 0;
        while (((ch = fgetc(p)) != EOF) && (nbv != el - 1))
        {
            if (ch == ';')
            {
                nbv++;
            }
        }
        if (ch != EOF)
        {
            ch = fgetc(p);
            if (ch != EOF)
            {
                fputc(num, p);
            }
            else
            {
                printf("%i is too far\n", el);
            }
        }
        else
        {
            printf("%i is too far\n", el);
        }
    }
    fflush(p);
}

/* -------------------------------------------------------------------------- */
/*                           Get one of the CSV file                          */
/* -------------------------------------------------------------------------- */
//rend la colone el du fichier p
int getfield(int el, FILE *p)
{
    rewind(p);
    char ch = '\0';
    if (el == 0)
    {
        ch = fgetc(p);
        return (ch - '0');
    }
    else
    {
        int nbv = 0;
        while ((ch != EOF) && (nbv != el))
        {
            ch = fgetc(p);
            if (ch == ';')
            {
                nbv++;
            }
        }
        if (ch != EOF)
        {
            ch = fgetc(p);
            return (ch - '0');
        }
        else
        {
            printf("%i is too far\n", el);
            return -1;
        }
    }
}

int establishConnection(char *ip, int port, int timeout)
{
    bool connected = false;
    int clientfd = -1;
    printf("Establishing connection... ");
    int seconds = 0;
    rio_t rio;
    while (connected == false && seconds <= timeout)
    {
        clientfd = open_clientfd(ip, port);

        Rio_readinitb(&rio, clientfd);
        int elu;
        Rio_readnb(&rio, &elu, sizeof(elu));

        if (elu == -1)
        {
            //Aucun serveur n'est libre...
            printf(RED "The server is busy. Try again later.\n" RESET);
            exit(0);
        }
        //On se connecte
        int slave_port = elu + 2122;
        usleep(10000);
        clientfd = open_clientfd(ip, slave_port);
        if (clientfd > 0)
        {
            connected = true;
        }
        else
        {
            continue;
        }
        seconds++;
    }
    printf(GREEN "OK\n" RESET);

    return clientfd;
}

int runTimeCheck(int fd, char *arg)
{
    if (strstr(arg, "server") != NULL)
    {
        return fd;
    }
    /* -------------------- Check if "downloads" folder exists -------------------- */
    DIR *dir = opendir("downloads");
    if (!dir)
    {
        /* -------------- Directory does not exist. We will create it. -------------- */
        mkdir("downloads", 0700);
    }
    return fd;
}

void help()
{
    printf("These are all the possible commands\n");
    printf(BOLD "(*) All the commands that are in blue require you to log in.\n\n" RESET);
    printf(BOLD "login " RESET ": login is used to log into an account\n");
    printf(BOLD "register " RESET ": register command is used to create an account\n");
    printf(BOLD "disconnect " RESET ": disconnect command is used to logout.\n");
    printf(BOLD "get " RESET ": get command is used to download a file from the FTP server\n");
    printf(BOLD "resume " RESET ": resume command is used to resume a failed download\n");
    printf(BOLD "ls " RESET ": ls command is used to list contents of a directory\n");
    printf(BOLD "pwd " RESET ": pwd command displays the name of current working directory\n");
    printf(BOLD BLUE "mkdir" RESET " : mkdir command is used to create a single directory\n");
    printf(BOLD BLUE "rm" RESET " : rm is used to remove files or empty directories\n");
    printf(BOLD BLUE "rm -r" RESET " : rm -r command is used to remove directories\n");
    printf(BOLD BLUE "put" RESET " : put command is used to upload a file to the FTP server\n");
}

/* ---------------------- Get current time as a string ---------------------- */
char *currentTime()
{
    time_t rawtime;
    time(&rawtime);
    return asctime(localtime(&rawtime));
}

/* -------------------------- Check if fd is valid -------------------------- */
bool isValidFD(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

/* -------------- Count the number of free slots in the server. ------------- */
int busyChildren()
{
    int i, c = 0;
    for (i = 0; i < NPROC; i++)
    {
        bool occupied = getfield(i, busy);
        if (occupied != false)
        {
            c++;
        }
    }
    return c;
}
void connectedClients()
{
    printf(GREEN INVERTED "Connected clients : %d/%d\n" RESET, busyChildren(), NPROC);
    fflush(stdout);
}

/* -------------------------------------------------------------------------- */
/*         Returns - if the client is trying to access sensitive data.        */
/* -------------------------------------------------------------------------- */
char securityBreachAttempt(char *cmd)
{
    if (strstr(cmd, ".ftpAccounts.db") != NULL || strstr(cmd, "busy.log") != NULL)
    {
        printf(BOLD RED "WARNING :" RESET " Client has tried to access sensitive data!\n");
        fflush(stdout);
        return '-';
    }
    return '+';
}
