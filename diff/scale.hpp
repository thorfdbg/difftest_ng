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
** $Id: scale.hpp,v 1.5 2016/02/12 16:12:43 thor Exp $
**
** This class scales images, converts them from and to float
*/

#ifndef DIFF_SCALE_HPP
#define DIFF_SCALE_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Scale
// This class converts images between various data types and scales them.
class Scale : public Meter, private ImageLayout {
  //
  // The file name under which the difference image shall be saved.
  const char  *m_pTargetFile;
  //
  // The component memory itself.
  UBYTE      **m_ppucImage;
  //
  // Convert to integer (from float)?
  bool         m_bMakeInt;
  //
  // Convert to float (from int)?
  bool         m_bMakeFloat;
  //
  // Convert to unsigned (from signed)?
  bool         m_bMakeUnsigned;
  //
  // Convert to signed (from unsigned)?
  bool         m_bMakeSigned;
  //
  // The desired output bitdepth. Only if ToInt is given,
  // zero if no scaling is considered.
  UBYTE        m_ucTargetDepth;
  //
  // Right-aligned padding into a higher bit-depths.
  bool         m_bPad;
  //
  // In case we map two images, here is a second buffer
  class Scale *m_pDest;
  //
  // Output specifications of the destination file.
  const struct ImgSpecs &m_TargetSpecs;
  //
  template<typename S,typename T>
  void Convert(const S *org ,ULONG obytesperpixel,ULONG obytesperrow,
	       T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
	       ULONG w, ULONG h,
	       double scale ,double shift,double min,double max);
  //
  //
  // Apply a scaling from the source to the image stored here.
  void ApplyScaling(class ImageLayout *src);
  //
public:
  //
  // Scale the difference image. Takes a file name. If the file name is NULL,
  // the scaler is run as a filter and the output is not saved to a file,
  // but changes the image in place.
  Scale(const char *filename,bool toint,bool tofloat,
	bool mkunsign,bool mksign,UBYTE targetdepth,bool pad,const struct ImgSpecs &specs)
    : m_pTargetFile(filename), m_ppucImage(NULL), 
      m_bMakeInt(toint), m_bMakeFloat(tofloat), 
      m_bMakeUnsigned(mkunsign), m_bMakeSigned(mksign),
      m_ucTargetDepth(targetdepth), m_bPad(pad), m_pDest(NULL),
      m_TargetSpecs(specs)
  {
  }
  //
  virtual ~Scale(void);
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
