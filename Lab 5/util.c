/*********** globals in main.c ***********/
extern PROC   proc[NPROC];
extern PROC   *running;

extern MINODE minode[NMINODE];   // minodes
extern MINODE *freeList;         // free minodes list
extern MINODE *cacheList;        // cacheCount minodes list

extern MINODE *root;

extern OFT    oft[NOFT];

extern char gline[256];   // global line hold token strings of pathname
extern char *name[64];    // token string pointers
extern int  num_tok;            // number of token strings                    

extern int ninodes, nblocks;
extern int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers

extern int  fd, dev;
extern char cmd[16], pathname[128], parameter[128];
extern int  requests, hits;

/**************** util.c file **************/

MINODE* dequeue(MINODE** list)
{
  MINODE* p = *list;
  if(p)
  {
    *list = p->next;
  }

  return p;
}

int enqueue(MINODE **queue, MINODE* p)
{
  MINODE* q = *queue;
  if(q == 0 || p->cacheCount > q->cacheCount)
  {
    *queue = p;
    p->next = q;
  }
  else{
    while(q->next && p->cacheCount <= q->next->cacheCount)
    {
      q = q->next;
    }

    p->next = q->next;
    q->next = p;
  }
}

int get_block(int dev, int blk, char buf[ ])
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = read(fd, buf, BLKSIZE);
  return n;
}

int put_block(int dev, int blk, char buf[ ])
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = write(fd, buf, BLKSIZE);
  return n; 
}       

int tokenize(char *pathname)
{
  // tokenize pathname into n token strings in (global) gline[ ]
  char *s;
   strcpy(gline, pathname);   // copy into global gpath[]
   s = strtok(gline, "/");    
   n = 0;
   while(s){
   name[n++] = s;           // token string pointers   
   s = strtok(0, "/");
   }
   name[n] = 0;               // name[n] = NULL pointer

   for(int i = 0; i < n; ++i){
      printf("token[%d] = %s\n", i, name[i]);
   }
}

MINODE *iget(int dev, int ino) // return minode pointer of (dev, ino)
{
    MINODE *mip;
    INODE *ip;
    int i, block, offset;
    char buf[BLKSIZE];
  /********** Write code to implement these ***********
  1. search cacheList for minode=(dev, ino);
  if (found){
     inc minode's cacheCount by 1;
     inc minode's shareCount by 1;
     return minode pointer;
  } */
  mip = cacheList; 
  ++requests;
 
  while(mip){
    if (mip->dev == dev && mip->ino == ino){
      //printf("found minode %d [%d %d] in cache\n", mip->id, dev, ino);
      mip->cacheCount++;
      mip->shareCount++;
      ++hits;
      return mip;
    }
    mip = mip->next;
  }

  /* needed (dev, ino) NOT in cacheList
  2. if (freeList NOT empty){
        remove a minode from freeList;
        set minode to (dev, ino), cacheCount=1 shareCount=1, modified=0;
 
        load INODE of (dev, ino) from disk into minode.INODE;

        enter minode into cacheList; 
        return minode pointer;
     } */

  mip = dequeue(&freeList);
  if (mip){
      mip->dev = dev;  mip->ino = ino;
      mip->cacheCount=1; mip->shareCount=1; mip->modified=0;
      
      // load INODE of (dev, ino) into mip->INODE
      block = (ino-1) / 4 + iblk;
      offset = (ino-1) % 4;
      get_block(dev, block, buf);     // char buf[BLKSIZE];

      ip = (INODE*)buf + (offset*2);
      mip->INODE = *ip;

      enqueue(&cacheList, mip);
      return mip;
  }
  /* freeList empty case:
  3. find a minode in cacheList with shareCount=0, cacheCount=SMALLest
     set minode to (dev, ino), shareCount=1, cacheCount=1, modified=0
     return minode pointer;

 NOTE: in order to do 3:
       it's better to order cacheList by INCREASING cacheCount,
       with smaller cacheCount in front ==> search cacheList
  ************/
  else{
    //order cacheList
    MINODE *p, *temp = 0;
    while(p = dequeue(&cacheList))
    {
      enqueue(&temp, p);
    }
    cacheList = temp;
    //traverse through cacheList
    MINODE* ecp = 0;
    while(cacheList)
    {
      ecp = cacheList;
      if(ecp->shareCount == 0)
      {
        break;
      }
    }

    ecp->dev = dev;
    ecp->ino = ino;
    ecp->shareCount = 1;
    ecp->cacheCount = 1;
    ecp->modified = 0;
    return ecp;
  }


}

int iput(MINODE *mip)  // release a mip
{
  /*******************
 1.  if (mip==0)                return;

     mip->shareCount--;         // one less user on this minode

     if (mip->shareCount > 0)   return;
     if (!mip->modified)        return;
 
 2. // last user, INODE modified: MUST write back to disk
    Use Mailman's algorithm to write minode.INODE back to disk)
    // NOTE: minode still in cacheList;
    *****************/
   INODE* ip;
   int i, block, offset;
   char buf[BLKSIZE];


   if(mip == 0) return;
   
   mip->shareCount--;

   if(mip->shareCount > 0) return;

   if(!mip->modified) return;

   //write inode back to disk
   block = (mip->ino - 1) / 4 + iblk;
   offset = (mip->ino - 1) % 4;

   //get block containing this inode
   get_block(mip->dev, block, buf);
   ip = (INODE*)buf + (offset*2);
   *ip = mip->INODE;
   put_block(mip->dev, block, buf);
   mip->cacheCount = 0; //deallocate
} 

int search(MINODE *mip, char *name)
{
  /******************
  search mip->INODE data blocks for name:
  if (found) return its inode number;
  else       return 0;
  ******************/
  char sbuf[BLKSIZE], temp[256];
   DIR *dp;
   char *cp;

   get_block(fd, mip->INODE.i_block[0], sbuf);

   dp = (DIR *)sbuf;
   cp = sbuf;

   while(cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;

      //search
      if(strcmp(temp, name) == 0)
      {
         printf("found %s at indode %d\n", name, dp->inode);
         return dp->inode;
      }

      cp += dp->rec_len;
      dp = (DIR *)cp;
   }
   return 0;

}

MINODE *path2inode(char *pathname) 
{
  /*******************
  return minode pointer of pathname;
  return 0 if pathname invalid;
  
  This is same as YOUR main loop in LAB5 without printing block numbers
  *******************/
    int ino = 0;
    MINODE* mip;
    
    if (pathname[0] == '/')
        mip = root;
    else
        mip = running->cwd;
    
    tokenize(pathname);
    
    for (int i=0; i < n; i++){

        if (!(mip->INODE.i_mode >> 14) == 1)
            return 0;

        ino = search(mip, name[i]);
        if (ino==0)
            return 0;

        iput(mip);             // release current mip
        mip = iget(dev, ino);  // change to new mip of (dev, ino)
    }
    return mip;
} 


int findmyname(MINODE *pip, int myino, char myname[256]) 
{
  /****************
  pip points to parent DIR minode: 
  search for myino;    // same as search(pip, name) but search by ino
  copy name string into myname[256]
  ******************/
 char sbuf[BLKSIZE], temp[256];
   DIR *dp;
   char *cp;

   get_block(fd, pip->INODE.i_block[0], sbuf);

   dp = (DIR *)sbuf;
   cp = sbuf;

   while(cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;

      //search
      if(myino == dp->inode)
      {
         //printf("found %s at indode %d\n", name, dp->inode);
         strcpy(myname, temp);
         return dp->inode;
      }

      cp += dp->rec_len;
      dp = (DIR *)cp;
   }
   return 0;

}
 
int findino(MINODE *mip, int *myino) 
{
  /*****************
  mip points at a DIR minode
  i_block[0] contains .  and  ..
  get myino of .
  return parent_ino of ..
  *******************/
  char sbuf[BLKSIZE], temp[256];
   DIR *dp;
   char *cp;

   get_block(fd, mip->INODE.i_block[0], sbuf);

   dp = (DIR *)sbuf;
   cp = sbuf;
   while(cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;

      //search
      if(strcmp(temp, ".") == 0)
      {
         //printf("found %s at indode %d\n", name, dp->inode);
         *myino = dp->inode;
      }
      if(strcmp(temp, "..") == 0)
      {
         //printf("found %s at indode %d\n", name, dp->inode);
         return dp->inode;
      }

      cp += dp->rec_len;
      dp = (DIR *)cp;
   }
}
