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
** $Id: invert.hpp,v 1.2 2014/01/04 11:35:28 thor Exp $
**
** This class scales inverts the color of all pixels (negative)
*/

#ifndef DIFF_INVERT_HPP
#define DIFF_INVERT_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Invert
// This class inverts the color of all pixels
class Invert : public Meter {
  //
  // Convert a single image to Invert.
  void ConvertImg(class ImageLayout *img);
  //
  template<typename S>
  void Convert(S *org ,ULONG obytesperpixel,ULONG obytesperrow,
	       ULONG w, ULONG h,LONG inversionpoint);
  //
public:
  //
  // Convert an image to its inverse.
  Invert(void)
  { }
  //
  virtual ~Invert(void)
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
