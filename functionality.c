#include "functionality.h"

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */
void
eval(char *cmdline)
{
    char *argv[MAXARGS]; /* Argument list for exexve() */
    char buf[MAXLINE];   /* Modified command-line */
    int bg;              /* Is it background? */
    pid_t pid = 0;       /* Process ID */
    sigset_t mask, mask_all, prev_mask;
    int state;

    sigfillset(&mask_all);
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    if (argv[0] == NULL) {
        return; /* Ignore empty spaces */
    }

    if (!builtin_cmd(argv)) {
        sigprocmask(SIG_BLOCK, &mask, &prev_mask);
        if ((pid = fork()) == 0) {
            setpgid(0, 0);
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        state = bg ? BG : FG;
        addjob(jobs, pid, state, cmdline);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        if (!bg) {
            waitfg(pid);
        } else {
            printf("[%d] (%d) %s\n", pid2jid(pid), pid, cmdline);
        }
    }
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int
parseline(const char *cmdline, char **argv)
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf) - 1] = ' ';   /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
        buf++;
        delim = strchr(buf, '\'');
    } else {
        delim = strchr(buf, ' ');
    }

    while (delim) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        } else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;

    if (argc == 0) /* ignore blank line */
        return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }
    return bg;
}
/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int
builtin_cmd(char **argv)
{
    if (!strcmp(argv[0], "quit")) {
        exit(0);
    } else if (!strcmp(argv[0], "fg") || !strcmp(argv[0], "bg")) {
        do_bgfg(argv);
        return 1;
    } else if (!strcmp(argv[0], "jobs")) {
        listjobs(jobs);
        return 1;
    }
    return 0; /* not a builtin command */
}

static int
isvalid(char *str)
{
    int length = 0;
    if (str[0] == '%') {
        str++;
    }
    length = strlen(str);
    for (int i = 0; i < length; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void
do_bgfg(char **argv)
{
    int jid = 0;
    pid_t pid = 0;
    struct job_t *job;
    sigset_t mask_all, prev_mask;
    sigfillset(&mask_all);

    sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);

    if (argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        return;
    } else if (!isvalid(argv[1])) {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        return;

    } else if (argv[1][0] == '%') {
        char *jid_str = &argv[1][1];
        jid = atoi(jid_str);
        job = getjobjid(jobs, jid);
        if (!job) {
            printf("(%d): No such job\n", jid);
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            return;
        }
    } else {
        pid = atoi(argv[1]);
        job = getjobpid(jobs, pid);
        if (!job) {
            printf("(%d): No such job\n", pid);
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            return;
        }
    }

    if (!strcmp(argv[0], "bg")) {
        kill(-job->pid, SIGCONT);
        job->state = BG;
        printf("[%d] (%d) %s\n", job->jid, job->pid, job->cmdline);
    } else if (!strcmp(argv[0], "fg")) {
        kill(-job->pid, SIGCONT);
        job->state = FG;
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        waitfg(job->pid);
    }
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void
waitfg(pid_t pid)
{
    sigset_t mask_sigchld, prev_mask;
    sigemptyset(&mask_sigchld);
    sigprocmask(SIG_BLOCK, &mask_sigchld, &prev_mask);

    while (fgpid(jobs) == pid) {
        sigsuspend(&prev_mask);
    }

    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
}
