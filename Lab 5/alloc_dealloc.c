int tst_bit(char *buf, int bit){
    //in chapter 11.3.1
    return buf[bit/4] & (1 << (bit % 4));
}

int set_bit(char* buf, int bit){
    //in chapter 11.3.1
    buf[bit/4] |= (1 << (bit % 4));
}

int clr_bit(char *buf, int bit) // clear bit in char buf[BLKSIZE]
{
   buf[bit/4] &= ~(1 << (bit%4)); 
}

int decFreeInodes(int dev){
    char buf[BLKSIZE];

   get_block(dev, 1, buf);
   SUPER* sp = (SUPER *)buf;
   sp->s_free_inodes_count--;
   put_block(dev, 1, buf);

   get_block(dev, 2, buf);
   GD* gp = (GD*)buf;
   gp->bg_free_inodes_count--;
   put_block(dev, 2, buf);
}


int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  SUPER* sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  GD* gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev){
    char buf[BLKSIZE];

   get_block(dev, 1, buf);
   SUPER* sp = (SUPER *)buf;
   sp->s_free_blocks_count--;
   put_block(dev, 1, buf);

   get_block(dev, 2, buf);
   GD* gp = (GD*)buf;
   gp->bg_free_blocks_count--;
   put_block(dev, 2, buf);
}

int incFreeBlocks(int dev){
    char buf[BLKSIZE];

   get_block(dev, 1, buf);
   SUPER* sp = (SUPER *)buf;
   sp->s_free_blocks_count++;
   put_block(dev, 1, buf);

   get_block(dev, 2, buf);
   GD* gp = (GD*)buf;
   gp->bg_free_blocks_count++;
   put_block(dev, 2, buf);
}

int ialloc(int dev){
   int i;
   char buf[BLKSIZE];

   get_block(dev, imap, buf);

   for(i=0; i < ninodes; i++){
      if (tst_bit(buf, i) == 0){
         set_bit(buf, i);
         put_block(dev, imap, buf);

         decFreeInodes(dev);

         printf("ialloc : ino=%d\n", i+1);
         return i+1;
      }
   }
   return 0;
}

int balloc(dev){
   int i;
   char buf[BLKSIZE];

   get_block(dev, bmap, buf);

   for(i=0; i < nblocks; i++){
      if (tst_bit(buf, i) == 0){
         set_bit(buf, i);
         put_block(dev, bmap, buf);

         decFreeBlocks(dev);

         printf("balloc : blk=%d\n", i+1);
         return i+1;
      }
   }
   return 0;
}

int idalloc(int dev, int ino)  // deallocate an ino number
{
  int i;  
  char buf[BLKSIZE];

  // return 0 if ino < 0 OR > ninodes
  if(ino < 0 | ino > ninodes)
   return;

  // get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);

  // write buf back
  put_block(dev, imap, buf);

  // update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int bdalloc(int dev, int blk)  // deallocate an blk number
{
  int i;  
  char buf[BLKSIZE];

  // return 0 if blk < 0 OR > nblocks
  if(blk < 0 | blk > nblocks)
   return;

  // get inode bitmap block
  get_block(dev, bmap, buf);
  clr_bit(buf, blk-1);

  // write buf back
  put_block(dev, bmap, buf);

  // update free inode count in SUPER and GD
  incFreeBlocks(dev);
}