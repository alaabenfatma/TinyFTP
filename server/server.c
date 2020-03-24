/*
 * echoserveri.c - An iterative echo server
 */

#include "../libs/utils.h"

#define MAX_NAME_LEN 256

void handler(int signal)
{
    for (int i = 0; i < NPROC; i++)
    {
        Kill(child_processes[i], SIGINT);
        Kill(child_processes[i], SIGTERM);
    }
    exit(0);
}
/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{

    /* -------------------------- Setting everuthing up ------------------------- */

    printf("Starting the server...");
    Signal(SIGINT, handler);
    EOF_BUFF = EOF;
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    int elu = 0;
    pid_t pid_elu = 0;
    int round_robin_radix = 1;
    int fd[2];
    pipe(fd);
    port = 2121;

    clientlen = (socklen_t)sizeof(clientaddr);
    current_directory = opendir(".");
    listenfd = Open_listenfd(port);
    printf(GREEN "OK" RESET "\n");
    /* ------------------------------- init pool ------------------------------ */
    pid_t pid;

    pid_t *child_processes; //All child processes
    child_processes = malloc(NPROC * sizeof(pid_t));

    pid = Fork();
    if (pid > 0)
    {
        //Save all child processes
        child_processes[0] = pid;
        //Declare the remaining childs
        for (int i = 1; i < NPROC; i++)
        {
            if ((pid = Fork()) > 0) //père
            {
                child_processes[i] = pid;
            }
        }
    }

    while (1)
    {
        if (pid > 0)
        {
            elu = NPROC % round_robin_radix;
            round_robin_radix++;
            pid_elu = child_processes[elu];
            write(fd[1], &elu, sizeof(elu));
            write(fd[1], &pid_elu, sizeof(pid_elu));
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            /* determine the name of the client */
            Getnameinfo((SA *)&clientaddr, clientlen,
                        client_hostname, MAX_NAME_LEN, 0, 0, 0);

            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                      INET_ADDRSTRLEN);
            time_t now = time(NULL);
            struct tm *t = localtime(&now);

            char date[100];
            strftime(date, sizeof(date) - 1, "%d/%m/%Y %H:%M", t);
            printf("Server connected to %s (" YELLOW "%s" RESET ") at (" CYAN "%s" RESET ")\n", client_hostname,
                   client_ip_string, date);
            //On indique au client le nouveau port auquel il doit se connecter (voir client)

            Rio_writen(connfd, &elu, sizeof(elu));
            Close(connfd);
        }
        else
        {
            int elu_fils; //On va lire "elu" du père avec un pipe.
            int pid_elu_fils;

            read(fd[0], &elu_fils, sizeof(elu_fils));
            read(fd[0], &pid_elu_fils, sizeof(pid_elu_fils));
            if (pid_elu_fils == getpid())
            {
                int next_port = 2122 + elu_fils;
                int listenfd = Open_listenfd(next_port);
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                printf("Connected to child : %i !\n", elu_fils);
            }
            cmd(connfd);
            Close(connfd);
            Close(listenfd);
        }
    }
    for (int i = 0; i < NPROC; i++)
    {
        Wait(NULL);
    }
    exit(0);
}
