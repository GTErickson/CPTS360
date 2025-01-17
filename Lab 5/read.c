int myread(int fd, char *buf, int nbytes){
    int lbk, blk, startByte;
    int count = 0;
    char readbuf[BLKSIZE];
    int dbuf[256];
    int dblk, pbytes;

    MINODE* mip = running->fd[fd]->inodeptr;
    int avail = mip->INODE.i_size - running->fd[fd]->offset; 

    char *cq = buf; 

    while(nbytes && avail){
        lbk = running->fd[fd]->offset / BLKSIZE;
        startByte = running->fd[fd]->offset % BLKSIZE;

        if (lbk < 12){ //Direct Blocks
            blk = mip->INODE.i_block[lbk];
        }
        else if(lbk >= 12 && lbk < 256 + 12){ //Indirect Blocks
            get_block(mip->dev, mip->INODE.i_block[12], dbuf);
            blk = dbuf[lbk - 12];
        }
        else{ //Double indirect Blocks
            get_block(mip->dev, mip->INODE.i_block[13], dbuf);
            lbk -= (12 + 256);
            dblk = dbuf[lbk / 256];
            get_block(mip->dev, dblk, dbuf);
            blk = dbuf[lbk % 256];
        }

        
        get_block(mip->dev, blk, readbuf);

        /* copy from startByte to buf[ ], at most remain bytes in this block */
        char *cp =  readbuf + startByte;   
        int remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

    //     while (remain > 0){
    //         *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
    //          running->fd[fd]->offset++;           // advance offset 
    //          count++;                  // inc count as number of bytes read
    //          avail--; nbytes--;  remain--;
    //          if (nbytes <= 0 || avail <= 0) 
    //              break;
    //    }
        if (nbytes > avail)
            pbytes = avail;
        else if (nbytes > remain)
            pbytes = remain;
        else
            pbytes = nbytes;

        strncpy(buf, cp, pbytes);
        nbytes -= pbytes;
        remain -= pbytes;
        running->fd[fd]->offset += pbytes;
        count += pbytes;
        avail -= pbytes;
    }
    //printf("myread: read %d char from file descriptor %d\n", count, fd);  
    //printf("%s\n", buf);
    return count;   // count is the actual number of bytes read

}

int read_file(){
    int fd = atoi(pathname);
    int nb = atoi(parameter);
    char buf[BLKSIZE];

    if (fd > -1 && fd < 8 && (running->fd[fd]->mode == 0 || running->fd[fd]->mode == 2)){
        int n = myread(fd, buf, nb);
        printf("%s\n", buf);
        return n;
    }
}