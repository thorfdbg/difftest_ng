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
 * An image class to load and save uncompressed PPM/PGM/PBM/PFM images
 * from a file or to a file.
 * $Id: simpleppm.hpp,v 1.7 2014/04/19 21:03:20 thor Exp $
 */

#ifndef SIMPLEPPM_HPP
#define SIMPLEPPM_HPP

/// Includes
#include "imglayout.hpp"
#include "std/stdio.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// SimplePpm
// This is the class for simple portable pixmap graphics.
class SimplePpm : public ImageLayout {
  // This is the image raw data
  // the data is in order {r,g,b} for colored pictures.
  UBYTE *m_pucImage;
  // This pointer is for word oriented images.
  UWORD *m_pusImage;
  // This pointer is for floating point images.
  FLOAT *m_pfImage;
  //
  // For shortcutting: The file we read from/write to.
  FILE  *m_pFile;
  //
  // The last character we read. For un-getting.
  int    m_iLastChar;
  //
  // Read an ascii string from the input file,
  // encoding a number. This number gets returned. Throws on error.
  LONG ReadNumber(void);
  //
  // Skip blank spaces in the bytestream.
  void SkipBlanks(void);
  //
  // Skip comment lines starting with #
  void SkipComment(void);
  //
  // Skip an entire line completely
  void SkipLine(void);
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
  //
public:
  //
  // default constructor
  SimplePpm(void);
  // Copy the image from another source for later saving.
  SimplePpm(const class ImageLayout &layout);
  // destructor
  ~SimplePpm(void);
  //
  // Save an image to a level 1 file descriptor, given its
  // width, height and depth. We only support grey level and
  // RGB here, no palette images.
  void SaveImage(const char *basename,const struct ImgSpecs &specs,bool pfs = false);
  //
  // Load an image from a level 1 file descriptor, keep it within
  // the internals of this class. The accessor methods below
  // should be used to find out more about this image.
  void LoadImage(const char *basename,struct ImgSpecs &specs);
};
///

///
#endif
