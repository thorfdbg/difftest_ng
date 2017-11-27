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
** Fill
** 
** $Id: fill.cpp,v 1.1 2017/11/27 11:37:13 thor Exp $
**
** This class fills an entire image with a given color
**
*/

/// Include
#include "std/stdlib.hpp"
#include "diff/meter.hpp"
#include "diff/fill.hpp"
#include "img/imglayout.hpp"
///

/// Fill::FillCanvas
// Templated filler class.
template<typename T>
void Fill::FillCanvas(T *org,ULONG bytesperpixel,ULONG bytesperrow,
		      ULONG width,ULONG height,T value)
{
  ULONG x,y;

  for(y = 0;y < height;y++) {
    T *line = org;
    for(x = 0;x < width;x++) {
      *line = value;
      line  = (T *)((UBYTE *)line + bytesperpixel);
    }
    org = (T *)((UBYTE *)org + bytesperrow);
  }
}
///

/// Fill::Measure
// The actual front-end implementation.
// Fills only the source image.
double Fill::Measure(class ImageLayout *src,class ImageLayout *,double in)
{
  UWORD c;
  UWORD d = src->DepthOf();
  const char *values = m_pcValues;
  char *end;
  //
  // Iterate over components
  for(c = 0;c < d;c++) {
    if (src->isFloat(c)) {
      double v = strtod(values,&end);
      if (*end != 0 && *end != ',')
	throw "invalid fill value specified for --fill";
      values = end + 1;
      if (src->BitsOf(c) <= 32) {
	FillCanvas((FLOAT *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		   src->WidthOf(c),src->HeightOf(c),(FLOAT)v);
      } else {
	FillCanvas((DOUBLE *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		   src->WidthOf(c),src->HeightOf(c),(DOUBLE)v);
      }
    } else {
      long min,max;
      long v = strtol(values,&end,0);
      if (*end != 0 && *end != ',')
	throw "invalid fill value specified for --fill";
      values = end + 1;
      if (src->isSigned(c)) {
	min = -(1L << (src->BitsOf(c) - 1));
	max = -(min + 1);
      } else {
	min = 0;
	max = (1UL << src->BitsOf(c)) - 1;
      }
      if (v < min || v > max)
	throw "fill value for --fill is out of range";
      if (src->isSigned(c)) {
	if (src->BitsOf(c) <= 8) {
	  FillCanvas((BYTE *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     src->WidthOf(c),src->HeightOf(c),(BYTE)v);
	} else if (src->BitsOf(c) <= 16) {
	  FillCanvas((WORD *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     src->WidthOf(c),src->HeightOf(c),(WORD)v);
	} else if (src->BitsOf(c) <= 32) {
	  FillCanvas((LONG *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     src->WidthOf(c),src->HeightOf(c),(LONG)v);
	}
      } else {
	if (src->BitsOf(c) <= 8) {
	  FillCanvas((UBYTE *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     src->WidthOf(c),src->HeightOf(c),(UBYTE)v);
	} else if (src->BitsOf(c) <= 16) {
	  FillCanvas((UWORD *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     src->WidthOf(c),src->HeightOf(c),(UWORD)v);
	} else if (src->BitsOf(c) <= 32) {
	  FillCanvas((ULONG *)src->DataOf(c),src->BytesPerPixel(c),src->BytesPerRow(c),
		     src->WidthOf(c),src->HeightOf(c),(ULONG)v);
	}
      }
    }
  }

  return in;
}
///
