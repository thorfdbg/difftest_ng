/*************************************************************************
** Copyright (c) 2003-2016 Accusoft 				        **
**									**
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
** This class defines the base structure for all meters, i.e. all
** possible measurements that can be made.
**
** $Id: meter.hpp,v 1.6 2016/06/04 10:44:09 thor Exp $
**
*/

#ifndef DIFF_METER_HPP
#define DIFF_METER_HPP

/// Includes
#include "interface/types.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Meter
// This class defines the base structure for all meters, i.e. all
// possible measurements that can be made.
class Meter {
  //
  // Pointer to the next meter on the agenda.
  class Meter *m_pNext;
  //
public:
  Meter(void)
    : m_pNext(NULL)
  {
  }
  //
  virtual ~Meter(void)
  {
  }
  //
  //
  class Meter *NextOf(void) const
  {
    return m_pNext;
  }
  //
  class Meter *&NextOf(void)
  {
    return m_pNext;
  }
  //
  // Perform the measurement, return the result.
  virtual double Measure(class ImageLayout *org,class ImageLayout *dist,double in) = 0;
  //
  // Return the name of this class.
  virtual const char *NameOf(void) const = 0;
  //
};
///

///
#endif
