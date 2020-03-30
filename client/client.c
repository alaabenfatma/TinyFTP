/*
 * echoclient.c - An echo client
 */
#include "../libs/utils.h"
int clientfd, port;
char filename[FILENAME_MAX];
char *host;
rio_t rio;
bool loggedIn = false;
void handler(int s)
{
    printf("Program is closing.");
    printf("%d\n", downloading);
    if (downloading != false)
    {
        ssize_t size = fileProperties(filename).st_size;
        printf("You are still downloading the file : loaded bytes %ld\n", size);
        FILE *tmp;
        tmp = fopen("crash.log", "w");
        strcpy(filename, strremove(filename, "downloads/"));
        fprintf(tmp, "%s,%d", filename, downloading);
        fclose(tmp);
    }
    Close(clientfd);
    exit(1);
}

/* -------------------------------------------------------------------------- */
/*    A get function that will be used to download a file from the server.    */
/* -------------------------------------------------------------------------- */
void c_get(char *query)
{
    int old_fd = clientfd;
    Rio_readinitb(&rio, clientfd);
    char contents[buffSize];
    if ((Rio_readnb(&rio, contents, 1)) > 0)
    {
        if (StartsWith(contents, "-"))
        {
            printf(RED "An error has occured on the server side. Please check your command.\n" RESET);
            fflush(stdout);
            return;
        }
        else
        {
        }
    }
    ssize_t original_size, s;
    Rio_readnb(&rio, &original_size, sizeof(original_size));

    strcpy(filename, "downloads/");

    strcat(filename, fileBaseName(getFirstArgument(query)));
    FILE *f;
    f = fopen(filename, "w");
    gettimeofday(&start, NULL);
    Rio_readinitb(&rio, clientfd);
    while ((s = Rio_readnb(&rio, contents, buffSize)) > 0)
    {
        if (contents[0] == EOF || sizeof contents == 0)
        {
            break;
        }
        Rio_readnb(&rio, &downloading, __SIZEOF_LONG__);
        Fputs(contents, f);
        printProgress("Downloading : ", downloading, original_size);
    }
    fflush(f);
    fclose(f);
    gettimeofday(&stop, NULL);
    downloading = 0;
    double secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    off_t file_size = fileProperties(filename).st_size;
    printf(GREEN "File has been downloaded successfully.\n" RESET);
    printf("%ld bytes received in %f seconds (%f Kbytes/s)\n", file_size, secs, (file_size / 1024 / secs));
    clientfd = old_fd;
}

/* -------------------------------------------------------------------------- */
/*      This function allows the user to resume an interrupted download.      */
/* -------------------------------------------------------------------------- */
void c_resume()
{
    FILE *f;
    f = fopen("crash.log", "r");
    if (f == NULL)
    {
        printf(RED "No download to resume." RESET);
        return;
    }
    char *crash_log = malloc(messageSize);

    fscanf(f, "%s", crash_log);
    Rio_writen(clientfd, crash_log, messageSize);
    Rio_readinitb(&rio, clientfd);
    char contents[buffSize];
    if ((Rio_readnb(&rio, contents, 1)) > 0)
    {
        if (StartsWith(contents, "-"))
        {
            printf(RED "An error has occured on the server side. Please check your command.\n" RESET);
            fflush(stdout);
            return;
        }
        else
        {
        }
    }
    ssize_t s;
    strcpy(filename, "downloads/");
    strcat(filename, nameOfCrashedFile());
    f = fopen(filename, "a");
    gettimeofday(&start, NULL);
    Rio_readinitb(&rio, clientfd);
    downloading = sizeOfCrashedFile(filename);
    printf("Resuming file transfer has started...\n");

    while ((s = Rio_readnb(&rio, contents, buffSize)) > 0)
    {
        if (contents[0] == EOF || sizeof contents == 0)
        {
            break;
        }
        Fputs(contents, f);
        downloading = +sizeof(contents);
    }
    fflush(f);
    fclose(f);
    gettimeofday(&stop, NULL);
    downloading = 0;
    double secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    off_t file_size = fileProperties(filename).st_size;
    printf(GREEN "%ld bytes received in %f seconds (%f Kbytes/s)\n" RESET, file_size, secs, (file_size / 1024 / secs));
    remove("crash.log");
}

/* -------------------------------------------------------------------------- */
/*          This functions gets the files & folders from the server.          */
/* -------------------------------------------------------------------------- */
void c_ls()
{
    char type = 'f';
    char *fname = malloc(messageSize);
    Rio_readinitb(&rio, clientfd);

    while ((Rio_readnb(&rio, fname, messageSize)))
    {
        Rio_readnb(&rio, &type, sizeof(char));
        if (fname[0] == EOF || sizeof fname == 0)
        {
            break;
        }

        if (type == 'f')
        {
            printf("%s\n", fname);
        }
        else
        {
            printf(GREEN "%s\n" RESET, fname);
        }
    }
    fflush(stdout);
}

/* -------------------------------------------------------------------------- */
/*             Return the current working directory of the server.            */
/* -------------------------------------------------------------------------- */
void c_pwd()
{
    char *working_directory = malloc(messageSize);
    Rio_readinitb(&rio, clientfd);
    Rio_readn(clientfd, working_directory, messageSize);
    printf("%s\n", working_directory);
}

/* -------------------------------------------------------------------------- */
/*                     Change the directory of FTP server                     */
/* -------------------------------------------------------------------------- */
void c_cd()
{
    /* -------- We have to ONLY check if the cd got executed well or not -------- */
    bool error = false;
    Rio_readinitb(&rio, clientfd);
    Rio_readnb(&rio, &error, sizeof(error));
    if (error == true)
    {
        printf(RED "Directory could not be changed." RESET);
    }
}

/* -------------------------------------------------------------------------- */
/*                     Create a folder on the server-side                     */
/* -------------------------------------------------------------------------- */
void c_mkdir()
{
    /* -------- We have to ONLY check if the mkdir got executed well or not -------- */
    bool error = false;
    Rio_readinitb(&rio, clientfd);
    Rio_readnb(&rio, &error, sizeof(error));
    if (error == true)
    {
        printf(RED "Directory could not be created." RESET);
    }
}
void c_rm(){
    /* -------- We have to ONLY check if the rm got executed well or not -------- */
    bool error = false;
    Rio_readinitb(&rio, clientfd);
    Rio_readnb(&rio, &error, sizeof(error));
    if (error == true)
    {
        printf(RED "File could not be removed." RESET);
    }
}
void c_rmdir(){
    /* -------- We have to ONLY check if the rm -r got executed well or not -------- */
    bool error = false;
    Rio_readinitb(&rio, clientfd);
    Rio_readnb(&rio, &error, sizeof(error));
    if (error == true)
    {
        printf(RED "Directory could not be removed." RESET);
    }

}
void c_put(char *fname){
    FILE *f;
    char buffer[buffSize];
    char *msg = malloc(sizeof(char));
    strcpy(msg, "+");
    f = fopen(fname, "rb");
    if (f == NULL)
    {
        strcpy(msg, "-");
    }
    Rio_writen(clientfd, msg, 1);
    if (StartsWith(msg, "-"))
    {
        return;
    }

    /* ------------------------ Send file size to server ------------------------ */
    ssize_t size = fileProperties(fname).st_size;
    Rio_writen(clientfd, &size, sizeof(size));
    long position;

    while (Fgets(buffer, buffSize, f) > 0)
    {
        position = ftell(f);
        if (rio_writen(clientfd, buffer, buffSize) != buffSize)
        {
            printf(RED "An error has occured during the transfer.\n" RESET);
            break;
        };
        rio_writen(clientfd, &position, __SIZEOF_LONG__);
        printProgress("Uploading : ", position, size);
    }
    buffer[0] = EOF;
    Rio_writen(clientfd, buffer, buffSize);
    printf(GREEN"\nFile has been uploaded successfully.\n"RESET);
    fclose(f);
}

int main(int argc, char **argv)
{
    
    Signal(SIGINT, handler);
    if (argc != 2)
    {
        fprintf(stderr, YELLOW "usage: %s <host>\n" RESET, argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;                          /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = establishConnection(host,port,5);
    welcome();
    char *query = malloc(MAXLINE);
    Rio_readinitb(&rio, clientfd);
    int clientX = clientfd;
    while (1)
    {
        clientfd = runTimeCheck(clientX);
        printf("ftp>");
        if (fgets(query, messageSize, stdin) == NULL)
        {
            break;
        }
        Rio_writen(clientfd, query, messageSize);
        if (StartsWith(query, "get"))
        {
            c_get(query);
        }
        else if (StartsWith(query, "resume"))
        {
            c_resume();
        }
        else if (StartsWith(query, "ls"))
        {
            c_ls();
        }
        else if (StartsWith(query, "pwd"))
        {
            c_pwd();
        }
        else if (StartsWith(query, "cd"))
        {
            c_cd();
        }
        else if (StartsWith(query, "mkdir"))
        {
            c_mkdir();
        }
        else if (StartsWith(query, "clear"))
        {
           clear();
        }
        else if (StartsWith(query, "rm"))
        {
           c_rm();
        }
        else if (StartsWith(query, "rm -r"))
        {
            c_rmdir();
        }
        else if (StartsWith(query, "put"))
        {
            c_put(getFirstArgument(query));
        }
        else if (StartsWith(query, "help"))
        {
            help();
        }
        else
        {
            if (strlen(query) > 0)
                printf(MAGENTA "This command is unknown.\n" RESET);
        }
        fflush(stdin);
    }
    Close(clientfd);
    exit(0);
}
