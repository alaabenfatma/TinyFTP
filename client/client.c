/*
 * echoclient.c - An echo client
 */
#include "../libs/csapp.h"
#include "../libs/utils.h"
#include "../libs/cmds.h"

int main(int argc, char **argv)
{
    int clientfd, port;

    char *host;

    rio_t rio;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);

    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS \n");
    char *query = malloc(MAXLINE);
    while (1)
    {
        printf("ftp>");
        if (fgets(query, MAXLINE, stdin) == NULL)
        {
            break;
        }
        Rio_writen(clientfd, query, strlen(query));
        Rio_readinitb(&rio, clientfd);
        if (StartsWith(query, "echo"))
        {
            //ignore
        }
        else if (StartsWith(query, "get"))
        {
            ssize_t s;
            char contents[buffSize];
            do 
            {
                s = Rio_readlineb(&rio, contents, buffSize);
                if (contents[0] == '\0' ||contents[0] == EOF )
                {
                    break;
                }
                fputs(contents, stdout);
            }while(s>0);
        }
        fflush(stdout);
    }

    Close(clientfd);
    exit(0);
}
