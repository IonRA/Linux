#ifndef ERROR_H
#define ERROR_H

enum error {
    SUCCES = 0,
    ERROR_OPPENING_FILE = (1 << (sizeof(int) * 8 - 1)) | 1,
    ERROR_READING_FILE = (1 << (sizeof(int) * 8 - 1)) | 2,
    ERROR_WRITING_FILE = (1 << (sizeof(int) * 8 - 1)) | 4,
    ERROR_PARSING_FILE = (1 << (sizeof(int) * 8 - 1)) | 8,
    ERROR_FILE_NOT_RESPONDING = (1 << (sizeof(int) * 8 - 1)) | 16,
    ERROR_CLOSING_FILE = (1 << (sizeof(int) * 8 - 1)) | 32,
    ERROR_OUT_OF_MEMORY = (1 << (sizeof(int) * 8 - 1)) | 64,
    ERROR_MATRIX_SUMATION = (1 << (sizeof(int) * 8 - 1)) | 128,
    ERROR_MATRIX_PRODUCT = (1 << (sizeof(int) * 8 - 1)) | 256,
    ERROR_LOGICAL = (1 << (sizeof(int) * 8 - 1)) | 512,
    ERROR_OUT_OF_BOUNDARIES = (1 << (sizeof(int) * 8 - 1)) | 1024,
    ERROR_FILE_STAT = (1 << (sizeof(int) * 8 - 1)) | 2048,
    ERROR_REMOVE_FILE = (1 << (sizeof(int) * 8 - 1)) | 4096,
    ERROR_CREATE_FILE = (1 << (sizeof(int) * 8 - 1)) | 8192
};

void printError(int errCode);

#endif

