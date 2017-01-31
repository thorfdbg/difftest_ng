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
** $Id: mrse.hpp,v 1.6 2017/01/31 11:58:04 thor Exp $
**
** This class measures the mean relative square error between two images, averaged over all samples
** and thus all components.
*/

#ifndef DIFF_MRSE_HPP
#define DIFF_MRSE_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class MRSE
class MRSE : public Meter {
  //
  // PSNR measurement type
  int m_Type;
  //
  // Templated implementations
  template<typename T>
  double Compute(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		 T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		 ULONG w,ULONG h);
public:
  //
  // Several options: Mean MRSE, minimum MRSE, and with YCbCr weights (yuck!)
  enum Type {
    Mean,
    Min,
    YCbCr,
    YUV,
    SamplingWeighted
  };
  //
  MRSE(Type t)
    : m_Type(t)
  { }
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    return "MRSE";
  }
};
///

///
#endif
