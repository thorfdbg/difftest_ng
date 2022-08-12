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
** $Id: butterfly.cpp,v 1.1 2022/08/12 06:55:23 thor Exp $
**
** This class combines two images in the butterfly style and saves
** the result to a file.
*/

/// Includes
#include "diff/butterfly.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///


/// Butterfly::Adjust
template<typename T>
void Butterfly::Merge(const T *org,ULONG obytesperpixel,ULONG obytesperrow,
		      const T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		      T *trg,ULONG tbytesperpixel,ULONG tbytesperrow,
		      ULONG w,ULONG h)
{
  ULONG x,xm,y;

  for(y = 0;y < h;y++) {
    const T *orgrow = org;
    const T *dstrow = dst;
    T       *trgrow = trg;
    for(x = 0,xm = w;x < w;x++,xm--) {
      if ((x * h > y * w) ^ (xm * h > y * w)) {
	*trgrow = *dstrow;
      } else {
	*trgrow = *orgrow;
      }
      //
      orgrow      = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (const T *)((const UBYTE *)(dstrow) + dbytesperpixel);
      trgrow      = (T *)((UBYTE       *)(trgrow) + tbytesperpixel);
    }
    org = (const T *)((const UBYTE *)(org) + obytesperrow);
    dst = (const T *)((const UBYTE *)(dst) + dbytesperrow);
    trg = (T *)((UBYTE *      )(trg) + tbytesperrow);
  }
}
///

/// Butterfly::~Butterfly
Butterfly::~Butterfly(void)
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

/// Butterfly::Measure
double Butterfly::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  UWORD comp;
  
  src->TestIfCompatible(dst);

  CreateComponents(*src);
  m_ppucImage = new UBYTE *[src->DepthOf()];
  memset(m_ppucImage,0,sizeof(UBYTE *) * src->DepthOf());

  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w     = src->WidthOf(comp);
    ULONG  h     = src->HeightOf(comp);
    UBYTE  bytes = ImageLayout::SuggestBPP(src->BitsOf(comp),src->isFloat(comp));
    UBYTE *mem   = new UBYTE[w * h * bytes];
    //
    m_ppucImage[comp]                    = mem;
    m_pComponent[comp].m_ucBits          = src->BitsOf(comp);
    m_pComponent[comp].m_bSigned         = false;
    m_pComponent[comp].m_bFloat          = src->isFloat(comp);
    m_pComponent[comp].m_ulWidth         = w;
    m_pComponent[comp].m_ulHeight        = h;
    m_pComponent[comp].m_ulBytesPerPixel = bytes;
    m_pComponent[comp].m_ulBytesPerRow   = w * bytes;
    m_pComponent[comp].m_pPtr            = mem;
    //
    if (src->isSigned(comp)) {
      if (src->BitsOf(comp) <= 8) {
	Merge<BYTE>((const BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		    (const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		    (BYTE *     )mem,sizeof(BYTE),sizeof(BYTE) * w,w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	Merge<WORD>((const WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		    (const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		    (WORD *     )mem,sizeof(WORD),sizeof(WORD) * w,w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	Merge<LONG>((const LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		    (const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		    (LONG *     )mem,sizeof(LONG),sizeof(LONG) * w,w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	Merge<FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		    (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		    (FLOAT *     )mem,sizeof(FLOAT),sizeof(FLOAT) * w,w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	Merge<DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		      (DOUBLE *     )mem,sizeof(DOUBLE),sizeof(DOUBLE) * w,w,h);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (src->BitsOf(comp) <= 8) {
	Merge<UBYTE>((const UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		    (const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		    (UBYTE *     )mem,sizeof(UBYTE),sizeof(UBYTE) * w,w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	Merge<UWORD>((const UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		     (const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		     (UWORD *     )mem,sizeof(UWORD),sizeof(UWORD) * w,w,h);
      } else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	Merge<ULONG>((const ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		     (const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		     (ULONG *     )mem,sizeof(ULONG),sizeof(ULONG) * w,w,h);
      } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	Merge<FLOAT>((const FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		     (const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		     (FLOAT *     )mem,sizeof(FLOAT),sizeof(FLOAT) * w,w,h);
      } else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	Merge<DOUBLE>((const DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		     (const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		     (DOUBLE *     )mem,sizeof(DOUBLE),sizeof(DOUBLE) * w,w,h);
      } else {
	throw "unsupported data type";
      }
    }
  }

  SaveImage(m_pTargetFile,m_TargetSpecs);

  return in;
}
///
