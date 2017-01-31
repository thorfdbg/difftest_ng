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
** $Id: maxfreq.cpp,v 1.13 2017/01/31 11:58:04 thor Exp $
**
** This class locates the maximum frequency and returns the absolute value of this frequency
** as result.
*/

/// Includes
#include "diff/maxfreq.hpp"
#include "tools/fft.hpp"
#include "std/math.hpp"
#include "std/string.hpp"
#ifdef USE_GSL
///

/// MaxFreq::CopyToFFT
template<typename T>
void MaxFreq::CopyToFFT(T *org,ULONG obytesperpixel,ULONG obytesperrow,
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

/// MaxFreq::~MaxFreq
MaxFreq::~MaxFreq(void)
{
  int i;

  delete[] m_pdAbs;
  
  if (m_ppFFT) {
    for(i = 0;i < m_usDepth;i++) {
      if (m_ppFFT[i])
        delete m_ppFFT[i];
    }
    delete[] m_ppFFT;
  }

}
///

/// MaxFreq::Measure
double MaxFreq::Measure(class ImageLayout *src,class ImageLayout *dst,double)
{ 
  ULONG x,y;
  ULONG maxw = 0,maxh = 0;
  ULONG mod;
  UWORD comp;
  double fmax[16];
  ULONG  xm[16];
  ULONG  ym[16];

  m_usDepth  = src->DepthOf();
  m_ppFFT    = new FFT *[m_usDepth];
  memset(m_ppFFT,0,sizeof(class FFT *) * m_usDepth);

  memset(fmax,0,sizeof(fmax));
  memset(xm,0,sizeof(xm));
  memset(ym,0,sizeof(xm));

  for(comp = 0;comp < m_usDepth;comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    class FFT *fft;
    //
    m_ppFFT[comp] = fft = new class FFT(w,h,true);
    //
    if (w > maxw)
      maxw = w;
    if (h > maxh)
      maxh = h;
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
  }

  // Due to symmetry, only 1/4 needs to be investiaged.
  maxw  >>= 1;
  maxh  >>= 1;
  mod     = maxw;
  m_pdAbs = new double[maxw * maxh];
  
  for(y = 0;y < maxh;y++) {
    for(x = 0;x < maxw;x++) {
      double f = 0.0;
      for(comp = 0;comp < m_usDepth;comp++) {
	class FFT *fft = m_ppFFT[comp];
	if (x < fft->WidthOf() && y < fft->HeightOf()) {
	  double re = fft->DataOf()[y * fft->ModuloOf() + (x << 1) + 0];
	  double im = fft->DataOf()[y * fft->ModuloOf() + (x << 1) + 1];
	  f        += re * re + im * im;
	}
      }
      m_pdAbs[y * mod + x] = f;
    }
  }

  for(y = 0;y < maxh;y++) {
    for(x = 0;x < maxw;x++) {
      ULONG xp,yp;
      ULONG xmin = (x     > 3   )?(x - 3):(0);
      ULONG ymin = (y     > 3   )?(y - 3):(0);
      ULONG xmax = (x + 4 < maxw)?(x + 4):(maxw);
      ULONG ymax = (y + 4 < maxh)?(y + 4):(maxh);
      int i;
      double var = 0.0;
      for(yp = ymin;yp < ymax;yp++) {
	for(xp = xmin;xp < xmax;xp++) {
	  var += m_pdAbs[yp * mod + xp];
	}
      }
      if (var > 0.0) {
	var = var / ((xmax - xmin) * (ymax - ymin));
	var = m_pdAbs[y * mod + x] / var;
      } else {
	var = 1.0;
      }
      for(i = 0;i < 16;i++) {
	if (var > fmax[i]) {
	  if (i < 15) {
	    memmove(fmax+i+1,fmax+i,(15-i)*sizeof(double));
	    memmove(xm+i+1  ,xm+i  ,(15-i)*sizeof(ULONG));
	    memmove(ym+i+1  ,ym+i  ,(15-i)*sizeof(ULONG));
	  }
	  fmax[i] = var;
	  xm[i]   = x;
	  ym[i]   = y;
	  break;
	}
      }
    }
  }

  switch(m_Type) {
  case MaxH:
    return xm[0];
  case MaxV:
    return ym[0];
  case MaxR:
    return sqrt(double(xm[0]) * xm[0] + double(ym[0]) * ym[0]);
  case Var:
    return fmax[0];
  case Pattern:
    {
      double idxmax = 0.0;
      int i,j,k,l;
      
      for(i = 0;i < 16;i++) {
	ULONG x1 = xm[i], y1 = ym[i];
	for(j = i+1;j < 16;j++) {
	  double idx = 0.0;
	  ULONG x2 = xm[j], y2 = ym[j];
	  for(k = 0;k < 16;k++) {
	    if (i != k) {
	      ULONG x3 = xm[k],y3 = ym[k];
	      for(l = 0;l < 16;l++) {
		if (j != l) {
		  ULONG x4 = xm[l],y4 = ym[l];
		  if (x1 - x2 == x3 - x4 && y1 - y2 == y3 - y4) {
		    idx += fmax[i] + fmax[j] + fmax[k] + fmax[l];
		  }
		}
	      }
	    }
	  }
	  if (idx > idxmax) {
	    idxmax = idx;
	  }
	}
      }
      return idxmax;
    }
  }
  return 0.0;
}
///

#endif
