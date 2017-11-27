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
** Fill
** 
** $Id: fill.hpp,v 1.1 2017/11/27 11:37:13 thor Exp $
**
** This class fills an entire image with a given color
**
*/

#ifndef DIFF_FILL_HPP
#define DIFF_FILL_HPP

/// Include
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Fill
// This class fills an entire image with a given color
class Fill : public Meter {
  //
  // Templated filler class.
  template<typename T>
  void  FillCanvas(T *org,ULONG bytesperpixel,ULONG bytesperrow,
		   ULONG width,ULONG height,T value);
  //
  // The configuration string that contains all the fill values. This is directly
  // coming from the command line.
  const char *m_pcValues;
  //
public:
  Fill(const char *fill)
    : m_pcValues(fill)
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

