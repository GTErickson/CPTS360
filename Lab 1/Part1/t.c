/* Part 1 */

#include <stdio.h>
#include <stdlib.h>

typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";
int  BASE = 10;

void rpu(u32 x)
{
        char c;
        if (x) {
                c = ctable[x % BASE];
                rpu(x / BASE);
                putchar(c);
        }
}

void printu(u32 x)
{
        x == 0 ? putchar('0') : rpu(x);
        putchar(' ');
}

void prints(char* s){
    while (*s){ 
        putchar(*s);
        if (*s == '\n'){
            putchar('\r');
        }
        ++s;
    }
}

int printx(u32 x){
    prints("0x");
    BASE = 16;
    printu(x);
}

int printd(int x){
    if (x < 0){
        putchar('-');
        // Use twos complement 
        x = ~x + 1;  //or x = -x;
    }
    BASE = 10;
    printu(x);
}

void myprintf(char* fmt, ...){
    char* cp = fmt;
    int* ip = (&fmt) + 1;

    while(*cp){
        if (*cp == '%'){
            //Check next char
            cp++;
            if(*cp == 'c'){
                putchar(*ip);
            }
            else if (*cp == 's'){
                prints(*ip);
            }
            else if (*cp == 'u' || *cp == 'd'){
                printd(*ip);

            }
            else if (*cp == 'x'){
                printx(*ip);
            }
            else{
                prints("\n\nInvalid input");
            }
            ip++;
        }
        else{
            putchar(*cp);
        }
        cp++;
    }

}

int main(int argc, char* argv[], char* env[])
{
    //Printing argc, argv[], and env[]
    myprintf("argc=%d   argv[]=", argc);
    for(int i = 0; i < argc; ++i){
        myprintf("%s ", *(argv + i));
    }
    myprintf("   env[]=");
    for(int i = 0; i < 3; ++i){
        myprintf("%s ", *(env + i));
    }
    //Print test
    myprintf("\nchar=%c string=%s dec=%u hex=%x neg=%d\n\n\n", 'A', "this is a test", 100, 100, -100);
}
