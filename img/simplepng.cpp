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
 * An image class to load and save PNG images
 * from a file or to a file.
 * $Id: simplepng.cpp,v 1.9 2017/01/31 11:58:04 thor Exp $
 */

/// Includes
#include "config.h"
#include "std/stdio.hpp"
#include "std/string.hpp"
#include "std/stdlib.hpp"
#include "tools/file.hpp"
#include "img/imgspecs.hpp"
#include "img/simplepng.hpp"
///

/// Defines
#ifdef USE_PNG
#include <png.h>
///

/// RAII helpers
template<bool writing>
class Png {
  //
  // The administrated object 
  png_structp png_ptr;
  //
  png_infop info_ptr;
  //
  // This is pretty much a GNU extension, namely to
  // throw "through a C API".
  static void error_function(png_structp,const char *error)
  {
    throw error;
  }
  //
public:
  Png(void)
    : png_ptr(NULL), info_ptr(NULL)
  {
    if (writing) {
      png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      if (png_ptr == NULL)
	throw "unable to initialize the PNG writer";
      
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL) {
	png_destroy_write_struct(&png_ptr,NULL);
	throw "unable to initialize the PNG writer";
      }
    } else {
      png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      if (png_ptr == NULL)
	throw "unable to initialize the PNG reader";
      
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL) {
	png_destroy_read_struct(&png_ptr,NULL,NULL);
	throw "unable to initialize the PNG reader";
      }
    }
    png_set_error_fn(png_ptr,NULL,&error_function,NULL);
  }
  //
  ~Png(void)
  {
    if (writing)
      png_destroy_write_struct(&png_ptr,&info_ptr);
    else
      png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
  }
  //
  operator png_structp (void) const
  {
    return png_ptr;
  }
  //
  png_infop info(void) const
  {
    return info_ptr;
  }
};
///
  
/// SimplePng::SimplePng
// The default constructor
SimplePng::SimplePng(void)
  : m_pImage(NULL), m_ppRowPointers(NULL),
    m_puqRed(NULL), m_puqGreen(NULL), m_puqBlue(NULL),
    m_bPalettized(false), m_ulPaletteSize(0)
{
}
///

/// SimplePng::SimplePng
// Copy constructor
SimplePng::SimplePng(const class ImageLayout &il)
  : ImageLayout(il), m_pImage(NULL), m_ppRowPointers(NULL),
    m_puqRed(NULL), m_puqGreen(NULL), m_puqBlue(NULL),
    m_bPalettized(false), m_ulPaletteSize(0)
{
}
///

/// SimplePng::~SimplePng
// Destructor
SimplePng::~SimplePng(void)
{
  if (m_pImage)
    free(m_pImage);
  delete[] m_ppRowPointers;
  delete[] m_puqRed;
  delete[] m_puqGreen;
  delete[] m_puqBlue;
}
///

/// SimplePng::LoadImage
// Load an image from a level 1 file descriptor, keep it within
// the internals of this class. The accessor methods below
// should be used to find out more about this image.
void SimplePng::LoadImage(const char *basename,struct ImgSpecs &specs)
{
  UBYTE bits;
  UBYTE shift = 0; // as PNG may upshift on color expansion, this is the downshift.
  UBYTE pbcomp = 1;
  ULONG pbrow,y;
  UWORD c;
  bool alpha  = false;
  png_byte header[8];
  File source(basename,"rb");

  if (m_pComponent)
    throw "Image is already loaded";
  
  if (fread(header, 1, sizeof(header), source) != sizeof(header))
    throw "input file is not a PNG file";

  if (png_sig_cmp(header, 0, 8))
    throw "input file is not a PNG file";

  Png<false> reader;

  png_init_io(reader,source);
  png_set_sig_bytes(reader, sizeof(header));
  png_read_info(reader,reader.info());

  m_ulWidth  = png_get_image_width(reader,reader.info());
  m_ulHeight = png_get_image_height(reader,reader.info());
  bits       = png_get_bit_depth(reader,reader.info());

  specs.Palettized = ImgSpecs::No;
  
  switch(png_get_color_type(reader,reader.info())) {
  case PNG_COLOR_TYPE_GRAY:
    m_usDepth = 1;
    if (png_get_valid(reader,reader.info(),PNG_INFO_tRNS)) {
      png_set_tRNS_to_alpha(reader);
      m_usDepth++;
      alpha = true;
    }
    if (bits < 8) {
      png_set_expand_gray_1_2_4_to_8(reader);
      shift = 8 - bits; // backshift to normalize to the standard alignment.
    }
    break;
  case PNG_COLOR_TYPE_PALETTE:
    m_usDepth        = 3;
    m_bPalettized    = true;
    specs.Palettized = ImgSpecs::Yes;
    bits             = 8;
    png_set_palette_to_rgb(reader);
    if (png_get_valid(reader,reader.info(),PNG_INFO_tRNS)) {
      png_set_tRNS_to_alpha(reader);
      m_usDepth++;
      alpha = true;
    }
    // get palette.
    {
      int i,entries;
      png_colorp palette;
      
      if (png_get_PLTE(reader,reader.info(),&palette,&entries)) {
	m_ulPaletteSize = entries;
	m_puqRed        = new UQUAD[entries];
	m_puqGreen      = new UQUAD[entries];
	m_puqBlue       = new UQUAD[entries];

	for(i = 0;i < entries;i++) {
	  m_puqRed[i]   = palette[i].red;
	  m_puqGreen[i] = palette[i].green;
	  m_puqBlue[i]  = palette[i].blue;
	}
      } else throw "failure in the PNG reader: cannot obtain the palette in a palette coded file";
    }
    break;
  case PNG_COLOR_TYPE_RGB:
    m_usDepth = 3;
    if (png_get_valid(reader,reader.info(),PNG_INFO_tRNS)) {
      png_set_tRNS_to_alpha(reader);
      m_usDepth++;
      alpha = true;
    }
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    m_usDepth = 4;
    if (png_get_valid(reader,reader.info(),PNG_INFO_tRNS)) {
      png_set_tRNS_to_alpha(reader);
    }
    alpha = true;
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    m_usDepth = 2;
    if (png_get_valid(reader,reader.info(),PNG_INFO_tRNS)) {
      png_set_tRNS_to_alpha(reader);
    }
    alpha = true;
    break;
  default:
    throw "detected unknown or unsupported PNG color type";
  }
  //
  // create memory for the data
  if (bits <= 8) {
    m_pImage = malloc(m_ulWidth*m_ulHeight*m_usDepth * sizeof(UBYTE));
    pbcomp   = sizeof(UBYTE);
    pbrow    = sizeof(UBYTE) * m_ulWidth * m_usDepth;
  } else {
    m_pImage = malloc(m_ulWidth*m_ulHeight*m_usDepth * sizeof(UWORD));
    pbcomp   = sizeof(UWORD);
    pbrow    = sizeof(UWORD) * m_ulWidth * m_usDepth;
#ifdef J2K_LIL_ENDIAN
    // Convert to little endian.
    png_set_swap(reader);
#endif
  }
  if (m_pImage == NULL)
    throw "cannot allocate PNG image, out of memory";
  //
  // Create the image layout.
  CreateComponents(m_ulWidth,m_ulHeight,m_usDepth);
  //
  // Create the row pointers.
  m_ppRowPointers = new APTR[m_ulHeight];
  for(y = 0;y < m_ulHeight;y++)
    m_ppRowPointers[y] = ((UBYTE *)m_pImage) + y * pbrow;

  for(c = 0;c < m_usDepth;c++) {
    m_pComponent[c].m_pPtr            = (UBYTE *)(m_pImage) + pbcomp * c;
    m_pComponent[c].m_ulBytesPerRow   = pbrow;
    m_pComponent[c].m_ulBytesPerPixel = pbcomp * m_usDepth;
    m_pComponent[c].m_ucBits          = bits;
  }
  
  png_read_image(reader,(png_byte**)(m_ppRowPointers));
  png_read_end(reader,reader.info());
  
  // Adjust low-grey by shifting back to the original.
  if (shift > 0) {
    ULONG x;
    UWORD comps = m_usDepth;
    assert(bits < 8); // We have UBYTE data here always.
    
    if (alpha)
      comps--; // alpha is not shifted (is this correct??)

    for(y = 0;y < m_ulHeight;y++) {
      for(c = 0;c < comps;c++) {
	UBYTE *p  = ((UBYTE *)m_pComponent[c].m_pPtr) + y * m_pComponent[c].m_ulBytesPerRow;
	ULONG bpp = m_pComponent[c].m_ulBytesPerPixel;
	for(x = 0;x < m_ulWidth;x++) {
	  *p >>= shift;
	  p   += bpp;
	}
      }
    }
  }
  //
  // That's it. RAII cleans up.
}
///

/// SimplePng::SaveImage
// Save an image to a level 1 file descriptor, given its
// width, height and depth. We only support grey level and
// RGB here, no palette images.
void SimplePng::SaveImage(const char *basename,const struct ImgSpecs &)
{
  UWORD c;
  UBYTE depth = m_pComponent[0].m_ucBits;
  ULONG x,y;
  int colortype;
  assert(m_pImage == NULL);

  for(c = 0;c < m_usDepth;c++) {
    if (m_pComponent[c].m_ucBits != depth)
      throw "PNG does not support components with differing bit depths";
    if (m_pComponent[c].m_ulWidth  != m_ulWidth ||
	m_pComponent[c].m_ulHeight != m_ulHeight)
      throw "PNG does not support subsampling";
    if (m_pComponent[c].m_bSigned)
      throw "PNG does not support signed components";
    if (m_pComponent[c].m_bFloat)
      throw "PNG does not support floating point";
  }

  if (m_usDepth == 1 || m_usDepth == 2) {
    if (depth != 1 && depth != 2 && depth != 4 && depth != 8 && depth != 16)
      throw "PNG does not support bit depths other than 1,2,4,8 or 16 for grey scale images";
    if (m_usDepth == 1) {
      colortype = PNG_COLOR_TYPE_GRAY;
    } else {
      colortype = PNG_COLOR_TYPE_GRAY_ALPHA;
    }
  } else if (m_usDepth == 3 || m_usDepth == 4) {
    if (depth != 8 && depth != 16)
      throw "PNG does not support bit depths other than 8 or 16 for color images";
    if (m_usDepth == 3) {
      colortype = PNG_COLOR_TYPE_RGB;
    } else {
      colortype = PNG_COLOR_TYPE_RGB_ALPHA;
    }
  } else {
    throw "PNG does not support more than four components";
  }
  
  File target(basename,"wb");
  Png<true> writer;

  png_init_io(writer,target);

  png_set_IHDR(writer,writer.info(),
	       m_ulWidth,m_ulHeight,depth,colortype,
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(writer,writer.info());

  /*
  ** Allocate an auxilluary row buffer as the data need not to be in the PNG
  ** required form.
  */
  if (depth > 8) {
    m_pImage = malloc(m_ulWidth * m_usDepth * sizeof(UWORD));
#ifdef J2K_LIL_ENDIAN
    // Convert to little endian.
    png_set_swap(writer);
#endif
  } else {
    m_pImage = malloc(m_ulWidth * m_usDepth * sizeof(UBYTE));
  }

  for(y = 0;y < m_ulHeight;y++) {
    /*
    ** bring the data back into the order PNG needs them. As
    ** the components could have been read by any other module,
    ** this need not to be the way how they are stored right now.
    ** Bummer! Unlike announced,
    ** automatic bit-packing does not work as of writing this...
    */
    if (depth < 8) {
      assert(m_usDepth == 1);
      UBYTE *src = ((UBYTE *)m_pComponent[0].m_pPtr) + y * m_pComponent[0].m_ulBytesPerRow;
      ULONG bpp  = m_pComponent[0].m_ulBytesPerPixel;
      UBYTE *dst = (UBYTE *)(m_pImage) + 0;
      UBYTE shift = 8;
      UBYTE data  = 0;
      for(x = 0;x < m_ulWidth;x++) {
	shift -= depth;
	data  |= *src << shift;
	if (shift == 0) {
	  *dst  = data;
	  dst  += m_usDepth;
	  shift = 8;
	  data  = 0;
	}
	src += bpp;
      }
      if (shift != 8)
	*dst  = data;
    } else if (depth == 8) {
      for(c = 0;c < m_usDepth;c++) {
	UBYTE *src = ((UBYTE *)m_pComponent[c].m_pPtr) + y * m_pComponent[c].m_ulBytesPerRow;
	ULONG bpp  = m_pComponent[c].m_ulBytesPerPixel;
	UBYTE *dst = (UBYTE *)(m_pImage) + c;
	for(x = 0;x < m_ulWidth;x++) {
	  *dst = *src;
	  src += bpp;
	  dst += m_usDepth;
	}
      }
    } else {
      for(c = 0;c < m_usDepth;c++) {
	UWORD *src = (UWORD *)(((UBYTE *)m_pComponent[c].m_pPtr) + y * m_pComponent[c].m_ulBytesPerRow);
	ULONG bpp  = m_pComponent[c].m_ulBytesPerPixel;
	UWORD *dst = (UWORD *)(m_pImage) + c;
	for(x = 0;x < m_ulWidth;x++) {
	  *dst = *src;
	  src  = (UWORD *)((UBYTE *)(src) + bpp);
	  dst += m_usDepth;
	}
      }
    }
    png_write_row(writer,(png_byte*)m_pImage);
  }
  
  png_write_end(writer, writer.info());
  // RAII cleans up
}
///

///
#endif
///
