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
** $Id: suppress.cpp,v 1.1 2019/07/24 10:45:06 thor Exp $
**
** This class suppresses (resets to the source) those pixels in
** the destination that are less than a threshold away from the
** source.
*/

/// Includes
#include "diff/suppress.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// DiffImg::Threshold
// The implementation of the algorithm.
template<typename T>
void Suppress::Threshold(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
			 T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
			 T thres,ULONG w,ULONG h)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    const T *orgrow = org;
    T       *dstrow = dst;
    for(x = 0;x < w;x++) {
      if (*dstrow <= *orgrow + thres && *dstrow >= *orgrow - thres)
	*dstrow = *orgrow;
      //
      orgrow      = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)      ((      UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)      ((      UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Suppress::Measure
double Suppress::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  UWORD comp;
  
  src->TestIfCompatible(dst);

  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w     = src->WidthOf(comp);
    ULONG  h     = src->HeightOf(comp);
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	Threshold<BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			(      BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			BYTE(m_dThres),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	Threshold<WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			(      WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			WORD(m_dThres),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	Threshold<LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			(      LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			LONG(m_dThres),w,h);
      } else if (src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	Threshold<FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			 (      FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			 FLOAT(m_dThres),w,h);
      } else if (src->isFloat(comp) && src->BitsOf(comp) <= 64) {
	Threshold<DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			  (      DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			  DOUBLE(m_dThres),w,h);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	Threshold<UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			(      UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			 UBYTE(m_dThres),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	Threshold<UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			 (      UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			WORD(m_dThres),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	Threshold<ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			 (      ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			LONG(m_dThres),w,h);
      } else if (src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	Threshold<FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			 (      FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			 FLOAT(m_dThres),w,h);
      } else if (src->isFloat(comp) && src->BitsOf(comp) <= 64) {
	Threshold<DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			  (      DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			  DOUBLE(m_dThres),w,h);
      } else {
	throw "unsupported data type";
      }
    }
  }

  return in;
}
///
