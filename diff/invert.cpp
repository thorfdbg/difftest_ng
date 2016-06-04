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
** $Id: invert.cpp,v 1.5 2016/06/04 10:44:08 thor Exp $
**
** This class converts between RGB and Invert signals
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/invert.hpp"
#include "std/math.hpp"
///

/// Invert::Convert
template<typename S>
void Invert::Convert(S *p,ULONG obytesperpixel,ULONG obytesperrow,
		     ULONG w, ULONG h,LONG ip)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    S *row = p;
    for(x = 0;x < w;x++) {
      *row = ip - *row;
      row  = (S *)((UBYTE *)(row) + obytesperpixel);
    }
    p  = (S *)((UBYTE *)(p) + obytesperrow);
  }
}
/// 

/// Invert::ConvertImg
// Convert a single image to Invert.
void Invert::ConvertImg(class ImageLayout *img)
{
  int i;
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  int depth     = img->DepthOf();
  LONG ip       = 0;
  
  //
  // Find the conversion offsets.
  for(i = 0;i < depth;i++) {
    if (img->isSigned(i)) {
      ip = 0;
    } else {
      if (img->isFloat(i)) {
	ip = 1;
      } else {
	ip = (1UL << (img->BitsOf(i))) - 1;
      }
    }
    //
    if (img->isFloat(i)) {
      if (img->BitsOf(i) <= 32) {
	Convert<FLOAT>((FLOAT *)img->DataOf(i),
		       img->BytesPerPixel(i),
		       img->BytesPerRow(i)  ,w,h,ip);
      } else if (img->BitsOf(i) == 64) {
	Convert<DOUBLE>((DOUBLE *)img->DataOf(i),
			img->BytesPerPixel(i),
			img->BytesPerRow(i)  ,w,h,ip);
      } else throw "unsupported source format";
    } else {
      if (img->BitsOf(i) <= 8) {
	if (img->isSigned(i)) {
	  Convert<BYTE>((BYTE *)img->DataOf(i),
			img->BytesPerPixel(i),
			img->BytesPerRow(i)  ,w,h,ip);
	} else {
	  Convert<UBYTE>((UBYTE *)img->DataOf(i),
			 img->BytesPerPixel(i),
		       img->BytesPerRow(i)  ,w,h,ip);
	}
      } else if (img->BitsOf(i) <= 16) {
	if (img->isSigned(i)) {
	  Convert<WORD>((WORD *)img->DataOf(i),
			img->BytesPerPixel(i),
			img->BytesPerRow(i)  ,w,h,ip);
	} else {
	  Convert<UWORD>((UWORD *)img->DataOf(i),
			 img->BytesPerPixel(i),
			 img->BytesPerRow(i)  ,w,h,ip);
	}
      } else if (img->BitsOf(i) <= 32) {
	if (img->isSigned(i)) {
	  Convert<LONG>((LONG *)img->DataOf(i),
			img->BytesPerPixel(i),
			img->BytesPerRow(i)  ,w,h,ip);
	} else {
	  Convert<ULONG>((ULONG *)img->DataOf(i),
			 img->BytesPerPixel(i),
			 img->BytesPerRow(i)  ,w,h,ip);
	}
      } else throw "unsupported source format";
    }
  }
}
///

/// Invert::Measure
double Invert::Measure(class ImageLayout *src,class ImageLayout *,double in)
{
  ConvertImg(src);
  //ConvertImg(dst);

  return in;
}
///

