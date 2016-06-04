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
 * An image class to load and save uncompressed PPM/PGM/PBM images
 * from a file or to a file.
 * $Id: simplergbe.cpp,v 1.9 2016/06/04 10:44:09 thor Exp $
 */

/// Includes
#include "std/stdlib.hpp"
#include "std/string.hpp"
#include "tools/file.hpp"
#include "tools/halffloat.hpp"
#include "simplergbe.hpp"
#include "imgspecs.hpp"
///

/// SimpleRGBE::SimpleRGBE
// Default constructor.
SimpleRGBE::SimpleRGBE(void)
  : m_pfImage(NULL), m_pucTmp(NULL)
{
}
///

/// SimpleRGBE::SimpleRGBE
// Copy constructor, reference a PPM image.
SimpleRGBE::SimpleRGBE(const class ImageLayout &org)
  : ImageLayout(org), m_pfImage(NULL), m_pucTmp(NULL)
{
}
///

/// SimpleRGBE::~SimpleRGBE
// Dispose the object, delete the image
SimpleRGBE::~SimpleRGBE(void)
{
  delete[] m_pfImage;
  delete[] m_pucTmp;
}
///

/// SimpleRGBE::ReadNumber
// Read an ascii string from the input file,
// encoding a number. This number gets returned. Throws on error.
LONG SimpleRGBE::ReadNumber(void)
{
  LONG number   = 0;     // integer number (so far)
  bool negative = false; // sign of the number (true if negative);
  bool valid    = false; // gets true as soon as we get at least one valid digit.
  LONG in;

  //
  // Skip any leading blanks
  SkipBlanks();
  //
  in = Get();
  if (in == '+') {
    // number is positive, do nothing
  } else if (in == '-') {
    // number is negative, invert sign.
    negative = true;
  } else {
    // no sign, put data back.
    LastUnDo();
  }
  // Skip another block of blanks.
  SkipBlanks();
  // Now read digits, one by one.
  do {
    in = Get();
    if (in < '0' || in > '9')
      break; // stop on first invalid input.
    // Check whether the number would overflow.
    if (number >= 214748364) {
      PostError("Invalid number in RGBE file, number %ld too large.\n",long(number));
    }
    // Otherwise, add the digit.
    number = number * 10 + in - '0';
    valid  = true;
  } while(true);
  //
  // put back the last digit, we should not have read it as it is
  // not part of the number.
  LastUnDo();
  //
  // Now check whether we got at least a single valid digit. If
  // not, there is no valid number to deliver.
  if (!valid) {
    PostError("Invalid number in RGBE file, found no valid digit. First digit code is %ld\n",long(in));
  }
  //
  if (negative)
    number = -number;

  return number;
}
///

/// SimpleRGBE::SkipBlanks
// Skip blank spaces in the bytestream.
void SimpleRGBE::SkipBlanks(void)
{
  LONG ch;

  do {
    ch = Get();
  } while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
  //
  // Put the last read character back into the stream.
  LastUnDo();
}
///

/// SimpleRGBE::ReadField
// Get the next field, starting with non-! and ending with =
void SimpleRGBE::ReadField(char field[1024])
{
  char *p = field;
  LONG ch;
  
  do {
    ch = Get();
    if (ch < 0) 
      throw "unexpected EOF while reading RGBE file";
    if (ch == '!') {
      do {
	ch = Get();
	if (ch < 0)
	  throw "unexpected EOF while reading RGBE file";
      } while(ch != '\n');
    } else break;
  } while(true);

  LastUnDo();
  SkipBlanks();

  do {
    ch = Get();
    if (ch < 0)
      throw "unexpected EOF while reading RBGE file";
    if (ch == '\n')
      throw 0;
    if (p - field > 1023)
      throw "field tag too large while reading RGBE file, invalid format";
    if (ch == '=')
      break;
    *p++ = ch;
  } while(true);

  // Now remove trailing blanks
  while(p > field) {
    p--;
    if (*p != ' ' && *p != '\t') {
      *++p = 0; // terminate here
      break;
    }
  }
}
///

/// SimpleRGBE::ReadFieldValue
// Read the value associated to a field
void SimpleRGBE::ReadFieldValue(char value[1024])
{
  char *p = value;
  LONG ch;

  SkipBlanks();
  
  do {
    ch = Get();
    if (ch < 0)
      throw "unexpected EOF while reading RGBE field value, invalid format";
    if (ch == '\n')
      break;
    *p++ = ch;
    if (p - value > 1023)
      throw "field value too large while reading RGBE file, invalid format";
  } while(true);

  // Now remove trailing blanks
  while(p > value) {
    p--;
    if (*p != ' ' && *p != '\t') {
      *++p = 0; // terminate here
      break;
    }
  }
}
///

/// SimpleRGBE::SkipComment
// Skip comment lines starting with !
void SimpleRGBE::SkipComment(void)
{
  LONG c;
  //
  //
  do {
    c = Get();
    // Skip blanks, white spaces, etc.
  } while(c == ' ' || c == '\t' || c == '\r');
  //
  // Check the last character. If a new line, check for comments.
  if (c == '\n') {
    // Possibly a # is following.
    do {
      c = Get();
      if (c == '!') {
	// A comment line.
	do {
	  c = Get();
	} while(c != '\n' && c != '\r' && c != -1);
      } else {
	// put it back
	LastUnDo();
	break;
      }
    } while(true);
  } else {
    LastUnDo();
  }
}
///

/// SimpleRGBE::LoadLineUncompressed
// Read a non-RLE coded version of a hdr file.
void SimpleRGBE::LoadLineUncompressed(ULONG width,FLOAT *buffer)
{  
  ULONG x;
  //
  for(x=0;x<width;x++) {
    LONG dt1,dt2,dt3,dt4;
    //
    // Check whether we need the next bit.
    dt1 = Get();
    dt2 = Get();
    dt3 = Get();
    dt4 = Get();
    if (dt1 < 0 || dt2 < 0 || dt3 < 0 || dt4 < 0)
      throw "unexpected EOF in RGBE stream\n";
    //
    RGBE2Float(dt1,dt2,dt3,dt4,buffer);
    buffer += 3;
  }
}
///

/// SimpleRGBE::LoadLineCompressed
// Read an RLE-encoded version of a hdr file.
void SimpleRGBE::LoadLineCompressed(ULONG width,FLOAT *buffer)
{
  UBYTE *tmp    = m_pucTmp;
  UBYTE *tend,*trun,*ttmp;
  int c;

  assert(tmp);
  for(c = 0;c < 4;c++) {
    ttmp = tmp  + c;
    tend = ttmp + (width << 2);
    
    while(ttmp < tend) {
      LONG count = Get(); // get the instruction - or - count.
      if (count < 0)
	throw "invalid .rgbe file, unexpected EOF";
      
      // Actually, 128 shouldn't be allowed here, but some
      // programs don't take this too serious. It seems that
      // 128 means 128 individual values, not a run.
      trun   = ttmp + ((count == 128)?(128 << 2):((count & 0x7f) << 2));
      if (trun > tend /*|| trun == ttmp*/)
	throw "invalid .rgbe file, run length compression across rows attempted";
      
      if (count > 128) {
	// The run case.
	LONG v = Get();
	if (v < 0)
	  throw "invalid .rgbe file, unexpected EOF";
	
	while(ttmp < trun) {
	  *ttmp = v;
	  ttmp += 4;
	}
      } else {
	// The non-run case
	while(ttmp < trun) {
	  LONG v = Get();
	  if (v < 0)
	    throw "invalid .rgbe file, unexpected EOF";
	  *ttmp = v;
	  ttmp += 4;
	}
      }
    }
    assert(ttmp == tend);
  }
  //
  // Now convert the data and feed it into the real buffer.
  tend = tmp + (width << 2);
  while(tmp < tend) {
    RGBE2Float(tmp[0],tmp[1],tmp[2],tmp[3],buffer);
    buffer += 3;
    tmp    += 4;
  }
}
///

/// SimpleRGBE::LoadImage
// Load an image from an already open (binary) PPM or PGM file
// Throw in case the file should be invalid.
void SimpleRGBE::LoadImage(const char *basename,struct ImgSpecs &specs)
{ 
  LONG ch;
  UWORD i;
  ULONG y;
  bool formatok = false;
  char field[1024],value[1024];
  File file(basename,"rb");
  //
  //
  if (m_pComponent || m_pucTmp) {
    PostError("Image is already loaded.\n");
  }
  //
  m_pFile = file;
  //
  // Bummer! Dolby jpeg-hdr writes an invalid header.
  if (Get() == 'H') {
    bool value = true;
    LastUnDo();
    try {
      ReadField(field);
    } catch(int) {
      if (strncmp(field,"HDR ",4))
	PostError("Stream is no valid RGBE file.\n");
      value = false;
    }
    if (value)
      PostError("Stream is no valid RGBE file.\n");
  } else {
    LastUnDo();
    // Read the header of the file.
    if (Get() != '#' || Get() != '?') {
      PostError("Stream is no valid RGBE file.\n");
    }
    //
    // Actually, I do not really care about the value.
    ReadFieldValue(value);
  }
  //
  // Now read the fields, parse them off one by another, until the format
  // field is found, which is then the end of the file.
  do {
    LONG ch = Get();
    //
    if (ch < 0) {
      throw "invaliod RGBE format, unexpected EOF";
    } else if (ch == '#') {
      // Is a comment, just skip
      ReadFieldValue(value);
    } else if (ch == '\n') {
      // A single blank line, data starts here.
      if (!formatok) 
	throw "invalid RGBE format, couldn't find format specifier";
      break;
    } else {
      // A regular field
      LastUnDo();
      try {
	ReadField(field);
	ReadFieldValue(value);
      } catch(int) {
	// A field without value.
	continue;
      }
      //
      if (!strcmp(field,"FORMAT")) {
	if (strcmp(value,"32-bit_rle_rgbe"))
	  throw "unknown RGBE sample representation, cannot parse file";
	formatok = true;
      }
    }
  } while(true);
  //
  // The next line contains the size of the image.
  SkipBlanks();
  ch = Get();
  if (ch != '-' && ch != '+')
    throw "invalid RGBE format, expected + or - for height indicator";
  ch = Get();
  if (ch != 'Y')
    throw "invalid RGBE format, expected Y as height indicator";
  //
  m_ulHeight = ReadNumber();
  //
  SkipBlanks();
  ch = Get();
  if (ch != '-' && ch != '+')
    throw "invalid RGBE format, expected + or - for width indicator";
  ch = Get();
  if (ch != 'X')
    throw "invalid RGBE format, expected X as width indicator";
  //
  m_ulWidth = ReadNumber();
  //
  // The depth is always 3.
  m_usDepth = 3;
  //
  // Now build the component array.
  CreateComponents(m_ulWidth,m_ulHeight,m_usDepth);  
  //
  specs.ASCII      = ImgSpecs::No;
  specs.Palettized = ImgSpecs::No;
  specs.YUVEncoded = ImgSpecs::No;
  //
  m_pfImage  = new FLOAT[m_ulWidth * m_ulHeight * m_usDepth];
  //
  // Ok, now fill out the components.
  for(i = 0; i < m_usDepth; i++) {
    m_pComponent[i].m_ucBits          = 32; // is always float
    m_pComponent[i].m_ulBytesPerPixel = m_usDepth * sizeof(FLOAT); // Notice "per byte"
    m_pComponent[i].m_ulBytesPerRow   = m_usDepth * sizeof(FLOAT) * m_ulWidth;
    m_pComponent[i].m_pPtr            = m_pfImage + i;
    m_pComponent[i].m_bFloat          = true;
    m_pComponent[i].m_bSigned         = true; 
    // set to true for consistency with other float formats
  }
  //
  // Skip a single whitespace character.
  ch = Get();
  // Check for MS-Dos line separator \r\n
  if (ch == '\r') {
    ch = Get();
    if (ch != '\n') {
      // no MS-Dos separator, MacOs separator! Iek!
      LastUnDo();
      ch = '\n';
    }
  }
  //
  // A new line must be found here.
  if (ch != '\n') 
    throw "invalid format, expected a new line before the data block";
  //
  assert(m_pucTmp == NULL);
  //
  FLOAT *buffer = m_pfImage;
  for(y = 0;y < m_ulHeight;y++,buffer += m_ulWidth * m_usDepth) {
    // Now read the data, component wise interleaved. 
    // RLE-compressed or uncompressed. Depends on the representation.
    if (m_ulWidth < 8 || m_ulWidth > 0x7fff) {
      // This data is never compressed
      LoadLineUncompressed(m_ulWidth,buffer);
    } else {
      // This data might be compressed. Read the initial pixel. Note that
      // this is *NOT* safe in the sense that a 100% detection can be
      // ensured.
      LONG dt1 = Get();
      LONG dt2 = Get();
      LONG dt3 = Get();
      LONG dt4 = Get();
      if (dt1 < 0 || dt2 < 0 || dt3 < 0 || dt4 < 0)
	throw "unexpected EOF in RGBE stream\n";
      //
      // Check for RLE compression. Yuck. Big Yuck!
      if (dt1 == 2 && dt2 == 2 && dt3 < 128 && ULONG((dt3 << 8) | dt4) == m_ulWidth) {
	if (m_pucTmp == NULL)
	  m_pucTmp = new UBYTE[m_ulWidth << 2];
	LoadLineCompressed(m_ulWidth,buffer);
      } else {
	// Otherwise, uncompressed. Put the pixel in raw into the output
	// buffer, and continue.
	RGBE2Float(dt1,dt2,dt3,dt4,buffer);
	LoadLineUncompressed(m_ulWidth - 1,buffer + 3);
      }
    }
  }
  //
  if (ferror(m_pFile)) {
    PostError("I/O error while reading the stream.\n");
  }
}
///

/// SimpleRGBE::SaveImage
// Save the image to a PGM/PPM file, throw in case of error.
void SimpleRGBE::SaveImage(const char *basename,const struct ImgSpecs &)
{
  UWORD i;
  ULONG x,y;
  UBYTE prec   = 8;
  FLOAT scale  = 1.0f;
  File output(basename,"wb");
  //
  // Must exist.
  if (m_pComponent == NULL) {
    PostError("No image loaded to save data of.\n");
  }
  //
  m_pFile = output;
  //
  // Only RGB is supported here, requires exactly three components.
  if (m_usDepth == 3) {
    // Write RGBE file. Though component depth must match.
    for(i = 0;i < 3;i++) {
      if (m_pComponent[i].m_ucBits  != m_pComponent[0].m_ucBits  ||
	  m_pComponent[i].m_bSigned != m_pComponent[0].m_bSigned ||
	  m_pComponent[i].m_bFloat  != m_pComponent[0].m_bFloat) {
	PostError("Images with different bitdepths or number formats are not supported by the RGBE format.\n");
      }  
      if (m_pComponent[i].m_ucSubX  != 1 ||
	  m_pComponent[i].m_ucSubY  != 1) {
	PostError("RGBE does not support subsampling.\n");
      }
    }
    if (m_pComponent[0].m_bSigned && m_pComponent[0].m_bFloat == false)
      PostError("RGBE does not support signed integer formats.\n");
    prec = m_pComponent[0].m_ucBits;
    if (m_pComponent[i].m_bFloat == false) {
      // Scale integer components such that the maximal value becomes 1.0
      scale = 1.0 / ((1UL << prec) - 1);
    }
  } else {
    // unsupported type. Outch.
    PostError("Unsupported number of components for RGBE output.\n");
  }
  //
  // Write the format specifier and the image dimension.
  // Exposure and gamma cannot be written since they are unknown.
  fprintf(m_pFile,"#?RGBE\n");
  fprintf(m_pFile,"FORMAT=32-bit_rle_rgbe\n\n");
  // Write image dimensions.
  fprintf(m_pFile,"-Y %lu +X %lu\n",(unsigned long)m_ulHeight,(unsigned long)(m_ulWidth));
  //
  if (m_pComponent[0].m_bFloat == false && prec == 32) {
    ULONG *dt1 = (ULONG *)(m_pComponent[0].m_pPtr);
    ULONG *dt2 = (ULONG *)(m_pComponent[1].m_pPtr);
    ULONG *dt3 = (ULONG *)(m_pComponent[2].m_pPtr);
    //
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	WriteRGBE(*dt1 * scale,*dt2 * scale,*dt3 * scale);	
	dt1 = (ULONG *)((UBYTE *)(dt1) + m_pComponent[0].m_ulBytesPerPixel);
	dt2 = (ULONG *)((UBYTE *)(dt2) + m_pComponent[1].m_ulBytesPerPixel);
	dt3 = (ULONG *)((UBYTE *)(dt3) + m_pComponent[2].m_ulBytesPerPixel);
      }
      dt1 = (ULONG *)((UBYTE *)(dt1) - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel + m_pComponent[0].m_ulBytesPerRow);
      dt2 = (ULONG *)((UBYTE *)(dt2) - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel + m_pComponent[1].m_ulBytesPerRow);
      dt3 = (ULONG *)((UBYTE *)(dt3) - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel + m_pComponent[2].m_ulBytesPerRow);
    }
  } else if (m_pComponent[0].m_bFloat == false && prec <= 8) {
    UBYTE *p0 = (UBYTE *)(m_pComponent[0].m_pPtr);
    UBYTE *p1 = (UBYTE *)(m_pComponent[1].m_pPtr);
    UBYTE *p2 = (UBYTE *)(m_pComponent[2].m_pPtr);
    //
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	WriteRGBE(*p0 * scale,*p1 * scale,*p2 * scale);
	p0 += m_pComponent[0].m_ulBytesPerPixel;
	p1 += m_pComponent[1].m_ulBytesPerPixel;
	p2 += m_pComponent[2].m_ulBytesPerPixel;
      }
      p0 += m_pComponent[0].m_ulBytesPerRow - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel;
      p1 += m_pComponent[1].m_ulBytesPerRow - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel;
      p2 += m_pComponent[2].m_ulBytesPerRow - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel;
    }
  } else if (m_pComponent[0].m_bFloat == false && prec <= 16) {
    UWORD *p0 = (UWORD *)(m_pComponent[0].m_pPtr);
    UWORD *p1 = (UWORD *)(m_pComponent[1].m_pPtr);
    UWORD *p2 = (UWORD *)(m_pComponent[2].m_pPtr);
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	WriteRGBE(*p0 * scale,*p1 * scale,*p2 * scale);
	p0 = (UWORD *)((UBYTE *)(p0) + m_pComponent[0].m_ulBytesPerPixel);
	p1 = (UWORD *)((UBYTE *)(p1) + m_pComponent[1].m_ulBytesPerPixel);
	p2 = (UWORD *)((UBYTE *)(p2) + m_pComponent[2].m_ulBytesPerPixel);
      }
      p0 = (UWORD *)((UBYTE *)(p0) + m_pComponent[0].m_ulBytesPerRow - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel);
      p1 = (UWORD *)((UBYTE *)(p1) + m_pComponent[1].m_ulBytesPerRow - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel);
      p2 = (UWORD *)((UBYTE *)(p2) + m_pComponent[2].m_ulBytesPerRow - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel);
    }
  } else if (m_pComponent[0].m_bFloat && prec == 32) {
    FLOAT *dt1 = (FLOAT *)(m_pComponent[0].m_pPtr);
    FLOAT *dt2 = (FLOAT *)(m_pComponent[1].m_pPtr);
    FLOAT *dt3 = (FLOAT *)(m_pComponent[2].m_pPtr);
    //
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	WriteRGBE(*dt1,*dt2,*dt3);
	dt1 = (FLOAT *)((UBYTE *)(dt1) + m_pComponent[0].m_ulBytesPerPixel);
	dt2 = (FLOAT *)((UBYTE *)(dt2) + m_pComponent[1].m_ulBytesPerPixel);
	dt3 = (FLOAT *)((UBYTE *)(dt3) + m_pComponent[2].m_ulBytesPerPixel);
      }
      dt1 = (FLOAT *)((UBYTE *)(dt1) - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel + m_pComponent[0].m_ulBytesPerRow);
      dt2 = (FLOAT *)((UBYTE *)(dt2) - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel + m_pComponent[1].m_ulBytesPerRow);
      dt3 = (FLOAT *)((UBYTE *)(dt3) - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel + m_pComponent[2].m_ulBytesPerRow);
    }
  } else if (m_pComponent[0].m_bFloat && prec == 64) {
    DOUBLE *dt1 = (DOUBLE *)(m_pComponent[0].m_pPtr);
    DOUBLE *dt2 = (DOUBLE *)(m_pComponent[1].m_pPtr);
    DOUBLE *dt3 = (DOUBLE *)(m_pComponent[2].m_pPtr);
    //
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	WriteRGBE(*dt1,*dt2,*dt3);
	dt1 = (DOUBLE *)((UBYTE *)(dt1) + m_pComponent[0].m_ulBytesPerPixel);
	dt2 = (DOUBLE *)((UBYTE *)(dt2) + m_pComponent[1].m_ulBytesPerPixel);
	dt3 = (DOUBLE *)((UBYTE *)(dt3) + m_pComponent[2].m_ulBytesPerPixel);
      }
      dt1 = (DOUBLE *)((UBYTE *)(dt1) - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel + m_pComponent[0].m_ulBytesPerRow);
      dt2 = (DOUBLE *)((UBYTE *)(dt2) - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel + m_pComponent[1].m_ulBytesPerRow);
      dt3 = (DOUBLE *)((UBYTE *)(dt3) - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel + m_pComponent[2].m_ulBytesPerRow);
    }
  } else if (m_pComponent[0].m_bFloat && prec == 16) {
    HALF *dt1 = (HALF *)(m_pComponent[0].m_pPtr);
    HALF *dt2 = (HALF *)(m_pComponent[1].m_pPtr);
    HALF *dt3 = (HALF *)(m_pComponent[2].m_pPtr);
    //
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	WriteRGBE(H2F(*dt1),H2F(*dt2),H2F(*dt3));
	dt1 = (HALF *)((UBYTE *)(dt1) + m_pComponent[0].m_ulBytesPerPixel);
	dt2 = (HALF *)((UBYTE *)(dt2) + m_pComponent[1].m_ulBytesPerPixel);
	dt3 = (HALF *)((UBYTE *)(dt3) + m_pComponent[2].m_ulBytesPerPixel);
      }
      dt1 = (HALF *)((UBYTE *)(dt1) - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel + m_pComponent[0].m_ulBytesPerRow);
      dt2 = (HALF *)((UBYTE *)(dt2) - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel + m_pComponent[1].m_ulBytesPerRow);
      dt3 = (HALF *)((UBYTE *)(dt3) - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel + m_pComponent[2].m_ulBytesPerRow);
    }
  }
  //
  if (ferror(m_pFile)) {
    PostError("IO error while writing the PPM file.\n");
  }
}
///
