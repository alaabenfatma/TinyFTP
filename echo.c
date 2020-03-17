/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"

void echo(int connfd)
{
    /*
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %u bytes\n", (unsigned int)n);
        Rio_writen(connfd, buf, n);
    }
    */

    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    char *query = "";
    char* filename="";

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
        
        //////////////////////////

        //get the filename
        if((strlen(query)>4) && (query[0]=='g') && (query[1]=='e') && (query[2]=='t') && (query[3]==' ')){
            filename = malloc((strlen(query)-5)*sizeof(char));
            for(int i=4;i<strlen(query);i++){
                filename[i-4] = query[i];
            }
            FILE* f = fopen("filename","r");
            //TODO: TO CONTINUE
        }
        ///////////////
        //printf("server received %u bytes\n", (unsigned int)n);
        Rio_writen(connfd, buf, n);
    }
}
