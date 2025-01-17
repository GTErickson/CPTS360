// link, unlink, symlink file

int truncate(INODE ip)
{
   int ibuf[256], lbuf[256];
   int i = 0;
   //direct blocks
   for (int l = 0; l < 12; ++l){
      if(ip.i_block[l])
         ip.i_block[l] = 0; //deallocate
   }
   

   //indirect blocks
   int b12 = ip.i_block[12];
   if (b12){
      i = 0;
      get_block(dev, b12, ibuf);
      while(ibuf[i] && i < 256){
         ibuf[i] = 0; //deallocate
         i++;
      }
   }


   //double indirect blocks
   int b13 = ip.i_block[13];
   if (b13){
      i = 0;
      get_block(dev, b13, ibuf);
      while(ibuf[i] && i < 256){
         b12 = ibuf[i];
         if (b12){
            int l = 0;
            get_block(dev, b12, lbuf);
            while(lbuf[l] && l < 256){
               lbuf[l] = 0; //deallocate
               l++;
            }
         }
         ++i;
      }
}
}

int link(char* to_link)
{
    MINODE* mip = path2inode(pathname);

    if((mip->INODE.i_mode & 0xF000) == 0x4000){
        printf("Error: link to DIR not allowed\n");
        iput(mip);
        return -1;
    }

    char * arg1 = basename(pathname);
    char * parent = dirname(pathname);
    printf("parent: %s\n", parent);
    printf("child: %s\n", arg1);
    printf("newfile: %s\n", to_link);

    MINODE *pip = path2inode(parent);

    if(!pip){ //Verify parent is found
        printf("Error! Could not find parent\n");
        return -1;
    }
    if(!((pip->INODE.i_mode & 0xF000) == 0x4000)){ //Verify parent is DIR
        printf("Error! Parent is not a DIR\n");
        return -1;
    }
    if(search(pip, to_link)){ //Verify child does not already exist in parent
        printf("Error! Child already exists in DIR\n");
        return -1;
    }

    
    //create file
    enter_child(pip, mip->ino, to_link);



    
    mip->INODE.i_links_count++;
    mip->INODE.i_atime == time(0L);
    mip->modified = 1;

    iput(pip);

}



int unlink()
{
    //get pathname's INODE into memory
    MINODE* mip = path2inode(pathname);

    //verify it isn't a DIR
    if((mip->INODE.i_mode & 0xF000) == 0x4000){
        printf("Error: cannot unlink a DIR\n");
        iput(mip);
        return -1;
    }    

    //decrement links count
    mip->INODE.i_links_count--;

    //remove file completely
    if(mip->INODE.i_links_count == 0)
    {
        //deallocate data blocks
        truncate(mip->INODE);
        printf("Deallocating all data blocks\n");

        //deallocate inode
        mip->ino = 0;

    }

    char * child = basename(pathname);
    char * parent = dirname(pathname);
    printf("parent: %s\n", parent);
    printf("child: %s\n", child);

    MINODE *pip = path2inode(parent);

    rm_child(pip, child);

    pip->modified = 1;
    iput(pip);
    iput(mip);


}