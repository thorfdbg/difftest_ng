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
** $Id: flipextend.hpp,v 1.1 2019/07/24 13:04:44 thor Exp $
**
** This class flips images over vertically and horizonally, doubling
** their size.
*/

#ifndef DIFF_FLIPEXTEND_HPP
#define DIFF_FLIPEXTEND_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class FlipExtend
// This class flips images over vertically and horizonally, doubling
// their size.
class FlipExtend : public Meter, private ImageLayout {
  //
public:
  // The direction within which to proceed.
  enum FlipDirection {
    FlipX,
    FlipY
  };
  //
private:
  //
  // The component memory itself.
  UBYTE           **m_ppucImage;
  //
  // How to flip.
  FlipDirection     m_Dir;
  //
  // In case we map two images, here is a second buffer
  class FlipExtend *m_pDest;
  //
  template<typename T>
  void ExtendHorizontal(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
			T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
			ULONG w, ULONG h);
  //
  template<typename T>
  void ExtendVertical(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
		      T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		      ULONG w, ULONG h);
  //
  //
  // Apply the flip-over from the given layout to the image stored here.
  void ApplyExtension(class ImageLayout *src);
  //
public:
  //
  // Extend source and target image by flipping them over.
  // Scale the difference image. Takes a file name. If the file name is NULL,
  // the scaler is run as a filter and the output is not saved to a file,
  // but changes the image in place.
  FlipExtend(FlipDirection dir)
    : m_ppucImage(NULL), m_Dir(dir), m_pDest(NULL)
  {  }
  //
  virtual ~FlipExtend(void);
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
