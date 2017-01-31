/*************************************************************************
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
** $Id: shift.hpp,v 1.4 2017/01/31 11:58:04 thor Exp $
**
** This class shifts images in X or Y direction.
*/

#ifndef DIFF_SHIFT_HPP
#define DIFF_SHIFT_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imgspecs.hpp"
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
  // Output specifications of the destination file.
  const struct ImgSpecs &m_TargetSpecs;
  //
  // Templated implementations: Shift the image horizontally to the right
  template<typename T>
  static void shiftRight(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx);
  //
  template<typename T>
  static void shiftLeft(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx);
  //
  template<typename T>
  static void shiftDown(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dy);
  //
  template<typename T>
  static void shiftUp(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dy);
  //
  template<typename T>
  void shift(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx,int dy) const
  {
    if (dx > 0) {
      if (ULONG(dx) > w)
	dx = w;
      shiftRight(org,boundary,obytesperpixel,obytesperrow,w,h,dx);
    } else if (dx < 0) {
      dx = -dx;
      if (ULONG(dx) > w)
	dx = w;
      shiftLeft(org,boundary,obytesperpixel,obytesperrow,w,h,dx);
    }
    if (dy > 0) {
      if (ULONG(dy) > h)
	dy = h;
      shiftDown(org,boundary,obytesperpixel,obytesperrow,w,h,dy);
    } else if (dy < 0) {
      dy = -dy;
      if (ULONG(dy) > h)
	dy = h;
      shiftUp(org,boundary,obytesperpixel,obytesperrow,w,h,dy);
    }
  }
  //
  void shift(class ImageLayout *src) const;
  //
public:
  //
  //
  Shift(int deltax,int deltay,const struct ImgSpecs &specs)
    : dx(deltax), dy(deltay),
      m_TargetSpecs(specs)
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
