#include <stdio.h>
#include "error.h"

void printError(int errCode)
{
    if (!errCode)
    {
        printf("No errors have occured!\n");
        return;
    }

    int i = sizeof(int) * 8 - 1;
    int err = 1;

    while (i--)
    {
        if (err & errCode)
            switch ((enum error) (err | (1 << sizeof(int) * 8 - 1)))
            {
                case ERROR_OPPENING_FILE:
                    printf("An error has occured when oppening the file!\n");
                break;

                case ERROR_READING_FILE:
                    printf("An error has occured while reading from file!\n");
                break;

                case ERROR_WRITING_FILE:
                    printf("An error has occured while writing in file!\n");
                break;

                case ERROR_PARSING_FILE:
                    printf("An error has occured while parsing the file!\n");
                break;

                case ERROR_FILE_NOT_RESPONDING:
                    printf("The file is not responding\n");
                break;

                case ERROR_CLOSING_FILE:
                    printf("An error has occured when closing the file!\n");
                break;

                case ERROR_OUT_OF_MEMORY:
                    printf("Program runned out of memory!\n");
                break;

                case ERROR_MATRIX_SUMATION:
                    printf("Error: The matrices to be summed must have same sizes!\n");
                break;

                case ERROR_MATRIX_PRODUCT:
                    printf("Error: Left matrix columns have to be as many as right matrix rows!\n");
                break;

                case ERROR_LOGICAL:
                    printf("Logical fallacy has occured!\n");
                break;

                case ERROR_OUT_OF_BOUNDARIES:
                    printf("Error: Indices exeed matrix sizes!\n");
                break;

                case ERROR_FILE_STAT:
                    printf("An error has occured while checking file status!\n");
                break;

                case ERROR_REMOVE_FILE:
                    printf("Error: Can not remove file!\n");
                break;

                case ERROR_CREATE_FILE:
                    printf("An error has occured while creating file!\n");
                break;

                default:
                    printf("An unidentefied error has occured!\n");
            }

        err <<= 1;
    }
}
