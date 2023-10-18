/*
 * kitty.c
 *
 * A somewhat contrived replacement for 'cat', for use with ISSE Assignment 7.
 *
 * kitty will copy its stdin to stdout, just like cat.
 *
 * In addition, upon startup kitty will check its open file
 * descriptors. Kitty expects to have been started with only stdin,
 * stdout, and stderr open. If other file descriptors are open, kitty
 * will print a diagnostic to stderr and terminate with a non-zero
 * exit code.
 *
 * Finally, kitty expects to receive a single command line argument in
 * the form -n, with n as a small integer. See the usage message for
 * details.
 *
 * Author: Howdy Pierce <howdy@sleepymoose.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define EXE_NAME "kitty"
#define CATFOOD "CATFOOD"
#define CATFOOD_EXP_VAL "yummy"
#define KITTYLITTER "KITTYLITTER"

/*
 * Prints a usage message to stderr, and exits
 *
 * Does not return
 */
void usage()
{
  fprintf(stderr, "Usage: " EXE_NAME " -n\n\n");
  fprintf(stderr, "where n is one of the following integers:\n");
  fprintf(stderr, "0  Perform no environment or file descriptor checks\n");
  fprintf(stderr, "1  Perform no environment checks\n");
  fprintf(stderr, "2  Ensure that the env variable " CATFOOD " is present and set to 'yummy'\n");
  fprintf(stderr, "3  Ensure that the env variable " KITTYLITTER " is _not_ set\n");
  fprintf(stderr, "4  Ensure that the environment contains _only_ the variables PATH, \n");
  fprintf(stderr, "   HOME and " CATFOOD "\n");
  fprintf(stderr, "5  Force a nonzero error exit, without copying anything\n\n");
  fprintf(stderr, "To prove that it has run, " EXE_NAME " will create the file " EXE_NAME ".n\n");
  fprintf(stderr, "in the current working directory.\n");

  exit(1);
}

/*
 * 'touches' the file ./.EXENAME.pid.suffix
 *
 * Parameters:
 *   suffix   the suffix to append to the filename
 *
 * Returns: If the touch was successful, this function
 * returns. Otherwise it prints an error message and exits.
 */
void touch(const char *suffix)
{
  char fname[32];

  snprintf(fname, sizeof(fname), ".%s.%d.%s", EXE_NAME, getpid(), suffix);

  const int create_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  int fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, create_mode);
  if (fd == -1)
  {
    perror("open");
    exit(1);
  }
  close(fd);
}

/*
 * Return true if the file ./.EXENAME.suffix is present
 *
 * Parameters:
 *   suffix   the suffix to append to the filename
 *
 * Returns: true/false based on presence of the file
 */
bool is_file(const char *suffix)
{
  struct stat s;
  char fname[32];
  snprintf(fname, sizeof(fname), ".%s.%s", EXE_NAME, suffix);

  return (stat(fname, &s) == 0);
}

/*
 * Run through all possible file descriptors, and ensure that none are
 * open except stdin, stdout, and stderr.
 *
 * If other file descriptors are open, print a diagnostic to
 * stderr. In this case, exit(1) unless runlevel = 0, in which case
 * proceed.
 *
 * Parameters:
 *   runlevel   The kitty runlevel
 *
 * Returns: none
 */
void check_fds(int runlevel)
{
  bool error = false;

  for (int i = STDERR_FILENO + 1; i < 1024; i++)
  {
    if (fcntl(i, F_GETFD, 0) != -1)
    {
      fprintf(stderr, EXE_NAME " error: File descriptor %d is open\n", i);
      error = true;
    }
  }

  if (!error)
    touch("fd_ok");

  if (error && runlevel)
    exit(1);
}

/*
 * Performs environment validation according to the runlevel:
 *
 * 0,1  No checks
 * 2    Ensure the environment variable CATFOOD is present and set to 'yummy'
 * 3    Ensure that the environment variable KITTYLITTER is _not_ set
 * 4    Ensure that the environment contains _only_ the variables PATH,
 *       HOME and CATFOOD
 * 5    Force a nonzero error exit, without copying anything
 *
 * Parameters:
 *   runlevel   the kitty runlevel
 *   envp       pointer to all the environment variables
 *
 * Returns: If the environment validation was successful, this
 * function returns. Otherwise it prints an error message and exits.
 */
void check_env(int runlevel, char *envp[])
{
  bool error = false;
  const char *val = NULL;

  int env_size = 0;
  while (envp[env_size])
    env_size++;

  switch (runlevel)
  {
  case 0:
  case 1:
    return;

  case 4:
    if (env_size != 3)
    {
      fprintf(stderr, EXE_NAME " error: Expected only 3 environment variables, found %d\n",
              env_size);
      error = true;
    }
    // fall through

  case 2:
    val = getenv(CATFOOD);
    if (!val)
    {
      fprintf(stderr, EXE_NAME " error: Expected to find the environment variable " CATFOOD "\n");
      error = true;
    }
    else if (strcmp(val, CATFOOD_EXP_VAL) != 0)
    {
      fprintf(stderr, EXE_NAME " error: Expected to find the environment variable " CATFOOD " set to " CATFOOD_EXP_VAL "\n");
      error = true;
    }

    break;

  case 3:
    val = getenv(KITTYLITTER);
    if (val)
    {
      fprintf(stderr, EXE_NAME " error: Did NOT expect to find the environment variable " KITTYLITTER "\n");
      error = true;
    }
    break;
  }

  if (error)
    exit(1);

  touch("env_ok");

  return;
}

int main(int argc, char *argv[], char *envp[])
{
  int runlevel;
  int c;

  if (argc != 2)
    usage();

  runlevel = argv[1][1] - '0';
  if (runlevel < 0 || runlevel > 5)
    usage();

  if (runlevel == 5 || (runlevel == 3 && is_file("force_exit")))
  {
    fprintf(stderr, EXE_NAME " error: Forcing an error exit\n");
    exit(1);
  }

  touch("launch");

  check_fds(runlevel);

  check_env(runlevel, envp);

  while ((c = fgetc(stdin)) != EOF)
    if (fputc(c, stdout) == EOF)
    {
      perror("fputc");
      exit(1);
    }

  touch("eof_ok");

  return 0;
}
