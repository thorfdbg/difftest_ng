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
** This is the trivial decoder which doesn't do anything to the data
** but rather reads them as it comes, directly.
**
** $Id: trivialdecoder.hpp,v 1.4 2016/06/04 10:44:10 thor Exp $
**
*/

#ifndef TIFF_TRIVIALDECODER_HPP
#define TIFF_TRIVIALDECODER_HPP

/// Includes
#include "interface/types.hpp"
#include "tiff/tiffparser.hpp"
#include "tiff/decoderbase.hpp"
#include "tiff/decoderfunctions.hpp"
///

/// class TrivialDecoder
class TrivialDecoder : public DecoderBase, public DecoderFunctions<TrivialDecoder> {
  //
  //
public:
  // Start an LZW decoder on the given data buffer with the given number of bytes in the buffer.
  TrivialDecoder(UBYTE *buffer,ULONG size,bool bigendian);
  //
  ~TrivialDecoder(void)
  {
  }
  //
  // Return/Decode the next byte from the data
  UBYTE GetUBYTE(void)
  {
    return Read();
  }  
  // 
};
///

#endif

