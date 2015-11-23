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
** $Id: ycbcr.hpp,v 1.3 2014/01/04 11:35:28 thor Exp $
**
** This class converts between RGB and YCbCr signals
*/

#ifndef DIFF_YCBCR_HPP
#define DIFF_YCBCR_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class YCbCr
// This class converts images between rgb and ycbcr data.
class YCbCr : public Meter {
  //
  // This bool is set for backwards conversion, i.e. YCbCr->RGB
  bool m_bInverse;
  //
  // This flag is set in case the chroma components shall be made
  // signed.
  bool m_bMakeSigned;
  //
  // Forwards conversion
  template<typename S,typename T>
  static void ToYCbCr(S *r,S *g,S *b,
		      double yoffset,double coffset,
		      double min,double max,
		      double cmin,double cmax,
		      ULONG bppr,ULONG bppg,ULONG bppb,
		      ULONG bprr,ULONG bprg,ULONG bprb,
		      ULONG w, ULONG h);  
  //
  //
  // Backwards conversion.
  template<typename S,typename T>
  static void FromYCbCr(S *y,T *cb,T *cr,
			double yoffset,double coffset,
			double min,double max,
			ULONG bppy,ULONG bppcb,ULONG bppcr,
			ULONG bpry,ULONG bprcb,ULONG bprcr,
			ULONG w, ULONG h);
  //
  // Convert a single image to YCbCr.
  void ToYCbCr(class ImageLayout *img);
  //
  // Convert a single image from YCbCr to RGB
  void FromYCbCr(class ImageLayout *img);
  //
public:
  //
  // Forwards or backwards conversion to and from YCbCr
  YCbCr(bool inverse,bool makesigned)
    : m_bInverse(inverse), m_bMakeSigned(makesigned)
  {
  }
  //
  virtual ~YCbCr(void)
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
