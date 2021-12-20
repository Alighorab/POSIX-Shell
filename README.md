# Tiny SHell - TSH


## Description

- This project is a tiny UNIX shell supports only job control.
- It's the 5th lab of [15-213: Introduction to Computer Systems.](https://www.cs.cmu.edu/afs/cs.cmu.edu/academic/class/15213-f15/www/schedule.html)
- Main routine is very simple, It (1): **prints** a prompt, (2): **reads** the command-line, (3): **evaluate** it.
- Evaluation routine is also very simple, It: 
    - (1): **parses** the command-line to build `argv` vector, 
    - (2): checks if it's **built-in** command to execute it, if not, it **forks** a child and call **execve** to execute it, 
    - (3): checks if the job is **background** or **foreground**,
        - (a): if it's **bg**; **tsh** doesn't wait for it to terminate.
        - (b): if it's **fg**; **tsh** waits for it to terminate and then reaps it.


## How to use it?
- Open your terminal and run the following:
    - `git clone <this repo>`
    - `cd Tiny-Shell`
    - `make`
    - `./tsh`
- The following commands are supported:
    - `quit`: terminates the shell.
    - `jobs`: lists all background jobs.
    - `bg <job>`: restarts `<job>` by sending it a `SIGCONT` signal, and then runs it in the background. The `<job>` argument can be either a PID or a JID.
    - `fg <job>`: restarts `<job>` by sending it a `SIGCONT` signal, and then runs it in the foreground. The `<job>` argument can be either a PID or a JID.

- You can also run your own programs by giving `tsh` the absolute path of your program.
    - For example:
        - `tsh> /bin/ls -a`
        - `tsh> /bin/echo 'Hello world'`
        - `tsh> /bin/ps a`

**Hint 1**: Programs such as `more`, `less`, `vi`, and `emacs` do strange things with the terminal settings. Don’t run these programs from your shell. Stick with simple text-based programs such as `/bin/ls`, `/bin/ps`, and `/bin/echo`.

**Hint 2**: tsh need not support pipes (`|`) or I/O redirection (`<` and `>`).

## Lab Files

- `tsh.c`: the main routine.
- `functionality.c`: the main functionality of the shell, i.e., parsing and evaluating the command-line.
- `helpers.c`: contains the job data structure and other helper functions to modify it.
- `handlers.c`: contains signal handlers for the predicted signals: `SIGTSTP`, `SIGINT`, `SIGTERM`, and `SIGCHLD`.
- `./tshref`: the reference solution for the shell. (provided with the lab handout)
- `./sdriver.pl`: The trace-driven shell driver. Use the -h argument to find out how to use it.
- `traces/*`: The 15 trace files that control the shell driver.
- `tests/*`: programs to test on the shell.
- `shlab.pdf`: lab writup.

## The purpose of this lab

- To become more familiar with the concepts of process control and signalling. 
