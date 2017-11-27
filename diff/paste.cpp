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
** Paste
** 
** $Id: paste.cpp,v 1.1 2017/11/27 13:21:16 thor Exp $
**
** This class copies a second image into the first image.
**
*/

/// Include
#include "std/stdlib.hpp"
#include "diff/meter.hpp"
#include "diff/paste.hpp"
#include "img/imglayout.hpp"
///

/// Paste::PasteImage
// Templated filler class.
template<typename T>
void Paste::PasteImage(T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		       const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
		       ULONG tx,ULONG ty,ULONG width,ULONG height)
{
  ULONG x,y;

  dst = (T*)((UBYTE *)dst + tx * dbytesperpixel + ty * dbytesperrow);
  
  for(y = 0;y < height;y++) {
    T *dstline       = dst;
    const T *srcline = src;
    for(x = 0;x < width;x++) {
      *dstline = *srcline;
      srcline  = (const T *)((const UBYTE *)srcline + sbytesperpixel);
      dstline  =       (T *)((      UBYTE *)dstline + dbytesperpixel);
    }
    src = (const T *)((const UBYTE *)src + sbytesperrow);
    dst =       (T *)((      UBYTE *)dst + dbytesperrow);
  }
}
///

/// Paste::Measure
// The actual front-end implementation.
double Paste::Measure(class ImageLayout *target,class ImageLayout *src,double in)
{
  UWORD c;
  UWORD d = src->DepthOf();
  //
  // Iterate over components
  for(c = 0;c < d;c++) {
    // Check the subsampling factors. They should be identical.
    if (target->SubXOf(c) != src->SubXOf(c) ||
	target->SubYOf(c) != src->SubXOf(c))
      throw "subsampling factors of source and destination are not identical, cannot paste";
    if (target->BitsOf(c) != src->BitsOf(c))
      throw "bit precisions of source and destination are not identical, cannot paste";
    if (target->isSigned(c) != src->isSigned(c))
      throw "signedness of source and destination are not identical, cannot paste";
    if (target->isFloat(c) != src->isFloat(c))
      throw "source and destination must have the same data type, cannot paste float into integer and vice versa";
    //
    // Compute the effective start position and dimension.
    if (m_ulDestX % target->SubXOf(c) != 0 || m_ulDestY % target->SubYOf(c))
      throw "target position must be divisible by the subsampling factors";
    //
    ULONG tx = m_ulDestX / target->SubXOf(c);
    ULONG ty = m_ulDestY / target->SubYOf(c);
    //
    // If this pastes outside of the target are, continue and go on with the
    // next component.
    if (tx >= target->WidthOf(c) || ty >= target->HeightOf(c))
      continue;
    //
    // Compute the effective dimension.
    ULONG width  = src->WidthOf(c);
    ULONG height = src->HeightOf(c);
    //
    // Potentially clip at the right or bottom edge.
    if (tx + width  > target->WidthOf(c))
      width  = target->WidthOf(c) - tx;
    if (ty + height > target->HeightOf(c))
      height = target->HeightOf(c) - ty;
    if (src->isFloat(c)) {
      if (src->BitsOf(c) <= 32) {
	PasteImage((FLOAT *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		   (FLOAT *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		   tx,ty,width,height);
      } else {
	PasteImage((DOUBLE *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		   (DOUBLE *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		   tx,ty,width,height);
      }
    } else {
      if (src->isSigned(c)) {
	if (src->BitsOf(c) <= 8) {
	  PasteImage((BYTE *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		     (BYTE *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     tx,ty,width,height);
	} else if (src->BitsOf(c) <= 16) {
	  PasteImage((WORD *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		     (WORD *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     tx,ty,width,height);
	} else if (src->BitsOf(c) <= 32) {
	  PasteImage((LONG *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		     (LONG *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     tx,ty,width,height);		  
	}
      } else {
	if (src->BitsOf(c) <= 8) {
	  PasteImage((UBYTE *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		     (UBYTE *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     tx,ty,width,height);
	} else if (src->BitsOf(c) <= 16) {
	  PasteImage((UWORD *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		     (UWORD *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     tx,ty,width,height);
	} else if (src->BitsOf(c) <= 32) {
	  PasteImage((ULONG *)target->DataOf(c),target->BytesPerPixel(c),target->BytesPerRow(c),
		     (ULONG *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     tx,ty,width,height);
	}
      }
    }
  }

  return in;
}
///
