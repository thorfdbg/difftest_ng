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
** $Id: upsampler.hpp,v 1.10 2025/12/01 09:06:43 thor Exp $
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
public:
  enum FilterType {
    Centered,
    Cosited,
    Boxed
  };
  //
private:
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
  // Set if only the chroma component is subsampled.
  bool        m_bChromaOnly;
  //
  // Set if the upsampling is automatic to 1x1
  bool        m_bAutomatic;
  //
  FilterType  m_FilterType;
  //
  // Is upsampling limited to a region?
  bool        m_bRegional;
  //
  // Definition of the region that is not upsampled.
  LONG        m_lX1,m_lY1;
  LONG        m_lX2,m_lY2;
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
  template<typename S>
  void RegionalBilinearFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
			      S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
			      ULONG w,ULONG h,LONG x1,LONG y1,LONG x2,LONG y2,
			      S min,S max);
  //
  template<typename S>
  void BoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
		 S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
		 ULONG w,ULONG h,
		 int sx,int sy);
  //
  template<typename S>
  void RegionalBoxFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
			 S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
			 ULONG w,ULONG h,LONG x1,LONG y1,LONG x2,LONG y2);
  //
  // Compute the width of the upsampled image.
  ULONG UpsampledWidth(ULONG width) const
  {
    if (m_bRegional) {
      if (m_lX2 >= LONG(width))
	throw "non-upsampled region is larger than the image";
      return m_lX2 - m_lX1 + m_lX1 * m_ucScaleX + (width - m_lX2) * m_ucScaleX;
    } else {
      return width * m_ucScaleX;
    }
  }
  //
  // Compute the height of the upsampled image.
  ULONG UpsampledHeight(ULONG height) const
  {
    if (m_bRegional) {
      if (m_lY2 >= LONG(height))
	throw "non-upsampled region is larger than the image";
      return m_lY2 - m_lY1 + m_lY1 * m_ucScaleY + (height - m_lY2) * m_ucScaleY;
    } else {
      return height * m_ucScaleY;
    }
  }
  //
public:
  Upsampler(UBYTE sx,UBYTE sy,bool chromaonly,FilterType type,bool automatic = false)
    : m_ppucSource(NULL),m_ppucDestination(NULL), m_usAllocated(0),
      m_ucScaleX(sx), m_ucScaleY(sy), m_bChromaOnly(chromaonly),
      m_bAutomatic(automatic), m_FilterType(type), m_bRegional(false)
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
  //
  // Limit upsampling to a region outside of the given rectangle.
  void LimitRegion(LONG x1,LONG y1,LONG x2,LONG y2)
  {
    if (m_ucScaleX != 2 || m_ucScaleY != 2)
      throw "only upsampling factors 2,2 are supported for regional upscaling";
    if (m_bChromaOnly)
      throw "regional upsampling cannot be combined with chroma upsampling";
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


  
