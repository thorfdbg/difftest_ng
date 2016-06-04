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
** This file contains basic support for half-float numbers.
**
** $Id: halffloat.hpp,v 1.4 2016/06/04 10:44:10 thor Exp $
**
*/

#ifndef TOOLS_HALFFLOAT_HPP
#define TOOLS_HALFFLOAT_HPP

/// Includes
#include "interface/types.hpp"
#include "std/math.hpp"
///

/// Type definitions
// The type itself is called HALF.
typedef WORD HALF;
///

/// H2F: Convert a half-float to a floating point number
inline FLOAT H2F(HALF in)
{
  bool negative = (in < 0); // The sign bit is at the same position.
  int exponent  = ((in & 0x7c00) >> 10) - 15; // 15 is the exponent bias
  int mantissa  = (in & 0x03ff);

  // Denormalized numbers
  if (exponent == -15) {
    exponent++; // but do not include the implicit one-bit
  } else if (exponent == 16) {
    // Return INF or -INF. Problem is that we cannot easily generate a reasonable NAN here.
    return (negative)?(-HUGE_VAL):(HUGE_VAL);
  } else {
    // Include the implicit one-bit.
    mantissa |= 0x0400;
  }

  if (negative)
    mantissa = -mantissa;
  
  return ldexp(mantissa / 1024.0,exponent);
}
///

/// F2H: Convert a floating point number to half-float
inline HALF F2H(FLOAT in)
{
  bool negative = (in < 0);
  int  exponent;
  int  mantissa = 2048.0 * frexpf((negative)?(-in):(in),&exponent);

  if (mantissa == 0) {
    // Zero and -Zero
    return (negative)?(0x8000):(0x0000);
  } 

  /*
  ** this would be the fixup code.
  mantissa <<= 1; // shift into explicit one-bit
  exponent--;     // normalize the exponent correspondingly.
  */
  exponent += 14; // Exponent bias is 15. If this is too large to represent, generate INFs
  if (exponent >= 31) {
    return (negative)?(0xfc00):(0x7c00);
  } else if (exponent < 1) {
    // Denormalize the number.
    while(exponent < 1) {
      exponent++;
      mantissa >>= 1;
    }
    // This is represented with the exponent zero.
    exponent = 0;
  }
  //
  // Now combine the results into one value. Remove the implicit one,
  // and shift the exponent in place 
  mantissa = (mantissa & 0x03ff) | (exponent << 10);
  if (negative)
    mantissa |= 0x8000;

  return mantissa;
}
///

///
#endif
