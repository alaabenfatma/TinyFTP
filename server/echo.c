/*
 * echo - read and echo text lines until client closes connection
 */
#include "../libs/csapp.h"
#include "../libs/param.h"
int StartsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
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
    FILE *f; 

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
        puts(query);
        //get the filename
        
        if (StartsWith(query,"get"))
        {
            filename = malloc(8 * sizeof(char));
            for (int i = 4; i < 12; i++)
            {
                filename[i - 4] = query[i];
                filename[i - 3] = '\0';
            }

            //start transfert
            f = fopen(filename, "r");
            while (fgets(fileBuffer, sizeof(fileBuffer), f) != NULL)
            {
                Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            }
            //send EOF when finished (Idk why, but it is somehow not detected otherwise)
            fileBuffer[0] = EOF;
            Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            printf("Transfer successfully complete.\n");
        }
        else{
            return ;
        }
    }
}
