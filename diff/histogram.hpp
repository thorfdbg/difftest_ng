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
** $Id: histogram.hpp,v 1.6 2020/09/15 09:45:49 thor Exp $
**
** This class saves the histogram to a file or writes it to
** stdout.
*/

#ifndef DIFF_HISTOGRAM_HPP
#define DIFF_HISTOGRAM_HPP

/// Includes
#include "diff/meter.hpp"
///

/// class Histogram
// This class saves the histogram to a file or writes it to
// stdout.
class Histogram : public Meter {
  //
  // The file name under which the difference image shall be saved.
  const char *m_pcTargetFile;
  //
  // The threshold for measuring pixel ratios.
  LONG        m_lThres;
  //
  // The histogram array.
  ULONG      *m_pulHist;
  //
  // Measure the difference histogram, place results into the given array
  // after adding the given offset to the difference.
  template<typename T>
  static void Measure(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
		      T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		      ULONG w      ,ULONG h,ULONG *hist  ,LONG offset);
  //
public:
  //
  // Construct the histogram. Takes a file name.
  Histogram(const char *filename)
    : m_pcTargetFile(filename), m_lThres(-1), m_pulHist(NULL)
  {
  }
  //
  // Construct the histogram for measuring pixel difference ratios.
  Histogram(LONG thres)
    : m_pcTargetFile(NULL), m_lThres(thres), m_pulHist(NULL)
  {
  }
  //
  virtual ~Histogram(void);
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    if (!m_pcTargetFile)
      return "PxAboveThres";
    
    return NULL;
  }
};
///

///
#endif

