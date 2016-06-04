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
** This file contains the base class for all TIFF type buffer decoders.
** They provide bit/byte/word/long/quad byte access.
**
** $Id: decoderbase.hpp,v 1.4 2016/06/04 10:44:10 thor Exp $
**
*/

#ifndef TIFF_DECODERBASE_HPP
#define TIFF_DECODERBASE_HPP

/// Includes
#include "interface/types.hpp"
///

/// class DecoderBase
class DecoderBase {
  //
  // Pointer to the buffer and its end
  UBYTE *m_pucBuffer;
  UBYTE *m_pucBufferEnd;
  //
protected:
  // Return the next byte from the input buffer, throw on error.
  UBYTE Read(void)
  {
    if (m_pucBuffer >= m_pucBufferEnd)
      throw "run out of data in TIFF decompression, input stream is possibly corrupt";
    return *m_pucBuffer++;
  }
  //
  // Extract bits from a byte buffer, possibly update it.
  // bitpos is the number of bits left in the current buffer byte.
  ULONG ReadBits(UBYTE &bitpos,UBYTE bits,bool issigned)
  {
    ULONG res  = 0;
    UBYTE sign = bits;

    if (m_pucBuffer >= m_pucBufferEnd)
      throw "run out of data in TIFF decompression, input stream is possibly corrupt";
    
    do {
      UBYTE avail = bitpos;
      if (avail > bits)
	avail = bits; // do not remove more bits than requested.
      
      // remove avail bits from the byte
      res     = (res << avail) | ((m_pucBuffer[0] >> (bitpos - avail)) & ((1UL << avail) - 1));
      bits   -= avail;
      bitpos -= avail;
      if (bitpos == 0) { 
	m_pucBuffer++;
	bitpos = 8;
      }
    } while(bits);
    
    if (issigned) {
      if (res & (1 << (sign - 1))) { // result is negative
	res |= ULONG(-1) << sign;
      }
    }
    
    return res;
  }
  //
  // Extract bits from a byte buffer, possibly update it, but read them in
  // reverse order.
  ULONG ReadBitsReversed(UBYTE &bitpos,UBYTE bits,bool issigned)
  {
    ULONG res  = 0;
    ULONG mask = (1UL << bits) - 1;
    UBYTE tpos = 0;

    if (m_pucBuffer >= m_pucBufferEnd)
      throw "run out of data in TIFF decompression, input stream is possibly corrupt";
    
    do {
      UBYTE avail = bitpos;
      if (avail > bits)
	avail = bits; // do not remove more bits than requested.
      
      // remove avail bits from the byte, but from the lower end.
      res    |= (m_pucBuffer[0] >> (8 - bitpos)) << tpos;
      bits   -= avail;
      bitpos -= avail;
      tpos   += avail;
      if (bitpos == 0) { 
	m_pucBuffer++;
	bitpos = 8;
      }
    } while(bits);
    
    res &= mask; // mask out unused upper bits.
    if (issigned) {
      if (res & ((mask >> 1) + 1)) { // result is negative
	res |= ~mask; // set upper bits
      }
    }
    
    return res;
  }
  //
  //
public:
  // Start an LZW decoder on the given data buffer with the given number of bytes in the buffer.
  DecoderBase(UBYTE *buffer,ULONG size)
    : m_pucBuffer(buffer), m_pucBufferEnd(buffer + size)
  {
  }
  //
  ~DecoderBase(void)
  {
  }
};
///

#endif

