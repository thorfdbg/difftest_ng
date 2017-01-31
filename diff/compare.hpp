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
** $Id: compare.hpp,v 1.6 2017/01/31 11:58:03 thor Exp $
**
** Compare the input value against a threshold, fail if the comparison is false.
*/

#ifndef DIFF_COMPARE_HPP
#define DIFF_COMPARE_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Compare
class Compare : public Meter {
  //
  // comparison type
  int    m_Type;
  //
  // The value/threshold to compare against.
  double m_dVal;
  //
  // A buffer to contain the error line.
  char   m_cBuffer[80];
  //
  //
public:
  //
  // Several options for comparison
  enum Type {
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    SmallerEqual,
    Smaller
  };
  //
  Compare(Type t,double value)
    : m_Type(t), m_dVal(value)
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
