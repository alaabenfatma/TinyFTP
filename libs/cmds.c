#include "cmds.h"

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
