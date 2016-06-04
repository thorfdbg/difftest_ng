/*************************************************************************
** Copyright (c) 2003-2016 Accusoft 				        **
**									**
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
 * An image class to load and save EXR images
 * from a file or to a file.
 * $Id: simpleexr.cpp,v 1.7 2016/06/04 10:44:09 thor Exp $
 */

/// Includes
#include "std/stdlib.hpp"
#include "tools/file.hpp"
#include "simpleexr.hpp"
#include "imgspecs.hpp"
#ifdef USE_EXR
#include <ImfInputFile.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImathBox.h>
#include <half.h>
#include <Iex.h>
#include <ImfHeader.h>
#include <ImfStandardAttributes.h>
using namespace Imf;
using namespace Imath;
///

/// SimpleEXR::SimpleEXR
// Default constructor.
SimpleEXR::SimpleEXR(void)
  : m_pfImage(NULL)
{
}
///

/// SimpleEXR::SimpleEXR
// Copy constructor, reference a PPM image.
SimpleEXR::SimpleEXR(const class ImageLayout &org)
  : ImageLayout(org), m_pfImage(NULL)
{
}
///

/// SimpleEXR::~SimpleEXR
// Dispose the object, delete the image
SimpleEXR::~SimpleEXR(void)
{
  delete[] m_pfImage;
}
///

/// SimpleEXR::LoadImage
// Load an image from an already open (binary) PPM or PGM file
// Throw in case the file should be invalid.
void SimpleEXR::LoadImage(const char *basename,struct ImgSpecs &specs)
{ 
  try {
    ::FLOAT *data;
    if (m_pComponent) {
      PostError("Image is already loaded.\n");
    }
    //
    RgbaInputFile in(basename);
    double scale           = 1.0;
    if( hasWhiteLuminance( in.header() ) ) {
      scale = whiteLuminance( in.header() );
    }
    //
    // Check what to do about the scale.
    if (specs.AbsoluteRadiance != ImgSpecs::Yes) {
      specs.RadianceScale = scale;
      scale               = 1.0;
    }
    //
    Box2i dw   = in.dataWindow();
    m_ulWidth  = dw.max.x - dw.min.x + 1;
    m_ulHeight = dw.max.y - dw.min.y + 1;
    m_usDepth  = 3;
    //
    specs.ASCII      = ImgSpecs::No;
    specs.Palettized = ImgSpecs::No;
    specs.YUVEncoded = ImgSpecs::No;
    //
    Array2D <Rgba> pixels (m_ulHeight, m_ulWidth);
    in.setFrameBuffer(&pixels[0][0] - dw.min.y * m_ulWidth - dw.min.x, 1, m_ulWidth);
    in.readPixels (dw.min.y, dw.max.y);
    //
    // Now build the component array.
    CreateComponents(m_ulWidth,m_ulHeight,m_usDepth);
    //
    assert(m_pfImage == NULL);
    //
    // Compute the bit depth from the precision
    data = m_pfImage  = new ::FLOAT[m_ulWidth * m_ulHeight * m_usDepth];
    //
    // Ok, now fill out the components.
    for(UWORD i = 0; i < m_usDepth; i++) {
      m_pComponent[i].m_ucBits          = 32;
      m_pComponent[i].m_ulBytesPerPixel = m_usDepth * 4; // Notice the "per byte" indicator!
      m_pComponent[i].m_ulBytesPerRow   = m_usDepth * 4 * m_ulWidth;
      m_pComponent[i].m_pPtr            = m_pfImage + i;
      m_pComponent[i].m_bFloat          = true;
      m_pComponent[i].m_bSigned         = true;
    }
    for (ULONG y = 0; y < m_ulHeight; ++y) {
      for (ULONG x = 0; x < m_ulWidth; ++x) {
	Rgba &pixel = pixels[y][x];
	*data++     = pixel.r * scale;
	*data++     = pixel.g * scale;
	*data++     = pixel.b * scale;
      }
    }
  } catch(const Iex::BaseExc &ex) {
    static char e[256];
    strncpy(e,ex.what(),255);
    e[255] = 0;
    throw e;
  }
}
///

/// SimpleEXR::SaveImage
// Save the image to a PGM/PPM file, throw in case of error.
void SimpleEXR::SaveImage(const char *basename,const struct ImgSpecs &specs)
{
  try {
    ::FLOAT *r,*g,*b;
    ULONG rbpp,gbpp,bbpp;
    ULONG rbpr,gbpr,bbpr;
    //
    // Must exist.
    if (m_pComponent == NULL) {
      PostError("No image loaded to save data of.\n");
    }
    //
    r    = (::FLOAT *)(m_pComponent[0].m_pPtr);
    rbpp = m_pComponent[0].m_ulBytesPerPixel;
    rbpr = m_pComponent[0].m_ulBytesPerRow;
    //
    if (m_usDepth > 1) {
      g    = (::FLOAT *)(m_pComponent[1].m_pPtr);
      gbpp = m_pComponent[1].m_ulBytesPerPixel;
      gbpr = m_pComponent[1].m_ulBytesPerRow;
    } else {
      g    = r;
      gbpp = rbpp;
      gbpr = rbpr;
    }
    if (m_usDepth > 2) {
      b    = (::FLOAT *)(m_pComponent[2].m_pPtr);
      bbpp = m_pComponent[2].m_ulBytesPerPixel;
      bbpr = m_pComponent[2].m_ulBytesPerRow;
    } else {
      b    = g;
      bbpp = rbpp;
      bbpr = rbpr;
    }
    //
    Array2D <Rgba> pixels (m_ulHeight, m_ulWidth);
    for (ULONG y = 0; y < m_ulHeight; ++y) {
      for (ULONG x = 0; x < m_ulWidth; ++x) {
	Rgba &pixel = pixels[y][x];
	pixel.r     = *r;
	pixel.g     = *g;
	pixel.b     = *b;
	r           = (::FLOAT *)(((UBYTE *)r) + rbpp);
	g           = (::FLOAT *)(((UBYTE *)g) + gbpp);
	b           = (::FLOAT *)(((UBYTE *)b) + bbpp);
      }
      r           = (::FLOAT *)(((UBYTE *)r) - rbpp * m_ulWidth + rbpr);
      g           = (::FLOAT *)(((UBYTE *)g) - gbpp * m_ulWidth + gbpr);
      b           = (::FLOAT *)(((UBYTE *)b) - bbpp * m_ulWidth + bbpr);
    }
    Header hdr(m_ulWidth,m_ulHeight);
    addWhiteLuminance(hdr,specs.RadianceScale);
    RgbaOutputFile out(basename,hdr, WRITE_RGB);
    out.setFrameBuffer(&pixels[0][0], 1, m_ulWidth);
    out.writePixels(m_ulHeight);
  } catch(const Iex::BaseExc &ex) {
    static char e[256];
    strncpy(e,ex.what(),255);
    e[255] = 0;
    throw e;
  }
}
///

///
#endif
