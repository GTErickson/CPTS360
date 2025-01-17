/** C4.1.c file: compute matrix sum by threads */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define N 8

int A[N][N], sum[N];

int total = 0; //Global total
pthread_mutex_t *m; //mutex ptr


void func(void *arg) // threads function
{
    int j, row, sum = 0;
    pthread_t tid = pthread_self(); // get thread ID number
    row = (int)arg; // get row number from arg
    printf("Thread %d [%lu] computes sum of row %d\n", row, tid, row);
    for (j=0; j<N; j++) // compute sum of A[row]in global sum[row]
    {
        sum += A[row][j]; //Compute partial sums
    }
    printf("Thread %d [%lu] update total with %d : ",row, tid, sum);
    pthread_mutex_lock(m);
        total += sum;
    pthread_mutex_unlock(m);
    printf("total = %d\n", total);
    
    
    pthread_exit((void*)0); // thread exit: 0=normal termination
}

int main (int argc, char *argv[])
{
    pthread_t thread[N]; // thread IDs
    int i, j, r;
    void *status;
    printf("Main: initialize A matrix\n");
    for (i=0; i<N; i++)
    {
        sum[i] = 0;
        for (j=0; j<N; j++)
        {
            A[i][j] = i*N + j + 1;
            printf("%4d ", A[i][j]);
        }
        printf("\n");
    }
    m = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)); //Crete mutex m
    pthread_mutex_init(m, NULL); //initialize mutex m to NULL
    printf("Main: create %d threads\n", N);
    for(i=0; i<N; i++) 
    {
        pthread_create(&thread[i], NULL, func, (void *)i);
    }
    printf("Main: try to join with threads\n");
    for(i=0; i<N; i++) 
    {
        pthread_join(thread[i], &status);
        printf("Main: joined with %d [%lu]: status=%d\n",i, thread[i], (int)status);
    }
    printf("Main: total = %d\n", total); //print total
    pthread_mutex_destroy(m); //destroy m
    pthread_exit(NULL);
}


// Main: initialize A matrix
//    1    2    3    4    5    6    7    8 
//    9   10   11   12   13   14   15   16 
//   17   18   19   20   21   22   23   24 
//   25   26   27   28   29   30   31   32 
//   33   34   35   36   37   38   39   40 
//   41   42   43   44   45   46   47   48 
//   49   50   51   52   53   54   55   56 
//   57   58   59   60   61   62   63   64 
// Main: create 8 threads
// Main: try to join with threads
// Thread 2 [140708055250688] computes sum of row 2
// Thread 2 [140708055250688] done: sum[2] = 164
// Thread 3 [140708046857984] computes sum of row 3
// Thread 3 [140708046857984] done: sum[3] = 228
// Thread 4 [140708038465280] computes sum of row 4
// Thread 4 [140708038465280] done: sum[4] = 292
// Thread 5 [140708030072576] computes sum of row 5
// Thread 5 [140708030072576] done: sum[5] = 356
// Thread 6 [140708021679872] computes sum of row 6
// Thread 6 [140708021679872] done: sum[6] = 420
// Thread 7 [140708013287168] computes sum of row 7
// Thread 7 [140708013287168] done: sum[7] = 484
// Thread 1 [140708063643392] computes sum of row 1
// Thread 1 [140708063643392] done: sum[1] = 100
// Thread 0 [140708072036096] computes sum of row 0
// Thread 0 [140708072036096] done: sum[0] = 36
// Main: joined with 0 [140708072036096]: status=0
// Main: joined with 1 [140708063643392]: status=0
// Main: joined with 2 [140708055250688]: status=0
// Main: joined with 3 [140708046857984]: status=0
// Main: joined with 4 [140708038465280]: status=0
// Main: joined with 5 [140708030072576]: status=0
// Main: joined with 6 [140708021679872]: status=0
// Main: joined with 7 [140708013287168]: status=0
// Main: compute and print total sum: tatal = 2080