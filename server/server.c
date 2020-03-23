/*
 * echoserveri.c - An iterative echo server
 */

#include "../libs/csapp.h"
#include "../libs/utils.h"
#include "../libs/cmds.h"

#define MAX_NAME_LEN 256

void handler(int signal)
{
    for (int i = 0; i < NPROC; i++)
    {
        Kill(child_processes[i], SIGINT);
    }
    exit(0);
}
/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    Signal(SIGINT,handler);
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    port = 2121;

    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);

    /* ------------------------------- init pool ------------------------------ */
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

            cmd(connfd);
            
        }
        else{
            
        }
    }
    Wait(NULL);
    exit(0);
}
