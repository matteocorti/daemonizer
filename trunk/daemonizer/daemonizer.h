
/*
 * This file is part of daemonizer and is released under the Apache 2.0 license
 * plese see the REAME, AUTHORS and COPYING files
 */

/*

RCS information
enable substitution with:
 $ svn propset svn:keywords "Id Revision HeadURL Source Date"

 $Id$
 $Revision$
 $HeadURL$
 $Date: 2008-03-21 12:36:09 +0100 (Fri, 21 Mar 2008)$
 
*/

/**
 * @file   daemonizer.h
 * @author John Kelly, Matteo Corti
 * @brief  the main daemonizer program file
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
