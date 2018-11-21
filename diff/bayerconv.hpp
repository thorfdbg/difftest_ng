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
** $Id: bayerconv.hpp,v 1.5 2018/11/21 13:57:32 thor Exp $
**
** This class converts bayer pattern images into four-component images
** and back. It *does not* attempt to de-bayer the images.
*/

#ifndef DIFF_BAYERCONV_HPP
#define DIFF_BAYERCONV_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// class BayerConv
// This class converts bayer pattern images into four-component images
// or three-component 422 images and back.
// It *does not* attempt to de-bayer the images.
class BayerConv : public Meter, private ImageLayout {
  //
public:
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
  //
  // The component memory itself.
  UBYTE     **m_ppucSource;
  UBYTE     **m_ppucDestination;
  //
  // Number of allocated components
  UWORD       m_usAllocated;
  //
  // Templated extractor class. This takes a subpixel from the
  // source and copies it to the target.
  template<typename T>
  void ExtractSubPixels(ULONG width,T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
			const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
			ULONG subx,ULONG suby);
  //
  // Templated injector class. This creates subpixels from the source and
  // injects them into the target.
  template<typename T>
  void InsertSubPixels(ULONG width,T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		       const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
		       ULONG subx,ULONG suby);
  //
  // Direction of the conversion.
  // if true, then conversion from 4-component to bayer, otherwise reverse.
  bool        m_bToBayer;
  //
  // If true, use conversion to or from 422 subsampled to bayer where the
  // green component is assigned to luma and red and blue to the chroma
  // components. This is ugly, but just another representation.
  bool        m_b422;
  //
  // If true, bayer components are always reshuffled into RGGB order.
  bool        m_bReshuffle;
  //
  // The sample arrangement for the 422 conversion.
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
  // Release the memory for the target components
  // that have been allocated.
  void ReleaseComponents(UBYTE **&p);
  //
  // Convert from Bayer to four-components.
  void ConvertFromBayer(UBYTE **&dest,class ImageLayout *src);
  //
  // Convert from four-component to Bayer
  void ConvertToBayer(UBYTE **&dest,class ImageLayout *src);
  //
  // Convert from Bayer to 422 three-components.
  void Convert422FromBayer(UBYTE **&dest,class ImageLayout *src);
  //
  // Convert from 422 three-component to Bayer
  void Convert422ToBayer(UBYTE **&dest,class ImageLayout *src);
  //
  // Create the image data from the dimensions computed
  void CreateImageData(UBYTE **&data,class ImageLayout *src);
  //
public:
  // This gets a single parameter indicating the conversion direction. True if the conversion
  // direction is from 4-component to bayer, false if conversion from bayer to 4-components.
  // If reshuffle is set, the components are always brought into the order RGGB.
  BayerConv(bool tobayer,bool is422,bool reshuffle,SampleArrangement s = RGGB)
    : m_ppucSource(NULL),m_ppucDestination(NULL), m_usAllocated(0),
      m_bToBayer(tobayer), m_b422(is422), m_bReshuffle(reshuffle)
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
  virtual ~BayerConv(void);
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
