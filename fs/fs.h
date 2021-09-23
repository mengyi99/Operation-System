#ifndef INCLUDE_FS_H_
#define INCLUDE_FS_H_

#include "type.h"


#define KFS_MAGIC (0x66666666)
#define OFFSET_512M (0x20000000)

#define FS_SIZE OFFSET_512M 
#define OFFSET_FS OFFSET_512M
#define block_SIZE 0x1000    

#define INODE_MAP 0x1000 //1 block
#define block_MAP 4 * 0x1000  //4 block
#define INODE_NUM 512* 0x1000 // 8 block

#define MAX_DIRECT_NUM 10
#define MAX_NAME_LENGTH 20


#define NUM_INODE 32 * 512


#define INIT_USED_block 518    // 1 + 4 + 1 + 512

#define TYPE_FILE   1
#define TYPE_DIR    2
#define TYPE_SELF   3
#define TYPE_FATHER 4
#define TYPE_ROOT   5
#define TYPE_SLINK  6

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   3

#define BUFFER 0xffffffffa8000000

#define ROOT_ID 0

// offset is block offset
typedef struct superblock
{
    uint32_t magic;
    uint32_t start_block;
    uint32_t block_number;
    uint32_t used_block;

    uint32_t block_map_offset;
    uint32_t block_map;
    uint32_t inode_map_offset;
    uint32_t inode_map;
    uint32_t inode_offset;
    uint32_t inode_num;
    uint32_t data_offset;
    uint32_t data_num;

    uint32_t inode_size;
    uint32_t dir_size;
} super_block_t;

typedef struct inode_entry 
{
    uint32_t id;    // inode number
    uint32_t type;
    uint32_t mode;
    uint32_t link_soft;
    uint32_t links_cnt;
    uint32_t fsize;
    uint32_t fnum;  // 目录中文件数
    uint32_t direct_table[MAX_DIRECT_NUM];
    uint32_t indirect_1_ptr;
    uint32_t indirect_2_ptr;
    uint32_t indirect_3_ptr;
    uint32_t padding[12];
} inode_entry_t;    // 32 * 4B = 128B

 
typedef struct dir_entry
{
    uint32_t id;
    uint32_t type;
    uint32_t mode;
    char name[MAX_NAME_LENGTH];
} dir_entry_t;      // 32B

typedef struct fd
{
    uint32_t inode_id;
    uint32_t mode;
    uint32_t r_offset;
    uint32_t w_offset;
} fd_t;

// extern inode_entry_t current_dir_entry;


int do_link(char *src, char *dest, uint32_t soft);


void init_fs();
void print_fs_info();
void read_super_block();
int mkdir(char *name);
int rmdir(char *name);
int enter_dir(char *name);
int readdir();
int open(char *name,uint32_t access);
int write(uint32_t fd,char *buff,uint32_t size,uint32_t w);
int read(uint32_t fd,char *buff,uint32_t size,uint32_t r);
int close(uint32_t fd);
int cat(char *name);
int mknod(char *name);
#endif