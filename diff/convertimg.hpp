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
**
** $Id: convertimg.hpp,v 1.3 2014/01/04 11:35:27 thor Exp $
**
** This class saves the original image unmodified for conversion and testing
** purposes.
*/

#ifndef DIFF_CONVERT_HPP
#define DIFF_CONVERT_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class ConvertImg
// This class saves the original image unmodified for conversion and testing
// purposes.
class ConvertImg : public Meter {
  //
  // The file name under which the difference image shall be saved.
  const char            *m_pcTargetFile;
  //
  // The specificiations of the output file.
  const struct ImgSpecs &m_TargetSpecs;
  //
public:
  //
  // Construct the difference image. Takes a file name.
  ConvertImg(const char *filename,const struct ImgSpecs &spec)
    : m_pcTargetFile(filename), m_TargetSpecs(spec)
  {
  }
  //
  virtual ~ConvertImg(void)
  {
  }
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    return NULL;
  }
};
///

///
#endif
