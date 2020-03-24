/*
 * echo - read and echo text lines until client closes connection
 */
#include "../libs/utils.h"
int Connfd;
void cmd(int connfd)
{
    Connfd = connfd;
    size_t n;
    char query[messageSize];
    rio_t rio;
    Rio_readinitb(&rio, Connfd);
    while ((n = Rio_readnb(&rio, query, messageSize)) != 0)
    {
        printf("[CMD]"YELLOW" %s \n"RESET, query);

        if (StartsWith(query, "echo"))
        {
            echo(getFirstArgument(query));
        }
        else if (StartsWith(query, "get"))
        {
            get(getFirstArgument(query));
        }
        else if (StartsWith(query, "resume"))
        {
            resume();
        }
        else if (StartsWith(query, "ls"))
        {
            ls();
        }
    }
}
void echo(char *msg)
{
    printf("Client sent : %s\n", msg);
    strcpy(msg, "Server has received your message.");
    Rio_writen(Connfd, msg, messageSize);
}
void get(char *filename)
{
    printf("We will try to transfer the file %s to the client %d\n", filename, Connfd);
    fflush(stdout);
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
    }
    buffer[0] = EOF;
    Rio_writen(Connfd, buffer, buffSize);
    fclose(f);
}
void resume()
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
void ls()
{

    /* -- can be either f or d -- */
    char type = 'f';
    struct dirent *dir;
    if (current_directory)
    {
        while ((dir = readdir(current_directory)) != NULL)
        {
            Rio_writen(Connfd, dir->d_name, messageSize);

            /* ----- Finding the type of the file helps us with printing the colors ----- */
            if (is_file(dir->d_name))
            {
                type = 'f';
                Rio_writen(Connfd, &type, sizeof(char));
            }
            else
            {
                type = 'd';
                Rio_writen(Connfd, &type, sizeof(char));
            }
        }
        Rio_writen(Connfd,&EOF_BUFF, messageSize);
        closedir(current_directory);
    }
}