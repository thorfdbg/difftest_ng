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
** $Id: compare.cpp,v 1.7 2017/01/31 11:58:03 thor Exp $
**
** Compare the input value against a threshold, fail if the comparison is false.
*/

/// Includes
#include "std/stdio.hpp"
#include "diff/meter.hpp"
#include "diff/compare.hpp"
///

/// Compare::Measure
// Perform the actual comparison
double Compare::Measure(class ImageLayout *,class ImageLayout *,double in)
{
  switch(m_Type) {
  case Greater:
    if (in > m_dVal)
      return in;
    break;
  case GreaterEqual:
    if (in >= m_dVal)
      return in;
    break;
  case Equal:
    if (in == m_dVal)
      return in;
    break;
  case NotEqual:
    if (in != m_dVal)
      return in;
    break;
  case SmallerEqual:
    if (in <= m_dVal)
      return in;
    break;
  case Smaller:
    if (in < m_dVal)
      return in;
    break;
  }

  {
    const char *type = "";

    switch(m_Type) {
    case Greater:
      type = ">";
      break;
    case GreaterEqual:
      type = ">=";
      break;
    case Equal:
      type = "==";
      break;
    case NotEqual:
      type = "!=";
      break;
    case SmallerEqual:
      type = "<=";
      break;
    case Smaller:
      type = "<";
      break;
    }
    
    sprintf(m_cBuffer,"comparison %g %s %g failed",in,type,m_dVal);
    
    throw m_cBuffer;
  }
}
///
