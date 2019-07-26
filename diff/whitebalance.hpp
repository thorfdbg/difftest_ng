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
** $Id: whitebalance.hpp,v 1.2 2019/07/26 05:46:35 thor Exp $
**
** This class scales the components of images with component dependent
** scale factors.
*/

#ifndef DIFF_WHITEBALANCE_HPP
#define DIFF_WHITEBALANCE_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class WhiteBalance
// This class scales the components of images with component dependent
// scale factors.
class WhiteBalance : public Meter {
  //
public:
  //
  enum Operation {
    Scale,  // scale components
    Shift   // shift components
  };
  //
private:
  //
  // The type of operation desired. Scale or shift.
  Operation    m_Type;
  //
  // The number of components for which we recorded scaling data.
  UWORD        m_usComponents;
  //
  // The array of scaling factors or shifting offsets.
  DOUBLE      *m_pdFactors;
  //
  // This never changes the data type.
  template<typename T>
  void Convert(T *dst ,ULONG bytesperpixel,ULONG bytesperrow,
	       ULONG w, ULONG h,double scale ,double min,double max);
  //
  template<typename T>
  void ShiftConv(T *dst ,ULONG bytesperpixel,ULONG bytesperrow,
		 ULONG w, ULONG h,double offset ,double min,double max);
  //
  //
  // Apply a scaling from the source to the image stored here.
  void ApplyScaling(class ImageLayout *src);
  //
public:
  //
  // Apply a component dependent white balance to the image.
  WhiteBalance(Operation type,const char *factors);
  //
  virtual ~WhiteBalance(void);
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
