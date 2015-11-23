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
 * An image class to load and save uncompressed PPM/PGM/PBM images
 * from a file or to a file.
 * $Id: simpleppm.cpp,v 1.18 2014/10/18 09:35:20 thor Exp $
 */

/// Includes
#include "std/stdlib.hpp"
#include "tools/file.hpp"
#include "simpleppm.hpp"
#include "imgspecs.hpp"
///

/// SimplePpm::SimplePpm
// Default constructor.
SimplePpm::SimplePpm(void)
  : m_pucImage(NULL), m_pusImage(NULL), m_pfImage(NULL)
{
}
///

/// SimplePpm::SimplePpm
// Copy constructor, reference a PPM image.
SimplePpm::SimplePpm(const class ImageLayout &org)
  : ImageLayout(org), m_pucImage(NULL), m_pusImage(NULL), m_pfImage(NULL)
{
}
///

/// SimplePpm::~SimplePpm
// Dispose the object, delete the image
SimplePpm::~SimplePpm(void)
{
  delete[] m_pucImage;
  delete[] m_pusImage;
  delete[] m_pfImage;
}
///

/// SimplePpm::ReadNumber
// Read an ascii string from the input file,
// encoding a number. This number gets returned. Throws on error.
LONG SimplePpm::ReadNumber(void)
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
      PostError("Invalid number in PPM file, number %ld too large.\n",long(number));
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
    PostError("Invalid number in PPM file, found no valid digit. First digit code is %ld\n",long(in));
  }
  //
  if (negative)
    number = -number;

  return number;
}
///

/// SimplePpm::SkipBlanks
// Skip blank spaces in the bytestream.
void SimplePpm::SkipBlanks(void)
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

/// SimplePpm::SkipLine
// Skip an entire line
void SimplePpm::SkipLine(void)
{
  LONG c;
  do {
    c = Get();
  } while(c != '\n' && c != -1);
}
///

/// SimplePpm::SkipComment
// Skip comment lines starting with #
void SimplePpm::SkipComment(void)
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
      if (c == '#') {
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

/// SimplePpm::LoadImage
// Load an image from an already open (binary) PPM or PGM file
// Throw in case the file should be invalid.
void SimplePpm::LoadImage(const char *basename,struct ImgSpecs &specs)
{ 
  LONG data;
  UWORD i;
  LONG precision;
  ULONG x,y;
  UBYTE bits;
  double scale = 1.0;
  bool raw; // raw or ASCII?  
  bool flt = false; // pfm or ppm?
  bool pfs = false; // pfs or pfm?
  bool bigendian = true; // default is bigendian.
  File file(basename,"rb");
  //
  //
  if (m_pComponent) {
    PostError("Image is already loaded.\n");
  }
  //
  m_pFile = file;
  //
  // Read the header of the file. This must be P6 for a
  // color image, and P5 for a grey-scale image. We currently
  // do not support ASCII images.
  data = Get();
  if (data != 'P') {
    PostError("Stream is no valid PPM file.\n");
  }
  //
  data      = Get();
  precision = 0;
  switch(data) {
  case '6':
    // A color image. Allocate three components.
    m_usDepth = 3;
    raw       = true;
    break;
  case '3':
    // A color image, ascii encoded.
    m_usDepth = 3;
    raw       = false;
    break;
  case '5':
    // A B&W image, raw encoded.
    m_usDepth = 1;
    raw       = true;
    break;
  case '2':
    m_usDepth = 1;
    raw       = false;
    break;
  case '4':
    // A bitplane, raw encoded.
    m_usDepth = 1;
    precision = 1;
    raw       = true;
    break;
  case '1':
    // A bitplane, ascii encoded.
    m_usDepth = 1;
    precision = 1;
    raw       = false;
    break;
  case 'F':
    // color floating point image
    m_usDepth = 3;
    precision = 32;
    raw       = true;
    flt       = true; 
    if (Get() == 'S') {
      // Had the string PFS. Check the version number.
      if (Get() != '1') {
	throw "image is not a valid PFS file, or uses an unknown PFS version";
      }
      pfs = true;
    } else {
      LastUnDo();
    }
    break;
  case 'f':
    // grey-scale floating point image
    m_usDepth = 1;
    precision = 32;
    raw       = true;
    flt       = true;
    break;
  default:
    // Not a PPM file.
    throw "image is not a valid PNM file, or uses a PNM type unknown to this program";
  } 
  //
  specs.ASCII      = (raw)?(ImgSpecs::No):(ImgSpecs::Yes);
  specs.Palettized = ImgSpecs::No;
  specs.YUVEncoded = ImgSpecs::No;
  //
  SkipComment();
  // Read the m_ulWidth and the m_ulHeight off the stream. This will also
  // strip blanks we might have in the stream now.
  data  = ReadNumber();
  if (data <= 0)
    throw "PNM image is not valid, width must be positive";
  m_ulWidth = data;
  SkipComment();
  data = ReadNumber();
  if (data <= 0)
    throw "PNM image is not valid, height must be positive";
  m_ulHeight = data;
  //
  if (flt) {
    // PFM comes with a scale that also defines the endian-ness.
    SkipComment();
    SkipBlanks();    
    if (pfs) {
      LONG channels;
      LONG tags;
      // Pfs comes here with a channel count.
      channels = ReadNumber();
      if (channels < 1 || channels > MAX_UWORD)
	throw "the PFS file defines too many channels, must be at least one and at most 65535";
      m_usDepth = UWORD(channels);
      bigendian = false;
      // Read the number of frames.
      SkipComment();
      tags = ReadNumber();
      SkipComment();
      if (tags < 0)
	throw "invalid number of frame tags in PFS file, must be non-negative";
      while(tags) {
	SkipLine();
	tags--;
      }
      while(channels) {
	SkipLine(); // The channel name. I do not care.
	SkipComment();
	tags = ReadNumber();
	SkipComment();
	if (tags < 0)
	  throw "invalid number of frame tags in PFS file, must be non-negative";
	while(tags) {
	  SkipLine();
	  tags--;
	}
	channels--;
      }
      if (Get() != 'E')
	throw "invalid end of header indicator in PFS file";
      if (Get() != 'N')
	throw "invalid end of header indicator in PFS file";
      if (Get() != 'D')
	throw "invalid end of header indicator in PFS file";
      if (Get() != 'H')
	throw "invalid end of header indicator in PFS file";
    } else {
      if (fscanf(m_pFile,"%lf",&scale) != 1)
	throw "image is not a valid PFM file, scale encoded incorrectly";
      if (scale < 0.0) {
	bigendian = false;
	scale     = -scale;
      }
      // The scale itself is uninteresting here.
    }
  } else {
    // PBM doesn't come with the precision. All others do.
    if (precision == 0) {
      SkipComment();
      precision = ReadNumber();
    }
  }
  //
  // Now build the component array.
  CreateComponents(m_ulWidth,m_ulHeight,m_usDepth);
  //
  // Compute the bit depth from the precision
  if (flt) {
    bits = 32;
  } else {
    bits = 0;
    while(precision >= 1) {
      bits++;
      precision >>= 1;
    }
    //
    // At most WORDs.
    if (bits > 16) {
      PostError("Unsupported bit depth in PPM file.\n");
    }
  }
  //
  // The next step depends on whether we are UBYTE or UWORD.
  if (bits == 32) {
    m_pfImage  = new FLOAT[m_ulWidth * m_ulHeight * m_usDepth];
    //
    // Ok, now fill out the components. PFM is interleaved, PFS is separate.
    if (pfs) { 
      for(i = 0; i < m_usDepth; i++) {
	m_pComponent[i].m_ucBits          = bits;
	m_pComponent[i].m_ulBytesPerPixel = 4; 
	m_pComponent[i].m_ulBytesPerRow   = 4 * m_ulWidth;
	m_pComponent[i].m_pPtr            = m_pfImage + m_ulWidth * m_ulHeight * i;
	m_pComponent[i].m_bFloat          = true;
	m_pComponent[i].m_bSigned         = true;
      }
    } else {
      for(i = 0; i < m_usDepth; i++) {
	m_pComponent[i].m_ucBits          = bits;
	m_pComponent[i].m_ulBytesPerPixel = m_usDepth * 4; // Notice the "per byte" indicator!
	m_pComponent[i].m_ulBytesPerRow   = m_usDepth * 4 * m_ulWidth;
	m_pComponent[i].m_pPtr            = m_pfImage + i;
	m_pComponent[i].m_bFloat          = true;
	m_pComponent[i].m_bSigned         = true;
      }
    }
  } else if (bits > 8) {
    m_pusImage = new UWORD[m_ulWidth * m_ulHeight * m_usDepth];
    //
    // Ok, now fill out the components.
    for(i = 0; i < m_usDepth; i++) {
      m_pComponent[i].m_ucBits          = bits;
      m_pComponent[i].m_ulBytesPerPixel = m_usDepth * 2; // Notice the "per byte" indicator!
      m_pComponent[i].m_ulBytesPerRow   = m_usDepth * 2 * m_ulWidth;
      m_pComponent[i].m_pPtr            = m_pusImage + i;
    }
  } else {
    m_pucImage = new UBYTE[m_ulWidth * m_ulHeight * m_usDepth];
    //
    // Ok, now fill out the components.
    for(i = 0; i < m_usDepth; i++) {
      m_pComponent[i].m_ucBits          = bits;
      m_pComponent[i].m_ulBytesPerPixel = m_usDepth;
      m_pComponent[i].m_ulBytesPerRow   = m_usDepth * m_ulWidth;
      m_pComponent[i].m_pPtr            = m_pucImage + i;
    }
  }
  //
  if (!pfs) {
    // Skip a single whitespace character.
    data = Get();
    // Check for MS-Dos line separator \r\n
    if (data == '\r') {
      data = Get();
      if (data != '\n') {
	// no MS-Dos separator, MacOs separator! Iek!
	LastUnDo();
	data = '\r';
      }
    }
    //
    // A new line must be found here.
    if (data != ' ' && data != '\n' && data != '\r' && data != '\t') {
      PostError("Malformed PPM stream.\n");
    }
  }
  //
  // Now read the data, component wise interleaved. Depends on the representation.
  if (flt) {
    //
    // Check what to do about the scale.
    if (specs.AbsoluteRadiance != ImgSpecs::Yes) {
      // Here keep the scale in the specs, write it out later.
      specs.RadianceScale = scale;
      scale = 1.0;
    }
    //
    FLOAT *buffer = m_pfImage; // assumes that the FPU endianness is equal to the integer endianness
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	for(i=0;i<m_usDepth;i++) {
	  LONG dt1,dt2,dt3,dt4;
	  union {
	    ULONG ul;
	    FLOAT f;
	  } u;
	  //
	  // Check whether we need the next bit.
	  dt1 = Get();
	  dt2 = Get();
	  dt3 = Get();
	  dt4 = Get();
	  if (dt1 < 0 || dt2 < 0 || dt3 < 0 || dt4 < 0)
	    PostError("Unexpected EOF in PPM stream.\n");
	  if (bigendian) {
	    u.ul = (ULONG(dt1) << 24) | (ULONG(dt2) << 16) | (ULONG(dt3) <<  8) | (ULONG(dt4) <<  0);
	  } else {
	    u.ul = (ULONG(dt4) << 24) | (ULONG(dt3) << 16) | (ULONG(dt2) <<  8) | (ULONG(dt1) <<  0);
	  }
	  *buffer++ = scale * u.f;
	}
      }
    }
  } else if (raw) {
    if (bits == 1) {
      // Bit-packed data.
      UBYTE *target = m_pucImage;
      for(y=0;y<m_ulHeight;y++) {
	int mask = 0x00;
	for(x=0;x<m_ulWidth;x++) {
	  //
	  // Check whether we need the next bit.
	  if (mask == 0) {
	    data = Get();
	    if (data == -1) {
	      PostError("Unexpected EOF in PPM stream.\n");
	    }
	    mask = 0x80;
	  }
	  *target++ = (data & mask)?0:1;
	  mask    >>= 1;
	}
      }
    } else if (bits <= 8) {
      // Byte-packed data.
      UBYTE *target = m_pucImage;
      for(y=0;y<m_ulHeight;y++) {
	for(x=0;x<m_ulWidth;x++) {
	  for(i=0;i<m_usDepth;i++) {
	    //
	    // Check whether we need the next bit.
	    data = Get();
	    if (data == -1) {
	      PostError("Unexpected EOF in PPM stream.\n");
	    }
	    *target++ = data;
	  }
	}
      }
    } else {
      // WORD packed data.
      UWORD *target = m_pusImage;
      for(y=0;y<m_ulHeight;y++) {
	for(x=0;x<m_ulWidth;x++) {
	  for(i=0;i<m_usDepth;i++) {
	    //
	    // Check whether we need the next bit.
	    data  = Get() << 8; // most significant first.
	    data |= Get();      // note that or'ing EOF will still keep this at -1
	    if (data == -1) {
	      PostError("Unexpected EOF in PPM stream.\n");
	    }
	    *target++ = data;
	  }
	}
      }
    }
  } else {
    UBYTE *bytedata = m_pucImage;
    UWORD *worddata = m_pusImage;
    // ASCII encoding.
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	for(i=0;i<m_usDepth;i++) {
	  data = ReadNumber();
	  if (bits > 8) {
	    *worddata++ = data;
	  } else if (bits > 1) {
	    *bytedata++ = data;
	  } else {
	    // PBM is just inverted... Juck.
	    *bytedata++ = (data)?0:1;
	  }
	}
      }
    }
  }
  //
  if (ferror(m_pFile)) {
    PostError("I/O error while reading the stream.\n");
  }
}
///

/// SimplePpm::SaveImage
// Save the image to a PGM/PPM file, throw in case of error.
void SimplePpm::SaveImage(const char *basename,const struct ImgSpecs &specs,bool pfs)
{
  UWORD i;
  ULONG x,y;
  UBYTE prec    = 8;
  UWORD offset  = 0; // added for signed components
  bool raw      = (specs.ASCII        != ImgSpecs::Yes);
  bool dolittle = (specs.LittleEndian == ImgSpecs::Yes);
  File output(basename,"wb");
  //
  // Must exist.
  if (m_pComponent == NULL) {
    PostError("No image loaded to save data of.\n");
  }
  //
  m_pFile = output;
  //
  // Check whether this is a color of a grey-scale image. We only support
  // one or three components.
  Put('P'); // Indicator for PPM/PGM
  //
  if (pfs) {
    Put('F');
    Put('S');
    Put('1');
  } else {
    if (m_usDepth == 1) {
      // grey scale or B&W. 
      prec = m_pComponent[0].m_ucBits;
      if (m_pComponent[0].m_bFloat) {
	if (prec != 32)
	  PostError("Unable to save PPM files with anything but 32 bit IEEE single precision float");
	Put('f');
      } else if (prec == 1) {
	// Write PBM file.
	Put(raw?'4':'1');
      } else {
	if (prec > 16)
	  PostError("PPM supports at most 16 bits per pixel");
	// Write PPM file.
	Put(raw?'5':'2');
      }
    } else if (m_usDepth == 3) {
      // Write PGM file. Though component depth must match.
      prec =  m_pComponent[0].m_ucBits;
      for(i = 0;i < 3;i++) {
	if (m_pComponent[i].m_ucBits  != m_pComponent[0].m_ucBits  ||
	    m_pComponent[i].m_bSigned != m_pComponent[0].m_bSigned ||
	    m_pComponent[i].m_bFloat  != m_pComponent[0].m_bFloat) {
	  PostError("Images with different bitdepths or number formats are not supported by the PPM format.\n");
	}  
	if (m_pComponent[i].m_ucSubX  != 1 ||
	    m_pComponent[i].m_ucSubY  != 1) {
	  PostError("PNM does not support subsampling.\n");
	}
      }
      prec = m_pComponent[0].m_ucBits;
      if (m_pComponent[0].m_bFloat) {
	if (prec != 32)
	  PostError("Unable to save PPM files with anything but 32 bit IEEE single precision float");
	Put('F');
      } else { 
	if (prec > 16)
	  PostError("PPM supports at most 16 bits per pixel");
	Put(raw?'6':'3');
      }
    } else {
      // unsupported type. Outch.
      PostError("Unsupported number of components for PPM output.\n");
    }
  }
  //
  // Write a line feed, and width and height.
  fprintf(m_pFile,"\n%lu %lu\n",
	  (unsigned long)(m_ulWidth),
	  (unsigned long)(m_ulHeight));
  //
  if (pfs) {
    fprintf(m_pFile,"%d\n0\n",m_usDepth);
    // Write channel names. Use Y for one channel, XYZ for three channels, CHx for n channels.
    if (m_usDepth == 1) {
      fprintf(m_pFile,"Y\n0\n");
    } else if (m_usDepth == 3) {
      fprintf(m_pFile,"X\n0\n");
      fprintf(m_pFile,"Y\n0\n");
      fprintf(m_pFile,"Z\n0\n");
    } else {
      for(i = 0;i < m_usDepth;i++) {
	fprintf(m_pFile,"CH%d\n0\n",i);
      }
    }
    Put('E');
    Put('N');
    Put('D');
    Put('H');
  } else {
    // If we are not PBM, write precision.
    if (prec == 32) {
      // Write the recorded radiance scale
      if (dolittle) {
	fprintf(m_pFile,"%g\n",-specs.RadianceScale);
      } else {
	fprintf(m_pFile,"%g\n",specs.RadianceScale);
      }
    } else if (prec > 1) {
      fprintf(m_pFile,"%lu\n",(1UL << prec) - 1);
    }
    // Get offset from the signed-ness of the component.
    if (m_pComponent[0].m_bSigned) {
      offset = (1UL << prec) >> 1;
    }
  }
  //
  // Check whether we shall write raw or ASCII.
  if (pfs) {
    for(i = 0;i < m_usDepth;i++) {
      ULONG *dt1 = (ULONG *)(m_pComponent[i].m_pPtr);
      //
      // Floating point, always little endian
      for(y=0;y<m_ulHeight;y++) {
	for(x=0;x<m_ulWidth;x++) {
	  Put(*dt1 >>  0);
	  Put(*dt1 >>  8);
	  Put(*dt1 >> 16);
	  Put(*dt1 >> 24);
	  dt1 = (ULONG *)((UBYTE *)(dt1) + m_pComponent[0].m_ulBytesPerPixel);
	}
	dt1 = (ULONG *)((UBYTE *)(dt1) - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel + m_pComponent[0].m_ulBytesPerRow);
      }
    }
  } else if (prec == 32) {
    ULONG *dt1 = (ULONG *)(m_pComponent[0].m_pPtr);
    ULONG *dt2 = (m_usDepth == 3)?((ULONG *)(m_pComponent[1].m_pPtr)):(NULL);
    ULONG *dt3 = (m_usDepth == 3)?((ULONG *)(m_pComponent[2].m_pPtr)):(NULL);
    //
    // Floating point.
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	if (dolittle) {
	  Put(*dt1 >>  0);
	  Put(*dt1 >>  8);
	  Put(*dt1 >> 16);
	  Put(*dt1 >> 24);
	} else {
	  Put(*dt1 >> 24);
	  Put(*dt1 >> 16);
	  Put(*dt1 >>  8);
	  Put(*dt1 >>  0);
	}
	dt1 = (ULONG *)((UBYTE *)(dt1) + m_pComponent[0].m_ulBytesPerPixel);
	if (m_usDepth == 3) {
	  if (dolittle) {
	    Put(*dt2 >>  0);
	    Put(*dt2 >>  8);
	    Put(*dt2 >> 16);
	    Put(*dt2 >> 24);
	  } else {
	    Put(*dt2 >> 24);
	    Put(*dt2 >> 16);
	    Put(*dt2 >>  8);
	    Put(*dt2 >>  0);
	  }
	  dt2 = (ULONG *)((UBYTE *)(dt2) + m_pComponent[1].m_ulBytesPerPixel);
	  if (dolittle) {
	    Put(*dt3 >>  0);
	    Put(*dt3 >>  8);
	    Put(*dt3 >> 16);
	    Put(*dt3 >> 24);
	  } else {
	    Put(*dt3 >> 24);
	    Put(*dt3 >> 16);
	    Put(*dt3 >>  8);
	    Put(*dt3 >>  0);
	  }
	  dt3 = (ULONG *)((UBYTE *)(dt3) + m_pComponent[2].m_ulBytesPerPixel);
	}
      }
      dt1 = (ULONG *)((UBYTE *)(dt1) - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel + m_pComponent[0].m_ulBytesPerRow);
      if (m_usDepth == 3) {
	dt2 = (ULONG *)((UBYTE *)(dt2) - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel + m_pComponent[1].m_ulBytesPerRow);
	dt3 = (ULONG *)((UBYTE *)(dt3) - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel + m_pComponent[2].m_ulBytesPerRow);
      }
    }
  } else if (prec == 1 && raw) {
    UBYTE *dat = (UBYTE *)(m_pComponent[0].m_pPtr);
    // bitpacked, one component only.
    for(y=0;y<m_ulHeight;y++) {
      int data   = 0;
      int mask   = 0x80;
      UBYTE *row = dat;
      for(x=0;x<m_ulWidth;x++) {	  
	if (mask == 0) {
	  Put(~data);
	  mask = 0x80;
	  data = 0;
	}
	if (*row) 
	  data |= mask;
	row    += m_pComponent[0].m_ulBytesPerPixel;
	mask >>= 1;
      }
      Put(~data);
      dat += m_pComponent[0].m_ulBytesPerRow;
    }
  } else if (prec <= 8) {
    UBYTE *p0,*p1 = NULL,*p2 = NULL; // shutup g++    
    UBYTE cnt = 0;
    p0 = (UBYTE *)(m_pComponent[0].m_pPtr);
    if (m_usDepth == 3) {
      p1 = (UBYTE *)(m_pComponent[1].m_pPtr);
      p2 = (UBYTE *)(m_pComponent[2].m_pPtr);
    }
    if (raw && offset == 0 && m_usDepth == 1 && m_pComponent[0].m_ulBytesPerPixel == 1 && 
	m_pComponent[0].m_ulBytesPerRow == m_ulWidth * m_pComponent[0].m_ulBytesPerPixel) {
      // Write fast in one block.
      fwrite(p0,1,m_pComponent[0].m_ulBytesPerRow * m_ulHeight,m_pFile);
    } else if (raw && m_usDepth == 3 && offset == 0 &&
	       m_pComponent[0].m_ulBytesPerPixel == 3 && 
	       m_pComponent[0].m_ulBytesPerRow   == m_ulWidth * m_pComponent[0].m_ulBytesPerPixel &&
	       m_pComponent[1].m_ulBytesPerPixel == 3 && 
	       m_pComponent[1].m_ulBytesPerRow   == m_ulWidth * m_pComponent[1].m_ulBytesPerPixel &&
	       m_pComponent[2].m_ulBytesPerPixel == 3 && 
	       m_pComponent[2].m_ulBytesPerRow   == m_ulWidth * m_pComponent[2].m_ulBytesPerPixel &&
	       p1 == p0 + 1 && p2 == p0 + 2) {
      // Ditto. Interleaved pixels.
      fwrite(p0,1,m_pComponent[0].m_ulBytesPerRow * m_ulHeight,m_pFile);
    } else {
      for(y=0;y<m_ulHeight;y++) {
	for(x=0;x<m_ulWidth;x++) {
	  if (raw) Put(*p0 + offset); else fprintf(m_pFile,"%3u ",*p0 + offset);
	  p0 += m_pComponent[0].m_ulBytesPerPixel;
	  if (m_usDepth == 3) {
	    if (raw) Put(*p1 + offset); else fprintf(m_pFile,"%3u ",*p1 + offset);
	    p1 += m_pComponent[1].m_ulBytesPerPixel;
	    if (raw) Put(*p2 + offset); else fprintf(m_pFile,"%3u ",*p2 + offset);
	    p2 += m_pComponent[2].m_ulBytesPerPixel;
	  }
	  if (!raw) {
	    cnt++;
	    if (cnt > 16) {
	      fprintf(m_pFile,"\n");
	      cnt = 0;
	    }
	  }
	}
	p0 += m_pComponent[0].m_ulBytesPerRow - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel;
	if (m_usDepth == 3) {
	  p1 += m_pComponent[1].m_ulBytesPerRow - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel;
	  p2 += m_pComponent[2].m_ulBytesPerRow - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel;
	}
      }
    }
  } else {
    UWORD *p0,*p1 = NULL,*p2 = NULL; // shutup g++
    UBYTE cnt = 0;
    p0 = (UWORD *)(m_pComponent[0].m_pPtr);
    if (m_usDepth == 3) {
      p1 = (UWORD *)(m_pComponent[1].m_pPtr);
      p2 = (UWORD *)(m_pComponent[2].m_pPtr);
    }
    for(y=0;y<m_ulHeight;y++) {
      for(x=0;x<m_ulWidth;x++) {
	if (raw) Put((*p0 + offset) >> 8),Put(*p0 + offset); else fprintf(m_pFile,"%3u ",*p0 + offset);
	p0 = (UWORD *)((UBYTE *)(p0) + m_pComponent[0].m_ulBytesPerPixel);
	if (m_usDepth == 3) {
	  if (raw) Put((*p1 + offset) >> 8),Put(*p1 + offset); else fprintf(m_pFile,"%3u ",*p1 + offset);
	  p1 = (UWORD *)((UBYTE *)(p1) + m_pComponent[1].m_ulBytesPerPixel);
	  if (raw) Put((*p2 + offset) >> 8),Put(*p2 + offset); else fprintf(m_pFile,"%3u ",*p2 + offset);
	  p2 = (UWORD *)((UBYTE *)(p2) + m_pComponent[2].m_ulBytesPerPixel);
	}
	if (!raw) {
	  cnt++;
	  if (cnt > 16) {
	    fprintf(m_pFile,"\n");
	    cnt = 0;
	  }
	}
      }
      p0 = (UWORD *)((UBYTE *)(p0) + m_pComponent[0].m_ulBytesPerRow - m_ulWidth * m_pComponent[0].m_ulBytesPerPixel);
      if (m_usDepth == 3) {
	p1 = (UWORD *)((UBYTE *)(p1) + m_pComponent[1].m_ulBytesPerRow - m_ulWidth * m_pComponent[1].m_ulBytesPerPixel);
	p2 = (UWORD *)((UBYTE *)(p2) + m_pComponent[2].m_ulBytesPerRow - m_ulWidth * m_pComponent[2].m_ulBytesPerPixel);
      }
    }
  }
  //
  if (ferror(m_pFile)) {
    PostError("IO error while writing the PPM file.\n");
  }
}
///
