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
** This is an Os abstraction of the stddef
** include file. It might possibly contain fixes for
** various Os derivations from the intended stdlib.
**
** $Id: stddef.hpp,v 1.4 2016/06/04 10:44:10 thor Exp $
*/

#ifndef STDDEF_HPP
#define STDDEF_HPP
#include "config.h"

#if defined(HAVE_STDDEF_H) && defined(HAS_PTRDIFF_T)
#include <stddef.h>
#else
typedef long ptrdiff_t;
#endif

#endif
