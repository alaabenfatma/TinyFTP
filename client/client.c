/*
 * echoclient.c - An echo client
 */
#include "../libs/csapp.h"
#include "../libs/param.h"

char folder[] = "downloads";
int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;
    FILE *f;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <host> \n", argv[0]);
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
    printf("client connected to server OS\n");

    Rio_readinitb(&rio, clientfd);
    while (1)
    {
        printf("ftp>");

        if (Fgets(buf, MAXLINE, stdin) == NULL)
        {
            break;
        }
        puts(buf);
        f = fopen("downloads/transfered", "w");

        Rio_writen(clientfd, buf, strlen(buf));
        ssize_t seen;
        while ((seen = Rio_readnb(&rio, buf, buffSize)) > 0)
        {
            if (buf[0] == EOF)
            {
                break;
            }
            Fputs(buf, f);
            fflush(f);
        }
    }
    Close(clientfd);
    exit(0);
}
