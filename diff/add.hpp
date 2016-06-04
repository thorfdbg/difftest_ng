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
** $Id: add.hpp,v 1.4 2016/06/04 10:44:08 thor Exp $
**
** This class adds the second image to the first and merges the two
** under a common name.
*/

#ifndef DIFF_ADD_HPP
#define DIFF_ADD_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class AddImg
// This class adds the second image to the first and merges the two
// under a common name.
class AddImg : public Meter, private ImageLayout {
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
  AddImg(const char *filename,const struct ImgSpecs &spec)
    : m_pcTargetFile(filename), m_TargetSpecs(spec)
  {
  }
  //
  virtual ~AddImg(void)
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
