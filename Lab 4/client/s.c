#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <netinet/in.h>  
#include <sys/socket.h>

/************ socket address structure ************
struct sockaddr_in {
   sa_family_t    sin_family;   // AF_INET for TCP/IP
   in_port_t      sin_port;     // port number
   struct in_addr sin_addr;     // IP address
};
***************************************************/

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>

char *serverIP = "127.0.0.1";  // local host IP address
//char *serverIP = "172.19.44.43";  // by ifconfig
#define PORT 1234              // hardcoded port number

#define MAX 256
char line[MAX], in[MAX], cwd[MAX], root[MAX];
char *name[64]; 
int n;

struct stat mystat, *sp;
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int main() 
{
    struct sockaddr_in saddr;  // server addr struct
    struct sockaddr_in caddr;  // client addr struct 

    int sfd, cfd;              // sockets 
    int n, length;

    DIR *dp;
    struct dirent *ep;
    
    int me = getpid();
    getcwd(cwd, MAX);
    int l = chroot(cwd);
    
    printf("1. create a socket\n");
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
    
    printf("3. bind socket to server\n");
    if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(0); 
    }
      
    // Now server is ready to listen and verification 
    if ((listen(sfd, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(0); 
    }
    while(1){

       printf("server %d ready for client to connect\n", me);
      
       // Try to accept a client connection as socket descriptor cfd
       length = sizeof(caddr);
       cfd = accept(sfd, (struct sockaddr *)&caddr, &length);
       if (cfd < 0){
          printf("server: accept error\n");
          exit(1);
       }

       printf("server %d: accepted a client connection from\n", me);
       printf("-----------------------------------------------\n");
       printf("IP=%s  port=%d\n", inet_ntoa(caddr.sin_addr),
                                      ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

       int pid = fork();
       if (pid){
        close(cfd);
       } else {
        //me = getpid();
        // Processing loop
        while(1){
          getcwd(cwd, MAX);
          printf("server %d: ready for next request ....\n", me);

          n = read(cfd, line, MAX);
          if (n==0){
            printf("server: client gone, server %d loops\n", me);
            close(cfd);
            break;
          }

          // show the line string
          printf("server %d: read  n=%d bytes; recv=[%s]\n", me, n, line);

          strcpy(in, line);
          tokenize();

          if(!strcmp(name[0], "pwd")){
            getcwd(cwd, MAX);
            n = write(cfd, cwd, MAX);
          }
          else if (!strcmp(name[0], "cd")){
            int r = chdir(name[1]);
            getcwd(cwd, MAX);
            n = write(cfd, cwd, MAX);
          }
          else if (!strcmp(name[0], "ls")){
              dp = opendir(cwd);
              while (ep = readdir(dp)){
                ls_file(ep->d_name);
                n = write(cfd, line, MAX);
              }
              n = write(cfd, "END ls\n",MAX);
          }
          else if (!strcmp(name[0], "cp")){
            int fd_src, t = 0, bytes;
            char block[MAX];
            struct stat st;
            stat(name[1], &st);
            sprintf(block, "%ld", st.st_size);
            bytes = write(cfd, block, MAX);
            fd_src = open(name[1], O_RDONLY);
            while(t < st.st_size){
              read(fd_src, block, MAX);
              bytes = write(cfd, block, MAX);
              t += bytes;
            }
            close(fd_src);
          }
          else{
            strcat(line, "ECHO");
            n = write(cfd, line, MAX);
          }
          //n = write(cfd, line, MAX);
          // strcat(line, " ECHO");
          // // send the echo line to client 
          //n = write(cfd, line, MAX);

          printf("server %d: wrote n=%d bytes; send=[%s]\n", me, n, line);
        }
       }
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
    char temp[MAX];
    strcpy(line, "");
    sp = &fstat;
    if ( (r = lstat(fname, &fstat)) < 0){
        printf("can’t stat %s\n", fname);
        exit(1);
    }
    if ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
        strcat(line, "-");  //printf("%c",'-');
    if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
        strcat(line, "d");  //printf("%c",'d');
    if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
        strcat(line, "l");  //printf("%c",'l');
    for (i=8; i >= 0; i--){
        if (sp->st_mode & (1 << i)){ // print r|w|x
            temp[0] = t1[i]; temp[1] = '\0';
            strcat(line, temp);  //printf("%c", t1[i]);
        }
        else {
            temp[0] = t2[i]; temp[1] = '\0';
            strcat(line, temp); // or print -
        }
        }
        sprintf(temp,"%4d ",sp->st_nlink); // link count
        strcat(line, temp);
        sprintf(temp,"%4d ",sp->st_gid); // gid
        strcat(line, temp);
        sprintf(temp,"%4d ",sp->st_uid); // uid
        strcat(line, temp);
        sprintf(temp,"%8d ",sp->st_size); // file size
        strcat(line, temp);
        // print time
        strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form
        ftime[strlen(ftime)-1] = 0; // kill \n at end
        strcat(line,ftime);  //printf("%s ",ftime);
        // print name
        strcat(line, " "); strcat(line, basename(fname));  //printf("%s", basename(fname)); // print file basename
        // print -> linkname if symbolic file
        // if ((sp->st_mode & 0xF000)== 0xA000){
        // // use readlink() to read linkname
        // printf(" -> %s", linkname); // print linked name
    // }
    sprintf(temp,"\n");
    strcat(line,temp);
}      // 