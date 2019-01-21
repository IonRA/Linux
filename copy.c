#include <stdio.h>
#include "directory.h"
#include "error.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("You need to specify both the source and destination directories!\n");
        return 0;
    }

    int failed;

    if (failed = copyDirectory(argv[2], argv[1]))
    {
        printError(failed);
        perror("Message from perror");
    }

    return 0;
}
