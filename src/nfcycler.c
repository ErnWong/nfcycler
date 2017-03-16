#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#define DLOG 1
#define dlog(...) \
  do { \
    if (DLOG) \
    { \
      printf("[nfcycler] "); \
      printf(__VA_ARGS__); \
    } \
  } while (0)

#define CHILDCMD_LEN 262144
#define FD_READ_END 0
#define FD_WRITE_END 1

pid_t childpid = 0;

void exitCleanup()
{
  if (childpid == 0)
  {
    dlog("Child is closing\n");
    return;
  }
  dlog("Killing Child\n");
  kill(childpid, SIGTERM);
}

void sigExitCleanly()
{
  dlog("Quitting cleanly\n");
  exit(EXIT_SUCCESS);
}

void setupHandlers()
{
  dlog("Registering signal handlers\n");
  atexit(exitCleanup);

  struct sigaction exitAction;
  exitAction.sa_handler = sigExitCleanly;
  sigemptyset(&exitAction.sa_mask);
  exitAction.sa_flags = 0;
  sigaction(SIGTERM, &exitAction, NULL);
  sigaction(SIGINT, &exitAction, NULL);
  sigaction(SIGHUP, &exitAction, NULL);
}

int main(int argc, char * argv[])
{
  setupHandlers();

  //           _______________
  //    head  |               |
  //   .----> | child process | ---->.
  //   |      |_______________| tail |
  //   |                             |
  //   `---<------<------<------<----`
  //              (nfcycler)

  int fdhead[2];
  int fdtail[2];
  char childcmd[CHILDCMD_LEN] = {0};
  char buffer[128] = {0};

  int cmdLength = 0;
  for (int i = 1; i < argc; i++)
  {
    int sizeLeft = CHILDCMD_LEN - cmdLength;
    int argSize = snprintf(childcmd + cmdLength, sizeLeft, "%s ", argv[i]);
    if (argSize > sizeLeft)
    {
      fprintf(stderr, "whoa, your command is a bit too long!\n");
      exit(EXIT_FAILURE);
    }
    cmdLength += argSize;
  }
  dlog("Received command: %s\n", childcmd);

  pipe(fdhead);
  pipe(fdtail);
  if ((childpid = fork()) == -1)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (childpid == 0)
  {
    dlog("Hi from child\n");

    close(fdhead[FD_WRITE_END]);
    close(fdtail[FD_READ_END]);

    if (dup2(fdhead[FD_READ_END], STDIN_FILENO) == -1)
      perror("child dup2 fdhead[read] <---> stdin");

    if (dup2(fdtail[FD_WRITE_END], STDOUT_FILENO) == -1)
      perror("child dup2 fdtail[write] <---> stdout");

    if (execl("/bin/sh", "/bin/sh", "-c", childcmd, (char *)NULL) == -1)
      perror("child execl");
  }
  else
  {
    dlog("Hi from parent\n");

    close(fdhead[FD_READ_END]);
    close(fdtail[FD_WRITE_END]);

    while (true)
    {
      dlog("Waiting for input\n");
      int len = read(fdtail[FD_READ_END], buffer, sizeof(buffer));

      dlog("Got input!\n");
      if (len == -1)
        perror("Parent read");

      dlog("Length: %d\n", len);
      dlog("Attempting to write: %s\n", buffer);

      if (write(fdhead[FD_WRITE_END], buffer, len) == -1)
        perror("Parent write");

      dlog("Finished writing\n");
    }
    puts("Parent process ended");
    exit(EXIT_SUCCESS);
  }
}
