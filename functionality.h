#ifndef FUNCTIONALITY_h
#define FUNCTIONALITY_h

#include "handlers.h"
#include "helpers.h"

extern struct job_t jobs[MAXJOBS];
extern int nextjid;    /* next job ID to allocate */
extern int verbose;    /* if true, print additional output */
extern char **environ; /* defined in libc */

/* Function prototypes */

void eval(char *cmdline);
int parseline(const char *cmdline, char **argv);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

#endif
