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
** $Id: upsampler.hpp,v 1.3 2016/06/04 10:44:09 thor Exp $
**
** This class upsamples in the spatial domain
*/

#ifndef DIFF_UPSAMPLER_HPP
#define DIFF_UPSAMPLER_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Upsampler
// This class upsamples images in the spatial domain
class Upsampler : public Meter, private ImageLayout {
  //
  // The component memory itself.
  UBYTE     **m_ppucSource;
  UBYTE     **m_ppucDestination;
  //
  // Scaling coordinates.
  UBYTE       m_ucScaleX,m_ucScaleY;
  //
  // Set if only the chroma component is subsampled.
  bool        m_bChromaOnly;
  //
  // Set if the sample position is cosited instead of centered.
  bool        m_bCosited;
  //
  // Release the temporary buffer.
  void ReleaseComponents(UBYTE **p);
  //
  // Perform the actual downsampling.
  void Upsample(UBYTE **&data,class ImageLayout *src);
  //
  template<typename S>
  void BilinearFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
		      S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
		      ULONG w,ULONG h,
		      S min,S max,
		      int sx,int sy);
  //
  //
public:
  Upsampler(UBYTE sx,UBYTE sy,bool chromaonly,bool cosited)
    : m_ppucSource(NULL),m_ppucDestination(NULL),
      m_ucScaleX(sx), m_ucScaleY(sy), m_bChromaOnly(chromaonly), m_bCosited(cosited)
  { }
  //
  virtual ~Upsampler(void);
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


  
