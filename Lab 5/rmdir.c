int rm_child(MINODE *pip, char *name){
    //Search for entry
    char buf[BLKSIZE];
    DIR* dp, *prev;
    char* cp, temp[256];
    for(int i = 0; i < 12; ++i){
        if (pip->INODE.i_block[i] == 0)
            continue;
        else{
            get_block(dev, pip->INODE.i_block[i], buf);
            dp = (DIR*)buf;
            cp = buf;
            while(cp < buf + BLKSIZE){
                strcpy(temp, name);
                temp[dp->name_len] = 0;
                //rintf("|%s|  %d  %d |%s|  ", temp, strncmp(temp, dp->name, (dp->name_len)), dp->name_len, dp->name);

                if (!strncmp(temp, dp->name, (dp->name_len))){ 
                    printf("rmdir: found DIR\n");    // found DIR

                    
                    if(prev){
                        prev->rec_len += dp->rec_len;
                    }
                    else{
                        //if only entry
                        // if((prev == 0) && (next == 0))
                        // {
                        //     dp->inode = 0;
                        //     dp->name_len = dp->name_len - BLKSIZE;
                            
                        //     //move parents nonzero blocks upward
                        //     int i = 0;
                        //     while(pip->INODE.i_block[i + 1])
                        //     {
                        //         pip->INODE.i_block[i + 1] = pip->INODE.i_block[i];
                        //     }

                        // }
                        // else
                        // {
                        //     //no trailing entries
                        //     dp->inode = 0;
                        //     dp->name_len = 0;
                        // }
                        
                        

                        dp->inode = 0;
                        dp->name_len = 0;

                        
                    }

                    put_block(dev, pip->INODE.i_block[i], buf);
                    return 1;
                }
                    prev = dp;
                    cp += dp->rec_len;
                    dp = (DIR*)cp;
            }
        }
    }
}

int rmdir(){
    if((!strcmp(pathname, ".")) | (!strcmp(pathname, "..")) | (!strcmp(pathname, "/"))){
        printf("Error: can't rmdir .. or . or /\n");
        return -1;
    }
    
    MINODE *mip = path2inode(pathname);
    
    if (!mip){
        printf("DIR not found\n");
        return -1;
    }
    if(!(mip->INODE.i_mode & 0xF000) == 0x4000){
        printf("Error: not a DIR\n");
        iput(mip);
        return -1;
    }
    if(mip->INODE.i_links_count > 2){
        printf("Error: DIR not empty\n");
        iput(mip);
        return -1;
    }
    if(mip->shareCount > 1){
        printf("Error: DIR is busy\n");
        iput(mip);
        return -1;
    }
    
    int pino = findino(mip, &(mip->ino));
    

    for (int i=0; i<12; i++){
        if (mip->INODE.i_block[i]==0)
            continue;
        else{
            bdalloc(mip->dev, mip->INODE.i_block[i]);
        }
    }
    idalloc(mip->dev, mip->ino);
    iput(mip);

    MINODE *pip = iget(dev, pino);
    char* temp = basename(pathname);
    rm_child(pip, temp);

    pip->INODE.i_links_count--;
    pip->INODE.i_atime = pip->INODE.i_mtime = time(0L);
    pip->modified = 1;
    iput(pip);
    return 1;
}