/*************************************************************************
** Copyright (c) 2011-2014 Accusoft Corporation                         **
**                                                                      **
** Written by Thomas Richter (richter@rus.uni-stuttgart.de)             **
** Sponsored by Accusoft Corporation, Tampa, FL and                     **
** the Computing Center of the University of Stuttgart                  **
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
** $Id: restore.cpp,v 1.2 2014/01/04 11:35:28 thor Exp $
**
** This class acts as a filter and restricts the following meters to
** a single component.
*/

/// Includes
#include "diff/restore.hpp"
#include "img/imglayout.hpp"
///

/// Restore::Measure
// Does not measure, actually, rather performs the restoration
double Restore::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{

  *src = *m_pOrg;
  *dst = *m_pDst;

  return in;
}
///
