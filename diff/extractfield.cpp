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
** $Id: extractfield.cpp,v 1.1 2023/07/11 10:34:06 thor Exp $
**
** This class acts as a filter and extracts the even or odd lines
** of an image.
*/

/// Includes
#include "diff/extractfield.hpp"
#include "img/imglayout.hpp"
///

/// ExtractField::Measure
// Does not measure, actually, rather performs the restriction
double ExtractField::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{

  src->ExtractField(m_bOddField);
  dst->ExtractField(m_bOddField);

  return in;
}
///

