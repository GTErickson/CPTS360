// cd_ls_pwd.c file

char* readlink()
{
     //MINODE* mip = path2inode(pathname);
     MINODE *mip = iget(dev, link_ino);


    //verify it is a symLNK file
    if(!(mip->INODE.i_mode & 0xF000) == 0xA000){
        printf("Readlink failed: not a symLNK file\n");
        iput(mip);
        return -1;
    } 

    DIR* dp;
    char buf[BLKSIZE];
    
    get_block(dev, mip->INODE.i_block[2], buf);
    dp = (DIR*)buf;

    //printf("dp->name: %s\n", dp->name);

    return dp->name;

}

int cd()
{
  MINODE *mip = path2inode(pathname);

  if(mip == 0 || !((mip->INODE.i_mode & 0xF000) == 0x4000))
  {
    if (mip){
      printf("%s is not a directory\n", pathname);
    }
    else{
      printf("Directory %s not found\n", pathname);
    }
    return;
  }

  printf("cd to [dev, ino] = [%d, %d]\n", mip->dev, mip->ino);

  //verify mip not null and that it is a DIR
  if (((mip->INODE.i_mode >> 14) == 1) && mip != 0)
  {
    iput(running->cwd);
    running->cwd = mip;
  }

  printf("after cd: cwd = [%d, %d]\n", mip->dev, mip->ino);
}

int ls_file(MINODE *mip, char *name)
{
  char ftime[256];
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";

   if ((mip->INODE.i_mode & 0xF000) == 0x8000) // if (S_ISREG())
        printf("%c",'-');
    if ((mip->INODE.i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
        printf("%c",'d');
    if ((mip->INODE.i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
        printf("%c",'l');
    for (int i=8; i >= 0; i--){
        if (mip->INODE.i_mode & (1 << i)) // print r|w|x
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]); // or print -
        }
        printf("%4d ",mip->INODE.i_links_count); // link count
        printf("%4d ",mip->INODE.i_gid); // gids
        printf("%4d ",mip->INODE.i_uid); // uid
        printf("%8d ",mip->INODE.i_size); // file size
    //     // print time
        strcpy(ftime, ctime(&mip->INODE.i_mtime)); // print time in calendar form
        ftime[strlen(ftime)-1] = 0; // kill \n at end
        printf("%s ",ftime);
    //     // print name
        printf("%8s", basename(name)); // print file basename
        // print -> linkname if symbolic file
         if ((mip->INODE.i_mode & 0xF000)== 0xA000){
         // use readlink() to read linkname
         printf(" -> %s", readlink()); // print linked name
         }
    printf(" [%d %d]", mip->dev, mip->ino);

    printf("\n");
}
  
int ls_dir(MINODE *pip)
{
  char sbuf[BLKSIZE], temp[256];
  DIR  *dp;
  char *cp; 
  int ino = 0;
  printf("i_block[0] = %d\n", pip->INODE.i_block[0]);
  get_block(dev, pip->INODE.i_block[0], sbuf);
  dp = (DIR *)sbuf;
  cp = sbuf;
  while (cp < sbuf + BLKSIZE){
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;
    MINODE *mip = iget(dev, dp->inode);
    //printf("%d\n", mip->ino);
    
      ls_file(mip, temp);
    
    
    iput(mip);

    cp += dp->rec_len;
    dp = (DIR*)cp;
  }

}

int ls()
{
    MINODE *mip;

    if (pathname[0] == 0){
        mip = running->cwd;
    }
    else{
    //printf("test\n");
        mip = path2inode(pathname); // check mip not null !!
    }
    
    if ((mip->INODE.i_mode >> 14) == 1){
        ls_dir(mip);
    }
    else
        ls_file(mip, basename(pathname));

    iput(mip);
}


rpwd(MINODE* wd){
    int ino = 0;
    int pino = 0;
    char myname[256];
    if (wd == root)
        return;

    pino = findino(wd, &ino);
    MINODE* pip = iget(dev, pino);
    findmyname(pip, ino, myname);
    rpwd(pip);
    iput(pip);
    printf("/%s", myname);


  
}

int pwd()
{
  MINODE *wd = running->cwd;
  if (wd == root)
    printf("/");
  else
    rpwd(wd);

  putchar('\n');
}