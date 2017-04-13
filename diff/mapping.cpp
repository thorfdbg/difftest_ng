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
** $Id: mapping.cpp,v 1.13 2017/02/13 09:42:05 thor Exp $
**
** This class works like the scaler, but more elaborate as it allows a couple
** of less trivial conversions: gamma mapping, log mapping and half-log mapping.
*/

/// Includes
#include "diff/mapping.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///


/// Mapping::ToGamma
// Convert to int using a gamma mapping.
template<typename S,typename T>
void Mapping::ToGamma(const S *org ,ULONG obytesperpixel,ULONG obytesperrow,
		      T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		      ULONG w, ULONG h, double scale, double limF, double gamma)
{
  ULONG x,y;
  double invgamma = 1.0 / gamma;
  
  for(y = 0;y < h;y++) {
    const S *orgrow = org;
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v    = *orgrow;
      if (v < 0.0 || isnan(v)) {
	v = 0.0;
      } else if (isinf(v)) {
	v = scale;
      } else {
	v = scale * pow(v / limF,invgamma);
	if (v > scale)
	  v = scale;
      }
      *dstrow = T(v + 0.5);
      orgrow  = (const S *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (T *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const S *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Mapping::InvGamma
// Inverse gamma mapping of the above.
template<typename S,typename T>
void Mapping::InvGamma(const S *src  ,ULONG obytesperpixel,ULONG obytesperrow,
		       T *dst        ,ULONG dbytesperpixel,ULONG dbytesperrow,
		       ULONG w, ULONG h, double scale, double outscale, double gamma)
{ 
  ULONG x,y;
  double invscale = 1.0 / scale;
  
  for(y = 0;y < h;y++) {
    const S *orgrow = src;
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v    = *orgrow;
      v = outscale * pow(v * invscale,gamma);
      *dstrow = v;
      orgrow  = (const S *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (T       *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    src = (const S *)((const UBYTE *)(src) + obytesperrow);
    dst = (T       *)((UBYTE *)(dst)       + dbytesperrow);
  }
}
///

/// Mapping::ToHalfLog
// Convert to int using a half-log map.
void Mapping::ToHalfLog(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
			UWORD *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
			ULONG w, ULONG h)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    const FLOAT *orgrow = org;
    UWORD *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v  = *orgrow;
      bool sign = (v < 0.0)?(true):(false);
      int  exponent;
      int  mantissa;
      
      if (v < 0.0) v = -v;
      
      if (isinf(v)) {
	exponent = 31;
	mantissa = 0;
      } else if (v == 0.0) {
	exponent = 0;
	mantissa = 0;
      } else {
	double man = 2.0 * frexp(v,&exponent); // must be between 1.0 and 2.0, not 0.5 and 1.
	// Add the exponent bias.
	exponent  += 15 - 1; // exponent bias
	// Normalize the exponent by modifying the mantissa.
	if (exponent >= 31) { // This must be denormalized into an INF, no chance.
	  exponent = 31;
	  mantissa = 0;
	} else if (exponent <= 0) {
	  man *= 0.5; // mantissa does not have an implicit one bit.
	  while(exponent < 0) {
	    man *= 0.5;
	    exponent++;
	  }
	  mantissa = int(man * (1 << 10));
	} else {
	  mantissa = int(man * (1 << 10)) & ((1 << 10) - 1);
	}
      }

      UWORD out = ((sign)?(0x8000):(0x0000)) | (exponent << 10) | mantissa;
      *dstrow   = out;
      //
      orgrow  = (const FLOAT *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (UWORD *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const FLOAT *)((const UBYTE *)(org) + obytesperrow);
    dst = (UWORD *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Mapping::ToLog
void Mapping::ToLog(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
		    FLOAT *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		    ULONG w, ULONG h)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    const FLOAT *orgrow = org;
    FLOAT *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v  = *orgrow;
      
      if (v < m_dGamma) {
	v = m_dGamma;
      }
      if (v < 0.0)
	v = 0.0;

      *dstrow = log(v);
      //
      orgrow  = (const FLOAT *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (FLOAT *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const FLOAT *)((const UBYTE *)(org) + obytesperrow);
    dst = (FLOAT *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Mapping::ToPU2
// Apply mapping to a perceptually uniform space with Rafal's PU-Map. The output
// is identical to the matlab script except that it is not scaled by 255. If you
// want to do that, use --touns 8.
void Mapping::ToPU2(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
		    FLOAT *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		    ULONG w, ULONG h)
{ 
  ULONG x,y;

  assert(m_PU_Lut);

  for(y = 0;y < h;y++) {
    const FLOAT *orgrow = org;
    FLOAT *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v  = *orgrow; 
      int n; // entry in the lookup
      double frc,x;
      double pt;
      double pu_l = 31.9270;
      double pu_h = 149.9244;
      double ylo,yhi;
      double l    = (v > 0.0)?(log(v) / log(10.0)):(-5.0);
    
      if (l < -5.0) {
	l = -5.0;
      } else if (l > 10.0) {
	l = 10.0;
      }
      
      x   = (l + 5.0) / 15.0 * 4095;
      n   = (int)(x);
      frc = x - n;
      ylo = m_PU_Lut[n];
      yhi = m_PU_Lut[(n+1 < 4096)?(n+1):4095];
      pt  = (1.0 - frc) * ylo + frc * yhi;
      
      *dstrow = (pt - pu_l) / (pu_h - pu_l);
      //
      orgrow  = (const FLOAT *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (FLOAT *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const FLOAT *)((const UBYTE *)(org) + obytesperrow);
    dst = (FLOAT *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Mapping::ToHalfExp
// Inverse computation from half-log to int.
void Mapping::ToHalfExp(const UWORD *org ,ULONG obytesperpixel,ULONG obytesperrow,
			FLOAT *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
			ULONG w, ULONG h)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    const UWORD *orgrow = org;
    FLOAT *dstrow       = dst;
    for(x = 0;x < w;x++) {
      UWORD h = *orgrow;
      bool sign      = (h & 0x8000)?(true):(false);
      UBYTE exponent = (h >> 10) & ((1 << 5) - 1);
      UWORD mantissa = h & ((1 << 10) - 1);
      double v;

      if (exponent == 0) { // denormalized
	v = ldexp(float(mantissa),-14-10);
      } else if (exponent == 31) {
	v = HUGE_VAL;
      } else {
	v = ldexp(float(mantissa | (1 << 10)),-15-10+exponent);
      }

      *dstrow = (sign)?(-v):(v);
      
      orgrow  = (const UWORD *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (FLOAT *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const UWORD *)((const UBYTE *)(org) + obytesperrow);
    dst = (FLOAT *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Mapping::FromPQ
// Compute the luminances from PQ-values
template<typename T>
void Mapping::FromPQ(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
		     FLOAT *dst   ,ULONG dbytesperpixel,ULONG dbytesperrow,
		     ULONG w, ULONG h, double scale)
{
  ULONG x,y;
  const double m1 = 2610.0 / 4096.0 * 0.25;
  const double m2 = 2523.0 / 4096.0 * 128.0;
  const double c1 = 3424.0 / 4096.0;
  const double c2 = 2413.0 / 4096.0 * 32.0;
  const double c3 = 2392.0 / 4096.0 * 32.0;
  const double lmax = 10000;
  
  for(y = 0;y < h;y++) {
    const T *orgrow = org;
    FLOAT *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double n = double(*orgrow) / scale; // normalized sample value
      double l = pow((pow(n,1.0 / m2) - c1) / (c2 - c3 * pow(n,1.0 / m2)),1.0 / m1);
      *dstrow  = l * lmax;
      orgrow  = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (FLOAT *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const T *)((const UBYTE *)(org) + obytesperrow);
    dst = (FLOAT *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Mapping::ToPQ
// Compute PQ-values from luminances.
template<typename T>
void Mapping::ToPQ(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
		   T *dst           ,ULONG dbytesperpixel,ULONG dbytesperrow,
		   ULONG w, ULONG h, double scale)
{
  ULONG x,y;
  const double m1 = 2610.0 / 4096.0 * 0.25;
  const double m2 = 2523.0 / 4096.0 * 128.0;
  const double c1 = 3424.0 / 4096.0;
  const double c2 = 2413.0 / 4096.0 * 32.0;
  const double c3 = 2392.0 / 4096.0 * 32.0;
  const double lmax = 10000; // peak luminance
  
  for(y = 0;y < h;y++) {
    const FLOAT *orgrow = org;
    T *dstrow           = dst;
    for(x = 0;x < w;x++) {
      double l = *orgrow / lmax; // luminance (hopefully in nits)
      double n = pow((c2*pow(l,m1) + c1)/(c3*pow(l,m1) + 1.0),m2);
      if (n > 1.0)
	n = 1.0;
      *dstrow  = T(n * scale);
      orgrow  = (const FLOAT *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow  = (T *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const FLOAT *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Mapping::~Mapping
Mapping::~Mapping(void)
{
  int i;

  if (m_ppucImage) {
    for(i = 0;i < m_usDepth;i++) {
      if (m_ppucImage[i])
        delete[] m_ppucImage[i];
    }
    delete[] m_ppucImage;
  }

  delete m_pDest;
  delete[] m_PU_Lut;
}
///

/// Mapping::CreatePUMap
// Create the lookup table for the perceptually uniform map.
void Mapping::CreatePUMap(void)
{
  int i;
  double sum  = 0.0;

  assert(m_PU_Lut == NULL);

  m_PU_Lut = new double[4096]; 

  for(i = 0;i < 4096;i++) {
    double cvi_sens_drop   = 4.0627;
    double cvi_trans_slope = 1.6596;
    double cvi_low_slope   = 0.2712;
    double scale           = 30.162;
    double y     = -5.0 + 15.0 * i / 4095.0;
    double l_lut = pow(10.0,y);
    double s = scale * pow(pow((cvi_sens_drop / l_lut),cvi_trans_slope) + 1.0,-cvi_low_slope);
    double thr = l_lut / s;
    double dL  = 1 / thr * l_lut * log(10.0);
    sum       += dL;
    if (i > 0) {
      m_PU_Lut[i]  = (y + 5.0) / (2 * i) * sum;
      sum         += dL;
    } else {
      m_PU_Lut[i] = 0.0;
    }
  }
}
///

/// Mapping::ComputeLimF
// Compute the limF value as the 95% percentile of the luminance for
// greyscale.
double Mapping::ComputeLimF(const FLOAT *org,ULONG obytesperpixel,ULONG obytesperrow,
			    ULONG w,ULONG h)
{
  ULONG x,y;
  ULONG count[65536] = {0};
  ULONG total = 0;
  ULONG percentile,cnt;
  double min = HUGE_VAL,max = -HUGE_VAL;
  const FLOAT *p = org;
  int i;

  for(y = 0;y < h;y++) {
    const FLOAT *orgrow = p;
    for(x = 0;x < w;x++) {
      double v = *orgrow;
      if (v >= 0.0 && !isinf(v) && !isnan(v)) {
	if (v < min)
	  min = v;
	if (v > max)
	  max = v;
      }
      orgrow  = (const FLOAT *)((const UBYTE *)(orgrow) + obytesperpixel);
    }
    p = (const FLOAT *)((const UBYTE *)(p) + obytesperrow);
  }
  //
  if (max > min) {
    for(y = 0;y < h;y++) {
      const FLOAT *orgrow = org;
      for(x = 0;x < w;x++) {
	double v = *orgrow;
	if (v >= 0.0 && !isinf(v) && !isnan(v)) {
	  ULONG w = ULONG(65535 * (v - min) / (max - min));
	  if (w < 65536) {
	    total++;
	    count[w]++;
	  }
	}
	orgrow  = (const FLOAT *)((const UBYTE *)(orgrow) + obytesperpixel);
      }
      org = (const FLOAT *)((const UBYTE *)(org) + obytesperrow);
    }
    //
    if (total > 0) {
      // Find the 5%% percentile.
      percentile = total * 0.005;
      cnt        = 0;
      for(i = 65535;i >= 0;i--) {
	cnt += count[i];
	if (cnt >= percentile)
	  break;
      }
      double w = (i + 0.5) / 65535.0;
      // Use the inverse function to get the approximate inverse percentile.
      return w * (max - min) + min;
    }
  }
  //
  // Whatever...
  return 1.0;
}
///

/// Mapping::ComputeLimF
// Compute the limit for RGB images, making a conversion from "something like 601" RGB to
// YCbCr, and only looking at luma.
double Mapping::ComputeLimF(const FLOAT *r,ULONG rbytesperpixel,ULONG rbytesperrow,
			    const FLOAT *g,ULONG gbytesperpixel,ULONG gbytesperrow,
			    const FLOAT *b,ULONG bbytesperpixel,ULONG bbytesperrow,
			    ULONG w,ULONG h)
{ 
  ULONG x,y;
  ULONG count[65536] = {0};
  ULONG total = 0;
  ULONG percentile,cnt;
  const FLOAT *rp = r,*gp = g,*bp = b;
  double min = HUGE_VAL;
  double max = -HUGE_VAL;
  int i;

  for(y = 0;y < h;y++) {
    const FLOAT *rorgrow = rp;
    const FLOAT *gorgrow = gp;
    const FLOAT *borgrow = bp;
    for(x = 0;x < w;x++) {
      // This is probably not the correct color space, but it should be good enough to estimate a
      // range for the images.
      double v = *rorgrow * 0.299 + *gorgrow * 0.587 + *borgrow * 0.114;
      if (v >= 0.0 && !isinf(v) && !isnan(v)) {
	if (v < min)
	  min = v;
	if (v > max)
	  max = v;
      }
      rorgrow  = (const FLOAT *)((const UBYTE *)(rorgrow) + rbytesperpixel);
      gorgrow  = (const FLOAT *)((const UBYTE *)(gorgrow) + gbytesperpixel);
      borgrow  = (const FLOAT *)((const UBYTE *)(borgrow) + bbytesperpixel);
    }
    rp = (const FLOAT *)((const UBYTE *)(rp) + rbytesperrow);
    gp = (const FLOAT *)((const UBYTE *)(gp) + gbytesperrow);
    bp = (const FLOAT *)((const UBYTE *)(bp) + bbytesperrow);
  } 
  //
  // Second loop, compute the histogram.
  if (max > min) {
    for(y = 0;y < h;y++) {
      const FLOAT *rorgrow = r;
      const FLOAT *gorgrow = g;
      const FLOAT *borgrow = b;
      for(x = 0;x < w;x++) {
	// This is probably not the correct color space, but it should be good enough to estimate a
	// range for the images.
	double v = *rorgrow * 0.299 + *gorgrow * 0.587 + *borgrow * 0.114;
	if (v >= 0.0 && !isinf(v) && !isnan(v)) {
	  ULONG w = ULONG(65535 *(v - min) / (max - min));
	  if (w < 65535) {
	    total++;
	    count[w]++;
	  }
	}
	rorgrow  = (const FLOAT *)((const UBYTE *)(rorgrow) + rbytesperpixel);
	gorgrow  = (const FLOAT *)((const UBYTE *)(gorgrow) + gbytesperpixel);
	borgrow  = (const FLOAT *)((const UBYTE *)(borgrow) + bbytesperpixel);
      }
      r = (const FLOAT *)((const UBYTE *)(r) + rbytesperrow);
      g = (const FLOAT *)((const UBYTE *)(g) + gbytesperrow);
      b = (const FLOAT *)((const UBYTE *)(b) + bbytesperrow);
    }
    //
    if (total > 0) {
      // Find the 5%% percentile.
      percentile = total * 0.005;
      cnt        = 0;
      for(i = 65535;i >= 0;i--) {
	cnt += count[i];
	if (cnt >= percentile)
	  break;
      }
      double w = (i + 0.5) / 65535.0;
      // Use the inverse function to get the approximate inverse percentile.
      return w * (max - min) + min;
    }
  }
  //
  // Whatever...
  return 1.0;
}
///

/// Mapping::ApplyMap
// Apply a map from the source image to the target image that must be
// already initialized.
void Mapping::ApplyMap(class ImageLayout *src,class ImageLayout *dst)
{ 
  UWORD comp;
  double limf = 1.0;
  //
  // Need to compute the 95% percentile?
  if (m_Type == Gamma && m_bInverse == false) {
    if (src->DepthOf() == 1) {
      if (src->isFloat(0)) {
	if (src->BitsOf(0) != 16 && src->BitsOf(0) != 32)
	  throw "this conversion tool expects 16 bit half float or float input";
	limf = ComputeLimF((const FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			 src->WidthOf(0),src->HeightOf(0));
      }
    } else if (src->DepthOf() == 3) {
      if (src->isFloat(0) && src->isFloat(1) && src->isFloat(2)) {
	if (src->WidthOf(0) != src->WidthOf(1) || src->HeightOf(0) != src->HeightOf(1) ||
	    src->WidthOf(0) != src->WidthOf(2) || src->HeightOf(0) != src->HeightOf(2))
	  throw "gamma conversion from RGB input works only for non-subsampled input"; 
	if ((src->BitsOf(0) != 16 && src->BitsOf(0) != 32) ||
	    (src->BitsOf(1) != 16 && src->BitsOf(1) != 32) ||
	    (src->BitsOf(2) != 16 && src->BitsOf(2) != 32))
	  throw "this conversion tool expects 16 bit half float or float input";
	limf = ComputeLimF((const FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			   (const FLOAT *)src->DataOf(1),src->BytesPerPixel(1),src->BytesPerRow(1),
			   (const FLOAT *)src->DataOf(2),src->BytesPerPixel(2),src->BytesPerRow(2),
			   src->WidthOf(0),src->HeightOf(0));
      }
    }
  }
  //
  assert(src->DepthOf() == dst->DepthOf());
  //
  if (m_Type == PU2)
    CreatePUMap();
  //
  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    double lim  = limf;
    //
    if (m_bInverse) { 
      // Integer to float.
      if (src->isFloat(comp))
	throw "this conversion tool operates on integer input only";
      if (src->isSigned(comp))
	throw "this conversion tool works on unsigned integers only";
      //
      // make sure the target is float.
      assert(dst->isFloat(comp));
      assert(!dst->isSigned(comp));
      assert(dst->BitsOf(comp) == 32);
      assert(dst->WidthOf(comp) == w);
      assert(dst->HeightOf(comp) == h);
    } else {
      if (m_Type == Gamma) {
	if (!src->isFloat(comp) && src->isSigned(comp))
	  throw "this conversion tool expects floating point or unsigned input";
	if (!src->isFloat(comp))
	  lim = ((1UL << (src->BitsOf(comp))) - 1);
      } else {
	if (!src->isFloat(comp))
	  throw "this conversion tool expects floating point input";
	if (m_Type != PQ && src->BitsOf(comp) != 16 && src->BitsOf(comp) != 32)
	  throw "this conversion tool expects 16 bit half float or float input";
      }
      //
      // Ensure the target has the right depths.
      if (m_Type == Log || m_Type == PU2) {
	assert(dst->isFloat(comp));
	assert(!dst->isSigned(comp));
	assert(dst->BitsOf(comp) == 32);
      } else {
	assert(!dst->isFloat(comp));
	assert(!dst->isSigned(comp));
	assert(dst->BitsOf(comp) == m_ucTargetDepth);
      }
    } 
    //
    //
    switch(m_Type) {
    case Gamma:
      if (m_bInverse) {
	if (src->BitsOf(comp) <= 8) {
	  InvGamma<UBYTE,FLOAT>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h,(1UL << src->BitsOf(comp)) - 1,1.0,m_dGamma);
	} else if (src->BitsOf(comp) <= 16) {
	  InvGamma<UWORD,FLOAT>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h,(1UL << src->BitsOf(comp)) - 1,1.0,m_dGamma);
	} else if (src->BitsOf(comp) <= 32) {
	  InvGamma<ULONG,FLOAT>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				w,h,(1UL << src->BitsOf(comp)) - 1,1.0,m_dGamma);
	} else throw "unsupported source bit depth, must be between 1 and 32 bits per pixel";
      } else {
	if (src->isFloat(comp)) {
	  if (m_ucTargetDepth <= 8) {
	    ToGamma<FLOAT,UBYTE>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 16) {
	    ToGamma<FLOAT,UWORD>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 32) {	  
	    ToGamma<FLOAT,ULONG>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else throw "unsupported target bit depth, must be between 1 and 32 bits per pixel";
	} else if (src->BitsOf(comp) <= 8) {
	  if (m_ucTargetDepth <= 8) {
	    ToGamma<UBYTE,UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 16) {
	    ToGamma<UBYTE,UWORD>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 32) {	  
	    ToGamma<UBYTE,ULONG>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else throw "unsupported target bit depth, must be between 1 and 32 bits per pixel";
	} else if (src->BitsOf(comp) <= 16) {
	  if (m_ucTargetDepth <= 8) {
	    ToGamma<UWORD,UBYTE>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 16) {
	    ToGamma<UWORD,UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 32) {	  
	    ToGamma<UWORD,ULONG>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else throw "unsupported target bit depth, must be between 1 and 32 bits per pixel";
	} else if (src->BitsOf(comp) <= 32) {
	  if (m_ucTargetDepth <= 8) {
	    ToGamma<ULONG,UBYTE>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 16) {
	    ToGamma<ULONG,UWORD>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else if (m_ucTargetDepth <= 32) {	  
	    ToGamma<ULONG,ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				 w,h,(1UL << m_ucTargetDepth) - 1,lim,m_dGamma);
	  } else throw "unsupported target bit depth, must be between 1 and 32 bits per pixel";
	}
      }
      break;
    case HalfLog:
      if (m_bInverse) {
	if (src->BitsOf(comp) != 16 || src->isSigned(comp))
	  throw "source data must be 16 bit unsigned integer";
	ToHalfExp((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		  (FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		  w,h);
      } else {
	if (m_ucTargetDepth != 16)
	  throw "this tool converts data to 16 bit unsigned integer";
	ToHalfLog((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		  (UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		  w,h);
      }
      break;
    case Log:
      ToLog((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
	    (FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
	    w,h);
      break;
    case PU2:
      ToPU2((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
	    (FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
	    w,h);
      break;
    case PQ: // Inverse PQ, i.e. from PQ to luminances.
      if (m_bInverse) {
	if (src->BitsOf(comp) <= 8) {
	  FromPQ<UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			(FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			w,h,(1UL << src->BitsOf(comp)) - 1);
	} else if (src->BitsOf(comp) <= 16) {
	  FromPQ<UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			(FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			w,h,(1UL << src->BitsOf(comp)) - 1);
	} else if (src->BitsOf(comp) <= 32) {
	  FromPQ<ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			(FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			w,h,(1UL << src->BitsOf(comp)) - 1);
	}
      } else {
	if (m_ucTargetDepth <= 8) {
	  ToPQ<UBYTE>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      (UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		      w,h,(1UL << m_ucTargetDepth) - 1);
	} else if (m_ucTargetDepth <= 16) {
	   ToPQ<UWORD>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       (UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		       w,h,(1UL << m_ucTargetDepth) - 1);
	} else if (m_ucTargetDepth <= 32) {
	  ToPQ<ULONG>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      (ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		      w,h,(1UL << m_ucTargetDepth) - 1);
	}
      }
      break;
    default:
      throw "unsupported conversion requested";
      break;
    }
  }
}
///

/// Mapping::CreateTargetBuffer
// Create a buffer for the target image.
void Mapping::CreateTargetBuffer(class ImageLayout *src)
{
  UWORD comp;
  
  CreateComponents(*src);
  m_ppucImage = new UBYTE *[src->DepthOf()];
  memset(m_ppucImage,0,sizeof(UBYTE *) * src->DepthOf());

  // Create components for the target image.
  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    //
    if (m_bInverse) { 
      // Integer to float.
      if (src->isFloat(comp))
	throw "this conversion tool operates on integer input only";
      if (src->isSigned(comp))
	throw "this conversion tool works on unsigned integers only";
      //
      FLOAT *mem = new FLOAT[w * h];
      m_ppucImage[comp] = (UBYTE *)mem;
      //
      m_pComponent[comp].m_ucBits          = 32;
      m_pComponent[comp].m_bSigned         = false;
      m_pComponent[comp].m_bFloat          = true;
      m_pComponent[comp].m_ulWidth         = w;
      m_pComponent[comp].m_ulHeight        = h;
      m_pComponent[comp].m_ulBytesPerPixel = sizeof(FLOAT);
      m_pComponent[comp].m_ulBytesPerRow   = w * sizeof(FLOAT);
      m_pComponent[comp].m_pPtr            = mem;
    } else {
      if (m_Type == Gamma) {
	if (!src->isFloat(comp) && src->isSigned(comp))
	  throw "this conversion tool expects unsigned or floating point input";
      } else {
	if (!src->isFloat(comp))
	  throw "this conversion tool expects floating point input";
	if (src->BitsOf(comp) != 16 && src->BitsOf(comp) != 32)
	  throw "this conversion tool expects 16 bit half float or float input";
      }
      //
      if (m_Type == Log || m_Type == PU2) {
	FLOAT *mem = new FLOAT[w * h];
	m_ppucImage[comp] = (UBYTE *)mem;
	//
	m_pComponent[comp].m_ucBits          = m_ucTargetDepth;
	m_pComponent[comp].m_bSigned         = false;
	m_pComponent[comp].m_bFloat          = true;
	m_pComponent[comp].m_ulWidth         = w;
	m_pComponent[comp].m_ulHeight        = h;
	m_pComponent[comp].m_ulBytesPerPixel = sizeof(FLOAT);
	m_pComponent[comp].m_ulBytesPerRow   = w * sizeof(FLOAT);
	m_pComponent[comp].m_pPtr            = mem;
      } else {
	UBYTE bps  = (m_ucTargetDepth + 7) >> 3;
	UBYTE *mem = new UBYTE[w * h * bps];
	m_ppucImage[comp] = mem;
	//
	m_pComponent[comp].m_ucBits          = m_ucTargetDepth;
	m_pComponent[comp].m_bSigned         = false;
	m_pComponent[comp].m_bFloat          = false;
	m_pComponent[comp].m_ulWidth         = w;
	m_pComponent[comp].m_ulHeight        = h;
	m_pComponent[comp].m_ulBytesPerPixel = bps;
	m_pComponent[comp].m_ulBytesPerRow   = w * bps;
	m_pComponent[comp].m_pPtr            = mem;
      }
    } 
  }
}
///

/// Mapping::Measure
double Mapping::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{ 
  if (m_bFilter) { 
    CreateTargetBuffer(src);
    //
    // Now apply the conversion.
    ApplyMap(src,this);
    //
    assert(m_pDest == NULL);
    m_pDest = new class Mapping(NULL,m_Type,m_dGamma,m_bInverse,m_ucTargetDepth,true,m_TargetSpecs);
    //
    m_pDest->CreateTargetBuffer(dst);
    m_pDest->ApplyMap(dst,m_pDest);
    //
    // Replace now the original images by the modified versions.
    src->Swap(*this);
    dst->Swap(*m_pDest);
  } else {
    CreateTargetBuffer(src);
    //
    // Now apply the conversion.
    ApplyMap(src,this);
    
    SaveImage(m_pTargetFile,m_TargetSpecs);
  }

  return in;
}
///

