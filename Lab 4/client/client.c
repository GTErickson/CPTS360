#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h> 
#include <arpa/inet.h>

#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>      
#include <time.h> 

struct sockaddr_in saddr;     // server addr struct

char *serverIP = "127.0.0.1"; // server IP address
//char *serverIP = "172.19.44.43"; // server IP address
#define PORT 1234             // server port number

int  sfd, n;                     // socket  

#define MAX   256
char line[MAX], in[MAX], cwd[128], temp[MAX];
char *name[64];  

struct stat mystat, *sp;
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int main(int argc, char *argv[], char *env[]) 
{ 
    int n;                      
    printf("1. create a TCP socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP); 
    saddr.sin_port = htons(PORT); 

    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 

    printf("********  processing loop  *********\n");
    while (1){
        getcwd(cwd, MAX);
        printf("input a line : ");
        bzero(line, MAX);                // zero out line[ ]
        fgets(line, MAX, stdin);         // get a line (end with \n) from stdin


        line[strlen(line)-1] = 0;        // kill \n at end
        strcpy(in, line);                //Save input
        tokenize();

        if (line[0]==0)                  // exit if NULL line
            exit(0); 
        
        //local commands
        if (!strcmp(name[0], "lpwd")){
            printf("%s\n", cwd);
        }
        else if (!strcmp(name[0], "lmkdir")){
            int r = mkdir(name[1], 0755);
        }
        else if (!strcmp(name[0], "lrmdir")){
            int r = rmdir(name[1]);
        }
        else if (!strcmp(name[0], "lcd")){
            int r = chdir(name[1]);
        }
        else if (!strcmp(name[0], "lls")){
            struct dirent *ep;
            DIR *dp = opendir(cwd);
            while (ep = readdir(dp)){
                ls_file(ep->d_name);
            }
            printf("END ls\n");
        }
        else if (!strcmp(name[0], "lcd")){
            int r = chdir(name[1]);
        }
        else if (!strcmp(name[0], "lcp")){
            lcp();
        }
        //Server commands
        else if (!strcmp(name[0], "ls")){
            //strcpy(in, line);
            n = write(sfd, line, MAX);
            printf("client: wrote n=%d bytes; send=[%s]\n", n, line);
            sprintf(temp,"END ls\n");
            while (strcmp(line, temp)){
                n = read(sfd, line, MAX);
                printf("%s", line);
            }
        }
        else if (!strcmp(name[0], "pwd")){
            n = write(sfd, line, MAX);
            printf("client: wrote n=%d bytes; send=[%s]\n", n, line);
            n = read(sfd, line, MAX);
            printf("client: read  n=%d bytes; recv=[%s]\n",n, line);
            printf("%s\n", line);
        }
        else if (!strcmp(name[0], "cp")){
            cp();
        }
        else{
            n = write(sfd, line, MAX);
            printf("client: wrote n=%d bytes; send=[%s]\n", n, line);
            n = read(sfd, line, MAX);
            printf("client: read  n=%d bytes; recv=[%s]\n",n, line);
        }
        

        // Send ENTIRE line to server
        // n = write(sfd, line, MAX);
        // printf("client: wrote n=%d bytes; send=[%s]\n", n, line);

        // Read a line from sock and show it
        //n = read(sfd, line, MAX);
        //printf("client: read  n=%d bytes; recv=[%s]\n",n, line);
    }
}

int tokenize() 
{
  char *s;
  s = strtok(in, " ");    
  n = 0;
  while(s){
    name[n++] = s;           // token string pointers   
    s = strtok(0, " ");
  }
  name[n] = 0;               // name[n] = NULL pointer
}

int ls_file(char *fname)
{
    struct stat fstat, *sp;
    int r, i;
    char ftime[64];
    sp = &fstat;
    if ( (r = lstat(fname, &fstat)) < 0){
        printf("can’t stat %s\n", fname);
        exit(1);
    }
    if ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
        printf("%c",'-');
    if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
        printf("%c",'d');
    if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
        printf("%c",'l');
    for (i=8; i >= 0; i--){
        if (sp->st_mode & (1 << i)) // print r|w|x
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]); // or print -
        }
        printf("%4d ",sp->st_nlink); // link count
        printf("%4d ",sp->st_gid); // gid
        printf("%4d ",sp->st_uid); // uid
        printf("%8d ",sp->st_size); // file size
        // print time
        strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form
        ftime[strlen(ftime)-1] = 0; // kill \n at end
        printf("%s ",ftime);
        // print name
        printf("%s", basename(fname)); // print file basename
        // print -> linkname if symbolic file
        // if ((sp->st_mode & 0xF000)== 0xA000){
        // // use readlink() to read linkname
        // printf(" -> %s", linkname); // print linked name
    // }
    printf("\n");
}

void lcp() {
    int fd_src, fd_dest, bytes, t = 0;
    char buf[MAX];
    if (n < 3){
        printf("lcp: Too many arguments");
        exit(-15);
    }
    fd_src = open(name[1], O_RDONLY);
    if (fd_src < 0) {
        printf("no src given");
        exit(-15);
    }
    fd_dest = open(name[2], O_WRONLY | O_CREAT, 0644);
    if (fd_dest < 0) {
        printf("no dest given");
        exit(-15);
    }
    bytes = read(fd_src, &buf, sizeof(buf));
    while (bytes){
        write(fd_dest, &buf, sizeof(buf));
        t += bytes;
        bytes = read(fd_src, &buf, sizeof(buf));
    }
    close(fd_src);
    close(fd_dest);
}

void cp(){
    int bytes, fd_dest, t = 0, data_size;
    n = write(sfd, line, MAX);
    fd_dest = open(name[2], O_WRONLY | O_CREAT, 0644);
    bytes = read(sfd, line, MAX);
    data_size = atoi(line);
    while (t < data_size){
        bytes = read(sfd, line, MAX);
        write(fd_dest, line, MAX);
        t += bytes;
    }
    close(fd_dest);
}