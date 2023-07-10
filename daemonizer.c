
/*
 * This file is part of daemonizer and is released under the Apache 2.0 license
 * please see the REAME, AUTHORS and COPYING files
 */

/**
 * @file   daemonizer.c
 * @author John Kelly, Matteo Corti
 * @brief  the main daemonizer program file
 */

#include "daemonizer.h"

static int keepenv_flag = FALSE;  /**< keep environment variables   */

static int version_flag = FALSE;  /**< version */

/**
 * Prints the program's usage
 */
static void usage()
{

  printf("\nusage: %s [OPTION] program [ARGUMENTS]\n\n", PACKAGE_NAME);
  printf
    ("Starts the specified program with the optional arguments detaching it\n");
  printf("from the current terminal\n\n");
  printf("Options\n");
  printf(" -e, --keep-environment   keeps environment variables\n");
  printf(" -h, --help               show this help screen\n");
  printf(" -l, --log=file           log file\n");
  printf(" -p, --pid=file           PID file\n");
  printf("     --version            prints the program version and exits\n");
  printf("\nPlease see the %s(1) man page for full documentation\n\n",
	 PACKAGE_NAME);

}

/** Default signal handler
 * @param   number  signal number
 */
static void catchsig(int signal_number)
{
  printf("Caught signal %d\n", signal_number);
  if (fflush(NULL) < 0) {
    fprintf(stderr, "Error: failure flushing STDIO stream: %s\n",
	    strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (signal_number == SIGSEGV) {
    exit(EXIT_FAILURE);
  }
}


/** Generates a string with the date and time
 * @param   string  string which will hold the timestamp
 * @param   len     length of the string
 * @return          the string with the timestamp
 */
static /* @null@ */ char *logtime( /* @out@ */ char *string, size_t len)
{

  size_t written_chars;
  time_t epoch;
  struct tm *broken_down_time;

  if (len < 2) {
    errno = EINVAL;
    perror("logtime");
    string = NULL;
  } else {
    epoch = time(NULL);
    if (!(broken_down_time = localtime(&epoch))) {
      string[0] = '?';
      string[1] = '\0';
      errno = EPROTO;
      perror("logtime");
    } else {
      written_chars =
	strftime(string, len, "%a %b %d %Y %H:%M:%S", broken_down_time);
      if (written_chars == 0) {
	string[0] = '?';
	string[1] = '\0';
	errno = EOVERFLOW;
	perror("logtime");
      }
    }
  }
  return string;
}

/**
 * Main program
 * @param argc number of command line arguments
 * @param argv array of strings with the command line arguments
 */
int main(int argc, char **argv)
{

/* the program to daemonize */
  char *program = NULL;

/* the list of command line arguments */
  char **arguments = NULL;

/* the optional log file */
  static char *log_file = "/var/log/daemonizer";
  
/* the optional pid file */
  static char *pid_file;

/* the file handler for the pid file */
  FILE * pid_file_stream;

/* the search path */
  char *path = "/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin";

/* helper variable for the command line options processing */
  int c;

/* process ID: used to store the return value of fork */
  pid_t pid;

/* process status (written by wait(2) */
  int status;

/* generic file descriptor */
  int fd;

/* string to hold the timestamps */
  char timestamp[512];

/* timeout for the wait ppid hack: see comment below */
  int timeout;

/* ppid hack: see comment below */
  pid_t ppid;

  int flags;
  int open_max;
  mode_t mode;
  char **ev;
  extern char **environ;
  struct flock mylock;
  struct sigaction signow;

/* Process command line options */

  for (;;) {

    static struct option long_options[] = {
      {"keep-environment", no_argument, NULL, (int)'e'},
      {"log", required_argument, NULL, (int)'l'},
      {"pid", required_argument, NULL, (int)'p'},
      {"version", no_argument, &version_flag, TRUE},
      {"help", no_argument, NULL, (int)'h'},
      {NULL, 0, NULL, 0}
    };

/* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, "ehl:p:", long_options, &option_index);

/* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {

    case 'e':
      keepenv_flag = TRUE;
      break;

    case 'h':
      usage();
      exit(EXIT_SUCCESS);

    case 'l':
      log_file = optarg;
      break;

    case 'p':
      pid_file = optarg;
      break;

    case '?':
      usage();

/* getopt_long already printed an error message. */
      exit(EXIT_SUCCESS);

    case 0:
      break;

    default:
      fprintf(stderr, "Internal error: cannot process option\n");
      abort();
    }

  }

/* Check the number of arguments */

  if ((argc - optind) < 1) {
    fprintf(stderr, "Error: No program specified\n");
    usage();
    exit(EXIT_FAILURE);
  }

  program = malloc(strlen(argv[optind]));
  if (!program) {
    fprintf(stderr, "Error: Out of memory\n");
    exit(EXIT_FAILURE);
  }
  strcpy(program, argv[optind++]);

/* build the argument list */

  if (argc > optind) {

/* the program has to be called with some command line arguments * (the first 
 * param, arguments[0] is the program itself) */
    arguments = &(argv[optind - 1]);
  }

  errno = 0;

/* fork 1 */
  pid = fork();

  if (pid < 0) {

/* fork 1: error */

    perror("failure creating 1st child");
    exit(EXIT_FAILURE);

  } else if (pid > 0) {

/* fork 1: parent process */

    if (waitpid(pid, &status, 0) < 0) {
      perror("failure waiting for 1st child");
      exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);

  } else {

/* fork 1: child process */

/* create a new session (see setsid(2)) */

    if (setsid() < 0) {
      perror("failure starting new session");
      exit(EXIT_FAILURE);
    }

/* fork 2 */
    pid = fork();

    if (pid < 0) {

/* fork 2: error */

      perror("failure creating 2nd child");
      exit(EXIT_FAILURE);

    } else if (pid > 0) {

/* fork 2: parent process */

      _exit(EXIT_SUCCESS);

    } else {

/* fork 2: child process */

/* set the umask: write access for the owner only 022 */
      (void)umask(S_IWGRP | S_IWOTH);

/* signal handlers */

      signow.sa_handler = catchsig;
      if (sigemptyset(&signow.sa_mask) < 0) {
	perror("failure initializing signal set");
	exit(EXIT_FAILURE);
      }
      signow.sa_flags = 0;
      if (sigaction(SIGHUP, &signow, NULL) < 0) {
	perror("failure initializing SIGHUP handler");
	exit(EXIT_FAILURE);
      }
      if (sigaction(SIGINT, &signow, NULL) < 0) {
	perror("failure initializing SIGINT handler");
	exit(EXIT_FAILURE);
      }
      if (sigaction(SIGQUIT, &signow, NULL) < 0) {
	perror("failure initializing SIGQUIT handler");
	exit(EXIT_FAILURE);
      }
      if (sigaction(SIGSEGV, &signow, NULL) < 0) {
	perror("failure initializing SIGSEGV handler");
	exit(EXIT_FAILURE);
      }

/* get the maximum number of open files per user id */
      if ((open_max = (int)sysconf(_SC_OPEN_MAX)) < 0) {
	perror("failure querying _SC_OPEN_MAX");
	exit(EXIT_FAILURE);
      }

/* close all the possible file descriptors */
      for (fd = 3; fd < open_max; fd++) {
	if (close(fd) < 0) {
	  if (errno == EBADF) {

/* bad file descriptor */
	    continue;
	  }
	  printf("fd = %u\n", (unsigned int)fd);
	  perror("failure closing file");
	  exit(EXIT_FAILURE);
	}
      }

      errno = 0;

/* close STDOUT */
      if (close(STDOUT_FILENO) < 0) {
	perror("failure closing STDOUT");
	exit(EXIT_FAILURE);
      }

/* open standard out to the log file */
      flags = O_WRONLY | O_APPEND | O_CREAT;
      mode = S_IRUSR | S_IWUSR | S_IRGRP;
      if (open(log_file, flags, mode) != STDOUT_FILENO) {
	fprintf(stderr, "failure opening STDOUT to %s\n", log_file);
	exit(EXIT_FAILURE);
      }

/* lock the log file */
      mylock.l_type = (short int)F_WRLCK;
      mylock.l_whence = (short int)SEEK_SET;
      mylock.l_start = 0;
      mylock.l_len = 1;
      mylock.l_pid = 0;
      if (fcntl(STDOUT_FILENO, F_SETLK, &mylock) < 0) {
	perror("failure locking LOGFILE");
	exit(EXIT_FAILURE);
      }

/* open STDOUT for append */
      if (!fdopen(STDOUT_FILENO, "a")) {
	perror("failure opening STDOUT stream");
	exit(EXIT_FAILURE);
      }

/* close STDERR */
      if (close(STDERR_FILENO) < 0) {
	printf("failure closing STDERR: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

/* open STDERR to the log file */
      if (dup2(STDOUT_FILENO, STDERR_FILENO) != STDERR_FILENO) {
	printf("failure opening STDERR to LOGFILE: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

/* open STDERR for append */
      if (!fdopen(STDERR_FILENO, "a")) {
	printf("failure opening STDERR stream: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

/* close STDIN */
      if (close(STDIN_FILENO) < 0) {
	printf("failure closing STDIN: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

/* /dev/null to STDIN */
      if (open("/dev/null", O_RDONLY) != STDIN_FILENO) {
	printf("failure opening STDIN to /dev/null: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

/* log: success */
      pid = getpid();
      if (logtime(timestamp, sizeof(timestamp)) != NULL) {
	printf("%s success starting pid %d\n", timestamp, (int)pid);
      } else {
	printf("failure setting generating timestamps\n");
	exit(EXIT_FAILURE);
      }

/* flush all open output streams */
      if (fflush(NULL) < 0) {
	printf("failure flushing STDIO stream: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

      if (keepenv_flag == FALSE) {
#ifdef HAVE_CLEARENV
	if (clearenv() != 0) {
	  printf("failure clearing environment\n");
	  exit(EXIT_FAILURE);
	}
#else

/* from the clearenv man page: If it is unavailable the assignment environ =
 * NULL; will probably do. */
/* Doing so on OS X will cause a segfault - its environ(7) manpage only states:
   Direct access can be made through the global variable environ,
   though it is recommended that changes to the enviromment still be made
   through the environment routines.
*/
#ifdef __APPLE__
  if (environ != NULL) {
    int i = 0;
    for (ev = environ; (ev != NULL) && (*ev != NULL); ev++) {
      char* var = strsep(ev,"=");
      // this should unset the envvar, however environ remains unaffected?
      if (unsetenv(var) != 0) {
        printf("failure clearing environment\n");
        exit(EXIT_FAILURE);
      }
      // ensure it is unset
      environ[i] = NULL;
      i++;
    }
  }
#else
	environ = NULL;
#endif // __APPLE__
#endif // __HAVE_CLEARENV__
      }

/* set the PATH environment variable */
      if (setenv("PATH", path, 1) < 0) {
	printf("failure setting PATH: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

/* set the shell */
      if (setenv("SHELL", "/bin/sh", 1) < 0) {
	printf("failure setting SHELL: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }


/* log the environment variables */
      for (ev = environ; (ev != NULL) && (*ev != NULL); ev++) {
	if (logtime(timestamp, sizeof(timestamp))) {
	  printf("%s %s\n", timestamp, *ev);
	} else {
	  printf("failure setting generating timestamps\n");
	  exit(EXIT_FAILURE);
	}
      }

      if (fflush(NULL) < 0) {
	printf("failure flushing STDIO stream: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
      }

/* 
 * The child of the final fork gets adopted by init, but not
 * immediately. The kernel has to reap the parent first, and in
 * the meantime, the child and parent are racing. So by the time
 * you exec the target daemon, the adoption by init may not be
 * complete, and ppid still shows the id of the dying parent.
 *
 * That has implications for the target daemon.
 * See this debian bug report:
 * http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=416179
 *
 * Hack:
 *   we wait until the ppid is 1 with a timeout check
 */

      timeout = 0;

      ppid = getppid();
      while (ppid != 1 && timeout++ < 10) {
	usleep(20);
	ppid = getppid();
      }

      if (ppid != 1) {
	printf("Child not adoped by init: timeout reached\n");
	exit(EXIT_FAILURE);
      }

      if (pid_file != NULL) {
        
/* write the PID */

        pid_file_stream = fopen(pid_file, "w");
        if (pid_file_stream == NULL) {
          perror("failure opening pid file: ");
          exit(EXIT_FAILURE);
        }

        if (fprintf(pid_file_stream, "%i\n", (int)pid) < 0) {
          perror("failure writing PID to pid file: ");
          exit(EXIT_FAILURE);
        }          
        
        if (fclose(pid_file_stream) != 0) {
          perror("failure closing pid file: ");
          exit(EXIT_FAILURE);
        }
        
      }
      
/* everything seems OK: let's start the daemon */
      
      if (execvp(program, arguments) < 0) {
	printf("failure starting %s: %s\n", program, strerror(errno));
	exit(EXIT_FAILURE);
      }

      exit(EXIT_SUCCESS);
    }
  }
  
}
