/*************************************************************************
** Copyright (c) 2011-2014 Accusoft Corporation                         **
**                                                                      **
** Written by Thomas Richter (richter@rus.uni-stuttgart.de)             **
** Sponsored by Accusoft Corporation, Tampa, FL and                     **
** the Computing Center of the University of Stuttgart                  **
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
 * Implementation of the SimpleBMP class, used to render
 * and save simple (hence, the name) BMP images. It
 * does not support any kind of RLC encoded images.
 *
 * $Id: simplebmp.cpp,v 1.20 2014/01/04 11:35:28 thor Exp $
 *
 */

/// Includes
#include "interface/types.hpp"
#include "std/stdio.hpp"
#include "std/string.hpp"
#include "tools/file.hpp"
#include "simplebmp.hpp"
#include "imgspecs.hpp"
///

/// Defines
// Some defines that are BMP specific.
#define BI_RGB  0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3

// Sizes 
#define OLD_OS2_BITMAP     12
#define NEW_WINDOWS_BITMAP 40
#define NEW_OS2_BITMAP     64
#define VERSION_4_HEADER   108
#define VERSION_5_HEADER   124
///

/// SimpleBmp::SimpleBmp
// default constructor
SimpleBmp::SimpleBmp(void) 
  : m_pucImage(NULL), m_puqRed(NULL), m_puqGreen(NULL), m_puqBlue(NULL), m_pucOutbuf(NULL),
    m_bPalettized(false), m_ulPaletteSize(0)
{ }
///

/// SimpleBmp::SimpleBmp
// copy constructor: Use another image data, used to save the image
// in here.
SimpleBmp::SimpleBmp(const class ImageLayout &il)
  : ImageLayout(il), m_pucImage(NULL), m_puqRed(NULL), m_puqGreen(NULL), m_puqBlue(NULL), m_pucOutbuf(NULL)
{ }
///

/// SimpleBmp::~SimpleBmp 
// destructor
SimpleBmp::~SimpleBmp(void) 
{
  // dispose the image in memory
  delete[] m_pucImage;
  delete[] m_puqRed;
  delete[] m_puqGreen;
  delete[] m_puqBlue;
  delete[] m_pucOutbuf;
}
///

/// GetULONG
// Fetch a little endian ULONG from the stream
static ULONG getULONG(File &fp)
{
  int i1,i2,i3,i4;

  i1 = getc(fp);
  i2 = getc(fp);
  i3 = getc(fp);
  i4 = getc(fp);

  return (i1 << 0) | (i2 << 8) | (i3 << 16) | (i4 << 24);
}
///

/// SimpleBmp::LoadImage
// Load a simple image from a BMP stream.
void SimpleBmp::LoadImage(const char *basename,struct ImgSpecs &specs)
{
  ULONG padw = 0;     // padded width
  ULONG j,k,ix,iy;    // loopcounters
  WORD bpad;  
  UBYTE *tmpptr = NULL; // temporary pointer (shutup g++)
  UWORD bitcount;
  UBYTE bitshift = 0;
  ULONG compression;  // compression mode
  ULONG offsetbits;
  ULONG typesize;
  ULONG clrused;
  UWORD planes;
  ULONG rmask = 0x1f << 10;
  ULONG gmask = 0x1f <<  5;
  ULONG bmask = 0x1f <<  0;
  UBYTE rshift = 10;
  UBYTE gshift =  5;
  UBYTE bshift =  0;
  LONG w,h;
  File fp(basename,"rb");
  struct BMPHeader bmh; // bmh gets feed into here.
  //
  if (m_pComponent) {
    PostError("Image is already loaded.\n");
  }
  //
  // read the BMP header now.
  //
  if (fread(&bmh,sizeof(bmh),1,fp) != 1) {
    PostError("Could not read the image header.\n");
  }
  //
  // check if file type is Windows Bitmap
  if ((bmh.bmh_id0 != 'B' || bmh.bmh_id1 != 'M') &&
      (bmh.bmh_id0 != 'B' || bmh.bmh_id1 != 'A') &&
      (bmh.bmh_id0 != 'C' || bmh.bmh_id1 != 'I') &&
      (bmh.bmh_id0 != 'C' || bmh.bmh_id1 != 'P') &&
      (bmh.bmh_id0 != 'I' || bmh.bmh_id1 != 'C') &&
      (bmh.bmh_id0 != 'P' || bmh.bmh_id1 != 'T')) {
    PostError("Image is not a valid BMP stream.\n");
  }
  //
  // get the buffer offset bits. Convert from LE to native
  // This is the offset from the start of the file to the 
  // start of the raster.
  offsetbits = GetULONG(bmh.bmh_OffsetBits);
  // get the type/size
  typesize   = GetULONG(bmh.bmh_TypeSize);
  //
  // check for old or new bitmap format
  if (typesize >= NEW_WINDOWS_BITMAP) {
    struct NewImageData nid;
    // Read it in one go.
    if (fread(&nid,sizeof(nid),1,fp) != 1) {
      PostError("Invalid BMP file, could not read the bitmap header.\n");
    }
    //
    // Obtain width, height and others from the nid.
    w                 = (LONG)GetULONG(nid.nid_Width);
    h                 = (LONG)GetULONG(nid.nid_Height);
    planes            = GetUWORD(nid.nid_Planes);
    bitcount          = GetUWORD(nid.nid_BitCount);
    compression       = GetULONG(nid.nid_Compress);
    clrused           = GetULONG(nid.nid_ClrU);
    //
    // If this is the Os/2 format, we do not support Huffman coding.
    if (typesize == NEW_OS2_BITMAP) {
      // Compression is encoded in a different way.
      if (compression == 3)
	PostError("Invalid BMP file, Huffman compressed Os/2 bitplanes not supported.");
    }
  } else {    //old bitmap format 
    struct OldImageData oid;
    // Read it in one go
    if (fread(&oid,sizeof(oid),1,fp) != 1) {
      PostError("Invalid BMP file, could not read the bitmap header.\n");
    }
    // Get width, height, etc...
    // Data in w,h is actually signed.
    w                 = (LONG)WORD(GetUWORD(oid.oid_Width));
    h                 = (LONG)WORD(GetUWORD(oid.oid_Height));
    planes            = GetUWORD(oid.oid_Planes);
    bitcount          = GetUWORD(oid.oid_BitCount);
    clrused           = 0;
    // set to non compression
    compression       = BI_RGB; 
  }
  // only color bitmaps: This must be reconsidered later...
  m_usDepth           = 3;
  //
  // check for valid width and height.
  if (w<=0 || w>(1<<15) || h>(1<<15) || (-h)>(1<<15) || h==0) {
    PostError("Image dimensions are invalid.\n");
  }
  // 
  // Note that the height might be negative.
  if (h<0)  m_ulHeight = ULONG(-h);
  else      m_ulHeight = ULONG(h);
  m_ulWidth            = ULONG(w);
  // 
  // error checking for valid bitcounts. We only support
  // grey scale and 24 bit images.
  switch(bitcount) {
  case 1:
  case 2:
  case 4:
  case 8:
  case 16:
  case 24:
  case 32:
    break;
  default:
    PostError("Bit depth is unsupported.\n");
  }
  //
  // check for valid planes
  if (planes != 1) {
    PostError("Unsupported number of planes.\n");
  }
  //
  // check for valid compression modes: We currently don't
  // support run-length coding *sigh*
  if (compression >  BI_BITFIELDS) { // Huffman, PNG or JPEG compression are not supported here.
    PostError("Unsupported BMP compression type.\n");
  }
  //
  // Compression mode and bitcount must fit.
  if (bitcount==24 && compression != BI_RGB) {
    PostError("Unsupported compression for 24 bpp bitcount.\n");
  }
  if ((bitcount==16 || bitcount == 32) && (compression != BI_RGB && compression != BI_BITFIELDS)) {
    PostError("Unsupported compression for 16/32 bpp bitcount.\n");
  }
  if (bitcount == 16) {
    rshift = 10;
    gshift = 5;
    bshift = 0;
    rmask  = 0x1f << 10;
    gmask  = 0x1f <<  5;
    bmask  = 0x1f <<  0;
  } else if (bitcount == 24) {
    rshift = 16;
    gshift = 8;
    bshift = 0;
    rmask = 0xff << 16;
    gmask = 0xff <<  8;
    bmask = 0xff <<  0;
  }
  //
  if (typesize != OLD_OS2_BITMAP) {
    ULONG read = sizeof(struct NewImageData) + sizeof(ULONG);
    // skip ahead to colormap, using BmpTypeSize
    // 40 bytes read from BmpTypeSize to ClrImportant 
    // Note that in our setup, the size is part of the
    // bitmap header, not part of the BITMAPINFOHEADER (nid,oid).
    if (typesize < read) 
      PostError("Invalid BMP file - offset is out of range");
    j = typesize - read;
    while(j>0) {
      getc(fp);
      j--;
    }
    // Compute the number of offsetbits we had to skip to come from
    // here to the raster. We read now "typesize" + sizeof(bmh) - 4
    // bytes.
    read = typesize + sizeof(bmh) - sizeof(ULONG);
    if (offsetbits < read)
      PostError("Invalid BMP file - offset is out of range");
    bpad = offsetbits - read;
  } else {
    bpad = 0;
  }
  // bpad must now be the offset from here to the start of the
  // raster.
  // load up colormap, if any exists and we need to care of it.
  if (bitcount < 16) {
    bool isgrey = true; // set if found as a grey-scale image.
    // calcuate the number of colors. If clrused is set, it contains the number of
    // colors.
    ULONG col = (clrused) ? clrused : (1 << bitcount);
    // create memory
    m_puqRed   = new UQUAD [col];
    m_puqGreen = new UQUAD [col];
    m_puqBlue  = new UQUAD [col];
    //
    for (k=0; k < col; k++) {
      int r,g,b; // the palette entries
      b = getc(fp), g = getc(fp), r = getc(fp);
      // Check for EOF now. The BMP is invalid in case
      // we abort in the middle of the palette.
      if (r < 0) {
	PostError("BMP palette is invalid.\n");
      }
      //
      // Check whether all colors are equal. If so, we have a
      // grey scale image (urgh), otherwise we have a color
      // image.
      if (r != b && b != g) {
	isgrey          = false;
	m_bPalettized   = true;
	m_ulPaletteSize = col;
      }
      //
      // Insert now the data into the palette entries.
      m_puqRed[k]   = r;
      m_puqGreen[k] = g;
      m_puqBlue[k]  = b;
      // Skip bytes for alpha channel
      if (typesize != OLD_OS2_BITMAP) {
	getc(fp);
	if (bpad < 4)
	  PostError("Invalid BMP file - offset is out of range");
	bpad -= 4; // bpad exists only for non-OS2 bitmaps.
      }
    }
    // Reconsider the image color
    if (isgrey)
      m_usDepth = 1;
  }
  //
  specs.ASCII      = ImgSpecs::No;
  specs.Palettized = m_bPalettized?(ImgSpecs::Yes):(ImgSpecs::No);
  specs.YUVEncoded = ImgSpecs::No;
  //
  if (compression == BI_BITFIELDS) {
    ULONG mask;
    //
    if (bpad < 3 * 4)
      PostError("BMP file invalid, missing bitfield masks");
    if (bitcount <= 8)
      PostError("BMP file invalid, bitfield compression requires more than 8 bits per pixel");
    bpad -= 12;
    rmask = getULONG(fp);
    gmask = getULONG(fp);
    bmask = getULONG(fp);
    //
    for(mask = rmask,rshift = 0;(mask & 0x01) == 0;rshift++,mask >>= 1) {}
    for(mask = gmask,gshift = 0;(mask & 0x01) == 0;gshift++,mask >>= 1) {}
    for(mask = bmask,bshift = 0;(mask & 0x01) == 0;bshift++,mask >>= 1) {}
  }
  if (typesize != OLD_OS2_BITMAP) {
    // drop any unused bytes between the color map (if present)
    // and the start of the actual bitmap data.    
    while (bpad > 0) {
      getc(fp);
      bpad--;
    }
  }
  //
  // create memory for the data 
  m_pucImage         = new UBYTE [m_ulWidth*m_ulHeight*m_usDepth];
  // Create the image layout.
  CreateComponents(m_ulWidth,m_ulHeight,m_usDepth);
  //
  // The defaults are already fine, except that we now need to define
  // the image data pointers.
  for(k = 0; k < m_usDepth; k++) {
    m_pComponent[k].m_pPtr            = m_pucImage + k;
    m_pComponent[k].m_ulBytesPerRow   = m_ulWidth * m_usDepth;
    m_pComponent[k].m_ulBytesPerPixel = m_usDepth;
    switch(bitcount) {
    case 32:
    case 24:
      break; // This is true-color
    case 16:
      m_pComponent[k].m_ucBits = 5; // this is RGB packed into 16 bit
      break;
    case 8:
    case 4:
    case 2:
    case 1:
      m_pComponent[k].m_ucBits = bitcount; // this is n-level bitcount
      break;
    default:
      PostError("Invalid bmp bitcount detected");
      break;
    }
  }
  // Compute the number of elements packed into
  // one byte. 2^bitshift is this number.
  if (bitcount <= 8) {
    k = bitcount;
    bitshift = 0;
    while(k < 8) {
      bitshift++;
      k <<= 1;
    }
  }
  if (compression == BI_BITFIELDS) {
    int bits,mask;
    for(mask = rmask,bits = 0;mask != 0;mask >>= 1) if (mask & 1) bits++;
    m_pComponent[0].m_ucBits = bits;
    for(mask = gmask,bits = 0;mask != 0;mask >>= 1) if (mask & 1) bits++;
    m_pComponent[1].m_ucBits = bits;
    for(mask = bmask,bits = 0;mask != 0;mask >>= 1) if (mask & 1) bits++;
    m_pComponent[2].m_ucBits = bits;
  }
  //
  // 8 - bit Windows Bitmaps
  if (bitcount <= 8) {
    if (compression == BI_RGB) {   //read uncompressed data 
      ULONG w = (m_ulWidth + (1 << bitshift) - 1) >> bitshift; // bytes per row really required.
      padw    = ((w + 3) & (~3)) - w; // pad bytes
      for (iy = 0; iy < m_ulHeight; iy++) {
	int freebits   = 0;
	int pixel,value = 0;
	// bitmaps are oriented upside down unless noted otherwise.
	if (h>0)
	  tmpptr = m_pucImage + ((m_ulHeight-iy-1) * m_ulWidth * m_usDepth);
	else
	  tmpptr = m_pucImage + iy * m_ulWidth * m_usDepth;
	//
	for (ix=0; ix<m_ulWidth; ix++) {
	  //
	  // Get the pixel value. Fail on EOF
	  if (freebits == 0) {
	    value    = getc(fp);
	    if (value < 0)
	      PostError("Unexpected EOF in the BMP file.\n");
	    freebits = 8;
	  }
	  freebits  -= bitcount;
	  pixel      = (value >> freebits) & ((1 << bitcount) - 1);
	  //
	  // Insert into the array, depending on whether we have
	  // color or not.
	  if (m_usDepth == 3) {
	    // Color. Insert RGB in this order.
	    *tmpptr++      = m_puqRed[pixel];
	    *tmpptr++      = m_puqGreen[pixel];
	    *tmpptr++      = m_puqBlue[pixel];
	  } else {
	    // If there are less than 8 bits per pixel, go not through the palette
	    // since the image is not supposed to be displayed but rather analyzed.
	    if (bitcount >= 8) {
	      // grey. We could use any palette entry here because
	      // they are all equal.
	      *tmpptr++      = m_puqGreen[pixel];
	    } else {
	      *tmpptr++      = pixel;
	    }
	  }
	}
	// Now skip the padding.
	for(k=0;k<padw;k++)
	  getc(fp);
      }
    } else if (compression == BI_RLE8 || compression == BI_RLE4) {
      int count = 0,pixel = 0,value = 0; // shut up g++.
      bool done = false;
      bool calc = true;
      bool absm = false;
      bool padb = false;
      bool lown = false;
      if (compression == BI_RLE8) {
	padw   = (m_ulWidth + 3) & (~3);
	if (bitcount != 8)
	  PostError("found RLE8 compression in bmp file, but bits per pixel != 8");
      } else {
	padw   = (m_ulWidth + 1) >> 1;
	padw   = (padw + 3) & (~3);
	padw <<= 1;
	if (bitcount != 4)
	  PostError("found RLE4 compression in bmp file, but bits per pixel != 4");
       }
      ix = 0;iy = 0;
      while(!done) {
	// Recompute the current pixel address?
	if (calc) {
	  if (h>0)
	    tmpptr = m_pucImage + ((m_ulHeight-iy-1) * m_ulWidth * m_usDepth);
	  else
	    tmpptr = m_pucImage + iy * m_ulWidth * m_usDepth;
	  tmpptr  += ix;
	  calc = false;
	}
	// Now get the next action token if the last run is done
	if (count <= 0) {
	  // In case we need padding, do now
	  if (padb) {
	    getc(fp);
	    padb = false;
	  }
	  // Re-read count/pixel value for the run
	  count = getc(fp);
	  value = getc(fp);
	  lown  = false;
	  // Check for EOF
	  if (value < 0) {
	    PostError("Unexpected EOF in the BMP stream.\n");
	  }
	  //
	  // Check for escape symbol
	  if (count == 0) {
	    switch(value) {
	    case 0:
	      {
		// Abort the current line (unless we aborted it
		// anyhow before).
		if (ix) {
		  ix   = 0;
		  iy++;
		  if (iy >= m_ulHeight)
		    done = true;
		  // recompute row address
		  calc = true;
		}
		continue;
	      }
	    case 1:
	      {
		// Abort the frame completely
		done = true;
		continue;
	      }
	    case 2:
	      {
		int deltax,deltay;
		// continue frame at indicated position
		deltax = getc(fp);
		deltay = getc(fp);
		if (deltay < 0) {
		  PostError("Unsupported EOF in the BMP stream.\n");
		}
		ix    += deltax;
		iy    += deltay;
		calc   = true;
		continue;
	      }
	    default:
	      // absolute mode enganged
	      count = value;
	      absm  = true;
	      // Check for padding
	      if (compression == BI_RLE8) {
		if (count & 1)
		  padb = true;
	      } else {
		if (((count + 1) >> 1) & 1)
		  padb = true;
	      }
	    }
	  } else {
	    // else of "escape symbol"
	    // disable absolute mode
	    absm    = false;
	  }
	}
	//
	// Ok, now check whether we are in absolute mode. If
	// so, we must fetch a new pixel value.
	if (absm) {
	  if (compression == BI_RLE8 || lown == false) {
	    value = getc(fp);
	  }
	  if (value < 0) {
	    PostError("Unsupported EOF in the BMP stream.\n");
	  }
	}
	if (compression == BI_RLE8) {
	  pixel = value;
	} else {
	  if (lown) {
	    pixel = value & 0x0f;
	    lown  = false;
	  } else {
	    pixel = value >> 4;
	    lown  = true;
	  }
	}
	// Otherwise, we know the pixel value already
	// Insert into the array, depending on whether we have
	// color or not.
	if (ix < m_ulWidth) {
	  if (m_usDepth == 3) {
	    // color. Insert RGB in this order.
	    *tmpptr++    = m_puqRed[pixel];
	    *tmpptr++    = m_puqGreen[pixel];
	    *tmpptr++    = m_puqBlue[pixel];
	  } else {
	    // If there are less than 8 bits per pixel, go not through the palette
	    // since the image is not supposed to be displayed but rather analyzed.
	    if (bitcount >= 8) { 
	      // grey. We could use any palette entry here because
	      // they are all equal.
	      *tmpptr++  = m_puqGreen[pixel];
	    } else {
	      *tmpptr++  = pixel;
	    }
	  }
	}
	// Decrement count, increment write position.
	count--;
	ix++;
	if (ix >= padw) {
	  ix = 0;
	  calc = true; // recompute pointer.
	  iy++;
	  if (iy >= m_ulHeight)
	    done = true; // abort
	}
      }
    } else {
      PostError("Invalid BMP file, unsupported compression type");
    }
  } else if (compression == BI_RGB) {
    // RGB 16/24/32 bit images
    // calculate the padding
    ULONG w = (m_ulWidth * bitcount + 7) >> 3;
    padw    = ((w + 3) & (~3)) - w;
    for (iy=0; iy<m_ulHeight; iy++) {
      if (h>0)
	tmpptr = m_pucImage + ((m_ulHeight-iy-1) * m_ulWidth * m_usDepth);
      else
	tmpptr = m_pucImage + iy * m_ulWidth * m_usDepth;
      //
      // data arrives as BGR, we keep it as RGB.
      for (ix = 0; ix < m_ulWidth; ix++) {
	int r,g,b;
	int b1 = 0,b2 = 0,b3 = 0,b4 = 0;
	ULONG v;
	b1 = getc(fp);
	b2 = getc(fp);
	if (bitcount > 16)
	  b3 = getc(fp);
	if (bitcount > 24)
	  b4 = getc(fp);
	if (feof(fp) || ferror(fp)) {
	  PostError("Unexpected EOF in the BMP stream.\n");
	}
	v   = (b1 << 0) | (b2 << 8) | (b3 << 16) | (b4 << 24);
	r   = (v & rmask) >> rshift;
	g   = (v & gmask) >> gshift;
	b   = (v & bmask) >> bshift;
	//
	// Insert the data now.
	*tmpptr++      = r;
	*tmpptr++      = g;
	*tmpptr++      = b;
      }
      // skip padding bytes.
      for (k=0; k<padw; k++) 
	getc(fp);
    }
  } else {
    PostError("Invalid BMP file, unsupported compression type");
  }
  // Final check: Check for an error in the stream or an
  // premature EOF. We should now be at the EOF, not beyond it.
  if (feof(fp) || ferror(fp)) {
    PostError("Error or premature EOF in the BMP stream.\n");
  }
}
///

/// SimpleBmp::SaveImage
// NOTE: This saves only eight bit images
// as BMP doesn't cover 16 bit images.
void SimpleBmp::SaveImage(const char *basename,const struct ImgSpecs &)
{
  ULONG ix, iy, k ;      // loopcounters
  UBYTE *tmpptr;         // temporary pointer
  ULONG offsetbits;
  ULONG buffersize;
  UWORD bitcount;
  UBYTE bitshift = 0;
  UWORD padw   = 0;      // # of padding bytes
  ULONG rmask  = 0,gmask  = 0,bmask  = 0;
  UBYTE rshift = 0,bshift = 0,gshift = 0;
  int  bitcnt = 0;
  bool bitfields = false;
  struct BMPHeader bmh;    // header (to be filled out)
  struct NewImageData nid; // we always write new BMPs
  File fp(basename,"wb");
  //
  // We must either have RGB or grey scale data. Everything else is
  // unsupported in here.
  if (m_usDepth != 1 && m_usDepth != 3) {
    PostError("BMP supports only one or three component images.\n");
  }
  if (m_pComponent == NULL) {
    PostError("No image loaded to save.\n");
  }
  //
  // Check whether the component layout is BMP encodable.
  for(k = 0; k < m_usDepth; k++) { 
    if (m_pComponent[k].m_bFloat) {
      PostError("Unable to save BMP files for floating point data.\n");
    } 
    bitcnt += m_pComponent[k].m_ucBits;
    if (m_pComponent[k].m_ucBits != m_pComponent[0].m_ucBits) {
      bitfields = true;
    }
    if (m_pComponent[k].m_bSigned != false) {
      PostError("BMP does not support signed image data.\n");
    }
    if (m_pComponent[k].m_ucSubX  != 1) {
      PostError("BMP does not support subsampling.\n");
    }
    if (m_pComponent[k].m_ucSubY  != 1) {
      PostError("BMP does not support subsampling.\n");
    }
  }
  if (m_usDepth == 3 && (bitcnt != 24 && bitcnt != 15))
    bitfields = true;
  if (bitfields) {
    if (bitcnt > 32)
      PostError("Cannot store more than 32 bits per pixel in total.\n");
    if (m_usDepth != 3) // Huh??
      PostError("Requires RGB images for bitfield storage.\n");
  }
  //
  // Insert the magic cookies
  bmh.bmh_id0       = 'B';
  bmh.bmh_id1       = 'M';
  //
  // The following is required within the BMP header
  // compute the number of offsetbits: Bytes from the
  // start of the file to the body.
  offsetbits        = sizeof(bmh) + sizeof(nid);
  if (m_usDepth == 1) {
    bitcount = m_pComponent[0].m_ucBits;
    switch(bitcount) {
    case 1:
      bitshift = 3;
      break;
    case 2:
      bitshift = 2; // unusual.
      break;
    case 4:
      bitshift = 1;
      break;
    case 8:
      bitshift = 0; // this is the regular case.
      break;
    default:
      PostError("BMP requires 1,2,4, or 8 bits per sample for grey-scale images");
      break;
    }
  } else {
    if (bitfields) {
      if (bitcnt > 16) {
	bitcount = 32;
      } else {
	bitcount = 16;
      }
      bmask  = (1UL << m_pComponent[0].m_ucBits) - 1;
      bshift = 0;
      gmask  = ((1UL << m_pComponent[1].m_ucBits) - 1) << m_pComponent[0].m_ucBits;
      gshift = m_pComponent[0].m_ucBits;
      rmask  = ((1UL << m_pComponent[2].m_ucBits) - 1) << (m_pComponent[0].m_ucBits + m_pComponent[1].m_ucBits);
      rshift = m_pComponent[1].m_ucBits + m_pComponent[0].m_ucBits;
    } else {
      if (m_pComponent[0].m_ucBits == 5) {
	bitcount = 16;
      } else {
	bitcount = 24;
      }
    }
  }
  // If we have a palette, we must include it in the count.
  if (m_usDepth == 1) {
    offsetbits     += 4 * (1 << bitcount); // the color table, 4 bytes each
  }
  if (bitfields)
    offsetbits     += 3 * 4;   // for the fields.
  // Compute the padding now. Rows are always padded to entire
  // rows of ULONGs
  buffersize  = (3 + ((m_ulWidth * bitcnt + 7) >> 3)) & -4; // Bytes in total, round up to four bytes.
  buffersize *= m_ulHeight;
  SetULONG(bmh.bmh_BufSize,buffersize + offsetbits);
  SetUWORD(bmh.bmh_res1,0);
  SetUWORD(bmh.bmh_res2,0);
  SetULONG(bmh.bmh_OffsetBits,offsetbits);
  //
  // The nid does not include its size in our convention
  SetULONG(bmh.bmh_TypeSize,sizeof(nid) + sizeof(ULONG));
  // Now fill out the nid.
  SetULONG(nid.nid_Width,m_ulWidth);
  SetULONG(nid.nid_Height,m_ulHeight);
  SetUWORD(nid.nid_Planes,1); // always one plane
  SetUWORD(nid.nid_BitCount,bitcount); // bits per pixel.
  SetULONG(nid.nid_Compress,(bitfields)?(BI_BITFIELDS):(BI_RGB)); // no compression
  SetULONG(nid.nid_SizeImage,buffersize); // allowed to be zero for no compression
  // XPelsPerMeter, YPelsPerMeter would be available in the
  // JP2 File Format, but this simple interface doesn't provide
  // access to it.
  SetULONG(nid.nid_XPPM,0 /*2835*/);
  SetULONG(nid.nid_YPPM,0 /*2835*/); // dots per meter. Approximately 72 dpi.
  // No unused palette entries for true color.
  SetULONG(nid.nid_ClrU,0); // everything used
  SetULONG(nid.nid_Imp,0);  // everything important.
  //
  // Now write both, the bmp header and the new image data
  if (fwrite(&bmh,sizeof(bmh),1,fp) != 1) {
    PostError("Could not write BMP header.\n");
  }
  //
  if (fwrite(&nid,sizeof(nid),1,fp) != 1) {
    PostError("Could not write BMP header.\n");
  }
  //
  // FIXME: Need to adjust pad bits for palette.
  // FIXME: Need to understand padding
  if (bitfields) {
    // Need to write out the bitmasks for red,green,blue.
    putc(rmask >>  0,fp);
    putc(rmask >>  8,fp);
    putc(rmask >> 16,fp);
    putc(rmask >> 24,fp);
    
    putc(gmask >>  0,fp);
    putc(gmask >>  8,fp);
    putc(gmask >> 16,fp);
    putc(gmask >> 24,fp);
    
    putc(bmask >>  0,fp);
    putc(bmask >>  8,fp);
    putc(bmask >> 16,fp);
    putc(bmask >> 24,fp);
  }
  //
  // As we have to write grey-scale images as palette images,
  // write a dummy palette now.
  if (bitcount <= 8) {
    ULONG entries = 1U << bitcount;
    UBYTE factor  = 255 / (entries - 1);
    for(k = 0;k < entries;k++) {
      putc(k * factor,fp); // B
      putc(k * factor,fp); // G
      putc(k * factor,fp); // R: all the same for grey scale
      putc(0         ,fp); // spare for NEWIMAGE, or alpha.
    }
    // Check for error conditions.
    if (ferror(fp)) {
      PostError("IO error writing BMP image.\n");
    }
    // Write grey-scale data
    ULONG w = (m_ulWidth + (1 << bitshift) - 1) >> bitshift; // Bytes in total
    padw    = ((w + 3) & (~3)) - w; // unused padding bytes, always multiplies of four bytes.
    for (iy = 0; iy < m_ulHeight; iy++) {
      UBYTE usedbits  = 0;
      UBYTE bitbuffer = 0;
      // bitmaps are oriented upside down.
      tmpptr = ((UBYTE *)(m_pComponent[0].m_pPtr)) + ((m_ulHeight - iy - 1) * m_pComponent[0].m_ulBytesPerRow);
      for (ix = 0; ix<m_ulWidth; ix++) {
	// Just write the image data plain to the file.
	usedbits  += bitcount;
	bitbuffer |= *tmpptr << (8 - usedbits);
	if (usedbits >= 8) {
	  putc(bitbuffer,fp);
	  bitbuffer = 0;
	  usedbits  = 0;
	  if (ferror(fp)) {
	    PostError("IO error writing BMP image.\n");
	  }
	}
	tmpptr += m_pComponent[0].m_ulBytesPerPixel;
      }
      //
      if (usedbits > 0) {
	putc(bitbuffer,fp);
	if (ferror(fp)) {
	  PostError("IO error writing BMP image.\n");
	}
      }
      //
      // Write padding
      for(k=0;k<padw;k++)
	putc(0,fp);
    }
  } else {
    // write color image
    switch(bitcount) {
    case 32:
      padw   = ((m_ulWidth * 4 + 3) & (~3)) - m_ulWidth * 4; // actually, always zero
      break;
    case 24:
      padw   = ((m_ulWidth * m_usDepth + 3) & (~3)) - m_ulWidth * m_usDepth;
      break;
    case 16:
      padw   = ((m_ulWidth * 2 + 3) & (~3)) - m_ulWidth * 2;
      break;
    default:
      PostError("Unsupported bit count for BMP image");
      break;
    }
    //
    // Generate an output buffer.
    delete[] m_pucOutbuf;
    m_pucOutbuf = NULL;
    m_pucOutbuf = new UBYTE[padw + m_ulWidth * 4];
    // Clear pad-bytes to siled valgrind.
    memset(m_pucOutbuf, 0 ,padw + m_ulWidth * 4);
    //
    for (iy=0; iy < m_ulHeight; iy++) {
      UBYTE *t0,*t1,*t2,*o;
      t0 = ((UBYTE *)m_pComponent[0].m_pPtr) + ((m_ulHeight - iy - 1) * m_pComponent[0].m_ulBytesPerRow);
      t1 = ((UBYTE *)m_pComponent[1].m_pPtr) + ((m_ulHeight - iy - 1) * m_pComponent[1].m_ulBytesPerRow);
      t2 = ((UBYTE *)m_pComponent[2].m_pPtr) + ((m_ulHeight - iy - 1) * m_pComponent[2].m_ulBytesPerRow);
      o  = m_pucOutbuf;
      //
      // data has to be written as BGR, we keep it as RGB.
      for (ix = 0; ix < m_ulWidth; ix++) {
	if (bitfields) {
	  ULONG v = (*t0 << rshift) | (*t1 << gshift) | (*t2 << bshift);
	  if (bitcount == 16) {
	    *o++ = v & 0xff;
	    *o++ = v >> 8;
	  } else {
	    *o++ = v & 0xff;
	    *o++ = v >> 8;
	    *o++ = v >> 16;
	    *o++ = v >> 24;
	  }
	} else {
	  if (bitcount == 24) {
	    *o++ = *t2; // B
	    *o++ = *t1; // G
	    *o++ = *t0; // R
	  } else {
	    UWORD v = (*t0 << 10) | (*t1 << 5) | (*t2 << 0);
	    *o++    = v & 0xff;
	    *o++    = v >> 8;
	  }
	}
	//
	// Advance pointers.
	t0 += m_pComponent[0].m_ulBytesPerPixel;
	t1 += m_pComponent[1].m_ulBytesPerPixel;
	t2 += m_pComponent[2].m_ulBytesPerPixel;
      }	
      // Write output buffer now, includes padding.
      fwrite(m_pucOutbuf,1,padw + (o - m_pucOutbuf),fp);
      if (ferror(fp)) {
	PostError("IO error writing the BMP file.\n");
      }
    }
    delete[] m_pucOutbuf;
    m_pucOutbuf = NULL;
  }
  //
  // Now check for an error and return to the caller.
  if (ferror(fp)) {
    PostError("IO error writing the BMP file.\n");
  }
}
///
