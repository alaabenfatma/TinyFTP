//Permettera de gérer le fichier qui indique si un serveur esclave est occupé

#include "param.h"

FILE *initfield()
{
    FILE *stream = fopen("busy", "w+");
    for (int i = 0; i < NPROC - 1; i++)
    {
        fputc('0', stream);
        fputc(';', stream);
    }
    fputc('0', stream);
    fflush(stream);
    return stream;
}

//modifie la colone el par num dans p
void setfield(int el, char num, FILE *p)
{
    rewind(p);
    if (el == 0)
    {
        fputc(num, p);
    }
    else
    {
        char ch;
        int nbv = 0;
        while (((ch = fgetc(p)) != EOF) && (nbv != el - 1))
        {
            if (ch == ';')
            {
                nbv++;
            }
        }
        if (ch != EOF)
        {
            ch = fgetc(p);
            if (ch != EOF)
            {
                fputc(num, p);
            }
            else
            {
                printf("%i is too far\n", el);
            }
        }
        else
        {
            printf("%i is too far\n", el);
        }
    }
    fflush(p);
}

//rend la colone el du fichier p
int getfield(int el, FILE *p)
{
    rewind(p);
    char ch = '\0';
    if (el == 0)
    {
        ch = fgetc(p);
        return (ch - '0');
    }
    else
    {
        int nbv = 0;
        while ((ch!= EOF) && (nbv != el ))
        {
            ch = fgetc(p);
            if (ch == ';')
            {
                nbv++;
            }
        }
        if (ch != EOF)
        {
            ch = fgetc(p);
            return (ch - '0');
        }
        else
        {
            printf("%i is too far\n", el);
            return -1;
        }
    }
}

/*
int main()
{
    FILE *stream = initfield();
    setfield(2, '5', stream);
    printf("%i\n", getfield(2, stream));
}
*/