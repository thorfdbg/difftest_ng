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
** $Id: downsampler.cpp,v 1.11 2025/12/01 09:06:43 thor Exp $
**
** This class downscales in the spatial domain
*/

/// Includes
#include "diff/downsampler.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// Downsampler::BoxFilter
template<typename S>
void Downsampler::BoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
			    S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
			    ULONG w,ULONG h,
			    S min,S max,
			    int sx,int sy)
{
  ULONG x,y;
  ULONG xo,yo;
  ULONG xm,ym;

  for(y = 0;y < h;y += sy) {
    S *dstrow = dest;
    const S *ssrow  = org;
    for(x = 0;x < w;x += sx) {
      const S *srcrow  = ssrow;
      double sum = 0.0;
      ULONG cnt = 0;
  
      xm = x + sx;
      if (xm > w)
	xm = w;
      
      ym = y + sy;
      if (ym > h)
	ym = h;

      for(yo = y;yo < ym;yo++) {
	const S *src = srcrow;
	for(xo = x;xo < xm;xo++) {
	  sum += *src;
	  src  = (const S *)(((const UBYTE *)src) + obytesperpixel);
	  cnt++;
	}
	srcrow = (const S *)(((const UBYTE *)srcrow) + obytesperrow);
      }
      
      if (cnt > 0)
	sum = sum / cnt;
      if (sum > max)
	sum = max;
      if (sum < min)
	sum = min;
      *dstrow = S(sum);
      dstrow  = (S *)(((UBYTE *)dstrow) + tbytesperpixel);
      ssrow   = (const S *)(((const UBYTE *)ssrow ) + obytesperpixel * sx);
    }
    dest  = (S *)(((UBYTE *)dest) + tbytesperrow);
    org   = (const S *)(((const UBYTE *)org)  + obytesperrow * sy);
  }
}
///

/// Downsampler::RegionalBoxFilter
template<typename S>
void Downsampler::RegionalBoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
				    S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
				    ULONG w,ULONG h,LONG x1,LONG y1,LONG x2,LONG y2,
				    S min,S max)
{
  ULONG x,y;
  ULONG xo,yo;
  ULONG xm,ym;

  for(y = 0;y < h;) {
    S *dstrow = dest;
    const S *ssrow  = org;
    for(x = 0;x < w;) {
      const S *srcrow  = ssrow;
      double sum = 0.0;
      ULONG cnt = 0;

      if (LONG(x) < x1) {
	xm = x + 2;
	if (LONG(xm) > x2)
	  xm = x2;
      } else if (LONG(x) >= x2) {
	xm = x + 2;
	if (xm > w)
	  xm = w;
      } else {
	xm = x + 1;
      }
      
      if (LONG(y) < y1) {
	ym = y + 2;
	if (LONG(ym) > y2)
	  ym = y2;
      } else if (LONG(y) >= y2) {
	ym = y + 2;
	if (ym > h)
	  ym = h;
      } else {
	ym = y + 1;
      }

      for(yo = y;yo < ym;yo++) {
	const S *src = srcrow;
	for(xo = x;xo < xm;xo++) {
	  sum += *src;
	  src  = (const S *)(((const UBYTE *)src) + obytesperpixel);
	  cnt++;
	}
	srcrow = (const S *)(((const UBYTE *)srcrow) + obytesperrow);
      }
      
      if (cnt > 0)
	sum = sum / cnt;
      if (sum > max)
	sum = max;
      if (sum < min)
	sum = min;
      *dstrow = S(sum);
      dstrow  = (S *)(((UBYTE *)dstrow) + tbytesperpixel);
      ssrow   = (const S *)(((const UBYTE *)ssrow ) + obytesperpixel * (xm - x));
      x       = xm;
    }
    dest  = (S *)(((UBYTE *)dest) + tbytesperrow);
    org   = (const S *)(((const UBYTE *)org)  + obytesperrow * (ym - y));
    y     = ym;
  }
}
///

/// Downsampler::~Downsampler
Downsampler::~Downsampler(void)
{
  ReleaseComponents(m_ppucSource);
  ReleaseComponents(m_ppucDestination);
}
///

/// Downsampler::ReleaseComponents
void Downsampler::ReleaseComponents(UBYTE **p)
{
  int i;

  if (p) {
    for(i = 0;i < m_usAllocated;i++) {
      if (p[i])
        delete[] p[i];
    }
    delete[] p;
    p = NULL;
  }
}
///


/// Downsampler::Downsample
void Downsampler::Downsample(UBYTE **&data,class ImageLayout *src)
{
  UWORD i;
  // Delete the old image components. Does not release the
  // memory we hold.
  delete[] m_pComponent;
  m_pComponent  = NULL;
  ReleaseComponents(data);
  //
  if (m_bChromaOnly) {
    m_ulWidth     = src->WidthOf();
    m_ulHeight    = src->HeightOf();
  } else { 
    m_ulWidth     = DownsampledWidth(src->WidthOf());
    m_ulHeight    = DownsampledHeight(src->HeightOf());
  }
  //
  m_usDepth     = src->DepthOf();
  m_pComponent  = new struct ComponentLayout[m_usDepth];
  //
  // Initialize component dimensions.
  for(i = 0; i < m_usDepth; i++) {
    if (m_bChromaOnly == false || i > 0) {
      m_pComponent[i].m_ulWidth  = DownsampledWidth(src->WidthOf(i));
      m_pComponent[i].m_ulHeight = DownsampledHeight(src->HeightOf(i));
    } else {
      m_pComponent[i].m_ulWidth  = src->WidthOf(i);
      m_pComponent[i].m_ulHeight = src->HeightOf(i);
    }
    m_pComponent[i].m_ucBits   = src->BitsOf(i);
    m_pComponent[i].m_bSigned  = src->isSigned(i);
    m_pComponent[i].m_bFloat   = src->isFloat(i);
    if (m_bChromaOnly && i > 0) { 
      m_pComponent[i].m_ucSubX   = src->SubXOf(i) * m_ucScaleX;
      m_pComponent[i].m_ucSubY   = src->SubYOf(i) * m_ucScaleY;
    } else {
      m_pComponent[i].m_ucSubX   = src->SubXOf(i);
      m_pComponent[i].m_ucSubY   = src->SubYOf(i);
    }
  }
  //
  data = new UBYTE *[m_usDepth];
  memset(data,0,sizeof(UBYTE *) * m_usDepth);
  m_usAllocated = m_usDepth;
  //
  for(i = 0;i < m_usDepth;i++) {
    UBYTE bps = ImageLayout::SuggestBPP(m_pComponent[i].m_ucBits,m_pComponent[i].m_bFloat);
    //
    data[i]                           = new UBYTE[m_pComponent[i].m_ulWidth * m_pComponent[i].m_ulHeight * bps];
    m_pComponent[i].m_ulBytesPerPixel = bps;
    m_pComponent[i].m_ulBytesPerRow   = bps * m_pComponent[i].m_ulWidth;
    m_pComponent[i].m_pPtr            = data[i];
  }
  //
  for(i = 0;i < m_usDepth;i++) {
    UBYTE sx,sy;
    //
    if (m_bChromaOnly == false || i > 0) { 
      sx = m_ucScaleX;
      sy = m_ucScaleY;
    } else {
      sx = 1;
      sy = 1;
    }
    //
    if (m_bRegional) {
      if (isSigned(i)) {
	if (BitsOf(i) <= 8) {
	  RegionalBoxFilter<BYTE>((BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				  BYTE(-1UL << (BitsOf(i) - 1)),BYTE((1UL << (BitsOf(i) - 1)) - 1));
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  RegionalBoxFilter<WORD>((WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				  WORD(-1UL << (BitsOf(i) - 1)),WORD((1UL << (BitsOf(i) - 1)) - 1));
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  RegionalBoxFilter<LONG>((LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				  LONG(-1UL << (BitsOf(i) - 1)),LONG((1UL << (BitsOf(i) - 1)) - 1));
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  RegionalBoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				   (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				   src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				   -HUGE_VAL,HUGE_VAL);
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  RegionalBoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				    (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				    src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				    -HUGE_VAL,HUGE_VAL);
	} else {
	  throw "unsupported data type";
	}
      } else {
	if (BitsOf(i) <= 8) {
	  RegionalBoxFilter<UBYTE>((UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				   (UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				   src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				   0,UBYTE((1UL << BitsOf(i)) - 1));
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  RegionalBoxFilter<UWORD>((UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				   (UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				   src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				   0,UWORD((1UL << BitsOf(i)) - 1));
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  RegionalBoxFilter<ULONG>((ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				   (ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				   src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				   0,LONG((1UL << BitsOf(i)) - 1));
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  RegionalBoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				   (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				   src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				   0.0,HUGE_VAL);
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  RegionalBoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				    (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				    src->WidthOf(i),src->HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
				    0.0,HUGE_VAL);
	} else {
	  throw "unsupported data type";
	}
      }
    } else { // of is regional, else part
      if (isSigned(i)) {
	if (BitsOf(i) <= 8) {
	  BoxFilter<BYTE>((BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			  (BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			  src->WidthOf(i),src->HeightOf(i),
			  BYTE(-1UL << (BitsOf(i) - 1)),BYTE((1UL << (BitsOf(i) - 1)) - 1),
			  sx,sy);
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  BoxFilter<WORD>((WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			  (WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			  src->WidthOf(i),src->HeightOf(i),
			  WORD(-1UL << (BitsOf(i) - 1)),WORD((1UL << (BitsOf(i) - 1)) - 1),
			  sx,sy);
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  BoxFilter<LONG>((LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			  (LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			  src->WidthOf(i),src->HeightOf(i),
			  LONG(-1UL << (BitsOf(i) - 1)),LONG((1UL << (BitsOf(i) - 1)) - 1),
			  sx,sy);
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  BoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			   (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			 src->WidthOf(i),src->HeightOf(i),
			   -HUGE_VAL,HUGE_VAL,
			   sx,sy);
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  BoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			    src->WidthOf(i),src->HeightOf(i),
			    -HUGE_VAL,HUGE_VAL,
			    sx,sy);
	} else {
	  throw "unsupported data type";
	}
      } else {
	if (BitsOf(i) <= 8) {
	  BoxFilter<UBYTE>((UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			   (UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			   src->WidthOf(i),src->HeightOf(i),
			   0,UBYTE((1UL << BitsOf(i)) - 1),
			   sx,sy);
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  BoxFilter<UWORD>((UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			   (UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			   src->WidthOf(i),src->HeightOf(i),
			   0,UWORD((1UL << BitsOf(i)) - 1),
			   sx,sy);
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  BoxFilter<ULONG>((ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			   (ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			   src->WidthOf(i),src->HeightOf(i),
			   0,LONG((1UL << BitsOf(i)) - 1),
			   sx,sy);
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  BoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			   (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			   src->WidthOf(i),src->HeightOf(i),
			   0.0,HUGE_VAL,
			   sx,sy);
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  BoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			    src->WidthOf(i),src->HeightOf(i),
			    0.0,HUGE_VAL,
			    sx,sy);
	} else {
	  throw "unsupported data type";
	}
      }
    }
  }
  //
  Swap(*src);
}
///

/// Downsampler::Measure
double Downsampler::Measure(class ImageLayout *src,class ImageLayout *dest,double in)
{
  Downsample(m_ppucSource,src);
  Downsample(m_ppucDestination,dest);

  return in;
}
///

  
