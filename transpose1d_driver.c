/* test driver program for 1D matrix transpose */
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

extern int transpose1d(int *a, int n, int blockdim, MPI_Comm comm);

int main (int argc, char *argv[]) {
    int   numtasks, taskid;
    int i, j, n, nlocal; 
    int *alocal;
    int offset;
    int checkrank = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);

    if (argc != 2) {
        if (taskid == 0) {
            fprintf(stderr, "Usage: %s <n>\n", argv[0]);
            fprintf(stderr, "where n is a multiple of the number of tasks.\n");
        }   
        MPI_Finalize();
        exit(0);
    }

    /* Read row/column dimension from command line */
    n = atoi(argv[1]);

    if (n%numtasks) {
        if (taskid == 0) {
            fprintf(stderr, "Usage: %s <n>\n", argv[0]);
            fprintf(stderr, "where n is a multiple of the number of tasks.\n");
        }   
        MPI_Finalize();
        exit(0);
    }

    nlocal = n/numtasks;
    // nlocal = n;

    /* Allocate local row block */
    alocal = (int *) malloc(n*nlocal*sizeof(int));

    /* Initialize local block */
    offset = taskid*n*nlocal;
    for (i = 0; i < nlocal; i++)
        for (j = 0; j < n; j++)
            alocal[i*n + j] = offset + i*n + j + 1;

    if (taskid == checkrank) {
        printf("process %d local block before transpose\n", taskid);
        for (i = 0; i < nlocal; i++) {
            for (j = 0; j < n; j++) 
                    printf("%10d ", alocal[i*n + j]);
            printf("\n");
        }
    }

    transpose1d(alocal, n, nlocal, MPI_COMM_WORLD);

    if (taskid == checkrank) {
        printf("process %d local block after transpose\n", taskid);
        for (i = 0; i < nlocal; i++) {
            for (j = 0; j < n; j++) 
                    printf("%10d ", alocal[i*n + j]);
            printf("\n");
        }
        // for (i = 0; i < n; i++) {
        //     for (j = 0; j < nlocal; j++) 
        //             printf("%10d ", alocal[i*nlocal + j]);
        //     printf("\n");
        // }

        // printf("\n\n%d", nlocal);
        // printf("\n%d\n\n", n);
    }

    MPI_Finalize();
    return(0);
}

int transpose1d(int *a, int n, int blockdim, MPI_Comm comm) {

    int numtasks, taskid;
    int temp = 0; // will hold the temporary matrix value
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Request request;
    MPI_Status status;

    // n == arg 0
    // num tasks = processors
    // blockdim = number of rows per task = num tasks / processors


    // Scatter in parts
    // int i, j, k, count;
    int i, j, k, count;
    for(i = 0; i < blockdim; i++ ) {
        for(j = taskid * blockdim, count = 0; count < n; count ++ ) {
            // index = i*n + j
            // printf("%d: %d %d %d %d\n", taskid, j/blockdim, 0, j*blockdim, a[i*blockdim + j]);
            MPI_Irecv( &temp, 1, MPI_INT, MPI_ANY_SOURCE, taskid, comm, &request);
            MPI_Send(a + (i*n + j), 1, MPI_INT, j/blockdim, j/blockdim, comm);
            // printf("%d: sent \n", taskid);
            // printf("%d: waiting to recieve %d\n", taskid, count);
            MPI_Wait( &request, &status );
            // printf("%d: recieved from %d\n", taskid, status.MPI_SOURCE);
            a[status.MPI_SOURCE*blockdim + i*n + j%blockdim] = temp;
            j=(j+1)%n;
        }

        MPI_Barrier(comm);
    }

    // do mini transpose in ever blockdim by blockdim square
    // every blockdim square
    for( i = 0; i < numtasks; i++ ) {
        // every row
        for( j = 0; j < blockdim; j++ ) {
            for( k = 0; k < j; k++ ) {
                temp = a[j*n + i*blockdim + k];
                a[j*n + i*blockdim + k] = a[k*n + i*blockdim + j];
                a[k*n + i*blockdim + j] = temp;
            }
        }
    }
    
    // just a check to see if it worked
    // if( taskid == 1 ) {
    //     for (i = 0; i < blockdim; i++) {
    //         for (j = 0; j < n; j++)
    //             printf(" i: %d", alocal[i*n + j]);
    //         printf("\n");
    //     }
    // }

    // transpose arrays
    // for(i = 0; i < blockdim; i++ ) {
    //     for(j = 0; j < n; j+= blockdim ) {
    //         a[i*n + j] = alocal[i*n + j];
    //     }
    // }

    // if( taskid == 1 ) {
    //     for (i = 0; i < blockdim; i++) {
    //         for (j = 0; j < n; j++)
    //             printf(" i: %d", a[i*n + j]);
    //         printf("\n");
    //     }
    // }

    // send out finished array
    // MPI_Gather( a, int send_count, MPI_Datatype send_datatype, 
    //     void* recv_data, int recv_count, MPI_Datatype recv_datatype, int root, MPI_Comm communicator);

    // printf("HI %d %d %d %d\n", n, blockdim, numtasks, taskid);
    return(0);
}
 

