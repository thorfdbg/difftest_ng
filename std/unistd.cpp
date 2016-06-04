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

/// Includes
#include "unistd.hpp"
#include "math.hpp"
///

#ifndef HAVE_SLEEP
#ifdef WIN32
// Comes with its own "sleep" version
#else
unsigned int sleep(unsigned int seconds)
{
  int x;
  
  // Pretty stupid default implementation
  seconds <<= 8;
  while(seconds > 0) {
    double y = 1.1;
    for(x = 0;x < 32767;x++) {
      // Just keep it busy.
      y = pow(y,y);
      if (y > 2.0)
	y /= 2.0;
      
    }
    seconds--;
  }
  return 0;
}
#endif
#endif

  
/// longseek
// A generic 64-bit version of seek
long long longseek(int stream,long long offset,int whence)
{
#if defined(HAVE_LSEEK64)
  return lseek64(stream,offset,whence);
#elif defined(HAVE_LLSEEK)
  return llseek(stream,offset,whence);
#elif defined(HAVE_LSEEK)
  return lseek(stream,offset,whence);
#elif defined(HAVE__LSEEKI64)
  return _lseeki64(stream,offset,whence);
#else
  // No seeking possible.
  return -1;
#endif
}
///
