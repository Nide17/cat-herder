/*
 * cat-herder.c
 *
 * Create three child processes, modify their file descriptors as required,
 * modify their environment, wait for all three children to exit, and then exit itself.
 *
 * Author: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define KITTY "kitty"
extern char **environ;

/*
 * Forks a child process, and returns the pid of the child to the parent
 *
 * Parameters:
 *   None
 *
 * Returns:
 *  pid     Process ID of the child process
 */
pid_t fork_child()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }
    return pid;
}
/*
 * Sets environment variables for the child process
 *
 * Parameters:
 *  n      Number representing the child process (2, 3, or 4)
 *
 * Returns:
 *  None
 */
void set_env(int n)
{
    // Set all parent's environment variables to child's environment
    for (int i = 0; environ[i] != NULL; i++)
    {
        if (putenv(environ[i]) != 0)
        {
            perror("putenv");
            exit(1);
        }
    }

    if (n == 2)
    {
        // Parent environment variables + CATFOOD
        if (setenv("CATFOOD", "yummy", 1) != 0)
        {
            perror("setenv CATFOOD");
            exit(1);
        }
    }

    if (n == 3)
    {
        // Parent environment variables - KITTYLITTER
        if (unsetenv("KITTYLITTER") != 0)
        {
            perror("unsetenv KITTYLITTER");
            exit(1);
        }
    }

    if (n == 4)
    {
        // Getting the parent's environment variables
        char *path = getenv("PATH");
        char *home = getenv("HOME");

        // Unsetting all environment variables
        if (clearenv() != 0)
        {
            perror("clearenv");
            exit(1);
        }

        // PATH and HOME set to parent's environment
        if (setenv("PATH", path, 1) != 0)
        {
            perror("setenv PATH");
            exit(1);
        }

        if (setenv("HOME", home, 1) != 0)
        {
            perror("setenv HOME");
            exit(1);
        }

        // Parent environment variables + CATFOOD
        if (setenv("CATFOOD", "yummy", 1) != 0)
        {
            perror("setenv CATFOOD");
            exit(1);
        }
    }
}

/*
 * Executes kitty process with the given argument
 *
 * Parameters:
 *  n      Number representing the child process (2, 3, or 4)
 *
 * Returns:
 *  None
 */
void exec_kitty(char *n)
{
    char *args[] = {KITTY, n, NULL};
    if (execv(KITTY, args) < 0)
    {
        perror("execv");
        exit(1);
    }
}

/*
 * Closes all open file descriptors except stdin, stdout
 *
 * Parameters:
 *  None
 *
 * Returns:
 *  None
 */
void close_fds()
{
    for (int i = 3; i < sysconf(_SC_OPEN_MAX); i++)
        if (i != STDIN_FILENO && i != STDOUT_FILENO)
            close(i);
}

/*
 * Waits for child process to exit with status 0
 *
 * Parameters:
 *  pid_kitty      Child process id to wait for.
 *
 * Returns:
 *  None
 */
void wait_child(pid_t pid_kitty)
{
    int status;
    if (waitpid(pid_kitty, &status, 0) < 0 || WEXITSTATUS(status) != 0)
    {
        fprintf(stderr, "Error waiting for child with PID %d: %s\n", pid_kitty, strerror(errno));
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    // Check if inputfile and outputfile are provided
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile>\n", argv[0]);
        exit(1);
    }

    // Create two pipes for communication between processes: kitty -2 < (inputfile) | kitty -3 | kitty -4 > (outputfile)
    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("pipe");
        exit(1);
    }

    // Create three child processes to run kitty -2, kitty -3, and kitty -4
    // Child process 1 (kitty -2)
    pid_t pid_kitty_2 = fork_child();
    if (pid_kitty_2 == 0)
    {
        // Set environment variables for kitty -2
        set_env(2);

        // cat-herder inputfile
        const char *inputfile = argv[1];

        int fdin = open(inputfile, O_RDONLY);
        if (fdin < 0)
        {
            perror("open inputfile");
            exit(1);
        }

        // Putting kitty -2's stdin on the input file
        dup2(fdin, STDIN_FILENO);

        // Putting kitty -2's stdout on pipe1 write end: write stdout to pipe1
        dup2(pipe1[1], STDOUT_FILENO);

        // Close all file descriptors except stdin, stdout
        close_fds();

        // Execute kitty -2
        exec_kitty("-2");
    }

    // Create second child process: kitty -3 < pipe1
    pid_t pid_kitty_3 = fork_child();
    if (pid_kitty_3 == 0)
    {
        // Set environment variables for kitty -3
        set_env(3);

        // Putting kitty -3's stdin on pipe1: read stdin from pipe1 read end
        dup2(pipe1[0], STDIN_FILENO);

        // Putting kitty -3's stdout on pipe2: write stdout to pipe2 write end
        dup2(pipe2[1], STDOUT_FILENO);

        // Close all file descriptors except stdin, stdout
        close_fds();

        // Execute kitty -3
        exec_kitty("-3");
    }

    // Create third child process: kitty -4 > pipe2
    pid_t pid_kitty_4 = fork_child();
    if (pid_kitty_4 == 0)
    {
        // Set environment variables for kitty -4
        set_env(4);

        // cat-herder outputfile
        const char *outputfile = argv[2];

        // Open outputfile for writing the output of kitty -4
        int fdout = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fdout < 0)
        {
            perror("open outputfile");
            exit(1);
        }

        // Putting kitty -4's stdout on the output file
        dup2(fdout, STDOUT_FILENO);

        // Putting kitty -4's stdin on pipe2 stdout: read stdin from pipe2 read end
        dup2(pipe2[0], STDIN_FILENO);

        // Close all file descriptors except stdin, stdout
        close_fds();

        // Execute kitty -4
        exec_kitty("-4");
    }

    // Close pipe file descriptors in the parent process
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    // Wait for all child processes to exit with status 0 before exiting the parent process
    wait_child(pid_kitty_2);
    wait_child(pid_kitty_3);
    wait_child(pid_kitty_4);

    // exit the parent process with status 0 if all child processes exit with status 0
    exit(0);
}
