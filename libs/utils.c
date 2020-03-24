#include "../libs/csapp.h"
#include "../libs/utils.h"

struct stat fileProperties(char *filename)
{
    struct stat st;
    stat(filename, &st);
    return st;
}
char *strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}
char *nameOfCrashedFile()
{
    FILE *f;
    f = fopen("crash.log", "r");
    char *fname = malloc(messageSize);
    fscanf(f, "%s", fname);
    fname = strtok(fname, ",");
    fclose(f);
    return fname;
}
long sizeOfCrashedFile()
{
    FILE *f;
    f = fopen("crash.log", "r");
    char *fname = malloc(messageSize);
    fscanf(f, "%s", fname);
    fname = strtok(fname, ",");
    fname = strtok(NULL, ",");
    fclose(f);
    char **err=malloc(sizeof(10));
    return (long)strtol(fname,err,10);
}

void printProgress(ssize_t downloaded,ssize_t size)
{
        printf("Downloading : %ld/%ld (%d%%)", downloaded,size,(int)percentage((double)size,(double)downloaded));
        printf("\r");
        fflush(stdout);
}

double percentage(double size, double downloaded)
{
    return (double)downloaded / size * 100.0;
}
int StartsWith(const char *a, const char *b)
{
    if (strncmp(a, b, strlen(b)) == 0)
        return 1;
    return 0;
}

char *getFirstArgument(char cmd[])
{
    char *argument = malloc(strlen(cmd));
    if (StartsWith(cmd, "get"))
    {
        int j = 0;
        for (int i = 4; i < strlen(cmd)-1; i++)
        {
            argument[j] = cmd[i];
            j++;
        }
    }
    else if (StartsWith(cmd, "echo"))
    {
        int j = 0;
        for (int i = 5; i < strlen(cmd)-1; i++)
        {
            argument[j] = cmd[i];
            j++;
        }
    }
    
    return argument;
}

/* ------------------------ return false if directory ----------------------- */

int is_file(char *path)
{
    return S_ISREG(fileProperties(path).st_mode);
}
