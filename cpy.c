#include <stdio.h>                                              // include proper header

#define BUFFSIZE    512                                         // power of 2 is kind to system

int main(void)
{
    char buffer[BUFFSIZE];
    size_t bytes;
    FILE *fin, *fou;
    fin = fopen("bird.jpg", "rb");
    fou = fopen("bb.jpg", "wb");
    if(fin == NULL || fou == NULL)
        return 1;                                               // or other action
    while ((bytes = fread(buffer, 1, BUFFSIZE, fin)) != 0) {
        if(fwrite(buffer, 1, bytes, fou) != bytes) {
            return 1;                                           // or other action
        }
    }
    fclose(fou);
    fclose(fin);
    return 0;
}