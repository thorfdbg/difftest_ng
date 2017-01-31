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
** This file contains the logic to decode LZW encoded TIFF files as (the only)
** compression mechanism by this implementation.
**
** $Id: lzwdecoder.hpp,v 1.8 2017/01/31 11:58:05 thor Exp $
**
*/

#ifndef TIFF_LZWDECODER_HPP
#define TIFF_LZWDECODER_HPP

/// Includes
#include "interface/types.hpp"
#include "tiff/tiffparser.hpp"
#include "tiff/decoderbase.hpp"
#include "tiff/decoderfunctions.hpp"
///

/// class LZWDecoder
class LZWDecoder : public DecoderBase, public DecoderFunctions<LZWDecoder> {
  //
  // The current bit position in the code
  UBYTE  m_ucBitPos;
  //
  // Number of bits per code, depends on the table size.
  UBYTE  m_ucBitsPerCode;
  //
  // Number of entries in the table.
  UWORD  m_usTableSize;
  //
  // The previous code just decoded before.
  UWORD  m_usOldCode;
  //
  // Pointer in the output string buffer, namely where the next string
  // from the output table is removed. Note that this is read end to start,
  // in reverse order.
  UBYTE *m_pucNextOutput;
  //
  // Set for old-style compatibility LZW-codes that use a different bit-filling
  // order.
  bool   m_bLefty;
  //
  enum {
    MaxTableSize     = (1 << 12) - 1 + 1024, // maximal number of entries allowed here.
    OutputBufferSize = (1 << 16)
  };
  //
  // The table is encoded in the following way: The first array contains the postfix,
  // the second table the offset of the prefix in the postfix if there is one, or MAX_UWORD
  // in case there is no prefix and the entry is the start of the string.
  UBYTE m_ucCodeTable[MaxTableSize];
  //
  UWORD m_usPrefixTable[MaxTableSize];
  //
  // The output buffer.
  UBYTE m_ucOutputBuffer[OutputBufferSize];
  //
  // Initialize the table, including ClearCode and EOICode
  void InitializeTable();
  //
  // Return the next code from the buffer.
  UWORD GetNextCode(void)
  {
    if (m_bLefty) {
      return ReadBitsReversed(m_ucBitPos,m_ucBitsPerCode,false);
    } else {
      return ReadBits(m_ucBitPos,m_ucBitsPerCode,false);
    }
  }
  //
  // Special codes
  enum {
    ClearCode  = 256,		// re-initialize the string table
    EOICode    = 257,           // end of data
    LastPrefix = 0xffff         // last entry, no prefix anymore.
  };
  //
  // Add an entry into the table which has as prefix the string that
  // belongs to the prefix code, and that has the given postfix character.
  void AddToTable(UWORD prefix,UBYTE postfix);
  //
  // Fill in the string encoded by the given code into the output table,
  // possibly enlarging the table. Note that this string is filled in in reverse
  // order, it is inverted by the read-out procedure. If reserve is given,
  // the indicated number of bytes are additionally reserved at the end of
  // the string (and thus at the beginning of the buffer)
  void StringFromCode(UWORD code,int reserve = 0);
  //
  // Refill the output buffer of the LZW decoder by decoding the next
  // codeword in the input buffer
  void RefillBuffer(void);
public:
  // Start an LZW decoder on the given data buffer with the given number of bytes in the buffer.
  LZWDecoder(UBYTE *buffer,ULONG size,bool bigendian);
  //
  ~LZWDecoder(void);
  //
  // Return/Decode the next byte from the data
  UBYTE GetUBYTE(void)
  {
    // Need to refill the output buffer?
    if (m_pucNextOutput <= m_ucOutputBuffer) {
      RefillBuffer();
    }
    
    // Deliver the next output character.
    // Note that this is reversed.
    return *(--m_pucNextOutput);
  }  
  //
};
///

#endif

