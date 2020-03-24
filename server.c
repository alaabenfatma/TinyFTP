#include "param.h"

void handler(int signal)
{
    //Handler de Ctrl-C
    Kill(0, SIGKILL);
    exit(0);
}

int main(int argc, char **argv)
{
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    int elu = 0;
    int master = 0;

    //paramètres de connection
    port = 2121;
    clientlen = (socklen_t)sizeof(clientaddr);
    listenfd = Open_listenfd(port);
    int success = 0; // utilisé pour voir si get à fonctionné correctement ou pas
    int tourniquet = 0;

    //PIPE
    int pip1[2];
    pipe(pip1);
    int pip2[2];
    pipe(pip2);

    //POOL
    pid_t pid;

    pid_t *child_processes; //All child processes
    child_processes = malloc(NPROC * sizeof(pid_t));

    pid = Fork();
    if (pid > 0)
    {
        master = getpid(); //sauvegarde du pid père
        //Save all child processes
        child_processes[0] = pid;
        //Declare the remaining childs
        for (int i = 1; i < NPROC; i++)
        {
            if (((pid = Fork()) > 0) && (master == getpid())) //père
            {
                child_processes[i] = pid;
            }
        }
    }

    while (1)
    {
        if (master == getpid()) //père
        {
            //On utilise le modulo pour répartir de manière circulaire
            elu = (tourniquet) % NPROC;
            tourniquet++;
            int pid_elu = child_processes[elu];
            //On va écrire elu et son pid au fils
            for (int i = 0; i < NPROC; i++)
            {
                write(pip1[1], &elu, sizeof(elu));
                write(pip2[1], &pid_elu, sizeof(pid_elu));
            }
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            /* determine the name of the client */
            Getnameinfo((SA *)&clientaddr, clientlen,
                        client_hostname, MAX_NAME_LEN, 0, 0, 0);

            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                      INET_ADDRSTRLEN);

            printf("server connected to %s (%s)\n", client_hostname,
                   client_ip_string);
            //On indique au client le nouveau port auquel il doit se connecter (voir client)
            Rio_writen(connfd, &elu, sizeof(elu));
            Close(connfd);
        }
        else
        {
            int elu_fils; //On va lire "elu" du père avec un pipe.
            int pid_elu_fils;
            read(pip1[0], &elu_fils, sizeof(elu_fils));
            read(pip2[0], &pid_elu_fils, sizeof(pid_elu_fils));
            if (pid_elu_fils == getpid())
            { //fils
                //definition d'un nouveau port pour ouverture de connection
                int new_port = 2122 + elu_fils;
                int listenfd2 = Open_listenfd(new_port);
                connfd = Accept(listenfd2, (SA *)&clientaddr, &clientlen);
                printf("Connected to child : %i !\n", elu_fils);
                //Initialisation du repertoire courant:
                dir = opendir("./");

                if (success == -1)
                {
                    //Cas où on a eu une erreur durant le transfert
                    //On continue à transferer là on s'était arrété
                    char fileBuffer[buffSize];
                    fileBuffer[0] = '+';
                    //On envoie ce caractère pour informer le client que son telechargement de
                    //fichier avait echoué
                    //Il peut ainsi ouvrir le fichier en "a" et non en "r" (voir coté client)
                    Rio_writen(connfd, fileBuffer, sizeof(fileBuffer));
                    success = get2(connfd, cr);
                }
                else if (success == -2)
                {
                    //On a eu un "bye" (voir coté client). On se deconnecte
                    success = 0;
                    Close(listenfd2);
                    Close(connfd);
                }
                else
                {
                    //Cas de base
                    success = get(connfd);
                }
                Close(listenfd2);
                Close(connfd);
                closedir(dir);
            }
        }
    }
    for (int i = 0; i < NPROC; i++)
    {
        Wait(NULL);
    }
    exit(0);
}
