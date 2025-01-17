/*********** A Multitasking System ************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "wait.c"
#include "queue.c"

// global variables
PROC proc[NPROC];   // 9 PROCs
PROC *freeList;     // FREE proc list
PROC *readyQueue;   // priority queue of READY procs
PROC *sleepList;    // list of SLEEP procs
PROC *running;      // pointer to current RUNNING proc

int body()
{
  char command[64];
  int event = -1;
  while(1){
    printf("***************************************\n");

    printf("P%d running: Parent=%d ", running->pid, running->ppid);
    printChildList(running->child);
    
    printFreeList(freeList);
    printReadyQueue(readyQueue);
    printSleepList(sleepList);
    
    printf("input a command: [ps|fork|switch] | [sleep|wakeup|exit|wait] : ");

    fgets(command, 64, stdin);
    command[strlen(command)-1] = 0; // kill \n at end
    
    if (!strcmp(command, "ps"))
      ps();
    else if (!strcmp(command, "fork"))
      kfork(body, 1);
    else if (!strcmp(command, "switch"))
      tswitch();
    else if (!strcmp(command, "sleep")){
      printf("Please input event: ");
      if (running->pid != 1){
        scanf("%d", &event);
        getchar();
        ksleep(event);
      }
      else
      printf("P1 doesn't sleep\n");
    }
    else if (!strcmp(command, "wakeup")){
      printf("Please input event: ");
      scanf("%d", &event);
      getchar();
      kwakeup(event);
    }
    else if (!strcmp(command, "exit")){
      printf("Please input exitCode: ");
      scanf("%d", &event);
      getchar();
      kexit(event);
    }
    else if (!strcmp(command, "wait")){
      kwait(&running->pid);
    }
    else
      printf("invalid command\n");
  }
}

int kfork(int (*func), int priority)
{
  PROC *p;
  int  i;
  /*** get a proc from freeList for child proc: ***/
  p = dequeue(&freeList);
  if (!p){
    printf("no more proc\n");
    return -1;
  }


  /* initialize the new proc and its stack */
  p->status = READY;
  p->priority = priority;     // for ALL PROCs except P0
  p->ppid = running->pid;
  p->parent = running;
  p->child = 0; p->sibling = 0;

  //Add p to child list of parent
  if (running->child){
    PROC *q = running->child;
    PROC *prev;
    while(q->sibling){
      prev = q;
      q = q->sibling;
    }
    q->sibling = p;
  }
  else{
    running->child = p;
  }
  
  //                    -1   -2  -3  -4  -5  -6  -7  -8   -9
  // kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag|
  for (i=1; i<10; i++)
    p->kstack[SSIZE - i] = 0;

  p->kstack[SSIZE-1] = (int)func;
  p->saved_sp = &(p->kstack[SSIZE - 9]); 

  enqueue(&readyQueue, p);
  printf("P%d forked a child P%d\n", running->pid, p->pid);
  
  return p->pid;
}

int init()
{
  int i;
  PROC *p;
  
  // 1. all PROCs in freeList
  for (i = 0; i < NPROC; i++){
    p = &proc[i];
    p->pid = i; 
    p->status = FREE;
    p->priority = 0;
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;

  freeList = &proc[0];        
  readyQueue = 0;

  // 2. create P0 as the initial running process
  running = dequeue(&freeList);
  running->status = READY;
  running->priority = 0;     // P0 has lowest prioriy 0
  running->parent = running;
  running->child = 0; running->sibling = 0;
  
  printFreeList(freeList);
  printf("init complete: P0 running\n"); 
}


/*************** main() ***************/
int main()
{
  printf("Welcome to 360 Multitasking System\n");
  init();
  printf("P0 fork P1\n");
  kfork(body, 1);  

  while(1){
    if (readyQueue){
        printf("P0: switch task\n");
        tswitch();
    }
  }
}

/*********** scheduler *************/
int scheduler()
{ 
  printf("proc %d in scheduler()\n", running->pid);
  if (running->status == READY)
      enqueue(&readyQueue, running);
  printReadyQueue(readyQueue);
  running = dequeue(&readyQueue);
  printf("next running = %d\n", running->pid);  
}


char *pstatus[]={"FREE   ","READY  ","SLEEP  ","BLOCK  ","ZOMBIE", "RUNNING"};

int ps()
{
  int i; PROC *p;
  printf("pid   ppid    status\n");
  printf("--------------------\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    printf(" %d      %d     ", p->pid, p->ppid);
    if (p==running)
      printf("%s\n", pstatus[5]);
    else
      printf("%s\n", pstatus[p->status]);
  }
}