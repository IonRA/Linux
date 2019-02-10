#include "handler.h"
#include <signal.h>
#include <unistd.h>

void handle_sig_talk(int signum)
{
    char sigMessage[32];

    char mss = 'a';
    int sig = 1;

    while (mss <= 'z')
    {
        if (sig != SIGKILL && sig != SIGCONT && sig != SIGSTOP)
        {
            sigMessage[sig] = mss;
            ++mss;
        }

        ++sig;
    }

    sigMessage[30] = ' ' ;
    sigMessage[31] = '\n';

    write(STDOUT_FILENO, sigMessage + signum , 1);
}