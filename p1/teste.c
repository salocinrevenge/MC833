#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAXDATASIZE 1000 // max number of bytes we can get at once

int main()
{
    char *received = "<CONTINUE> Legal! filme inserido!\n";
    printf("saida: %d\n ", strstr(received, "<CONTINUE>")!=NULL); 



}