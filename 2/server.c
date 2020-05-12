#include <sys/mman.h> //Shared memory
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <float.h>     //DBL_MIN
#include <semaphore.h> //Library for the semaphores
#include <signal.h>

#define SHRMEM "/mem_example"  //Path to named shared memory
#define SEM_NAME "/sem_read"   //Path to the semaphore read
#define SEM_NAME2 "/sem_write" //Path to the semaphore write

void sighandler(int signum);

int main(int argc, char const *argv[])
{
    int shrmem_fd;          //File descriptor for the shared memory
    double *shrmem;         //Double pointer to the shared memory segment
    double value_read = 0;  //Variable to store the double read from the shared memory
    int c = 0;              //Counter to count the number of doubles read from the shared memory
    char double_to_str[50]; //This variable is used to convert double to ASCII to store in output.asc

    struct sigaction sig;
    sig.sa_handler = sighandler; /* Handle function to run when a signal is received */

    if (sigaction(SIGTERM, &sig, NULL) == -1) /* Associate SIGHUP with sigaction struct sig*/
        perror("SIGACTION--SIGTERM");

    /* Creates the two semaphores */
    sem_t *sem_id_read = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    sem_t *sem_id_write = sem_open(SEM_NAME2, O_CREAT, 0666, 1); //This semaphores is initialized with 1 because we want to write first

    /* Check for error in opening semaphores */
    if (sem_id_read == SEM_FAILED || sem_id_write == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    /* Opens the input.bin file for binary writing */
    FILE *file = fopen("input.bin", "wb");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    // Create the shared memory object
    if ((shrmem_fd = shm_open(SHRMEM, O_RDONLY, 0666)) == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // Map the shared memory
    if ((shrmem = mmap(NULL, sizeof(double), PROT_READ, MAP_SHARED, shrmem_fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    while (1)
    {
        sem_wait(sem_id_read);                       //Locks the semaphore for reading
        memcpy(&value_read, shrmem, sizeof(double)); //Reads the double varaible in the shared memory
        sem_post(sem_id_write);                      //Unlocks/Signals the semaphores for writing

        if (value_read == DBL_MIN) //If the DBL_MIN value is read then break the while(1)
            break;

        printf("%lf\n", value_read);                  /*DEBUG*/
        fwrite(&value_read, sizeof(double), 1, file); //Write to the input.bin file
        c = c + 1;                                    //Increments the counter of values read
    }
    printf("DONE READING SHARED MEMORY!\n"); //DEBUG

    shm_unlink(SHRMEM);   //Unlinks the shared memory object
    sem_unlink(SEM_NAME); //Unlinks the semaphore
    sem_unlink(SEM_NAME2);

    fclose(file); //Closes the file

    double array[c]; //This array is to store the values read from the input.bin file

    /* Opens the file input.bin for binary reading */
    FILE *file2 = fopen("input.bin", "rb");
    if (file2 == NULL)
    {
        perror("file read");
        exit(1);
    }

    /* Opens the output.asc for writing */
    FILE *file3 = fopen("output.asc", "w");
    if (file3 == NULL)
    {
        perror("file3");
        exit(1);
    }

    fread(array, sizeof(double), c, file2); // Read the input.bin file and stores the values in the array, only using one read
    fclose(file2);                          //Closes the file

    for (int i = 0; i < c; i++)
    {
        array[i] = array[i] * 4.0;                       //Multiply the values by 4.0
        printf("%lf\n",array[i]);
        snprintf(double_to_str, 50, "%lf \n", array[i]); //Convert the double values to ascii
        fprintf(file3, "%s", double_to_str);             //And stores it in the output.asc
    }

    fclose(file3);

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