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
** $Id: clamp.hpp,v 1.1 2017/06/12 14:57:16 thor Exp $
**
** This class clamps the input to a given min and max range.
*/

#ifndef DIFF_CLAMP_HPP
#define DIFF_CLAMP_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Clamp
// This class clamps the input and output to a given range.
class Clamp : public Meter {
  //
private:
  //
  // Minimum and maximum for floating point operation
  DOUBLE         m_dMin;
  DOUBLE         m_dMax;
  //
  template<typename S>
  void ClampRange(S *org ,ULONG obytesperpixel,ULONG obytesperrow,
		  S min, S max, ULONG w, ULONG h);
  //
  // Apply clamping to the given image.
  void ApplyClamping(class ImageLayout *src);
  //
  //
public:
  //
  // Clamp the image given a floating point difference minimum and maximum
  // Scale the difference image. Takes a file name.
  Clamp(DOUBLE min,DOUBLE max)
    : m_dMin(min), m_dMax(max)
  {
  }
  //
  virtual ~Clamp(void)
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
