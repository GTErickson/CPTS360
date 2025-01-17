int cat(){

    //printf("In cat\n");
   char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
   int n;
   int total = 0;
   int i = 0;

    int fd = open_file("0");

    if (fd > -1){
        while( n = myread(fd, mybuf, 1024)){
            mybuf[n] = 0;             // as a null terminated string
            // total += n;
            // mylseek(fd, total);
            printf("%s", mybuf);
            // i = 0;
            // while(mybuf[i] != 0)
            // {
            //     printf("%c", mybuf[i]);
            //     i++;
            // }
        }
        close_file(fd);
        return 1;
        
    }
    else{
        quit();
        
        //return -1;
    }
}

int cp()
{
    char temp_buf[256];

    int fd = open_file("0"); //open for read
    //strncpy(pathname, parameter, strlen(parameter));
    for(int i = 0; i < 128; ++i){
         pathname[i] = 0;
      }
    strncpy(temp_buf, parameter, strlen(parameter));
    strncpy(pathname, temp_buf, strlen(parameter));

    //printf("pathname: %s", pathname);
    int gd = open_file("1"); //open for write / creat
   // pfd();

    char mybuf[BLKSIZE];
    int n;
    while( n = myread(fd, mybuf, BLKSIZE))
    {
        //printf("n = %d", n);
        //printf("test\n");
        mywrite(gd, mybuf, n);
    }

    close_file(fd);
    close_file(gd);

}