int mywrite(int fd, char buf[ ], int nbytes) 
{
    OFT* oftp = running->fd[fd];
    MINODE* mip = running->fd[fd]->inodeptr;

    char *cq = buf;
    
    while (nbytes > 0 ){

        int lbk = oftp->offset / BLKSIZE;
        int startByte = oftp->offset % BLKSIZE;
        int blk, dblk;
        int pbytes;

    // I only show how to write DIRECT data blocks, you figure out how to 
    // write indirect and double-indirect blocks.

    if (lbk < 12){                         // direct block
        if (mip->INODE.i_block[lbk] == 0){   // if no data block yet

        mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
        }
        blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
    }


    else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
        // HELP INFO:
        // if (i_block[12] == 0){
        //     allocate a block for it;
        //     zero out the block on disk !!!!
        // }
        // get i_block[12] into an int ibuf[256];
        // blk = ibuf[lbk - 12];
        // if (blk==0){
        //     allocate a disk block;
        //     record it in ibuf[lbk - 12];
        // }
        // write ibuf[ ] back to disk block i_block[12];
        if (mip->INODE.i_block[12] == 0){
            mip->INODE.i_block[12] = balloc(mip->dev);
            char obuf[BLKSIZE] = {'0'};
            put_block(mip->dev, mip->INODE.i_block[12], obuf);
        }
        int ibuf[256];
        get_block(mip->dev, mip->INODE.i_block[12], ibuf);
        blk = ibuf[lbk - 12];
        if(blk == 0){
            blk = balloc(mip->dev);
            ibuf[lbk - 12] = blk;            
   
        }
        put_block(mip->dev, mip->INODE.i_block[12], ibuf);
    }
    else{
            // double indirect blocks */
            if (mip->INODE.i_block[13] == 0){
                mip->INODE.i_block[13] = balloc(mip->dev);
                char obuf[BLKSIZE] = {'0'};
                put_block(mip->dev, mip->INODE.i_block[13], obuf);
            }
            int ibuf[256];
            get_block(mip->dev, mip->INODE.i_block[13], ibuf);
            lbk -= (256+12);
            dblk = ibuf[lbk / 256];
            if(dblk == 0){
                dblk = balloc(mip->dev);
                ibuf[lbk - 12] = dblk; 
                //put_block(mip->dev, mip->INODE.i_block[13], ibuf);
            }
            get_block(mip->dev, dblk, ibuf);
            blk = ibuf[lbk % 256];
            if(blk == 0){
                blk = balloc(mip->dev);
                ibuf[lbk - 12] = blk; 
            }
            put_block(mip->dev, dblk, ibuf);
            put_block(mip->dev, mip->INODE.i_block[13], ibuf);
        }

        /* all cases come to here : write to the data block */
        char wbuf[BLKSIZE];
        get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
        char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
        int remain = BLKSIZE - startByte;     // number of BYTEs remain in this block
        // while (remain > 0){               // write as much as remain allows  
        //     *cp++ = *cq++;              // cq points at buf[ ]
        //     nbytes--; remain--;         // dec counts
        //     oftp->offset++;             // advance offset
        //     if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
        //         mip->INODE.i_size++;    // inc file size (if offset > fileSize)
        //     if (nbytes <= 0) break;     // if already nbytes, break
        // }
        if (nbytes > remain)
            pbytes = remain;
        else
            pbytes = nbytes;
        
        strncpy(cp, buf, pbytes);
        nbytes -= pbytes;
        oftp->offset += pbytes;
        if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
            mip->INODE.i_size += (oftp->offset - mip->INODE.i_size);

        put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
        
        // loop back to outer while to write more .... until nbytes are written
    }
    //mip->dirty = 1;       // mark mip dirty for iput() 
    //printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
    return nbytes;
}


int write_file()
{
    int fd = atoi(pathname);
    //Parameter is the text string for write

    if(running->fd[fd]->mode == 0){
        printf("Error: fd is in read\n");
        return -1;
    }

    char buf[1024];
    strncpy(buf, parameter, strlen(parameter));

    return mywrite(fd, buf, strlen(parameter));
}