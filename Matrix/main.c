#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "fmatrix.h"
#include "error.h"

int main (int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Wrong input arguments!\n");
        printf("Specify two operands paths and the result path!\n");
        return 0;
    }

    int rowNum1, colNum1;

    printf("Type the number of rows of the first matrix: ");

    scanf("%d", &rowNum1);

    printf("Type the number of columns of the first matrix: ");

    scanf("%d", &colNum1);

    printf("\nCreating first matrix...\n");

    int failed;

    if (failed = newMatrix(argv[1], rowNum1, colNum1))
    {
        printError(failed);
        perror("Message from perror");
        return 0;
    }

    printf("First matrix has been created!\n");

    printf("\nFilling first matrix:\n");

    double value;

    for (int i = 0; i < rowNum1; ++i)
        for (int j = 0; j < colNum1; ++j)
        {
            printf("mat1[%d, %d] = ", i, j);

            scanf("%lf", &value);

            if (failed = set(argv[1], i, j, value))
            {
                remove(argv[1]);
                printError(failed);
                perror("Message from perror");
                return 0;
            }
        }

    int rowNum2, colNum2;

    printf("First matrix has been filled!\n\n");

    printf("First matrix looks like this:\n");

    if (failed = printMatrix(argv[1]))
    {
        remove(argv[1]);
        printError(failed);
        perror("Message from perror");
        return 0;
    }

    printf("\nType the number of rows of the second matrix: ");

    scanf("%d", &rowNum2);

    printf("Type the number of rows of the second matrix: ");

    scanf("%d", &colNum2);

    printf("Creating second matrix:\n");

    if (failed = newMatrix(argv[2], rowNum2, colNum2))
    {
        remove(argv[1]);
        printError(failed);
        perror("Message from perror");
        return 0;
    }

    printf("Second matrix has been created!\n");

    printf("\nFilling second matrix:\n");

    for (int i = 0; i < rowNum2; ++i)
        for (int j = 0; j < colNum2; ++j)
        {
            printf("mat2[%d, %d] = ", i, j);

            scanf("%lf", &value);

            if (failed = set(argv[2], i, j, value))
            {
                remove(argv[2]);
                printError(failed);
                perror("Message from perror");
                return 0;
            }
        }

    printf("Second matrix has been filled!\n\n");

    printf("Second matrix looks like this:\n");

    if (failed = printMatrix(argv[2]))
    {
        remove(argv[1]);
        printError(failed);
        perror("Message from perror");
        return 0;
    }

    printf("\nType:\n[1] to calculate the sum of the matrices.");
    printf("\n[2] to calculate the product of small matrices.");
    printf("\n[3] to calculate the product of large matrices.");
    printf("\n[OTHER] to exit the program.\n\n");

    int choice;

    scanf("%d", &choice);

    switch (choice)
    {
        case 1:
                if (failed = matrixSum(argv[1], argv[2], argv[3]))
                {
                    remove(argv[1]);
                    remove(argv[2]);
                    printError(failed);
                    perror("Message from perror");
                    return 0;
                }
        break;

        case 2:
                if (failed = fastMatProd(argv[1], argv[2], argv[3]))
                {
                    remove(argv[1]);
                    remove(argv[2]);
                    printError(failed);
                    perror("Message from perror");
                    return 0;
                }
        break;

        case 3:
                if (failed = largeMatProd(argv[1], argv[2], argv[3]))
                {
                    remove(argv[1]);
                    remove(argv[2]);
                    printError(failed);
                    perror("Message from perror");
                    return 0;
                }
        break;

        default:
                return 0;
    }

    if (choice == 1)
        printf("\nThe sum of the two matrices looks like this:\n");
    else
        printf("\nThe product of the two matrices looks like this:\n");
       
    if (failed = printMatrix(argv[3]))
    {
        remove(argv[1]);
        remove(argv[2]);
        remove(argv[3]);
        printError(failed);
        perror("Message from perror");
    }

    return 0;
}
