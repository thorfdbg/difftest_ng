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
** $Id: suppress.hpp,v 1.1 2019/07/24 10:45:06 thor Exp $
**
** This class suppresses (resets to the source) those pixels in
** the destination that are less than a threshold away from the
** source.
*/

#ifndef DIFF_SUPPRESS_HPP
#define DIFF_SUPRESS_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Suppress
// This class suppresses (resets to the source) those pixels in
// the destination that are less than a threshold away from the
// source.
class Suppress : public Meter {
  //
  // The suppression threshold.
  DOUBLE                 m_dThres;
  //
  template<typename T>
  static void Threshold(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
			T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
			T thres,ULONG w,ULONG h);
  //
public:
  //
  // Construct the difference image. Takes a file name.
  Suppress(DOUBLE thres)
    : m_dThres(thres)
  {
  }
  //
  virtual ~Suppress(void)
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

