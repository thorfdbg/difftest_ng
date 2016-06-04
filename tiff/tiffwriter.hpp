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
**
** This file defines a simple tiff writer class. This phases the need for
** libtiff out as it does not support tiffs with varying bit depths.
**
** $Id: tiffwriter.hpp,v 1.7 2016/06/04 10:44:10 thor Exp $
**
*/

#ifndef TIFF_TIFFWRITER_HPP
#define TIFF_TIFFWRITER_HPP

/// Includes
#include "interface/types.hpp"
#include "std/stdio.hpp"
///

/// class TiffWriter
// A simple tiff writer class.
class TiffWriter {
  //
  // The file we write to.
  FILE       *m_pFile;
  //
  // The name of the file.
  const char *m_pcFilename;
  //
  // The stripe buffer and its buffer size.
  UBYTE      *m_pucStrip;
  //
  // The stripe buffer size.
  ULONG       m_ulBufferSize;
  //
  // Valid data in the buffer.
  ULONG       m_ulStripData;
  //
  // A single tag. These get sorted into a singly-linked list, then
  // offset-allocated.
  struct Tag {
    // The next tag. They form a singly-linked list.
    struct Tag *ti_pNext;
    //
    // The tag value
    UWORD       ti_usTag;
    //
    // The entry type.
    UWORD       ti_usType;
    //
    // The number of elements in here.
    ULONG       ti_ulCount;
    //
    // The allocated offset in the file
    // if any. Zero if in-line.
    ULONG       ti_ulOffset;
    //
    // The data (allocated).
    ULONG      *ti_pulData;
    //
  public:
    Tag(void)
      : ti_pNext(NULL), ti_ulCount(0), ti_ulOffset(0), ti_pulData(NULL)
    { }
    //
    ~Tag(void)
    {
      delete[] ti_pulData;
    }
  }          *m_pTags; // List of tags, sorted in ascending order.
  //
  // A big or little endian format?
  bool        m_bBigEndian;
  //
  // A couple of helpers.
  // Note that we write in little-endian since that seems to be more
  // common.
  //
  // Write a single byte
  void PutByte(UBYTE out);
  //
  // Write a short.
  void PutWord(UWORD out);
  //
  // Write a long.
  void PutLong(ULONG out);
  //
  //
public:
  TiffWriter(const char *filename,bool bigendian = false);
  //
  ~TiffWriter(void);
  //
  // Create a new tag of the given tag value, given type and given
  // count.
  void *DefineTag(UWORD tag,UWORD type,ULONG count);
  //
  // Fill in a tag value for the given tag at the given index.
  void DefineTagValue(void *tag,ULONG index,ULONG value);
  //
  // Create a simple scalar tag and define its value.
  void DefineScalarTag(UWORD tag,ULONG value);
  //
  // Create a scalar tag based on a floating point value.
  void DefineFloatTag(UWORD tag,FLOAT value);
  //
  // Layout the tags, compute all the offsets needed, and return
  // the first available offset for the image data.
  ULONG LayoutTags(void);
  //
  // Write out the IFD for the image and the tag data
  void WriteIFD(void);
  //
  // Allocate a buffer for a stripe to put data into.
  UBYTE *GetStripBuffer(ULONG sz);
  //
  // Write the strip buffer out.
  void PushStripBuffer(void);
  //
  // Write bits aligned into a buffer
  static void PutBits(UBYTE *&buffer,UBYTE &bitpos,UBYTE bps,ULONG value)
  {
    value &= (1UL << bps) - 1;
    
    while (bps >= bitpos) {
      // We have to write more bits than there is room in the bit buffer.
      // Hence, the complete buffer can be filled.
      *buffer |= value >> (bps - bitpos);
      buffer++;
      bps     -= bitpos;
      bitpos   = 8;
      // Remove upper bits.
      value   &= (1UL << bps) - 1;
    }
    // We have to write less than there is room in the bit buffer. Hence,
    // we must upshift the remaining bits to fit into the bit buffer.
    if (bitpos - bps < 8)
      *buffer |= value << (bitpos - bps);
    bitpos  -= bps;
  }
  //
  // Write UWORD into a buffer.
  void PutUWORD(UBYTE *&buffer,UWORD d)
  {
    if (m_bBigEndian) {
      *buffer++ = d >> 8;
      *buffer++ = d;
    } else {
      *buffer++ = d;
      *buffer++ = d >> 8;
    }
  }
  //
  // Write ULONG in little endian into a buffer.
  void PutULONG(UBYTE *&buffer,ULONG d)
  {
    if (m_bBigEndian) {
      *buffer++ = d >> 24;
      *buffer++ = d >> 16;
      *buffer++ = d >> 8;
      *buffer++ = d;
    } else {
      *buffer++ = d;
      *buffer++ = d >> 8;
      *buffer++ = d >> 16;
      *buffer++ = d >> 24;
    }
  }
  //
  // Write UQUAD in little endian into a buffer.
  void PutUQUAD(UBYTE *&buffer,UQUAD d)
  {
    if (m_bBigEndian) {
      *buffer++ = d >> 56;
      *buffer++ = d >> 48;
      *buffer++ = d >> 40;
      *buffer++ = d >> 32;
      *buffer++ = d >> 24;
      *buffer++ = d >> 16;
      *buffer++ = d >> 8;
      *buffer++ = d;
    } else {
      *buffer++ = d;
      *buffer++ = d >> 8;
      *buffer++ = d >> 16;
      *buffer++ = d >> 24;
      *buffer++ = d >> 32;
      *buffer++ = d >> 40;
      *buffer++ = d >> 48;
      *buffer++ = d >> 56;
    }
  }
};
///

///
#endif
