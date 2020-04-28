/**
 * TODO: Create the message queues msq1 and msq2
 * TODO: Implement the code for the producer and the consumer
 *       * PRODUCER:
 *                  * genarate numbers between 1 and 49
 *                  * send values to msq1
 *       * CONSUMER: 
 *                  * Wait for message
 *                  * Process information, verify if it is repeated, if not save to file
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define M 40 /*number of initial numbers to send to msq1 */
#define N 4  /*number of consumers */

/* message structure */
struct message
{
    long mtype;
    int number;
};

void producer(int msq1, int msq2);
void consumer(int n, int msq1, int msq2);

int main(int argc, char const *argv[])
{
    /* Creation of the message queues */
    int msq1 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    int msq2 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);

    /* Check for creation errors */
    if (msq1 == -1)
    {
        perror("Message queues creation error");
        exit(1);
    }

    /* Check for creation errors */
    if (msq2 == -1)
    {
        perror("Message queues creation error");
        exit(1);
    }

    if (fork() == 0)
    {
        /* CHILD */
        for (int i = 1; i <= N; i++)
        {
            consumer(i, msq1, msq2);
        }

        exit(0); /* Child exit success*/
    }
    else
    {
        /* PARRENT */
        producer(msq1, msq2);
    }

    wait(NULL);

    /* destroy message queue */
    if ((msgctl(msq1, IPC_RMID, NULL) == -1))
    {
        perror("msgctl 1");

        exit(1);
    }

    /* destroy message queue */
    if ((msgctl(msq2, IPC_RMID, NULL) == -1))
    {
        perror("msgctl 2");

        exit(1);
    }

    exit(0);
}

void producer(int msq1, int msq2)
{
    int upper = 49;
    int lower = 1;

    struct message msg;
    msg.mtype = 23;

    struct message mayproduce;
    mayproduce.number = 1;
    mayproduce.mtype = 24;

    srand(time(0));

    /* Sends M mayproduce == 1 to msq2 */
    for (int i = 0; i < M; i++)
    {
        if (msgsnd(msq2, &mayproduce, sizeof(long) + sizeof(int), 0) == -1)
        {
            perror("msgsnd2");
            exit(1);
        }
    }

    printf("MSQ2 setup!\n");

    int c = 0;    /* counts the number of consumers that have finished */
    while (c < N) /* While the number of consumers dead is less than 4 */
    {
        if (msgrcv(msq2, &mayproduce, sizeof(long) + sizeof(int), 0, 0) == -1)
        {
            perror("msgrcv2");
            exit(1);
        }
        else
        {
            if (mayproduce.number == 1)
            {
                /* Generate random number */
                msg.number = (rand() % (upper - lower + 1)) + lower;

                /* Sends random number to msq1 */
                if (msgsnd(msq1, &msg, sizeof(long) + sizeof(int), 0) == -1)
                {
                    perror("msgsnd");
                    exit(1);
                }
                else
                {
                    printf("Producer generated a number: %d\n", msg.number);
                }
            }
            else /* if mayproduce == 0 */
            {
                c++; /* increments the counter for the finished consumer */
            }
        }
    }
    printf("All consumers are dead. Terminating...\n");
}

void consumer(int n, int msq1, int msq2)
{
    struct message msg;
    struct message mayproduce2;
    char fpath[] = "Key_";
    char ftype[] = ".txt";
    char nc[6];
    int num = 0;
    int key[] = {0, 0, 0, 0, 0, 0};
    bool repeat = false;

    sprintf(nc, "%d", n);
    strcat(nc, ftype);
    strcat(fpath, nc);

    printf("%s\n", fpath);

    FILE *fp;
    fp = fopen(fpath, "w+");

    if (fp == NULL)
    {
        printf("Error!");
        exit(1);
    }

    int i = 0;

    while (i < 6)
    {
        /* Receives product from msq1 */
        if (msgrcv(msq1, &msg, sizeof(long) + sizeof(int), 0, 0) == -1)
        {
            perror("msgrcv");

            exit(1);
        }
        else
        {
            /* Checks for repeated value */
            for (int j = 0; j < 6; j++)
            {
                printf("%d) key[j]=%d and number=%d\n",n,key[j],msg.number);
                if (key[j] == msg.number)
                {
                    repeat = true;
                }
            }

            if (repeat == true)
            {
                i = i - 1;
                repeat=false;
            }
            else
            {
                key[i] = msg.number;
                /* if the value is not repeated then write to file */
                fprintf(fp, "%d ", msg.number);
                /* and send mayproduce == 1 to msq2 */
                mayproduce2.number = 1;
                if (msgsnd(msq2, &mayproduce2, sizeof(long) + sizeof(int), 0) == -1)
                {
                    perror("msgsnd3");
                    exit(1);
                }
            }
            i = i + 1;
        }
    }

    /* Raffle key is complete */
    mayproduce2.number = 0;

    /* Send mayproduce == 0 to msq2 */
    if (msgsnd(msq2, &mayproduce2, sizeof(long) + sizeof(int), 0) == -1)
    {
        perror("mproduce0");
        exit(1);
    }

    rewind(fp);

    fscanf(fp, "%d", &num);
    while (!feof(fp))
    {
        printf("%d ", num);
        fscanf(fp, "%d", &num);
    }
    printf("\n");

    fclose(fp);
}