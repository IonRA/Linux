#include "handler.h"
#include <signal.h>
#include <unistd.h>

int SIGTABLE[26];
int msg_not_dlvd = 0;
int eol = 0;

__pid_t rcv_pid = 0;

void init_sig_table()
{
    int sig = 1;
    int c = 0;

    while (c != 26)
    {
        if (sig != SIGKILL && sig != SIGCONT && sig != SIGSTOP)
        {
            SIGTABLE[c] = sig;
            ++c;
        }

        ++sig;
    }
}

void handle_sig_talk(int signum)
{
    if (signum == 31)
        eol = 1;

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

    kill(rcv_pid, SIGRTMIN);
}

void handle_ack(int signum)
{
    if (signum != SIGRTMIN)
        msg_not_dlvd = 1;
}
