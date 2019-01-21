#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "fmatrix.h"
#include "error.h"


int newMatrix(const char*  file, int rowNum, int colNum)
{
    if (rowNum <= 0 || colNum <= 0)
        return ERROR_LOGICAL;

    int fDescriptor = open(file, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);

    if (fDescriptor == -1)
        return ERROR_OPPENING_FILE;

    if (
        write(fDescriptor, (char*)&rowNum, sizeof(int)) == -1 ||
        write(fDescriptor, (char*)&colNum, sizeof(int)) == -1
        )
    {
        if (close(fDescriptor) == -1)
            return ERROR_CLOSING_FILE | ERROR_WRITING_FILE;

        return ERROR_WRITING_FILE;
    }

    double dZero = 0;

    int failed;

    for (int i = 0; i < rowNum; ++i)
        for (int j = 0; j < colNum; ++j)
            if (write(fDescriptor, (char*)&dZero, sizeof(double)) == -1)
            {
                failed = ERROR_WRITING_FILE;

                if (close(fDescriptor) == -1)
                    failed |= ERROR_CLOSING_FILE;

                if (remove(file) == -1)
                    failed |= ERROR_REMOVE_FILE;

                return failed;
            }

    if (close(fDescriptor) == -1)
    {
        failed = ERROR_CLOSING_FILE;

        if (remove(file) == -1)
            failed |= ERROR_REMOVE_FILE;

        return failed;
    }

    return SUCCES;
}

int size(const char* file, int dim)
{
    if (dim < 0)
        return ERROR_LOGICAL;

    int fDescriptor = open(file, O_RDONLY, S_IRUSR);

    if (fDescriptor == -1)
        return ERROR_OPPENING_FILE;

    int failed;

    if (lseek(fDescriptor, dim * sizeof(int), SEEK_SET) == -1)
    {
        failed = ERROR_PARSING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    int dimSize;

    if (read(fDescriptor, (char*)&dimSize, sizeof(int)) == -1)
    {
        failed = ERROR_READING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    return dimSize;
}

int get(const char* file, int row, int col, double* dest)
{
    if (row < 0 || col < 0)
        return ERROR_LOGICAL;

    int fDescriptor = open(file, O_RDONLY, S_IRUSR);

    if (fDescriptor == -1)
        return ERROR_OPPENING_FILE;

    int rowNum;

    int failed;

    if (read(fDescriptor, (char*)&rowNum, sizeof(int)) == -1)
    {
        failed = ERROR_READING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    int colNum;

    if (read(fDescriptor, (char*)&colNum, sizeof(int)) == -1)
    {
        failed = ERROR_READING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    if (lseek(fDescriptor, row * colNum * sizeof(double) + col * sizeof(double), SEEK_CUR) == -1)
    {
        failed = ERROR_PARSING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    double value;

    if (read(fDescriptor, (char*)&value, sizeof(double)) == -1)
    {
        failed = ERROR_READING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    if (close(fDescriptor) == -1)
        return ERROR_FILE_NOT_RESPONDING;

    *dest = value;

    return SUCCES;
}

int set(const char* file, int row, int col, double value)
{
    if (row < 0 || col < 0)
        return ERROR_LOGICAL;

    int fDescriptor = open(file, O_RDWR, S_IRUSR | S_IWUSR);

    if (fDescriptor == -1)
        return ERROR_OPPENING_FILE;

    int rowNum;

    int failed;

    if (read(fDescriptor, (char*)&rowNum, sizeof(int)) == -1)
    {
        failed = ERROR_READING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    int colNum;

    if (read(fDescriptor, (char*)&colNum, sizeof(int)) == -1)
    {
        failed = ERROR_READING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    if (lseek(fDescriptor, row * colNum * sizeof(double) + col * sizeof(double), SEEK_CUR) == -1)
    {
        failed = ERROR_PARSING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    if (write(fDescriptor, (char*)&value, sizeof(double)) == -1)
    {
        failed = ERROR_WRITING_FILE;

        if (close(fDescriptor) == -1)
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    if (close(fDescriptor) == -1)
        return ERROR_CLOSING_FILE;

    return SUCCES;
}

int matrixSum(const char* fMat1, const char* fMat2, const char* sum)
{
    int rowNum1, colNum1;

    if ((rowNum1 = size(fMat1, 0)) < 0 || (colNum1 = size(fMat1, 1)) < 0)
        return rowNum1 < 0? rowNum1 : colNum1;

    int rowNum2, colNum2;

    if ((rowNum2 = size(fMat2, 0)) < 0 || (colNum2 = size(fMat2, 1)) < 0)
        return rowNum2 < 0? rowNum2 : colNum2;

    if (rowNum1 != rowNum2 || colNum1 != colNum2)
        return ERROR_MATRIX_SUMATION;

    int failed;

    if (failed = newMatrix(sum, rowNum1, colNum1))
        return failed;

    double valMat1, valMat2;

    for (int i = 0; i < rowNum1; ++i)
        for (int j = 0; j < colNum1; ++j)
            if (
                    (failed = get(fMat1, i, j, &valMat1)) ||
                    (failed = get(fMat2, i, j, &valMat2)) ||
                    (failed = set(sum, i, j, valMat1 + valMat2))
                )
            {
                if (remove(sum) == -1)
                    return ERROR_FILE_NOT_RESPONDING;

                return failed;
            }

    return SUCCES;
}

int fastMatProd(const char* fMat1, const char* fMat2, const char* prod)
{
    int rowNum1, colNum1;

    if ((rowNum1 = size(fMat1, 0)) < 0 || (colNum1 = size(fMat1, 1)) < 0)
        return rowNum1 < 0? rowNum1 : colNum1;

    int rowNum2, colNum2;

    if ((rowNum2 = size(fMat2, 0)) < 0 || (colNum2 = size(fMat2, 1)) < 0)
        return rowNum2 < 0? rowNum2 : colNum2;

    if (colNum1 != rowNum2)
        return ERROR_MATRIX_PRODUCT;

    double** mat2 = (double**)malloc(rowNum2 * sizeof(double*));

    if (mat2 == NULL)
        return ERROR_OUT_OF_MEMORY;

    int i, j;

    for (i = 0; i < rowNum2; ++i)
    {
        mat2[i] = (double*)malloc(colNum2 * sizeof(double));

        if (mat2[i] == NULL)
        {
            for (j = 0; j < i; ++j)
                free(mat2[j]);

            free(mat2);
            
            return ERROR_OUT_OF_MEMORY;
        }
    }

    int failed;

    for (i = 0; i < rowNum2; ++i)
        for (j = 0; j < colNum2; ++j)
            if (failed = get(fMat2, i, j, *(mat2 + i) + j))
            {
                for (j = 0; j < rowNum2; ++j)
                    free(mat2[j]);

                free(mat2);

                return failed;
            }

    double* valuesMat1 = (double*)malloc(rowNum2 * sizeof(double));

    if (valuesMat1 == NULL)
    {
        for (j = 0; j < rowNum2; ++j)
            free(mat2[j]);

        free(mat2);

        return ERROR_OUT_OF_MEMORY;
    }

    if (failed = newMatrix(prod, rowNum1, colNum2))
    {
        for (j = 0; j < rowNum2; ++j)
            free(mat2[j]);

        free(mat2);

        free(valuesMat1);

        return failed;
    }

    int k;

    double res;

    for (i = 0; i < rowNum1; ++i)
    {
        for (k = 0; k < rowNum2; ++k)
            if (failed = get(fMat1, i, k, valuesMat1 + k))
            {
                if (remove(prod) == -1)
                    failed |= ERROR_REMOVE_FILE;

                for (j = 0; j < rowNum2; ++j)
                    free(mat2[j]);

                free(mat2);

                free(valuesMat1);
            }

        for (j = 0; j < colNum2; ++j)
        {
            res = 0;

            for (k = 0; k < rowNum2; ++k)
                res += valuesMat1[k] * mat2[k][j];

            if (failed = set(prod, i, j, res))
            {
                if (remove(prod) == -1)
                    failed |= ERROR_REMOVE_FILE;

                for (j = 0; j < rowNum2; ++j)
                    free(mat2[j]);

                free(mat2);

                free(valuesMat1);

                return failed;
            }
        }
    }
    
    for (j = 0; j < rowNum2; ++j)
        free(mat2[j]);

    free(mat2);

    free(valuesMat1);

    return SUCCES;
}

int largeMatProd(const char* fMat1, const char* fMat2, const char* prod)
{
    int rowNum1, colNum1;

    if ((rowNum1 = size(fMat1, 0)) < 0 || (colNum1 = size(fMat1, 1)) < 0)
        return rowNum1 < 0? rowNum1 : colNum1;

    int rowNum2, colNum2;

    if ((rowNum2 = size(fMat2, 0)) < 0 || (colNum2 = size(fMat2, 1)) < 0)
        return rowNum2 < 0? rowNum2 : colNum2;

    if (colNum1 != rowNum2)
        return ERROR_MATRIX_PRODUCT;

    int failed;

    if (failed = newMatrix(prod, rowNum1, colNum2))
        return failed;

    int k;

    double prodMatVal, valMat1, valMat2;

    int i, j;

    for (i = 0; i < rowNum1; ++i)
        for (j = 0; j < colNum2; ++j)
        {
            prodMatVal = 0;

            for (k = 0; k < rowNum2; ++k)
            {
                if (failed = get(fMat1, i, k, &valMat1))
                {
                    if (remove(prod) == -1)
                        failed |= ERROR_REMOVE_FILE;

                    return failed;
                }

                if (failed = get(fMat2, k, j, &valMat2))
                {
                    if (remove(prod) == -1)
                        failed |= ERROR_REMOVE_FILE;

                    return failed;
                }

                prodMatVal += valMat1 * valMat2;
            }

            if (failed = set(prod, i, j, prodMatVal))
            {
                if (remove(prod) == -1)
                    failed |= ERROR_REMOVE_FILE;

                return failed;
            }
        }

    return SUCCES;
}

int printMatrix(const char* mat)
{
    int rowNum, colNum;

    if ((rowNum = size(mat, 0)) < 0 || (colNum = size(mat, 1)) < 0)
        return rowNum < 0? rowNum : colNum;

    int failed;

    double value;

    for (int i = 0; i < rowNum; ++i)
    {
        for (int j = 0; j < colNum; ++j)
        {
            if (failed = get(mat, i, j, &value))
                return failed;

            printf("%lf ", value);
        }

        printf("\n");
    }

    return SUCCES;
}
