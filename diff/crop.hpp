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
** $Id: crop.hpp,v 1.6 2020/09/15 09:45:49 thor Exp $
**
** This class crops images, i.e. extracts rectangular regions from images
*/

#ifndef DIFF_CROP_HPP
#define DIFF_CROP_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// class Crop
// This class extracts rectangulare image regions
class Crop : public Meter {
  //
public:
  // Cropping modes.
  enum CropMode {
    CropBoth = 0,
    CropSrc  = 1,
    CropDst  = 2
  };
  //
private:
  //
  // The cropping rectangle. Must fit into the image rectangle.
  // This is given as the coordinates of the upper left and lower right
  // edge, both inclusive.
  ULONG       m_ulX1;
  ULONG       m_ulY1;
  ULONG       m_ulX2;
  ULONG       m_ulY2;
  //
  // The cropping mode
  CropMode    m_Mode;
  //
public:
  //
  // Crop an image region. Given are the edge coordinates, inclusive.
  Crop(ULONG x1,ULONG y1,ULONG x2,ULONG y2,CropMode mode)
    : m_ulX1(x1), m_ulY1(y1), m_ulX2(x2), m_ulY2(y2),
      m_Mode(mode)
  { }
  //
  virtual ~Crop(void)
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
