/*
  Define author
  John Kelly, October 6, 2007
  Matteo Corti, March, 2008

  Define copyright
  Copyright John Kelly, 2007. All rights reserved.

  Define license
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this work except in compliance with the License.
  You may obtain a copy of the License at:
  http://www.apache.org/licenses/LICENSE-2.0

  Define symbols and (words)
  XC ...........  eXeCutable
  rv ...........  return value
  tx ...........  temporary index
  ts ...........  temporary string
*/

#include "daemonizer.h"

#define LOGTIME(ts) logtime ((ts), sizeof ((ts)))

static int verbose_flag = FALSE; /**< command line argument: verbose output */
static int version_flag = FALSE; /**< command line argument: version        */

/**
 * Prints the program's usage
 */
void usage() {
  
  printf("\nusage: %s [OPTION] program [ARGUMENTS]\n\n", PACKAGE_NAME);
  printf("Starts the specified program (full path) with the optional arguments detaching it\n");
  printf("from the current terminal\n\n");
  printf("Options\n");
  printf(" -h, --help         show this help screen\n");
  printf(" -l, --log          log file\n");
  printf(" -v, --verbose      increase verbosity\n");
  printf("     --version      prints the program version and exits\n");
  printf("\nPlease see the %s(1) man page for full documentation\n\n", PACKAGE_NAME);

}

void catchsig (
               int sn
               ) {
  printf ("Caught signal %d\n", sn);
  fflush (NULL);
  if (sn == SIGSEGV) {
    exit (EXIT_FAILURE);
  }
}

char *logtime (
               char *ts,
               size_t sz
               ) {
  int rv;
  time_t es;
  struct tm *dt;

  if (sz < 2) {
    errno = EINVAL;
    perror ("logtime");
    ts = NULL;
  } else {
    es = time (NULL);
    if (!(dt = localtime (&es))) {
      ts[0] = '?';
      ts[1] = '\0';
      errno = EPROTO;
      perror ("logtime");
    } else {
      rv = strftime (ts, sz, "%a %b %d %Y %H:%M:%S", dt);
      if (rv == 0) {
        ts[0] = '?';
        ts[1] = '\0';
        errno = EOVERFLOW;
        perror ("logtime");
      }
    }
  }
  return ts;
}

/**
 * Main program
 * @param argc number of command line arguments
 * @param argv array of strings with the command line arguments
 */
int main ( int argc, char ** argv ) {

  /* the program to daemonize */
  char *  program  = NULL;

  /* the list of command line arguments */
  char ** arguments = NULL;

  /* number of command line arguments */
  int     arguments_number = 0;
  
  /* the optional log file */
  char * log_file = "/var/log/daemonizer";

  /* the search path */
  char * path     = "/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin";
  
  /* helper variable for the command line options processing */
  int    c;

  /* generic counter */
  int   i;
  
  /* Process command line options */
  
  while (TRUE) {

    static struct option long_options[] = {
      {"log",         required_argument, NULL, 'l'},
      {"verbose",     no_argument,       NULL, 'v'},
      {"version",     no_argument,       &version_flag, TRUE},
      {"help",        no_argument,       NULL, 'h'},
      {NULL, 0, NULL, 0}
    };

    /* getopt_long stores the option index here. */
    int option_index = 0;
     
    c = getopt_long (argc, argv, "hvl:",
		     long_options, &option_index);
    
    /* Detect the end of the options. */
    if (c == -1)
      break;
     
    switch (c) {

    case 'v':
      verbose_flag = TRUE;
      break;

    case 'h':
      usage();
      exit(EXIT_SUCCESS);

    case 'l':
      log_file = optarg;
      break;

    case '?':
      usage();
      /* getopt_long already printed an error message. */
      exit(EXIT_SUCCESS);
      
    case 0:
      break;

    default:
      abort ();
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

  if (argc > optind) {
    /* the program has to be called with some command line arguments */
    arguments        = &(argv[optind]);
    arguments_number = argc - optind;    
  }
  
  if (verbose_flag) {
    
    printf("Daemonizing: %s\n", program);
    
    if (arguments) {
      printf("  with %i command line arguments:", arguments_number);
      for (i=0; i < arguments_number; i++) {
        printf(" |%s|", arguments[i]);
      }
    }
    printf("\n");
    
    printf("Logging to:  %s\n",  log_file);
  }

  int flags;
  int status;
  long open_max;
  pid_t pid;
  mode_t mode;
  uint32_t tx;
  char **ev;
  extern char **environ;
  char ts[512];
  struct flock mylock;
  struct sigaction signow;  
    
  errno = 0;
  pid = fork ();
  if (pid < 0) {
    perror ("failure creating 1st child");
    exit (EXIT_FAILURE);
  } else if (pid > 0) {
    if (wait (&status) < 0) {
      perror ("failure waiting for 1st child");
      exit (EXIT_FAILURE);
    }
    _exit (EXIT_SUCCESS);
  } else {
    if (setsid () < 0) {
      perror ("failure starting new session");
      exit (EXIT_FAILURE);
    }
    pid = fork ();
    if (pid < 0) {
      perror ("failure creating 2nd child");
      exit (EXIT_FAILURE);
    } else if (pid > 0) {
      _exit (EXIT_SUCCESS);
    } else {
      umask (00022);
      signow.sa_handler = catchsig;
      sigemptyset (&signow.sa_mask);
      signow.sa_flags = 0;
      if (sigaction (SIGHUP, &signow, NULL) < 0) {
        perror ("failure initializing SIGHUP handler");
        exit (EXIT_FAILURE);
      }
      if (sigaction (SIGINT, &signow, NULL) < 0) {
        perror ("failure initializing SIGINT handler");
        exit (EXIT_FAILURE);
      }
      if (sigaction (SIGQUIT, &signow, NULL) < 0) {
        perror ("failure initializing SIGQUIT handler");
        exit (EXIT_FAILURE);
      }
      if (sigaction (SIGSEGV, &signow, NULL) < 0) {
        perror ("failure initializing SIGSEGV handler");
        exit (EXIT_FAILURE);
      }
      if (chdir ("/") < 0) {
        perror ("failure changing working directory");
        exit (EXIT_FAILURE);
      }
      if ((open_max = sysconf (_SC_OPEN_MAX)) < 0) {
        perror ("failure querying _SC_OPEN_MAX");
        exit (EXIT_FAILURE);
      }
      for (tx = 3; tx < open_max; tx++) {
        if (close (tx) < 0) {
          if (errno == EBADF)
            continue;
          printf ("fd = %u\n", (unsigned int) tx);
          perror ("failure closing file");
          exit (EXIT_FAILURE);
        }
      }
      errno = 0;
      if (close (STDOUT_FILENO) < 0) {
        perror ("failure closing STDOUT");
        exit (EXIT_FAILURE);
      }
      flags = O_WRONLY | O_CREAT;
      mode  = S_IRUSR | S_IWUSR | S_IRGRP;
      if (open (log_file, flags, mode) != STDOUT_FILENO) {
        fprintf (stderr, "failure opening STDOUT to %s\n", log_file);
        exit (EXIT_FAILURE);
      }
      mylock.l_type = F_WRLCK;
      mylock.l_whence = SEEK_SET;
      mylock.l_start = 0;
      mylock.l_len = 1;
      mylock.l_pid = 0;
      if (fcntl (STDOUT_FILENO, F_SETLK, &mylock) < 0) {
        perror ("failure locking LOGFILE");
        exit (EXIT_FAILURE);
      }
      if (ftruncate (STDOUT_FILENO, 0) < 0) {
        perror ("failure clearing LOGFILE");
        exit (EXIT_FAILURE);
      }
      if (lseek (STDOUT_FILENO, 0, SEEK_SET) != 0) {
        perror ("failure initializing LOGFILE");
        exit (EXIT_FAILURE);
      }
      if (!fdopen (STDOUT_FILENO, "a")) {
        perror ("failure opening STDOUT stream");
        exit (EXIT_FAILURE);
      }
      if (close (STDERR_FILENO) < 0) {
        printf ("failure closing STDERR: %s\n", strerror (errno));
        exit (EXIT_FAILURE);
      }
      if (dup2 (STDOUT_FILENO, STDERR_FILENO) != STDERR_FILENO) {
        printf ("failure opening STDERR to LOGFILE: %s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
      }
      if (!fdopen (STDERR_FILENO, "a")) {
        printf ("failure opening STDERR stream: %s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
      }
      if (close (STDIN_FILENO) < 0) {
        printf ("failure closing STDIN: %s\n", strerror (errno));
        exit (EXIT_FAILURE);
      }
      if (open ("/dev/null", O_RDONLY) != STDIN_FILENO) {
        printf ("failure opening STDIN to /dev/null: %s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
      }
      printf ("%s success starting pid %d\n", LOGTIME (ts), getpid ());
      if (fflush (NULL) < 0) {
        printf ("failure flushing STDIO stream: %s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
      }
#ifdef HAVE_CLEARENV            
      if (clearenv () != 0) {
        printf ("failure clearing environment\n");
        exit (EXIT_FAILURE);
      }
#endif
      if (setenv ("PATH", path, 1) < 0) {
        printf ("failure setting PATH: %s\n", strerror (errno));
        exit (EXIT_FAILURE);
      }
      if (setenv ("SHELL", "/bin/sh", 1) < 0) {
        printf ("failure setting SHELL: %s\n", strerror (errno));
        exit (EXIT_FAILURE);
      }
      for (ev = environ; ev && *ev; ev++) {
        printf ("%s %s\n", LOGTIME (ts), *ev);
      }
      if (fflush (NULL) < 0) {
        printf ("failure flushing STDIO stream: %s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
      }
      fprintf(stderr, "execvP(%s, %s, %s, ...)\n", program, path, *arguments);
      if (execvP(program, ts, arguments) < 0) {
        printf ("failure starting %s: %s\n", program, strerror (errno));
        exit (EXIT_FAILURE);
      }
      exit (EXIT_SUCCESS);
    }
  }
}
