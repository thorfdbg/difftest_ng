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
** $Id: mergefields.hpp,v 1.1 2023/07/11 11:54:11 thor Exp $
**
** This class merges source and destination together in one single 
** frame, de-interlacing them to a progressive image.
*/

#ifndef DIFF_MERGEFIELDS_HPP
#define DIFF_MERGEFIELDS_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// class MergeFields
// This class merges source and destination together in one single 
// frame, de-interlacing them to a progressive image.
class MergeFields : public Meter, private ImageLayout {
  //
  // Memory management for the components
  // created by this object.
  //
  // The number of components in here.
  UWORD             m_usDepth;
  //
  // The memory array that holds the components.
  UBYTE           **m_ppucMemory;
  //
  template <typename T>
  static void InterleaveData(T *dst,ULONG bytesperpixel,ULONG bytesperrow,
			     ULONG width,ULONG height,
			     const void *even,ULONG srcbpp,ULONG srcbpr,
			     const void *odd, ULONG dstbpp,ULONG dstbpr);
  //
public:
  MergeFields(void)
    : m_usDepth(0), m_ppucMemory(NULL)
  { }
  //
  ~MergeFields(void);
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
