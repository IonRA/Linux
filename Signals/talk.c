#include <stdio.h>
#include <signal.h> 
#include <wait.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "handler.h"

int char_to_sig(char c);

int main()
{
    init_sig_table();

    signal(SIGRTMIN, handle_ack);

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

    printf("Receiver PID is: ");

    if (scanf("%d%*c", &rcv_pid) != 1)
    {
        printf("Error: Failed to read from stdin!\n");
        perror("Message from perror");

        return 0;
    }

    if (getpgid(rcv_pid) <= 0)
    {
        printf("The receiver's PID does not exist!\n");
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

    for (int i = 0; i < strlen(line); ++i)
    {
        kill(rcv_pid, char_to_sig(line[i]));
        pause();

        if (msg_not_dlvd)
        {
            printf("Error: Receiver not responding!\n");
        
            free(line);

            return 0;
        }
    }

    while(!eol){}

    free(line);

    return 0;
}


int char_to_sig(char c)
{
    if (c == ' ')
        return 30;

    if (c == '\n')
        return 31;

    return SIGTABLE[c - 'a'];
}
