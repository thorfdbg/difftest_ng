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
** $Id: peakpos.cpp,v 1.3 2014/01/22 20:44:22 thor Exp $
**
** This class finds the pixel position of the peak error.
*/

/// Includes
#include "diff/peakpos.hpp"
#include "img/imglayout.hpp"
#include "std/math.hpp"
///

/// PeakPos::PeakError
template<typename T>
PeakPos::Pixel PeakPos::PeakError(T *org,ULONG obytesperpixel,ULONG obytesperrow,
				  T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
				  ULONG w,ULONG h)
{
  double error = 0.0;
  ULONG x,y;
  ULONG px = 0;
  ULONG py = 0;

  for(y = 0;y < h;y++) {
    T *orgrow = org;
    T *dstrow = dst;
    for(x = 0;x < w;x++) {
      double diff = *orgrow - *dstrow;
      diff        = fabs(diff);
      if (diff > error) {
	error     = diff;
	px        = x;
	py        = y;
      }
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
  }

  {
    struct Pixel pix;
    pix.error = error;
    pix.x     = px;
    pix.y     = py;
    
    return pix;
  }
}
///

/// PeakPos::Measure
double PeakPos::Measure(class ImageLayout *src,class ImageLayout *dst,double)
{
  struct Pixel px,pxmax = {0,0,0.0};
  double error = 0.0;
  UWORD comp;

  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    double prc  = src->isFloat(comp)?(1.0):(double(UQUAD(1) << src->BitsOf(comp)) - 1.0);
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	px = PeakError<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	px = PeakError<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     w,h);
       } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	px = PeakError<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	px = PeakError<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				      (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				      w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	px = PeakError<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				       (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				       w,h);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	px = PeakError<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				      (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				      w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	px = PeakError<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				      (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				      w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	px = PeakError<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				      (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				      w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	px = PeakError<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				      (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				      w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	px = PeakError<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				       (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				       w,h);
      } else {
	throw "unsupported data type";
      }
    }
    //
    px.error /= prc;
    //
    if (px.error > error) {
      pxmax.error = error;
      pxmax.x     = px.x;
      pxmax.y     = px.y;
    }
  }
  
  if (px.error > 0.0) {
    switch(m_Type) {
    case PeakX:
      return pxmax.x;
    case PeakY:
      return pxmax.y;
    }
  }
  throw "images are identical";
}
///
