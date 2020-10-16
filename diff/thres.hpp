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
** $Id: thres.hpp,v 1.9 2020/10/16 10:14:12 thor Exp $
**
** This class finds the minimum and the maximum difference.
*/

#ifndef DIFF_THRES_HPP
#define DIFF_THRES_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Thres
class Thres : public Meter {
  //
  // Threshold measurement type
  int m_Type;
  //
  // Templated implementations
  template<typename T>
  double Error(T *org,ULONG obytesperpixel,ULONG obytesperrow,
	       T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
	       ULONG w,ULONG h);
public:
  //
  enum Type {
    Min,
    Max,
    Avg,
    Drift,
    Peak,
    Toe,
    Head
  };
  //
  Thres(Type t)
    : m_Type(t)
  { }
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    switch(m_Type) {
    case Min:
      return "Minimum";
    case Max:
      return "Maximum";
    case Avg:
      return "Average Error";
    case Drift:
      return "Drift";
    case Peak:
      return "Peak Error";
    case Toe:
      return "Toe value";
    case Head:
      return "Head value";
    }
    return NULL;
  }
};
///

///
#endif
