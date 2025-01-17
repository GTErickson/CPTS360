/* Part 2 */

#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

struct partition {
	u8 drive;             // drive number FD=0, HD=0x80, etc.

	u8  head;             // starting head
	u8  sector;           // starting sector
	u8  cylinder;         // starting cylinder

	u8  sys_type;         // partition type: NTFS, LINUX, etc.

	u8  end_head;         // end head
	u8  end_sector;       // end sector
	u8  end_cylinder;     // end cylinder

	u32 start_sector;     // starting sector counting from 0
	u32 nr_sectors;       // number of of sectors in partition
};

char *dev = "vdisk";
int fd;

// read a disk sector into char buf[512]
int read_sector(int fd, int sector, char *buf)
{
        lseek(fd, sector*512, SEEK_SET);  // lssek to byte sector*512
        read(fd, buf, 512);               // read 512 bytes into buf[ ]
}

int main()
{
  struct partition *p;
  char buf[512];

  fd = open(dev, O_RDONLY);   // open dev for READ
  read(fd, buf, 512);               // read 512 bytes into buf[ ]
  read_sector(fd, 0, buf);    // read in MBR at sector 0    

  p = &buf[446];

  //print all for sectors
  printf("     start_sector  nr_sectors  sys_type\n");
  for (int i = 0; i < 4; ++i){
      printf("P%d : %-14u%-14u%x\n", (i + 1), (p + i)->start_sector, (p + i)->nr_sectors, (p + i)->sys_type);
      
  }
  putchar('\n');

  printf("********Looking for Extend Partition********\n");
  p += 3;
  int extStart = p->start_sector;
  //print extStart to see it;
  printf("Ext Partition 4 : start_ector=%d\n", extStart);
  int localMBR = extStart;

  for(int i = 0; 1; ++i){
    printf("next local sector=%d\n", localMBR);
    printf("--------------------------------------------\n");
    read_sector(fd, localMBR, buf);
    p = &buf[446];
    // print entry 1 and 2's start_sector, nr_sector;
    printf("Entry1 : start_sector=%u nr_sectors=%u\n", p->start_sector, p->nr_sectors);
    printf("Entry2 : start_sector=%u nr_sectors=%u\n", (p + 1)->start_sector, (p + 1)->nr_sectors);
    printf("--------------------------------------------\n");
    // compute and print P5's begin, end, nr_sectors
    printf("     start_sector  end_sectors  nr_sectors\n");
    printf("P%d : %-14u%-13u%u\n\n", (i + 5), (p->start_sector + localMBR), (localMBR + (p + 1)->start_sector - 1) ,p->nr_sectors);

    if ((p + 1)->start_sector != 0){
        //compute next localMBR sector;
        localMBR = (p + 1)->start_sector + extStart;
    }
    else{
      printf("End of Partitions\n");
      break;
    }
  }
}
