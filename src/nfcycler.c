#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <argp.h>

const char * argp_program_version = "nfcycler 0.0.0";
const char * argp_program_bug_address = "Ernest Wong <ewon521@gmail.com>";

static char doc[] = "nfcycler - executes your shell commands, and pipes its stdout back into its stdin";

static char args_doc[] = "\"your | shell | command\"";

static struct argp_option options[] =
{
  {"quiet",         'q',  0,  0,            "Suppress all output", 0},
  {"silent",        's',  0,  OPTION_ALIAS, 0, 0},
  {"verbose",       'v',  0,  0,            "Produce verbose output", 0},
  {"print-payload", 'p',  0,  0,            "Prints the data from the pipe to stdout", 0},
  { 0 }
};

typedef struct
Arguments
{
  char *args[1];
  bool silent;
  bool verbose;
  bool printPayload;
}
Arguments;
Arguments nfcyclerArgs =
{
  { "" },
  false,
  false,
  false
};

static error_t
parse_opt(int key, char * arg, struct argp_state * state)
{
  Arguments * arguments = state->input;

  switch(key)
  {
  case 'q': case 's':
    arguments->silent = true;
  case 'v':
    arguments->verbose = true;
    break;
  case 'p':
    arguments->printPayload = true;
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >= 1)
    {
      puts("Whoa! Too many arguments.");
      argp_usage(state);
    }
    arguments->args[state->arg_num] = arg;
    break;
  case ARGP_KEY_END:
    if (state->arg_num < 1)
    {
      puts("Missing the command argument");
      argp_usage(state);
    }
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

void parseArguments(int argc, char * argv[])
{
  argp_parse(&argp, argc, argv, 0, 0, &nfcyclerArgs);
}

#define ilog(...) \
  do { \
    if (!nfcyclerArgs.silent) \
    { \
      printf("[nfcycler] "); \
      printf(__VA_ARGS__); \
    } \
  } while (0)

#define dlog(...) \
  do { \
    if (nfcyclerArgs.verbose) \
    { \
      printf("[nfcycler] "); \
      printf(__VA_ARGS__); \
    } \
  } while (0)

#define plog(...) \
  do { \
    if (nfcyclerArgs.printPayload) \
    { \
      printf(__VA_ARGS__); \
      fflush(stdout); \
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

  dlog("Received command: %s\n", nfcyclerArgs.args[0]);

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

    if (execl("/bin/sh", "/bin/sh", "-c", nfcyclerArgs.args[0], (char *)NULL) == -1)
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
      plog("-----------------------------\n");
      plog("%s", buffer);
      plog("-----------------------------\n");

      if (write(fdhead[FD_WRITE_END], buffer, len) == -1)
        perror("Parent write");

      dlog("Finished writing\n");
    }
    ilog("Parent process ended\n");
    exit(EXIT_SUCCESS);
  }
}
