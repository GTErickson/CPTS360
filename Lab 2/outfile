#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[ ], char *env[ ]){
    for (int i; *(env + i) != 0; ++i){
        printf("%s\n", *(env + i));
    }

    return 0;
}