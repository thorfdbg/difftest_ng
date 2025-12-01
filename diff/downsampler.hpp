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
** $Id: downsampler.hpp,v 1.8 2025/12/01 09:06:43 thor Exp $
**
** This class downscales in the spatial domain
*/

#ifndef DIFF_DOWNSAMPLER_HPP
#define DIFF_DOWNSAMPLER_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Downsampler
// This class downsamples images in the spatial domain by a simple box filter.
class Downsampler : public Meter, private ImageLayout {
  //
  // The component memory itself.
  UBYTE     **m_ppucSource;
  UBYTE     **m_ppucDestination;
  //
  // Number of allocated components
  UWORD       m_usAllocated;
  //
  // Scaling coordinates.
  UBYTE       m_ucScaleX,m_ucScaleY;
  //
  // Edges of the region that is not downsampled.
  LONG        m_lX1,m_lY1;
  LONG        m_lX2,m_lY2;
  //
  // Set if only the chroma component is subsampled.
  bool        m_bChromaOnly;
  //
  // Set if downsampling is constrained to be outside of a
  // given region.
  bool        m_bRegional;
  //
  // Release the temporary buffer.
  void ReleaseComponents(UBYTE **p);
  //
  // Perform the actual downsampling.
  void Downsample(UBYTE **&data,class ImageLayout *src);
  //
  template<typename S>
  void BoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
		 S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
		 ULONG w,ULONG h,
		 S min,S max,
		 int sx,int sy);
  //
  template<typename S>
  void RegionalBoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
			 S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
			 ULONG w,ULONG h,LONG x1,LONG y1,LONG x2,LONG y2,
			 S min,S max);
  //
  //
  ULONG DownsampledWidth(ULONG w) const
  {
    if (m_bRegional) {
      if (LONG(w) < m_lX2)
	throw "the end coordinate X2 of --outside must be smaller than the width";
      return (m_lX1 + m_ucScaleX - 1) / m_ucScaleX + (m_lX2 - m_lX1) + (w + m_ucScaleX - 1) / m_ucScaleX - (m_lX2 + m_ucScaleX - 1) / m_ucScaleX;
    } else {
      return (w + m_ucScaleX - 1) / m_ucScaleX;
    }
  }
  //
  ULONG DownsampledHeight(ULONG h) const
  {
    if (m_bRegional) {
      if (LONG(h) < m_lY2)
	throw "the end coordinate Y2 of --outside must be smaller than the height";
      return (m_lY1 + m_ucScaleY - 1) / m_ucScaleY + (m_lY2 - m_lY1) + (h + m_ucScaleY - 1) / m_ucScaleY - (m_lY2 + m_ucScaleY - 1) / m_ucScaleY;
    } else {
      return (h + m_ucScaleY - 1) / m_ucScaleY;
    }
  }
  //
public:
  Downsampler(UBYTE sx,UBYTE sy,bool chromaonly)
    : m_ppucSource(NULL),m_ppucDestination(NULL), m_usAllocated(0),
      m_ucScaleX(sx), m_ucScaleY(sy), m_bChromaOnly(chromaonly), m_bRegional(false)
  { }
  //
  virtual ~Downsampler(void);
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    return NULL;
  }
  //
  // Limit upsampling to a region outside of the given rectangle.
  void LimitRegion(LONG x1,LONG y1,LONG x2,LONG y2)
  {
    if (m_ucScaleX != 2 || m_ucScaleY != 2)
      throw "only upsampling factors 2,2 are supported for regional downscaling";
    if (m_bChromaOnly)
      throw "regional downsampling cannot be combined with chroma downsampling";
    if (m_lX1 > m_lX2 || m_lY1 > m_lY2)
      throw "start coordinate of --outside must be smaller than end coordinate";
    m_bRegional = true;
    m_lX1       = x1;
    m_lY1       = y1;
    m_lX2       = x2;
    m_lY2       = y2;
  }
};
///

///
#endif


  
