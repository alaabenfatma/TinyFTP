#include "csapp.h"

#define MAX_NAME_LEN 256
#define NPROC 5

pid_t* child_processes;

void echo(int connfd);

void handler(int signal){
    for(int i=0;i<NPROC;i++){
        Kill(child_processes[i],SIGINT);
    }
    exit(0);
}

int main(int argc, char **argv)
{

    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    
    //All child processes
    child_processes = malloc(NPROC*sizeof(pid_t));

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);

    //POOL
    pid_t pid = 1;
    for (int i = 0; i < NPROC; i++)
    {
        if (pid > 0)
        {
            pid = Fork();
            child_processes[i] = pid;
        }
    }

    while (1)
    {
        
        if (pid == 0)
        {
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

            /* determine the name of the client */
            Getnameinfo((SA *)&clientaddr, clientlen,
                        client_hostname, MAX_NAME_LEN, 0, 0, 0);

            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                      INET_ADDRSTRLEN);

            printf("server connected to %s (%s)\n", client_hostname,
                   client_ip_string);
            

            echo(connfd);
            Close(connfd);
        }
    }
    Wait(NULL);
    exit(0);
}
