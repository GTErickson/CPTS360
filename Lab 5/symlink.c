int symlink(char* to_link)
{

    //get pathname's INODE into memory
    //assume oldNAME has <= 60 chars
    //printf("parent: %s\n", pathname);
    MINODE* mip = path2inode(pathname);

    //verify oldNAME exists
    if(!((mip->INODE.i_mode & 0xF000) == 0x4000) && !((mip->INODE.i_mode & 0xF000) == 0x8000)){
        printf("Error: oldNAME is not a DIR or REG\n");
        iput(mip);
        return -1;
    }    


    char * arg1 = basename(pathname);
    char * parent = dirname(pathname);
    printf("parent: %s\n", parent);
    printf("child: %s\n", arg1);
    printf("newfile: %s\n", to_link);

    MINODE *pip = path2inode(parent);

   int ino =  my_creat(pip, to_link); //create file /x/y/z

    MINODE *new_mip = iget(dev, ino);

    new_mip->INODE.i_mode = 0xA1A4; //change /x/y/z's type to LNK

    DIR* dp;
    char buf[BLKSIZE];
    int num = 0;
     for(int i = 0; i < 12; ++i){
        if (pip->INODE.i_block[i] == 0){
            num++;break;
        }
     }
    

    get_block(dev, new_mip->INODE.i_block[2], buf);
    dp = (DIR*)buf;
    char *cp; 

    cp = buf;
   
     strncpy(dp->name, arg1, strlen(arg1)); //store old file name
    //printf("arg1: %s", dp->name);
    new_mip->INODE.i_size = strlen(arg1); //set file size to length of oldNAME
    new_mip->modified = 1; //mark dirty
    put_block(dev, new_mip->INODE.i_block[2], buf);
    iput(new_mip); //write back

    printf("Check write back: \n");
    get_block(dev, new_mip->INODE.i_block[2], buf);
    dp = (DIR*)buf;
    printf("dp->name: %s", dp->name);


    link_ino = new_mip->ino;

    pip->modified = 1;
    iput(pip);


}