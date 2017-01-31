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
** $Id: packbitsdecoder.hpp,v 1.6 2017/01/31 11:58:05 thor Exp $
**
*/

#ifndef TIFF_PACKBITSDECODER_HPP
#define TIFF_PACKBITSDECODER_HPP

/// Includes
#include "interface/types.hpp"
#include "tiff/tiffparser.hpp"
#include "tiff/decoderbase.hpp"
#include "tiff/decoderfunctions.hpp"
///

/// class PackBitsDecoder
class PackBitsDecoder : public DecoderBase, public DecoderFunctions<PackBitsDecoder> {
  //
  // The current (remaining) runlength or
  // zero if no runlength compression.
  UBYTE  m_ucRunLength;
  //
  // The current (remaining) number of bytes to
  // copy.
  UBYTE  m_ucCopyLength;
  //
  // The current output byte for runlength coding.
  UBYTE m_ucOutBuffer;
  //
public:
  // Start an LZW decoder on the given data buffer with the given number of bytes in the buffer.
  PackBitsDecoder(UBYTE *buffer,ULONG size,bool bigendian);
  //
  ~PackBitsDecoder(void)
  {
  }
  //
  // Return/Decode the next byte from the data
  UBYTE GetUBYTE(void)
  {
    while (m_ucRunLength == 0 && m_ucCopyLength == 0) {
      BYTE ins;
      // Refill
      ins = Read();
      if (ins >= 0) {
	// direct case
	m_ucCopyLength = ins + 1;
      } else if (ins == -128) {
	continue; // no-op
      } else {
	// runlength case
	m_ucOutBuffer = Read();
	m_ucRunLength = -ins+1;
      }
    }
	
    if (m_ucRunLength) {
      m_ucRunLength--;
      return m_ucOutBuffer;
    } else {
      m_ucCopyLength--;
      return Read();
    }
  }  
  // 
};
///

#endif

