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
** $Id: bayerconv.cpp,v 1.2 2018/09/04 11:44:48 thor Exp $
**
** This class converts bayer pattern images into four-component images
** and back. It *does not* attempt to de-bayer the images.
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/bayerconv.hpp"
#include "std/string.hpp"
///

/// BayerConv::ExtractSubPixels
// Templated extractor class. This takes a subpixel from the
// source and copies it to the target.
template<typename T>
void BayerConv::ExtractSubPixels(T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
				 const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
				 ULONG subx,ULONG suby)
{
  ULONG x,y;

  src              = (const T *)((const UBYTE *)(src) + subx * sbytesperpixel + suby * sbytesperrow);
  sbytesperpixel <<= 1;
  sbytesperrow   <<= 1;
  
  for(y = 0;y < m_ulHeight;y++) {
    const T *s = src;
    T *d = dst;
    //
    for(x = 0;x < m_ulWidth; x++) {
      *d = *s;
      s  = (const T *)((const UBYTE *)(s) + sbytesperpixel);
      d  = (T *)((UBYTE *)(d) + dbytesperpixel);
    }
    src = (const T *)((const UBYTE *)(src) + sbytesperrow);
    dst = (T *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// BayerConv::InsertSubPixels
// Templated injector class. This creates subpixels from the source and
// injects them into the target.
template<typename T>
void BayerConv::InsertSubPixels(T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
				const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
				ULONG subx,ULONG suby)
{
  ULONG x,y;
  
  dst              = (T *)((UBYTE *)(dst) + subx * dbytesperpixel + suby * dbytesperrow);
  dbytesperpixel <<= 1;
  dbytesperrow   <<= 1;

  for(y = suby;y < m_ulHeight;y += 2) {
    const T *s = src;
    T *d = dst;
    //
    for(x = subx;x < m_ulWidth; x += 2) {
      *d = *s;
      s  = (const T *)((const UBYTE *)(s) + sbytesperpixel);
      d  = (T *)((UBYTE *)(d) + dbytesperpixel);
    }
    src = (const T *)((const UBYTE *)(src) + sbytesperrow);
    dst = (T *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// BayerConv::~BayerConv
// Release the memory for the target image
BayerConv::~BayerConv(void)
{
  ReleaseComponents(m_ppucSource);
  ReleaseComponents(m_ppucDestination);
}
///

/// BayerConv::ReleaseComponents
// Release the memory for the target components
// that have been allocated.
void BayerConv::ReleaseComponents(UBYTE **p)
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

/// BayerConv::CreateImageData
// Create the image data from the dimensions computed
void BayerConv::CreateImageData(UBYTE **&data,class ImageLayout *src)
{
  UWORD i;
  //
  assert(m_pComponent == NULL);
  assert(data == NULL);
  //
  // Allocate the component data pointers.
  m_pComponent = new struct ComponentLayout[m_usDepth];
  //
  // Initialize the component dimensions.
  for(i = 0;i < m_usDepth;i++) {
    m_pComponent[i].m_ulWidth  = m_ulWidth;
    m_pComponent[i].m_ulHeight = m_ulHeight;
    m_pComponent[i].m_ucBits   = src->BitsOf(0);
    m_pComponent[i].m_bSigned  = src->isSigned(0);
    m_pComponent[i].m_ucSubX   = src->SubXOf(0);
    m_pComponent[i].m_ucSubY   = src->SubYOf(0);
  }
  //
  // Allocate the component pointers.
  data = new UBYTE *[m_usDepth];
  m_usAllocated = m_usDepth;
  memset(data,0,sizeof(UBYTE *) * m_usDepth);
  //
  // Fill up the component data pointers.
  for(i = 0;i < m_usDepth;i++) {
    UBYTE bps = (m_pComponent[i].m_ucBits + 7) >> 3;
    if (m_pComponent[i].m_bFloat && m_pComponent[i].m_ucBits == 16)
      bps = sizeof(FLOAT); // stored as float
    //
    data[i]                           = new UBYTE[m_pComponent[i].m_ulWidth * m_pComponent[i].m_ulHeight * bps];
    m_pComponent[i].m_ulBytesPerPixel = bps;
    m_pComponent[i].m_ulBytesPerRow   = bps * m_pComponent[i].m_ulWidth;
    m_pComponent[i].m_pPtr            = data[i];
  }
}
///

/// BayerConv::ConvertFromBayer
// Convert from Bayer to four-components.
void BayerConv::ConvertFromBayer(UBYTE **&dest,class ImageLayout *src)
{
  UWORD i;
  //
  // Delete the (potential) old component.
  delete[] m_pComponent;
  m_pComponent = NULL;
  ReleaseComponents(dest);
  //
  // Check the depth.
  if (src->DepthOf() != 1)
    throw "the source image is not grey-scale, must have exactly one component for the conversion";
  // Dimensions must be even.
  if ((src->WidthOf() & 1) | (src->HeightOf() & 1))
    throw "the source image dimensions are odd, but they must be even";
  //
  // Compute the dimensions of the bayern sub-images.
  m_ulWidth    = src->WidthOf()  >> 1;
  m_ulHeight   = src->HeightOf() >> 1;
  m_usDepth    = 4;
  CreateImageData(dest,src);
  //
  // Now perform the extraction.
  for(i = 0;i < m_usDepth;i++) {
    ULONG sx = i & 1;
    ULONG sy = i >> 1;
    //
    if (isSigned(i)) {
      if (BitsOf(i) <= 8) {
	ExtractSubPixels<BYTE>((BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			       (BYTE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 16) {
	ExtractSubPixels<WORD>((WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			       (WORD *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<LONG>((LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			       (LONG *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<FLOAT>((FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(i) && BitsOf(i) == 64) {
	ExtractSubPixels<DOUBLE>((DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				 (DOUBLE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				 sx,sy);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (BitsOf(i) <= 8) {
	ExtractSubPixels<UBYTE>((UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(UBYTE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 16) {
	ExtractSubPixels<UWORD>((UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(UWORD *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<ULONG>((ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(ULONG *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<FLOAT>((FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(i) && BitsOf(i) == 64) {
	ExtractSubPixels<DOUBLE>((DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				 (DOUBLE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				 sx,sy);
      } else {
	throw "unsupported data type";
      }
    }
  }
  //
  Swap(*src);
}
///

/// BayerConv::ConvertToBayer
// Convert from four-components to Bayer.
void BayerConv::ConvertToBayer(UBYTE **&dest,class ImageLayout *src)
{
  UWORD i;
  //
  // Delete the (potential) old component.
  delete[] m_pComponent;
  m_pComponent = NULL;
  ReleaseComponents(dest);
  //
  // Check the depth.
  if (src->DepthOf() != 4)
    throw "the source image does not consist of four components";
  //
  // All sub-images must have the same size
  for(i = 0;i < 4;i++) {
    if (src->WidthOf(i)  != src->WidthOf(0) ||
	src->HeightOf(i) != src->HeightOf(i))
      throw "source image components must have all the same size";
    if (src->SubXOf(i) != 1 || src->SubYOf(i) != 1)
      throw "source image image subsampling must be all 1,1";
    if (src->BitsOf(i) != src->BitsOf(0))
      throw "the bit depth of the source components must be all identical";
    if (src->isSigned(i) != src->isSigned(0))
      throw "the signedness of all source components must be identical";
    if (src->isFloat(i) != src->isFloat(0))
      throw "the source components must be all float or all integer";
  }
  //
  // Compute image dimensions of the new image.
  //
  // Compute the dimensions of the bayern sub-images.
  m_ulWidth    = src->WidthOf()  << 1;
  m_ulHeight   = src->HeightOf() << 1;
  m_usDepth    = 1;
  CreateImageData(dest,src);
  //
  // Now perform the extraction.
  for(i = 0;i < 4;i++) {
    ULONG sx = i & 1;
    ULONG sy = i >> 1;
    //
    if (isSigned(0)) {
      if (BitsOf(0) <= 8) {
	InsertSubPixels<BYTE>((BYTE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      (BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 16) {
	InsertSubPixels<WORD>((WORD *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      (WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<LONG>((LONG *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      (LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      sx,sy);
      } else if (isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<FLOAT>((FLOAT *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) == 64) {
	InsertSubPixels<DOUBLE>((DOUBLE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
				(DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				sx,sy);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (BitsOf(0) <= 8) {
	InsertSubPixels<UBYTE>((UBYTE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 16) {
	InsertSubPixels<UWORD>((UWORD *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<ULONG>((ULONG *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<FLOAT>((FLOAT *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) == 64) {
	InsertSubPixels<DOUBLE>((DOUBLE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
				(DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				sx,sy);
      } else {
	throw "unsupported data type";
      }
    }
  }
  //
  Swap(*src);
}
///
 

/// BayerConv::Measure
// Convert the source and the destination one by one.
double BayerConv::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  if (m_bToBayer) {
    ConvertToBayer(m_ppucSource,src);
    ConvertToBayer(m_ppucDestination,dst);
  } else {
    ConvertFromBayer(m_ppucSource,src);
    ConvertFromBayer(m_ppucDestination,dst);
  }
  
  return in;
}
///

