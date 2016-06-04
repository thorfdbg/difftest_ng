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
** $Id: fftimg.cpp,v 1.13 2016/06/04 10:44:08 thor Exp $
**
** This class saves the FFT of the difference image as a normalized 8bpp image
** with the same number of components as the original.
*/

/// Includes
#include "diff/fftimg.hpp"
#include "tools/fft.hpp"
#include "std/math.hpp"
#include "std/string.hpp"
#ifdef USE_GSL
///

/// FFTImg::CopyToFFT
template<typename T>
void FFTImg::CopyToFFT(T *org,ULONG obytesperpixel,ULONG obytesperrow,
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

/// FFTImg::~FFTImg
FFTImg::~FFTImg(void)
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

}
///

/// FFTImg::Measure
double FFTImg::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{ 
  UWORD comp;
  double max = 0.0;

  CreateComponents(*src);
  m_ppucImage = new UBYTE *[src->DepthOf()];
  memset(m_ppucImage,0,sizeof(UBYTE *) * src->DepthOf());
  m_ppFFT     = new class FFT *[src->DepthOf()];
  memset(m_ppFFT,0,sizeof(class FFT *) * src->DepthOf());

  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    UBYTE *mem  = new UBYTE[w * h];
    class FFT *fft;
    ULONG  x,y;
    //
    m_ppucImage[comp]                    = mem;
    m_pComponent[comp].m_ucBits          = 8;
    m_pComponent[comp].m_bSigned         = false;
    m_pComponent[comp].m_ulWidth         = w;
    m_pComponent[comp].m_ulHeight        = h;
    m_pComponent[comp].m_ulBytesPerPixel = 1;
    m_pComponent[comp].m_ulBytesPerRow   = w;
    m_pComponent[comp].m_pPtr            = mem;
    m_ppFFT[comp] = fft                  = new class FFT(w,h,m_bWindow);
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
    // Normalize the components.
    for(y = 0;y < h;y++) {
      const double *data = fft->DataOf() + y * fft->ModuloOf();
      for(x = 0;x < w;x++,data += 2) {
	double v = sqrt(data[0] * data[0] + data[1] * data[1]);
	if (v > max && x != 0 && y != 0)
	  max = v;
      }
    }
  }
  //
  // Avoid division by zero if identical.
  if (max <= 0.0)
    max = 1.0;
  //
  for(comp = 0;comp < src->DepthOf();comp++) {
    class FFT *fft = m_ppFFT[comp];
    UBYTE *mem     = m_ppucImage[comp];
    ULONG  w       = src->WidthOf(comp);
    ULONG  h       = src->HeightOf(comp);
    ULONG x,y;
    //
    // Now fill in the target 
    for(y = 0;y < h;y++) {
      const double *data = fft->DataOf() + y * fft->ModuloOf();
      UBYTE *out         = mem + ((y < (h >> 1))?(y + (h >> 1)):(y - (h >> 1))) * w;
      out               += w >> 1;
      for(x = 0;x < w;x++,data += 2) {
	double v = sqrt(data[0] * data[0] + data[1] * data[1]) * 255 / max;
	UBYTE dt;
	if (v < 0.0) {
	  dt = 0;
	} else if (v > 255.0) {
	  dt = 255;
	} else {
	  dt = UBYTE(v);
	}
	if (x == (w >> 1))
	  out -= w;
	*out++ = dt;
      }
    }
  }

  SaveImage(m_pTargetFile);

  return in;
}
///

#endif
