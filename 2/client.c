//ALWAYS RUN THE CLIENT FIRST
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
#include <signal.h>

#define SHRMEM "/mem_example"
#define SEM_NAME "/sem_read"
#define SEM_NAME2 "/sem_write"

void sighandler(int signum);

int main(int argc, char const *argv[])
{
    int shrmem_fd;          //File descriptor for the shared memory
    double *shrmem;         //Pointer to the shared memory segment
    double value_read;      //Value read from input.asc
    double final = DBL_MIN; //Last value to send

    struct sigaction sig;
    sig.sa_handler = sighandler; /* Handle function to run when a signal is received */

    if (sigaction(SIGTERM, &sig, NULL) == -1) /* Associate SIGHUP with sigaction struct sig*/
        perror("SIGACTION--SIGTERM");
    /* Creates the semaphores */
    sem_t *sem_id_read = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    sem_t *sem_id_write = sem_open(SEM_NAME2, O_CREAT, 0666, 1);

    /* Checks for error */
    if (sem_id_read == SEM_FAILED || sem_id_write == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    /* Opens the input.asc for reading */
    FILE *file = fopen("input.asc", "r");
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

    /* Reads the input.asc until EOF is hit */
    fscanf(file, "%lf", &value_read);
    while (!feof(file))
    {
        printf("%lf\n", value_read); /* DEBUG */

        sem_wait(sem_id_write);                      //Locks the semaphore for writing
        memcpy(shrmem, &value_read, sizeof(double)); //Copies the value read from the file to the shared memory
        sem_post(sem_id_read);                       //Unlocks/Signals the semaphores for reading

        fscanf(file, "%lf", &value_read);
    }

    // Sending the DBL_MIN value
    sem_wait(sem_id_write);
    memcpy(shrmem, &final, sizeof(final));
    sem_post(sem_id_read);

    shm_unlink(SHRMEM);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_NAME2);

    fclose(file);

    return 0;
}

void sighandler(int signum)
{
    if (signum == SIGTERM)
    {
        printf("SIGTERM received realeasing memory\n");
        shm_unlink(SHRMEM);   //Unlinks the shared memory object
        sem_unlink(SEM_NAME); //Unlinks the semaphore
        sem_unlink(SEM_NAME2);
    }
}