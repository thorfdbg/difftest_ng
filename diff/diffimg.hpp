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
** $Id: diffimg.hpp,v 1.13 2017/01/31 11:58:03 thor Exp $
**
** This class saves the difference image as a normalized 8bpp image
** with the same number of components as the original.
*/

#ifndef DIFF_DIFFIMG_HPP
#define DIFF_DIFFIMG_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class DiffImg
// This class saves the difference image as a normalized 8bpp image
// with the same number of components as the original.
class DiffImg : public Meter, private ImageLayout {
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
  // Scale to range.
  bool                   m_bScale;
  //
  template<typename T,typename D>
  static void Adjust(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
		     T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		     APTR trg     ,ULONG tbytesperpixel,ULONG tbytesperrow,D dmin,D dmax,
		     double &scale,double &shift,ULONG w,ULONG h);
  //
public:
  //
  // Construct the difference image. Takes a file name.
  DiffImg(const char *filename,const struct ImgSpecs &specs,bool scale)
    : m_pTargetFile(filename), m_ppucImage(NULL), m_TargetSpecs(specs), m_bScale(scale)
  {
  }
  //
  virtual ~DiffImg(void);
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

