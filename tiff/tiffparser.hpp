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
** This file contains a simple tiff parser class. This phases the need for
** libtiff out as it does not support tiffs with varying bit depths.
**
** $Id: tiffparser.hpp,v 1.6 2014/10/18 19:18:10 thor Exp $
**
*/

#ifndef TIFF_TIFFPARSER_HPP
#define TIFF_TIFFPARSER_HPP

/// Includes
#include "interface/types.hpp"
#include "std/stdio.hpp"
///

/// class TiffParser
// This is a simple tiff parser for most basic TIFF support.
class TiffParser {
  //
  // The file we read from.
  FILE       *m_pFile;
  //
  // The file name, kept here for error messages
  const char *m_pcFilename;
  //
  // An indicator for the endianness. True for bigendian.
  bool        m_bBigEndian;
  //
  // Location of the first IFD. Note that we only support one of them.
  ULONG       m_ulIFDPos;
  //
  // The bits per pixel value.
  ULONG      *m_pulBitsPerPixel;
  //
  // The color map. Entries are actually restricted to UWORD.
  ULONG      *m_pulColorMap;
  //
  // The subsampling factors.
  ULONG      *m_pulSubsampling;
  //
  // The sample formats for all samples.
  ULONG      *m_pulSampleFormats;
  //
  // The number of compressed bytes in each strip.
  ULONG      *m_pulStripByteCount;
  //
  // The file offset for each stripe.
  ULONG      *m_pulStripOffset;
  //
  // The number of units/strips in the image.
  ULONG       m_ulUnits;
  //
  // The number of strips 
  // The current data buffer, and its size.
  UBYTE      *m_pucBuffer;
  ULONG       m_ulBufferSize;
  //
  //
  // Read a one-byte entry from the file.
  UBYTE GetByte(void);
  //
  // Read a two-byte entry, be endian-aware.
  UWORD GetWord(void);
  //
  // Read a four-byte entry, be endian-aware.
  ULONG GetLong(void);
  //
  // Read a floating point single precision IEEE value.
  FLOAT GetFloat(void);
  //
  // Read a floating point double precision IEEE value.
  DOUBLE GetDouble(void);
  //
  // Seek to the indicated offset, throw on error.
  void  Seek(ULONG pos);
  //
  // Position the file pointer on the tiff tag at the IFD starting
  // at the indicated position. Return true if this entry exists.
  // Return false otherwise. Note that this is not very fast as a 
  // lot of seeking is done, but nevermind.
  bool FindTag(UWORD tag);
  //
  // Read a scalar entry from the TIFF directory, return
  // true if found, otherwise return false.
  bool GetScalarTag(UWORD matchtag,ULONG &value);
  //
  // Read a scalar entry from the TIFF directory, return
  // true if found, otherwise return false. This handles
  // also floating point tags.
  bool GetScalarTag(UWORD matchtag,DOUBLE &value);
  //
  // Read a vectorial type from the TIFF directory,
  // return true if found, then the type is allocated and the size is
  // returned. Otherwise, false is returned.
  bool GetVectorTag(UWORD matchtag,ULONG &size,ULONG *&vector);
  //
public:
  TiffParser(const char *filename);
  //
  ~TiffParser(void);
  //
  // Return true in case the file is big endian.
  bool   isBigEndian(void) const
  {
    return m_bBigEndian;
  }
  //
  // Get a couple of elementary TIFF properties.
  ULONG  GetImageWidth(void);
  ULONG  GetImageHeight(void);
  ULONG  GetImageDepth(void);
  //
  // Return the photometric interpretation of the file.
  // If this is *NOT* present, assume RGB.
  ULONG  GetPhotometricInterpretation(void);
  //
  // Return an array of the number of bits per pixel. Its
  // size is identical to the image depths.
  const ULONG *GetBitsPerPixel(void);
  //
  // Return the compression type. Actually, not much can be
  // handled by simpletiff anyhow.
  ULONG  GetCompression(void);
  //
  // Get the prediction mode, only for LZW
  ULONG  GetPredictor(void);
  //
  // Return the colormap of the TIFF file if it is a lookup model.
  // Otherwise, return null.
  const ULONG *GetColorMap(void);
  //
  // Return the subsampling factors in X and Y direction for
  // YCbCr images. This is an array of two LONGs, or NULL
  // in case the image is not in YCbCr.
  const ULONG *GetSubsampling(void);
  //
  // Get the planar configuration of the file, whether samples
  // are organized in separate planes or interleaved.
  ULONG  GetPlanarConfig(void);
  //
  // Return the sample formats, one per channel. There are as
  // many channels as the image depth (not as in the colormap!)
  const ULONG *GetSampleFormat(void);
  //
  // Return an indicator whether this TIFF file contains tiles.
  bool   isTiled(void);
  //
  //
  // The following calls make only sense if isTiled() returns false
  // and the image is striped:
  //
  // Return the number of rows per strip for striped images.
  ULONG  GetRowsPerStrip(void);
  //
  // Return the number of strips per image.
  ULONG  GetStripsPerImage(void);
  //
  // If the configuration is "SEPARATE", then each channel goes into
  // a separate storage strip, and thus while the number of
  // geometrical strips remains the same, the number of addressable
  // units change. This is also the size of the array returned below.
  ULONG  GetAddressableStrips(void);
  //
  // Return the number of bytes for each strip.
  const ULONG *GetStripByteCount(void);
  //
  // Return the file offsets into the strips.
  const ULONG *GetStripOffset(void);
  //
  //
  // The following calls make only sense if isTiled() returns true
  // and the image is tiled:
  //
  // Get the width of all tiles.
  ULONG  GetTileWidth(void);
  //
  // Return the height of all tiles.
  ULONG  GetTileHeight(void);
  //
  // Return the number of tiles in the image.
  ULONG  GetTilesPerImage(void);
  //
  // If the configuration is "SEPARATE", then each channel goes into
  // a separate storage tile, and thus while the number of
  // geometrical tiles remains the same, the number of addressable
  // units change. This is also the size of the array returned below.
  ULONG  GetAddressableTiles(void);
  //
  // Return the array of tile byte counts, one entry per tile.
  const ULONG *GetTileByteCount(void);
  //
  // Return the array of tile file offsets, one entry per tile.
  const ULONG *GetTileOffset(void);
  //
  // Return the data for addressable unit "i" (where i is either
  // a tile, tile component, stripe or stripe component). The
  // data is *not* endian-corrected, but read in in raw.
  UBYTE *GetDataOfUnit(ULONG i,ULONG &size);
  //
  // Get the sample value to NITS conversion factor, or 1.0 if it is
  // not recorded.
  DOUBLE GetScaleFactor(void);
  //
  // Get a UWORD from a buffer, endian-corrected.
  UWORD GetUWORD(UBYTE *&buffer)
  {
    UWORD v;
    
    if (m_bBigEndian) {
      v = (buffer[0] << 8) | (buffer[1]);
      buffer += 2;
    } else {
      v = (buffer[1] << 8) | (buffer[0]);
      buffer += 2;
    }
    return v;
  }  
  //
  // Get a ULONG from a buffer, endian-corrected.
  ULONG GetULONG(UBYTE *&buffer)
  {
    ULONG v;
    
    if (m_bBigEndian) {
      v = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]);
      buffer += 4;
    } else {
      v = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | (buffer[0]);
      buffer += 4;
    }
    return v;
  } 
  //
  // Get a UQUAD from a buffer, endian-corrected.
  UQUAD GetUQUAD(UBYTE *&buffer)
  {
    UQUAD v;
    
    if (m_bBigEndian) {
      v = 
	(UQUAD(buffer[0]) << 56) |
	(UQUAD(buffer[1]) << 48) |
	(UQUAD(buffer[2]) << 40) |
	(UQUAD(buffer[3]) << 32) |
	(UQUAD(buffer[4]) << 24) |
	(UQUAD(buffer[5]) << 16) |
	(UQUAD(buffer[6]) <<  8) |
	(UQUAD(buffer[7]) <<  0);
      buffer += 8;
    } else {
      v = 
	(UQUAD(buffer[7]) << 56) |
	(UQUAD(buffer[6]) << 48) |
	(UQUAD(buffer[5]) << 40) |
	(UQUAD(buffer[4]) << 32) |
	(UQUAD(buffer[3]) << 24) |
	(UQUAD(buffer[2]) << 16) |
	(UQUAD(buffer[1]) <<  8) |
	(UQUAD(buffer[0]) <<  0);
      buffer += 8;
    }
    return v;
  }
  //
  // Extract bits from a byte buffer, possibly update it.
  // bitpos is the number of bits left in the current buffer byte.
  static inline ULONG GetBits(UBYTE *&buffer,UBYTE &bitpos,UBYTE bits,bool issigned)
  {
    ULONG res  = 0;
    UBYTE sign = bits;
    
    do {
      UBYTE avail = bitpos;
      if (avail > bits)
	avail = bits; // do not remove more bits than requested.
      
      // remove avail bits from the byte
      res     = (res << avail) | ((buffer[0] >> (bitpos - avail)) & ((1UL << avail) - 1));
      bits   -= avail;
      bitpos -= avail;
      if (bitpos == 0) {
	buffer++;
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
};
///

#endif
