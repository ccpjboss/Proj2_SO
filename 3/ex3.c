#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *server(void *arg);

int main(int argc, char const *argv[])
{
    pthread_t thread;
    int arr[2];                                //int array to send to the thread
    int *res = (int *)malloc(sizeof(int) * 2); //Allocate space for the result of the thread

    /* Ask for input */
    for (int i = 0; i < 2; i++)
    {
        printf("Insert a number: ");
        scanf("%d", &arr[i]); //Store it in the array
    }

    pthread_create(&thread, NULL, server, &arr); //Creates the thread and send the array as argument and the handle funtion is the server funtion
    pthread_join(thread, (void **)&res);         //Returns the values and stores it the res array that needs to be cast to a void pointer
    printf("The result of the sum: %d \nResult of the product: %d\n", res[0], res[1]);

    return 0;
}

void *server(void *arg) //Thread funtions can only received void * arguments
{
    int *n = (int *)arg; //Cast void* to int*
    int *results = (int *)malloc(2 * sizeof(int)); //Allocate space for the result 

    /* Do the operations */
    results[0] = n[0] + n[1];
    results[1] = n[0] * n[1];

    pthread_exit(results); //And the result gets returned to the main thread
}
