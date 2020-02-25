#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Swaps two elements, used by bubble sort
void swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Best case O(n) implementation of bubble sort.
void bubbleSort(int * arr, int n)
{
    for (int i = n-2; i >= 0; i--)
    {
        int exchanged = 0;
        for (int j = 0; j <= i; j++)
        {
            if (arr[j] > arr[j+1])
            {
                exchanged = 1;
                swap(&arr[j], &arr[j+1]);
            }
        }
        if (!exchanged)
            break;
    }
}

// Merges a and b into a new sorted array
int* merge(int* a, int aSize, int* b, int bSize)
{
    int totalSize = aSize+bSize;
    int* final = (int*)malloc(totalSize*sizeof(int));
    int i = 0, j = 0, k = 0;
    while (i < aSize && j < bSize)
    {
        if (a[i] <= b[j])
        {
            final[k++] = a[i++];
        }
        else
        {
            final[k++] = b[j++];
        }
    }

    // Copy remaining elements from the arrays.
    while (i < aSize)
        final[k++] = a[i++];
    
    while (j < bSize)
        final[k++] = b[j++];
    
    return final;
}

int main(int argc, char ** argv)
{
    int rank = 0, numTasks=0, size=0, maxSize=0;
    int *arr = NULL;
    int* part = NULL;
    int* toMerge = NULL;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);

    if (rank == 0)
    {
        FILE* fptr;
        fptr = fopen("input.txt", "r");
        if (fptr == NULL)
        {
            fprintf(stderr,"Cannot open file\n");
            exit(0);
        }

        // Initialize array to be sorted with size 16.
        arr = (int*)malloc(sizeof(int)*16);
        size = 0;
        maxSize = 10;

        // Read data from file, and dynamically allocate required memory.
        int i = 0;
        while (!feof(fptr))
        {
            fscanf(fptr, "%d", arr+size);
            size++;
            if (size == maxSize)
            {
                maxSize *= 2;
                arr = realloc(arr, maxSize*sizeof(int));
            }
        }
        fclose(fptr);
    }

    // Broadcast the size of our input array.
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Scatter data
    int scatterSize = size/numTasks;
    if (size % numTasks)
        scatterSize++;

    part = (int *)malloc(scatterSize*sizeof(int));
    MPI_Scatter(arr, scatterSize, MPI_INT, part, scatterSize, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Free the memory we used for the array to prepare for the output.
    free(arr);
    arr = NULL;

    int partSize = (size >= scatterSize * (rank + 1)) ? scatterSize : size - scatterSize * rank;
    bubbleSort(part, partSize);

    // Merge the arrays of processors in a tree-like fashion.
    for (int i = 1; i < numTasks; i*=2)
    {
        if (rank%(2*i) != 0)
        {
            MPI_Send(part, partSize, MPI_INT, rank-i, 0, MPI_COMM_WORLD);
            break;
        }
        else if (rank+i < numTasks)
        {
            // Find the size of the chunk expected to be recieved.
            int expectedSize = (size >= scatterSize * (rank+2*i)) ? scatterSize * i : size - scatterSize * (rank + i);

            // Recieve the chunk
            toMerge = (int *)malloc(expectedSize*sizeof(int));
            MPI_Recv(toMerge, expectedSize, MPI_INT, rank+i, 0, MPI_COMM_WORLD, &status);

            // Merge the memory into that node's part.
            arr = merge(part, partSize, toMerge, expectedSize);
            part = arr;
            partSize += expectedSize;

            // Free extra memory we used.
            free(toMerge);
        }
    }

    // Print sorted data!
    if (rank == 0)
    {
        for (int i = 0; i < partSize; i++)
            printf("%d\n", arr[i]);
    }

    MPI_Finalize();
}