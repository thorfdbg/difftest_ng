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
** $Id: stripe.cpp,v 1.2 2014/01/04 11:35:28 thor Exp $
**
** This class measures the PSNR between two images, averaged over all samples
** and thus all components.
*/

/// Includes
#include "diff/stripe.hpp"
#include "img/imglayout.hpp"
#include "std/math.hpp"
///

/// Stripe::MSE
// Traditional non-directional MSE
template<typename T>
double Stripe::MSE(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		   T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		   ULONG w,ULONG h)
{
  double error = 0.0;
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *orgrow = org;
    T *dstrow = dst;
    for(x = 0;x < w;x++) {
      double diff = *orgrow - *dstrow;
      error      += diff * diff;
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

/// Stripe::MSE_Hor
// l^2 in Horizontal direction, l^infinity in vertical direction
template<typename T>
double Stripe::MSE_Hor(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		       T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		       ULONG w,ULONG h)
{
  double error = 0.0;
  ULONG x,y;

  for(y = 0;y < h;y++) {
    double err = 0.0;
    T *orgrow = org;
    T *dstrow = dst;
    for(x = 0;x < w;x++) {
      double diff = *orgrow - *dstrow;
      err        += diff * diff;
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
    }
    if (err > error)
      error = err;
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
  }

  return error * h;
}
///

/// Stripe::MSE_Ver
// l^2 in vertical direction, l^infinity in horizontal direction
template<typename T>
double Stripe::MSE_Ver(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		       T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		       ULONG w,ULONG h)
{
  double error = 0.0;
  ULONG x,y;

  for(x = 0;x < w;x++) {
    double err = 0.0;
    T *orgrow = org;
    T *dstrow = dst;
    for(y = 0;y < h;y++) {
      double diff = *orgrow - *dstrow;
      err        += diff * diff;
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperrow);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperrow);
    }
    if (err > error)
      error = err;
    org = (T *)((const UBYTE *)(org) + obytesperpixel);
    dst = (T *)((const UBYTE *)(dst) + dbytesperpixel);
  }

  return error * w;
}
///

/// Stripe::Measure
double Stripe::Measure(class ImageLayout *src,class ImageLayout *dst,double)
{
  double max   = 0.0;
  UWORD comp;

  for(comp = 0;comp < src->DepthOf();comp++) {
    double mseh = 0.0;
    double msev = 0.0;
    double mse  = 0.0;
    ULONG  w   = src->WidthOf(comp);
    ULONG  h   = src->HeightOf(comp);
    double prc = (src->isFloat(comp))?(1.0):(double(UQUAD(1) << src->BitsOf(comp)) - 1.0);
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	mseh = MSE_Hor<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
	msev = MSE_Ver<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
 	mse  = MSE<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h);
     } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	mseh = MSE_Hor<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
	msev = MSE_Ver<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
	mse  = MSE<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h);
       } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	mseh = MSE_Hor<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
	msev = MSE_Ver<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   w,h);
	mse  = MSE<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	mseh = MSE_Hor<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	msev = MSE_Ver<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	mse  = MSE<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	mseh = MSE_Hor<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     w,h);
	msev = MSE_Ver<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     w,h);
	mse  = MSE<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	mseh = MSE_Hor<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	msev = MSE_Ver<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	mse  = MSE<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	mseh = MSE_Hor<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	msev = MSE_Ver<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	mse  = MSE<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	mseh = MSE_Hor<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	msev = MSE_Ver<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	mse  = MSE<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	mseh = MSE_Hor<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	msev = MSE_Ver<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    w,h);
	mse  = MSE<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	mseh = MSE_Hor<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     w,h);
	msev = MSE_Ver<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     w,h);
	mse  = MSE<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h);
      } else {
	throw "unsupported data type";
      }
    }
    //
    mseh /= (w * h) * prc * prc;
    msev /= (w * h) * prc * prc;
    mse  /= (w * h) * prc * prc;
    //
    mse   = (mse - msev) - (mse - mseh);
    if (mse < 0)
      mse = -mse;
    if (mse > max)
      max = mse;
    //
  }
    
  return -10.0 * log(max) / log(10.0);
}
///
