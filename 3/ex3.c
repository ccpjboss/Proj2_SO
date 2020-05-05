#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Isto test
void *server(void *arg);

int main(int argc, char const *argv[])
{
    pthread_t thread;
    int arr[2];
    int *res=(int *)malloc(sizeof(int)*2);

    for (int i = 0; i < 2; i++)
    {
        printf("Insert a number: ");
        scanf("%d", &arr[i]);
    }

    pthread_create(&thread, NULL, server, &arr);
    pthread_join(thread,(void **)&res);
    printf("%d %d\n",res[0],res[1]);
    return 0;
}

void *server(void *arg)
{
    int *n = (int *)arg;
    int *results = (int *)malloc(2 * sizeof(int));

    results[0] = n[0] + n[1];
    results[1] = n[0] * n[1];
    printf("%d %d\n",results[0],results[1]);

    pthread_exit(results);
}
