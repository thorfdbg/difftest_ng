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
**
** $Id: shift.hpp,v 1.1 2016/10/31 14:55:24 thor Exp $
**
** This class shifts images in X or Y direction.
*/

#ifndef DIFF_SHIFT_HPP
#define DIFF_SHIFT_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Shift
class Shift : public Meter {
  //
  // Shift direction.
  int dx;
  int dy;
  //
  // Templated implementations: Shift the image horizontally to the right
  template<typename T>
  static void shiftRight(T *org,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx);
  //
  template<typename T>
  static void shiftLeft(T *org,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx);
  //
  template<typename T>
  static void shiftDown(T *org,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dy);
  //
  template<typename T>
  static void shiftUp(T *org,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dy);
  //
  template<typename T>
  void shift(T *org,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx,int dy)
  {
    if (dx > 0) {
      if (ULONG(dx) > w)
	dx = w;
      shiftRight(org,obytesperpixel,obytesperrow,w,h,dx);
    } else if (dx < 0) {
      dx = -dx;
      if (ULONG(dx) > w)
	dx = w;
      shiftLeft(org,obytesperpixel,obytesperrow,w,h,dx);
    }
    if (dy > 0) {
      if (ULONG(dy) > h)
	dy = h;
      shiftDown(org,obytesperpixel,obytesperrow,w,h,dy);
    } else if (dy < 0) {
      dy = -dy;
      if (ULONG(dy) > h)
	dy = h;
      shiftUp(org,obytesperpixel,obytesperrow,w,h,dy);
    }
  }
  //
  void shift(class ImageLayout *src);
  //
public:
  //
  //
  Shift(int deltax,int deltay)
    : dx(deltax), dy(deltay)
  {
  }
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    return NULL;
  }
};
///

///
#endif
