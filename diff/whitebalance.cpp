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
** $Id: whitebalance.cpp,v 1.3 2020/02/07 07:22:07 thor Exp $
**
** This class scales the components of images with component dependent
** scale factors.
*/

/// Includes
#include "diff/whitebalance.hpp"
#include "std/string.hpp"
#include "std/stdlib.hpp"
#include "std/math.hpp"
///

/// WhiteBalance::Convert
template<typename T>
void WhiteBalance::Convert(T *dst ,ULONG bytesperpixel,ULONG bytesperrow,
			   ULONG w, ULONG h,double scale ,double min,double max)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v    = *dstrow * scale;
      if (v < min) {
	*dstrow   = min;
      } else if (v > max) {
	*dstrow   = max;
      } else {
	*dstrow   = v;
      }
      dstrow      = (T *)((UBYTE *)(dstrow) + bytesperpixel);
    }
    dst = (T *)((UBYTE *)(dst) + bytesperrow);
  }
}
///

/// WhiteBalance::ShiftConv
template<typename T>
void WhiteBalance::ShiftConv(T *dst ,ULONG bytesperpixel,ULONG bytesperrow,
			     ULONG w, ULONG h,double offset,double min,double max)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v    = *dstrow + offset;
      if (v < min) {
	*dstrow   = min;
      } else if (v > max) {
	*dstrow   = max;
      } else {
	*dstrow   = v;
      }
      dstrow      = (T *)((UBYTE *)(dstrow) + bytesperpixel);
    }
    dst = (T *)((UBYTE *)(dst) + bytesperrow);
  }
}
///

/// WhiteBalance::WhiteBalance
WhiteBalance::WhiteBalance(Operation type,const char *factors)
  : m_Type(type)
{
  const char *in = factors;
  char *end;
  int i,cnt = 0;

  do {
    double v = strtod(in,&end);
    switch(m_Type) {
    case Scale:
      if (*end != '\0' && *end != ',')
	throw "the --scale argument must consist of comma-separated floating point numbers";
      if (v <= 0.0)
	throw "the --scale arguments must be positive integers";
      break;
    case Shift:
      if (*end != '\0' && *end != ',')
	throw "the --offset argument must consist of comma-separated floating point numbers";
      break;
    }
    in = end + 1;
    cnt++;
  } while(*end);

  if (cnt > 0xffff)
    throw "too many parameters";

  in = factors;
  i  = 0;
  m_pdFactors = new DOUBLE[cnt];
  do {
    m_pdFactors[i] = strtod(in,&end);
    in = end + 1;
    i++;
  } while(*end);
  
  m_usComponents = cnt;
}
///

/// WhiteBalance::~WhiteBalance
WhiteBalance::~WhiteBalance(void)
{
  delete[] m_pdFactors;
}
///

/// WhiteBalance::ApplyScaling
// Apply a scaling from the source to the image stored here.
void WhiteBalance::ApplyScaling(class ImageLayout *src)
{
  UWORD comp;
  
  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    UBYTE bps   = src->BitsOf(comp);
    double min,max;
    double scale = 1.0;
    //
    if (comp < m_usComponents) {
      scale = m_pdFactors[comp]; // Or offsets...
    } else switch(m_Type) {
      case Scale:
	scale = 1.0;
	break;
      case Shift:
	scale = 0.0;
	break;
      }
    //
    if (src->isFloat(comp)) {
      min = -HUGE_VAL;
      max =  HUGE_VAL; // never clip
    } else {
      if (src->isSigned(comp)) {
	min = -(QUAD(1)  << (bps - 1));
	max =  (QUAD(1)  << (bps - 1)) - 1;
      } else {
	min = 0;
	max =  (UQUAD(1) <<  bps     ) - 1;
      }
    }
    //
    switch(m_Type) {
    case Scale:
      if (src->BitsOf(comp) <= 8) {
	if (src->isSigned(comp)) {
	  Convert<BYTE>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			w,h,scale,min,max);
	} else {
	  Convert<UBYTE>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			 w,h,scale,min,max);
	}
      } else if (src->BitsOf(comp) <= 16 && !src->isFloat(comp)) {
	if (src->isSigned(comp)) {
	  Convert<WORD>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			w,h,scale,min,max);
	} else {
	  Convert<UWORD>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			 w,h,scale,min,max);
	}
      } else if (src->BitsOf(comp) <= 32 && !src->isFloat(comp)) {
	if (src->isSigned(comp)) {
	  Convert<LONG>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			w,h,scale,min,max);
	} else {
	  Convert<ULONG>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			 w,h,scale,min,max);
	}
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	Convert<FLOAT>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       w,h,scale,min,max);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	Convert<DOUBLE>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			w,h,scale,min,max);
      } else {
	throw "unsupported data format";
      }
      break;
    case Shift:
      if (src->BitsOf(comp) <= 8) {
	if (src->isSigned(comp)) {
	  ShiftConv<BYTE>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      w,h,scale,min,max);
	} else {
	  ShiftConv<UBYTE>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       w,h,scale,min,max);
	}
      } else if (src->BitsOf(comp) <= 16 && !src->isFloat(comp)) {
	if (src->isSigned(comp)) {
	  ShiftConv<WORD>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      w,h,scale,min,max);
	} else {
	  ShiftConv<UWORD>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       w,h,scale,min,max);
	}
      } else if (src->BitsOf(comp) <= 32 && !src->isFloat(comp)) {
	if (src->isSigned(comp)) {
	  ShiftConv<LONG>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      w,h,scale,min,max);
	} else {
	  ShiftConv<ULONG>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       w,h,scale,min,max);
	}
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	ShiftConv<FLOAT>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		     w,h,scale,min,max);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	ShiftConv<DOUBLE>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      w,h,scale,min,max);
      } else {
	throw "unsupported data format";
      }
      break;
    }
  }
}
///

/// WhiteBalance::Measure
double WhiteBalance::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  ApplyScaling(src);
  ApplyScaling(dst);
  return in;
}
///

