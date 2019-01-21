#ifndef FMATRIX_H
#define FMATRIX_H

int newMatrix(const char* file, int rowNum, int colNum);
int size(const char* file, int dim);
int get(const char* file, int row, int col, double* dest);
int set(const char* file, int row, int col, double value);
int matrixSum(const char* fMat1, const char* fMat2, const char* sum);
int fastMatProd(const char* fMat1, const char* fMat2, const char* prod);
int largeMatProd(const char* fMat1, const char* fMat2, const char* prod);
int printMatrix(const char* mat);

#endif
