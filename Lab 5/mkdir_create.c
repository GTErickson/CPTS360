// mkdir_create.c

int enter_child(MINODE *pip, int myino, char* myname){
    int NEED_LEN = 4*((8 + strlen(myname) + 3) / 4), IDEAL_LEN, REMAIN;
    char buf[BLKSIZE];
    DIR* dp;
    char* cp;
    for(int i = 0; i < 12; ++i){
        if (pip->INODE.i_block[i] == 0){
            break;
        }
        get_block(dev, pip->INODE.i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;
        while(dp){
            IDEAL_LEN = 4*((8 + dp->name_len + 3)/4);
            REMAIN = dp->rec_len - IDEAL_LEN;
            if (REMAIN >= NEED_LEN){      // found enough space for new entry
                dp->rec_len = IDEAL_LEN;   // trim dp's rec_len to its IDEAL_LEN
                //Enter Child
                cp += dp->rec_len;
                dp = (DIR*)cp;
                dp->inode = myino;
                dp->rec_len = REMAIN;
                dp->name_len = strlen(myname);
                strcpy(dp->name, myname);
                //dp->name[strlen(dp->name) - 1] = 0;
                put_block(dev, pip->INODE.i_block[i], buf);
                return 1;
            }

            cp += dp->rec_len;
            dp = (DIR*)cp;
        }   
    }
}

int mymkdir(MINODE *pip, char *name){

    int ino = ialloc(dev);
    int bno = balloc(dev);

    MINODE *mip = iget(dev, ino);
    INODE *ip = &(mip->INODE);

    ip->i_mode = 0x41ED;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = BLKSIZE;
    ip->i_links_count = 2;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2;
    ip->i_block[0] = bno;
    
    for(int i = 1; i < 15; ++i){
        ip->i_block[i] = 0;
    }

    mip->modified = 1;
    iput(mip);

    //Make data block 0 of Inode to contain . and ..
    char buf[BLKSIZE];

    DIR* dp = (DIR*)buf;
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    strcpy(dp->name, ".");
    
    dp = (DIR*)(buf + 12);
    dp->inode = pip->ino;
    dp->rec_len = BLKSIZE - 12;
    dp->name_len = 2;
    strcpy(dp->name, "..");

    put_block(dev, bno, buf);

    //Enter child
    enter_child(pip, ino, name);

}

int make_dir(){

    char * child = basename(pathname);
    char * parent = dirname(pathname);
    printf("%s\n", parent);
    printf("%s\n", child);

    MINODE *pip = path2inode(parent);

    if(!pip){ //Verify parent is found
        printf("Error! Could not find parent\n");
        return -1;
    }
    if(!((pip->INODE.i_mode & 0xF000) == 0x4000)){ //Verify parent is DIR
        printf("Error! Parent is not a DIR\n");
        return -1;
    }
    if(search(pip, child)){ //Verify child does not already exist in parent
        printf("Error! Child already exists in DIR\n");
        return -1;
    }

    mymkdir(pip, child);

    pip->INODE.i_links_count++;
    pip->INODE.i_atime == time(0L);
    pip->modified = 1;

    iput(pip);
}

creat_file(){
    char * child = basename(pathname);
    char * parent = dirname(pathname);
    printf("%s\n", parent);
    printf("%s\n", child);

    MINODE *pip = path2inode(parent);

    if(!pip){ //Verify parent is found
        printf("Error! Could not find parent\n");
        return -1;
    }
    if(!((pip->INODE.i_mode & 0xF000) == 0x4000)){ //Verify parent is DIR
        printf("Error! Parent is not a DIR\n");
        return -1;
    }
    if(search(pip, child)){ //Verify child does not already exist in parent
        printf("Error! Child already exists in DIR\n");
        return -1;
    }

    my_creat(pip, child);

    pip->INODE.i_links_count++;
    pip->INODE.i_atime == time(0L);
    pip->modified = 1;

    iput(pip);
}

int my_creat(MINODE *pip, char *name)
{
//   Same as mymkdir() except 
//     INODE's file type = 0x81A4 OR 0100644
//     links_count = 1
//     NO data block, so size = 0
//     do NOT inc parent's link count.
    int ino = ialloc(dev);
    int bno = balloc(dev);

    MINODE *mip = iget(dev, ino);
    INODE *ip = &(mip->INODE);

    ip->i_mode = 0x81A4;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = 0;
    ip->i_links_count = 1;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);

    mip->modified = 1;
    iput(mip);

    //Enter child
    enter_child(pip, ino, name);

    return ino;
}