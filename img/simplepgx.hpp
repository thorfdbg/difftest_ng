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
 * An image class to load and save uncompressed PGX images.
 * This is the image file format that is defined by JPEG2000 part 4
 * for encoding the test streams.
 *
 * $Id: simplepgx.hpp,v 1.9 2016/06/04 10:44:09 thor Exp $
 */

#ifndef SIMPLEPGX_HPP
#define SIMPLEPGX_HPP

/// Includes
#include "imglayout.hpp"
#include "std/string.hpp"
#include "std/stdio.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// SimplePgx
// This is the class for simple portable extended pixmap graphics.
class SimplePgx : public ImageLayout {
  // This is the image raw data
  // the data is in order {r,g,b} for colored pictures.
  UBYTE *m_pucImage;
  // This pointer is for word oriented images.
  UWORD *m_pusImage;
  //
  // The last character we read. For un-getting.
  int    m_iLastChar;
  //
  // A per-component buffer containing the component names.
  struct ComponentName {
    struct ComponentName *m_pNext;
    char * const          m_pName;
    //
    // Dimensions of the file.
    ULONG                 m_ulWidth;
    ULONG                 m_ulHeight;
    UBYTE                 m_ucDepth;
    bool                  m_bSigned;
    bool                  m_bFloat; // is floating point (IEEE format)
    bool                  m_bLE; // endian-ness: true if little-endian
    //
    // The memory. This really administrates it.
    UBYTE                *m_pData;
    //
    ComponentName(const char *name)
      : m_pNext(NULL), m_pName(new char[strlen(name) + 1]), 
	m_ulWidth(0), m_ulHeight(0), m_ucDepth(0),
	m_bSigned(false), m_bFloat(false), m_bLE(false), m_pData(NULL)
    {
      strcpy(m_pName,name);
    }
    ~ComponentName(void)
    {
      delete[] m_pName;
      delete[] m_pData;
    }
  }     *m_pNameList;
  //
public:
  //
  // default constructor
  SimplePgx(void);
  // Copy the image from another source for later saving.
  SimplePgx(const class ImageLayout &layout);
  // destructor
  ~SimplePgx(void);
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
