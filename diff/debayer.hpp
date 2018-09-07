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
** $Id: debayer.hpp,v 1.3 2018/09/07 07:39:27 thor Exp $
**
** This class runs a debayer filter on the image converting it from grey scale
** to RGB. This is mostly experimental.
*/

#ifndef DIFF_DEBAYER_HPP
#define DIFF_DEBAYER_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Debayer
// This class saves the original image unmodified for conversion and testing
// purposes.
class Debayer : public Meter, private ImageLayout {
  //
public:
  // Various debayering methods.
  enum Method {
    Bilinear,
    ADH // Adaptive Homogeneity-Directed Demosaic Algorithm, by Keigo Hirakawa
  };
  //
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
  // The component memory itself.
  UBYTE     **m_ppucSource;
  UBYTE     **m_ppucDestination;
  //
  // Number of allocated components
  UWORD       m_usAllocated;
  //
  // The sample positions of the red subpixel.
  LONG        m_lrx,m_lry;
  //
  // The sample positions of the first green subpixel
  LONG        m_lgx,m_lgy;
  // The sample positions of the second green subpixel
  LONG        m_lkx,m_lky;
  //
  // The sample positions of the blue subpixel
  LONG        m_lbx,m_lby;
  //
  // The upsampling method.
  Method      m_Method;
  //
  // The actual Debayer algorithm.
  template<typename T,typename S>
  void BilinearKernel(const T *src,LONG bytesperpixel,LONG bytesperrow,
		      T *r,T *g,T *b,S min,S max);
  //
  // Run the ADH kernel. This requires additional arrays of the size of
  // the full image.
  template<typename T>
  void ADHKernel(const T *src,
		 FLOAT *horr,FLOAT *verr,
		 FLOAT *horg,FLOAT *verg,
		 FLOAT *horb,FLOAT *verb,
		 FLOAT *horl ,FLOAT *verl,
		 FLOAT *horca,FLOAT *verca,
		 FLOAT *horcb,FLOAT *vercb,
		 LONG bytesperpixel,LONG bytesperrow,
		 T *rp,T *gp,T *bp,FLOAT min,FLOAT max);
  //
  // Bilinear interpolation.
  void BilinearInterpolate(UBYTE **&dest,class ImageLayout *src);
  //
  // This is a smarter interpolation ported from dcraw, which again
  // uses the algorithm from Keigo Hirakawa and Thomas W. Parks:
  // "Adapaptive Homogeneity directed demosaicing algorithm"
  void ADHInterpolate(UBYTE **&dest,class ImageLayout *src);
  //
  // Release the memory for the target components
  // that have been allocated.
  void ReleaseComponents(UBYTE **&p);
  //
  // Create the image data from the dimensions computed
  void CreateImageData(UBYTE **&data,class ImageLayout *src);
  //
public:
  //
  Debayer(Method m,SampleArrangement s)
    : m_ppucSource(NULL), m_ppucDestination(NULL), m_usAllocated(0), m_Method(m)
  {
    switch(s) {
    case GRBG:
      m_lrx = 1,m_lry = 0;
      m_lgx = 0,m_lgy = 0;
      m_lkx = 1,m_lky = 1;
      m_lbx = 0,m_lby = 1;
      break;
    case RGGB:
      m_lrx = 0,m_lry = 0;
      m_lgx = 1,m_lgy = 0;
      m_lkx = 0,m_lky = 1;
      m_lbx = 1,m_lby = 1;
      break;
    case GBRG:
      m_lrx = 0,m_lry = 1;
      m_lgx = 0,m_lgy = 0;
      m_lkx = 1,m_lky = 1;
      m_lbx = 1,m_lby = 0;
      break;
    case BGGR:
      m_lrx = 1,m_lry = 1;
      m_lgx = 1,m_lgy = 0;
      m_lkx = 0,m_lky = 1;
      m_lbx = 0,m_lby = 0;
      break;
    }
  }
  //
  virtual ~Debayer(void);
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
