#include "fs.h"
#include "string.h"
#include "common.h"
#include "screen.h"
#include "sched.h"

static uint8_t superblock_buffer[block_SIZE];
static uint8_t inodemap_buffer[INODE_MAP];
static uint8_t blockmap_buffer[block_MAP];
static inode_entry_t inode_buffer;

static super_block_t *superblock = (super_block_t *)superblock_buffer;
static fd_t fds[15];
static num_open_files = 0;
static inode_entry_t current_dir_entry;
static current_dir_id = 0;

void read_superblock()
{
    sd_card_read(superblock, OFFSET_FS, block_SIZE);
}

void write_superblock()
{
    sd_card_write(superblock, OFFSET_FS, block_SIZE);
}
void read_blockmap()
{
    sd_card_read(blockmap_buffer,superblock->block_map_offset,block_MAP);
}

void write_blockmap()
{
    sd_card_write(blockmap_buffer,superblock->block_map_offset,block_MAP);
}

void read_inodemap()
{
    sd_card_read(inodemap_buffer, superblock->inode_map_offset,INODE_MAP);
}

void write_inodemap()
{
    sd_card_write(inodemap_buffer, superblock->inode_map_offset,INODE_MAP);
}

void read_inode(uint32_t id)
{
    uint32_t block = id / (block_SIZE / sizeof(inode_entry_t));
    uint32_t part = id % (block_SIZE / sizeof(inode_entry_t));
    sd_card_read((void *)BUFFER, superblock->inode_offset + block * block_SIZE, block_SIZE);
    memcpy((uint8_t *)&inode_buffer, (uint8_t *)(BUFFER + part * sizeof(inode_entry_t)), sizeof(inode_entry_t));
}

void write_inode(uint32_t id)
{
    uint32_t block = id / (block_SIZE / sizeof(inode_entry_t));
    uint32_t part = id % (block_SIZE / sizeof(inode_entry_t));
    sd_card_read((void *)BUFFER, superblock->inode_offset + block * block_SIZE, block_SIZE);
    memcpy((uint8_t *)(BUFFER + part * sizeof(inode_entry_t)), (uint8_t *)&inode_buffer, sizeof(inode_entry_t));
    sd_card_write((void *)BUFFER, superblock->inode_offset + block * block_SIZE, block_SIZE);
}

void read_block(uint32_t id)
{
    sd_card_read((void *)BUFFER, OFFSET_FS + id * block_SIZE, block_SIZE);
}

void write_block(uint32_t id)
{
    sd_card_write((void *)BUFFER, OFFSET_FS + id * block_SIZE, block_SIZE);
}

uint32_t alloc_inode()
{
    read_inodemap();
    int i, j;
    for (i = 0; i < INODE_MAP; i++)
    {
        if (inodemap_buffer[i] != 0xff)
        {
            break;
        }
    }
    uint8_t temp = inodemap_buffer[i];
    for (j = 0; j < 8; j++)
    {
        if ((temp & 0x01) == 0)
        {
            break;
        }
        else
        {
            temp = temp >> 1;
        }
    }
    temp = 0x01 << j;
    inodemap_buffer[i] |= temp;
    write_inodemap();
    write_superblock();
    return (8 * i + j);
}

void free_inode(uint32_t id)
{
    int i = id / 8;
    int j = id % 8;
    uint8_t temp = 0x01 << j;
    read_inodemap();
    inodemap_buffer[i] &= (~temp);
    write_inodemap();
    write_superblock();
}

uint32_t alloc_block()
{
    read_blockmap();
    int i, j;
    for (i = 0; i < block_MAP; i++)
    {
        if (blockmap_buffer[i] != 0xff)
        {
            break;
        }
    }
    uint8_t temp = blockmap_buffer[i];
    for (j = 0; j < 8; j++)
    {
        if ((temp & 0x01) == 0)
        {
            break;
        }
        else
        {
            temp = temp >> 1;
        }
    }
    temp = 0x01 << j;
    blockmap_buffer[i] |= temp;
    write_blockmap();
    superblock->used_block++;
    write_superblock();
    return (8 * i + j);
}

void free_block(uint32_t id)
{
    int i = id / 8;
    int j = id % 8;
    uint8_t temp = 0x01 << j;
    read_blockmap();
    blockmap_buffer[i] &= (~temp);
    write_blockmap();
    superblock->used_block--;
    write_superblock();
}

void init_superblock()
{
    bzero(superblock_buffer, block_SIZE);
    superblock->magic = KFS_MAGIC;
    superblock->start_block = OFFSET_FS;
    superblock->block_number = FS_SIZE/block_SIZE;

    superblock->block_map_offset = OFFSET_FS+block_SIZE;
    superblock->block_map = block_MAP ;
    superblock->inode_map_offset = superblock->block_map_offset+superblock->block_map;
    superblock->inode_map = INODE_MAP;
    
    superblock->inode_offset = superblock->inode_map_offset + superblock->inode_map;
    superblock->inode_num = INODE_NUM;
    
    superblock->data_offset = superblock->inode_offset + superblock->inode_num;
    superblock->data_num =  FS_SIZE - superblock->data_offset + OFFSET_FS;
    

    superblock->used_block = INIT_USED_block;
    superblock->inode_size = sizeof(inode_entry_t);
    superblock->dir_size = sizeof(dir_entry_t);
    write_superblock();
}

void init_inodemap()
{
   bzero(inodemap_buffer,INODE_MAP);
   write_inodemap();
}

void init_blockmap()
{
    bzero(blockmap_buffer,block_MAP);

    int i, j;
    for (i = 0; i < INIT_USED_block / 8; i++)
    {
        blockmap_buffer[i] = 0xff;
    }
    for (j = 0; j < INIT_USED_block % 8; j++)
    {
        blockmap_buffer[i] |= (0x01 << j);
    }
    write_blockmap();
}

void init_inode(uint32_t block, uint32_t father, uint32_t self, uint32_t mode, uint32_t type)
{
    inode_buffer.id = self;
    inode_buffer.type = type;
    inode_buffer.mode = mode;
    inode_buffer.links_cnt = 0;
    if (type == TYPE_DIR || type == TYPE_ROOT)
    {
        inode_buffer.fsize = 2 * sizeof(dir_entry_t);
    }
    else
    {
        inode_buffer.fsize = 0;
    }
    inode_buffer.fnum = 0;
    inode_buffer.direct_table[0] = block;
    int i;
    for (i = 1; i < MAX_DIRECT_NUM; i++)
    {
        inode_buffer.direct_table[i] = 0;
    }
    inode_buffer.indirect_1_ptr = 0;
    inode_buffer.indirect_2_ptr = 0;
    inode_buffer.indirect_3_ptr = 0;

    write_inode(inode_buffer.id);
}

// init directory block
void init_dir_block(uint32_t block, uint16_t father, uint16_t self)
{
    read_block(block);

    dir_entry_t *dir = (dir_entry_t *)BUFFER;
    dir->name[0] = '.';
    dir->name[1] = '.';
    dir->name[2] = '\0';
    dir->id = father;
    dir->type = TYPE_FATHER;
    dir->mode = O_RDWR;

    dir = (dir_entry_t *)(BUFFER + sizeof(dir_entry_t));
    dir->name[0] = '.';
    dir->name[1] = '\0';
    dir->id = self;
    dir->type = TYPE_SELF;
    dir->mode = O_RDWR;

    write_block(block);
}

void init_rootdir()
{
    uint32_t self = alloc_inode();
    if (self != ROOT_ID)
    {
        kprintf("[ERROR] ROOT DIR INODE NUMBER = %d, NOT ZERO\n\r", self);
    }
    uint32_t block = alloc_block();
    init_inode(block, self, self, O_RDWR, TYPE_ROOT);
    init_dir_block(block, self, self);
    memcpy((uint8_t *)&current_dir_entry, (uint8_t *)&inode_buffer, sizeof(inode_entry_t));
}


int is_name_in_dir(char *name)
{
    read_block(current_dir_entry.direct_table[0]);
    int i;
    dir_entry_t *dir;
    for (i = 0; i < current_dir_entry.fnum; i++)
    {
        dir = (dir_entry_t *)(BUFFER + (i + 2) * sizeof(dir_entry_t));

        if (!strcmp(dir->name, name))
        {
            break;
        }
    }
    if (i == current_dir_entry.fnum)
    {

        return -1; // failure
    }
    else
    {
        return i;
    }
}


int enter_dir(char *name)
{
    int id;
    if (name[strlen(name) - 1] == '/' && strlen(name) != 1)
    {
        name[strlen(name) - 1] = '\0';
    }
    char name_buf[10];
    while (strlen(name))
    {
        if (name[0] == '/')
        {
            id = ROOT_ID;
            strcpy(name_buf, "root");
            name++;
        }
        else if (name[0] == '.' && name[1] == '.')
        {
            if (name[2] == '/' || name[2] == '\0')
            {
                read_block(current_dir_entry.direct_table[0]);
                dir_entry_t *dir = (dir_entry_t *)BUFFER; // ..
                id = dir->id;
                if (name[2] == '/')
                {
                    name += 3;
                }
                else
                {
                    name += 2;
                }
                strcpy(name_buf, "..");
            }
            else
            {
                kprintf("Invalid path: %s\n", name);
                return -1;
            }
        }
        else if (name[0] == '.')
        {
            if (name[1] == '/' || name[1] == '\0')
            {
                if (name[1] == '/')
                {
                    name += 2;
                }
                else
                {
                    // '\0'
                    name += 1;
                    break;
                }
            }
            else
            {
                kprintf("Invalid path: %s", name);
                return -1;
            }
            continue;
        }
        else
        {
            int j;
            for (j = 0; j < strlen(name) && name[j] != '/'; j++)
            {
                name_buf[j] = name[j];
            }
            name_buf[j] = '\0';
            
            int i = is_name_in_dir(name_buf);
            if(i == -1)
            {
                kprintf("Dir: %s not found...\n", name);
                return -1;
            }
            
            dir_entry_t *dir = (dir_entry_t *)(BUFFER + (i + 2) * sizeof(dir_entry_t));
            id = dir->id;
            if (dir->type == TYPE_FILE) {
               kprintf("%s : Not a directory.\n",name);
               return;
            }
            
            else if (dir->type == TYPE_SLINK) {
            read_inode(id);
            read_block(inode_buffer.direct_table[0]);
            uint32_t *p_data = (uint32_t *)BUFFER;
            id = *p_data;
            dir_entry_t *dir = (dir_entry_t *)(BUFFER + (id + 2) * sizeof(dir_entry_t));
            if(current_dir_entry.link_soft == 0)
               {
               kprintf("No Such SoftLink.\n");
               return;
               }  
            }
            
            if (name[j] == '\0')
            {
                name += j;
            }
            else
            {
                name += j + 1;
            }
        }
        kprintf("Opening dir: %s (id = %d)...\n", name_buf, id);
        read_inode(id);
        memcpy((uint8_t *)&current_dir_entry, (uint8_t *)&inode_buffer, sizeof(inode_entry_t));
    }
    return 0;
}

int readdir()
{

    read_block(current_dir_entry.direct_table[0]);
    int i;
    dir_entry_t *dir = (dir_entry_t *)(BUFFER + 2 * sizeof(dir_entry_t));
    if (current_dir_entry.fnum>0)
    {
    for (i = 0; i < current_dir_entry.fnum; i++)
    {
        dir_entry_t *dir = (dir_entry_t *)(BUFFER + (i + 2) * sizeof(dir_entry_t));
        kprintf("%s(%s)   ",dir->name,(dir->type == TYPE_DIR) ? "DIR" : (dir->type == TYPE_FILE)? "FILE":"SLINK");
    }
    kprintf("\n");
    }
    return 0;
}

int mkdir(char *name)
{
    uint32_t id = alloc_inode();
    uint32_t block = alloc_block();
    init_inode(block, current_dir_entry.id, id, O_RDWR, TYPE_DIR);
    init_dir_block(block, current_dir_entry.id, id);

    // modify current dir block
    read_block(current_dir_entry.direct_table[0]); // TODO: more dir
    dir_entry_t *dir = (dir_entry_t *)(BUFFER + (current_dir_entry.fnum + 2) * sizeof(dir_entry_t));
    dir->id = id;
    dir->type = TYPE_DIR;
    dir->mode = O_RDWR;
    strcpy(dir->name, name);
    write_block(current_dir_entry.direct_table[0]);

    // modify current dir entry
    current_dir_entry.fsize += sizeof(dir_entry_t);
    current_dir_entry.fnum += 1;
    current_dir_entry.link_soft += 1;
    current_dir_entry.links_cnt += 1;
    // printk("current id = %d inode.fnum = %d\n\r", current_dir_entry.id, current_dir_entry.fnum);
    memcpy((uint8_t *)&inode_buffer, (uint8_t *)&current_dir_entry, sizeof(inode_entry_t));
    write_inode(current_dir_entry.id);

    return 0;
}

int rmdir(char *name)
{
    dir_entry_t *dir;
    int i = is_name_in_dir(name);
    dir = (dir_entry_t *)(BUFFER + (i + 2) * sizeof(dir_entry_t));
    if (i == -1 )
    {
        kprintf("No such file or directory.\n",name);
        return -1; // failure
    }
    else if(dir->type != TYPE_DIR)
    {
       kprintf("failed to remove '%s' : Not a directory\n",name);
       return;
    }
    else 
    {
        // modify current dir inode
        current_dir_entry.fsize -= sizeof(dir_entry_t);
        current_dir_entry.fnum -= 1;
        current_dir_entry.link_soft -= 1;
        current_dir_entry.links_cnt -= 1;
        memcpy((uint8_t *)&inode_buffer, (uint8_t *)&current_dir_entry, sizeof(inode_entry_t));
        write_inode(current_dir_entry.id);

        // free rm dir inode and block
        if(current_dir_entry.links_cnt == 0)
        {
        read_inode(dir->id);
        free_block(inode_buffer.direct_table[0]); // TODO: more block
        free_inode(dir->id);
        }

        // modify current dir block
        read_block(current_dir_entry.direct_table[0]);
        bzero(dir, sizeof(dir_entry_t));
        if (i != current_dir_entry.fnum)
        {
            memcpy((char *)dir, (char *)(BUFFER + (current_dir_entry.fnum + 2) * sizeof(dir_entry_t)), sizeof(dir_entry_t));
        }
        write_block(current_dir_entry.direct_table[0]);
    }
    return 0;
}


int rm(char *name)
{
    dir_entry_t *dir;
    int i = is_name_in_dir(name);
    dir = (dir_entry_t *)(BUFFER + (i + 2) * sizeof(dir_entry_t));
    if (i == -1)
    {
        kprintf("No such file or directory\n",name);
        return -1; // failure
    }
    else if(dir->type == TYPE_DIR)
    {
       kprintf("cannot remove '%s' : Is a directory\n",name);
       return;
    }
    else
    {
        // modify current dir inode
        current_dir_entry.fsize -= sizeof(dir_entry_t);
        current_dir_entry.fnum -= 1;
        current_dir_entry.link_soft -= 1;
        current_dir_entry.links_cnt -= 1;
        memcpy((uint8_t *)&inode_buffer, (uint8_t *)&current_dir_entry, sizeof(inode_entry_t));
        write_inode(current_dir_entry.id);

        // free rm dir inode and block
        if(current_dir_entry.links_cnt == 0)
        {
        read_inode(dir->id);
        free_block(inode_buffer.direct_table[0]); // TODO: more block
        free_inode(dir->id);
        }

        // modify current dir block
        read_block(current_dir_entry.direct_table[0]);
        bzero(dir, sizeof(dir_entry_t));
        if (i != current_dir_entry.fnum)
        {
            memcpy((char *)dir, (char *)(BUFFER + (current_dir_entry.fnum + 2) * sizeof(dir_entry_t)), sizeof(dir_entry_t));
        }
        write_block(current_dir_entry.direct_table[0]);
    }
    return 0;
}

int mknod(char *name)
{
int id = is_name_in_dir(name);
    if (id == -1)
    {
        id = init_file(name, O_RDWR);
        return 0;
    }
    else
    {
        kprintf("File: %s has existed\n", name);
        return -1; 
    }
}

int init_file(char *name, uint32_t access)
{
    // TODO: check name
    kprintf("Initializing file: %s...\n", name);

    uint32_t id = alloc_inode();
    uint32_t block = alloc_block();
    init_inode(block, current_dir_entry.id, id, O_RDWR, TYPE_FILE);

    // modify current dir block
    read_block(current_dir_entry.direct_table[0]);
    dir_entry_t *dir = (dir_entry_t *)(BUFFER + (current_dir_entry.fnum + 2) * sizeof(dir_entry_t));
    dir->id = id;
    dir->type = TYPE_FILE;
    dir->mode = access;
    strcpy(dir->name, name);
    write_block(current_dir_entry.direct_table[0]);

    // modify current dir entry
    current_dir_entry.fsize += sizeof(dir_entry_t);
    current_dir_entry.fnum += 1;
    current_dir_entry.link_soft += 1;
    current_dir_entry.links_cnt += 1;
    memcpy((uint8_t *)&inode_buffer, (uint8_t *)&current_dir_entry, sizeof(inode_entry_t));
    write_inode(current_dir_entry.id);
    return id;
}



int open(char *name, uint32_t access)
{
   int id = is_name_in_dir(name);
    if (id == -1)
    {
        kprintf("File: %s not found, creating...\n", name);
        id = init_file(name, access);
    }
    else
    {
        dir_entry_t *dir = (dir_entry_t *)(BUFFER + (id + 2) * sizeof(dir_entry_t));
        id = dir->id;
    }
    kprintf("Opening file: %s (id = %d)...\n", name, id);
    read_inode(id);
    fds[num_open_files].inode_id = id;
    fds[num_open_files].mode = inode_buffer.mode;
    fds[num_open_files].r_offset = 0;
    fds[num_open_files].w_offset = 0;
    return num_open_files++;
}

int write(uint32_t fd, char *buff, uint32_t size,uint32_t w)
{
    read_inode(fds[fd].inode_id);

    int block_id;
    int write_offset;
    int write_size;
    uint32_t begin_block;
    char * p_block;
    uint32_t * id_block;

    while (size > 0)
    {
        begin_block = fds[fd].w_offset / block_SIZE ;
        if (fds[fd].w_offset <= MAX_DIRECT_NUM * block_SIZE)
        {
            block_id = inode_buffer.direct_table[begin_block];
            if (block_id == 0) {
                block_id = alloc_block();
                inode_buffer.direct_table[begin_block] = block_id;
            }
            read_block(block_id);
        }
        else if (fds[fd].w_offset <= MAX_DIRECT_NUM * block_SIZE + (block_SIZE / sizeof(uint32_t)) * block_SIZE)
        {
            block_id = inode_buffer.indirect_1_ptr;
            if (block_id == 0) {
                block_id = alloc_block();
                inode_buffer.indirect_1_ptr = block_id;
                read_block(block_id);
                bzero((void *)BUFFER, block_SIZE);
                write_block(block_id);
            }
            read_block(block_id);
            id_block = (uint32_t *)(BUFFER + (begin_block - MAX_DIRECT_NUM) * sizeof(uint32_t));
            // if (*id_block == 0) {
                int temp = alloc_block();
                *id_block = temp;
                write_block(block_id);
                block_id = temp;
            read_block(block_id);
        }
        else if (fds[fd].w_offset <= MAX_DIRECT_NUM * block_SIZE + (block_SIZE / sizeof(uint32_t)) * block_SIZE + (block_SIZE / sizeof(uint32_t)) * (block_SIZE / sizeof(uint32_t)) * block_SIZE)
        {
            block_id = inode_buffer.indirect_2_ptr;
            if (block_id == 0) {
               block_id = alloc_block();
                inode_buffer.indirect_2_ptr = block_id;
                read_block(block_id);
                bzero((void *)BUFFER, block_SIZE);
                write_block(block_id);
            }
            read_block(block_id);
            id_block = (uint32_t *)(BUFFER + (begin_block - MAX_DIRECT_NUM - (block_SIZE / sizeof(uint32_t))) / (block_SIZE / sizeof(uint32_t))  * sizeof(uint32_t));
            if (*id_block == 0) {
                int temp = alloc_block();
                *id_block = temp;
                write_block(block_id);
                block_id = temp;
            }
            else {
                block_id = *id_block;
            }
            read_block(block_id);
            id_block = (uint32_t *)(BUFFER + ((begin_block - MAX_DIRECT_NUM - (block_SIZE / sizeof(uint32_t))) % (block_SIZE / sizeof(uint32_t))) * sizeof(uint32_t));
            if (*id_block == 0) {
                int temp = alloc_block();
                *id_block = temp;
                write_block(block_id);
                block_id = temp;
            }
            else {
                block_id = *id_block;
            }
            read_block(block_id);
        }
        else
        {
            kprintf("Too large size. Not supported.\n");
        }
        write_offset = fds[fd].w_offset - (begin_block -w) * block_SIZE;
        write_size = ((block_SIZE - write_offset) > size) ? size : (block_SIZE - write_offset);

        p_block = (char *)(BUFFER + write_offset);
        memcpy(p_block, buff, write_size);
        write_block(block_id);
        buff += write_size;
        fds[fd].w_offset += write_size+w*block_SIZE;
        if (inode_buffer.fsize < fds[fd].w_offset)
        {
            inode_buffer.fsize = fds[fd].w_offset;
        }
        size -= write_size;
    }

    write_inode(fds[fd].inode_id);
    return 0;

    
}

int read(uint32_t fd, char *buff, uint32_t size,uint32_t r)
{
    read_inode(fds[fd].inode_id);
    int end = fds[fd].r_offset + size;
    if (end > inode_buffer.fsize)
    {
        kprintf("Read offset(0x%x) is out of fsize(0x%x)!\n", end, inode_buffer.fsize);
        return -1; // failure
    }


    int block_id;
    int read_offset;
    int read_size;
    uint32_t begin_block;
    char * p_block;
    uint32_t * id_block;
    while (size > 0)
    {
        begin_block = fds[fd].r_offset / block_SIZE ;
        if (fds[fd].r_offset <= MAX_DIRECT_NUM * block_SIZE)
        {
            block_id = inode_buffer.direct_table[begin_block];
            read_block(block_id);
        }
        else if (fds[fd].r_offset <= MAX_DIRECT_NUM * block_SIZE + (block_SIZE / sizeof(uint32_t)) * block_SIZE)
        {
            block_id = inode_buffer.indirect_1_ptr;
            read_block(block_id);
            id_block = (uint32_t *)(BUFFER + (begin_block - MAX_DIRECT_NUM) * sizeof(uint32_t));
            block_id = *id_block;
            read_block(block_id);
        }
        else if (fds[fd].r_offset <= MAX_DIRECT_NUM * block_SIZE + (block_SIZE / sizeof(uint32_t)) * block_SIZE + (block_SIZE / sizeof(uint32_t)) * (block_SIZE / sizeof(uint32_t)) * block_SIZE)
        {
            block_id = inode_buffer.indirect_2_ptr;
            read_block(block_id);
            id_block = (uint32_t *)(BUFFER + (begin_block - MAX_DIRECT_NUM - (block_SIZE / sizeof(uint32_t))) / (block_SIZE / sizeof(uint32_t)) * sizeof(uint32_t));
            block_id = *id_block;
            read_block(block_id);
            id_block = (uint32_t *)(BUFFER + ((begin_block - MAX_DIRECT_NUM - (block_SIZE / sizeof(uint32_t))) % (block_SIZE / sizeof(uint32_t))) * sizeof(uint32_t));
            block_id = *id_block;
            read_block(block_id);
        }
        else
        {
            kprintf("Too large size. Not supported.\n");
        }
        read_offset = fds[fd].r_offset - begin_block * block_SIZE;
        read_size = ((block_SIZE - read_offset) > size) ? size : (block_SIZE - read_offset);


        p_block = (char *)(BUFFER + read_offset);
        memcpy(buff, p_block, read_size);
        buff += read_size;
        fds[fd].r_offset += read_size+r*block_SIZE;
        size -= read_size;
    }

    write_inode(fds[fd].inode_id);
    return 0;
}

int close(uint32_t fd)
{
   num_open_files--;
    if (fd == num_open_files)
    {
        bzero(&fds[fd], sizeof(fd_t));
    }
    else
    {
        memcpy((char *)&fds[fd], (char *)&fds[num_open_files], sizeof(fd_t));
    }
    return 0;
}



int cat(char *name)
{
 int id = is_name_in_dir(name);
    if (id == -1)
    {
        kprintf("File: %s not found\n", name);
        return -1; // open failure
    }
    else
    {
        dir_entry_t *dir = (dir_entry_t *)(BUFFER + (id + 2) * sizeof(dir_entry_t));
        id = dir->id;
        if (dir->type == TYPE_SLINK) {
            read_inode(id);
            read_block(inode_buffer.direct_table[0]);
            uint32_t *p_data = (uint32_t *)BUFFER;
            id = *p_data;
            if(current_dir_entry.link_soft == 0)
            {
               kprintf("No Such SoftLink.\n");
               return;
            }  
        }
        else if (dir->type != TYPE_FILE)
        {
            kprintf("%s is not a file, but a dir\n", name);
            return -1; // failure
        }
    }
    read_inode(id);
    read_block(inode_buffer.direct_table[0]);
    char *data_block = (char *)BUFFER;
    int i;
    for (i = 0; i < inode_buffer.fsize; i++)
    {
        kprintf("%c", data_block[i]);
        if (i > block_SIZE) {
            break;
        }
    }
    return 0;
}

int do_link(char *src, char *dest, uint32_t soft)
{
 int src_id = is_name_in_dir(src);
    if (src_id == -1)
    {
        kprintf("File: %s not found\n", src);
        return -1; // open failure
    }
    read_block(current_dir_entry.direct_table[0]);
    dir_entry_t *src_dir = (dir_entry_t *)(BUFFER + (src_id + 2) * sizeof(dir_entry_t));
    int src_dir_id = src_dir->id;
    
    if (soft) {
        uint32_t id = alloc_inode();
        uint32_t block = alloc_block();
        init_inode(block, current_dir_entry.id, id, O_RDWR, TYPE_FILE);

        // modify current dir block
        dir_entry_t *dir = (dir_entry_t *)(BUFFER + (current_dir_entry.fnum + 2) * sizeof(dir_entry_t));
        read_block(current_dir_entry.direct_table[0]);
        dir->id = id;
        dir->type = TYPE_SLINK;
        dir->mode = src_dir->mode;
        strcpy(dir->name, dest);
        write_block(current_dir_entry.direct_table[0]);

        // modify current dir entry
        current_dir_entry.fsize += sizeof(dir_entry_t);
        current_dir_entry.fnum += 1;
        // printk("current id = %d inode.fnum = %d\n\r", current_dir_entry.id, current_dir_entry.fnum);
        memcpy((uint8_t *)&inode_buffer, (uint8_t *)&current_dir_entry, sizeof(inode_entry_t));
        write_inode(current_dir_entry.id);
        
        read_block(block);
        uint32_t *p_data = (uint32_t *)BUFFER;
        *p_data = src_dir_id;
        write_block(block);
        return 0;
    }
    else {
       if(src_dir->type == TYPE_DIR)
    {
       kprintf("ln : %s : hard link not allowed for directory\n",src_dir->name);
       return;
       }
        // modify current dir block
        dir_entry_t *dir = (dir_entry_t *)(BUFFER + (current_dir_entry.fnum + 2) * sizeof(dir_entry_t));
        dir->id = src_dir_id;
        dir->type = src_dir->type;
        dir->mode = src_dir->mode;
        strcpy(dir->name, dest);
        write_block(current_dir_entry.direct_table[0]);

        // modify current dir entry
        current_dir_entry.fsize += sizeof(dir_entry_t);
        current_dir_entry.fnum += 1;
        current_dir_entry.links_cnt += 1;
        // printk("current id = %d inode.fnum = %d\n\r", current_dir_entry.id, current_dir_entry.fnum);
        memcpy((uint8_t *)&inode_buffer, (uint8_t *)&current_dir_entry, sizeof(inode_entry_t));
        write_inode(current_dir_entry.id);
        return 0;
    }
}

void read_super_block()
{
    sd_card_read(superblock, OFFSET_FS, block_SIZE);//read superblock
    if(superblock->magic == KFS_MAGIC)
    {    
      read_inode(0);
      memcpy((uint8_t *)&current_dir_entry, (uint8_t *)&inode_buffer, sizeof(inode_entry_t));
    }
    else
      init_fs();

}

void print_fs_info()
{
    sd_card_read(superblock, OFFSET_FS, block_SIZE);//read superblock

    kprintf("magic : 0x%x(KFS)\n", superblock->magic);
    kprintf("used block : %d/%d , start block : %d(0x%x)\n", superblock->used_block,superblock->start_block/block_SIZE,superblock->start_block/block_SIZE,OFFSET_FS);
    kprintf("block map offset : %d , occupied block : %d\n", (superblock->block_map_offset-FS_SIZE)/block_SIZE,superblock->block_map/block_SIZE);
    kprintf("inode map offset : %d , occupied block : %d \n",(superblock->inode_map_offset-FS_SIZE)/block_SIZE,superblock->inode_map/block_SIZE);
    kprintf("inode offset : %d , occupied block : %d\n", (superblock->inode_offset-FS_SIZE)/block_SIZE, superblock->inode_num/block_SIZE);
    kprintf("data offset : %d , occupied block : %d\n", (superblock->data_offset-FS_SIZE)/block_SIZE,superblock->data_num/block_SIZE);
    kprintf("inode entry size : %dB, dir entry size : %dB\n", superblock->inode_size, superblock->dir_size);

}

void init_fs()
{
    kprintf("[FS] Starting initialize filesystem!     \n");

    // init superblock
    init_superblock();
    kprintf("[FS] Setting superblock...\n");
    kprintf("     magic : 0x%x\n",superblock->magic);
    kprintf("     number block :  %d,start block : %d\n",superblock->block_number,superblock->start_block/block_SIZE);
    kprintf("     block map offset : %d (%d)\n",(superblock->block_map_offset-FS_SIZE)/block_SIZE,superblock->block_map/block_SIZE);
    kprintf("     inode map offset : %d (%d)\n",(superblock->inode_map_offset-FS_SIZE)/block_SIZE,superblock->inode_map/block_SIZE);
    kprintf("     inode offset : %d (%d)\n",(superblock->inode_offset-FS_SIZE)/block_SIZE,superblock->inode_num/block_SIZE);
    kprintf("     data offset : %d (%d)\n",(superblock->data_offset-FS_SIZE)/block_SIZE,superblock->data_num/block_SIZE);
    kprintf("     inode entry size : 128B, dir entry size : 32B\n");


    // init block bitmap
    kprintf("[FS] Setting inode-map...  \n");
    init_inodemap();

    // init inode bitmap
    kprintf("[FS] Setting block-map...              \n");
    init_blockmap();

    // init root directory
    kprintf("[FS] Setting inode...                  \n");
    init_rootdir();

    kprintf("[FS] Initializing filesystem finished!   \n");

}
