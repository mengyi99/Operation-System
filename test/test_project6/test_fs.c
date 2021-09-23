#include "test_fs.h"
#include "fs.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"

static char buff[4096];

void test_fs(void)
{
    int i, j;
    int fd = open("1.txt", O_RDWR);

    // write 'hello world!' * 10
    for (i = 0; i < 10; i++)
    {
        write(fd, "hello world!\n", 13,0);
        do_scheduler();
    }
    // read
    
    vt100_move_cursor(0, 1);
    for (i = 0; i < 10; i++)
    {
        read(fd, buff, 13,0);
        do_scheduler();
        for (j = 0; j < 13; j++)
        {
            printk("%c", buff[j]);
        }
    }

    close(fd);
    do_exit();

}

void test_bf(void)
{
    char s[4096];
    int a,j;
    for(a=0;a<4096;a++)
    {
       s[a]='a';
    }
    
    int i;
    int fd = open("2.txt", O_RDWR);

    // write 'hello world!' * 10
        write(fd,s,4096,0);
        
        vt100_move_cursor(0, 1); 
        printk("Finish write block 1");

        write(fd,s,0,2046);

        write(fd, s, 4096,0);
        vt100_move_cursor(0, 2); 
        printk("Finish write");


    // read
        read(fd, buff, 4096,0);

        
        vt100_move_cursor(0, 4); 
        printk("Finish read block 1:");
        for (j = 0; j < 13; j++)
        {
            printk("%c", buff[j]);
        }

        read(fd, buff, 0,2046);

        read(fd, buff,4096,0);
        vt100_move_cursor(0, 5); 
        printk("Finish read :");
  
        for (j = 0; j < 13; j++)
        {
            printk("%c", buff[j]);
        }

    close(fd);
    do_exit();

}
