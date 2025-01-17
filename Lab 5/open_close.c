int open_file(char* parameter)
{
    int mode = -1;
    if (strncmp(parameter, "0", 1) == 0)
        mode = 0;
    else if (strncmp(parameter, "1", 1) == 0)
        mode = 1;
    else if (strncmp(parameter, "2", 1) == 0)
        mode = 2;
    else if (strncmp(parameter, "3", 1) == 0)
        mode = 3;
    else{
        printf("Error: Invalid mode for open\n");
        return -1;
    }

    if(!(mode > -1 && mode < 4)){ //Check mode
        printf("Error: Invalid mode for open\n");
        exit(-1);
    }

    MINODE *mip = path2inode(pathname); //Get pathname ino

    if (!mip){ // pathname does not exist:
        if (mode==0)    // READ: file must exist
	        return -1;
		    
        // mode=R|RW|APPEND: creat file first
        creat_file();  // make sure YOUR creat_file() use pathname 

	    mip = path2inode(pathname);
	    
        printf("[%d %d]", mip->dev, mip->ino); // print mip's [dev, ino]
    }
    
    //Make checks
    if ((mip->INODE.i_mode & 0xF000) != 0x8000){
        printf("Error: Open -- not reg file");
        return -1;
    }

    for(int i = 0; running->fd[i]; i++){
        if (running->fd[i]->inodeptr == mip){
            if (mode != running->fd[i]->mode){
                printf("Opened for incompatible modes\n");
                return -1;
            }
        }
    }


    OFT* oftp = &oft[count];
    oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 

    oftp->shareCount = 1;

    oftp->inodeptr = mip;  // point at the file's minode[]
    switch(mode){
        case 0 : oftp->offset = 0;     // R: offset = 0
                break;
        case 1 : truncate(mip->INODE);        // W: truncate file to 0 size
                oftp->offset = 0;
                break;
        case 2 : oftp->offset = 0;     // RW: do NOT truncate file
                break;
        case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
                break;
        default: printf("invalid mode\n");
                  return(-1);
    }

    int fd;
    for(fd = 0; running->fd[fd]; ++fd);

    running->fd[fd] = oftp;

    if (mode != 0) {
        mip->INODE.i_mtime = time(0L);
    }
    mip->INODE.i_atime = time(0L);

    mip->modified = 1;
    ++count;
    return fd;
}

int pfd(){
    // if(!running->fd[0]){
    //     printf("No opened file\n");
    //     return -1;
    // }
    char buf[BLKSIZE];
    int count = 0;
    for(int i = 0; i < 8; ++i){
        if (running->fd[i])
            ++count;
    }
    if(count){
        printf("fd    mode    offset    INODE\n");
        printf("---   -----  -------    ------\n");
        for(int i = 0; i < 8; ++i){
            if (running->fd[i])
                printf("%3d   %5d  %7d    [%d, %d]\n", i, running->fd[i]->mode, running->fd[i]->offset, running->fd[i]->inodeptr->dev, running->fd[i]->inodeptr->ino);
        }
    }
    else{
        printf("No opened files\n");
    }
}

int mylseek(int fd, int position){
    int temp = running->fd[fd]->offset;
    if (position < running->fd[fd]->inodeptr->INODE.i_size){
        running->fd[fd]->offset = position;
        //printf("In lseek\n");
    }
    return temp;
}

int close_file(int fd){

    if(fd > 7 || fd < 0){
        printf("Error: fd out of range\n");
    }
    if(!running->fd[fd]){
        printf("Error: oft not found\n");
        return -1;
    }

    OFT* oftp = running->fd[fd];
    running->fd[fd] = 0;

    oftp->shareCount--;
    if (oftp->shareCount > 0)
        return 0;

    // last user of this OFT entry ==> dispose of its minode
    MINODE* mip = oftp->inodeptr;
    iput(mip);

    return 0; 
}

int dup(int fd){ 
    if(!running->fd[fd]) //verify fd is an opened file descriptor 
        { return -1; } 
    int i; for(i = 0; 
    running->fd[i]; ++i); //get first open fd slot 
    running->fd[i] = running->fd[fd]; //duplicate 
    
    //increment OFT sharecount 
    running->fd[i]->shareCount++; 
    running->fd[fd]->shareCount++; 
} int 

dup2(int fd, int gd) { 
    //close gd if already opened 
    if(running->fd[gd]) { close(gd); } 
    running->fd[gd] = running->fd[fd]; //duplicate fd into gd 
    
    //increment OFT sharecount 
    running->fd[gd]->shareCount++; 
    running->fd[fd]->shareCount++; 
}