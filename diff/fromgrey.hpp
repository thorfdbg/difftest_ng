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
** $Id: fromgrey.hpp,v 1.1 2020/10/16 09:38:15 thor Exp $
**
** This class converts a grey-scale image to an RGB image.
*/

#ifndef DIFF_FROMGREY_HPP
#define DIFF_FROMGREY_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class FromGrey
// This class converts a grey-scale image to an RGB image.
class FromGrey : public Meter, private ImageLayout {
  //
  void GreyToRGB(class ImageLayout *src);
  //
public:
  //
  // Construct the difference image. Takes a file name.
  FromGrey(void)
  {
  }
  //
  virtual ~FromGrey(void)
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
