#ifndef HANDLER_H
#define HANDLER_H

extern int SIGTABLE[26];

extern int msg_not_dlvd;

extern int rcv_pid;

extern int eol;

void init_sig_table();
void handle_sig_talk(int signum);
void handle_ack(int signum);

#endif
