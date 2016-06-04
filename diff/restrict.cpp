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
** $Id: restrict.cpp,v 1.6 2016/06/04 10:44:09 thor Exp $
**
** This class acts as a filter and restricts the following meters to
** a single component.
*/

/// Includes
#include "diff/restrict.hpp"
#include "img/imglayout.hpp"
///

/// Restrict::Measure
// Does not measure, actually, rather performs the restriction
double Restrict::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{

  if (m_usComp >= src->DepthOf())
    throw "the specified component for --only does not exist";
  
  src->Restrict(m_usComp,m_usCount);
  dst->Restrict(m_usComp,m_usCount);

  return in;
}
///

