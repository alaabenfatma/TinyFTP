/*
 * echo - read and echo text lines until client closes connection
 */
#include "../libs/csapp.h"
#include "../libs/utils.h"
#include "../libs/cmds.h"
void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %u bytes\n", (unsigned int)n);
        printf("Le client a écrit \n: %s",buf);
        Rio_writen(connfd, buf, n);
    }
}
void get(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    char *query = "";
    char *filename = "";
    char fileBuffer[buffSize];
    char *response = malloc(MAXLINE);
    int fd_file;
    size_t size;
    int clientIsPresent;
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        //On enleve le retour chariot
        query = strdup(buf);

        for (int i = 0; query[i] != '\0'; i++)
        {

            if (query[i] == '\n')
            {
                query[i] = '\0';
            }
        }
        //get the filename
        if (StartsWith(query, "get"))
        {
            filename = malloc(8 * sizeof(char));
            filename = getFirstArgument(query);

            //start transfert
            if ((fd_file = open(filename, O_RDONLY, NULL)) > 0)
            {
                size = fileProperties(filename).st_size;
                printf("The file is of size : %lu\n", size);
                Rio_writen(connfd, &size, sizeof(size));

                Rio_readinitb(&rio, fd_file);

                clientIsPresent = 1;

                while ((n = Rio_readnb(&rio, fileBuffer, buffSize)) != 0 && clientIsPresent != 0)
                {
                    if (rio_writen(connfd, fileBuffer, n) == -1)
                    {
                        printf("La connexion a un client a été perdu");
                        clientIsPresent = 0;
                    }
                    usleep(1000);
                }

                close(fd_file);
            }
            else
            {
                response = "File does not exist.\nOperation cancelled.";
            }
            Rio_writen(connfd, response, MAXLINE);
        }
        else
        {
            return;
        }
    }
    Close(connfd);
}
