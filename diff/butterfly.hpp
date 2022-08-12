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
** $Id: butterfly.hpp,v 1.1 2022/08/12 06:55:01 thor Exp $
**
** This class combines two images in the butterfly style and saves
** the result to a file.
*/

#ifndef DIFF_BUTTERFLY_HPP
#define DIFF_BUTTERFLY_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Butterfly
// This class combines two images in the butterfly style and saves
// the result to a file.
class Butterfly : public Meter, private ImageLayout {
  //
  // The file name under which the difference image shall be saved.
  const char            *m_pTargetFile;
  //
  // The component memory itself.
  UBYTE                **m_ppucImage;
  //
  // Specifications of the output file.
  const struct ImgSpecs &m_TargetSpecs;
  //
  template<typename T>
  static void Merge(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
		    const T *dst ,ULONG dbytesperpixel,ULONG dbytesperrow,
		    T *trg       ,ULONG tbytesperpixel,ULONG tbytesperrow,
		    ULONG w,ULONG h);
  //
public:
  //
  // Interleave two images in butterfly style
  Butterfly(const char *filename,const struct ImgSpecs &specs)
    : m_pTargetFile(filename), m_ppucImage(NULL), m_TargetSpecs(specs)
  {
  }
  //
  virtual ~Butterfly(void);
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

