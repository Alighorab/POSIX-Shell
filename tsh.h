#include "functionality.h"

/*
 * tsh - A tiny shell program with job control
 */

extern char **environ;             /* defined in libc */
extern int verbose;                /* if true, print additional output */
extern struct job_t jobs[MAXJOBS]; /* The job list */
