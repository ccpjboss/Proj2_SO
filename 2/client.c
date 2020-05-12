#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <semaphore.h>

#define SHRMEM "/mem_example"
#define SEM_NAME "/sem_read"
#define SEM_NAME2 "/sem_write"

int main(int argc, char const *argv[])
{
    /* SHARED MEMORY */
    int shrmem_fd;
    double *shrmem;

    /* SEMAPHORES */
    sem_t *sem_id_read = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    sem_t *sem_id_write = sem_open(SEM_NAME2, O_CREAT, 0666, 1);

    if (sem_id_read == SEM_FAILED || sem_id_write == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }
    /* Files */
    char *file_name = "input.asc";
    double value_read;
    double final = DBL_MIN;

    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    // Create the shared memory object
    if ((shrmem_fd = shm_open(SHRMEM, O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // configure the size of the shared memory object
    ftruncate(shrmem_fd, sizeof(double));

    // Map the shared memory
    if ((shrmem = mmap(NULL, sizeof(double), PROT_WRITE | PROT_READ, MAP_SHARED, shrmem_fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    fscanf(file, "%lf", &value_read);
    while (!feof(file))
    {
        printf("%lf\n", value_read);
        sem_wait(sem_id_write);
        memcpy(shrmem, &value_read, sizeof(double));
        sem_post(sem_id_read);
        fscanf(file, "%lf", &value_read);
    }

    sem_wait(sem_id_write);
    memcpy(shrmem, &final, sizeof(final));
    sem_post(sem_id_read);

    shm_unlink(SHRMEM);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_NAME2);
    fclose(file);

    return 0;
}
