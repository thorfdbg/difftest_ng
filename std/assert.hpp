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
** An abstraction layer around the standard assert call. This also
** translates the j2k CHECK_LEVEL into the apropriate macros for assert().
**
** $Id: assert.hpp,v 1.2 2014/01/04 11:35:29 thor Exp $
*/

#ifndef STD_ASSERT_HPP
#define STD_ASSERT_HPP

# if HAVE_ASSERT_H
#  if CHECK_LEVEL > 0
#   undef NDEBUG
#   include <assert.h>
#  else
#   define assert(x)
#  endif
# else
#  if CHECK_LEVEL > 0
#   define assert(x)  { if(!(x)) {volatile char *x = NULL; *x = 0;}}
#  else
#   define assert(x)
#  endif
# endif
#endif
