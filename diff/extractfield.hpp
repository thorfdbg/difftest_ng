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
** $Id: extractfield.hpp,v 1.1 2023/07/11 10:34:06 thor Exp $
**
** This class acts as a filter and extracts the even or odd lines
** of an image.
*/

#ifndef DIFF_EXTRACTFIELD_HPP
#define DIFF_EXTRACTFIELD_HPP

/// Includes
#include "diff/meter.hpp"
///

/// class ExtractField
// This class acts as a filter and extracts the even or odd lines
// of an image.
class ExtractField : public Meter {
  //
  // The field (true = odd, false = even) to
  // extract from progressive.
  bool m_bOddField;
  //
public:
  ExtractField(bool oddfield)
    : m_bOddField(oddfield)
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
