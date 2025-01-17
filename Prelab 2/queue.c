char *status[] = {"FREE","READY","SLEEP","BLOCK","ZOMBIE", "RUNNING"};

int enqueue(PROC **queue, PROC *p)  // NOTE **queue
{
    // enter p into queue by priority; FIFO if same priority
    PROC *q = *queue;
    if (q == 0 || p->priority > q->priority){
        *queue = p;
        p->next = q;
    }
    else{
        while(q->next && p->priority <= q->next->priority)
            q = q->next;
        p->next = q->next;
        q->next = p;
    }
}

// remove and return first PROC from queue 
PROC *dequeue(PROC **queue)         // NOTE **queue 
{
    // remove and return FIRST element in queue
    PROC *q = *queue;

    if(q)
        *queue = q->next;

    return(q);

}

int printFreeList(PROC *p)  // print list p
{
    // print the list as  freeList=[pid pri]->[pid pri] ->..-> NULL 
    printf("freeList=");
    while(p){
        printf("[%d %d]->", p->pid, p->priority);
        p = p->next;
    }
    printf("NULL\n");
}

int printReadyQueue(PROC *p)  // print list p
{
    // print the list as  readyQueue=[pid pri]->[pid pri] ->..-> NULL 
    printf("readyQueue=");
    while(p){
        printf("[%d %d]->", p->pid, p->priority);
        p = p->next;
    }
    printf("NULL\n");
}

int printSleepList(PROC *p)  // print list p
{
    // print the list as  sleepList=[pid event]->[pid event] ->..-> NULL 
    printf("sleepList=");
    while(p){
        printf("[%d 0x%x]->", p->pid, p->event);
        p = p->next;
    }
    printf("NULL\n");
}

int printChildList(PROC *p)  // print list p
{
    // print the list as  childList=[pid status]->[pid status] ->..-> NULL 
    printf("childList=");
    while(p){
        printf("[%d %s]->", p->pid, *(status + p->status));
        p = p->sibling;
    }
    printf("NULL\n");
}