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
** $Id: histogram.cpp,v 1.11 2019/07/24 10:45:05 thor Exp $
**
** This class saves the histogram to a file or writes it to
** stdout.
*/

/// Includes
#include "diff/histogram.hpp"
#include "img/imglayout.hpp"
#include "std/assert.hpp"
#include "std/errno.hpp"
#include "std/string.hpp"
///

/// Histogram::Measure
template<typename T>
void Histogram::Measure(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
			T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
			ULONG w      ,ULONG h,ULONG *hist  ,LONG offset)
{
  ULONG x,y;
  
   for(y = 0;y < h;y++) {
    T *orgrow     = org;
    T *dstrow     = dst;
    for(x = 0;x < w;x++) {
      LONG diff = LONG(*orgrow) - LONG(*dstrow) + offset;
      assert(diff >= 0);
      hist[diff]++;
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
   }
}
///

/// Histogram::~Histogram
Histogram::~Histogram(void)
{
  delete[] m_pulHist;
}
///

/// Histogram::Measure
// Measure the histogram
double Histogram::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  ULONG size   = 0;
  ULONG offset = 0;
  ULONG i;
  UWORD comp;

  src->TestIfCompatible(dst);
  
  assert(m_pulHist == NULL);

  for(comp = 0;comp < src->DepthOf();comp++) {
    UBYTE bits = src->BitsOf(comp);
    ULONG tsiz = 2 * ((1UL << bits) - 1) + 1;
    ULONG toff = ((1UL << bits) - 1);
    if (src->isFloat(comp))
      throw "histogram not supported for floating point types";
    if (bits > 16)
      throw "histogram not supported for more than 16bpp";
    if (tsiz > size)
      size = tsiz;
    if (toff > offset)
      offset = toff;
  }

  m_pulHist = new ULONG[size];
  memset(m_pulHist,0,sizeof(ULONG) * size);

  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG w = src->WidthOf(comp);
    ULONG h = src->HeightOf(comp);
    
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	Measure<const BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			    (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			    w,h,m_pulHist,offset);
      } else if (src->BitsOf(comp) <= 16) {
	Measure<const WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			    (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			    w,h,m_pulHist,offset);
      } else {
        throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	Measure<const UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			     (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			     w,h,m_pulHist,offset);
      } else if (src->BitsOf(comp) <= 16) {
	Measure<const UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			     (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			     w,h,m_pulHist,offset);
      } else {
        throw "unsupported data type";
      }
    }
  }

  /*
  ** If there is no target file name, a difference pixel ratio is expected to
  ** be measured.
  */

  if (!m_pcTargetFile) {
    UQUAD total = 0;
    UQUAD above = 0;
    
    for(comp = 0;comp < src->DepthOf();comp++) {
      total += UQUAD(src->WidthOf()) * UQUAD(src->HeightOf());
    }
    for(i = 0;i < size;i++) {
      if (i > offset + m_lThres || i < offset - m_lThres)
	above += m_pulHist[i];
    }

    in = double(above) / total;
    
  } else {
    FILE *out = NULL;
    
    if (strcmp(m_pcTargetFile,"-")) {
      out = fopen(m_pcTargetFile,"w");
      if (out == NULL) {
	ImageLayout::PostError("unable to open the histogram output file %s: %s\n",m_pcTargetFile,strerror(errno));
	return in; // code should never go here.
      }
    }
    
    if (out) {
      for(i = 0;i < size;i++) {
	if (m_pulHist[i]) {
	  fprintf(out,"%d\t%u\n",i - offset,m_pulHist[i]);
	}
      }
    } else {
      int cnt = 0;
      for(i = 0;i < size;i++) {
	if (m_pulHist[i]) {
	  printf("%+4d:\t%8u\t",i - offset,m_pulHist[i]);
	  if (++cnt > 3) {
	    printf("\n");
	    cnt = 0;
	  }
	}
      }
      if (cnt != 0)
	printf("\n");
    }
    
    if (out)
      fclose(out);
  }

  return in;
}
///
