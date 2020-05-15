#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

#define M 40 /*number of initial numbers to send to msq1 */
#define N 4  /*number of consumers */

/* message structure */
struct message
{
    long mtype;
    int number; //key
};

int msq1, msq2; //message queues id

void producer(int msq1, int msq2);        //Producer funtion
void consumer(int n, int msq1, int msq2); //Consumer funtion
void sighandler(int signum);              //Signal handler

int main(int argc, char const *argv[])
{

    struct sigaction sig;
    sig.sa_handler = sighandler; /* Handle function to run when a signal is received */

    if (sigaction(SIGTERM, &sig, NULL) == -1) /* Associate SIGHUP with sigaction struct sig*/
        perror("SIGACTION--SIGTERM");

    /* Creation of the message queues */
    msq1 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    msq2 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);

    /* Check for creation errors on msq1 creation*/
    if (msq1 == -1)
    {
        perror("Message queues creation error");
        exit(1);
    }

    /* Check for creation errors for msq2 creation */
    if (msq2 == -1)
    {
        perror("Message queues creation error");
        exit(1);
    }

    for (int i = 0; i < N; i++)
    {
        if (fork() == 0) //Fork the main process
        {
            /* CHILD */
            consumer(i, msq1, msq2);
            exit(0); /* Child exit success*/
        }
    }
    /* PARRENT */
    producer(msq1, msq2);

    for (int i = 0; i < N; i++)
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
                if (key[j] == msg.number)
                {
                    repeat = true;
                }
            }

            if (repeat == true)
            {
                i = i - 1;
                repeat = false;
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

void sighandler(int signum)
{
    if (signum == SIGTERM)
    {
        printf("SIGTERM received realeasing memory\n");

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
    }
}