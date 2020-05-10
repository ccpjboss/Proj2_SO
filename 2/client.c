#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define SHRMEM "/mem_example"

int main(int argc, char const *argv[])
{
    /* SHARED MEMORY */
    int shrmem_fd;
    double *shrmem;

    /* Files */
    char *file_name = "input.asc";
    double value_read;

    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    // Create the shared memory object
    if ((shrmem_fd = shm_open(SHRMEM, O_RDWR | O_CREAT, 0)) == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // configure the size of the shared memory object
    ftruncate(shrmem_fd, sizeof(double));

    // Map the shared memory
    if ((shrmem = mmap(NULL, sizeof(double), PROT_WRITE, MAP_SHARED, shrmem_fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    fscanf(file, "%lf", &value_read);
    while (!feof(file))
    {
        printf("%lf\n", value_read);
        memcpy(shrmem,&value_read,sizeof(double));
        fscanf(file, "%lf", &value_read);
    }

    fclose(file);
    return 0;
}
