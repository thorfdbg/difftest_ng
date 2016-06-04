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
 * An image class to load and save EXR images
 * from a file or to a file.
 * $Id: simpleexr.hpp,v 1.4 2016/06/04 10:44:09 thor Exp $
 */

#ifndef SIMPLEEXR_HPP
#define SIMPLEEXR_HPP

/// Includes
#include "imglayout.hpp"
#include "std/stdio.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// SimpleEXR
// This is the class for simple EXR graphics.
#ifdef USE_EXR
class SimpleEXR : public ImageLayout {
  //
  // This is the image raw data
  FLOAT *m_pfImage;
  //
public:
  //
  // default constructor
  SimpleEXR(void);
  // Copy the image from another source for later saving.
  SimpleEXR(const class ImageLayout &layout);
  // destructor
  ~SimpleEXR(void);
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
#endif
///

///
#endif
