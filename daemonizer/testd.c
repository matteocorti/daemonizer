/* Copyright (c) 2008  Matteo Corti
 * This file is part of daemonizer
 *
 * This is a dummy program to test daemonizer
 *
 * You may distribute this file under the terms the Apace 2.0 license
 * License.  See the file COPYING for more information.
 */

/**
 * @file   testd.c
 * @author Matteo Corti
 * @brief  Dummy program to test daemonizer
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main( /*@unused@*/ int argc, /*@unused@*/ char ** argv) {

  int counter = 0;
  
  while (1) {

    fprintf(stdout, "running (out) %i\n", counter);
    fprintf(stderr, "running (err) %i\n", counter++);

    if (fflush(NULL) < 0) {
      fprintf(stderr, "Error flushing output streams\n");
      exit(EXIT_FAILURE);
    }

    (void)sleep(5);

  }

}
