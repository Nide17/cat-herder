## cat-herder

__INTRODUCTION__

This repository contains a C program called "cat-herder" that mimics a shell pipeline using fork and exec in a contrived scenario called "cat herding". The program creates three child processes, modifies their file descriptors as required, modifies their environment, waits for all three children to exit, and then exits itself. The exit code from cat-herder should be 0 if all three children exit with code 0; otherwise, cat-herder should exit with code 1

__DESCRIPTION__

In this scenario, there is a provided program called "kitty", which resembles the standard Linux utility cat but with a few exceptions:

- kitty only operates on standard input (stdin) and standard output (stdout); it does not accept file names as command-line arguments.
- kitty ensures that it has open files only for stdin, stdout, and stderr.
- kitty checks for the presence or absence of certain environment variables, depending on its argument (kitty -2, kitty -3, or kitty -4).

The objective of the cat-herder program is to execute a sequence of kitty processes to copy the content of an input file to an output file, adhering to the specific requirements outlined below.

__EXECUTION__

cat-herder operates by creating three child processes that execute kitty instances with different arguments and settings:

**kitty -2:**

Ensures that the child's environment includes the environment variable CATFOOD=yummy.

**kitty -3:**

Ensures that the child's environment is the same as the parent's environment, except that the environment variable KITTYLITTER should not be present in the child's environment, even if it is present when cat-herder is invoked.

**kitty -4:**

Creates a limited environment with only three environment variables: PATH and HOME (set to their values in the parent's environment), and CATFOOD=yummy.

The three kitty processes are set up in a pipeline as follows:

```shell
kitty -2 < (inputfile) | kitty -3 | kitty -4 > (outputfile)
```

Where inputfile is copied to outputfile through this pipeline.

__IMPLEMENTATION__

The cat-herder program is implemented in C and uses the system functions to achieve the assignment's objectives. Below are the key components of the code:

- **fork_child()**: Forks a child process and returns the child's PID to the parent.
- **set_env()**: Sets environment variables based on the process number (2, 3, or 4).
- **exec_kitty()**: Executes the kitty process with the provided argument.
- **close_fds()**: Closes all open file descriptors except stdin and stdout.
- **wait_child()**: Waits for a child process to exit with status 0 and handles any errors.
- **main()**: The main function of cat-herder, responsible for setting up the pipeline of kitty processes, managing environment variables, and performing error handling.

__COMPILATION__

Compile the cat-herder and kitty source codes using a C compiler. For example, if you have GCC installed, you can compile as follows:

```shell
gcc -Wall -Werror kitty.c -o kitty && gcc -Wall -Werror cat-herder.c -o cat-herder
```
To run cat-herder, run the compiled programs, providing the input and output file as command-line arguments:

```shell
./cat-herder <inputfile> <outputfile>
```

The program will execute the three kitty processes and copy the content from inputfile to outputfile. It will exit with a status of 0 if all child processes exit successfully.

- inputfile: The input file whose contents will be copied.
- outputfile: The output file where the contents of inputfile will be written.

Where using the same file for both inputfile and outputfile is considered an error.

__ERROR CHECKING__

cat-herder performs the following error checks:

- Checks for the correct number of command-line arguments (i.e., inputfile and outputfile).
- Ensures that inputfile and outputfile are not the same file.
- Handles errors during file opening, for example, when opening inputfile and outputfile.

__IMPORTANCE__

The cat-herder program demonstrates the use of fork and exec functions, manipulation of environment variables, and management of file descriptors to create a customized shell pipeline for copying the content of an input file to an output file. It serves as a practical example of process management in a Linux environment and showcases error handling in system programming.

__KEYWORDS__

<mark>ISSE</mark>     <mark>CMU</mark>     <mark>Assignment7</mark>     <mark>cat-herder</mark>     <mark>C Programming</mark>     <mark>Processes</mark>     <mark>Fork/Exec</mark>     <mark>Pipes</mark>

__AUTHOR__

 Written by parmenin (Niyomwungeri Parmenide ISHIMWE) at CMU-Africa - MSIT

__DATE__

 October 18, 2023