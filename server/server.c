#include "../libs/utils.h"

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
    char client_hostname[256];
    int elu = 0;
    int master = 0;
    strcpy(global_path, ".");
    //paramètres de connection
    port = 2121;
    clientlen = (socklen_t)sizeof(clientaddr);
    listenfd = Open_listenfd(port);
    int tourniquet = 1;

    //PIPE
    int pip1[2];
    pipe(pip1);
    int pip2[2];
    pipe(pip2);

    //POOL
    pid_t pid;

    pid_t *child_processes; //All child processes
    child_processes = malloc(NPROC * sizeof(pid_t));

    busy = initfield(); //Fichier qui indique un un serveur est occupé ou pas

    master = getpid();

    //AJOUTE : Gestion des clients par les NPROC fils
    int i;
    for (i = 0; i < NPROC; i++)
    {
        if (getpid() == master)
        {

            if ((pid = fork()) == -1)
            {
                perror("ERROR FORK");
                exit(1);
            }
            else
            {
                child_processes[i] = pid;
            }
        }
    }

    /* --------- Create the accounts database if does not exist already. -------- */

    initDB();
    while (1)
    {
        int i;
        if (master == getpid()) //père
        {
            //On utilise le modulo pour répartir de manière circulaire
            elu = (tourniquet) % NPROC;
            tourniquet++;
            int pid_elu = child_processes[elu];
            //On va écrire elu et son pid au fils
            for (i = 0; i < NPROC; i++)
            {
                write(pip1[1], &elu, sizeof(elu));
                write(pip2[1], &pid_elu, sizeof(pid_elu));
            }
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            /* determine the name of the client */

            //On cherche un esclave libre (hahaha)
            fflush(busy);
            elu = -1;
            for (i = 0; i < NPROC; i++)
            {
                bool occupied = getfield(i, busy);
                if (occupied == false)
                {
                    elu = i;
                    pid_elu = child_processes[elu];
                }
            }

            if (elu > -1)
            { //Si on a trouve un esclave libre:
                //On indique au client le nouveau port auquel il doit se connecter (voir client)
                Rio_writen(connfd, &elu, sizeof(elu));
            }
            else
            {
                //Tous les esclaves sont occupés...
                elu = 0;
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
                int listenfd2 = Open_listenfd(new_port);
                int connfd = Accept(listenfd2, (SA *)&clientaddr, &clientlen);
                Getnameinfo((SA *)&clientaddr, clientlen,
                            client_hostname, 256, 0, 0, 0);

                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                          INET_ADDRSTRLEN);
                printf("[SERVER] Server connected to " BOLD "%s" RESET " (" BOLD "%s" RESET ") at %s\n", client_hostname,
                       client_ip_string, currentTime());
                printf("This client has been linked to child [" CYAN "%d" RESET "]\n", elu_fils);

                //Deviens occupé
                setfield(elu_fils, '1', busy);
                fflush(busy);

                //Initialisation du repertoire courant:
                current_directory = opendir(".");
                connectedClients();
                s_cmd(connfd, elu_fils);
                Close(listenfd2);
                Close(connfd);
                closedir(current_directory);
                //Ce libère
                setfield(elu_fils, '0', busy);
                fflush(busy);
                
                connectedClients();
            }
        }
    }
    for (i = 0; i < NPROC; i++)
    {
        Wait(NULL);
    }
    exit(0);
}
