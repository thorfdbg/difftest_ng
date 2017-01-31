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
** $Id: add.cpp,v 1.5 2017/01/31 11:58:03 thor Exp $
**
** This class adds the second image to the first and merges the two
** under a common name.
*/

/// Includes
#include "diff/add.hpp"
///

/// AddImg::Measure
double AddImg::Measure(class ImageLayout *src,class ImageLayout *sr2,double in)
{
  ULONG width  = (src->WidthOf()  > sr2->WidthOf()) ?(src->WidthOf()) :(sr2->WidthOf());
  ULONG height = (src->HeightOf() > sr2->HeightOf())?(src->HeightOf()):(sr2->HeightOf());
  UWORD d1     = src->DepthOf();
  UWORD d2     = sr2->DepthOf();
  UWORD depth  = src->DepthOf() + sr2->DepthOf();
  UWORD i;

  CreateComponents(width,height,depth);
  
  for(i = 0;i < d1;i++) {
    m_pComponent[i].m_ucBits  = src->BitsOf(i);
    m_pComponent[i].m_bSigned = src->isSigned(i);
    m_pComponent[i].m_bFloat  = src->isFloat(i);
    m_pComponent[i].m_ucSubX  = m_ulWidth  / src->WidthOf(i);
    m_pComponent[i].m_ucSubY  = m_ulHeight / src->HeightOf(i);
    m_pComponent[i].m_ulWidth = src->WidthOf(i);
    m_pComponent[i].m_ulHeight= src->HeightOf(i);
    m_pComponent[i].m_ulBytesPerPixel = src->BytesPerPixel(i);
    m_pComponent[i].m_ulBytesPerRow   = src->BytesPerRow(i);
    m_pComponent[i].m_pPtr            = const_cast<APTR>(src->DataOf(i));
  }
  for(i = 0;i < d2;i++) {
    m_pComponent[i+d1].m_ucBits  = sr2->BitsOf(i);
    m_pComponent[i+d1].m_bSigned = sr2->isSigned(i);
    m_pComponent[i+d1].m_bFloat  = sr2->isFloat(i);
    m_pComponent[i+d1].m_ucSubX  = m_ulWidth  / sr2->WidthOf(i);
    m_pComponent[i+d1].m_ucSubY  = m_ulHeight / sr2->HeightOf(i);
    m_pComponent[i+d1].m_ulWidth = sr2->WidthOf(i);
    m_pComponent[i+d1].m_ulHeight= sr2->HeightOf(i);
    m_pComponent[i+d1].m_ulBytesPerPixel = sr2->BytesPerPixel(i);
    m_pComponent[i+d1].m_ulBytesPerRow   = sr2->BytesPerRow(i);
    m_pComponent[i+d1].m_pPtr            = const_cast<APTR>(sr2->DataOf(i));
  }

  SaveImage(m_pcTargetFile,m_TargetSpecs);
  
  return in;
}
///
