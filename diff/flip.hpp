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
** $Id: flip.hpp,v 1.3 2016/06/04 10:44:08 thor Exp $
**
** This class flips the image in X or Y direction.
*/

#ifndef DIFF_FLIP_HPP
#define DIFF_FLIP_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Flip
class Flip : public Meter {
  //
  // Flip type
  int m_Type;
  //
  // Templated implementations: Flip horizontally.
  template<typename T>
  void doFlipX(T *org,ULONG obytesperpixel,ULONG obytesperrow,
	       ULONG w,ULONG h);
  //
  // Flip vertically.
  template<typename T>
  void doFlipY(T *org,ULONG obytesperpixel,ULONG obytesperrow,
	       ULONG w,ULONG h);
  //
public:
  //
  // Several options: Flip horizontally or vertically.
  enum Type {
    FlipX,
    FlipY
  };
  //
  Flip(Type t)
    : m_Type(t)
  { }
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
