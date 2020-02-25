#include <stdlib.h>
#include <stdio.h>

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
    fscanf(fptr, "%d %d", &m, &n);
    int** arr = (int**)malloc(sizeof(int*)*m);
    for (int i = 0; i < m; i++)
        arr[0] = (int*)malloc(sizeof(int)*n);

    if (m+1 != n)
    {
        fprintf("Invalid input!");
        return;
    }
    
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fscanf(fptr, "%d", &arr[i][j]);
        }
    }
}