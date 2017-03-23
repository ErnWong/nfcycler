#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <sysexits.h>
#include "commander/commander.h"


static bool gVerbose = false;
static bool gQuiet = false;
static bool gPrintPayload = false;
static char * gChildCmd = "";

static pid_t childpid = 0;


#define CHILDCMD_LEN 262144
#define FD_READ_END 0
#define FD_WRITE_END 1

#define UNUSED(x) (void)(x)
#define ilog(...) \
  do { \
    if (!gQuiet) \
    { \
      fprintf(stderr, "[nfcycler] "); \
      fprintf(stderr, __VA_ARGS__); \
    } \
  } while (0)

#define dlog(...) \
  do { \
    if (gVerbose) \
    { \
      fprintf(stderr, "[nfcycler] "); \
      fprintf(stderr, __VA_ARGS__); \
    } \
  } while (0)

#define plog(...) \
  do { \
    if (gPrintPayload) \
    { \
      printf(__VA_ARGS__); \
      fflush(stdout); \
    } \
  } while (0)

static void
optUsage(command_t * cmd)
{
  UNUSED(cmd);
  printf("Usage: nfcycler [-ipquvV] [--print-payload] [--quiet] [--verbose]\n");
  printf("            [--help] [--usage] [--version] \"your | shell | command\"\n");
  exit(0);
}

static void
optVerbose(command_t * cmd)
{
  UNUSED(cmd);
  gVerbose = true;
}

static void
optQuiet(command_t * cmd)
{
  UNUSED(cmd);
  gQuiet = true;
}

static void
optPrintPayload(command_t * cmd)
{
  UNUSED(cmd);
  gPrintPayload = true;
}

static void
optInit(command_t * cmd)
{
  UNUSED(cmd);
  fprintf(stderr, "Sorry, --init is not supported yet\n");
  exit(EX_UNAVAILABLE);
}


void parseArguments(int argc, char * argv[])
{
  command_t cmd;
  command_init(&cmd, "nfcycler", "0.0.1");
  cmd.usage = "[options] <shell command>";
  command_option(&cmd, "-u", "--usage", "give a short usage message", optUsage);
  command_option(&cmd, "-v", "--verbose", "be crazy and log everything", optVerbose);
  command_option(&cmd, "-q", "--quiet", "suppress informative logs", optQuiet);
  command_option(&cmd, "-p", "--print-payload", "log the pipe's value real time", optPrintPayload);
  command_option(&cmd, "-i", "--init [command]", "supply a payload initialisation command", optInit);
  command_parse(&cmd, argc, argv);

  if (1 > cmd.argc)
  {
    fprintf(stderr, "Missing shell command argument\n");
    fprintf(stderr, "Try `nfcycler --help' or `nfcycler --usage' for more info\n");
    exit(EX_USAGE);
  }
  else if (1 < cmd.argc)
  {
    fprintf(stderr, "Too many arguments\n");
    fprintf(stderr, "Try `nfcycler --help' or `nfcycler --usage' for more info\n");
    exit(EX_USAGE);
  }
  gChildCmd = malloc(strlen(cmd.argv[0]));
  strcpy(gChildCmd, cmd.argv[0]);

  command_free(&cmd);
}

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
  ilog("nfcycler is closing\n");
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
  parseArguments(argc, argv);
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
  char buffer[128] = {0};

  dlog("Received command: %s\n", gChildCmd);

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

    ilog("Child Started.\n");
    ilog("If no errors pop up, nfcycler is ready and processing...\n");

    if (dup2(fdhead[FD_READ_END], STDIN_FILENO) == -1)
      perror("child dup2 fdhead[read] <---> stdin");

    if (dup2(fdtail[FD_WRITE_END], STDOUT_FILENO) == -1)
      perror("child dup2 fdtail[write] <---> stdout");

    close(fdhead[FD_READ_END]);
    close(fdtail[FD_WRITE_END]);

    if (execl("/bin/sh", "/bin/sh", "-c", gChildCmd, (char *)NULL) == -1)
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
      dlog("Attempting to write: %s", buffer);
      plog("%s", buffer);

      if (write(fdhead[FD_WRITE_END], buffer, len) == -1)
        perror("Parent write");

      dlog("Finished writing\n");
    }
    ilog("Parent process ended\n");
    exit(EXIT_SUCCESS);
  }
}
