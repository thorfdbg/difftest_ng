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
** This image class doesn't do much useful, it only represents a blank
** image of a given dimension.
**
*/

#ifndef BLANKIMG_HPP
#define BLANKIMG_HPP


/// Includes
#include "imglayout.hpp"
#include "std/stdio.hpp"
///

/// BlankImg
// This image class doesn't do much useful, it only represents a blank
// image of a given dimension.
class BlankImg  : public ImageLayout {
  // This is the image raw data
  // the data is in order {r,g,b} for colored pictures. It contains only zeros....
  UBYTE *m_pucImage;
  //
public:
  //
  // Copy the image from another source for later saving.
  BlankImg(const class ImageLayout &layout);
  // destructor
  ~BlankImg(void);
  //
  // Allocate all the memory and assign it to the components.
  void Blank(void);
};
///

///
#endif
