
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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

/** @def TRUE
 * Boolean true
 */
#define TRUE  1

/** @def FALSE
 * Boolean false
 */
#define FALSE 0
