/*
 * echo - read and echo text lines until client closes connection
 */
#include "../libs/utils.h"
int Connfd;
int current_child = 0;
rio_t rio;
bool clientCrashing = false;
bool clientLoggedIn = false;
char username[messageSize];
void s_cmd(int connfd, int child)
{
    Connfd = connfd;
    current_child = child;
    size_t n;
    char query[messageSize];
    clientCrashing = false;
    Rio_readinitb(&rio, Connfd);
    Rio_readnb(&rio, &username, messageSize);
    while ((n = Rio_readnb(&rio, query, messageSize)) != 0)
    {
        //Remove the line that indicates the number of connected clients.

        Rio_readnb(&rio, &username, messageSize);
        Connfd = runTimeCheck(connfd, "server");
        printf("[" CYAN "%s" RESET "]" YELLOW " %s" RESET " at %s", username, query, currentTime());

        if (strcmp(username, "Anonymous") != 0)
        {
            clientLoggedIn = true;
        }
        else
        {
            clientLoggedIn = false;
        }

        if (StartsWith(query, "get "))
        {
            s_get(getFirstArgument(query));
        }
        else if (!strcmp(query, "resume"))
        {
            s_resume();
        }
        else if (!strcmp(query, "ls"))
        {
            s_ls();
        }
        else if (!strcmp(query, "pwd"))
        {
            s_pwd();
        }
        else if (StartsWith(query, "cd "))
        {
            s_cd(getFirstArgument(query));
        }
        else if (StartsWith(query, "mkdir "))
        {
            if (clientLoggedIn)
                s_mkdir(getFirstArgument(query));
        }
        else if (StartsWith(query, "rm -r "))
        {
            if (clientLoggedIn)
                s_rmdir(getFirstArgument(query));
        }
        else if (StartsWith(query, "rm "))
        {
            if (clientLoggedIn)
                s_rm(getFirstArgument(query));
        }
        else if (StartsWith(query, "put "))
        {
            if (clientLoggedIn)
                s_put(getFirstArgument(query));
        }
        else if (!strcmp(query, "bye"))
        {
            return;
        }
        else if (!strcmp(query, "register"))
        {
            s_createAccount();
        }
        else if (!strcmp(query, "login"))
        {
            if (!clientLoggedIn)
                s_loginAccount();
        }
    }
    printf("[" CYAN "%s" RESET "] has disconnected.\n", username);
}

void s_get(char *filename)
{
    FILE *f;
    char buffer[buffSize];
    char *msg = malloc(sizeof(char));
    msg[0] = securityBreachAttempt(filename);
    f = fopen(filename, "rb");
    if (f == NULL)
    {
        strcpy(msg, "-");
        printf("Could not open file. Operation will be ignored.\n");
    }
    Rio_writen(Connfd, msg, 1);
    if (StartsWith(msg, "-"))
    {
        return;
    }
    Rio_readinitb(&rio, Connfd);
    /* ------------------------ Send file size to client ------------------------ */
    ssize_t s, size = fileProperties(filename).st_size;
    Rio_writen(Connfd, &size, sizeof(size));
    long position;

    while ((s = fread(buffer, 1, buffSize, f)) != 0)
    {
        position = ftell(f);
        if (rio_writen(Connfd, buffer, buffSize) != s && position != size)
        {
            printf(RED BOLD "An error has occured during the transfer.\n" RESET);
            fflush(stdout);
            clientCrashing = true;
            return;
        };
        rio_writen(Connfd, &s, sizeof(long));
        rio_writen(Connfd, &position, sizeof(long));
        printProgress("Uploading : ", position, size);
    }
    fclose(f);
}
void s_resume()
{

    /* ---------------------------- Init resume data ---------------------------- */

    rio_t rio;
    Rio_readinitb(&rio, Connfd);

    char *str = malloc(messageSize);
    Rio_readnb(&rio, str, messageSize);
    printf("Data : %s\n", str);

    if (StartsWith(str, "X"))
    {
        printf(RED "Client trying to resume a non-found download operation.\n" RESET);
        fflush(stdout);
        return;
    }

    char *filename = strtok(str, ",");
    char *p = strtok(NULL, ",");
    char **eptr = malloc(__SIZEOF_LONG__);
    long position = strtol(p, eptr, 10);
    fflush(stdout);
    /* --------------------------- Resuming the upload -------------------------- */
    printf("Resuming transfer...\n");
    FILE *f;
    char buffer[buffSize];
    char *msg = malloc(sizeof(char));
    msg[0] = securityBreachAttempt(filename);

    f = fopen(filename, "r");
    if (f == NULL)
    {
        strcpy(msg, "-");
        printf("Could not open file. Operation will be ignored.\n");
    }
    Rio_writen(Connfd, msg, sizeof(char));
    if (StartsWith(msg, "-"))
    {
        return;
    }

    ssize_t s, size = fileProperties(filename).st_size;
    Rio_writen(Connfd, &size, sizeof(size));
    fseek(f, position, SEEK_SET);

    while ((s = fread(buffer, 1, buffSize, f)) != 0)
    {
        if (clientCrashing == true)
        {
            return;
        }
        position = ftell(f);
        if (rio_writen(Connfd, buffer, buffSize) != s && position != size)
        {
            printf(RED BOLD "An error has occured during the transfer.\n" RESET);
            fflush(stdout);
            clientCrashing = true;
            return;
        };
        rio_writen(Connfd, &s, sizeof(long));
        rio_writen(Connfd, &position, sizeof(long));
        printProgress("Uploading : ", position, size);
    }
    fclose(f);
    printf("File has been uploaded.\n");
    fflush(stdout);
}
void s_ls()
{
    /* -- can be either f or d -- */
    char type = 'f';
    struct dirent *dir;
    current_directory = opendir(global_path);
    if (current_directory)
    {
        while ((dir = readdir(current_directory)) != NULL)
        {
            fflush(stdout);
            Rio_writen(Connfd, dir->d_name, messageSize);

            /* ----- Finding the type of the file helps us with printing the colors ----- */
            if (is_file(dir->d_name))
                type = 'f';
            else
                type = 'd';
            Rio_writen(Connfd, &type, sizeof(char));
        }

        type = EOF;
        Rio_writen(Connfd, &type, messageSize);
        Rio_writen(Connfd, &type, messageSize);
    }
    //closedir(current_directory);
}
void s_pwd()
{
    char *path = malloc(messageSize);
    getcwd(path, messageSize);
    Rio_writen(Connfd, path, messageSize);
}
void s_cd(char *path)
{
    bool error = false;
    if (chdir(path) == -1)
    {
        printf(RED "Failed to change directory: %s\n" RESET, strerror(errno));
        error = true;
    }
    Rio_writen(Connfd, &error, sizeof error);
    fflush(stdout);
}
void s_mkdir(char *fname)
{
    bool error = false;
    if (mkdir(fname, 0700))
    {
        printf(RED "Failed to create directory: %s\n" RESET, strerror(errno));
        error = true;
    }
    else
    {
        printf("Folder %s has been created\n", fname);
    }
    Rio_writen(Connfd, &error, sizeof(bool));
    fflush(stdout);
}
void s_rmdir(char *fname)
{
    /* ------------------ error = true; if something goes wrong. ----------------- */
    printf("dir : %s\n", fname);

    bool error = !(s_removeDirectory(fname));
    Rio_writen(Connfd, &error, sizeof(bool));
}
void s_rm(char *fname)
{
    printf("rm : %s\n", fname);
    bool error = (remove(fname) != 0);
    Rio_writen(Connfd, &error, sizeof(bool));
}
void s_put(char *filename)
{

    downloading = false;
    char *msg = malloc(messageSize);
    char *contents = malloc(buffSize);
    Rio_readinitb(&rio, Connfd);
    Rio_readnb(&rio, msg, messageSize);
    if (strstr(msg, "-") != NULL)
    {
        printf(RED "An error has occured on the client side. Please check your command.\n" RESET);
        fflush(stdout);
        return;
    }

    ssize_t original_size;
    rio_readnb(&rio, &original_size, sizeof(ssize_t));
    printf("SIZE %lu\n", original_size);
    printf("fname : %s\n", filename);
    sleep(1);
    gettimeofday(&start, NULL);
    FILE *f;
    f = fopen(filename, "wb");
    size_t chunk = 0;
    while (original_size != downloading)
    {
        rio_readnb(&rio, contents, buffSize);
        rio_readnb(&rio, &chunk, sizeof(long));
        fwrite(contents, 1, chunk, f);
        rio_readnb(&rio, &downloading, sizeof(long));
        printProgress("Downloading", downloading, original_size);
    }
    fflush(f);
    fclose(f);
    gettimeofday(&stop, NULL);
    downloading = false;

    /* ----------------------------- Download result ---------------------------- */
    double secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    off_t file_size = fileProperties(filename).st_size;
    printf(GREEN "File has been downloaded successfully.\n" RESET);
    printf("%ld bytes received in %f seconds (%f Kbytes/s)\n", file_size, secs, (file_size / 1024 / secs));
}

void s_bye()
{
    //IGNORE.
}

void s_createAccount()
{

    account acc;
    rio_readinitb(&rio, Connfd);
    rio_readnb(&rio, &acc, sizeof(acc));
    char response[messageSize];

    FILE *db = fopen(ftpAccountsPath, "a+");
    if (db == NULL)
    {
        strcpy(response, "Account could not be created. Server-side error.");
        printf("%s\n", response);
        fclose(db);
        rio_writen(Connfd, &response, messageSize);
        return;
    }
    //Check if name already exists.
    char name[256];
    while (!feof(db))
    {
        fscanf(db, "%s", name);
        strcpy(name, strtok(name, "|"));
        fflush(stdout);
        if (!strcmp(name, acc.username))
        {
            strcpy(response, RED "username already exists. Try again." RESET);
            fclose(db);
            rio_writen(Connfd, &response, messageSize);
            return;
        }
    }
    fprintf(db, "%s|%s|%s", acc.username, acc.password, currentTime());
    strcpy(response, GREEN "Your account has been created.");
    strcat(response, RESET "\nNow you can log into your account.");
    printf("The account has been created.\n");
    rio_writen(Connfd, &response, messageSize);
    fclose(db);
    return;
}
void s_loginAccount()
{
    printf("The client is inserting his data...\n");
    account acc;
    rio_readinitb(&rio, Connfd);
    rio_readnb(&rio, &acc, sizeof(acc));
    char response[messageSize];
    strcpy(response, "-");
    FILE *db = fopen(ftpAccountsPath, "r+");
    if (db == NULL)
    {
        printf(RED "%s\n" RESET, response);
        fclose(db);
        rio_writen(Connfd, &response, messageSize);
        return;
    }

    //check matching account data.
    char name[256], pwd[256];
    while (fgets(name, 256, db) != false)
    {
        
        strcpy(name, strtok(name, "|"));
        strcpy(pwd, strtok(NULL, "|"));
        fflush(stdout);
        if (!strcmp(name, acc.username))
        {
            if (!strcmp(pwd, acc.password))
            {
                strcpy(response, "+");
                fclose(db);
                rio_writen(Connfd, &response, messageSize);
                printf("[SERVER] User, %s, has logged in.\n", name);
                return;
            }
        }
    }

    printf("Someone has tried to log in!\n");
    rio_writen(Connfd, &response, messageSize);
    fclose(db);
}
