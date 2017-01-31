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
** $Id: colorhist.hpp,v 1.5 2017/01/31 11:58:03 thor Exp $
**
** Shows a reduced histogram of the image on stdout.
*/

#ifndef DIFF_COLORHIST_HPP
#define DIFF_COLORHIST_HPP

/// Includes
#include "diff/meter.hpp"
///

/// class ColorHistogram
// This class shows a reduced histogram for all components on stdout.
class ColorHistogram : public Meter {
  //
  // The histogram array.
  ULONG      *m_pulHist;
  //
  // The bucket size of the histogram. Usually 1.0.
  double      m_dBucketSize;
  //
  // Measure the difference histogram, place results into the given array
  // after adding the given offset to the difference.
  template<typename T>
  static void Measure(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
		      T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		      ULONG w      ,ULONG h,ULONG *hist,double mult);
  //
public:
  //
  // Construct the histogram. Takes a file name.
  ColorHistogram(double bucketsize)
    : m_pulHist(NULL), m_dBucketSize(bucketsize)
  {
  }
  //
  virtual ~ColorHistogram(void);
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

