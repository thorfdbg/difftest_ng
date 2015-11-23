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
** $Id: fftfilt.cpp,v 1.11 2014/01/04 11:35:28 thor Exp $
**
** This class filters the image with an FFT and selects only frequences around a horizontal and
** vertical base frequency and a given radius.
*/

/// Includes
#include "diff/fftfilt.hpp"
#include "tools/fft.hpp"
#include "std/math.hpp"
#include "std/string.hpp"
#ifdef USE_GSL
///

/// FFTFilt::CopyToFFT
template<typename T>
void FFTFilt::CopyToFFT(T *org,ULONG obytesperpixel,ULONG obytesperrow,
			T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
			double *target,ULONG stride,ULONG w,ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *orgrow      = org;
    T *dstrow      = dst;
    double *trgrow = target + y * stride;
    for(x = 0;x < w;x++) {
      trgrow[0]   = *orgrow - *dstrow; // real part
      trgrow[1]   = 0.0; // imaginary part.
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
      trgrow     += 2;
    }
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// FFTFilt::CopyFromFFT
template<typename T>
void FFTFilt::CopyFromFFT(double *src,ULONG stride,ULONG w,ULONG h,
			  T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
			  double min,double max,double scale,double shift)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *dstrow      = dst;
    double *srcrow = src;
    for(x = 0;x < w;x++) {
      double v = srcrow[0] * scale + shift; // The imaginary part should be zero. Or close to zero...
      if (v < min) {
	*dstrow = min;
      } else if (v > max) {
	*dstrow = max;
      } else {
	*dstrow = T(v);
      }
      dstrow      = (T *)((UBYTE *)(dstrow) + dbytesperpixel);
      srcrow     += 2;
    }
    dst = (T *)((UBYTE *)(dst) + dbytesperrow);
    src += stride;
  }
}
///

/// FFTFilt::~FFTFilt
FFTFilt::~FFTFilt(void)
{
  int i;

  if (m_ppucImage) {
    for(i = 0;i < m_usDepth;i++) {
      if (m_ppucImage[i])
        delete[] m_ppucImage[i];
    }
    delete[] m_ppucImage;
  }

  if (m_ppFFT) {
    for(i = 0;i < m_usDepth;i++) {
      if (m_ppFFT[i])
        delete m_ppFFT[i];
    }
    delete[] m_ppFFT;
  }

  delete[] m_pdFilter;
}
///

/// FFTFilt::CreateFilter
// Create a filter in the given array with the given stride variable around the
// given center with the given radius
void FFTFilt::CreateFilter(double *filt,ULONG stride,ULONG w,ULONG h,ULONG xc,ULONG yc,ULONG xr,ULONG yr)
{
  LONG xmin = xc - xr;
  LONG xmax = xc + xr + 1;
  LONG ymin = yc - yr;
  LONG ymax = yc + yr + 1;
  LONG x,y;

  if (xmin < 0)
    xmin = 0;
  if (xmax > LONG(w))
    xmax = w;
  if (ymin < 0)
    ymin = 0;
  if (ymax > LONG(h))
    ymax = h;

  for(y = ymin;y < ymax;y++) {
    for(x = xmin;x < xmax;x++) {
      double v = cos((x - xc) * M_PI / (2.0 * xr)) * cos((y - yc) * M_PI / (2.0 * yr));
      if (filt[y * stride + x] < v)
	filt[y * stride + x] = v;
    }
  }
}
///

/// FFTFilt::NormalizeFilter
// Normalize the filter for l^1-norm 1.
void FFTFilt::NormalizeFilter(double *filt,ULONG stride,ULONG w,ULONG h)
{
  ULONG x,y;
  double norm = 0.0;
  
  for(y = 0;y < h;y++) {
    for(x = 0;x < w;x++) {
      norm += filt[y * stride + x];
    }
  }

  if (norm > 0.0) {
    norm /= w * h;
    for(y = 0;y < h;y++) {
      for(x = 0;x < w;x++) {
	filt[y * stride + x] /= norm;
      }
    }
  }
}
///

/// FFTFilt::FindMinMax
// Update minimum and maximum over the FFT output.
void FFTFilt::FindMinMax(double *fft,ULONG stride,ULONG w,ULONG h,double &min,double &max)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    for(x = 0;x < w;x++) {
      double v = fft[y * stride + (x << 1) + 0]; // only the real part.
      if (v < min)
	min = v;
      if (v > max)
	max = v;
    }
  }
}
///

/// FFTFilt::Measure
double FFTFilt::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{ 
  UWORD comp;
  double vmin = HUGE_VAL;
  double vmax = -vmin;
  
  src->TestIfCompatible(dst);

  CreateComponents(*src);
  m_ppucImage = new UBYTE *[src->DepthOf()];
  memset(m_ppucImage,0,sizeof(UBYTE *) * src->DepthOf());
  m_ppFFT     = new class FFT *[src->DepthOf()];
  memset(m_ppFFT,0,sizeof(class FFT *) * src->DepthOf());

  for(comp = 0;comp < src->DepthOf();comp++) {
    class FFT *fft;
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    UBYTE *mem;
    ULONG xr    = m_ulRadius;
    ULONG yr    = m_ulRadius;
    UBYTE sbpp  = ((src->BitsOf(comp) + 7) >> 3);
    ULONG x,y;
    if (src->isFloat(comp) && sbpp <= 4)
      sbpp = 4; // cludge for half-float
    //
    m_ppucImage[comp]                    = mem = new UBYTE[w * h * sbpp];
    m_pComponent[comp].m_ucBits          = src->BitsOf(comp);
    m_pComponent[comp].m_bSigned         = src->isSigned(comp);
    m_pComponent[comp].m_bFloat          = src->isFloat(comp);
    m_pComponent[comp].m_ulWidth         = w;
    m_pComponent[comp].m_ulHeight        = h;
    m_pComponent[comp].m_ulBytesPerPixel = sbpp;
    m_pComponent[comp].m_ulBytesPerRow   = w * sbpp;
    m_pComponent[comp].m_pPtr            = mem;
    m_ppFFT[comp] = fft                  = new class FFT(w,h,false);
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	CopyToFFT<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	CopyToFFT<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	CopyToFFT<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	CopyToFFT<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	CopyToFFT<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       fft->DataOf(),fft->ModuloOf(),w,h);	
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	CopyToFFT<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	CopyToFFT<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	CopyToFFT<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	CopyToFFT<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       fft->DataOf(),fft->ModuloOf(),w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	CopyToFFT<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				fft->DataOf(),fft->ModuloOf(),w,h);	      
      } else {
	throw "unsupported data type";
      }
    }
    // Run the transformation.
    fft->ForwardsFFT();
    //
    // Apply the filter
    delete[] m_pdFilter;
    m_pdFilter = NULL;
    m_pdFilter = new double[w * h];
    memset(m_pdFilter,0,sizeof(double) * w * h);
    //
    if (m_ulCombX && m_ulCombY == 0)
      yr = h;
    if (m_ulCombY && m_ulCombX == 0)
      xr = w;
    //
    // Create now the filter, note that due to the symmetry, four copies of the filter must be created.
    for(x = m_ulXC, y = m_ulYC;x < w && y < h; x += m_ulCombX, y += m_ulCombY) {
      CreateFilter(m_pdFilter,w,w,h,x        ,y        ,xr,yr);
      CreateFilter(m_pdFilter,w,w,h,w - 1 - x,y        ,xr,yr);
      CreateFilter(m_pdFilter,w,w,h,x        ,h - 1 - y,xr,yr);
      CreateFilter(m_pdFilter,w,w,h,w - 1 - x,h - 1 - y,xr,yr);
      //
      if (m_ulCombX == 0 && m_ulCombY == 0)
	break;
    }
    //
    //NormalizeFilter(m_pdFilter,w,w,h);
    //
    // Now apply the filter.
    for(y = 0;y < h;y++) {
      for(x = 0;x < w;x++) {
	fft->DataOf()[y * fft->ModuloOf() + (x << 1) + 0] *= m_pdFilter[y * w + x];
	fft->DataOf()[y * fft->ModuloOf() + (x << 1) + 1] *= m_pdFilter[y * w + x];
      }
    }
    //
    // Run the backwards FFT.
    fft->BackwardsFFT();
    //
    // Update minimum and maximum if required.
    if (m_bNormalize) {
      FindMinMax(fft->DataOf(),fft->ModuloOf(),w,h,vmin,vmax);
    }
  }
  //
  //
  for(comp = 0;comp < src->DepthOf();comp++) {
    class FFT *fft = m_ppFFT[comp];
    UBYTE *mem     = m_ppucImage[comp];
    ULONG  w       = src->WidthOf(comp);
    ULONG  h       = src->HeightOf(comp);
    QUAD   min;
    QUAD   max;
    double scale,shift;
    //
    if (src->isFloat(comp)) {
      min     = 0.0;
      max     = 1.0;
    } else {
      min     = src->isSigned(comp)?(-( QUAD(1) << (src->BitsOf(comp) - 1)))  :(0);
      max     = src->isSigned(comp)?( (UQUAD(1) << (src->BitsOf(comp) - 1))-1):((UQUAD(1) << src->BitsOf(comp))-1);
    }
    
    if (m_bNormalize && vmax > vmin) {
      scale        = (double(max) - double(min)) / (vmax - vmin);
      shift        = min - vmin * scale;
    } else {
      scale        = 1.0 / (w * h);
      shift        = 0.0;
    }
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	CopyFromFFT<BYTE>(fft->DataOf(),fft->ModuloOf(),w,h,
			  (BYTE *)mem,1,w,min,max,scale,shift);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	CopyFromFFT<WORD>(fft->DataOf(),fft->ModuloOf(),w,h,
			  (WORD *)mem,2,w << 1,min,max,scale,shift);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	CopyFromFFT<LONG>(fft->DataOf(),fft->ModuloOf(),w,h,
			  (LONG *)mem,4,w << 1,min,max,scale,shift);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	CopyFromFFT<FLOAT>(fft->DataOf(),fft->ModuloOf(),w,h,
			   (FLOAT *)mem,4,w << 2,min,max,scale,shift);

      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	CopyFromFFT<DOUBLE>(fft->DataOf(),fft->ModuloOf(),w,h,
			    (DOUBLE *)mem,8,w << 3,min,max,scale,shift);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	CopyFromFFT<UBYTE>(fft->DataOf(),fft->ModuloOf(),w,h,
			  (UBYTE *)mem,1,w,min,max,scale,shift);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	CopyFromFFT<UWORD>(fft->DataOf(),fft->ModuloOf(),w,h,
			  (UWORD *)mem,2,w << 1,min,max,scale,shift);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	CopyFromFFT<ULONG>(fft->DataOf(),fft->ModuloOf(),w,h,
			  (ULONG *)mem,4,w << 1,min,max,scale,shift);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	CopyFromFFT<FLOAT>(fft->DataOf(),fft->ModuloOf(),w,h,
			   (FLOAT *)mem,4,w << 2,min,max,scale,shift);

      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	CopyFromFFT<DOUBLE>(fft->DataOf(),fft->ModuloOf(),w,h,
			    (DOUBLE *)mem,8,w << 3,min,max,scale,shift);
      } else {
	throw "unsupported data type";
      }
    }
  }

  SaveImage(m_pTargetFile);

  return in;
}
///

#endif
