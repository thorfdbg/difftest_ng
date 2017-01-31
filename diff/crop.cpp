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
** $Id: crop.cpp,v 1.5 2017/01/31 11:58:03 thor Exp $
**
** This class crops images, i.e. extracts rectangular regions from images
*/

/// Includes
#include "img/imglayout.hpp"
#include "diff/crop.hpp"
///

/// class Crop
// Crop an image region
double Crop::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  src->Crop(m_ulX1,m_ulY1,m_ulX2,m_ulY2);
  dst->Crop(m_ulX1,m_ulY1,m_ulX2,m_ulY2);
  
  return in;
}
///

