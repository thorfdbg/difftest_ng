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
** $Id: noise.cpp,v 1.1 2026/06/18 08:38:22 thor Exp $
**
** This class adds Gaussian or Poisson noise to the image
*/

/// Includes
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "img/imglayout.hpp"
#include "diff/noise.hpp"
///

/// Noise::addGaussian
template<typename T>
void Noise::addGaussian(T *org,T min,T max,ULONG bytesperpixel,ULONG bytesperrow,ULONG w,ULONG h,double sigma)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    T *src  = org;
    for(x = 0;x < w; x++) {
      double z;
      do {
	double u = random() / ((1UL << 31) - 1.0);
	double v = random() / ((1UL << 31) - 1.0);
	while(u == 0.0) {
	  u = random() / ((1UL << 31) - 1.0);
	}
	z = sqrt(-2.0 * log(u)) * cos(2 * M_PI * v) * sigma;
	if (*src + z >= min && *src + z <= max) break;
	z = sqrt(-2.0 * log(u)) * sin(2 * M_PI * v) * sigma;
      	if (*src + z >= min && *src + z <= max) break;
      } while(true);
      *src += z;
      src  = (T *)((UBYTE *)(src) + bytesperpixel);
    }
    org = (T *)((UBYTE *)(org) + bytesperrow);
  }
}
///

/// Noise::PoissonNoise
template<typename T>
void Noise::PoissonNoise(T *org,T max,ULONG bytesperpixel,ULONG bytesperrow,ULONG w,ULONG h)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    T *src  = org;
    for(x = 0;x < w; x++) {
      double emu = exp(-*src);
      T k = 0;
      double p   = 1.0;
      k = 0;
      do {
	p *= random() / ((1UL << 31) - 1.0);
	k++;
      } while(p > emu);
      k--;
      if (k > max)
	k = max;
      *src = k;
      src  = (T *)((UBYTE *)(src) + bytesperpixel);
    }
    org = (T *)((UBYTE *)(org) + bytesperrow);
  }
}
///
  
/// Noise::addGaussian
// add Gaussian noise to a source image
void Noise::addGaussian(class ImageLayout *img) const
{
  UWORD comp,d  = img->DepthOf();
  
  for(comp = 0;comp < d;comp++) {
    ULONG  w  = img->WidthOf(comp);
    ULONG  h  = img->HeightOf(comp);
    bool sign = img->isSigned(comp);
    UBYTE b   = img->BitsOf(comp);
    //
    if (img->BitsOf(comp) <= 8) {
      if (sign) {
	Noise::addGaussian<BYTE>((BYTE *)(img->DataOf(comp)),-(1 << (b - 1)),(1 << (b - 1)) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
      } else {
	Noise::addGaussian<UBYTE>((UBYTE *)(img->DataOf(comp)),0,(1 << b) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
      }
    } else if (!img->isFloat(comp) && img->BitsOf(comp) <= 16) { // 16 bit float is internally stored as 32 bit.
      if (sign) {
	Noise::addGaussian<WORD>((WORD *)(img->DataOf(comp)),-(1L << (b - 1)),(1L << (b - 1)) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
      } else {
	Noise::addGaussian<UWORD>((UWORD *)(img->DataOf(comp)),0,(1UL << b) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
      }
    } else if (img->BitsOf(comp) <= 32) {
      if (img->isFloat(comp)) {
	// This is stored in float, actually.
	Noise::addGaussian<FLOAT>((FLOAT *)(img->DataOf(comp)),-1.0 / 0.0,1.0 / 0.0,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
      } else if (sign) {
	Noise::addGaussian<LONG>((LONG *)(img->DataOf(comp)),-(1LL << (b - 1)),(1LL << (b - 1)) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
      } else {
	Noise::addGaussian<ULONG>((ULONG *)(img->DataOf(comp)),0,(1ULL << b) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
      }
    } else if (img->BitsOf(comp) <= 64 && img->isFloat(comp)) {
      Noise::addGaussian<DOUBLE>((DOUBLE *)(img->DataOf(comp)),-1.0 / 0.0,1.0 / 0.0,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h,m_dSigma);
    } else {
      throw "unsupported data type";
    }
  }
}
///

/// Noise::PoissonNoise
// add Poisson noise to a source image
void Noise::PoissonNoise(class ImageLayout *img) const
{
  UWORD comp,d  = img->DepthOf();
  
  for(comp = 0;comp < d;comp++) {
    ULONG  w  = img->WidthOf(comp);
    ULONG  h  = img->HeightOf(comp);
    bool sign = img->isSigned(comp);
    UBYTE b   = img->BitsOf(comp);
    //
    if (sign)
      throw "Poisson noise cannot be applied to signed samples";
    if (img->isFloat(comp))
      throw "Poison noise cannot be applied to floating point samples";
    //
    if (img->BitsOf(comp) <= 8) {
      Noise::PoissonNoise<UBYTE>((UBYTE *)(img->DataOf(comp)),(1 << b) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
    } else if (!img->isFloat(comp) && img->BitsOf(comp) <= 16) { // 16 bit float is internally stored as 32 bit.
      Noise::PoissonNoise<UWORD>((UWORD *)(img->DataOf(comp)),(1UL << b) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
    } else if (img->BitsOf(comp) <= 32) {
      Noise::PoissonNoise<ULONG>((ULONG *)(img->DataOf(comp)),(1ULL << b) - 1,img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
    } else {
      throw "unsupported data type";
    }
  }
}
///

/// Noise::Measure
double Noise::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  struct timeval tv;

  if (gettimeofday(&tv,NULL) == 0) {
    srandom(tv.tv_usec);
  }

  if (m_bPoisson) {
    PoissonNoise(src);
    PoissonNoise(dst);
  } else {
    addGaussian(src);
    addGaussian(dst);
  }

  return in;
}
///
