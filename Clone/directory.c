#include "directory.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include "error.h"
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int copyDirectory(const char * dest, const char* source)
{
    if (!strcmp(dest, source))
        return SUCCES;

    struct stat st;

    mode_t perm;

    if(stat(source, &st))
        return ERROR_FILE_STAT;

    perm = st.st_mode;

    DIR* dirSource = opendir(source);

    if (dirSource == NULL)
        return ERROR_OPPENING_FILE;

    int failed;

    if (mkdir(dest, perm))
    {
        failed = ERROR_CREATE_FILE;

        if (closedir(dirSource))
            failed |= ERROR_CLOSING_FILE;

        return failed;
    }

    struct dirent* entSource;

    errno = 0;

    int srcPathLen = strlen(source);
    int destPathLen = strlen(dest);

    while ((entSource = readdir(dirSource)) != NULL)
    {
        if (!strcmp(entSource->d_name, ".") || !strcmp(entSource->d_name, ".."))
            continue;

        int srcChildPathLen = srcPathLen + strlen(entSource->d_name) + 2;
        int destChildPathLen = destPathLen + strlen(entSource->d_name) + 2;

        char* srcChildPath = malloc(srcChildPathLen);

        if (srcChildPath == NULL)
        {
            failed = ERROR_OUT_OF_MEMORY;

            if (closedir(dirSource))
                failed |= ERROR_CLOSING_FILE;

            failed |= removeDirectory(dest);

            return failed;
        }

        char* destChildPath = malloc(destChildPathLen);

        if (destChildPath == NULL)
        {
            free(srcChildPath);

            failed = ERROR_OUT_OF_MEMORY;

            if (closedir(dirSource))
                failed |= ERROR_CLOSING_FILE;

            failed |= removeDirectory(dest);

            return failed;
        }

        strcpy(srcChildPath, source);
        strcpy(destChildPath, dest);

        if(source[srcPathLen - 1] != '/')
        {
            strcpy(srcChildPath + srcPathLen,"/");
            strcpy(srcChildPath + srcPathLen + 1, entSource->d_name);
        }
        else
            strcpy(srcChildPath + srcPathLen, entSource->d_name);

        if(dest[destPathLen - 1] != '/')
        {
            strcpy(destChildPath + destPathLen,"/");
            strcpy(destChildPath + destPathLen + 1, entSource->d_name);
        }
        else
            strcpy(destChildPath + destPathLen, entSource->d_name);

        if (stat(srcChildPath, &st))
        {
            free(srcChildPath);
            free(destChildPath);

            failed = ERROR_FILE_STAT;

            if (closedir(dirSource))
                failed |= ERROR_CLOSING_FILE;

            failed |= removeDirectory(dest);

            return failed;
        }

        if (S_ISDIR(st.st_mode))
        {
            if (failed = copyDirectory(destChildPath, srcChildPath))
            {
                free(srcChildPath);
                free(destChildPath);

                if (closedir(dirSource))
                    failed |= ERROR_CLOSING_FILE;

                failed |= removeDirectory(dest);

                return failed;
            }
        } 
        else if (failed = copyFile(destChildPath, srcChildPath))
        {
            free(srcChildPath);
            free(destChildPath);

            if (closedir(dirSource))
                failed |= ERROR_CLOSING_FILE;

            failed |= removeDirectory(dest);

            return failed;
        }

        free(srcChildPath);
        free(destChildPath);
    }

    if (closedir(dirSource))
        return ERROR_CLOSING_FILE;

    if (errno)
        return ERROR_READING_FILE;

    return SUCCES;
}

int copyFile(const char* dest, const char* source)
{
    struct stat st;

    if (stat(source, &st))
        return ERROR_FILE_STAT;

    int fDescSrc = open(source, O_RDONLY, S_IRUSR);

    if (fDescSrc == -1)
        return ERROR_OPPENING_FILE;

    int fDescDest = open(dest, O_CREAT | O_WRONLY, st.st_mode);

    int failed;

    char byte;

    while((failed = read(fDescSrc, (char*)&byte,1)) && failed != -1)
    {
        if (write(fDescDest, &byte, 1) == -1)
        {
            failed = ERROR_WRITING_FILE;
            if (close(fDescSrc) == -1)
                failed |= ERROR_CLOSING_FILE;

            if (close(fDescDest) == -1)
                failed |= ERROR_CLOSING_FILE;

            if (unlink(dest))
                failed |= ERROR_REMOVE_FILE;

            return failed;
        }
    }

    if (close(fDescSrc) == -1)
        return ERROR_CLOSING_FILE;

    if (close(fDescDest) == -1)
        return ERROR_CLOSING_FILE;

    return SUCCES;
}

int removeDirectory(const char* dir) {
    DIR *dirSource = opendir(dir);

    if (dirSource == NULL)
        return ERROR_OPPENING_FILE;

    int failed;

    struct dirent *entSource;

    struct stat st;

    errno = 0;

    int dirPathLen = strlen(dir);

    while ((entSource = readdir(dirSource)) != NULL)
    {
        if (!strcmp(entSource->d_name, ".") || !strcmp(entSource->d_name, ".."))
            continue;

        int dirChildPathLen = dirPathLen + strlen(entSource->d_name) + 2;

        char* dirChildPath = malloc(dirChildPathLen);

        if (dirChildPath == NULL)
        {
            failed = ERROR_OUT_OF_MEMORY;

            if (closedir(dirSource))
                failed |= ERROR_CLOSING_FILE;

            return failed;
        }

        strcpy(dirChildPath, dir);

        if(dir[dirPathLen - 1] != '/')
        {
            strcpy(dirChildPath + dirPathLen,"/");
            strcpy(dirChildPath + dirPathLen + 1, entSource->d_name);
        }
        else
            strcpy(dirChildPath + dirPathLen, entSource->d_name);


        if (stat(dirChildPath, &st))
        {
            free(dirChildPath);

            failed = ERROR_FILE_STAT;

            if (closedir(dirSource))
                failed |= ERROR_CLOSING_FILE;

            return failed;
        }

        if (S_ISDIR(st.st_mode))
        {
            if (failed = removeDirectory(dirChildPath))
            {
                free(dirChildPath);

                if (closedir(dirSource))
                    failed |= ERROR_CLOSING_FILE;

                return failed;
            }
        }
        else
        {
            if (failed = unlink(dirChildPath))
            {
                free(dirChildPath);

                failed = ERROR_REMOVE_FILE;

                if (closedir(dirSource))
                    failed |= ERROR_CLOSING_FILE;

                return failed;
            }
        }

        free(dirChildPath);
    }

    if (closedir(dirSource))
        return ERROR_CLOSING_FILE;

    if (errno)
        return ERROR_READING_FILE;

    if (rmdir(dir) < -1)
        return ERROR_REMOVE_FILE;

    return SUCCES;
}
