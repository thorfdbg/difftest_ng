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
** $Id: diffimg.cpp,v 1.20 2020/09/15 10:20:32 thor Exp $
**
** This class saves the difference image as a normalized 8bpp image
** with the same number of components as the original.
*/

/// Includes
#include "diff/diffimg.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///


/// DiffImg::Adjust
template<typename T,typename D>
void DiffImg::Adjust(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		     T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		     APTR o,ULONG tbytesperpixel,ULONG tbytesperrow,D dmin,D dmax,
		     double &scale,double &shift,ULONG w,ULONG h)
{
  double min = +HUGE_VAL;
  double max = -HUGE_VAL;
  ULONG x,y;
  D *trg     = (D *)(o);

  for(y = 0;y < h;y++) {
    T *orgrow = org;
    T *dstrow = dst;
    D *trgrow = trg;
    for(x = 0;x < w;x++) {
      double diff = *orgrow - *dstrow;
      if (trgrow) {
	diff      = (diff - shift) * scale;
	if (diff < dmin) {
	  *trgrow = 0;
	} else if (diff > dmax) {
	  *trgrow = dmax;
	} else {
	  *trgrow = D(diff);
	}
	trgrow      = trgrow + tbytesperpixel;
      } else {
	if (diff < min)
	  min = diff;
	if (diff > max)
	  max = diff;
      }
      //
      orgrow      = (T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((const UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((const UBYTE *)(dst) + dbytesperrow);
    if (trg) {
      trg = trg + tbytesperrow;
    }
  }

  if (trg == NULL) {
    // Compute scale and shift
    shift = min;
    if (max > min) {
      scale = dmax / (max - min);
    } else {
      scale = 1.0;
    }
  }
}
///

/// DiffImg::~DiffImg
DiffImg::~DiffImg(void)
{
  int i;

  if (m_ppucImage) {
    for(i = 0;i < m_usDepth;i++) {
      if (m_ppucImage[i])
	delete[] m_ppucImage[i];
    }
    delete[] m_ppucImage;
  }
}
///

/// DiffImg::Measure
double DiffImg::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  UWORD comp;
  
  src->TestIfCompatible(dst);

  CreateComponents(*src);
  m_ppucImage = new UBYTE *[src->DepthOf()];
  memset(m_ppucImage,0,sizeof(UBYTE *) * src->DepthOf());



  for(comp = 0;comp < src->DepthOf();comp++) {
    double scale = m_dFactor;
    double shift = 0.0;
    ULONG  w     = src->WidthOf(comp);
    ULONG  h     = src->HeightOf(comp);
    UBYTE  bytes = (m_bScale)?1:ImageLayout::SuggestBPP(src->BitsOf(comp),src->isFloat(comp));
    UBYTE *mem   = new UBYTE[w * h * bytes];
    //
    // Shift is the required shift to generate unsigned data from a
    // differential signal, before scaling to the target bitdepth.
    if (src->isFloat(comp)) {
      shift = -0.5 / m_dFactor;
    } else {
      shift = (-((1 << src->BitsOf(comp)) >> 1)) / m_dFactor;
    }
    //
    m_ppucImage[comp]                    = mem;
    m_pComponent[comp].m_ucBits          = m_bScale?8:src->BitsOf(comp);
    m_pComponent[comp].m_bSigned         = false;
    m_pComponent[comp].m_bFloat          = m_bScale?false:src->isFloat(comp);
    m_pComponent[comp].m_ulWidth         = w;
    m_pComponent[comp].m_ulHeight        = h;
    m_pComponent[comp].m_ulBytesPerPixel = bytes;
    m_pComponent[comp].m_ulBytesPerRow   = w * bytes;
    m_pComponent[comp].m_pPtr            = mem;
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	if (m_bScale) {
	  Adjust<const BYTE,UBYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const BYTE,UBYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const BYTE,UBYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	}
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	if (m_bScale) {
	  Adjust<const WORD,UBYTE>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const WORD,UBYTE>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const WORD,UWORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   mem,1,w,0,MAX_UWORD,scale,shift,w,h);
	}
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	if (m_bScale) {
	  Adjust<const LONG,UBYTE>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const LONG,UBYTE>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const LONG,ULONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				   (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				   mem,1,w,0,MAX_ULONG,scale,shift,w,h);
	}
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	if (m_bScale) {
	  Adjust<const FLOAT,UBYTE>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const FLOAT,UBYTE>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const FLOAT,FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,-HUGE_VAL,HUGE_VAL,scale,shift,w,h);
	}
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	if (m_bScale) {
	  Adjust<const DOUBLE,UBYTE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const DOUBLE,UBYTE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const DOUBLE,DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				      (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				      mem,1,w,-HUGE_VAL,HUGE_VAL,scale,shift,w,h);
	}
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	if (m_bScale) {
	  Adjust<const UBYTE,UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const UBYTE,UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const UBYTE,UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	}
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	if (m_bScale) {
	  Adjust<const UWORD,UBYTE>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const UWORD,UBYTE>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const UWORD,UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_UWORD,scale,shift,w,h);
	}
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	if (m_bScale) {
	  Adjust<const ULONG,UBYTE>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const ULONG,UBYTE>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const ULONG,ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_ULONG,scale,shift,w,h);
	}
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	if (m_bScale) {
	  Adjust<const FLOAT,UBYTE>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const FLOAT,UBYTE>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const FLOAT,FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				    mem,1,w,-HUGE_VAL,HUGE_VAL,scale,shift,w,h);
	}
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	if (m_bScale) {
	  Adjust<const DOUBLE,UBYTE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     NULL,0,0,0,MAX_UBYTE,scale,shift,w,h);
	  Adjust<const DOUBLE,UBYTE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				     mem,1,w,0,MAX_UBYTE,scale,shift,w,h);
	} else {
	  Adjust<const DOUBLE,DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				      (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
				      mem,1,w,-HUGE_VAL,HUGE_VAL,scale,shift,w,h);
	}
      } else {
	throw "unsupported data type";
      }
    }
  }

  SaveImage(m_pTargetFile,m_TargetSpecs);

  return in;
}
///
