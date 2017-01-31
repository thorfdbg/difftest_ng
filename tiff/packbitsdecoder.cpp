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
** This file contains the logic to decode packbits encoded TIFF files
**
** $Id: packbitsdecoder.cpp,v 1.6 2017/01/31 11:58:05 thor Exp $
**
*/

/// Includes
#include "interface/types.hpp"
#include "tiff/tiffparser.hpp"
#include "tiff/packbitsdecoder.hpp"
///

/// PackBitsDecoder::PackBitsDecoder
// Start a packbits decoder on the given data buffer with the given number of bytes in the buffer.
PackBitsDecoder::PackBitsDecoder(UBYTE *buffer,ULONG size,bool bigendian) 
  : DecoderBase(buffer,size), DecoderFunctions<PackBitsDecoder>(this,bigendian),
    m_ucRunLength(0), m_ucCopyLength(0)
{
}
///

