#include "handlers.h"


/*****************
 * Signal handlers
 *****************/


/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    sigset_t mask_all, prev_mask;
    pid_t pid;
    int status;

    sigfillset(&mask_all);
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);

        if (WIFEXITED(status)) {
            deletejob(jobs, pid);
        }
        else if (WIFSIGNALED(status)) {
            printf("Job [%d] (%d) terminated by signal %d\n", 
                    pid2jid(pid), pid, WTERMSIG(status)); 
            deletejob(jobs, pid);
        }
        else if (WIFSTOPPED(status)) {
            struct job_t* job = getjobpid(jobs, pid);
            job->state = ST; 
            printf("Job [%d] (%d) stopped by signal %d\n", 
                    pid2jid(pid), pid, WSTOPSIG(status)); 
        }

        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    }

    errno = olderrno;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    sigset_t mask_all, prev_mask;
    sigfillset(&mask_all);

    sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
    pid = fgpid(jobs);
    kill(-pid, SIGINT);
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);

    errno = olderrno;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    sigset_t mask_all, prev_mask;
    sigfillset(&mask_all);

    sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
    pid = fgpid(jobs);
    kill(-pid, SIGTSTP);
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);

    errno = olderrno;
}


/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

/*********************
 * End signal handlers
 *********************/

