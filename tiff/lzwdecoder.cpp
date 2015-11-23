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
**
** This file contains the logic to decode LZW encoded TIFF files as (the only)
** compression mechanism by this implementation.
**
** $Id: lzwdecoder.cpp,v 1.5 2014/01/04 11:35:29 thor Exp $
**
*/

/// Includes
#include "tiff/lzwdecoder.hpp"
#include "tiff/tiffparser.hpp"
#include "std/assert.hpp"
///

/// LZWDecoder::LZWDecoder
// Start an LZW decoder on the given data buffer with the given number of bytes in the buffer.
LZWDecoder::LZWDecoder(UBYTE *buffer,ULONG size,bool bigendian)
  : DecoderBase(buffer,size), DecoderFunctions<LZWDecoder>(this,bigendian),
    m_ucBitPos(8), m_ucBitsPerCode(9), m_usTableSize(0), 
    m_pucNextOutput(&m_ucOutputBuffer[0])
{
  if (size >= 2 && buffer[0] == 0 && (buffer[1] & 0x01)) {
    m_bLefty = true; // old style LZW code
  } else {
    m_bLefty = false;
  }
}
///

/// LZWDecoder::~LZWDecoder
LZWDecoder::~LZWDecoder(void)
{
}
///

/// LZWDecoder::InitializeTable
// Initialize the table, including ClearCode and EOICode
void LZWDecoder::InitializeTable()
{
  int i;

  for(i = 0;i < 256;i++) {
    m_ucCodeTable[i]   = i;          // this is the identity table
    m_usPrefixTable[i] = LastPrefix; // doesn't have a prefix anymore.
  }

  // Entries 256 and 257 are reserved.
  m_usTableSize   = EOICode + 1;
  m_ucBitsPerCode = 9;
}
///

/// LZWDecoder::StringFromCode
// Fill in the string encoded by the given code into the output table,
// possibly enlarging the table. Note that this string is filled in in reverse
// order, it is inverted by the read-out procedure. If reserve is given,
// the indicated number of bytes are additionally reserved at the end of
// the string (and thus at the beginning of the buffer)
void LZWDecoder::StringFromCode(UWORD code,int reserve)
{
  UBYTE *output = m_ucOutputBuffer + reserve;
  
  do {
    assert(code != EOICode && code != ClearCode && code < m_usTableSize);
    
    // If there is no room in the table, something strange happened.
    if (output >= &m_ucOutputBuffer[OutputBufferSize])
      throw "LZW output buffer overflow, probably a corrupt LZW?";

    *output++ = m_ucCodeTable[code];
    code      = m_usPrefixTable[code];
  } while(code != LastPrefix);
    
  m_pucNextOutput = output;
}
///

/// LZWDecoder::AddToTable
// Add an entry into the table which has as prefix the string that
// belongs to the prefix code, and that has the given postfix character.
void LZWDecoder::AddToTable(UWORD prefix,UBYTE postfix)
{
  assert(prefix != ClearCode && prefix != EOICode);
  
  if (m_usTableSize >= MaxTableSize)
    throw "LZW dictionary overflow, probably a corrupt LZW stream";

  m_ucCodeTable[m_usTableSize]   = postfix;
  m_usPrefixTable[m_usTableSize] = prefix;

  m_usTableSize++;
  if (m_bLefty) {
    if (m_usTableSize == 512)
      m_ucBitsPerCode = 10;
    if (m_usTableSize == 1024)
      m_ucBitsPerCode = 11;
    if (m_usTableSize == 2048)
      m_ucBitsPerCode = 12;
  } else {
    if (m_usTableSize == 511)
      m_ucBitsPerCode = 10;
    if (m_usTableSize == 1023)
      m_ucBitsPerCode = 11;
    if (m_usTableSize == 2047)
      m_ucBitsPerCode = 12;
  }
}
///

/// LZWDecoder::RefillBuffer
// Refill the output buffer of the LZW decoder
void LZWDecoder::RefillBuffer(void)
{   
  UWORD code;
  
  code = GetNextCode();
  if (code == EOICode)
    throw "reading past EOF of LZW input buffer";
  
  if (code == ClearCode) {
    InitializeTable();

    code = GetNextCode();

    if (code == EOICode)
      throw "reading past EOF of LZW input buffer";
    
    if (code == ClearCode)
      throw "detected double ClearCode in LZW input buffer, LZW stream corrupt";
    
    StringFromCode(code);
  } else {
    //
    // Here: Not EOI, not ClearCode.
    if (m_usTableSize < EOICode + 1)
      throw "initial ClearCode missing in LZW stream";
    //
    // Is the just decoded code in the table?
    if (code < m_usTableSize) {
      StringFromCode(code); // Write out the code
      // Add a new string into the table which has the prefix
      // given from the previous code, and has a new postfix which is
      // given by the first character of the string just decoded.
      // ... below ...
    } else if (code == m_usTableSize) {
      // Not in table. The new string is just the string from the previous
      // code, extended by a single character which is given by the
      // first character of the output string.
      StringFromCode(m_usOldCode,1);
      // Extend the string by one character, namely the first in
      // the buffer.
      m_ucOutputBuffer[0] = m_pucNextOutput[-1];
      //
      // Add this code to the table. The new string is the old
      // string extended by the first character.
      // ... below ...
    } else throw "detected invalid LZW code in TIFF input, LZW stream corrupt";
    AddToTable(m_usOldCode,m_pucNextOutput[-1]);
  }
  m_usOldCode = code;
}
///
