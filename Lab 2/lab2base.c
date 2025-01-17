/***** LAB2 base code *****/ 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

char gpath[128];    // hold token strings  
char *name[64];     // token string pointers
int  n;             // number of token strings

char dpath[128];    // hold dir strings in PATH
char *dir[64];      // dir string pointers
int  ndir;          // number of dirs   

int tokenize(char *pathname) 
{
  char *s;
  strcpy(gpath, pathname);   // copy into global gpath[]
  s = strtok(gpath, " ");    
  n = 0;
  while(s){
    name[n++] = s;           // token string pointers   
    s = strtok(0, " ");
  }
  name[n] = 0;               // name[n] = NULL pointer
}

void runRedirect(){
  for (int i = 0; i<n; ++i)
    {
      if (name[i][0] == '<'){ //stdin
        close(0);
        int fd=open(name[i+1], O_RDONLY);
        name[i] = 0;
      }
      if (name[i][0] == '>' && name[i][1] == '>'){ //stdout
        close(1);
        int fd=open(name[i+1], O_WRONLY | O_APPEND, 0664);
        name[i] = 0;
      }
      else if (name[i][0] == '>'){ //stdout
        close(1);
        int fd=open(name[i+1], O_WRONLY | O_CREAT, 0664);
        name[i] = 0;
      }
    }
}

void processSingle(){
  //to be finished
}

void processMultiple(){
  //to be finished
}

int main(int argc, char *argv[ ], char *env[ ])
{
  int i;
  int pid, status;
  char *cmd;
  char line[128];
  char *path[64];
  char *home = env[25];
  char *head;
  char *tail;
  int cmdCount = 1;   // number of commands
  char* cmdArr[64];    // array of all commands

  // YOU DO: Write code to print argc, argv and env
  printf("argc=%d\n", argc);
  for(int i = 0; i < argc; ++i){
      printf("argv[%d] = %s ", i, *(argv + i));
  }
  for(int i = 0; *(env + i); ++i){
      printf("\nenv[%d] = %s", i, *(env + i));
  }

  printf("\n********** Welcome to mysh **********\n");
  printf("%s\n%s\n", home, env[51]); //Print home and Path

  //Tokenize Path
  char *s;
  strcpy(gpath, env[51]);   // copy into global gpath[]
  s = strtok(gpath, "=");    
  int z = 0;
  while(s){
    path[z++] = s;           // token string pointers   
    s = strtok(0, ":");
  }
  path[z] = 0;   

  for(int i = 1; path[i]; ++i)
    printf("di[%d] = %s\n", i - 1, path[i]);

  while(1){
    int cmdCount = 1;   // number of commands
    char* cmdArr[64];
    printf("\n********** mysh processing loop **********\n");
    printf("mysh %d running\n", getpid());
    printf("mysh \% : ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;        // kill \n at end 
    if (line[0]==0) // if (strcmp(line, "")==0) // if line is NULL
      continue;
    
    tokenize(line);       // divide line into token strings

    cmdArr[0] = strtok(line, "|"); //Get first command
    while(cmdArr[cmdCount] = strtok(0, "|")) //Get other commands if available
      ++cmdCount; //Keep track of # of commands
    //cmdArr[cmdCount+1] = 0; //Terminate array after all commands

    for(int i = 0; i < cmdCount; ++i){
      printf("cmd[%d] = %s\n", i, cmdArr[i]);
    }

    if (cmdCount == 1) {

      for (i=0; i<n; i++){  // show token strings   
          printf("name[%d] = %s\n", i, name[i]);
      }

      cmd = name[0];         // line = name0 name1 name2 ....

      if (strcmp(cmd, "cd")==0){
        if (name[1])
          chdir(name[1]);
        else
          chdir(home);

        continue;
      }
      if (strcmp(cmd, "exit")==0)
        exit(0); 
      
      pid = fork();
      
      if (pid){
        printf("mysh %d forked a child sh %d\n", getpid(), pid);
        printf("mysh %d wait for child sh %d to terminate\n", getpid(), pid);
        pid = wait(&status);
        printf("\nZOMBIE child=%d exitStatus=%x\n", pid, status); 
        printf("mysh %d repeat loop\n", getpid());
      }
      else{
          printf("child sh %d running\n", getpid());
          runRedirect();

          //Handling simple commands
          for(int i = 1; path[i]; ++i){
            strcpy(line, path[i]); strcat(line, "/"); strcat(line, cmd);
            //printf("i=%d   %s\n", i-1, line);
            
            int r = execve(line, name, env);
          }
          
          // make a cmd line = ./cmd for execve()
          strcpy(line, "./"); strcat(line, cmd);  // line="./cmd"
          printf("line = %s\n", line);
          int r = execve(line, name, env);

          printf("execve failed r = %d\n", r);
          exit(1);
      }
    }
    else{ //multiple commands

        //Iterate for more than 2 commands
        for (int i = 0; i < cmdCount; ++i){
          char line1[128];
          strcpy(line, cmdArr[i]); //Line 1 = to the first command
          char line2[128];
          strcpy(line2, cmdArr[i + 1]); //Line 2 = to the second command

          //tokenize line 1
          tokenize(line1);
          char* arg1 = name; //Get all arguments for command 1
          char* cmd1 = name[0]; //Set command 1

          //tokenize line 2
          tokenize(line2);
          char* arg2 = name; //Get all arguments for command 2
          char* cmd2 = name[0]; //Set command 2

          int pd[2];
          pipe(pd);
          pid = fork();
          if (pid){
            close(pd[0]);
            close(1);
            dup(pd[1]);
            close(pd[1]);
            for(int i = 1; path[i]; ++i){
              strcpy(line1, path[i]); strcat(line1, "/"); strcat(line1, cmd1);
              //printf("i=%d   %s\n", i-1, line);
              
              int r = execve(line1, name, env);
            }
          }
          else{
            close(pd[1]);
            close(0);
            dup(pd[0]);
            close(pd[0]);
            for(int i = 1; path[i]; ++i){
              strcpy(line2, path[i]); strcat(line2, "/"); strcat(line2, cmd2);
              //printf("i=%d   %s\n", i-1, line);
              
              int r = execve(line2, name, env);
            }
          }

        }


      // int pd[2];
      // pipe(pd);
      // pid = fork();
      // if (pid){
      //   close(pd[0]);
      //   close(1);
      //   dup(pd[1]);
      //   close(pd[1]);
      //  int r = execve(cmdArr[0], name, env);
      // }
      // else{
      //   close(pd[1]);
      //   close(0);
      //   dup(pd[0]);
      //   close(pd[0]);
      //   int r = execve(cmdArr[1], name, env);
      // }
      
    }
  }
}

/********************* YOU DO ***********************
1. I/O redirections:

Example: line = arg0 arg1 ... > argn-1

  check each arg[i]:
  if arg[i] = ">" {
     arg[i] = 0; // null terminated arg[ ] array 
     // do output redirection to arg[i+1] as in Page 131 of BOOK
  }
  Then execve() to change image


2. Pipes:

Single pipe   : cmd1 | cmd2 :  Chapter 3.10.3, 3.11.2

Multiple pipes: Chapter 3.11.2
****************************************************/

    
