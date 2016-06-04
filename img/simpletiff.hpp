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
** An image class to load and save TIFF images (or some of them).
**
** $Id: simpletiff.hpp,v 1.21 2016/06/04 10:44:10 thor Exp $
*/

#ifndef SIMPLETIFF_HPP
#define SIMPLETIFF_HPP

/// Includes
#include "imglayout.hpp"
#include "std/string.hpp"
#include "std/stdio.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// SimpleTiff
// This is the class for simple portable extended pixmap graphics.
class SimpleTiff : public ImageLayout {
  //
  // A per-component buffer containing the component names.
  struct TiffComponent {
    // Dimensions of the component.
    ULONG                 m_ulWidth;
    ULONG                 m_ulHeight;
    UBYTE                 m_ucDepth;
    bool                  m_bSigned;
    bool                  m_bFloat;
    //
    // The memory. This really administrates it.
    UBYTE                *m_pData;
    //
    TiffComponent(void)
      : m_ulWidth(0), m_ulHeight(0), m_ucDepth(0),
	m_bSigned(false), m_bFloat(false), m_pData(NULL)
    {
    }
    ~TiffComponent(void)
    {
      delete[] m_pData;
    }
  }     **m_ppComponents;
  //
  // Number of entries here, required to release them properly.
  UWORD m_usCount;
  //
  // Copy data for TIFF images written in "striped mode".
  void ReadStriped(class TiffParser &parser,int lzw,bool hdiff,UWORD imgconfig,
		   ULONG inv,const ULONG *bits,const ULONG *fmt,
		   const ULONG *r,const ULONG *g,const ULONG *b,
		   DOUBLE scale);
  //
  // Read tiled data through the tiff interface.
  void ReadTiled(class TiffParser &parser,int lzw,bool hdiff,UWORD imgconfig,ULONG tw,ULONG th,
		 ULONG inv,const ULONG *bits,const ULONG *fmt,
		 const ULONG *r,const ULONG *g,const ULONG *b,
		 DOUBLE scale);

  // Unpack the data from the source buffer into the destination component.
  // The parser is the data source, comp the start component and cnt the number of
  // components to copy. xofs and y are offsets into the target plane where the data 
  // is to be copied to.
  // width and height are the dimensions of the rectangle to be copied.
  // b is the number of bits per component if constant, or zero if the bit depths varies.
  // bits[] is the bit depth arrary, fmt[] is the sample format array.
  // bytes is the number of bytes in the buffer. "inv" an inversion mask for white on
  // black images.
  template<class Decoder>
  void UnpackData(UBYTE *buffer,bool bigendian,bool prediction,
		  UWORD comp,UWORD cnt,ULONG xofs,ULONG y,
		  ULONG width,ULONG height,
		  UBYTE b,const ULONG *bits,const ULONG *fmt,
		  ULONG bytes,ULONG inv,DOUBLE scale);
  //
  // For palettized images, only one component input. Otherwise, the lookup
  // tables are applied, but even though data is here 16bpp, the output
  // is downshifted to 8 bpp (a multiplication would be more precise).
  template<class Decoder>
  void UnpackDataPaletized(UBYTE *buffer,bool bigendian,bool prediction,
			   ULONG xofs,ULONG y,
			   ULONG width,ULONG height,UBYTE bits,ULONG bytes,
			   const ULONG *r,const ULONG *g,const ULONG *b);
  //
  // Unpack the data from the source buffer into the destination component,
  // including the subsampling of the Cb and Cr components as indicated.
  // Y components are in a larger subbox of dimensions sx and sy.
  // xofs and yofs are offsets into the target plane where the data is to be
  // copied to. This always copies three components.
  // width and height are the dimensions of the rectangle to be copied.
  // bits is the number of bits per sample, size the buffer size.
  template<class Decoder>
  void UnpackDataYCbCr(UBYTE *buffer,bool bigendian,bool prediction,
		       ULONG xofs,ULONG yofs,
		       ULONG width,ULONG height,UBYTE b,ULONG bytes,
		       UBYTE sx   ,UBYTE sy);
public:
  //
  // default constructor
  SimpleTiff(void);
  //
  // Copy the image from another source for later saving.
  SimpleTiff(const class ImageLayout &layout);
  //
  // destructor
  ~SimpleTiff(void);
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
