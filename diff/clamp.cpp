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
** $Id: clamp.cpp,v 1.1 2017/06/12 14:57:16 thor Exp $
**
** This class clamps the input to a given min and max range.
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/clamp.hpp"
///

/// Clamp::ClampRange
template<typename S>
void Clamp::ClampRange(S *org ,ULONG obytesperpixel,ULONG obytesperrow,
		       S min, S max, ULONG w, ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    S *orgrow = org;
    for(x = 0;x < w;x++) {
      S org = *orgrow;
      if (org < min)
	org = min;
      if (org > max)
	org = max;
      *orgrow = org;
      orgrow  = (S *)((UBYTE *)(orgrow) + obytesperpixel);
    }
    org = (S *)((UBYTE *)(org) + obytesperrow);
  }
}
///

/// Clamp::ApplyClamping
// Apply the recorded clamping to the given image
void Clamp::ApplyClamping(class ImageLayout *src)
{
  UWORD comp,depth = src->DepthOf();

  for(comp = 0;comp < depth;comp++) {
    ULONG obytesperrow   = src->BytesPerRow(comp);
    ULONG obytesperpixel = src->BytesPerPixel(comp);
    DOUBLE min           = m_dMin;
    DOUBLE max           = m_dMax;
    APTR org             = src->DataOf(comp);

    if (!src->isFloat(comp)) {
      if (src->isSigned(comp)) {
	DOUBLE rmin = - (1LL << (src->BitsOf(comp) - 1));
	DOUBLE rmax =   (1LL << (src->BitsOf(comp) - 1)) - 1;
	if (min < rmin)
	  min = rmin;
	if (max > rmax)
	  max = rmax;
      } else {
	DOUBLE rmax =   (1LL << src->BitsOf(comp)) - 1;
	if (min < 0.0)
	  min = 0.0;
	if (max > rmax)
	  max = rmax;
      }
    }
    
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	ClampRange<BYTE>((BYTE *)org,obytesperpixel,obytesperrow,
			 BYTE(min),UBYTE(max),src->WidthOf(comp),src->HeightOf(comp));
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	ClampRange<WORD>((WORD *)org,obytesperpixel,obytesperrow,
			 WORD(min),WORD(max),src->WidthOf(comp),src->HeightOf(comp));
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	ClampRange<LONG>((LONG *)org,obytesperpixel,obytesperrow,
			 LONG(min),LONG(max),src->WidthOf(comp),src->HeightOf(comp));
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	ClampRange<FLOAT>((FLOAT *)org,obytesperpixel,obytesperrow,
			  min,max,src->WidthOf(comp),src->HeightOf(comp));
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	ClampRange<DOUBLE>((DOUBLE *)org,obytesperpixel,obytesperrow,
			   min,max,src->WidthOf(comp),src->HeightOf(comp));
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	ClampRange<UBYTE>((UBYTE *)org,obytesperpixel,obytesperrow,
			  UBYTE(min),UBYTE(max),src->WidthOf(comp),src->HeightOf(comp));
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	ClampRange<UWORD>((UWORD *)org,obytesperpixel,obytesperrow,
			  UWORD(min),UWORD(max),src->WidthOf(comp),src->HeightOf(comp));
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	ClampRange<ULONG>((ULONG *)org,obytesperpixel,obytesperrow,
			  ULONG(min),ULONG(max),src->WidthOf(comp),src->HeightOf(comp));
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	ClampRange<FLOAT>((FLOAT *)org,obytesperpixel,obytesperrow,
			  min,max,src->WidthOf(comp),src->HeightOf(comp));
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	ClampRange<DOUBLE>((DOUBLE *)org,obytesperpixel,obytesperrow,
			   min,max,src->WidthOf(comp),src->HeightOf(comp));
      } else {
	throw "unsupported data type";
      }
    }
  }
}
///

/// Clamp::Measure
// This implements the actual interface of the meter
double Clamp::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  ApplyClamping(src);
  ApplyClamping(dst);

  return in;
}
///
