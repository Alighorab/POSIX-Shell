#ifndef HANDLERS_h
#define HANDLERS_h

#include "helpers.h"

extern struct job_t jobs[MAXJOBS];

void sigquit_handler(int sig);
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

#endif
