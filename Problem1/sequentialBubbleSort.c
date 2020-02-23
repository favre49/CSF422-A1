#include <stdlib.h>
#include <stdio.h>

void swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleSort(int* arr, int n)
{
    int swapped = 0;
    for (int i = 0; i < n-1; i++)
    {
        swapped = 0;
        for (int j = 0; j < n-i-1; j++)
        {
            if (arr[j] > arr[j+1])
            {
                swap(&arr[j], &arr[j+1]);
                swapped = 1;
            }
        }

        if (!swapped)
            break;
    }
}

int main(int argc, char ** argv)
{
    FILE* fptr;
    fptr = fopen("input.txt", "r");
    if (fptr == NULL)
    {
        fprintf(stderr,"Cannot open file\n");
        exit(0);
    }

    // Initialize array to be sorted with size 16.
    int* arr = (int*)malloc(sizeof(int)*16);
    int size = 0;
    int maxSize = 10;

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

    bubbleSort(arr, size);

    for (int i = 0; i < size; i++)
        printf("%d\n", arr[i]);
}