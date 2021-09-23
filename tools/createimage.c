#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 0x200
#define OS_SIZE_LOC 0x1f0
#define BOOT_LOADER_SIG_OFFSET 0x1fe
#define BOOT_LOADER_SIG_1 0x55
#define BOOT_LOADER_SIG_2 0xaa
#define BOOT_MEM_LOC 0x7c00
#define OS_MEM_LOC 0x1000

/* structure to store command line options */
static struct {
    int vm;
    int extended;
} options;


/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr *ehdr, FILE *fp);
static void read_phdr(Elf64_Phdr *phdr, FILE *fp, int ph, Elf64_Ehdr *ehdr);
static void write_segment(Elf64_Phdr *phdr, FILE *fp, FILE *img);
static void write_os_size(FILE *img);
static void write_user_thread_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE *fp, FILE *img, int *nbytes, int *first);

static int nbytes; 

int main(int argc, char* argv[]) {
    options.extended = 0;
    options.vm = 0;
    nbytes = -1;
    int cnt = 0;
    argv++;

    for (int j = 1; j < argc; j++)
    {
        if ((*argv)[0] == '-' && (*argv)[1] == '-') // right fmt options
        {
            // test option
            if ((*argv)[2] == 'e')
                options.extended = 1;
            else if ((*argv)[2] == 'v')
                options.vm = 1;
            else
            {
                printf("Error: no such Arg!\n");
                exit(1);
            }
            argv++;
            continue;
        }
        else if ((*argv)[0] == '-' || (*argv)[1] == '-') // wrong fmt options
        {
            printf("Error: wrong format for args! Example: %s\n", ARGS);
            exit(1);
        }
        // files
        cnt++;
    }

    create_image(cnt, argv);
    return 0;
}

static void create_image(int nfiles, char *files[]) {
    FILE * image_file = fopen(IMAGE_FILE, "w+");
    for (int i = 0; i < nfiles; i++) {
        FILE * fp = fopen(files[i], "r+");
        Elf64_Ehdr *file_ehdr = (Elf64_Ehdr *)malloc(sizeof(Elf64_Ehdr));
        read_ehdr(file_ehdr, fp);
        for (int ph = 0; ph < file_ehdr->e_phnum; ph++) { 
       	Elf64_Phdr *file_phdr = (Elf64_Phdr *)malloc(sizeof(Elf64_Phdr));
        read_phdr(file_phdr, fp, ph, file_ehdr);
        if (options.extended) {
                printf("0x%x: %s\n", (unsigned int)file_phdr->p_vaddr, files[i]);
                printf("\t\toffset 0x%04lx", file_phdr->p_offset);
                printf("\t\tvaddr 0x%04lx\n", file_phdr->p_vaddr);
                printf("\t\tfilesz 0x%04lx", file_phdr->p_filesz);
                printf("\t\tmemsz 0x%04lx\n", file_phdr->p_memsz);
            }
            write_segment(file_phdr, fp, image_file);
            free(file_phdr);
        }
        free(file_ehdr);
        fclose(fp);
    }
    write_os_size(image_file);
    fclose(image_file);
}

static void read_ehdr(Elf64_Ehdr *ehdr, FILE *fp) {
    fseek(fp, 0, SEEK_SET);
    fread(ehdr, sizeof(Elf64_Ehdr), 1, fp);
}

static void read_phdr(Elf64_Phdr *phdr, FILE *fp, int ph, Elf64_Ehdr *ehdr) {
    fseek(fp,  (ehdr->e_phoff + ehdr->e_phentsize * ph) * (sizeof(char)), SEEK_SET);
    fread(phdr, sizeof(Elf64_Phdr), 1, fp);
}

static void write_segment(Elf64_Phdr *phdr, FILE *fp, FILE *img) {
	  int padding = (phdr->p_memsz - 1) / SECTOR_SIZE + 1;
    nbytes += padding;
    padding *= SECTOR_SIZE;

    char * buff =  (char *)malloc(padding * (sizeof(char)));
    memset(buff, 0, padding);    
    fseek(fp, (phdr->p_offset)* (sizeof(char)), SEEK_SET);
    fread(buff, sizeof(char), phdr->p_filesz, fp);
    fwrite(buff, sizeof(char), padding, img);
}

 
static void write_os_size(FILE *img) {
    fseek(img, OS_SIZE_LOC, SEEK_SET);
    fwrite(&nbytes, sizeof(int), 1, img);
}

static void error(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errno != 0) {
        perror(NULL);
    }
    exit(EXIT_FAILURE);
}
