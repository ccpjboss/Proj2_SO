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

#define M 40
#define N 4

/* message structure */
struct message {
    long mtype;
    char mtext[8192];
};

void producer();

int main(int argc, char const *argv[])
{
    /* Creation of the message queues */
    int msq1 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    int msq2 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);

    /* Check for creation errors */
    if (msq1 == -1 || msq2 == -1)
    {
        perror("Message queues creation error");
        return EXIT_FAILURE;
    }

    return 0;
}

void producer()
{
    int upper = 49;
    int lower = 1;
    int num = (rand() % (upper - lower + 1)) + lower;
}