/*
 * echoclient.c - An echo client
 */
#include "param.h"

void login(char *username, char *password)
{
    printf("Authentification utilisateur (entrer une chaine vide pour une connection annonyme)\n");
    printf("username: \n");
    fgets(username, sizeof(username), stdin);
    printf("password: \n");
    fgets(password, sizeof(password), stdin);
}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;
    FILE *f;

    int logged = 0; //utilisateur connecté?

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

    //Ici, on le serveur va dire au client sur quel fils il faut se connecter
    int elu;
    Rio_readnb(&rio, &elu, sizeof(elu));
    if(elu == -1){
        //Aucun serveur n'est libre...
        printf("Aucun serveur n'est libre, il faut qu'un autre client ce deconnecte.\n");
        exit(0);
    }
    //On se connecte
    printf("Conecting to port : %i",(elu + 2122));
    clientfd = Open_clientfd(host, elu + 2122);
    Rio_readinitb(&rio, clientfd);

    while (1)
    {
        printf("ftp>");
        if (Fgets(buf, MAXLINE, stdin) == NULL)
        {
            break;
        }

        Rio_writen(clientfd, buf, strlen(buf));

        //Si on écrit bye, on termine
        char is_bye[4];
        strncpy(is_bye, buf, 3);
        is_bye[3] = '\0';

        //ls
        char is_ls[3];
        strncpy(is_ls, buf, 2);
        is_ls[2] = '\0';

        //login
        char is_login[6];
        strncpy(is_login, buf, 5);
        is_login[5] = '\0';

        //cd
        char is_cd[4];
        strncpy(is_cd, buf, 3);
        is_cd[3] = '\0';

        //mkdir
        char is_mkdir[7];
        strncpy(is_mkdir, buf, 6);
        is_mkdir[6] = '\0';

        //put
        char is_put[5];
        strncpy(is_put, buf, 4);
        is_put[4] = '\0';

        //rm
        char is_rm[4];
        strncpy(is_rm, buf, 3);
        is_rm[3] = '\0';

        //rm -r
        char is_rm_r[7];
        strncpy(is_rm_r, buf, 6);
        is_rm_r[6] = '\0';

        //pwd
        char is_pwd[4];
        strncpy(is_pwd, buf, 3);
        is_pwd[3] = '\0';

        //get
        char is_get[5];
        strncpy(is_get, buf, 4);
        is_get[4] = '\0';

        if (strcmp(is_bye, "bye") == 0)
        {
            exit(0);
        }
        if (strcmp(is_bye, "cd ") == 0)
        {
        }
        else if (strcmp(is_ls, "ls") == 0)
        {
            ssize_t seen;
            while ((seen = Rio_readnb(&rio, buf, buffSize)) > 0)
            {
                if (buf[0] == EOF)
                {
                    break;
                }
                printf("%s\n", buf);
            }
        }
        else if (strcmp(is_login, "login") == 0)
        {
            logged = 0;
            char *username = malloc(CREDLENGTH * sizeof(char));
            char *password = malloc(CREDLENGTH * sizeof(char));
            ;
            login(username, password);
            Rio_writen(clientfd, username, CREDLENGTH);
            Rio_writen(clientfd, password, CREDLENGTH);
            //Verification du login
            Rio_readnb(&rio, buf, buffSize);
            if (buf[0] == '+')
            {
                logged = 1;
                printf("Connecté ! \n");
                printf("Bienvenue %s\n", username);
            }
            else
            {
                printf("Nom utilisateur ou mdp incorrecte...\n");
            }
        }
        else if (strcmp(is_pwd, "pwd") == 0)
        {
            Rio_readnb(&rio, buf, buffSize);
            printf("%s\n", buf);
        }
        else if (strcmp(is_mkdir, "mkdir ") == 0)
        {
            if (logged == 1)
            {
                Rio_readnb(&rio, buf, buffSize);
                if (buf[0] == '+')
                {
                    printf("Le dossier a été créé avec succès\n");
                }
                else
                {
                    printf("Le dossier existe déjà\n");
                }
            }
            else
            {
                printf("Vous devez être connecté pour executer cette commande...\n Tapez 'login' pour vous connecter\n");
            }
        }
        else if (strcmp(is_rm_r, "rm -r ") == 0)
        {
            if (logged == 1)
            {
                Rio_readnb(&rio, buf, buffSize);
                if (buf[0] == '+')
                {
                    printf("Le dossier a été supprimé avec succès\n");
                }
                else
                {
                    printf("Erreur dans la suppression du dossier\n");
                }
            }
            else
            {
                printf("Vous devez être connecté pour executer cette commande...\n Tapez 'login' pour vous connecter\n");
            }
        }
        else if (strcmp(is_rm, "rm ") == 0)
        {
            if (logged == 1)
            {
                Rio_readnb(&rio, buf, buffSize);
                if (buf[0] == '+')
                {
                    printf("Le fichier a été supprimé avec succès\n");
                }
                else
                {
                    printf("Erreur dans la suppression du fichier\n");
                }
            }
            else
            {
                printf("Vous devez être connecté pour executer cette commande...\n Tapez 'login' pour vous connecter\n");
            }
        }
        else if (strcmp(is_put, "put ") == 0)
        {
            if (logged == 1)
            {
                //On enleve le retour chariot
                char *query = strdup(buf);
                for (int i = 0; query[i] != '\0'; i++)
                {
                    if (query[i] == '\n')
                    {
                        query[i] = '\0';
                    }
                }
                //Pour le reste, meme strategie que "get" (voir partie serveur)
                int query_len;
                if (((query_len = strlen(query)) > 4) && (query[0] == 'p') && (query[1] == 'u') && (query[2] == 't') && (query[3] == ' '))
                {
                    char fileBuffer[buffSize];
                    char *filename = malloc(8 * sizeof(char));
                    for (int i = 4; i < query_len; i++)
                    {
                        filename[i - 4] = query[i];
                        filename[i - 3] = '\0';
                    }

                    //start transfert
                    f = fopen(filename, "r");
                    while (fgets(fileBuffer, sizeof(fileBuffer), f) != NULL)
                    {
                        int n = sizeof(fileBuffer);
                        rio_writen(clientfd, fileBuffer, n);
                    }
                    //send EOF when finished
                    fileBuffer[0] = EOF;
                    Rio_writen(clientfd, fileBuffer, sizeof(fileBuffer));
                    printf("fichier uploadé\n");
                }
            }
            else
            {
                printf("Vous devez être connecté pour executer cette commande...\n Tapez 'login' pour vous connecter\n");
            }
        }
        else if (strcmp(is_get, "get ") == 0)
        {
            ssize_t seen;
            ssize_t total_read;
            if ((seen = Rio_readnb(&rio, buf, buffSize)) > 0)
            {
                if (buf[0] == '+')
                {
                    f = fopen("transfered", "a");
                    fflush(f);
                }
                else
                {
                    f = fopen("transfered", "w");
                    Fputs(buf, f);
                    fflush(f);
                }
            }
            int msec = 0;
            clock_t before = clock(); //infos sur le telechargement
            while ((seen = Rio_readnb(&rio, buf, buffSize)) > 0)
            {
                if (buf[0] == EOF)
                {
                    break;
                }
                Fputs(buf, f);
                fflush(f);
                clock_t difference = clock() - before;
                msec = difference * 1000 / CLOCKS_PER_SEC;
                total_read += seen;
            }
            printf("Fichier téléchargé avec succès\n");
            int kb = total_read / 1000;
            int total_sec = msec / 1000;
            int kb_sec = kb / total_sec;
            printf("%ld bytes received in %d seconds (%d Kbytes/s)\n", total_read, total_sec, kb_sec);
        }
        else
        {
            printf("Commande invalide\n");
        }
    }
    Close(clientfd);
    exit(0);
}
