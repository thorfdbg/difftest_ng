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
** $Id: psnr.cpp,v 1.20 2016/09/19 11:52:18 thor Exp $
**
** This class measures the PSNR between two images, averaged over all samples
** and thus all components.
*/

/// Includes
#include "diff/psnr.hpp"
#include "img/imglayout.hpp"
#include "std/math.hpp"
///

/// PSNR::MSE
template<typename T>
double PSNR::MSE(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		 T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		 ULONG w,ULONG h,double &max,double &energy)
{
  double error = 0.0;
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *orgrow = org;
    T *dstrow = dst;
    for(x = 0;x < w;x++) {
      double diff = *orgrow - *dstrow;
      double orq  = *orgrow * *orgrow;
      //double dsq  = *dstrow * *dstrow;
      error      += diff * diff;
      //energy     += (orq + dsq) * 0.5;
      energy     += orq;
      if (orq > max) max = orq;
      // if (dsq > max) max = dsq;
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

/// PSNR::Measure
double PSNR::Measure(class ImageLayout *src,class ImageLayout *dst,double)
{
  double error  = 0.0;
  double max    = 0.0;
  double energy = 0.0;
  UWORD comp,d  = src->DepthOf();
  int type      = m_Type;

  if (d != 3 && type != Min && type != Mean) {
    fprintf(stderr,"the selected PSNR measurement is only available for three component images, reverting to minpsnr\n");
    type = Min;
  }

  for(comp = 0;comp < d;comp++) {
    double mse = 0.0;
    double erg = 0.0;
    ULONG  w   = src->WidthOf(comp);
    ULONG  h   = src->HeightOf(comp);
    double prc = (src->isFloat(comp))?(1.0):(double(UQUAD(1) << src->BitsOf(comp)) - 1.0);
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	mse = MSE<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      w,h,max,erg);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	mse = MSE<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      w,h,max,erg);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	mse = MSE<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      w,h,max,erg);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	mse = MSE<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,max,erg);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	mse = MSE<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h,max,erg);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	mse = MSE<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,max,erg);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	mse = MSE<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,max,erg);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	mse = MSE<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,max,erg);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	mse = MSE<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,max,erg);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	mse = MSE<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h,max,erg);
      } else {
	throw "unsupported data type";
      }
    }
    //
    if (m_bSNR || m_bLinear) {
      mse /= (w * h);
      erg /= (w * h);
    } else {
      mse /= (w * h) * prc * prc;
      erg /= (w * h) * prc * prc;
    }
    //
    switch(type) {
    case Mean:
      error  += mse / src->DepthOf();
      energy += erg / src->DepthOf();
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
	energy += erg * numerator / denominator;
      }
      break;
    case Min:
      if (mse > error)
	error = mse;
      if (erg > energy)
	energy = erg;
      break;
    case YCbCr:
      switch(comp) {
      case 0:
	error  += mse * 0.299;
	energy += erg * 0.299;
	break;
      case 1:
	error  += mse * 0.587;
	energy += erg * 0.587;
	break;
      case 2:
	error  += mse * 0.114;
	energy += erg * 0.144;
	break;
      }
      break;
    case YUV:
      switch(comp) {
      case 0:
	error  += mse * 0.222;
	energy += erg * 0.222;
	break;
      case 1:
	error  += mse * 0.707;
	energy += erg * 0.707;
	break;
      case 2:
	error  += mse * 0.071;
	energy += erg * 0.071;
	break;
      }
      break;
    }
  }

  if (m_bSNR) {
    if (m_bScaleToEnergy) {
      if (energy > 0.0) {
	error /= energy;
      }
    } else {
      if (max > 0.0) {
	error /= max;
      }
    }
  }
  
  if (m_bLinear) {
    return error;
  } else {
    return -10.0 * log(error) / log(10.0);
  }
}
///
