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
**
** $Id: restrict.hpp,v 1.6 2016/06/04 10:44:09 thor Exp $
**
** This class acts as a filter and restricts the following meters to
** a single component.
*/

#ifndef DIFF_RESTRICT_HPP
#define DIFF_RESTRICT_HPP

/// Includes
#include "diff/meter.hpp"
///

/// class Restrict
// This class acts as a filter and restricts the following meters to
// a single component.
class Restrict : public Meter {
  //
  // The component to restrict to.
  UWORD m_usComp;
  // The number of components to retain.
  UWORD m_usCount;
  //
public:
  Restrict(UWORD comp,UWORD count)
    : m_usComp(comp), m_usCount(count)
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
