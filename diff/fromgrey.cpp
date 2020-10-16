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
** $Id: fromgrey.cpp,v 1.1 2020/10/16 09:38:15 thor Exp $
**
** This class converts a grey-scale image to an RGB image.
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/fromgrey.hpp"
///

/// FromGrey::GreyToRGB
void FromGrey::GreyToRGB(class ImageLayout *src)
{
  UWORD i;
  
  CreateComponents(src->WidthOf(),src->HeightOf(),3);

  //
  // Now copy the attributes over.
  for(i = 0;i < 3;i++) {
    m_pComponent[i].m_ulWidth         = src->WidthOf(0);
    m_pComponent[i].m_ulHeight        = src->HeightOf(0);
    m_pComponent[i].m_ucBits          = src->BitsOf(0);
    m_pComponent[i].m_bSigned         = src->isSigned(0);
    m_pComponent[i].m_bFloat          = src->isFloat(0);
    m_pComponent[i].m_ucSubX          = src->SubXOf(0);
    m_pComponent[i].m_ucSubY          = src->SubYOf(0);
    m_pComponent[i].m_ulBytesPerPixel = src->BytesPerPixel(0);
    m_pComponent[i].m_ulBytesPerRow   = src->BytesPerRow(0);
    m_pComponent[i].m_pPtr            = src->DataOf(0);
  }
}
///

/// FromGrey::Measure
double FromGrey::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  if (src->DepthOf() != 1)
    throw "the source image must have depth 1";

  if (dst->DepthOf() != 1)
    throw "the source image must have depth 1";

  GreyToRGB(src);
  Swap(*src);

  GreyToRGB(dst);
  Swap(*dst);

  return in;
}
///
