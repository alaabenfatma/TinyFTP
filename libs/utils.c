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
    char *argument = malloc(strlen(cmd));
    int i, j = 0;
    if (StartsWith(cmd, "rm -r"))
    {
        for (i = 6; i < strlen(cmd) - 1; i++)
        {
            argument[j] = cmd[i];
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
    for (i = i + 1; i < strlen(cmd) - 1; i++)
    {
        argument[j] = cmd[i];
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

/* ------------------------ return false if directory ----------------------- */
int is_file(char *path)
{
    return S_ISREG(fileProperties(path).st_mode);
}

/* -------------------------------------------------------------------------- */
/*        We can use this hack to clear the screen and return ftp> only       */
/* -------------------------------------------------------------------------- */
void clearClientScreen()
{
    const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}

/* -------------------------------------------------------------------------- */
/*    Returns true if directory gets completely deleted. false, otherwose.    */
/* ------------------------ **fname** = folder name. ------------------------ */
/* -------------------------------------------------------------------------- */
bool s_removeDirectory(char *fname)
{
    if (fname[strlen(fname) - 2] != '/')
    {
        strcat(fname, "/");
    }
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
char *fileBaseName(char const *path)
{
    char *s = strrchr(path, '/');
    if (!s)
        return strdup(path);
    else
        return strdup(s + 1);
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
    printf("Please connect (" CYAN "press C" RESET ") or register (" CYAN "press R" RESET ")\n");
    char input = 'X';
    while (input != 'C' && input != 'R')
    {
        input = getchar();
    }
    char **data = getAccountInfo();
    if (input == 'C')
    {
        loginAccount(data[0], data[1]);
    }
    else if (input == 'R')
    {
        createAccount(data[0], data[1]);
    }
}

/* -------------------------------------------------------------------------- */
/*                         Gather login/register data                         */
/* -------------------------------------------------------------------------- */
char **getAccountInfo()
{
    int i = 0;
    char **data = malloc(sizeof(char *) * 2);
    if (!data)
        return NULL;
    for (i = 0; i < 2; i++)
    {
        data[i] = malloc(messageSize + 1);
        if (!data[i])
        {
            free(data);
            return NULL;
        }
    }
    printf("username : ");
    scanf("%s", data[0]);
    strcpy(data[1], readpassword());
    return data;
}

/* -------------------------------------------------------------------------- */
/*   Create an account on the server-side using the data sent by the client   */
/* -------------------------------------------------------------------------- */
bool createAccount(char *usr, char *pwd)
{
    return true;
}

/* -------------------------------------------------------------------------- */
/*                Connect to an existing account on the Server                */
/* -------------------------------------------------------------------------- */
bool loginAccount(char *usr, char *pwd)
{
    return true;
}

/* -------------------------------------------------------------------------- */
/*                              Create a CSV file                             */
/* -------------------------------------------------------------------------- */
FILE *initfield()
{
    FILE *stream = fopen("busy.log", "w+");
    for (int i = 0; i < NPROC - 1; i++)
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
    printf("Establishing connection...\n");
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
            printf("Aucun serveur n'est libre, il faut qu'un autre client ce deconnecte.\n");
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

int runTimeCheck(int fd)
{

    /* -------------------- Check if "downloads" folder exists -------------------- */
    DIR *dir = opendir("downloads");
    if (!dir)
    {
        /* -------------- Directory does not exist. We will create it. -------------- */
        mkdir("downloads",0700);
    }
    return fd;
}

void help(){
    printf("These are all the possible commands\n");
    printf("(*) All the commands that are in blue require you to log in\n");
    printf("login : login is used to login to an account");
    printf("register : register command is used to create an account\n");
    printf("get : get command is used to download a file from the FTP server\n");
    printf("resume : resume command is used to resume a failed download\n");
    printf("ls : ls command is used to list contents of a directory\n");
    printf("pwd : pwd command displays the name of current working directory\n");
    printf(BLUE"mkdir"RESET" : mkdir command is used to create a single directory\n");
    printf(BLUE"rm"RESET" : rm is used to remove files or empty directories\n");
    printf(BLUE"rm -r"RESET" : rm -r command is used to remove directories\n");
    printf(BLUE"put"RESET" : put command is used to upload a file to the FTP server\n");
    
}