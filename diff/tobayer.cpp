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
** $Id: tobayer.cpp,v 1.1 2020/11/25 08:13:52 thor Exp $
**
** This class converts an RGB image to a bayer image of a given bayer
** pattern arrangement. It just performs a simple sampling, not
** filtering.
*/

/// Includes
#include "diff/tobayer.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// ToBayer::~ToBayer
ToBayer::~ToBayer(void)
{
  ReleaseComponents(m_pucSource);
  ReleaseComponents(m_pucDestination);
}
///

/// ToBayer::ReleaseComponents
// Release the memory for the target components
// that have been allocated.
void ToBayer::ReleaseComponents(UBYTE *&p)
{
  delete[] p;
}
///

/// ToBayer::SampleData
// Sample the RGB array to generate artificial bayer data
template<typename T>
void ToBayer::SampleData(const T *r,const T *g,const T *b,
			 LONG rbytesperpixel,LONG rbytesperrow,
			 LONG gbytesperpixel,LONG gbytesperrow,
			 LONG bbytesperpixel,LONG bbytesperrow,
			 T *dst,ULONG width,ULONG height)
{
  ULONG x,y;

  for(y = 0;y < height;y+=2) {
    const T *rrow = r;
    const T *grow = g;
    const T *brow = b;
    T *drow       = dst;
    for(x = 0;x < width;x+=2) {
      switch(m_Pattern) {
      case RGGB:
	drow[0]         = *rrow;
	drow[1]         = *(const T *)((const UBYTE *)(grow) + gbytesperpixel);
	drow[width]     = *(const T *)((const UBYTE *)(grow) + gbytesperrow);
	drow[width + 1] = *(const T *)((const UBYTE *)(brow) + bbytesperpixel + bbytesperrow);
	break;
      case GRBG:
 	drow[0]         = *grow;
	drow[1]         = *(const T *)((const UBYTE *)(rrow) + rbytesperpixel);
	drow[width]     = *(const T *)((const UBYTE *)(brow) + bbytesperrow);
	drow[width + 1] = *(const T *)((const UBYTE *)(grow) + gbytesperpixel + gbytesperrow);
	break;
      case GBRG:
	drow[0]         = *grow;
	drow[1]         = *(const T *)((const UBYTE *)(brow) + bbytesperpixel);
	drow[width]     = *(const T *)((const UBYTE *)(rrow) + rbytesperrow);
	drow[width + 1] = *(const T *)((const UBYTE *)(grow) + gbytesperpixel + gbytesperrow);
	break;
      case BGGR:
     	drow[0]         = *brow;
	drow[1]         = *(const T *)((const UBYTE *)(grow) + gbytesperpixel);
	drow[width]     = *(const T *)((const UBYTE *)(grow) + gbytesperrow);
	drow[width + 1] = *(const T *)((const UBYTE *)(rrow) + rbytesperpixel + rbytesperrow);
	break;
      }
      rrow  = (const T *)((const UBYTE *)(rrow) + (rbytesperpixel << 1));
      grow  = (const T *)((const UBYTE *)(grow) + (gbytesperpixel << 1));
      brow  = (const T *)((const UBYTE *)(brow) + (bbytesperpixel << 1));
      drow += 2;
    }
    r    = (const T *)((const UBYTE *)(r) + (rbytesperrow << 1));
    g    = (const T *)((const UBYTE *)(g) + (gbytesperrow << 1));
    b    = (const T *)((const UBYTE *)(b) + (bbytesperrow << 1));
    dst += width << 1;
  }
}
///

/// ToBayer::CreateImageData
// Create the image data from the dimensions computed
void ToBayer::CreateImageData(UBYTE *&data,class ImageLayout *src)
{
  //
  assert(m_pComponent == NULL);
  assert(data == NULL);
  //
  // Allocate the component data pointers.
  m_ulWidth    = src->WidthOf();
  m_ulHeight   = src->HeightOf();
  m_usDepth    = 1;
  m_pComponent = new struct ComponentLayout[m_usDepth];
  //
  // Initialize the component dimensions.
  m_pComponent[0].m_ulWidth  = m_ulWidth;
  m_pComponent[0].m_ulHeight = m_ulHeight;
  m_pComponent[0].m_ucBits   = src->BitsOf(0);
  m_pComponent[0].m_bSigned  = src->isSigned(0);
  m_pComponent[0].m_bFloat   = src->isFloat(0);
  m_pComponent[0].m_ucSubX   = src->SubXOf(0);
  m_pComponent[0].m_ucSubY   = src->SubYOf(0);
  //
  // Fill up the component data pointers.
  UBYTE bps = ImageLayout::SuggestBPP(m_pComponent[0].m_ucBits,m_pComponent[0].m_bFloat);
  //
  data      = new UBYTE[m_pComponent[0].m_ulWidth * m_pComponent[0].m_ulHeight * bps];
  m_pComponent[0].m_ulBytesPerPixel = bps;
  m_pComponent[0].m_ulBytesPerRow   = bps * m_pComponent[0].m_ulWidth;
  m_pComponent[0].m_pPtr            = data;
}
///

/// ToBayer::Sample
// Sample source data to create a bayer pattern image (artificially).
void ToBayer::Sample(UBYTE *&dest,class ImageLayout *src)
{
  UWORD i;
  //
  // Delete the old data
  delete[] m_pComponent;
  m_pComponent = NULL;
  ReleaseComponents(dest);
  
  if (src->DepthOf() != 3)
    throw "Source image must have 3 components";

  if ((src->WidthOf() & 1) || (src->HeightOf() & 1))
    throw "Source image dimensions must be even";

  for(i = 0;i < 3;i++) {
    if (src->WidthOf(i)  != src->WidthOf() ||
	src->HeightOf(i) != src->HeightOf())
      throw "Source image must not be subsampled";
  }

  CreateImageData(dest,src);
  
  if (src->isFloat(0)) {
    switch(src->BitsOf(0)) {
    case 16:
    case 32:
      SampleData<FLOAT>((FLOAT *)(src->DataOf(0)),(FLOAT *)(src->DataOf(1)),(FLOAT *)(src->DataOf(2)),
			src->BytesPerPixel(0),src->BytesPerRow(0),
			src->BytesPerPixel(1),src->BytesPerRow(1),
			src->BytesPerPixel(2),src->BytesPerRow(2),
			(FLOAT *)dest,src->WidthOf(),src->HeightOf());
      break;
    case 64:
      SampleData<DOUBLE>((DOUBLE *)(src->DataOf(0)),(DOUBLE *)(src->DataOf(1)),(DOUBLE *)(src->DataOf(2)),
			 src->BytesPerPixel(0),src->BytesPerRow(0),
			 src->BytesPerPixel(1),src->BytesPerRow(1),
			 src->BytesPerPixel(2),src->BytesPerRow(2),
			 (DOUBLE *)dest,src->WidthOf(),src->HeightOf());
      break;
    default:
      throw "unsupported source pixel type";
      break;
    }
  } else {
    if (src->isSigned(0)) {
      switch((src->BitsOf(0) + 7) & -8) {
      case 8:
	SampleData<BYTE>((BYTE *)(src->DataOf(0)),(BYTE *)(src->DataOf(1)),(BYTE *)(src->DataOf(2)),
			 src->BytesPerPixel(0),src->BytesPerRow(0),
			 src->BytesPerPixel(1),src->BytesPerRow(1),
			 src->BytesPerPixel(2),src->BytesPerRow(2),
			(BYTE *)dest,src->WidthOf(),src->HeightOf());
	break;
      case 16:
	SampleData<WORD>((WORD *)(src->DataOf(0)),(WORD *)(src->DataOf(1)),(WORD *)(src->DataOf(2)),
			 src->BytesPerPixel(0),src->BytesPerRow(0),
			 src->BytesPerPixel(1),src->BytesPerRow(1),
			 src->BytesPerPixel(2),src->BytesPerRow(2),
			 (WORD *)dest,src->WidthOf(),src->HeightOf());
	break;
      case 32:
	SampleData<LONG>((LONG *)(src->DataOf(0)),(LONG *)(src->DataOf(1)),(LONG *)(src->DataOf(2)),
			 src->BytesPerPixel(0),src->BytesPerRow(0),
			 src->BytesPerPixel(1),src->BytesPerRow(1),
			 src->BytesPerPixel(2),src->BytesPerRow(2),
			 (LONG *)dest,src->WidthOf(),src->HeightOf());
	break;
      case 64:
	SampleData<QUAD>((QUAD *)(src->DataOf(0)),(QUAD *)(src->DataOf(1)),(QUAD *)(src->DataOf(2)),
			 src->BytesPerPixel(0),src->BytesPerRow(0),
			 src->BytesPerPixel(1),src->BytesPerRow(1),
			 src->BytesPerPixel(2),src->BytesPerRow(2),
			 (QUAD *)dest,src->WidthOf(),src->HeightOf());
	break;
      default:
	throw "unsupported source pixel type";
	break;
      }
    } else {
      switch((src->BitsOf(0) + 7) & -8) {
      case 8:
	SampleData<UBYTE>((UBYTE *)(src->DataOf(0)),(UBYTE *)(src->DataOf(1)),(UBYTE *)(src->DataOf(2)),
			  src->BytesPerPixel(0),src->BytesPerRow(0),
			  src->BytesPerPixel(1),src->BytesPerRow(1),
			  src->BytesPerPixel(2),src->BytesPerRow(2),
			  (UBYTE *)dest,src->WidthOf(),src->HeightOf());
	break;
      case 16:
	SampleData<UWORD>((UWORD *)(src->DataOf(0)),(UWORD *)(src->DataOf(1)),(UWORD *)(src->DataOf(2)),
			  src->BytesPerPixel(0),src->BytesPerRow(0),
			  src->BytesPerPixel(1),src->BytesPerRow(1),
			  src->BytesPerPixel(2),src->BytesPerRow(2),
			  (UWORD *)dest,src->WidthOf(),src->HeightOf());
	break;
      case 32:
	SampleData<ULONG>((ULONG *)(src->DataOf(0)),(ULONG *)(src->DataOf(1)),(ULONG *)(src->DataOf(2)),
			  src->BytesPerPixel(0),src->BytesPerRow(0),
			  src->BytesPerPixel(1),src->BytesPerRow(1),
			  src->BytesPerPixel(2),src->BytesPerRow(2),
			  (ULONG *)dest,src->WidthOf(),src->HeightOf());
	break;
      case 64:
	SampleData<UQUAD>((UQUAD *)(src->DataOf(0)),(UQUAD *)(src->DataOf(1)),(UQUAD *)(src->DataOf(2)),
			  src->BytesPerPixel(0),src->BytesPerRow(0),
			  src->BytesPerPixel(1),src->BytesPerRow(1),
			  src->BytesPerPixel(2),src->BytesPerRow(2),
			  (UQUAD *)dest,src->WidthOf(),src->HeightOf());
	break;
      default:
	throw "unsupported source pixel type";
	break;
      }
    }
  }
  //
  Swap(*src);
}
///

/// ToBayer::Measure
double ToBayer::Measure(class ImageLayout *src,class ImageLayout *dest,double in)
{
  Sample(m_pucSource,src);
  Sample(m_pucDestination,dest);
  
  return in;
}
///
