#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char** argv)
{
    double t1, t2;
    int rank, numTasks;
    double paratime = 0;
    int N = 0;
    float pivot = 0;
    float** arr = NULL;
    float* data = NULL;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);

    t1 = MPI_Wtime();

    // Read input from file.
    if (rank == 0)
    {
        FILE* fptr;
        fptr = fopen("input.txt", "r");
        if (fptr == NULL)
        {
            fprintf(stderr, "File could not be opened");
            return -1;
        }

        fscanf(fptr, "%d", &N);

        // Make data contiguous to make it easy to broadcast.
        data = (float*)malloc(N*(N+1)*sizeof(float));
        arr = (float**)malloc(N*sizeof(float*));
        for (int i = 0; i < N; i++)
            arr[i] = &(data[(N+1)*i]);

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N+1; j++)
            {
                fscanf(fptr, "%f", &arr[i][j]);
            }
        }
    }

    // Broadcast the number of linear equations to solve.
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Start Gaussian Elimination.
    for (int i = 0; i < N; i++)
    {
        // Execute serial part of the algorithm
        if (rank == 0)
        {
            int maxIdx = i;
            float max = arr[maxIdx][i];

            // Find the maximum element to use as pivot
            for (int j = i+1; j < N; j++)
            {
                if (fabs(arr[j][i]) > max)
                {
                    max = arr[j][i];
                    maxIdx = j;
                }
            }

            // If the pivot is zero, the matrix is singular and cannot be solved.
            if (arr[i][maxIdx] == 0)
            {
                fprintf(stderr, "Singular Matrix");
                return -1;
            }

            // Swap rows so that we can have the maximum element of the column in the pivot position
            if (maxIdx != i)
            {
                for (int k = 0; k < N+1; k++)
                {
                    float temp = arr[maxIdx][k];
                    arr[maxIdx][k] = arr[i][k];
                    arr[i][k] = temp;
                }
            }
            pivot = arr[i][i];
        }

        // Broadcast the pivot value to all processes.
        MPI_Bcast(&pivot, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Broadcast the pivot array
        float buf[N+1];
        if (rank == 0)
        {
            for (int j = 0; j <= N; j++)
            {
                buf[j] = arr[i][j];
            }
        }
        MPI_Bcast(buf, N+1, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Find the number of rows per processor and scatter the data to them all.
        int scatterSize = (N-(i+1))/numTasks;
        if ((N-(i+1))%numTasks)
            scatterSize++;

        float* part = (float*)malloc(scatterSize*(N+1)*sizeof(float));

        MPI_Scatter(data+(i+1)*(N+1), scatterSize*(N+1), MPI_FLOAT, part, scatterSize*(N+1), MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Size of the part recieved.
        int partSize = 0;
        if ((N-(i+1)) >= scatterSize*(rank+1))
        {
            partSize = scatterSize;
        }
        else
        {
            partSize = N-(i+1)-scatterSize*rank;
        }

        if (partSize < 0)
            partSize = 0;
        
        // Reduce rows.
        for (int j = 0; j < partSize; j++)
        {
            float f = part[j*(N+1)+i]/pivot;
            for (int k = i+1; k < N+1; k++)
                part[j*(N+1)+k] -= f*buf[k];
            part[j*(N+1)+i] = 0;
        }

        // Merge rows back into final array.
        for (int j = 1; j < numTasks; j *= 2)
        {
            if (rank%(2*j) != 0)
            {
                MPI_Send(part, partSize*(N+1), MPI_FLOAT, rank - j, 0, MPI_COMM_WORLD);
                break;
            }
            else if (rank+j < numTasks)
            {
                int expectedSize = 0;
                if ((N-(i+1)) >= scatterSize * (rank+2*j))
                {
                    expectedSize = scatterSize*j;
                }
                else
                {
                    expectedSize = N-(i+1)-scatterSize*(rank+j);
                }

                if (expectedSize <= 0)
                    expectedSize = 0;
                
                part = (float*)realloc(part, (partSize+expectedSize)*(N+1)*sizeof(float));
                MPI_Recv(part+partSize*(N+1), expectedSize*(N+1), MPI_FLOAT, rank+j, 0, MPI_COMM_WORLD, &status);
                partSize += expectedSize;
            }
        }

        // Assign the received rows to the array stored by process 0.
        if (rank == 0)
        {
            int startIdx = (i+1)*(N+1);
            for (int k = startIdx; k < N*(N+1); k++)
            {
                data[k] = part[k - startIdx];
            }

            for (int j = 0; j < N; j++)
                arr[j] = &(data[(N+1)*j]);
        }
        free(part);
    }

    // Back-substitute to find the result and print it.
    if (rank == 0)
    {
        float results[N];

        for (int i = N-1; i >= 0; i--)
        {
            results[i] = arr[i][N];

            for (int j = i+1; j < N; j++)
                results[i] -= arr[i][j]*results[j];
            
            results[i] = results[i]/arr[i][i];
        }

        t2 = MPI_Wtime();

        printf("Solution\n");
        for (int i = 0; i < N; i++)
            printf("%f\t",results[i]);

        printf("\nTime taken is %lf", t2-t1);
    }

    MPI_Finalize();
}