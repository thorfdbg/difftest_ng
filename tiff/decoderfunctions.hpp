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
** This file contains extensions for all common decoders that provide
** bit-IO and multi-byte IO as convenience.
**
** $Id: decoderfunctions.hpp,v 1.5 2017/01/31 11:58:05 thor Exp $
**
*/

#ifndef TIFF_DECODERFUNCTIONS_HPP
#define TIFF_DECODERFUNCTIONS_HPP

/// Includes
#include "interface/types.hpp"
///

/// class DecoderFunctions
template<class Base>
class DecoderFunctions {
  //
  // The base class we use for the operations.
  Base *m_pBase;
  //
  // The byte output buffer for bit-IO
  UBYTE m_ucByteBuffer;
  //
  // The current bit counter for bit-IO
  UBYTE m_ucOutBitPos;
  //
  // The endian indicator
  bool  m_bBigEndian;
  //
public:
  // Start an LZW decoder on the given data buffer with the given number of bytes in the buffer.
  DecoderFunctions(Base *base,bool bigendian)
    : m_pBase(base), m_ucOutBitPos(0), m_bBigEndian(bigendian)
  {
  }
  //
  ~DecoderFunctions(void)
  {
  }
  //
  // 
  UWORD GetUWORD(void)
  {
    UBYTE b1 = m_pBase->GetUBYTE();
    UBYTE b2 = m_pBase->GetUBYTE();
    UWORD v;
      
    if (m_bBigEndian) {
      v = (b1 << 8) | (b2);
    } else {
      v = (b2 << 8) | (b1);
    }
    return v;
  }  
  //
  // Get a ULONG from a buffer, endian-corrected.
  ULONG GetULONG(void)
  {
    UWORD b1 = GetUWORD();
    UWORD b2 = GetUWORD();
    ULONG v;
    
    if (m_bBigEndian) {
      v = (b1 << 16) | (b2);
    } else {
      v = (b2 << 16) | (b1);
    }
    return v;
  } 
  //
  // Get a UQUAD from a buffer, endian-corrected.
  UQUAD GetUQUAD(void)
  {
    ULONG b1 = GetULONG();
    ULONG b2 = GetULONG();
    UQUAD v;
    
    if (m_bBigEndian) {
      v = (UQUAD(b1) << 32) | b2;
    } else {
      v = (UQUAD(b2) << 32) | b1;
    }
    
    return v;
  }
  //
  // Extract bits from a byte buffer, possibly update it.
  // bitpos is the number of bits left in the current buffer byte.
  ULONG GetBits(UBYTE bits,bool issigned)
  {
    ULONG res  = 0;
    UBYTE sign = bits;
    
    do { 
      UBYTE avail;
      if (m_ucOutBitPos == 0) {
	m_ucByteBuffer = m_pBase->GetUBYTE();
	m_ucOutBitPos  = 8;
      }
      avail = m_ucOutBitPos;
      if (avail > bits)
	avail = bits; // do not remove more bits than requested.
      
      // remove avail bits from the byte
      res     = (res << avail) | ((m_ucByteBuffer >> (m_ucOutBitPos - avail)) & ((1UL << avail) - 1));
      bits   -= avail;
      m_ucOutBitPos -= avail;
    } while(bits);
    
    if (issigned) {
      if (res & (1 << (sign - 1))) { // result is negative
	res |= ULONG(-1) << sign;
      }
    }
    
    return res;
  }
  //
  // Align to the next byte position (at the end of a row)
  void ByteAlign(void)
  {
    if (m_ucOutBitPos < 8) {
      m_ucOutBitPos  = 0;
    }
  }
};
///

#endif

