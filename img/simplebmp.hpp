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
 * An image class to load and save uncompressed BMP images
 * from a file or to a file.
 * $Id: simplebmp.hpp,v 1.10 2017/01/31 11:58:04 thor Exp $
 */

#ifndef SIMPLEBMP_HPP
#define SIMPLEBMP_HPP

/// Includes
#include "imglayout.hpp"
#include "std/stdio.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// SimpleBmp
// This is the class for Windows 8 and 24 bit Bitmaps
class SimpleBmp : public ImageLayout {
  // This is the image raw data
  // the data is in order {r,g,b} for colored pictures. 
  UBYTE *m_pucImage;
  //
  // Palette of the image, if any. Using UQUADs here seems
  // wasteful, but this is exactly the type the palette box
  // expects, so we won't need to copy things over.
  UQUAD *m_puqRed;
  UQUAD *m_puqGreen;
  UQUAD *m_puqBlue;
  //
  // Output buffer
  UBYTE *m_pucOutbuf;
  //
  // A boolean indicator whether this is a palettized image (likely).
  // Note that gray-scale images are also palette images in the
  // sense of BMP. 
  bool   m_bPalettized;
  //
  // Number of entries in the palette.
  ULONG  m_ulPaletteSize;
  //
  // The following structure is used to keep a BMP file header
  // Windows calls this a BITMAPFILEHEADER
  struct BMPHeader {
    UBYTE  bmh_id0,bmh_id1;   // must be B and M, respectively
    UBYTE  bmh_BufSize[4];    // buffer size = file size, in 
    // Little Endian. Not that we need it....
    UBYTE  bmh_res1[2];
    UBYTE  bmh_res2[2]; // currently not used
    UBYTE  bmh_OffsetBits[4];  // ULONG LE offset
    // The following is actually part of the nid or oid...
    // We keep it here for consistency.
    UBYTE  bmh_TypeSize[4];    // BMP encoding scheme
  };
  // new BMP header, RGB 24 color images
  struct NewImageData {
    UBYTE  nid_Width[4];   // width, little endian
    UBYTE  nid_Height[4];  // height, little endian
    UBYTE  nid_Planes[2];  // # of planes. Must be one
    UBYTE  nid_BitCount[2];
    UBYTE  nid_Compress[4];  // compression type
    UBYTE  nid_SizeImage[4]; // image size
    UBYTE  nid_XPPM[4];      // X pels per meter
    UBYTE  nid_YPPM[4];      // Y pels per meter
    UBYTE  nid_ClrU[4];      // number of colors in the palette, or zero if the palette is filled.
    UBYTE  nid_Imp[4];       // number of "important" colors.
  };
  // old BMP header, grey scale, palette images
  struct OldImageData {
    UBYTE  oid_Width[2];
    UBYTE  oid_Height[2];
    UBYTE  oid_Planes[2];   // # of planes. Must be one.
    UBYTE  oid_BitCount[2]; // bit count. Must be one.
  };
  //
  // Unpack little endian words from a byte array. Note that
  // we want to be endian-independent
  static ULONG GetULONG(UBYTE *a)
  {
    return (a[0]) | (a[1]<<8) | (a[2]<<16) | (a[3]<<24);
  }
  static UWORD GetUWORD(UBYTE *a)
  {
    return (a[0]) | (a[1]<<8);
  }
  //
  // Little endian packing service follows
  static void SetULONG(UBYTE *a,ULONG v)
  {
    a[0] = UBYTE((v >>  0) & 0xff);
    a[1] = UBYTE((v >>  8) & 0xff);
    a[2] = UBYTE((v >> 16) & 0xff);
    a[3] = UBYTE((v >> 24) & 0xff);
  }
  static void SetUWORD(UBYTE *a,UWORD v)
  {
    a[0] = UBYTE((v >>  0) & 0xff);
    a[1] = UBYTE((v >>  8) & 0xff);
  }
  //
  //
public:
  //
  // default constructor
  SimpleBmp(void);
  // Copy the image from another source for later saving.
  SimpleBmp(const class ImageLayout &layout);
  // destructor
  ~SimpleBmp(void);
  // 
  // Returns an indicator whether this is a palette image. This only
  // works right now for BMP, even though J2K supports this, too.
  virtual bool IsPalettized(void) const
  {
    return m_bPalettized;
  }
  //
  // Return the size of the palette table, in entries.
  virtual ULONG PaletteSize(void) const
  {
    return m_ulPaletteSize;
  }
  //
  // Return the pointers to the palette table of the n-th channel.
  virtual const UQUAD *PaletteTableOf(int channel) const
  {
    switch(channel) {
    case 0:
      return m_puqRed;
    case 1:
      return m_puqGreen;
    case 2:
      return m_puqBlue;
    }
    return NULL;
  }
  //
  // Save an image to a level 1 file descriptor, given its
  // width, height and depth. We only support grey level and
  // RGB here, no palette images.
  void SaveImage(const char *basename,const struct ImgSpecs &specs);
  //
  // Load an image from a level 1 file descriptor, keep it within
  // the internals of this class. The accessor methods below
  // should be used to find out more about this image.
  void LoadImage(const char *basename,struct ImgSpecs &specs);
};
///

///
#endif
