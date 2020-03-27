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
  //  int elu_temp = -1;

    //PIPE
    int pip1[2];
    pipe(pip1);
    int pip2[2];
    pipe(pip2);

    //POOL
    pid_t pid;

    pid_t *child_processes; //All child processes
    child_processes = malloc(NPROC * sizeof(pid_t));

    FILE *busy = initfield(); //Fichier qui indique un un serveur est occupé ou pas

    pid = 1;

    master = getpid(); //sauvegarde du pid père
    //Save all child processes

    for (int i = 0; i < NPROC; i++)
    {
        if (pid > 0) //père
        {
            pid = Fork();
            child_processes[i] = pid;
            //pid = Fork();
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

            //On cherche un esclave libre (hahaha)
            fflush(busy);
            while ((getfield(elu, busy) != 0) && (elu != NPROC))
            {
                elu = (tourniquet) % NPROC;
                tourniquet++;
                pid_elu = child_processes[elu];
            }
            if (elu != NPROC)
            { //Si on a trouve un esclave libre:
                //On indique au client le nouveau port auquel il doit se connecter (voir client)
                elu_temp = elu;
                Rio_writen(connfd, &elu, sizeof(elu));
            }
            else
            {
                //Tous les esclaves sont occupés...
                elu = 0;
               // elu_temp = -1;
                int err = -1;
                Rio_writen(connfd, &err, sizeof(err));
            }

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
                printf("I AM ! listening to port : %i\n", new_port);
                socklen_t clientlen2;
                struct sockaddr_in clientaddr2;
                printf("\n LISTENING TO PORT : %i \n", new_port);
                int listenfd2 = Open_listenfd(new_port);

                connfd = Accept(listenfd2, (SA *)&clientaddr2, &clientlen2);
                printf("Connected to child : %i !\n", elu_fils);

                //Deviens occupé
                setfield(elu_fils, '1', busy);

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
                    //Ce libère
                    setfield(elu_fils, '0', busy);
                    fflush(busy);
                }
                else
                {
                    //Cas de base
                    success = get(connfd);
                }
                Close(listenfd2);
                Close(connfd);
                closedir(dir);
                //Ce libère
                setfield(elu_fils, '0', busy);
                fflush(busy);
            }
        }
    }
    for (int i = 0; i < NPROC; i++)
    {
        Wait(NULL);
    }
    exit(0);
}
