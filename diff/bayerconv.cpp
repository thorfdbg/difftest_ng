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
** $Id: bayerconv.cpp,v 1.7 2019/03/01 10:15:56 thor Exp $
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
void BayerConv::ExtractSubPixels(ULONG width,T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
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
    for(x = 0;x < width; x++) {
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
void BayerConv::InsertSubPixels(ULONG width,T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
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
    for(x = subx;x < width; x += 2) {
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
void BayerConv::ReleaseComponents(UBYTE **&p)
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
    m_pComponent[i].m_bFloat   = src->isFloat(0);
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
    UBYTE bps = ImageLayout::SuggestBPP(m_pComponent[i].m_ucBits,m_pComponent[i].m_bFloat);
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
    ULONG sx,sy;
    if (m_bReshuffle) {
      switch(i) {
      case 0:
	sx = m_lrx;
	sy = m_lry;
	break;
      case 1:
	sx = m_lgx;
	sy = m_lgy;
	break;
      case 2:
	sx = m_lkx;
	sy = m_lky;
	break;
      case 3:
	sx = m_lbx;
	sy = m_lby;
	break;
      }
    } else {
      sx = i & 1;
      sy = i >> 1;
    }
    //
    if (isSigned(i)) {
      if (BitsOf(i) <= 8) {
	ExtractSubPixels<BYTE>(m_ulWidth,(BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			       (BYTE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 16) {
	ExtractSubPixels<WORD>(m_ulWidth,(WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			       (WORD *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<LONG>(m_ulWidth,(LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			       (LONG *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<FLOAT>(m_ulWidth,(FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(i) && BitsOf(i) == 64) {
	ExtractSubPixels<DOUBLE>(m_ulWidth,(DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				 (DOUBLE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				 sx,sy);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (BitsOf(i) <= 8) {
	ExtractSubPixels<UBYTE>(m_ulWidth,(UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(UBYTE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 16) {
	ExtractSubPixels<UWORD>(m_ulWidth,(UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(UWORD *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<ULONG>(m_ulWidth,(ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(ULONG *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(i) && BitsOf(i) <= 32) {
	ExtractSubPixels<FLOAT>(m_ulWidth,(FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
				(FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(i) && BitsOf(i) == 64) {
	ExtractSubPixels<DOUBLE>(m_ulWidth,(DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
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
    if (src->WidthOf(i)  != src->WidthOf() ||
	src->HeightOf(i) != src->HeightOf())
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
    ULONG sx,sy;
    if (m_bReshuffle) {
      switch(i) {
      case 0:
	sx = m_lrx;
	sy = m_lry;
	break;
      case 1:
	sx = m_lgx;
	sy = m_lgy;
	break;
      case 2:
	sx = m_lkx;
	sy = m_lky;
	break;
      case 3:
	sx = m_lbx;
	sy = m_lby;
	break;
      }
    } else {
      sx = i & 1;
      sy = i >> 1;
    }
    //
    if (isSigned(0)) {
      if (BitsOf(0) <= 8) {
	InsertSubPixels<BYTE>(m_ulWidth,(BYTE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      (BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 16) {
	InsertSubPixels<WORD>(m_ulWidth,(WORD *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      (WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<LONG>(m_ulWidth,(LONG *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      (LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			      sx,sy);
      } else if (isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<FLOAT>(m_ulWidth,(FLOAT *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) == 64) {
	InsertSubPixels<DOUBLE>(m_ulWidth,(DOUBLE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
				(DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
				sx,sy);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (BitsOf(0) <= 8) {
	InsertSubPixels<UBYTE>(m_ulWidth,(UBYTE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 16) {
	InsertSubPixels<UWORD>(m_ulWidth,(UWORD *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<ULONG>(m_ulWidth,(ULONG *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<FLOAT>(m_ulWidth,(FLOAT *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       (FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) == 64) {
	InsertSubPixels<DOUBLE>(m_ulWidth,(DOUBLE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
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
 
/// BayerConv::Convert422FromBayer
// Convert from Bayer to 422 three-components.
void BayerConv::Convert422FromBayer(UBYTE **&dest,class ImageLayout *src)
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
  m_ulWidth    = src->WidthOf();
  m_ulHeight   = src->HeightOf() >> 1;
  m_usDepth    = 3;
  CreateImageData(dest,src);
  m_pComponent[1].m_ucSubX  = 2;
  m_pComponent[1].m_ulWidth = m_ulWidth >> 1;
  m_pComponent[2].m_ucSubX  = 2;
  m_pComponent[2].m_ulWidth = m_ulWidth >> 1;
  //
  // Now perform the extraction.
  for(i = 0;i < 4;i++) {
    LONG sx    = i & 1;
    LONG sy    = i >> 1;
    ULONG width = m_ulWidth >> 1;
    ULONG bpp;    // destination bytes per pixel. 
    ULONG dx = 0; // destination offset
    UWORD j;      // target component.
    if ((sx == m_lgx && sy == m_lgy) ||
	(sx == m_lkx && sy == m_lky)) {
      // Source is first or second green component.
      j   = 0;
      bpp = BytesPerPixel(0) << 1; // Interleave components
      dx  = sx; // Keep them in the right order horizontally.
    } else if (sx == m_lrx && sy == m_lry) {
      // Source is red.
      j   = 1;
      bpp = BytesPerPixel(1); // Interleave components
    } else if (sx == m_lbx && sy == m_lby) {
      // Source is blue.
      j   = 2;
      bpp = BytesPerPixel(2);
    }
    //
    if (isSigned(j)) {
      if (BitsOf(j) <= 8) {
	ExtractSubPixels<BYTE>(width,dx + (BYTE *)DataOf(j),bpp,BytesPerRow(j),
			       (BYTE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (!isFloat(j) && BitsOf(j) <= 16) {
	ExtractSubPixels<WORD>(width,dx + (WORD *)DataOf(j),bpp,BytesPerRow(j),
			       (WORD *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (!isFloat(j) && BitsOf(j) <= 32) {
	ExtractSubPixels<LONG>(width,dx + (LONG *)DataOf(j),bpp,BytesPerRow(j),
			       (LONG *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
			       sx,sy);
      } else if (isFloat(j) && BitsOf(j) <= 32) {
	ExtractSubPixels<FLOAT>(width,dx + (FLOAT *)DataOf(j),bpp,BytesPerRow(j),
				(FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(j) && BitsOf(j) == 64) {
	ExtractSubPixels<DOUBLE>(width,dx + (DOUBLE *)DataOf(j),bpp,BytesPerRow(j),
				 (DOUBLE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				 sx,sy);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (BitsOf(j) <= 8) {
	ExtractSubPixels<UBYTE>(width,dx + (UBYTE *)DataOf(j),bpp,BytesPerRow(j),
				(UBYTE *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (!isFloat(j) && BitsOf(j) <= 16) {
	ExtractSubPixels<UWORD>(width,dx + (UWORD *)DataOf(j),bpp,BytesPerRow(j),
				(UWORD *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (!isFloat(j) && BitsOf(j) <= 32) {
	ExtractSubPixels<ULONG>(width,dx + (ULONG *)DataOf(j),bpp,BytesPerRow(j),
				(ULONG *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(j) && BitsOf(j) <= 32) {
	ExtractSubPixels<FLOAT>(width,dx + (FLOAT *)DataOf(j),bpp,BytesPerRow(j),
				(FLOAT *)src->DataOf(0),src->BytesPerPixel(0),src->BytesPerRow(0),
				sx,sy);
      } else if (isFloat(j) && BitsOf(j) == 64) {
	ExtractSubPixels<DOUBLE>(width,dx + (DOUBLE *)DataOf(j),bpp,BytesPerRow(j),
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

/// BayerConv::Convert422ToBayer
// Convert from 422 three-components to Bayer.
void BayerConv::Convert422ToBayer(UBYTE **&dest,class ImageLayout *src)
{
  UWORD i;
  //
  // Delete the (potential) old component.
  delete[] m_pComponent;
  m_pComponent = NULL;
  ReleaseComponents(dest);
  //
  // Check the depth.
  if (src->DepthOf() != 3)
    throw "the source image does not consist of three components";
  //
  if (src->SubXOf(0) != 1 || src->SubYOf(0) != 1 ||
      src->SubXOf(1) != 2 || src->SubYOf(1) != 1 ||
      src->SubXOf(2) != 2 || src->SubYOf(2) != 1)
    throw "the source image is not 4:2:2 sampled";
  //
  if (src->WidthOf() & 1)
    throw "the source width must be even";
  //
  // All sub-images must have the right size.
  for(i = 0;i < 3;i++) {
    ULONG width = src->WidthOf();
    // Red and blue must be subsampled.
    if (i)
      width >>= 1;
    //
    if (src->WidthOf(i)  != width ||
	src->HeightOf(i) != src->HeightOf())
      throw "source image components must have all the same size";
    //
    // The bitdepth and signed-ness and datatypes
    // must also be all equal.
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
  m_ulWidth    = src->WidthOf();
  m_ulHeight   = src->HeightOf() << 1;
  m_usDepth    = 1;
  CreateImageData(dest,src);
  //
  // Now perform the extraction.
  for(i = 0;i < 4;i++) {
    LONG sx = i & 1;
    LONG sy = i >> 1;
    ULONG bpp; // source bytes per pixel.
    ULONG dx = 0;  // source offset into component.
    ULONG width = m_ulWidth;
    UWORD j;   // source component
    if ((sx == m_lgx && sy == m_lgy) ||
	(sx == m_lkx && sy == m_lky)) {
      // Source is green
      j   = 0;
      bpp = src->BytesPerPixel(0) << 1;
      dx  = sx;
    } else if (sx == m_lrx && sy == m_lry) {
      // Source is red
      j   = 1;
      bpp = src->BytesPerPixel(1);
    } else if (sx == m_lbx && sy == m_lby) {
      // Source is blue
      j   = 2;
      bpp = src->BytesPerPixel(2);
    }
    //
    if (isSigned(0)) {
      if (BitsOf(0) <= 8) {
	InsertSubPixels<BYTE>(width,(BYTE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      dx + (BYTE *)src->DataOf(j),bpp,src->BytesPerRow(j),
			      sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 16) {
	InsertSubPixels<WORD>(width,(WORD *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      dx + (WORD *)src->DataOf(j),bpp,src->BytesPerRow(j),
			      sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<LONG>(width,(LONG *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			      dx + (LONG *)src->DataOf(j),bpp,src->BytesPerRow(j),
			      sx,sy);
      } else if (isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<FLOAT>(width,(FLOAT *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       dx + (FLOAT *)src->DataOf(j),bpp,src->BytesPerRow(j),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) == 64) {
	InsertSubPixels<DOUBLE>(width,(DOUBLE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
				dx + (DOUBLE *)src->DataOf(j),bpp,src->BytesPerRow(j),
				sx,sy);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (BitsOf(0) <= 8) {
	InsertSubPixels<UBYTE>(width,(UBYTE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       dx + (UBYTE *)src->DataOf(j),bpp,src->BytesPerRow(j),
			       sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 16) {
	InsertSubPixels<UWORD>(width,(UWORD *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       dx + (UWORD *)src->DataOf(j),bpp,src->BytesPerRow(j),
			       sx,sy);
      } else if (!isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<ULONG>(width,(ULONG *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       dx + (ULONG *)src->DataOf(j),bpp,src->BytesPerRow(j),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) <= 32) {
	InsertSubPixels<FLOAT>(width,(FLOAT *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
			       dx + (FLOAT *)src->DataOf(j),bpp,src->BytesPerRow(j),
			       sx,sy);
      } else if (isFloat(0) && BitsOf(0) == 64) {
	InsertSubPixels<DOUBLE>(width,(DOUBLE *)DataOf(0),BytesPerPixel(0),BytesPerRow(0),
				dx + (DOUBLE *)src->DataOf(j),bpp,src->BytesPerRow(j),
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
  if (m_b422) {
    if (m_bToBayer) {
      Convert422ToBayer(m_ppucSource,src);
      Convert422ToBayer(m_ppucDestination,dst);
    } else {
      Convert422FromBayer(m_ppucSource,src);
      Convert422FromBayer(m_ppucDestination,dst);
    }
  } else {
    if (m_bToBayer) {
      ConvertToBayer(m_ppucSource,src);
      ConvertToBayer(m_ppucDestination,dst);
    } else {
      ConvertFromBayer(m_ppucSource,src);
      ConvertFromBayer(m_ppucDestination,dst);
    }
  }
  
  return in;
}
///

