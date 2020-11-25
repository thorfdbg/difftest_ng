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
** $Id: tobayer.hpp,v 1.1 2020/11/25 08:13:52 thor Exp $
**
** This class converts an RGB image to a bayer image of a given bayer
** pattern arrangement. It just performs a simple sampling, not
** filtering.
*/

#ifndef DIFF_TOBAYER_HPP
#define DIFF_TOBAYER_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class ToBayer
// This class converts an RGB image to a bayer pattern image of a given bayer arrangement.
class ToBayer : public Meter, private ImageLayout {
  //
public:
  //
  // Debayer sample arrangements.
  enum SampleArrangement {
    GRBG,
    RGGB,
    GBRG,
    BGGR
  };
  //
private:
  // The component output memory itself.
  UBYTE      *m_pucSource;
  UBYTE      *m_pucDestination;
  //
  // The sample arrangement.
  enum SampleArrangement m_Pattern;
  //
  // The actual Debayer algorithm.
  template<typename T>
  void SampleData(const T *r,const T *g,const T *b,
		  LONG rbytesperpixel,LONG rbytesperrow,
		  LONG gbytesperpixel,LONG gbytesperrow,
		  LONG bbytesperpixel,LONG bbytesperrow,
		  T *dst,ULONG width,ULONG height);
  //
  // Sampling of the source image to the target layout.
  void Sample(UBYTE *&dest,class ImageLayout *src);
  //
  // Release the memory for the target components
  // that have been allocated.
  void ReleaseComponents(UBYTE *&p);
  //
  // Create the image data from the dimensions computed
  void CreateImageData(UBYTE *&data,class ImageLayout *src);
  //
public:
  //
  ToBayer(SampleArrangement s)
    : m_pucSource(NULL), m_pucDestination(NULL), m_Pattern(s)
  { }
  //
  virtual ~ToBayer(void);
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
