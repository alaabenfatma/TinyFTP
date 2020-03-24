/*
 * echo - read and echo text lines until client closes connection
 */
#include "param.h"

int get(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    char *query = "";
    char *filename = "";
    FILE *f;
    int is_logged_in = 0;

    //utilisateurs : toto:tata et admin:nimda
    users[0][0] = "toto";
    users[0][1] = crypt("tata", "k7");
    users[1][0] = "admin";
    users[1][1] = crypt("nimda", "k7");
    users[2][0] = "root";
    users[2][1] = crypt("toor", "k7");

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

        /***Traitement de la requete***/

        //Si on a eu un "get"
        int query_len;
        if (((query_len = strlen(query)) > 4) && (query[0] == 'g') && (query[1] == 'e') && (query[2] == 't') && (query[3] == ' '))
        {
            char fileBuffer[buffSize];
            filename = malloc(8 * sizeof(char));
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
                //sauvegarder la ligne courante au cas où le client crash
                cr = ftell(f);
                fflush(f);
                fseek(f, cr, SEEK_SET);
                int seen;
                if ((seen = rio_writen(connfd, fileBuffer, n)) != (ssize_t)n)
                {
                    //Error:
                    printf("Erreur pendant le transfert, client deconnecté\n");
                    return -1;
                }
            }
            //send EOF when finished (Idk why, but it is somehow not detected otherwise)
            fileBuffer[0] = EOF;
            Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            printf("Fichier transferé\n");
        }
        //Si on a eu un "bye"
        else if (strcmp(query, "bye") == 0)
        {
            printf("Client deconnecté!\n");
            return -2;
        }
        //Si on a eu un "ls"
        else if (strcmp(query, "ls") == 0)
        {
            char fileBuffer[buffSize];
            struct dirent *ent;
            if (dir != NULL)
            {
                //On met le nom de tous les fichiers dans un buffer et on l'envoie au client
                while ((ent = readdir(dir)) != NULL)
                {
                    Rio_writen(connfd, ent->d_name, sizeof(fileBuffer));
                }
                fileBuffer[0] = EOF;
                Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            }
            else
            {
                //Erreur
                printf("Erreur dans ls\n");
            }
        }
        //Si on a eu un "login"
        else if (strcmp(query, "login") == 0)
        {
            char usr[buffSize];
            char pwd[buffSize];
            Rio_readnb(&rio, buf, CREDLENGTH);
            query = strdup(buf);

            for (int i = 0; query[i] != '\0'; i++)
            {
                if (query[i] == '\n')
                {
                    query[i] = '\0';
                }
            }
            strcpy(usr, query);
            Rio_readnb(&rio, buf, CREDLENGTH);

            query = strdup(buf);

            for (int i = 0; query[i] != '\0'; i++)
            {
                if (query[i] == '\n')
                {
                    query[i] = '\0';
                }
            }
            strcpy(pwd, query);
            is_logged_in = verify_login(usr, crypt(pwd, "k7"));
            char fileBuffer[buffSize];
            if (is_logged_in != 1)
            {
                fileBuffer[0] = '-';
            }
            else
            {
                //On envoie '+' si username et password sont bons
                fileBuffer[0] = '+';
            }
            Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
        }
        //Si on a eu un "pwd"
        else if (strcmp(query, "pwd") == 0)
        {
            char fileBuffer[buffSize];
            getcwd(fileBuffer, sizeof(fileBuffer));
            Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
        }
        //Si on a eu un "cd"
        else if (((query_len = strlen(query)) > 3) && (query[0] == 'c') && (query[1] == 'd') && (query[2] == ' '))
        {
            //On recupère le nom du dossier
            filename = malloc(8 * sizeof(char));
            for (int i = 3; i < query_len; i++)
            {
                filename[i - 3] = query[i];
                filename[i - 2] = '\0';
            }
            //On modifie le dossier courant avec le nom du dossier obtenu
            if ((dir = opendir(filename)) != NULL)
            {
                chdir(filename);
                printf("Accédé à %s\n", filename);
            }
            else
            {
                printf("Erreur: %s n'est pas un dossier ou n'existe pas\n", filename);
            }
        }
        //Si on a eu un "mkdir"
        else if (((query_len = strlen(query)) > 6) && (query[0] == 'm') && (query[1] == 'k') && (query[2] == 'd') && (query[3] == 'i') && (query[4] == 'r') && (query[5] == ' '))
        {
            if (is_logged_in == 1)
            {
                //On recupère le nom du dossier à créer
                filename = malloc(8 * sizeof(char));
                for (int i = 6; i < query_len; i++)
                {
                    filename[i - 6] = query[i];
                    filename[i - 5] = '\0';
                }
                struct stat st = {0};
                char fileBuffer[buffSize];
                if (stat(filename, &st) == -1)
                {
                    //Si il n'existe pas, on le créer et on dit au client que c'est bon
                    mkdir(filename, 0700);
                    fileBuffer[0] = '+';
                }
                else
                {
                    //Sinon, on dit au client qu'il existe déjà
                    fileBuffer[0] = '-';
                }
                Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            }
        }
        //Si on a eu un "put"
        else if (((query_len = strlen(query)) > 4) && (query[0] == 'p') && (query[1] == 'u') && (query[2] == 't') && (query[3] == ' '))
        {
            if (is_logged_in == 1)
            {
                //On recupère le nom du dossier à créer
                filename = malloc(8 * sizeof(char));
                for (int i = 4; i < query_len; i++)
                {
                    filename[i - 4] = query[i];
                    filename[i - 3] = '_';
                    filename[i - 2] = 't';
                    filename[i - 1] = '\0';
                }
                ssize_t seen;
                f = fopen(filename, "w");
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
        }
        //Si on a eu un "rm -r"
        else if (((query_len = strlen(query)) > 6) && (query[0] == 'r') && (query[1] == 'm') && (query[2] == ' ') && (query[3] == '-') && (query[4] == 'r') && (query[5] == ' '))
        {
            if (is_logged_in == 1)
            {
                //On recupère le nom du dossier à supprimer
                filename = malloc(20 * sizeof(char));
                for (int i = 6; i < query_len; i++)
                {
                    filename[i - 6] = query[i];
                    filename[i - 5] = '\0';
                }
                char fileBuffer[buffSize];
                //printf("%s\n",filename);
                if (remove_directory(filename) >= 0)
                {
                    //On informe le client que le fichier a été effacé avec succès
                    fileBuffer[0] = '+';
                }
                else
                {
                    //Sinon, on dit au client qu'une erreur est survenue
                    fileBuffer[0] = '-';
                }
                Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            }
        }
        //Si on a eu un "rm"
        else if (((query_len = strlen(query)) > 3) && (query[0] == 'r') && (query[1] == 'm') && (query[2] == ' '))
        {
            if (is_logged_in == 1)
            {
                //On recupère le nom du fichier à supprimer
                filename = malloc(8 * sizeof(char));
                for (int i = 3; i < query_len; i++)
                {
                    filename[i - 3] = query[i];
                    filename[i - 2] = '\0';
                }
                char fileBuffer[buffSize];
                if (remove(filename) == 0)
                {
                    //On informe le client que le fichier a été effacé avec succès
                    fileBuffer[0] = '+';
                }
                else
                {
                    //Sinon, on dit au client qu'une erreur est survenue
                    fileBuffer[0] = '-';
                }
                Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            }
        }
    }
    return 0;
}

int get2(int connfd, long pos)
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

        //get the filename
        if ((strlen(query) > 4) && (query[0] == 'g') && (query[1] == 'e') && (query[2] == 't') && (query[3] == ' '))
        {
            filename = malloc(8 * sizeof(char));
            for (int i = 4; i < 12; i++)
            {
                filename[i - 4] = query[i];
                filename[i - 3] = '\0';
            }

            //start transfert
            f = fopen(filename, "r");
            //On continue depuis là où on s'était arreté
            fflush(f);
            fseek(f, pos, SEEK_SET);
            while (fgets(fileBuffer, sizeof(fileBuffer), f) != NULL)
            {
                int n = sizeof(fileBuffer);
                //sauvegarder la ligne courante au cas où le client crash
                cr = ftell(f);
                fflush(f);
                fseek(f, cr, SEEK_SET);
                if (rio_writen(connfd, fileBuffer, n) != (ssize_t)n)
                {
                    //Error:
                    printf("Erreur pendant le transfert, client deconnecté\n");
                    return -1;
                }
            }
            //send EOF when finished (Idk why, but it is somehow not detected otherwise)
            fileBuffer[0] = EOF;
            Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
            printf("Fichier transferé\n");
        }
    }
    return 0;
}

int remove_directory(const char *path)
{
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d)
    {
        struct dirent *p;

        r = 0;

        while (!r && (p = readdir(d)))
        {
            int r2 = -1;
            char *buf;
            size_t len;

            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            {
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf)
            {
                struct stat statbuf;

                snprintf(buf, len, "%s/%s", path, p->d_name);

                if (!stat(buf, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        r2 = remove_directory(buf);
                    }
                    else
                    {
                        r2 = unlink(buf);
                    }
                }

                free(buf);
            }

            r = r2;
        }

        closedir(d);
    }

    if (!r)
    {
        r = rmdir(path);
    }

    return r;
}

//retourne 1 si username et password correspondent à un user et 0 sinon
int verify_login(char *username, char *password)
{
    int i = 0;
    while ((i < MAXUSERS) && (users[i][0] != NULL))
    {
        if ((strcmp(users[i][0], username) == 0) && (strcmp(users[i][1], password) == 0))
        {
            return 1;
        }
        else
        {
            i++;
        }
    }
    return 0;
}
