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
** $Id: restore.hpp,v 1.4 2016/06/04 10:44:09 thor Exp $
**
** This class restores the image to the original region and un-does
** the crop or restrict.
*/

#ifndef DIFF_RESTORE_HPP
#define DIFF_RESTORE_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// class Restore
// This class restores the activity to the full image region.
class Restore : public Meter {
  //
  // Pointers to where the original image is.
  class ImageLayout *&m_pOrg;
  class ImageLayout *&m_pDst;
  //
public:
  Restore(class ImageLayout *&orgcpy,class ImageLayout *&dstcpy)
    : m_pOrg(orgcpy), m_pDst(dstcpy)
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
