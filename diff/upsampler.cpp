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
** $Id: upsampler.cpp,v 1.13 2025/10/30 12:01:09 thor Exp $
**
** This class downscales in the spatial domain
*/

/// Includes
#include "diff/upsampler.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// Upsampler::BilinearFilter
template<typename S>
void Upsampler::BilinearFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
			       S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
			       ULONG w,ULONG h,
			       S min,S max,
			       int sx,int sy)
{
  ULONG x,y;
  ULONG sw  = w / sx;
  ULONG sh  = h / sy;
  double fx = (sx & 1)?0.0:0.5;
  double fy = (sy & 1)?0.0:0.5;
  int    cx = sx >> 1;
  int    cy = sy >> 1;

  if (m_FilterType == Upsampler::Cosited) {
    fx = fy = 0.0;
    cx = cy = 0;
  }
  
  for(y = 0;y < h;y++) {
    S *dstrow = dest;
    for(x = 0;x < w;x++) {
      const S *srclt,*srcrt,*srclb,*srcrb;
      LONG xo = (x + sx - cx) / sx - 1;
      LONG yo = (y + sy - cy) / sy - 1; // left and top sample location.
      double wx = (x - double(xo * sx) - cx + fx) / double(sx);
      double wy = (y - double(yo * sy) - cy + fy) / double(sy); // weight for the right pixel
      LONG xl   = (xo >= 0)?(xo):(0);
      LONG xr   = (xo + 1 < LONG(sw))?(xo + 1):(xo);
      LONG yt   = (yo >= 0)?(yo):(0);
      LONG yb   = (yo + 1 < LONG(sh))?(yo + 1):(yo);
      assert(wx >= 0.0 && wx < 1.0);
      assert(wy >= 0.0 && wy < 1.0);
      srclt  = (const S *)(((const UBYTE *)org) + xl * obytesperpixel + yt * obytesperrow);
      srcrt  = (const S *)(((const UBYTE *)org) + xr * obytesperpixel + yt * obytesperrow);
      srclb  = (const S *)(((const UBYTE *)org) + xl * obytesperpixel + yb * obytesperrow);
      srcrb  = (const S *)(((const UBYTE *)org) + xr * obytesperpixel + yb * obytesperrow);
      //
      double v = (1.0-wx) * (1.0-wy) * *srclt + wx * (1.0-wy) * *srcrt + (1.0-wx) * wy * *srclb + wx * wy * *srcrb;
      
      if (v > max)
	v = max;
      if (v < min)
	v = min;
      *dstrow = S(v);
      dstrow  = (S *)(((UBYTE *)dstrow) + tbytesperpixel);
    }
    dest  = (S *)(((UBYTE *)dest) + tbytesperrow);
  }
}
///

/// Upsampler::RegionalBilinearFilter
template<typename S>
void Upsampler::RegionalBilinearFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
				       S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
				       ULONG w,ULONG h,LONG x1,LONG y1,LONG x2,LONG y2,
				       S min,S max)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    S *dstrow = dest;
    const S *orgrow  = org; // First line to interpolate from
    const S *orgrow2 = org; // Second line to interpolate from
    if (((LONG(y) < y1 || LONG(y) >= y2)) && (y & 1) && y + 1 < h)
      orgrow2 = (const S *)(((const UBYTE *)orgrow2) + obytesperrow);
    for(x = 0;x < w;x++) {
      double v = (double(*orgrow) + double(*orgrow2)) * 0.5;
      if ((LONG(x) < x1 || LONG(x) >= x2) && (x & 1) && x + 1 < w) {
	const S *orgrowx  = (const S *)(((const UBYTE *)orgrow)  + obytesperpixel);
	const S *orgrow2x = (const S *)(((const UBYTE *)orgrow2) + obytesperpixel);
	double v2 = (double(*orgrowx) + double(*orgrow2x)) * 0.5;
	v = (v + v2) * 0.5;
      }
      if (v > max)
	v = max;
      if (v < min)
	v = min;
      *dstrow = S(v);
      dstrow  = (S *)(((UBYTE *)dstrow) + tbytesperpixel);
      if ((LONG(x) >= x1 && LONG(x) < x2) || (x & 1)) {
	orgrow  = (const S *)(((const UBYTE *)orgrow ) + obytesperpixel);
	orgrow2 = (const S *)(((const UBYTE *)orgrow2) + obytesperpixel);
      }
    }
    if ((LONG(y) >= y1 && LONG(y) < y2) || (y & 1))
      org = (const S *)(((const UBYTE *)org) + obytesperrow);
    dest  = (S *)(((UBYTE *)dest) + tbytesperrow);
  }
}
///

/// Upsampler::BoxFilter
template<typename S>
void Upsampler::BoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
			  S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
			  ULONG w,ULONG h,
			  int sx,int sy)
{
  ULONG x,y;
  
  for(y = 0;y < h;y++) {
    S *dstrow = dest;
    for(x = 0;x < w;x++) {
      const S *srclt;
      LONG xo = x / sx;
      LONG yo = y / sy;
      srclt  = (const S *)(((const UBYTE *)org) + xo * obytesperpixel + yo * obytesperrow);
      //
      *dstrow = S(*srclt);
      dstrow  = (S *)(((UBYTE *)dstrow) + tbytesperpixel);
    }
    dest  = (S *)(((UBYTE *)dest) + tbytesperrow);
  }
}
///

/// Upsampler::RegionalBoxFilter
template<typename S>
void Upsampler::RegionalBoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
				  S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
				  ULONG w,ULONG h,LONG x1,LONG y1,LONG x2,LONG y2)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    S *dstrow = dest;
    const S *orgrow = org;
    for(x = 0;x < w;x++) {
      *dstrow = S(*orgrow);
      if ((LONG(x) >= x1 && LONG(x) < x2) || (x & 1))
	orgrow = (const S *)(((const UBYTE *)orgrow) + obytesperpixel);
      dstrow  = (S *)(((UBYTE *)dstrow) + tbytesperpixel);
    }
    if ((LONG(y) >= y1 && LONG(y) < y2) || (y & 1))
      org = (const S *)(((const UBYTE *)org) + obytesperrow);
    dest  = (S *)(((UBYTE *)dest) + tbytesperrow);
  }
}
///



/// Upsampler::~Upsampler
Upsampler::~Upsampler(void)
{
  ReleaseComponents(m_ppucSource);
  ReleaseComponents(m_ppucDestination);
}
///

/// Upsampler::ReleaseComponents
void Upsampler::ReleaseComponents(UBYTE **p)
{
  int i;

  if (p) {
    for(i = 0;i < m_usDepth;i++) {
      if (p[i])
        delete[] p[i];
    }
    delete[] p;
    p = NULL;
  }
}
///


/// Upsampler::Upsample
void Upsampler::Upsample(UBYTE **&data,class ImageLayout *src)
{
  UWORD i;
  // Delete the old image components. Does not release the
  // memory we hold.
  delete[] m_pComponent;
  m_pComponent  = NULL;
  ReleaseComponents(data);
  //
  if (m_bChromaOnly || m_bAutomatic) {
    m_ulWidth     = src->WidthOf();
    m_ulHeight    = src->HeightOf();
  } else { 
    m_ulWidth     = UpsampledWidth(src->WidthOf());
    m_ulHeight    = UpsampledHeight(src->HeightOf());
  }
  //
  m_usDepth     = src->DepthOf();
  m_pComponent  = new struct ComponentLayout[m_usDepth];
  //
  // Initialize component dimensions.
  for(i = 0; i < m_usDepth; i++) {
    if (m_bAutomatic) {
      m_pComponent[i].m_ulWidth  = src->WidthOf();
      m_pComponent[i].m_ulHeight = src->HeightOf();
    } else {
      if (m_bChromaOnly == false || i > 0) { 
	m_pComponent[i].m_ulWidth  = UpsampledWidth(src->WidthOf(i));
	m_pComponent[i].m_ulHeight = UpsampledHeight(src->HeightOf(i));
      } else {
	m_pComponent[i].m_ulWidth  = src->WidthOf(i);
	m_pComponent[i].m_ulHeight = src->HeightOf(i);
      }
    }
    m_pComponent[i].m_ucBits   = src->BitsOf(i);
    m_pComponent[i].m_bSigned  = src->isSigned(i);
    m_pComponent[i].m_bFloat   = src->isFloat(i);
    if (m_bAutomatic || (m_bChromaOnly && i > 0) || m_bRegional) { 
      m_pComponent[i].m_ucSubX   = 1;
      m_pComponent[i].m_ucSubY   = 1;
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
    if (m_bAutomatic) {
      sx = src->SubXOf(i);
      sy = src->SubYOf(i);
    } else if (m_bChromaOnly == false || i > 0) { 
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
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<BYTE>((BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				    (BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				    WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<BYTE>((BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					 (BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					 WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					 BYTE(-1UL << (BitsOf(i) - 1)),BYTE((1UL << (BitsOf(i) - 1)) - 1));
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<WORD>((WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				    (WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				    WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<WORD>((WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					 (WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					 WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					 WORD(-1UL << (BitsOf(i) - 1)),WORD((1UL << (BitsOf(i) - 1)) - 1));
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<LONG>((LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				    (LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				    WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<LONG>((LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					 (LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					 WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					 LONG(-1UL << (BitsOf(i) - 1)),LONG((1UL << (BitsOf(i) - 1)) - 1));
	  }
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				     (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				     WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					  (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					  WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					  -HUGE_VAL,HUGE_VAL);
	  }
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				      (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				      WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					   (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					   WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					   -HUGE_VAL,HUGE_VAL);
	  }
	} else {
	  throw "unsupported data type";
	}
      } else {
	if (BitsOf(i) <= 8) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<UBYTE>((UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				     (UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				     WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<UBYTE>((UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					  (UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					  WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					  0,UBYTE((1UL << BitsOf(i)) - 1));
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<UWORD>((UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				     (UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				     WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<UWORD>((UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					  (UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					  WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					  0,UWORD((1UL << BitsOf(i)) - 1));
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<ULONG>((ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				     (ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				     WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<ULONG>((ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					  (ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					  WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					  0,ULONG((1UL << BitsOf(i)) - 1));
	  }
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				     (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				     WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					  (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					  WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					  0.0,HUGE_VAL);
	  }
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  if (m_FilterType == Boxed) {
	    RegionalBoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				      (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				      WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2);
	  } else {
	    RegionalBilinearFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
					   (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
					   WidthOf(i),HeightOf(i),m_lX1,m_lY1,m_lX2,m_lY2,
					   0.0,HUGE_VAL);
	  }
	} else {
	  throw "unsupported data type";
	}
      }
    } else { /* of regional filter */
      if (isSigned(i)) {
	if (BitsOf(i) <= 8) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<BYTE>((BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    (BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			    WidthOf(i),HeightOf(i),
			    sx,sy);
	  } else {
	    BilinearFilter<BYTE>((BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				 (BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				 WidthOf(i),HeightOf(i),
				 BYTE(-1UL << (BitsOf(i) - 1)),BYTE((1UL << (BitsOf(i) - 1)) - 1),
				 sx,sy);
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<WORD>((WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    (WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			    WidthOf(i),HeightOf(i),
			    sx,sy);
	  } else {
	    BilinearFilter<WORD>((WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				 (WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				 WidthOf(i),HeightOf(i),
				 WORD(-1UL << (BitsOf(i) - 1)),WORD((1UL << (BitsOf(i) - 1)) - 1),
				 sx,sy);
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<LONG>((LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    (LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			    WidthOf(i),HeightOf(i),
			    sx,sy);
	  } else {
	    BilinearFilter<LONG>((LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				 (LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				 WidthOf(i),HeightOf(i),
				 LONG(-1UL << (BitsOf(i) - 1)),LONG((1UL << (BitsOf(i) - 1)) - 1),
				 sx,sy);
	  }
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			     (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			     WidthOf(i),HeightOf(i),
			     sx,sy);
	  } else {
	    BilinearFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  WidthOf(i),HeightOf(i),
				  -HUGE_VAL,HUGE_VAL,
				  sx,sy);
	  }
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			      WidthOf(i),HeightOf(i),
			      sx,sy);
	  } else {
	    BilinearFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				   (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				   WidthOf(i),HeightOf(i),
				   -HUGE_VAL,HUGE_VAL,
				   sx,sy);
	  }
	} else {
	  throw "unsupported data type";
	}
      } else {
	if (BitsOf(i) <= 8) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<UBYTE>((UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			     (UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			     WidthOf(i),HeightOf(i),
			     sx,sy);
	  } else {
	    BilinearFilter<UBYTE>((UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  WidthOf(i),HeightOf(i),
				  0,UBYTE((1UL << BitsOf(i)) - 1),
				  sx,sy);
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 16) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<UWORD>((UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			     (UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			     WidthOf(i),HeightOf(i),
			     sx,sy);
	  } else {
	    BilinearFilter<UWORD>((UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  WidthOf(i),HeightOf(i),
				  0,UWORD((1UL << BitsOf(i)) - 1),
				  sx,sy);
	  }
	} else if (!isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<ULONG>((ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			     (ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			     WidthOf(i),HeightOf(i),
			     sx,sy);
	  } else {
	    BilinearFilter<ULONG>((ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  WidthOf(i),HeightOf(i),
				  0,ULONG((1UL << BitsOf(i)) - 1),
				  sx,sy);
	  }
	} else if (isFloat(i) && BitsOf(i) <= 32) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			     (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			     WidthOf(i),HeightOf(i),
			     sx,sy);
	  } else {
	    BilinearFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				  (FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				  WidthOf(i),HeightOf(i),
				  0.0,HUGE_VAL,
				  sx,sy);
	  }
	} else if (isFloat(i) && BitsOf(i) == 64) {
	  if (m_FilterType == Boxed) {
	    BoxFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			      WidthOf(i),HeightOf(i),
			      sx,sy);
	  } else {
	    BilinearFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				   (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				   WidthOf(i),HeightOf(i),
				   0.0,HUGE_VAL,
				   sx,sy);
	  }
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

/// Upsampler::Measure
double Upsampler::Measure(class ImageLayout *src,class ImageLayout *dest,double in)
{
  Upsample(m_ppucSource,src);
  Upsample(m_ppucDestination,dest);

  return in;
}
///

  
