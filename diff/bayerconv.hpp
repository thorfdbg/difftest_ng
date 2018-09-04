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
** $Id: bayerconv.hpp,v 1.2 2018/09/04 11:44:48 thor Exp $
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
// and back. It *does not* attempt to de-bayer the images.
class BayerConv : public Meter, private ImageLayout {
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
  void ExtractSubPixels(T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
			const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
			ULONG subx,ULONG suby);
  //
  // Templated injector class. This creates subpixels from the source and
  // injects them into the target.
  template<typename T>
  void InsertSubPixels(T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		       const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
		       ULONG subx,ULONG suby);
  //
  // Direction of the conversion.
  bool     m_bToBayer; // if true, then conversion from 4-component to bayer, otherwise reverse.
  //
  // Release the memory for the target components
  // that have been allocated.
  void ReleaseComponents(UBYTE **p);
  //
  // Convert from Bayer to four-components.
  void ConvertFromBayer(UBYTE **&dest,class ImageLayout *src);
  //
  // Convert from four-component to Bayer
  void ConvertToBayer(UBYTE **&dest,class ImageLayout *src);
  //
  // Create the image data from the dimensions computed
  void CreateImageData(UBYTE **&data,class ImageLayout *src);
  //
public:
  // This gets a single parameter indicating the conversion direction. True if the conversion
  // direction is from 4-component to bayer, false if conversion from bayer to 4-components.
  BayerConv(bool tobayer)
    : m_ppucSource(NULL),m_ppucDestination(NULL), m_usAllocated(0),
      m_bToBayer(tobayer)
  { }
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
