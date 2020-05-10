#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define SHRMEM "/mem_example"

int main(int argc, char const *argv[])
{
    /* SHARED MEMORY */
    int shrmem_fd;
    double* shrmem;
    double value_read;

    /* File */
    char* file_name = "input.bin";
    FILE *file = fopen(file_name, "wb");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    // Create the shared memory object
    if ((shrmem_fd=shm_open(SHRMEM,O_RDONLY | O_CREAT,0)) == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // Map the shared memory
    if ((shrmem =mmap(NULL,sizeof(double), PROT_READ,MAP_SHARED,shrmem_fd,0)) == MAP_FAILED) 
    {
        perror("mmap");
        exit(1);
    }
    memcpy(&value_read, shrmem, sizeof(double));
    fwrite(&value_read,sizeof(double),1,file);
    fclose(file);

    return 0;
}
