/********** wait.c file **********/

extern PROC *running;
extern PROC *sleepList;
extern PROC *readyQueue;
extern PROC *freeList;

/********* Required PRE-work ***********/

// 1. Implement ksleep() per the algorithm in 3.4.1
int ksleep(int event)
{
       running->event = event;
       running->status = SLEEP;
       enqueue(&sleepList, running);
       tswitch();
}

// 2. Implement kwakeup per the algorithm in 3.4.2
int kwakeup(int event)
{
       PROC *prev = 0;
       PROC *q = sleepList;

       while(q){
              if (q->event == event){
                     //if first element in the list
                     if (!prev){
                            sleepList = q->next;
                            q->status = READY;
                            enqueue(&readyQueue, q);
                            //free(q);
                            q = sleepList;
                     }
                     //Not the first element in the list
                     else {
                            prev->next = q->next;
                            q->status = READY;
                            enqueue(&readyQueue, q);
                            //free(q);
                            q = prev->next;
                     }
              }
              else{
                     prev = q;
                     q = q->next;
              }
       }
}

// 4. Implement kexit per the algorithm 3.5.1

int kexit(int exitValue)
{
  // ensure P1 never exit !!!!
  if(running->pid > 1){
       //get P1
       PROC *q = running->parent;
       PROC *p;
       PROC *prev;
       while(q->pid != 1){
              q = q->parent;
       }
       //Add children of running to P1
       p = q->child;
       while(p->sibling){
              prev = p;
              p = p->sibling;
       }
       p->sibling = running->child;
       //Set all moved childs parent PROC attribute
       p = q->child;
       while(p){
              p->parent = q;
              p->ppid = q->pid;
              p = p->sibling;
       }
       running->status = ZOMBIE;
       running->exitCode = exitValue;
       running->child = 0;
       if(running->parent->status == SLEEP){
              kwakeup(running->parent->event);
       }
       //printf("\n\n\n\n%d\n\n\n\n", q->pid);
       tswitch();
  }
  else{
       printf("Can't exit on P1");
  }
}


// 5. Implement kwait() per the algorithm in 3.5.3
int kwait(int *status)
{
       PROC *p = running->child;
       PROC *prev = 0;
       if (!p)
              return -1;
       else{
              while(p){
                     if(p->status == ZOMBIE){
                            if (!prev)
                                   running->child = p->sibling;
                            else
                                   prev->sibling = p->sibling;
                            p->status = FREE;
                            p->priority = 0;
                            enqueue(&freeList, p);
                            return p->pid;
                     }
                     prev = p;
                     p = p->sibling;
              }
              ksleep(status);
       }



       //return pid
}