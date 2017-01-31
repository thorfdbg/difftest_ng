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
 * An image class to load and save uncompressed RGBE images
 * from a file or to a file.
 * $Id: simplergbe.hpp,v 1.7 2017/01/31 11:58:04 thor Exp $
 */

#ifndef SIMPLERGBE_HPP
#define SIMPLERGBE_HPP

/// Includes
#include "imglayout.hpp"
#include "std/stdio.hpp"
#include "std/math.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// SimpleRGBE
// This class loads and saves RGBE images. RLE encoding seems to
// be misdefined as it cannot identify with certainty that a row is really RLE
// compressed or not.
class SimpleRGBE : public ImageLayout {
  // This is the image raw data. Note that the data is stored internally
  // as floating point triples.
  FLOAT *m_pfImage;
  //
  // For shortcutting: The file we read from/write to.
  FILE  *m_pFile;
  //
  // The last character we read. For un-getting.
  int    m_iLastChar;
  //
  // A temporary buffer used for runlength compression
  UBYTE *m_pucTmp;
  //
  // Read an ascii string from the input file,
  // encoding a number. This number gets returned. Throws on error.
  LONG ReadNumber(void);
  //
  // Skip blank spaces in the bytestream.
  void SkipBlanks(void);
  //
  // Skip comment lines starting with !
  void SkipComment(void);
  //
  // Get the next field, starting with non-! and ending with =
  void ReadField(char field[256]);
  //
  // Read the value associated to a field
  void ReadFieldValue(char value[256]);
  //
  // Read a byte, throw on EOF.
  LONG Get(void)
  {
    return (m_iLastChar = getc(m_pFile));
  }
  void LastUnDo(void)
  {
    ungetc(m_iLastChar,m_pFile);
  }
  void Put(UBYTE c)
  {
    putc(c,m_pFile);
  }
  //
  // Convert an RGBE quadrupel to an RGB tripel in float
  static void RGBE2Float(LONG r,LONG g,LONG b,LONG e,FLOAT *rgb)
  {
    FLOAT scale = 0.0f;

    if (e != 0) { // nonzero exponent, thus nonzero pixel
      scale  = ldexp(1.0,e-128-8);
    }
    rgb[0] = r * scale;
    rgb[1] = g * scale;
    rgb[2] = b * scale;
  }
  //
  // Convert rgb values if float into an RGBE quadrupel.
  void WriteRGBE(FLOAT r,FLOAT g,FLOAT b)
  {
    UBYTE rgbe[4];
    FLOAT max = 0.0;
    FLOAT scale = 0.0;
    int e = -128;

    if (r > max)
      max = r;
    if (g > max)
      max = g;
    if (b > max)
      max = b;

    if (max > 0.0f)
      scale = frexp(max,&e) * 256.0f / max; // scale will be zero and exponent minimal otherwise
      
    if (e > 127) {
      rgbe[0] = (r > 0.0)?(0xff):(0x00);
      rgbe[1] = (g > 0.0)?(0xff):(0x00);
      rgbe[2] = (b > 0.0)?(0xff):(0x00);
      rgbe[3] = 0xff;
    } else if (e <= -128) { // zero pixel, no denormalization
      rgbe[0] = 0;
      rgbe[1] = 0;
      rgbe[2] = 0;
      rgbe[3] = 0;
    } else {
      r *= scale;
      if (r > 255.0f) {
	rgbe[0] = 0xff;
      } else if (r < 0.0f) {
	rgbe[0] = 0x00;
      } else {
	rgbe[0] = r;
      }
      g *= scale;
      if (g > 255.0f) {
	rgbe[1] = 0xff;
      } else if (g < 0.0f) {
	rgbe[1] = 0x00;
      } else {
	rgbe[1] = g;
      }
      b *= scale;
      if (b > 255.0f) {
	rgbe[2] = 0xff;
      } else if (b < 0.0f) {
	rgbe[2] = 0x00;
      } else {
	rgbe[2] = b;
      }
      rgbe[3] = e + 128;
    }

    Put(rgbe[0]);
    Put(rgbe[1]);
    Put(rgbe[2]);
    Put(rgbe[3]);
  }
  //
  // Read a non-RLE coded version of a hdr file.
  void LoadLineUncompressed(ULONG width,FLOAT *buffer);
  //
  // Read an RLE-encoded version of a hdr file.
  void LoadLineCompressed(ULONG width,FLOAT *buffer);
  //
public:
  //
  // default constructor
  SimpleRGBE(void);
  // Copy the image from another source for later saving.
  SimpleRGBE(const class ImageLayout &layout);
  // destructor
  ~SimpleRGBE(void);
  //
  // Save an image to a level 1 file descriptor, given its
  // width, height and depth. 
  void SaveImage(const char *filename,const struct ImgSpecs &specs);
  //
  // Load an image from a level 1 file descriptor, keep it within
  // the internals of this class. The accessor methods below
  // should be used to find out more about this image.
  void LoadImage(const char *basename,struct ImgSpecs &specs);
};
///

///
#endif
