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
** This image class doesn't do much useful, it only represents a blank
** image of a given dimension.
**
*/

/// Includes
#include "imglayout.hpp"
#include "std/string.hpp"
#include "img/blankimg.hpp"
///

/// BlankImg::BlankImg
// Create a blank image of the given dimensions.
BlankImg::BlankImg(ULONG width,ULONG height,UWORD depth)
{
  m_ulWidth  = width;
  m_ulHeight = height;
  m_usDepth  = depth;
}
///
  
/// BlankImg::BlankImg
// Copy the image from another source for later saving.
BlankImg::BlankImg(const class ImageLayout &layout)
  : ImageLayout(layout), m_pucImage(NULL)
{
}
///

/// BlankImg::~BlankImg
BlankImg::~BlankImg(void)
{
  delete[] m_pucImage;
}
///

/// BlankImg::Blank
// Allocate all the memory and assign it to the components.
void BlankImg::Blank(void)
{
  UWORD d;
  size_t ntry = 1;
  size_t size = WidthOf() * HeightOf();

  assert(m_pucImage == NULL);

  for(d = 0;d < DepthOf();d++) {
    size_t ms = (BitsOf(d) + 7) >> 3;
    if (isFloat(d) && BitsOf(d) == 16)
      ms = 4; // half float is represented as float.
    if (ms > ntry)
      ntry = ms;
  }

  size *= ntry;

  //
  // It is sufficient to allocate this once and use the same memory
  // for all components.
  m_pucImage = new UBYTE[size];
  memset(m_pucImage,0,size * sizeof(UBYTE));

  for(d = 0;d < DepthOf();d++) {
    m_pComponent[d].m_ulBytesPerPixel = ULONG(ntry);
    m_pComponent[d].m_ulBytesPerRow   = ULONG(ntry * WidthOf());
    m_pComponent[d].m_pPtr            = m_pucImage;
  }
}
///

/// BlankImg::BlankSeparate
// Allocate all the memory and assign it to the components.
// This version allocated separate memory for the components
void BlankImg::BlankSeparate(void)
{
  UWORD d;
  size_t size = 0;
  UBYTE *mem;

  assert(m_pucImage == NULL);

  for(d = 0;d < DepthOf();d++) {
    size_t ms = (BitsOf(d) + 7) >> 3;
    if (isFloat(d) && BitsOf(d) == 16)
      ms = 4; // half float is represented as float.
    size += ms * WidthOf(d) * HeightOf(d);
  }

  mem = m_pucImage = new UBYTE[size];
  memset(m_pucImage,0,size * sizeof(UBYTE));

  for(d = 0;d < DepthOf();d++) {
    size_t ms = (BitsOf(d) + 7) >> 3;
    if (isFloat(d) && BitsOf(d) == 16)
      ms = 4; // half float is represented as float.
    m_pComponent[d].m_ulBytesPerPixel = ms;
    m_pComponent[d].m_ulBytesPerRow   = ms * WidthOf(d);
    m_pComponent[d].m_pPtr            = mem;
    mem += ms * WidthOf(d) * HeightOf(d);
  }
}
///
