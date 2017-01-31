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
** $Id: colorhist.cpp,v 1.6 2017/01/31 11:58:03 thor Exp $
**
** This class saves the histogram to a file or writes it to
** stdout.
*/

/// Includes
#include "diff/colorhist.hpp"
#include "img/imglayout.hpp"
#include "std/string.hpp"
#include "std/assert.hpp"
///

/// ColorHistogram::Measure
template<typename T>
void ColorHistogram::Measure(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
			     T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
			     ULONG w      ,ULONG h,ULONG *hist,double mult)
{
  ULONG x,y;
  
   for(y = 0;y < h;y++) {
    T *orgrow     = org;
    T *dstrow     = dst;
    for(x = 0;x < w;x++) {
      double diff = (double(*orgrow) - double(*dstrow)) * mult;
      if (diff < -5.5) {
	hist[0]++;
      } else if (diff < -4.5) {
	hist[1]++;
      } else if (diff < -3.5) {
	hist[2]++;
      } else if (diff < -2.5) {
	hist[3]++;
      } else if (diff < -1.5) {
	hist[4]++;
      } else if (diff < -0.5) {
	hist[5]++;
      } else if (diff <  0.5) {
	hist[6]++;
      } else if (diff <  1.5) {
	hist[7]++;
      } else if (diff <  2.5) {
	hist[8]++;
      } else if (diff <  3.5) {
	hist[9]++;
      } else if (diff <  4.5) {
	hist[10]++;
      } else if (diff <  5.5) {
	hist[11]++;
      } else {
	hist[12]++;
      }
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
   }
}
///

/// ColorHistogram::~ColorHistogram
ColorHistogram::~ColorHistogram(void)
{
  delete[] m_pulHist;
}
///

/// ColorHistogram::Measure
// Measure the histogram
double ColorHistogram::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  int i;
  UWORD comp;
  
  src->TestIfCompatible(dst);
  
  assert(m_pulHist == NULL);

  m_pulHist = new ULONG[src->DepthOf() << 4];
  memset(m_pulHist,0,(src->DepthOf() << 4) * sizeof(ULONG));
  
  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG w = src->WidthOf(comp);
    ULONG h = src->HeightOf(comp);
    
    if (src->isFloat(comp)) {
      if (src->BitsOf(comp) <= 32) {
	Measure<const FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			     (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			     w,h,m_pulHist + (comp << 4),m_dBucketSize);
      } else if (src->BitsOf(comp) == 64) {
	Measure<const DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      w,h,m_pulHist + (comp << 4),m_dBucketSize);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->isSigned(comp)) {
	if (src->BitsOf(comp) <= 8) {
	  Measure<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      w,h,m_pulHist + (comp << 4),m_dBucketSize);
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  Measure<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      w,h,m_pulHist + (comp << 4),m_dBucketSize);
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  Measure<const LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			      w,h,m_pulHist + (comp << 4),m_dBucketSize);
	} else {
	  throw "unsupported data type";
	}
      } else {
	if (src->BitsOf(comp) <= 8) {
	  Measure<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,m_pulHist + (comp << 4),m_dBucketSize);
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  Measure<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,m_pulHist + (comp << 4),m_dBucketSize);
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  Measure<const ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			       w,h,m_pulHist + (comp << 4),m_dBucketSize);
	} else {
	  throw "unsupported data type";
	}
      }
    }
  }

  for(i = 0;i <= 12;i++) {
    switch(i) {
    case 0:
      printf("< %+6.2g         \t",(i-5.5) / m_dBucketSize);
      break;
    case 12:
      printf("> %+6.2g         \t",(i-6.5) / m_dBucketSize);
      break;
    default:
      printf("  %+6.2g .. %+6.2g\t",(i-6.5) / m_dBucketSize,(i-5.5) / m_dBucketSize);
      break;
    }
    for(comp = 0;comp < src->DepthOf();comp++) {
      printf("%8u\t",m_pulHist[i + (comp << 4)]);
    }
    printf("\n");
  }
  
  return in;
}
///
