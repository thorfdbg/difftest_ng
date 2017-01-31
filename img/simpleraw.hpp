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
 * This class saves and loads images in any header-less format.
 *
 * $Id: simpleraw.hpp,v 1.11 2017/01/31 11:58:04 thor Exp $
 */

#ifndef SIMPLERAW_HPP
#define SIMPLERAW_HPP

/// Includes
#include "interface/types.hpp"
#include "img/imglayout.hpp"
///

/// class SimpleRaw
// This class saves and loads images in any format. To specify the format, use the file
// name. That is:
// filename@format
// where format is constructed as follows:
// <w>x<h>x<d>:<encoding>
// The first part is the dimension of the image, in pixels, width, height, depth (components)
// This part can be empty when saving images (but only then).
// <encoding> specifies the encoding of the data.
// Data is expected to come in sample interleaved format by default (see below how to reorganize
// data in separate planes), in various fields, where each field is an integer number of bits
// large, and each field can be encoded in either big or little endian.
// The encoding for a field is:
// {sign-indicator number of bits type-indicator endian-indicator = target channel}
// and fields are separated by comma.
// The endian indicator is + or no indicator for big-endian, - for little endian, 
// the type indicator is optional and can be 'F' or 'f' for float
// the number of bits is indicated in decimal,
// the target channel is a channel from 0 to depth-1, which, if omitted, presents a padding
// field that is ignored on reading and written as zero on writing.
// Fields are separated by either comma or point. A comma separates fields that
// are bit-packed together, and then (possibly) endian-swapped. At most 32 bits can
// be packed this way. A colon is used to separate non-packed fields.
//
// If separate planes are required, use square brackets instead of round brackets. Do not mix
// the two. For separate planes, two subsampling factors can be specified, as subsampling in x
// and subsampling in y direction. Division is always defined by rounding up to the next
// integer. Thus, the field definition for separate planes look like this:
// [endian indicator number of bits = target channel]/subx x suby
// where the / and the subsampling indicators are optional, and are generated on saving
// the image, or assumed to be 1x1 on loading.
//
// Examples:
// {2-},{10-=2},{10-=1},{10-=0}  "PixelFormat 32 bit BGR101010"
// {1-},{5-=2},{5-=1},{5-=0}     "PixelFormat 16 bit BGR555"
// {5-=2},{6-=1},{5-=0}          "PixelFormat 16 bit BGR565"
// {8=2}:{8=1}:{8=0}             "PixelFormat 24 bit BGR"
// {8=2}:{8=1}:{8=0}:{8}         "PixelFormat 32 bit BGR"
// {8=2}:{8=1}:{8=0}:{8=3}       "PixelFormat 32 bit BGRA"
// {8=2}:{8=1}:{8=0}:{8=3}       "PixelFormat 32 bit PBGRA"
// [1=0],                        "1bpp byte-packed format"
// [8=0]:[8=1]:[8=2]             "YUV plane separate"
// [8=0]:[8=1]/2x2:[8=2]/2x2     "YUV 420 with 8 bpp per component"
// [8=0]:[8=1]/2x1:[8=2]/2x1     "YUV 422 with 8 bpp per component"
// {10=1},{10=0},{10=2},{2}:{10=0},{10=1},{10=0},{2}:{10=2},{10=0},{10=1},{2}:{10=0},{10=2},{10=0},{2}
// This is the mess called V210
class SimpleRaw : public ImageLayout {
  //
  // Buffer for the file name, without the format specifications.
  // This is filled in by the method below.
  char *m_pcFilename;
  //
  // Create the component layouts from the file name (Rather complicated, though).
  void ComponentLayoutFromFileName(const char *filename);
  //
  // The extended component layout.
  struct RawLayout : public ImageLayout::ComponentLayout {
    //
    // Pointer to the next element on this list.
    struct RawLayout *m_pNext;
    //
    // The component/channel this goes to/comes from.
    UWORD             m_usTargetChannel;
    //
    // The endianness of the component (in addition)
    // Default is false, i.e. big-endian.
    bool              m_bLittleEndian;
    //
    // True for dummy/padding components. Default is false,
    // i.e. is a real component.
    bool              m_bIsPadding;
    //
    // True in case we start filling from the LSB instead
    // of the LSB. Yuck!
    bool              m_bLefty;
    //
    // True in case this is the start of a bit-packing series
    // of fields.
    bool              m_bStartPacking;
    //
    // This field contains the number of bits that are packed
    // together. It is only valid if the packflag above is
    // set, and must be either 8,16 or 32.
    UBYTE             m_ucBitsPacked;
    //
    RawLayout(void) 
      : m_pNext(NULL), m_bLittleEndian(false), m_bIsPadding(false),
	m_bLefty(false), m_bStartPacking(false), m_ucBitsPacked(0)
    { }
    //
  }    *m_pRawList;
  //
  // Nominal dimensions as specified by the user.
  ULONG m_ulNominalWidth;
  ULONG m_ulNominalHeight;
  UWORD m_usNominalDepth;
  //
  // Number of fields (not to be confused with the number of components)
  UWORD m_usFields;
  //
  // Separate or interleaved? Default is interleaved.
  bool  m_bSeparate;
  //
  // The current bit position for reading or writing bits.
  UBYTE m_ucBit;
  //
  // The input or output bit buffer.
  ULONG m_ulBitBuffer;
  //
  // Read a single pixel from the specified file.
  UQUAD ReadData(FILE *in,UBYTE bitsize,UBYTE packsize,bool littleendian,bool issigned,bool lefty);
  //
  // Write a single data item to the file.
  void WriteData(FILE *out,UQUAD data,UBYTE bitsize,UBYTE packsize,bool littleendian,bool lefty);
  //
  // On writing, flush to the next byte boundary.
  void BitAlignOut(FILE *out,UBYTE packsize,bool littleendian,bool lefty);
  //
  // On reading, advance to the next byte boundary.
  void BitAlignIn(void)
  {
    m_ucBit       = 0; // Fetch the next byte on in.
    m_ulBitBuffer = 0;
  }
  //
public:
  // default constructor
  SimpleRaw(void);
  // Copy the image from another source for later saving.
  SimpleRaw(const class ImageLayout &layout);
  // destructor
  ~SimpleRaw(void);
  //
  // Save an image to a level 1 file descriptor, given its
  // width, height and depth.
  void SaveImage(const char *nameandspecs,const struct ImgSpecs &specs);
  //
  // Load an image from a level 1 file descriptor, keep it within
  // the internals of this class. The accessor methods below
  // should be used to find out more about this image.
  void LoadImage(const char *nameandspecs,struct ImgSpecs &specs);
  //
};
///

#endif
