/*************************************************************************
** Copyright (c) 2011-2014 Accusoft Corporation                         **
**                                                                      **
** Written by Thomas Richter (richter@rus.uni-stuttgart.de)             **
** Sponsored by Accusoft Corporation, Tampa, FL and                     **
** the Computing Center of the University of Stuttgart                  **
**************************************************************************

This source file is part of difftest_ng, a universal image measuring
and conversion framework.

    difftest_ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    difftest_ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with difftest_ng.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

/*
** This is an Os abstraction of the stdlib
** include file. It might possibly contain fixes for
** various Os derivations from the intended stdlib.
**
** $Id: unistd.hpp,v 1.2 2014/01/04 11:35:29 thor Exp $
*/

#ifndef UNISTD_HPP
#define UNISTD_HPP
#include "config.h"

#if HAVE_BSTRINGS_H
#include <bstrings.h>
#elif HAVE_BSTRING_H
#include <bstring.h>
#endif

#if HAVE_UNISTD_H

#if defined(HAVE_LLSEEK) || defined(HAVE_LSEEK64)
# ifndef _LARGEFILE64_SOURCE
#  define _LARGEFILE64_SOURCE
# endif
# include <sys/types.h>
#endif

#include <unistd.h>
// *ix compatibility hack, or rather, win compatibility hack
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
#elif HAVE_IO_H
#include <io.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#if !defined(HAVE_OPEN) || !defined(HAVE_CLOSE) || !defined(HAVE_READ) || !defined(HAVE_WRITE)
#error "POSIX io functions are not available, won't compile without"
#endif

#ifndef HAVE_SLEEP
// Dummy implemented insite.
unsigned int sleep(unsigned int seconds);
#endif

// Provide definitions for STDIN_FILENO etc if not available.
#ifndef HAS_STDIN_FILENO
# define STDIN_FILENO 0
#endif

#ifndef HAS_STDOUT_FILENO
# define STDOUT_FILENO 1
#endif

#ifndef HAS_STDERR_FILENO
# define STDERR_FILENO 2
#endif

//
// A generic 64-bit version of seek
extern long long longseek(int fd,long long offset,int whence);

#endif
