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
** $Id: mrse.cpp,v 1.9 2017/01/31 11:58:04 thor Exp $
**
** This class measures the mean relative square error between two images, averaged over all samples
** and thus all components.
*/

/// Includes
#include "diff/mrse.hpp"
#include "img/imglayout.hpp"
#include "std/math.hpp"
///

/// MRSE::Compute
template<typename T>
double MRSE::Compute(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		     T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		     ULONG w,ULONG h)
{
  double error = 0.0;
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *orgrow = org;
    T *dstrow = dst;
    for(x = 0;x < w;x++) {
      double vorg = *orgrow;
      double vdst = *dstrow;
      double diff = vorg - vdst;
      double norm = vorg * vorg + vdst * vdst;
      if (norm > 0.0) {
	error    += (diff * diff) / norm;
      }
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
  }

  return error;
}
///

/// MRSE::Measure
double MRSE::Measure(class ImageLayout *src,class ImageLayout *dst,double)
{
  double error = 0.0;
  UWORD comp,d = src->DepthOf();
  int type = m_Type;

  if (d != 3 && type != Min && type != Mean) {
    fprintf(stderr,"the selected MRSE measurement is only available for three component images, reverting to minmrse\n");
    type = Min;
  }

  for(comp = 0;comp < d;comp++) {
    double mse = 0.0;
    ULONG  w   = src->WidthOf(comp);
    ULONG  h   = src->HeightOf(comp);
    double prc = (src->isFloat(comp))?(1.0):(double(UQUAD(1) << src->BitsOf(comp)) - 1.0);
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	mse = Compute<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	mse = Compute<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	mse = Compute<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				  w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	mse = Compute<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	mse = Compute<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	mse = Compute<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	mse = Compute<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	mse = Compute<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	mse = Compute<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	mse = Compute<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
      } else {
	throw "unsupported data type";
      }
    }
    //
    mse /= (w * h) * prc * prc;
    //
    switch(m_Type) {
    case Mean:
      error += mse / src->DepthOf();
      break;
    case SamplingWeighted:
      {
	int c;
	double numerator   = 1.0 / (src->SubXOf(comp) * src->SubYOf(comp));
	double denominator = 0.0;
	for(c = 0;c < d;c++) {
	  denominator += 1.0 / (src->SubXOf(c) * src->SubYOf(c));
	}
	error  += mse * numerator / denominator;
      }
      break;
    case Min:
      if (mse > error)
	error = mse;
      break;
    case YCbCr:
      switch(comp) {
      case 0:
	error += mse * 0.299;
	break;
      case 1:
	error += mse * 0.587;
	break;
      case 2:
	error += mse * 0.114;
	break;
      }
      break;
    case YUV:
      switch(comp) {
      case 0:
	error += mse * 0.222;
	break;
      case 1:
	error += mse * 0.707;
	break;
      case 2:
	error += mse * 0.071;
	break;
      }
      break;
    }
  }
    
  return -10.0 * log(error) / log(10.0);
}
///
