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
 * Test main file
 * 
 * $Id: imglayout.cpp,v 1.37 2017/01/31 11:58:04 thor Exp $
 *
 * This class defines the image layout, width, height and the
 * image depth of the individual components. It is supplied by
 * the loading class (j2k in our case, or BMP, or PGM, or PGX)
 * and provided to the saving class.
 */

/// Include
#include "std/stdarg.hpp"
#include "std/stdio.hpp"
#include "std/errno.hpp"
#include "cmd/main.hpp"
#include "img/imglayout.hpp"
#include "img/imgspecs.hpp"
#include "img/simpleppm.hpp"
#include "img/simplebmp.hpp"
#include "img/simplepgx.hpp"
#include "img/simpletiff.hpp"
#include "img/simplepng.hpp"
#include "img/simplergbe.hpp"
#include "img/simpleexr.hpp"
#include "img/simpleraw.hpp"
#include "img/simpledpx.hpp"
#include "img/blankimg.hpp"
///

/// ImageLayout::ImageLayout
ImageLayout::ImageLayout(void)
  : m_pNext(NULL), m_pFileStore(NULL),
    m_ulWidth(0), m_ulHeight(0), m_usDepth(0), m_usAlphaDepth(0), 
    m_pComponent(NULL)
{ }
///

/// ImageLayout::ImageLayout
// The copy constructor
ImageLayout::ImageLayout(const class ImageLayout &src)
  : m_pNext(NULL), m_pFileStore(NULL),
    m_ulWidth(src.m_ulWidth), m_ulHeight(src.m_ulHeight), 
    m_usDepth(src.m_usDepth), m_usAlphaDepth(src.m_usAlphaDepth),
    m_pComponent(new struct ComponentLayout[m_usDepth])
{
  UWORD i;
  //
  if (src.m_pComponent) {
    UWORD size = m_usDepth;
    // Now copy the component layouts over, including the memory pointers.
    for(i = 0; i < size; i++) {
      // The compiler-generated assignment operator is fine for us here.
      m_pComponent[i] = src.m_pComponent[i];
    }
  }
}
///

/// ImageLayout::~ImageLayout
ImageLayout::~ImageLayout(void)
{
  delete[] m_pComponent;
  if (m_pFileStore) {
    fclose(m_pFileStore);
  }
}
///

/// ImageLayout::operator=
// Assignment operator. Makes also a clone of the source,
// installing it here.
class ImageLayout &ImageLayout::operator=(const class ImageLayout &src)
{
  UWORD i,size = src.m_usDepth;
  //
  if (m_usDepth != src.m_usDepth || m_usAlphaDepth != src.m_usAlphaDepth || m_pComponent == NULL) {
    struct ComponentLayout *newcomp = new struct ComponentLayout[size];
    if (src.m_pComponent) {
      for(i = 0;i < size;i++) {
	// The default assignment is just fine.
	newcomp[i] = src.m_pComponent[i];
      }
    }
    delete[] m_pComponent;
    m_pComponent   = newcomp;
    m_usDepth      = src.m_usDepth;
    m_usAlphaDepth = src.m_usAlphaDepth;
  } else {
    // No need to allocate memory, just copy it over.
    if (src.m_pComponent) {
      for(i = 0;i < size;i++) {
	// The default assignment is just fine.
	m_pComponent[i] = src.m_pComponent[i];
      }
    }
  }
  m_ulWidth      = src.m_ulWidth;
  m_ulHeight     = src.m_ulHeight;

  return *this;
}
///

/// ImageLayout::PostError
// This can be called by subclasses to indicate an
// error
void TYPE_CDECL ImageLayout::PostError(const char *fmt,...)
{
  char buffer[4096];
  va_list args;
  //
  va_start(args,fmt);
  vsnprintf(buffer,4095,fmt,args);

  throw buffer;

  va_end(args);
}
///

/// ImageLayout::CreateComponents
// Allocate an image layout for the given width and height.
// Should only be used for building up the component array.
void ImageLayout::CreateComponents(ULONG w,ULONG h,UWORD d)
{
  UWORD i;
  // Delete the old image components. Does not release the
  // memory we hold.
  delete[] m_pComponent;
  m_pComponent  = NULL;
  //
  m_ulWidth     = w;
  m_ulHeight    = h;
  m_usDepth     = d;
  m_pComponent  = new struct ComponentLayout[d];
  //
  // Initialize component dimensions.
  for(i = 0; i < d; i++) {
    m_pComponent[i].m_ulWidth  = w;
    m_pComponent[i].m_ulHeight = h;
  }
}
///

/// ImageLayout::CreateComponents
// Create components from a template, using the same layout as the
// original.
void ImageLayout::CreateComponents(const class ImageLayout &o)
{ 
  UWORD i;
  // Delete the old image components. Does not release the
  // memory we hold.
  delete[] m_pComponent;
  m_pComponent  = NULL;
  //
  m_ulWidth     = o.m_ulWidth;
  m_ulHeight    = o.m_ulHeight;
  m_usDepth     = o.m_usDepth;
  m_pComponent  = new struct ComponentLayout[m_usDepth];
  //
  // Initialize component dimensions.
  for(i = 0; i < m_usDepth; i++) {
    m_pComponent[i].m_ulWidth  = o.m_pComponent[i].m_ulWidth;
    m_pComponent[i].m_ulHeight = o.m_pComponent[i].m_ulHeight;
    m_pComponent[i].m_ucBits   = o.m_pComponent[i].m_ucBits;
    m_pComponent[i].m_bSigned  = o.m_pComponent[i].m_bSigned;
    m_pComponent[i].m_bFloat   = o.m_pComponent[i].m_bFloat;
    m_pComponent[i].m_ucSubX   = o.m_pComponent[i].m_ucSubX;
    m_pComponent[i].m_ucSubY   = o.m_pComponent[i].m_ucSubY;
  }
}
///

/// ImageLayout::Swap
// Swap this image layout internals with that of the given source.
void ImageLayout::Swap(class ImageLayout &o)
{
  ULONG t;
  UWORD d;
  struct ComponentLayout *p;

  t = m_ulWidth ,m_ulWidth  = o.m_ulWidth ,o.m_ulWidth  = t;
  t = m_ulHeight,m_ulHeight = o.m_ulHeight,o.m_ulHeight = t;
  d = m_usDepth ,m_usDepth  = o.m_usDepth ,o.m_usDepth  = d;
  d = m_usAlphaDepth ,m_usAlphaDepth  = o.m_usAlphaDepth ,o.m_usAlphaDepth  = d;
  
  p = m_pComponent,m_pComponent = o.m_pComponent,o.m_pComponent = p;
}
///

/// ImageLayout::Restrict
// Reduce the image to a single component, namely the given one.
// Does not filter, etc...
void ImageLayout::Restrict(UWORD comp,UWORD count)
{
  UWORD first,last,i;
  
  if (comp >= m_usDepth)
    throw "component out of range";

  first = comp;
  last  = comp + count - 1;
  if (last >= m_usDepth)
    last = m_usDepth - 1;

  for(i = first;i <= last;i++) {
    m_pComponent[i - first] = m_pComponent[i];
  }
  m_usDepth       = last - first + 1;
  m_ulWidth       = WidthOf(0);
  m_ulHeight      = HeightOf(0);
}
///

/// ImageLayout::Crop
// Crop an image region from the image given the coordinates of the edges.
void ImageLayout::Crop(ULONG x1,ULONG y1,ULONG x2,ULONG y2)
{
  UWORD i;
  
  if (x1 > x2 || y1 > y2)
    throw "cropping region must be non-empty";

  if (x2 >= m_ulWidth)
    x2 = m_ulWidth - 1;

  if (y2 >= m_ulHeight)
    y2 = m_ulHeight - 1;
  
  for(i = 0;i < m_usDepth;i++) {
    if (m_pComponent[i].m_ucSubX <= 0 || m_pComponent[i].m_ucSubY <= 0)
      throw "detected invalid component subsampling while cropping";
    if (x1 % m_pComponent[i].m_ucSubX)
      throw "x1 cropping coordinate must be divisible by horizontal subsampling factor";
    if (y1 % m_pComponent[i].m_ucSubY)
      throw "y1 cropping coordinate must be divisible by vertical subsampling factor";
    if ((x2 + 1) % m_pComponent[i].m_ucSubX)
      throw "x2 + 1 must be divisible by horizontal subsampling factor for cropping";
    if ((y2 + 1) % m_pComponent[i].m_ucSubY)
      throw "y1 + 1 must be divisible by vertical subsampling factor for cropping";
  }
  
  m_ulWidth  = x2 - x1 + 1;
  m_ulHeight = y2 - y1 + 1;
  
  for(i = 0;i < m_usDepth;i++) {
    m_pComponent[i].m_ulWidth  = (x2 + 1) / m_pComponent[i].m_ucSubX - x1 / m_pComponent[i].m_ucSubX;
    m_pComponent[i].m_ulHeight = (y2 + 1) / m_pComponent[i].m_ucSubY - y1 / m_pComponent[i].m_ucSubY;
    m_pComponent[i].m_pPtr     = ((UBYTE *)m_pComponent[i].m_pPtr) + 
      (x1 / m_pComponent[i].m_ucSubX) * m_pComponent[i].m_ulBytesPerPixel +
      (y1 / m_pComponent[i].m_ucSubY) * m_pComponent[i].m_ulBytesPerRow;
  }
}
///

/// ImageLayout::LoadImage
// Load an image from the specified filespec using the appropriate file type,
// derived from the extension. Returns the proper loader.
class ImageLayout *ImageLayout::LoadImage(const char *filename,struct ImgSpecs &specs)
{  
  class ImageLayout *img = NULL;
  const char *ext        = strrchr(filename,'.');
  //
  if (ext == NULL) {
    throw "no file format extender, can't load source image";
    return NULL;
  }
  //
  //
  try {
    //
    // Now get the stream extender.
    if (!strcmp(ext,".ppm") || !strcmp(ext,".pgm") || !strcmp(ext,".pbm") || 
	!strcmp(ext,".pnm") || !strcmp(ext,".pfm") || !strcmp(ext,".pfs")) {
      // PPM family
      class SimplePpm *ppm = new SimplePpm;
      img = ppm;
      ppm->LoadImage(filename,specs);
    } else if (!strcmp(ext,".bmp")) {
      // BMP family
      class SimpleBmp *bmp = new SimpleBmp;
      img = bmp;
      bmp->LoadImage(filename,specs);
    } else if (!strcmp(ext,".pgx")) {
      // PGX for JPEG2000 part 4 compliance testing.
      class SimplePgx *pgx = new SimplePgx;
      img = pgx;
      pgx->LoadImage(filename,specs);
    } else if (!strcmp(ext,".tif") || !strcmp(ext,".tiff")) {
      // TIFF family
      class SimpleTiff *tif = new SimpleTiff;
      img = tif;
      tif->LoadImage(filename,specs);
    } else if (!strcmp(ext,".png")) {
      // PNG
#ifdef USE_PNG
      class SimplePng *png = new SimplePng;
      img = png;
      png->LoadImage(filename,specs);
#else
      throw "PNG support is not compiled in, sorry!";
#endif
    } else if (!strcmp(ext,".rgbe") || !strcmp(ext,".hdr")) {
      // RGBE family.
      class SimpleRGBE *rgbe = new SimpleRGBE;
      img = rgbe;
      rgbe->LoadImage(filename,specs);
    } else if (!strcmp(ext,".dpx")) {
      // DPX
      class SimpleDPX *dpx = new SimpleDPX;
      img = dpx;
      dpx->LoadImage(filename,specs);
    } else if (!strcmp(ext,".exr")) {
      // EXR
#ifdef USE_EXR
      class SimpleEXR *exr = new SimpleEXR;
      img = exr;
      exr->LoadImage(filename,specs);
#else
      throw "EXR support isnot compiled in, sorry!";
#endif
    } else if (!strncmp(ext,".raw",4)  || !strncmp(ext,".craw",5) ||
	       !strncmp(ext,".v210",4) || !strncmp(ext,".yuv",4)) {
      // RAW family
      class SimpleRaw *raw = new SimpleRaw;
      img = raw;
      raw->LoadImage(filename,specs);
    } else {
      PostError ("unknown source image file format, only pnm (pgm,pbm,ppm), pgx, tiff, rgbe, raw and bmp are supported");
    }
  } catch(...) {
    delete img;
    throw;
  }
  //
  return img;
}
///

/// ImageLayout::SaveImage
// Save an image back to a file
void ImageLayout::SaveImage(const char *filename,const struct ImgSpecs &specs)
{
  const char *ext        = strrchr(filename,'.');
  //
  // Now get the stream extender.
  if (ext == NULL) {
    throw "no file format extender, unknown format - can't save image";
  }
  //
  if (!strcmp(ext,".ppm") || !strcmp(ext,".pgm") || !strcmp(ext,".pbm") || 
      !strcmp(ext,".pfm") || !strcmp(ext,".pnm")) {
    // PPM family
    class SimplePpm ppm(*this);
    ppm.SaveImage(filename,specs);
  } else if (!strcmp(ext,".pfs")) {
    // PFS, part of the PPM family
    class SimplePpm ppm(*this);
    ppm.SaveImage(filename,specs,true);
  } else if (!strcmp(ext,".bmp")) {
    // BMP family
    class SimpleBmp bmp(*this);
    bmp.SaveImage(filename,specs);
  } else if (!strcmp(ext,".pgx")) {
    // PGX for compliance testing.
    class SimplePgx pgx(*this);
    pgx.SaveImage(filename,specs);
  } else if (!strcmp(ext,".tif") || !strcmp(ext,".tiff")) {
    // TIFF
    class SimpleTiff tif(*this);
    tif.SaveImage(filename,specs);
  } else if (!strcmp(ext,".png")) {
#ifdef USE_PNG
    class SimplePng png(*this);
    png.SaveImage(filename,specs);
#else
    throw "PNG support is not compiled in, sorry";
#endif
  } else if (!strcmp(ext,".exr")) {
#ifdef USE_EXR
    class SimpleEXR exr(*this);
    exr.SaveImage(filename,specs);
#else
    throw "EXR support is not compiled in, sorry";
#endif
  } else if (!strcmp(ext,".rgbe") || !strcmp(ext,".hdr")) {
    // RGBE
    class SimpleRGBE rgbe(*this);
    rgbe.SaveImage(filename,specs);
  } else if (!strcmp(ext,".dpx")) {
    // DPX
    class SimpleDPX dpx(*this);
    dpx.SaveImage(filename,specs);
  } else if (!strncmp(ext,".raw",4) || !strncmp(ext,".craw",5) ||
	     !strncmp(ext,".v210",4) || !strncmp(ext,".yuv",4)) {
    class SimpleRaw raw(*this);
    // raw
    raw.SaveImage(filename,specs);
  } else {
    fprintf(stderr,"unknown target image file format\n");
  }
}
///

/// ImageLayout::SaveImage
// Save an image with default specifications.
void ImageLayout::SaveImage(const char *filename)
{
  struct ImgSpecs stdspecs;

  SaveImage(filename,stdspecs);
}
///
/// ImageLayout::CloneLayout
// Clone the layout of an image and create an image of the same dimensions just
// with no data.
class ImageLayout *ImageLayout::CloneLayout(const class ImageLayout *org)
{
  class BlankImg *img = new BlankImg(*org);

  img->Blank();

  return img;
}
///

/// ImageLayout::TestIfCompatible
// Check whether the two images are compatible in dimension and depth
// to allow a comparison. Throw if not.
void ImageLayout::TestIfCompatible(const class ImageLayout *dst) const
{
  UWORD comp;
  
  if (DepthOf() != dst->DepthOf())
    throw "number of components of the two images are different, cannot compare";
  //
  for(comp = 0;comp < DepthOf();comp++) {
    if (WidthOf(comp)  != dst->WidthOf(comp) ||
	HeightOf(comp) != dst->HeightOf(comp))
      throw "component dimensions are different, cannot compare";
    if (isFloat(comp)  != dst->isFloat(comp))
      throw "component data types are different, cannot compare";
    if (!isFloat(comp)) {
      if (BitsOf(comp)   != dst->BitsOf(comp))
	throw "component precisions are different, cannot compare";
      if (isSigned(comp) != dst->isSigned(comp))
	throw "component signs are different, cannot compare";
    }
  }
}
///
