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
** $Id: flipextend.cpp,v 1.1 2019/07/24 13:04:44 thor Exp $
**
** This class flips images over vertically and horizonally, doubling
** their size.
*/

/// Includes
#include "diff/flipextend.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// FlipExtend::ExtendHorizontal
template<typename T>
void FlipExtend::ExtendHorizontal(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
				  T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
				  ULONG w, ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    const T *orgrow = org;
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      *dstrow     = *orgrow;
      orgrow      = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)      ((UBYTE *)      (dstrow) + dbytesperpixel);
    }
    for(x = 0;x < w;x++) {
      orgrow      = (const T *)((const UBYTE *)(orgrow) - obytesperpixel);
      *dstrow     = *orgrow;
      dstrow      = (T *)      ((UBYTE *)      (dstrow) + dbytesperpixel);
    }
    org = (const T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)      ((UBYTE *)      (dst) + dbytesperrow);
  }
}
///

/// FlipExtend::ExtendVertical
template<typename T>
void FlipExtend::ExtendVertical(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
				T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
				ULONG w, ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    const T *orgrow = org;
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      *dstrow     = *orgrow;
      orgrow      = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)      ((UBYTE *)      (dstrow) + dbytesperpixel);
    }
    org = (const T *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)      ((UBYTE *)      (dst) + dbytesperrow);
  }
  for(y = 0;y < h;y++) {
    org = (const T *)((const UBYTE *)(org) - obytesperrow);
    const T *orgrow = org;
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      *dstrow     = *orgrow;
      orgrow      = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)      ((UBYTE *)      (dstrow) + dbytesperpixel);
    }
    dst = (T *)      ((UBYTE *)      (dst) + dbytesperrow);
  }
}
///

/// FlipExtend::~FlipExtend
FlipExtend::~FlipExtend(void)
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
}
///

/// FlipExtend::ApplyExtension
// Apply the horizontal or vertical extension by flipping
// the image over.
void FlipExtend::ApplyExtension(class ImageLayout *src)
{
  UWORD comp;

  CreateComponents(*src);
  m_ppucImage = new UBYTE *[src->DepthOf()];
  memset(m_ppucImage,0,sizeof(UBYTE *) * src->DepthOf());

  switch(m_Dir) {
  case FlipX:
    m_ulWidth  <<= 1;
    break;
  case FlipY:
    m_ulHeight <<= 1;
    break;
  }

  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    UBYTE *mem;
    ULONG dbpp;
    ULONG dbpr;
    //
    // Now install the parameters.
    dbpp = ImageLayout::SuggestBPP(src->BitsOf(comp),src->isFloat(comp));
    switch(m_Dir) {
    case FlipX:
      mem  = new UBYTE[(w << 1) * h * dbpp];
      m_pComponent[comp].m_ulWidth         = w << 1;
      m_pComponent[comp].m_ulHeight        = h;
      break;
    case FlipY:
      mem  = new UBYTE[w * (h << 1) * dbpp];
      m_pComponent[comp].m_ulWidth         = w;
      m_pComponent[comp].m_ulHeight        = h << 1;
      break;
    }
    m_ppucImage[comp]                    = mem;
    m_pComponent[comp].m_ucBits          = src->BitsOf(comp);
    m_pComponent[comp].m_bSigned         = src->isSigned(comp);
    m_pComponent[comp].m_bFloat          = src->isFloat(comp);
    m_pComponent[comp].m_ulBytesPerPixel = dbpp;
    m_pComponent[comp].m_ulBytesPerRow   = dbpr = m_pComponent[comp].m_ulWidth * dbpp;
    m_pComponent[comp].m_pPtr            = mem;
    //
    if (src->BitsOf(comp) <= 8) {
      if (src->isSigned(comp)) {
	switch(m_Dir) {
	case FlipX:
	  ExtendHorizontal<BYTE>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (BYTE *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	case FlipY:
	  ExtendVertical<BYTE>  ((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (BYTE *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	}
      } else {
	switch(m_Dir) {
	case FlipX:
	  ExtendHorizontal<UBYTE>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (UBYTE *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	case FlipY:
	  ExtendVertical<UBYTE>  ((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (UBYTE *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	}
      }
    } else if (src->BitsOf(comp) <= 16 && !src->isFloat(comp)) {
      if (src->isSigned(comp)) {
	switch(m_Dir) {
	case FlipX:
	  ExtendHorizontal<WORD>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (WORD *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	case FlipY:
	  ExtendVertical<WORD>  ((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (WORD *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	}
      } else {
	switch(m_Dir) {
	case FlipX:
	  ExtendHorizontal<UWORD>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (UWORD *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	case FlipY:
	  ExtendVertical<UWORD>  ((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (UWORD *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	}
      }
    } else if (src->BitsOf(comp) <= 32 && !src->isFloat(comp)) {
      if (src->isSigned(comp)) {
	switch(m_Dir) {
	case FlipX:
	  ExtendHorizontal<LONG>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (LONG *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	case FlipY:
	  ExtendVertical<LONG>  ((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (LONG *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	}
      } else {
	switch(m_Dir) {
	case FlipX:
	  ExtendHorizontal<ULONG>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (ULONG *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	case FlipY:
	  ExtendVertical<ULONG>  ((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (ULONG *)mem              ,dbpp,dbpr,
				  w,h);
	  break;
	}
      }
    } else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
      switch(m_Dir) {
      case FlipX:
	ExtendHorizontal<FLOAT>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (FLOAT *)mem              ,dbpp,dbpr,
			       w,h);
	break;
      case FlipY:
	ExtendVertical<FLOAT>  ((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (FLOAT *)mem              ,dbpp,dbpr,
			       w,h);
	break;
      }
    } else if (src->BitsOf(comp) <= 64 && src->isFloat(comp)) {
      switch(m_Dir) {
      case FlipX:
	ExtendHorizontal<DOUBLE>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (DOUBLE *)mem              ,dbpp,dbpr,
			       w,h);
	break;
      case FlipY:
	ExtendVertical<DOUBLE>  ((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (DOUBLE *)mem              ,dbpp,dbpr,
			       w,h);
	break;
      }
    } else {
      throw "unsupported data type";
    }
  }
}
///

/// FlipExtend::Measure
double FlipExtend::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  // First apply on this image.
  ApplyExtension(src);
  //
  // Create a destination image.
  assert(m_pDest == NULL);
  m_pDest = new class FlipExtend(m_Dir);
  //
  // Also map the destination image.
  m_pDest->ApplyExtension(dst);
  //
  // Replace the original images with the modified versions.
  src->Swap(*this);
  dst->Swap(*m_pDest);
  
  return in;
}
///
