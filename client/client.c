/*
 * echoclient.c - An echo client
 */
#include "../libs/csapp.h"
#include "../libs/utils.h"
#include "../libs/cmds.h"
struct timeval stop, start;
bool downloading = false;
char filename[FILENAME_MAX];
void handler(int s)
{
    printf("Program is closing.");
    Sleep(1);
    printf("%d\n",downloading);
    if (downloading != false)
    {
        ssize_t size = fileProperties(filename).st_size;
        printf("You are still downloading the file : loaded bytes %ld\n", size);
        FILE *tmp;
        tmp = fopen("crash.log", "w");
        strcpy(filename, strremove(filename,"downloads/")); 
        fprintf(tmp,"%s,%ld",filename,size);
        fclose(tmp);
    }
    exit(1);
}
int main(int argc, char **argv)
{
    Signal(SIGINT,handler);
    
    int clientfd, port;

    char *host;

    rio_t rio;

    if (argc != 2)
    {
        fprintf(stderr, YELLOW "usage: %s <host>\n" RESET, argv[0]);
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
    printf(GREEN "client connected to server OS \n" RESET);
    char *query = malloc(MAXLINE);
    while (1)
    {
        printf("ftp>");
        if (fgets(query, messageSize, stdin) == NULL)
        {
            break;
        }
        Rio_writen(clientfd, query, strlen(query));

        Rio_readinitb(&rio, clientfd);
        if (StartsWith(query, "echo"))
        {
            Rio_readlineb(&rio, query, messageSize);
        }
        else if (StartsWith(query, "get"))
        {
            
            
            Rio_readinitb(&rio, clientfd);
            char contents[buffSize];
            if ((Rio_readlineb(&rio, contents, buffSize)) > 0)
            {
                if (StartsWith(contents, "-"))
                {
                    printf(RED "An error has occured on the server side. Please check your command.\n" RESET);
                    fflush(stdout);
                    continue;
                }
                else
                {
                }
            }
            printf("File transfer has started...\n");
            
            strcpy(filename, "downloads/");
            ssize_t s;
            strcat(filename, getFirstArgument(query));
            FILE *f;
            f = fopen(filename, "w");
            gettimeofday(&start, NULL);
            Rio_readinitb(&rio, clientfd);
            while ((s = Rio_readlineb(&rio, contents, buffSize)) > 0)
            {
                if (contents[0] == EOF || sizeof contents == 0)
                {
                    break;
                }
                Fputs(contents, f);
                downloading =+sizeof(contents);
                fflush(stdout);

            }
            fflush(f);
            fclose(f);
            gettimeofday(&stop, NULL);
            downloading = 0;
            double secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
            off_t file_size = fileProperties(filename).st_size;
            printf(GREEN "File has been downloaded successfully.\n" RESET);
            printf("%ld bytes received in %f seconds (%f Kbytes/s)\n", file_size, secs, (file_size / 1024 / secs));
        }
        else if(StartsWith(query,"resume")){
            FILE *f;
            f = fopen("crash.log","r");
            if(f==NULL){
                printf(RED "No download to resume." RESET);
                continue;
            }
            char *crash_log = malloc(messageSize);
            
            fscanf(f,"%s",crash_log);
            Rio_writen(clientfd,crash_log,messageSize);

            Rio_readinitb(&rio, clientfd);
            char contents[buffSize];
            if ((Rio_readlineb(&rio, contents, buffSize)) > 0)
            {
                if (StartsWith(contents, "-"))
                {
                    printf(RED "An error has occured on the server side. Please check your command.\n" RESET);
                    fflush(stdout);
                    continue;
                }
                else
                {
                }
            }
            printf("File transfer has started...\n");
            
            ssize_t s;
            strcpy(filename,"downloads/");
            strcat(filename, nameOfCrashedFile());
            f = fopen(filename, "a");
            gettimeofday(&start, NULL);
            Rio_readinitb(&rio, clientfd);
            while ((s = Rio_readlineb(&rio, contents, buffSize)) > 0)
            {
                if (contents[0] == EOF || sizeof contents == 0)
                {
                    break;
                }
                Fputs(contents, f);
                downloading =+sizeof(contents);
                fflush(stdout);

            }
            fflush(f);
            fclose(f);
            gettimeofday(&stop, NULL);
            downloading = 0;
            double secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
            off_t file_size = fileProperties(filename).st_size;
            printf(GREEN "%ld bytes received in %f seconds (%f Kbytes/s)\n" RESET, file_size, secs, (file_size / 1024 / secs));
        }
    }

    Close(clientfd);
    exit(0);
}
