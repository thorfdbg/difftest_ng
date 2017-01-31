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
** This file defines a simple tiff writer class. This phases the need for
** libtiff out as it does not support tiffs with varying bit depths.
**
** $Id: tiffwriter.cpp,v 1.9 2017/01/31 11:58:05 thor Exp $
**
*/

/// Includes
#include "tiff/tiffwriter.hpp"
#include "img/imglayout.hpp"
#include "std/errno.hpp"
#include "std/string.hpp"
///

/// TiffWriter::TiffWriter
// Create a new tiff writer from a file name.
TiffWriter::TiffWriter(const char *filename,bool bigendian)
  : m_pFile(NULL), m_pcFilename(filename), 
    m_pucStrip(NULL), m_ulBufferSize(0),
    m_pTags(NULL), m_bBigEndian(bigendian)
{
  m_pFile = fopen(filename,"wb");
  if (m_pFile == NULL) {
    ImageLayout::PostError("%s: unable to open the TIFF output file %s",strerror(errno),filename);
  }
  // Write the header
  if (m_bBigEndian) {
    PutByte('M');
    PutByte('M');
  } else {
    PutByte('I');
    PutByte('I');
  }
  PutWord(42); // the magic number
}
///

/// TiffWriter::~TiffWriter
// Dipose all of it, close the file as well.
TiffWriter::~TiffWriter(void)
{
  struct Tag *t;

  fclose(m_pFile);

  delete[] m_pucStrip;

  while((t = m_pTags)) {
    m_pTags = t->ti_pNext;
    delete t;
  }
}
///

/// TiffWriter::PutByte
// Write a byte to the output.
void TiffWriter::PutByte(UBYTE out)
{
  if (fwrite(&out,sizeof(out),1,m_pFile) != 1) {
    ImageLayout::PostError("%s: unable to write to the TIFF file %s",strerror(errno),m_pcFilename);
  }
}
///

/// TiffWriter::PutWord
// Write a 16 bit short to output
void TiffWriter::PutWord(UWORD out)
{
  if (m_bBigEndian) {
    PutByte(out >> 8);
    PutByte(out & 0xff);
  } else {
    PutByte(out & 0xff);
    PutByte(out >> 8);
  }
}
///

/// TiffWriter::PutLong
// Write a 32 bit short to output
void TiffWriter::PutLong(ULONG out)
{
  if (m_bBigEndian) {
    PutWord(out >> 16);
    PutWord(out & 0xffff);
  } else {
    PutWord(out & 0xffff);
    PutWord(out >> 16);
  }
}
///

/// TiffWriter::DefineTag
// Create a new tag of the given tag value, given type and given
// count.
void *TiffWriter::DefineTag(UWORD tag,UWORD type,ULONG count)
{
  struct Tag **prev = &m_pTags;
  struct Tag *t;

  // only byte,word,long,float supported here.
  assert(type == 1 || type == 3 || type == 4 || type == 11);

  // Find the tag where we should attach to.
  while(*prev) {
    if ((*prev)->ti_usTag > tag) // must attach in front of this.
      break;
    prev = &((*prev)->ti_pNext);
  }

  // Link it in.
  t             = new struct Tag();
  t->ti_pNext   = *prev;
  *prev         = t;
  t->ti_usTag   = tag;
  t->ti_usType  = type;
  t->ti_ulCount = count;
  t->ti_pulData = new ULONG[count];

  return t;
}
///

/// TiffWriter::DefineTagValue
// Fill in a tag value for the given tag at the given index.
void TiffWriter::DefineTagValue(void *t,ULONG index,ULONG value)
{
  struct Tag *tag = (struct Tag *)t;
  assert(tag);
  assert(tag->ti_pulData);
  assert(index < tag->ti_ulCount);

  tag->ti_pulData[index] = value;
}
///

/// TiffWriter::DefineScalarTag
// Create a simple scalar tag and define its value.
void TiffWriter::DefineScalarTag(UWORD tag,ULONG value)
{
  struct Tag *t;

  t = (struct Tag *)DefineTag(tag,(value > MAX_UWORD)?(4):(3),1);
  t->ti_pulData[0] = value;
}
///

/// TiffWriter::DefineFloatTag
// Create a simple scalar tag of a floating point type.
void TiffWriter::DefineFloatTag(UWORD tag,FLOAT value)
{
  struct Tag *t;
  union {
    ULONG ul;
    FLOAT f;
  } u;

  u.f = value;

  t = (struct Tag *)DefineTag(tag,11,1);
  t->ti_pulData[0] = u.ul;
}
///

/// TiffWriter::LayoutTags
// Layout the tags, compute all the offsets needed, and return
// the first available offset for the image data.
ULONG TiffWriter::LayoutTags(void)
{
  struct Tag *ti = m_pTags;
  ULONG offset   = 4 + 4 + 2; // header + IFD pointer + directory size.
  //
  // First, compute the size required for the tags itself.
  while(ti) {
    offset += 2+2+4+4; // the entry itself.
    ti = ti->ti_pNext;
  }
  // Add up the end of IFD chain entry.
  offset += 4;
  //
  // Now check which of the tags require links because data cannot be
  // fit into the data.
  ti = m_pTags;
  while(ti) {
    ULONG sz = 0,mx = 0;
    switch(ti->ti_usType) {
    case 1:
      sz = 1; // type = byte
      mx = MAX_LONG;
      break;
    case 3:
      sz = 2; // type = word
      mx = MAX_LONG >> 1;
      break;
    case 4:
    case 11:
      sz = 4; // type = long
      mx = MAX_LONG >> 2;
      break;
    default:
      assert(false);
    }
    if (ti->ti_ulCount > mx)
      ImageLayout::PostError("too much tag data for the TIFF file %s",m_pcFilename);
    sz *= ti->ti_ulCount;
    // If more than four bytes required, need to allocate extra storage.
    if (sz > 4) {
      ti->ti_ulOffset = offset; // allocate this offset.
      offset = (offset + sz + 1) & (~1); // align to a word boundary.
      if (offset < ti->ti_ulOffset)
	ImageLayout::PostError("too much tag data for the TIFF file %s",m_pcFilename);
    } else {
      ti->ti_ulOffset = 0;      // no offset required
    }
    //
    // Next one.
    ti = ti->ti_pNext;
  }

  return offset;
}
///

/// TiffWriter::WriteIFD
// Write out the IFD for the image and the tag data
void TiffWriter::WriteIFD(void)
{
  struct Tag *ti = m_pTags;
  UWORD count    = 0;
  //
  PutLong(4 + 4); // Offset to the IFD: skip the header, the IFD location.
  //
  // Count the dir entries.
  while(ti) {
    count++;
    if (count == 0)
      ImageLayout::PostError("too many IFD entries in TIFF file %s",m_pcFilename);
    ti = ti->ti_pNext;
  }
  //
  PutWord(count);
  //
  // Now write the tags itself.
  ti = m_pTags;
  while(ti) {
    PutWord(ti->ti_usTag);
    PutWord(ti->ti_usType);
    PutLong(ti->ti_ulCount);
    //
    // Either put the data directly, or the offset.
    if (ti->ti_ulOffset) {
      PutLong(ti->ti_ulOffset);
    } else {
      switch(ti->ti_usType) {
      case 1:
	PutByte((ti->ti_ulCount > 0)?(ti->ti_pulData[0]):(0));
	PutByte((ti->ti_ulCount > 1)?(ti->ti_pulData[1]):(0));
	PutByte((ti->ti_ulCount > 2)?(ti->ti_pulData[2]):(0));
	PutByte((ti->ti_ulCount > 3)?(ti->ti_pulData[3]):(0));
	break;
      case 3:
	PutWord((ti->ti_ulCount > 0)?(ti->ti_pulData[0]):(0));
	PutWord((ti->ti_ulCount > 1)?(ti->ti_pulData[1]):(0));
	break;
      case 4:
      case 11: // Actually, this is a float.
	PutLong((ti->ti_ulCount > 0)?(ti->ti_pulData[0]):(0));
	break;
      }
    }
    ti = ti->ti_pNext;
  }
  //
  // Write the link to the next IFD: There is none.
  PutLong(0);
  //
  // Now write the data linked to by the offsets.
  ti = m_pTags;
  while(ti) {
    if (ti->ti_ulOffset) {
      ULONG i;
      switch(ti->ti_usType) {
      case 1:
	for(i = 0;i < ti->ti_ulCount;i++) {
	  PutByte(ti->ti_pulData[i]);
	}
	// Align to byte boundary.
	if (ti->ti_ulCount & 1)
	  PutByte(0);
	break;
      case 3:
	for(i = 0;i < ti->ti_ulCount;i++) {
	  PutWord(ti->ti_pulData[i]);
	}
	break; 
      case 4:
      case 11:
	for(i = 0;i < ti->ti_ulCount;i++) {
	  PutLong(ti->ti_pulData[i]);
	}
	break;
      }
    }
    ti = ti->ti_pNext;
  }
}
///

/// TiffWriter::GetStripBuffer
// Allocate a buffer for a stripe to put data into.
UBYTE *TiffWriter::GetStripBuffer(ULONG sz)
{
  if (m_pucStrip && m_ulBufferSize >= sz) {
    m_ulStripData = sz;
    return m_pucStrip;
  }

  delete[] m_pucStrip;m_pucStrip = NULL;

  m_pucStrip = new UBYTE[m_ulBufferSize = m_ulStripData = sz];

  return m_pucStrip;
}
///

/// TiffWriter::PushStripBuffer
// Write the strip buffer out.
void TiffWriter::PushStripBuffer(void)
{
  if (m_pucStrip && m_ulStripData) {
    if (fwrite(m_pucStrip,sizeof(UBYTE),m_ulStripData,m_pFile) != m_ulStripData) {
      ImageLayout::PostError("%s: cannot write out TIFF image data to %s",strerror(errno),m_pcFilename);
    }
  }
}
///

