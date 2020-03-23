/*
 * echo - read and echo text lines until client closes connection
 */
#include "../libs/csapp.h"
#include "../libs/utils.h"
#include "../libs/cmds.h"
int Connfd;
void cmd(int connfd)
{
    Connfd = connfd;
    size_t n;
    char query[messageSize];
    rio_t rio;
    Rio_readinitb(&rio, Connfd);
    while ((n = Rio_readlineb(&rio, query, MAXLINE)) != 0)
    {
        printf("server received %u bytes\n", (unsigned int)n);

        if (StartsWith(query, "echo"))
        {

            echo(getFirstArgument(query));
        }
        else if (StartsWith(query, "get"))
        {
            get(getFirstArgument(query));
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
    printf("We will try to transfer the file %s to the client %d", filename, Connfd);
    int error = 0;
    FILE *f;
    char buffer[buffSize];
    ;
    f = fopen(filename, "rb");
    if (f == NULL)
    {
        error = 1;
        printf("an error has occured %d ", error);
    }
    else
    {

        /*int position = 0;
        fseek(f,position,SEEK_CUR);*/
        while(Fgets(buffer,buffSize,f)>0){
            Rio_writen(Connfd,buffer,buffSize);
            Fputs(buffer,stdout);
        }
        buffer[0]=EOF;
        Rio_writen(Connfd,buffer,buffSize);
                    Fputs(buffer,stdout);


    }
}
