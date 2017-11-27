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
** Paste
** 
** $Id: paste.hpp,v 1.1 2017/11/27 13:21:16 thor Exp $
**
** This class copies a second image into the first image.
**
*/

#ifndef DIFF_PASTE_HPP
#define DIFF_PASTE_HPP

/// Include
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Paste
// This class copies a second image into the first image.
class Paste : public Meter {
  //
  // Templated filler class.
  template<typename T>
  void  PasteImage(T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		   const T *src,ULONG sbytesperpixel,ULONG sbytesperrow,
		   ULONG tx,ULONG ty,ULONG width,ULONG height);
  //
  // The coordinates into which to paste the target image.
  // They still have to be modified by the subsampling parameters.
  ULONG m_ulDestX,m_ulDestY;
  //
public:
  Paste(ULONG x,ULONG y)
    : m_ulDestX(x), m_ulDestY(y)
  { }
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

