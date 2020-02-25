#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char** argv)
{
    FILE* fptr;
    fptr = fopen("input.txt", "r");
    if (fptr == NULL)
    {
        fprintf(stderr,"Cannot open file\n");
        exit(0);
    }

    int N; // Number of unknowns
    fscanf(fptr, "%d", &N);
    float** arr = (float**)malloc(sizeof(float*)*N);
    for (int i = 0; i < N; i++)
        arr[i] = (float*)malloc(sizeof(float)*(N+1));

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j <= N; j++)
        {
            fscanf(fptr, "%f", &arr[i][j]);
        }
    }

    for (int i = 0; i < N; i++)
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

        if (arr[i][maxIdx] == 0)
        {
            fprintf(stderr, "Singular Matrix");
            return -1;
        }

        if (maxIdx != i)
        {
            float* temp = arr[i];
            arr[i] = arr[maxIdx];
            arr[maxIdx] = temp;
        }

        for (int j = i+1; j < N; j++)
        {
            float f = arr[j][i]/arr[i][i];

            for (int k = i+1; k <= N; k++)
                arr[j][k] -= arr[i][k]*f;

            arr[j][i] = 0;
        }
    }

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N+1; j++)
        {
            printf("%f\t", arr[i][j]);
        }
        printf("\n");
    }

    float results[N];

    for (int i = N-1; i >= 0; i--)
    {
        results[i] = arr[i][N];

        for (int j = i+1; j < N; j++)
            results[i] -= arr[i][j]*results[j];
        
        results[i] = results[i]/arr[i][i];
    }

    printf("Solution\n");
    for (int i = 0; i < N; i++)
        printf("%f\t",results[i]);
}