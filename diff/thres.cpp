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
** $Id: thres.cpp,v 1.11 2020/10/16 10:14:12 thor Exp $
**
** This class finds the minimum and the maximum difference.
*/

/// Includes
#include "diff/thres.hpp"
#include "img/imglayout.hpp"
#include "std/math.hpp"
///

/// Thres::PeakError
template<typename T>
double Thres::Error(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		    T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		    ULONG w,ULONG h)
{
  double error;
  ULONG x,y;

  switch(m_Type) {
  case Toe:
    error = HUGE_VAL;
    break;
  case Head:
    error = -HUGE_VAL;
    break;
  default:
    error = 0;
    break;
  }
  
  for(y = 0;y < h;y++) {
    T *orgrow = org;
    T *dstrow = dst;
    for(x = 0;x < w;x++) {
      double diff = *orgrow - *dstrow;
      switch(m_Type) {
      case Min:
	if (diff < error)
	  error = diff;
	break;
      case Max:
	if (diff > error)
	  error = diff;
	break;
      case Drift:
	error += diff;
	break;
      case Avg:
	error += fabs(diff);
	break;
      case Peak:
	diff = fabs(diff);
	if (diff > error)
	  error = diff;
	break;
      case Toe:
	if (*orgrow < error)
	  error = *orgrow;
	break;
      case Head:
	if (*orgrow > error)
	  error = *orgrow;
	break;
      }
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
  }

  if (m_Type == Avg || m_Type == Drift)
    error /= w * h;
  
  return error;
}
///

/// Thres::Measure
double Thres::Measure(class ImageLayout *src,class ImageLayout *dst,double)
{
  double error;
  UWORD comp;
  
  switch(m_Type) {
  case Toe:
    error = HUGE_VAL;
    break;
  case Head:
    error = -HUGE_VAL;
    break;
  default:
    error = 0;
    break;
  }
  
  for(comp = 0;comp < src->DepthOf();comp++) {
    double peak = 0.0;
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	peak = Error<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	peak = Error<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	peak = Error<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	peak = Error<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	peak = Error<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	peak = Error<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	peak = Error<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	peak = Error<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	peak = Error<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	peak = Error<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
      } else {
	throw "unsupported data type";
      }
    }
    //
    switch(m_Type) {
    case Min:
    case Toe:
      if (peak < error)
	error = peak;
      break;
    case Max:
    case Peak:
    case Head:
      if (peak > error)
	error = peak;
      break;
    case Avg:
    case Drift:
      error += peak;
      break;
    }
  }
    
  if (m_Type == Avg || m_Type == Drift)
    error /= src->DepthOf();

  return error;
}
///
