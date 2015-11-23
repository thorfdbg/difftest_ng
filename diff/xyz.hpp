/*************************************************************************
** Copyright (c) 2011-2014 Accusoft Corporation                         **
**                                                                      **
** Written by Thomas Richter (richter@rus.uni-stuttgart.de)             **
** Sponsored by Accusoft Corporation, Tampa, FL and                     **
** the Computing Center of the University of Stuttgart                  **
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
** $Id: xyz.hpp,v 1.2 2014/11/20 20:05:06 thor Exp $
**
** This class converts between RGB and YCbCr signals
*/

#ifndef DIFF_XYZ_HPP
#define DIFF_XYZ_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class XYZ
// This class converts images between rgb and xyz data.
class XYZ : public Meter {
  // 
public:
  //
  // Conversion directions.
  enum ConversionDirection {
    RGBtoXYZ,
    XYZtoLMS,
    RGBtoLMS
  };
  //
private:
  //
  // This bool is set for backwards conversion, i.e. YCbCr->RGB
  bool                m_bInverse;
  //
  // The directions for the conversion.
  ConversionDirection m_Direction;
  //
  // Forwards conversion
  template<typename S>
  static void Multiply(S *r,S *g,S *b,
		       double min,double max,
		       ULONG bppr,ULONG bppg,ULONG bppb,
		       ULONG bprr,ULONG bprg,ULONG bprb,
		       ULONG w, ULONG h,
		       const double matrix[9]);  
  //
  // Conversion with a common matrix.
  void Multiply(class ImageLayout *img);
  //
public:
  //
  // Forwards or backwards conversion to and from XYZ
  XYZ(ConversionDirection direction,bool inverse)
    : m_bInverse(inverse), m_Direction(direction)
  {
  }
  //
  virtual ~XYZ(void)
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
