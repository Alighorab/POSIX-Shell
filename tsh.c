#include "tsh.h"

char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
char sbuf[MAXLINE];         /* for composing sprintf messages */


/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
    	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
    	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
    	    break;
	    default:
            usage();
	    }
    }

    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
    Signal(SIGQUIT, sigquit_handler); 

    initjobs(jobs);

    while (1) {

        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }

	    if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	        app_error("fgets error");
	    if (feof(stdin)) { /* End of file (ctrl-d) */
	        fflush(stdout);
	        exit(0);
	    }

	    eval(cmdline);
	    fflush(stdout);
	    fflush(stdout);
    } 

    exit(0); 
}
