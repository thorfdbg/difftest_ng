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
** $Id: mask.hpp,v 1.2 2014/01/04 11:35:28 thor Exp $
**
** This tool replaces the pixels in the distored image by the pixels from the original
** providing a masking operation. The masking is an alpha-blending in case the mask
** is not binary.
*/

#ifndef DIFF_MASK_HPP
#define DIFF_MASK_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Mask
class Mask : public Meter {
  //
  // File name of the mask.
  const char *m_pcMaskName;
  //
  // Invert the mask, i.e. make 1.0 opaque.
  bool        m_bInvert;
  //
  // Templated implementations
  template<typename T,typename S>
  void MixDown(const T *org,ULONG obytesperpixel,ULONG obytesperrow,
	       T       *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
	       const S *msk,ULONG mbytesperpixel,ULONG mbytesperrow,
	       double min,double max,ULONG w,ULONG h);
  //
  // Blend, but still with unknown mask type. This then goes into one of the above
  // functions.
  template<typename T>
  void Blend(const T *org,ULONG obytesperpixel,ULONG obytesperrow,
	     T       *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
	     UWORD comp,class ImageLayout *mask);
  //
public:
  //
  //
  Mask(const char *mask,bool invert)
    : m_pcMaskName(mask), m_bInvert(invert)
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
