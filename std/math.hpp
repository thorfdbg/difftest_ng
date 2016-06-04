/*************************************************************************
** Copyright (c) 2003-2016 Accusoft 				        **
**									**
** Written by Thomas Richter (THOR Software) for Accusoft	        **
** All Rights Reserved							**
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
** $Id: math.hpp,v 1.6 2016/06/04 10:44:10 thor Exp $
*/

#ifndef MATH_HPP
#define MATH_HPP

#if HAVE_MATH_H
#include <math.h>
#ifdef _MSC_VER
# if _MSC_VER <= 1700
#  define isnan(x) ((x) != (x))
#  define isinf(x) (!isnan(x) && isnan((x)-(x)))
# endif
#endif

#else
#error "math.h header not available, won't compile without"
#endif
#endif

