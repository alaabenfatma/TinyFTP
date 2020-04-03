/*
 * echoclient.c - An echo client
 */
#include "../libs/utils.h"
int clientfd, port;
char filename[FILENAME_MAX];
char *host;
rio_t rio;
bool loggedIn = false;
char username[messageSize];
bool crashing = false;
void handler(int s)
{
    sleep(1);
    crashing = true;
    printf(MAGENTA BOLD "\nProgram is closing.\n" RESET);
    
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

    exit(0);
}

/* -------------------------------------------------------------------------- */
/*    A get function that will be used to download a file from the server.    */
/* -------------------------------------------------------------------------- */
void c_get(char *query)
{
    downloading = false;
    Rio_readinitb(&rio, clientfd);
    char *contents = malloc(buffSize);
    Rio_readnb(&rio, contents, 1);
    if (StartsWith(contents, "-"))
    {
        printf(RED "An error has occured on the server side. Please check your command.\n" RESET);
        fflush(stdout);
        return;
    }

    ssize_t original_size;
    rio_readnb(&rio, &original_size, sizeof(original_size));

    strcpy(filename, "downloads/");

    strcat(filename, fileBaseName(getFirstArgument(query)));
    gettimeofday(&start, NULL);
    FILE *f;
    f = fopen(filename, "wb");
    size_t chunk = 0;
    while (original_size != downloading)
    {
        rio_readnb(&rio, contents, buffSize);
        rio_readnb(&rio, &chunk, sizeof(long));
        fwrite(contents, 1, chunk, f);
        rio_readnb(&rio, &downloading, sizeof(long));
        printProgress("Downloading", downloading, original_size);
    }
    fflush(f);
    fclose(f);
    gettimeofday(&stop, NULL);
    downloading = false;

    /* ----------------------------- Download result ---------------------------- */
    double secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    off_t file_size = fileProperties(filename).st_size;
    printf(GREEN "File has been downloaded successfully.\n" RESET);
    printf("%ld bytes received in %f seconds (%f Kbytes/s)\n", file_size, secs, (file_size / 1024 / secs));
}

/* -------------------------------------------------------------------------- */
/*      This function allows the user to resume an interrupted download.      */
/* -------------------------------------------------------------------------- */
void c_resume()
{

    FILE *f;
    char crash_log[messageSize];
    strcpy(crash_log, "X");
    f = fopen("crash.log", "r");
    if (f == NULL)
    {
        printf(RED "No download to resume.\n" RESET);
        Rio_writen(clientfd, &crash_log, messageSize);
        return;
    }
    fscanf(f, "%s", crash_log);
    usleep(60000);
    Rio_writen(clientfd, &crash_log, messageSize);
    Rio_readinitb(&rio, clientfd);
    char contents[buffSize];
    Rio_readnb(&rio, &contents, sizeof(char));

    if (StartsWith(contents, "-"))
    {
        printf(RED "An error has occured on the server side. Please check your command.\n" RESET);
        fflush(stdout);
        return;
    }

    strcpy(filename, "downloads/");
    strcat(filename, nameOfCrashedFile());
    f = fopen(filename, "a");
    gettimeofday(&start, NULL);
    downloading = sizeOfCrashedFile(filename);
    printf("Resuming " BOLD "%s (%d" RESET " bytes) transfer has started...\n", nameOfCrashedFile(), downloading);
    fflush(stdout);
    ssize_t original_size;
    size_t chunk = 0;
    rio_readnb(&rio, &original_size, sizeof(original_size));
    while (original_size != downloading)
    {
        rio_readnb(&rio, contents, buffSize);
        rio_readnb(&rio, &chunk, sizeof(long));
        fwrite(contents, 1, chunk, f);
        rio_readnb(&rio, &downloading, sizeof(long));
        printProgress("Downloading", downloading, original_size);
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
        printf(RED "Directory could not be changed.\n" RESET);
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
        printf(RED "Directory could not be created.\n" RESET);
    }
}
void c_rm()
{
    /* -------- We have to ONLY check if the rm got executed well or not -------- */
    bool error = false;
    Rio_readinitb(&rio, clientfd);
    Rio_readnb(&rio, &error, sizeof(error));
    if (error == true)
    {
        printf(RED "File could not be removed.\n" RESET);
    }
}
void c_rmdir()
{
    /* -------- We have to ONLY check if the rm -r got executed well or not -------- */
    bool error = false;
    Rio_readinitb(&rio, clientfd);
    Rio_readnb(&rio, &error, sizeof(error));
    if (error == true)
    {
        printf(RED "Directory could not be removed.\n" RESET);
    }
}
void c_put(char *fname)
{
    FILE *f;
    char buffer[buffSize];
    char msg[messageSize];
    strcpy(msg, "+");
    f = fopen(fname, "rb");
    if (f == NULL || strstr(fname, "/") != NULL)
    {
        strcpy(msg, "-");
    }
    usleep(30000);
    Rio_writen(clientfd, msg, messageSize);
    if (strstr(msg, "-") != NULL)
    {
        printf(RED "%s does not exist or its names contains /.\n" RESET, fname);
        return;
    }
    Rio_readinitb(&rio, clientfd);
    /* ------------------------ Send file size to client ------------------------ */
    ssize_t s, size = fileProperties(fname).st_size;
    Rio_writen(clientfd, &size, sizeof(ssize_t));
    long position;
    printf("Send %s (%lu bytes)", fname, size);
    while ((s = fread(buffer, 1, buffSize, f)) != 0)
    {
        position = ftell(f);
        if (rio_writen(clientfd, buffer, buffSize) != buffSize)
        {
            printf(RED BOLD "An error has occured during the transfer.\n" RESET);
            fflush(stdout);
            return;
        };
        rio_writen(clientfd, &s, sizeof(long));
        rio_writen(clientfd, &position, sizeof(long));
        printProgress("Uploading : ", position, size);
    }
    fclose(f);
    printf(GREEN BOLD "File (%s) has been uploaded successfully.\n" RESET, fname);
    fflush(stdout);
}
void c_bye(bool forced)
{
    exit(0);
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
    port = 2121; /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = establishConnection(host, port, 5);
    welcome();
    char *query = malloc(MAXLINE);
    Rio_readinitb(&rio, clientfd);
    int clientX = clientfd;
    strcpy(username,"Anonymous");
    Rio_writen(clientfd,username,messageSize);
    while (1)
    {
        clientfd = runTimeCheck(clientX, "client");
        printf("[%d]ftp>", clientfd);
        if (fgets(query, messageSize, stdin) == NULL)
        {
            break;
        }
        Rio_writen(clientfd, query, messageSize);
        Rio_writen(clientfd,username,messageSize);
        if (StartsWith(query, "get "))
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
        else if (StartsWith(query, "cd "))
        {
            c_cd();
        }
        else if (StartsWith(query, "mkdir "))
        {
            if (loggedIn)
                c_mkdir();
            else
            {
                printf(BLUE BOLD "Log into your account to make a modification on the server-side.\n" RESET);
            }
        }
        else if (StartsWith(query, "clear"))
        {
            clear();
        }
        else if (StartsWith(query, "rm -r "))
        {
            if (loggedIn)
                c_rmdir();
            else
            {
                printf(BLUE BOLD "Log into your account to make a modification on the server-side.\n" RESET);
            }
        }
        else if (StartsWith(query, "rm "))
        {
            if (loggedIn)
                c_rm();
            else
            {
                printf(BLUE BOLD "Log into your account to make a modification on the server-side.\n" RESET);
            }
        }

        else if (StartsWith(query, "put "))
        {
            if (loggedIn)
                c_put(getFirstArgument(query));
            else
            {
                printf(BLUE BOLD "Log into your account to make a modification on the server-side.\n" RESET);
            }
        }
        else if (StartsWith(query, "help"))
        {
            help();
        }
        else if (StartsWith(query, "bye"))
        {
            c_bye(false);
        }
        else
        {
            if (strlen(query) > 0)
                printf(MAGENTA "This command is unknown.\n" RESET);
        }
        fflush(stdout);
    }
    Close(clientfd);
    exit(0);
}
