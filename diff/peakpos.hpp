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
** $Id: peakpos.hpp,v 1.4 2016/06/04 10:44:09 thor Exp $
**
** This class finds the pixel position of the peak error.
*/

#ifndef DIFF_PEAKPOS_HPP
#define DIFF_PEAKPOS_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class PeakPos
class PeakPos : public Meter {
  //
  //
  struct Pixel {
    int x,y;
    double error;
  };
  //
  // PRE measurement type
  int m_Type;
  //
  // Templated implementations
  template<typename T>
  Pixel PeakError(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		  T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		  ULONG w,ULONG h);
public:
  //
  // Several options: X and Y position.
  enum Type {
    PeakX,
    PeakY
  };
  //
  PeakPos(Type t)
    : m_Type(t)
  { }
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    return "PeakPosition";
  }
};
///

///
#endif
