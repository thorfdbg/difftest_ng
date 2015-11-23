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
** $Id: dimension.hpp,v 1.3 2014/01/04 11:35:28 thor Exp $
**
*/

#ifndef DIFF_DIMENSION_HPP
#define DIFF_DIMENSION_HPP

/// Includes
#include "interface/types.hpp"
#include "diff/meter.hpp"
///

/// class Dimension
// This class performs elementary measurements of the dimensions of
// the image(s). Only the first is examined, but both images need the
// same layout anyhow.
class Dimension : public Meter {
  //
  // The type of the measurement to be performed.
  int   m_iType;
  //
  // The component to measure on (if any).
  UWORD m_usComp;
  //
public:
  //
  // Measurement types.
  enum Type {
    Width,
    Height,
    Depth,
    Precision,
    Signed,
    Float
  };
  //
  Dimension(Type t,UWORD component = 0)
    : m_iType(t), m_usComp(component)
  {
  }
  //
  virtual ~Dimension(void)
  {
  }
  //
  //
  // Perform the measurement, return the result.
  virtual double Measure(class ImageLayout *org,class ImageLayout *dist,double in);
  //
  // Return the name of this class.
  virtual const char *NameOf(void) const
  {
    switch(m_iType) {
    case Width:
      return "Width";
      break;
    case Height:
      return "Height";
      break;
    case Depth:
      return "Depth";
      break;
    case Precision:
      return "Precision";
      break;
    case Signed:
      return "Signedness";
      break;
    case Float:
      return "Floating Point";
      break;
    }
    return NULL;
  }
};
///

///
#endif
