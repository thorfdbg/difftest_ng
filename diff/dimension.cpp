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
** This class performs elementary measurements of the dimensions of
** the image(s). Only the first is examined, but both images need the
** same layout anyhow.
**
** $Id: dimension.cpp,v 1.3 2014/01/04 11:35:28 thor Exp $
**
*/

/// Includes
#include "img/imglayout.hpp"
#include "diff/dimension.hpp"
///


/// Dimension::Measure
// Perform the measurement, return the result.
double Dimension::Measure(class ImageLayout *org,class ImageLayout *,double)
{
  switch(m_iType) {
  case Width:
    return org->WidthOf();
    break;
  case Height:
    return org->HeightOf();
    break;
  case Depth:
    return org->DepthOf();
    break;
  case Precision:
    return org->BitsOf(m_usComp);
    break;
  case Signed:
    if (org->isSigned(m_usComp)) {
      return -1.0;
    } else {
      return 1.0;
    } 
  case Float:
    if (org->isFloat(m_usComp)) {
      return 1.0;
    } else {
      return 0.0;
    }
    break;
  }
  
  return 0.0;
}
///

 
