#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
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
    double value_read = 0;
    int c =0;
    sem_t *sem_id_read = sem_open(SEM_NAME,O_CREAT,0666,0);
    sem_t *sem_id_write = sem_open(SEM_NAME2,O_CREAT,0666,1);

    if (sem_id_read == SEM_FAILED || sem_id_write == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    /* File */
    char *file_name = "input.bin";
    FILE *file = fopen(file_name, "wb");
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
        sem_wait(sem_id_read);
        memcpy(&value_read, shrmem, sizeof(double));
        sem_post(sem_id_write);
        if (value_read == DBL_MIN)
            break;
        printf("%lf\n", value_read);
        fwrite(&value_read, sizeof(double), 1, file);
        c=c+1;
    }
    printf("DONE READING SHARED MEMORY!\n");
    shm_unlink(SHRMEM);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_NAME2);
    fclose(file);
     
    double array[c];
    FILE* file2 = fopen(file_name,"rb");
    FILE* file3 = fopen("output.asc","w");

    if (file2 == NULL)
    {
        perror("file read");
        exit(1);
    }

    if(file3 == NULL)
    {
        perror("file3");
        exit(1);
    }
    char value_to_write [50];
    fread(array,sizeof(double),c,file2) ;
    fclose(file2);

    for (int i = 0; i < c; i++)
    {
        array[i]=array[i]*4.0;
        snprintf(value_to_write,50,"%lf \n",array[i]);
        fprintf(file3,"%s",value_to_write);
    }
    fclose(file3);
    
    return 0;
}
