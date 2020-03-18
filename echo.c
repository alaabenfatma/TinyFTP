/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"

void get(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    char *query = "";
    char *filename = "";
    char fileBuffer[255];
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

        //get the filename
        if ((strlen(query) > 4) && (query[0] == 'g') && (query[1] == 'e') && (query[2] == 't') && (query[3] == ' '))
        {
            printf("%li\n",strlen(query));
            filename = malloc(8 * sizeof(char));
            printf("%s\n",query);
            for (int i = 4; i < 12; i++)
            {
                filename[i - 4] = query[i];
                filename[i - 3] = '\0';
            }

            //start transfert
            printf("%s\n",filename);
            f = fopen(filename, "r");
            while (fgets(fileBuffer, 255, f) != NULL)
            {
                Rio_writen(connfd, fileBuffer, 255);
            }
            printf("Fichier transferé\n");
        }
    }
}
