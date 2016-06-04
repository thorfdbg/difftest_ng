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
** $Id: convertimg.cpp,v 1.5 2016/06/04 10:44:08 thor Exp $
**
** This class saves the original image unmodified for conversion and testing
** purposes.
*/

/// Includes
#include "diff/convertimg.hpp"
///

/// ConvertImg::Measure
double ConvertImg::Measure(class ImageLayout *src,class ImageLayout *,double in)
{
  src->SaveImage(m_pcTargetFile,m_TargetSpecs);
  
  return in;
}
///
