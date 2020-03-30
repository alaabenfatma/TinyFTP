/*
 * echo - read and echo text lines until client closes connection
 */
#include "../libs/utils.h"
int Connfd;
void s_cmd(int connfd)
{
    Connfd = connfd;
    size_t n;
    char query[messageSize];
    rio_t rio;
    Rio_readinitb(&rio, Connfd);
    while ((n = Rio_readnb(&rio, query, messageSize)) != 0)
    {
        printf("[CMD]" YELLOW " %s" RESET, query);

        if (StartsWith(query, "get"))
        {
            s_get(getFirstArgument(query));
        }
        else if (StartsWith(query, "resume"))
        {
            s_resume();
        }
        else if (StartsWith(query, "ls"))
        {
            s_ls();
        }
        else if (StartsWith(query, "pwd"))
        {
            s_pwd();
        }
        else if (StartsWith(query, "cd"))
        {
            s_cd(getFirstArgument(query));
        }
        else if (StartsWith(query, "mkdir"))
        {
            s_mkdir(getFirstArgument(query));
        }
        else if (StartsWith(query, "rm"))
        {
            s_rm(getFirstArgument(query));
        }
        else if (StartsWith(query, "rm -r"))
        {
            s_rmdir(getFirstArgument(query));
        }
        else if (StartsWith(query, "put"))
        {
            s_put(getFirstArgument(query));
        }
    }
}

void s_get(char *filename)
{

    FILE *f;
    char buffer[buffSize];
    char *msg = malloc(sizeof(char));
    strcpy(msg, "+");
    f = fopen(filename, "rb");
    if (f == NULL)
    {
        strcpy(msg, "-");
    }
    Rio_writen(Connfd, msg, 1);
    if (StartsWith(msg, "-"))
    {
        return;
    }

    /* ------------------------ Send file size to client ------------------------ */
    ssize_t size = fileProperties(filename).st_size;
    Rio_writen(Connfd, &size, sizeof(size));
    long position;

    while (Fgets(buffer, buffSize, f) > 0)
    {
        position = ftell(f);
        if (rio_writen(Connfd, buffer, buffSize) != buffSize)
        {
            printf(RED "An error has occured during the transfer.\n" RESET);
            break;
        };
        rio_writen(Connfd, &position, __SIZEOF_LONG__);
        printProgress("Uploading : ", position, size);
    }
    buffer[0] = EOF;
    Rio_writen(Connfd, buffer, buffSize);
    fclose(f);
}
void s_resume()
{

    /* ---------------------------- Init resume data ---------------------------- */

    printf("Resuming transfer.\n");
    rio_t rio;
    Rio_readinitb(&rio, Connfd);
    char str[messageSize];
    Rio_readnb(&rio, str, messageSize);

    char *filename = strtok(str, ",");
    char *p = strtok(NULL, ",");
    char **eptr = malloc(__SIZEOF_LONG__);
    long position = strtol(p, eptr, 10);
    printf("%s %ld", filename, position);
    fflush(stdout);
    Rio_readinitb(&rio, Connfd);
    /* --------------------------- Resuming the upload -------------------------- */
    FILE *f;
    char buffer[buffSize];
    char *msg = malloc(sizeof(char));
    strcpy(msg, "+");
    f = fopen(filename, "rb");
    if (f == NULL)
    {
        strcpy(msg, "-");
    }
    Rio_writen(Connfd, msg, 1);
    if (StartsWith(msg, "-"))
    {
        return;
    }
    fseek(f, position, SEEK_SET);
    while (Fgets(buffer, buffSize, f) > 0)
    {
        if (rio_writen(Connfd, buffer, buffSize) != buffSize)
        {
            printf(RED "An error has occured during the transfer.\n" RESET);
            break;
        };
    }
    buffer[0] = EOF;
    Rio_writen(Connfd, buffer, buffSize);
    fclose(f);
    printf("File has been uploaded.");
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

        closedir(current_directory);
    }
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
    bool error = !(s_removeDirectory(fname));
    Rio_writen(Connfd, &error, sizeof(bool));
}
void s_rm(char *fname)
{
    bool error = (remove(fname) != 0);
    Rio_writen(Connfd, &error, sizeof(bool));
}
void s_put(char *filename)
{
    rio_t rio;
    Rio_readinitb(&rio, Connfd);
    char contents[buffSize];
    if ((Rio_readnb(&rio, contents, 1)) > 0)
    {
        if (StartsWith(contents, "-"))
        {
            printf(RED "An error has occured on the server side. Please check your command.\n" RESET);
            fflush(stdout);
            return;
        }
        else
        {
        }
    }
    ssize_t original_size, s;
    strcpy(filename, fileBaseName(filename));
    Rio_readnb(&rio, &original_size, sizeof(original_size));
    FILE *f;

    f = fopen(filename, "w");
    gettimeofday(&start, NULL);
    Rio_readinitb(&rio, Connfd);
    while ((s = Rio_readnb(&rio, contents, buffSize)) > 0)
    {
        if (contents[0] == EOF || sizeof contents == 0)
        {
            break;
        }
        Rio_readnb(&rio, &downloading, __SIZEOF_LONG__);
        Fputs(contents, f);
        printProgress("Downloading : ", downloading, original_size);
    }
    fflush(f);
    fclose(f);
    gettimeofday(&stop, NULL);
    downloading = 0;
    double secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    off_t file_size = fileProperties(filename).st_size;
    printf(GREEN "File has been downloaded successfully.\n" RESET);
    printf("%ld bytes received in %f seconds (%f Kbytes/s)\n", file_size, secs, (file_size / 1024 / secs));
}