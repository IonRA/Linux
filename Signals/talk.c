#include <stdio.h>
#include <signal.h> 
#include <wait.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "handler.h"

int main()
{
    int sig = 1;
    int sig_no = 0;

    while (sig_no != 28)
    {
        if (sig != SIGKILL && sig != SIGCONT && sig != SIGSTOP)
        {
            signal(sig, handle_sig_talk);
            ++sig_no;
        }

        ++sig;
    }

    __pid_t pid = getpid();

    printf("My PID is: %d\n", pid);

    __pid_t rcv_pid;

    printf("Reciever PID is: ");

    if (scanf("%d%*c", &rcv_pid) != 1)
    {
        printf("Error: Failed to read from stdin!\n");
        perror("Message from perror");

        return 0;
    }

    if (getpgid(rcv_pid) <= 0)
    {
        printf("The reciever's PID does not exist!\n");
        return 0;
    }

    char*  line = NULL;

    size_t len = 0;

    printf("Write a message: ");

    if (getline(&line, &len, stdin) == -1)
    {
        printf("Error: Failed to read from stdin!\n");
        perror("Message from perror");
        
        free(line);

        return 0;
    }

    printf("%s", line);

    free(line);

    return 0;
}
