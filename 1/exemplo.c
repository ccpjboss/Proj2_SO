#include <stdlib.h>                                                                                          
#include <unistd.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>

/* message structure */
struct message {
    long mtype;
    int number;
};

int main(void)
{
    /* create message queue */
    int msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if (msqid == -1) {
        perror("msgget");

        return EXIT_FAILURE;
    }

    /* fork a child process */
    pid_t pid = fork();
    if (pid == 0) {
        /* child */
        int n = 5;
        struct message message;
        message.mtype = 23;
        message.number = 5;

        /* send message to queue */
        if (msgsnd(msqid, &message, sizeof(long) + sizeof(n), 0) == -1) {
            perror("msgsnd");

            return EXIT_FAILURE;
        }
    } else {
        /* parent */

        /* wait for child to finish */
        (void)waitpid(pid, NULL, 0);

        /* receive message from queue */
        struct message message;
        if (msgrcv(msqid, &message, sizeof(long) + sizeof(int), 0, 0) == -1) {
            perror("msgrcv");

            return EXIT_FAILURE;
        }

        printf("%d\n", message.number);

        /* destroy message queue */
        if (msgctl(msqid, IPC_RMID, NULL) == -1) {
            perror("msgctl");

            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}