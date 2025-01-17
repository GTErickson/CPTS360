#include "type.h"

/********** globals **************/
PROC   proc[NPROC];
PROC   *running;

MINODE minode[NMINODE];   // in memory INODES
MINODE *freeList;         // free minodes list
MINODE *cacheList;        // cached minodes list

MINODE *root;             // root minode pointer

OFT    oft[NOFT];         // for level-2 only

char gline[256];          // global line hold token strings of pathname
char *name[64];           // token string pointers
int  n;                   // number of token strings                    

int count = 0;
char line[128];

int ninodes, nblocks;     // ninodes, nblocks from SUPER block
int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers
int link_ino;

int  fd, dev;
char cmd[16], pathname[128] = {'\0'}, parameter[128];
int  requests, hits;

// inode_size = s_inode_size in SUPERblock;           256
// compute      INODEsize  = sizeof(INODE);                        128
// compute      inodes_per_block = BLKSIZE/inode_size=1024/256  =    4
//              ifactor          = inode_size/INDOEsize=256/128 =    2

int inode_size = 256;
// int INODEsize = sizeof(INODE);
// int inodes_per_block = BLKSIZE / inode_size;
// int ifactor = inode_size / INDOEsize;


// start up files
#include "util.c"
#include "cd_ls_pwd.c"
#include "alloc_dealloc.c"
#include "mkdir_create.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "symlink.c"
#include "open_close.c"
#include "read.c"
#include "write.c"
#include "cat_cp.c"
#include "head_tail.c"

int init()
{
  int i, j;
  // initialize minodes into a freeList
  for (i=0; i<NMINODE; i++){
    MINODE *mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->id = i;
    mip->next = &minode[i+1];
  }
  minode[NMINODE-1].next = 0;
  freeList = &minode[0];       // free minodes list

  cacheList = 0;               // cacheList = 0

  for (i=0; i<NOFT; i++)
    oft[i].shareCount = 0;     // all oft are FREE
 
  for (i=0; i<NPROC; i++){     // initialize procs
     PROC *p = &proc[i];    
     p->uid = p->gid = i;      // uid=0 for SUPER user
     p->pid = i+1;             // pid = 1,2,..., NPROC-1

     for (j=0; j<NFD; j++)
       p->fd[j] = 0;           // open file descritors are 0
  }
  
  running = &proc[0];          // P1 is running
  requests = hits = 0;         // for hit_ratio of minodes cache
}

char *disk = "disk2";

int main(int argc, char *argv[ ]) 
{
  //char line[128];
  char buf[BLKSIZE];

  init();
  
  fd = dev = open(disk, O_RDWR);
  printf("dev = %d\n", dev);  // YOU should check dev value: exit if < 0

  // get super block of dev
  get_block(dev, 1, buf);
  SUPER *sp = (SUPER *)buf;  // you should check s_magic for EXT2 FS
  printf("check: superblock magic = 0x%x  ", sp->s_magic);
  if(sp->s_magic != 0xEF53)
   {
      printf("not an EXT2 FS\n");
      exit(2);
   }
   //printf("magic number: %d \n", sp->s_magic);
   printf("Ok\n");
  
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  printf("ninodes=%d  nblocks=%d  inode_size=%d\n", ninodes, nblocks, inode_size);

  get_block(dev, 2, buf);
  GD *gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = inodes_start = gp->bg_inode_table;

  printf("bmap=%d  imap=%d  iblk=%d\n", bmap, imap, iblk);

  // HERE =========================================================
  // MINODE *mip = freeList;         // remove minode[0] from freeList
  // freeList = freeList->next;
  // cacheList = mip;                // enter minode[0] in cacheList

  // // get root INODE
  // get_block(dev, iblk, buf);
  // INODE *ip = (INODE *)buf + 1;   // #2 INODE
  // mip->INODE = *ip;               // copy into mip->INODE

  // mip->cacheCount = 1;
  // mip->shareCount = 2;            // for root AND CWD
  // mip->modified   = 0;

  // root = mip;           // root points at #2 INODE in minode[0]
                        
  // printf("set P1's CWD to root\n");
  // running->cwd = root;           // CWD = root
  // Endhere ====================================================
  
 /********* write code for iget()/iput() in util.c **********
          Replace code between HERE and ENDhere with

  root         = iget(dev, 2);  
  running->cwd = iget(dev, 2);
 **********************************************************/
  root = iget(dev, 2);  
  running->cwd = iget(dev, 2);
   printf("rootShareCount = %d\n", root->shareCount);
  
  while(1){
      printf("P%d running\n", running->pid);
      for(int i = 0; i < 128; ++i){
         pathname[i] = 0;
         parameter[i] = 0;
      }
         
      printf("enter command [cd|ls|pwd|mkdir|creat|rmdir|link|unlink|symlink|show|hits|exit] : ");
      fgets(line, 128, stdin);
      line[strlen(line)-1] = 0;    // kill \n at end


      if (line[0]==0)
         continue;
        
      
      sscanf(line, "%s %s %64c", cmd, pathname, parameter);
      printf("pathname=%s parameter=%s\n", pathname, parameter);
         
      if (strcmp(cmd, "ls")==0)
	      ls();
      if (strcmp(cmd, "cd")==0)
	      cd();
      if (strcmp(cmd, "pwd")==0)
        pwd();
      if (strcmp(cmd, "mkdir")==0)
        make_dir();
      if (strcmp(cmd, "rmdir")==0)
         rmdir();
      if (strcmp(cmd, "creat")==0)
        creat_file();
      if (strcmp(cmd, "link")==0)
        link(parameter);
      if (strcmp(cmd, "unlink")==0)
        unlink();
      if (strcmp(cmd, "symlink")==0)
         symlink(parameter);
      if (strcmp(cmd, "open")==0)
         open_file(parameter);
      if (strcmp(cmd, "close")==0)
         close_file(atoi(pathname));
      if (strcmp(cmd, "pfd")==0)
         pfd();
      if (strcmp(cmd, "lseek")==0)
         mylseek(atoi(pathname), atoi(parameter));
      if (strcmp(cmd, "dup")==0)
	      dup(atoi(pathname));
      if (strcmp(cmd, "dup2")==0)
	      dup2(atoi(pathname), atoi(parameter));
      if (strcmp(cmd, "read")==0)
         read_file();
      if (strcmp(cmd, "write")==0)
         write_file();
      if (strcmp(cmd, "cat")==0)
         cat();
      if (strcmp(cmd, "cp")==0)
         cp();   
      if (strcmp(cmd, "head")==0)
         head_file();
      if (strcmp(cmd, "tail")==0)
         tail_file();
      if (strcmp(cmd, "show")==0)
	      show_dir(running->cwd);
      if (strcmp(cmd, "hits")==0)
	      hit_ratio();
      if (strcmp(cmd, "exit")==0)
	      quit();
  }
}


int show_dir(MINODE *mip)
{
  // show contents of mip DIR: same as in LAB5
  char sbuf[BLKSIZE], temp[256];
   DIR *dp;
   char *cp;

   // ASSUME only one data block i_block[0]
   // YOU SHOULD print i_block[0] number here
   printf("i_block[0] = %d\n", mip->INODE.i_block[0]);
   printf("*****************************************\n");
   printf("i_number rec_length  name_len   name\n");
   get_block(fd, mip->INODE.i_block[0], sbuf);

   dp = (DIR *)sbuf;
   cp = sbuf;
   while(cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      
      printf("%8d %8d %8d %8s\n", dp->inode, dp->rec_len, dp->name_len, temp);
      cp += dp->rec_len;
      dp = (DIR *)cp;
   }
   printf("*****************************************\n");
}

int hit_ratio()
{
  // print cacheList;
  printf("cacheList = ");
  MINODE *p = cacheList;
  while(p){
   printf("c%d[%d %d]s%d->", p->cacheCount, p->dev, p->ino, p->shareCount);
   p = p->next;
  }
  printf("NULL\n");
  // compute and print hit_ratio
  printf("Hits = %d | Requests = %d | Hit Ratio = %f\n", hits, requests, (float)hits/(float)requests);
}

int quit()
{
   MINODE *mip = cacheList;
   while(mip){
     if (mip->shareCount){
        mip->shareCount = 1;
        iput(mip);    // write INODE back if modified
     }
     mip = mip->next;
   }
   exit(0);
}