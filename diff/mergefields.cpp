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
** $Id: mergefields.cpp,v 1.1 2023/07/11 11:54:11 thor Exp $
**
** This class merges source and destination together in one single 
** frame, de-interlacing them to a progressive image.
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/mergefields.hpp"
#include "std/string.hpp"
///

/// MergeFields::~MergeFields
MergeFields::~MergeFields(void)
{

  if (m_ppucMemory) {
    UBYTE **mem = m_ppucMemory;
    UWORD  i    = m_usDepth;
    while(i) {
      delete *mem;
      mem++;
      i--;
    }
  }
}
///

/// MergeFields::InterleaveData
template<typename T>
void MergeFields::InterleaveData(T *dst,ULONG bytesperpixel,ULONG bytesperrow,
				 ULONG width,ULONG height,
				 const void *even,ULONG srcbpp,ULONG srcbpr,
				 const void *odd ,ULONG dstbpp,ULONG dstbpr)
{
  ULONG h = height;
  const T *evenrow = (const T*)(even);
  const T *oddrow  = (const T*)(odd);
  T *dstrow        = dst;

  while(h) {
    const T *even = evenrow;
    const T *odd  = oddrow;
    ULONG  w      = width;

    while(w) {
      *dst  = *even;
      dst   = (T *)(((UBYTE *)dst) + bytesperpixel);
      even  = (const T *)(((const UBYTE *)even) + srcbpp);
      --w;
    }
    dstrow  = (T *)(((UBYTE *)dstrow)  + bytesperrow);
    evenrow = (const T *)(((const UBYTE *)evenrow) + srcbpr);
       
    if (--h == 0)
      break;
    
    w   = width;
    dst = dstrow;
    while(w) {
      *dst  = *odd;
      dst   = (T *)(((UBYTE *)dst) + bytesperpixel);
      odd   = (const T *)(((const UBYTE *)odd) + dstbpp);
      --w;
    }
    dstrow  = (T *)(((UBYTE *)dstrow) + bytesperrow);
    oddrow  = (const T *)(((const UBYTE *)oddrow)  + dstbpr);    

    --h;
  }
}
///

/// MergeFields::Measure
double MergeFields::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  UWORD i;
  
  assert(m_usDepth == 0 && m_ppucMemory == NULL);

  src->TestIfCompatible(dst);

  m_usDepth    = src->DepthOf();
  m_ppucMemory = new UBYTE*[m_usDepth];
  memset(m_ppucMemory,0,sizeof(UBYTE *) * m_usDepth);

  CreateComponents(src->WidthOf(),src->HeightOf() + dst->HeightOf(),m_usDepth);
  
  for(i = 0;i < m_usDepth;i++) {
    m_pComponent[i].m_ucBits          = src->BitsOf(i);
    m_pComponent[i].m_bSigned         = src->isSigned(i);
    m_pComponent[i].m_bFloat          = src->isFloat(i);
    m_pComponent[i].m_ucSubX          = src->SubXOf(i);
    m_pComponent[i].m_ucSubY          = src->SubYOf(i);
    m_pComponent[i].m_ulWidth         = src->WidthOf(i);
    m_pComponent[i].m_ulHeight        = src->HeightOf(i) + dst->HeightOf(i);
    m_pComponent[i].m_ulBytesPerPixel = SuggestBPP(src->BitsOf(i),src->isFloat(i));
    m_pComponent[i].m_ulBytesPerRow   = m_pComponent[i].m_ulWidth * m_pComponent[i].m_ulBytesPerPixel;
    m_ppucMemory[i]                   = new UBYTE[m_pComponent[i].m_ulBytesPerRow * m_pComponent[i].m_ulHeight];
    m_pComponent[i].m_pPtr            = m_ppucMemory[i];
    if (src->BitsOf(i) <= 8) {
      InterleaveData<UBYTE>((UBYTE *)m_ppucMemory[i],m_pComponent[i].m_ulBytesPerPixel,m_pComponent[i].m_ulBytesPerRow,
			    WidthOf(i),HeightOf(i),
			    src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    dst->DataOf(i),dst->BytesPerPixel(i),dst->BytesPerRow(i));
    } else if (src->BitsOf(i) <= 16 && !src->isFloat(i)) {
      InterleaveData<UWORD>((UWORD *)m_ppucMemory[i],m_pComponent[i].m_ulBytesPerPixel,m_pComponent[i].m_ulBytesPerRow,
			    WidthOf(i),HeightOf(i),
			    src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    dst->DataOf(i),dst->BytesPerPixel(i),dst->BytesPerRow(i));
    } else if (src->BitsOf(i) <= 32) {
      // This can also be float (or half-float, represented as float)
      InterleaveData<ULONG>((ULONG *)m_ppucMemory[i],m_pComponent[i].m_ulBytesPerPixel,m_pComponent[i].m_ulBytesPerRow,
			    WidthOf(i),HeightOf(i),
			    src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    dst->DataOf(i),dst->BytesPerPixel(i),dst->BytesPerRow(i));
    } else if (src->BitsOf(i) <= 64) {
      InterleaveData<UQUAD>((UQUAD *)m_ppucMemory[i],m_pComponent[i].m_ulBytesPerPixel,m_pComponent[i].m_ulBytesPerRow,
			    WidthOf(i),HeightOf(i),
			    src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			    dst->DataOf(i),dst->BytesPerPixel(i),dst->BytesPerRow(i));
    } else throw "Invalid data type in source image for MergeFields";
  }

  //
  // Now move in our code
  Swap(*src);
  
  return in;
}
///
