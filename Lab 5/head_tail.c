int head_file(){

    int fd = open_file("0");
    char buf[BLKSIZE];
    int count = 0, i = 0;

    myread(fd, buf, BLKSIZE);

    while(i < BLKSIZE){
        if(buf[i] == '\n')
            ++count;
        if(count >= 10)
            break;
        ++i;
    }

    buf[i + 1] = 0;

    printf("%s\n", buf);

    close_file(fd);

    if(buf) return 1;
    else return 0;
    
}

int tail_file(){

    int fd = open_file("0");
    int p = running->fd[fd]->inodeptr->INODE.i_size;
    int count = 0, i = 0;
    char buf[BLKSIZE];
    char* cp;


    if (p > BLKSIZE)
        mylseek(fd, p - BLKSIZE);

    int n = myread(fd, buf, BLKSIZE);
    buf[n] = 0;
    cp = &buf[n];

    while(cp != buf){
        if(*cp == '\n')
            ++count;
        if(count >= 11)
            break;

        --cp;
    }

    if(cp != buf)
        ++cp;

    printf("%s\n", cp);

    close_file(fd);
}