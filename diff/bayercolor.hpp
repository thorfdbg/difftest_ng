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
** $Id: bayercolor.hpp,v 1.5 2019/03/06 14:38:31 thor Exp $
**
** This class implements various color transformations on CFA data,
** specific to CFA only. They generally depend on the Bayer layout,
** unlike the simpler transformations in the YCbCr class.
*/

#ifndef DIFF_BAYERCOLOR_HPP
#define DIFF_BAYERCOLOR_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "std/string.hpp"
///

/// Forwards
class ImgSpecs;
///

/// class BayerColor
// This class implements various color transformations on CFA data,
// specific to CFA only. They generally depend on the Bayer layout,
// unlike the simpler transformations in the YCbCr class.
class BayerColor : public Meter, private ImageLayout {
  //
  // This bool is set for the backwards transformation, i.e.
  // YCbCrD->RGGB
  bool m_bInverse;
  //
  // The component memory itself. This class only works for
  // 1-component Bayer images, i.e. frombayer/tobayer is not
  // needed here.
  UBYTE *m_pucSrcImage;
  UBYTE *m_pucDstImage;
  //
public:
  // The conversion to run
  enum Conversion {
    RCTD,          // This is the standard RCTD where the green average is only taken in one sub-pixel.
    RCTX,          // This is an RCT where green is averaged over four neighbours.
    YDgCoCgX       // This is the lifted YCgCo transformation as found in the ICIP paper from Taizo Suzuki
  };
  //
  // Bayer sample arrangements.
  // This is only required for the 422 conversion
  enum SampleArrangement {
    GRBG,
    RGGB,
    GBRG,
    BGGR
  };
  //
private:
  Conversion  m_Conversion;
  //
  // Sample positions in the 2x2 superpixel, first for the
  // red subpixel.
  LONG        m_lrx,m_lry;
  // The sample positions of the first green subpixel
  LONG        m_lgx,m_lgy;
  // The sample positions of the second green subpixel
  LONG        m_lkx,m_lky;
  // The sample positions of the blue subpixel
  LONG        m_lbx,m_lby;
  //
  // Depending on the type of the decorrelation, depatch into the corresponding
  // implementation.
  template<typename S,typename T>
  void DispatchDecorrelation(const S *src,T *dst,LONG offset,
			     ULONG srcbytesperpixel,ULONG srcbytesperrow);
  //
  // Same for the inverse decorrelation.
  template<typename S,typename T>
  void DispatchInverseDecorrelation(const S *src,T *dst,LONG offset,
				    ULONG srcbytesperpixel,ULONG srcbytesperrow);
  //
  // Conversions to and from RCTD. All these transformations
  // are lossless, and there is no clamping, but the range
  // may be extended.
  template<typename S,typename T>
  void ToRCTD(const S *in,T *out,LONG chromaoffset,
	      ULONG sbpp,ULONG sbpr,
	      ULONG tbpp,ULONG tbpr,
	      LONG w    ,LONG h);
  //
  template<typename S,typename T>
  void FromRCTD(const S *in,T *out,LONG chromaoffset,
		ULONG sbpp,ULONG sbpr,
		ULONG tbpp,ULONG tbpr,
		LONG w    ,LONG h);
  //
  // Conversions to and from RCTX. All these transformations
  // are lossless, and there is no clamping, but the range
  // may be extended.
  template<typename S,typename T>
  void ToRCTX(const S *in,T *out,LONG chromaoffset,
	      ULONG sbpp,ULONG sbpr,
	      ULONG tbpp,ULONG tbpr,
	      LONG w    ,LONG h);
  //
  template<typename S,typename T>
  void FromRCTX(const S *in,T *out,LONG chromaoffset,
		ULONG sbpp,ULONG sbpr,
		ULONG tbpp,ULONG tbpr,
		LONG w    ,LONG h);
  //
  // Conversion to and from YDgCoCg-X transformation
  template<typename S,typename T>
  void ToYDgCoCgX(const S *in,T *out,LONG chromaoffset,
		  ULONG sbpp,ULONG sbpr,
		  ULONG tbpp,ULONG tbpr,
		  LONG  w   ,LONG h);
  //
  template<typename S,typename T>
  void FromYDgCoCgX(const S *in,T *out,LONG chromaoffset,
		    ULONG sbpp,ULONG sbpr,
		    ULONG tbpp,ULONG tbpr,
		    LONG  w   ,LONG h);
  //
  // Allocate the arrays and initialize this image to
  // a layout that is identical to the source.
  void CreateImage(UBYTE *&dest,class ImageLayout *src,bool extendsrange);
  //
  // Forwards transform into the target argument given first from the
  // single component image given as source.
  void Decorrelate(UBYTE *&target,class ImageLayout *src);
  //
  // Backards transform into the target argument given first from
  // the single component image given as source
  void InverseDecorrelate(UBYTE *&target,class ImageLayout *src);
  //
public:
  BayerColor(bool inverse,Conversion conv,SampleArrangement s=RGGB)
    : m_bInverse(inverse), m_pucSrcImage(NULL), m_pucDstImage(NULL),
      m_Conversion(conv)
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
  virtual ~BayerColor(void);
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

