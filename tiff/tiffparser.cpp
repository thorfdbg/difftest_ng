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
** This file contains a simple tiff parser class. This phases the need for
** libtiff out as it does not support tiffs with varying bit depths.
**
** $Id: tiffparser.cpp,v 1.13 2016/06/04 10:44:10 thor Exp $
**
*/

/// Includes
#include "std/errno.hpp"
#include "std/assert.hpp"
#include "tiff/tiffparser.hpp"
#include "img/imglayout.hpp"
#include "tiff/tifftags.hpp"
#include "std/string.hpp"
///

/// TiffParser::TiffParser
// Construct the tiff parser from a file name.
TiffParser::TiffParser(const char *filename)
  : m_pFile(NULL), m_pulBitsPerPixel(NULL), m_pulColorMap(NULL), 
    m_pulSubsampling(NULL), m_pulSampleFormats(NULL),
    m_pulStripByteCount(NULL), m_pulStripOffset(NULL),
    m_ulUnits(0), m_pucBuffer(NULL), m_ulBufferSize(0)
{
  char header[4];
  
  m_pcFilename = filename; // keep it.
  m_pFile      = fopen(filename,"rb");
  if (m_pFile == NULL) {
    ImageLayout::PostError("Unable to open %s: %s",filename,strerror(errno));
    return; // not necessary;
  }

  try {
    if (fread(header,sizeof(char),4,m_pFile) == 4) {
      if (header[0] == 'I' && header[1] == 'I') {
	m_bBigEndian = false;
      } else if (header[0] == 'M' && header[1] == 'M') {
	m_bBigEndian = true;
      } else {
	ImageLayout::PostError("%s is not a TIFF file",filename);
	return; // not necessary
      }
    } else {
      ImageLayout::PostError("%s is not a TIFF file, cannot read header",filename);
      return; // not necessary;
    }
    
    if ((m_bBigEndian  && (header[2] != 0  || header[3] != 42)) ||
	(!m_bBigEndian && (header[2] != 42 || header[3] != 0))) {
      ImageLayout::PostError("%s contains an invalid TIFF version number",filename);
      return; // not necessary
    }

    // Now get the location of the first IFD
    m_ulIFDPos = GetLong();
  } catch(...) {
    fclose(m_pFile);
    throw;
  }
}
///

/// TiffParser::~TiffParser
TiffParser::~TiffParser(void)
{
  if (m_pFile) {
    fclose(m_pFile); m_pFile = NULL;
  }
  delete[] m_pulBitsPerPixel;
  delete[] m_pulColorMap;
  delete[] m_pulSubsampling;
  delete[] m_pulSampleFormats;
  delete[] m_pulStripByteCount;
  delete[] m_pulStripOffset;
  delete[] m_pucBuffer;
}
///

/// TiffParser::GetByte
// Read a one-byte entry from the file.
UBYTE TiffParser::GetByte(void)
{
  UBYTE buf;

  errno = 0;
  if (fread(&buf,sizeof(buf),1,m_pFile) != 1) {
    if (errno != 0) {
      ImageLayout::PostError("%s: reading from %s, not a valid TIFF file",strerror(errno),m_pcFilename);
      return 0;
    } else {
      ImageLayout::PostError("unexpected EOF while reading from %s, not a valid TIFF file",m_pcFilename);
      return 0;
    }
  }
  return buf;
}
///

/// TiffParser::GetWord
// Read a two-byte entry, be endian-aware.
UWORD TiffParser::GetWord(void)
{
  UBYTE lo,hi;

  if (m_bBigEndian) {
    hi = GetByte();
    lo = GetByte();
  } else {
    lo = GetByte();
    hi = GetByte();
  }

  return UWORD(lo) | (UWORD(hi) << 8);
}
///

/// TiffParser::GetLong
// Read a four-byte entry, be endian-aware.
ULONG TiffParser::GetLong(void)
{
  UWORD lo,hi;

  if (m_bBigEndian) {
    hi = GetWord();
    lo = GetWord();
  } else {
    lo = GetWord();
    hi = GetWord();
  }

  return ULONG(lo) | (ULONG(hi) << 16);
}
///

/// TiffParser::GetFloat
// Read a single-precision IEEE float.
FLOAT TiffParser::GetFloat(void)
{
  union {
    ULONG ul;
    FLOAT f;
  } u;

  u.ul = GetLong();

  return u.f;
}
///

/// TiffParser::GetDouble
// Read a double-precision IEEE float
DOUBLE TiffParser::GetDouble(void)
{
  union {
    UQUAD uq;
    DOUBLE d;
  } u;
  ULONG lo,hi;

  if (m_bBigEndian) {
    hi = GetLong();
    lo = GetLong();
  } else {
    lo = GetLong();
    hi = GetLong();
  }

  u.uq = UQUAD(lo) | (UQUAD(hi) << 32);

  return u.d;
}
///

/// TiffParser::Seek
// Seek to the indicated offset, throw on error.
void TiffParser::Seek(ULONG pos)
{  
  if (pos > MAX_LONG) {
    ImageLayout::PostError("Error parsing %s: file offset is beyond 2GB barrier",m_pcFilename);
  }
  
  if (fseek(m_pFile,pos,SEEK_SET) < 0) {
    ImageLayout::PostError("%s: cannot seek in TIFF file %s",strerror(errno),m_pcFilename);
  }
}
///

/// TiffParser::FindTag
// Position the file pointer on the tiff tag at the IFD starting
// at the indicated position. Return true if this entry exists.
// Return false otherwise. Note that this is not very fast as a 
// lot of seeking is done, but nevermind.
bool TiffParser::FindTag(UWORD matchtag)
{
  UWORD entries;
  
  Seek(m_ulIFDPos);

  entries = GetWord();
  while(entries) {
    UWORD tag = GetWord();
    if (tag == matchtag)
      return true; // found the entry.
    //
    // Otherwise, skip the entry. It follows a type field, followed by a four-byte count field,
    // followed by a value/offset field.
    GetWord();
    GetLong();
    GetLong();
    entries--;
  }

  return false;
}
///

/// TiffParser::GetScalarTag
// Read a scalar entry from the TIFF directory, return
// true if found, otherwise return false.
bool TiffParser::GetScalarTag(UWORD matchtag,ULONG &value)
{
  if (FindTag(matchtag)) {
    UWORD type  = GetWord();
    ULONG count = GetLong();
    if (count != 1) {
      ImageLayout::PostError("Expected a scalar type when parsing tag %d in file %s, invalid TIFF",matchtag,m_pcFilename);
      return false;
    }
    // Only numeric types are supported here.
    switch(type) {
    case 1: // byte entry.
      value = GetByte(); // directly in the entry.
      return true;
    case 3: // short entry.
      value = GetWord();
      return true;
    case 4: // long entry.
      value = GetLong();
      return true;
    default:
      ImageLayout::PostError("Expected a numeric integer type when parsing tag %d in file %s, invalid TIFF",matchtag,m_pcFilename);
      return false;
    }
  }
  return false;
}
///

/// TiffParser::GetScalarTag
// Read a scalar entry from the TIFF directory, return
// true if found, otherwise return false. This handles
// also floating point tags.
bool TiffParser::GetScalarTag(UWORD matchtag,DOUBLE &value)
{
  if (FindTag(matchtag)) {
    UWORD type  = GetWord();
    ULONG count = GetLong();
    if (count != 1) {
      ImageLayout::PostError("Expected a scalar type when parsing tag %d in file %s, invalid TIFF",matchtag,m_pcFilename);
      return false;
    }
    // Only numeric types are supported here.
    switch(type) {
    case 1: // byte entry.
      value = GetByte(); // directly in the entry.
      return true;
    case 3: // short entry.
      value = GetWord();
      return true;
    case 4: // long entry.
      value = GetLong();
      return true;
    case 6: // Byte
      value = (BYTE)(GetByte());
      return true;
    case 8: // signed word.
      value = (WORD)(GetWord());
      return true;
    case 9: // signed long.
      value = (LONG)(GetLong());
      return true;
    case 11: // A single precision IEEE float.
      value = GetFloat();
      return true;
    default:
      ImageLayout::PostError("Expected a numeric integer type when parsing tag %d in file %s, invalid TIFF",matchtag,m_pcFilename);
      return false;
    }
  }
  return false;
}
///

/// TiffParser::GetVectorTag
// Read a vectorial type from the TIFF directory,
// return true if found, then the type is allocated and the size is
// returned. Otherwise, false is returned.
bool TiffParser::GetVectorTag(UWORD matchtag,ULONG &size,ULONG *&vector)
{
  ULONG *tmp = NULL;
  assert(vector == NULL);

  try {
    if (FindTag(matchtag)) {
      ULONG i;
      UWORD type  = GetWord();
      ULONG count = GetLong();
      //
      // Note that count == 0 is also a valid count.
      tmp         = new ULONG[count];
      switch(type) {
      case 1: // byte entry.
	// At most four bytes fit into the IFD. Otherwise, it is an offset into the file.
	if (count <= 4) {
	  for(i = 0;i < count;i++) {
	    tmp[i] = GetByte();
	  }
	} else {
	  Seek(GetLong()); // is an offset.
	  for(i = 0;i < count;i++) {
	    tmp[i] = GetByte();
	  }
	}
	break;
      case 3: // short entry.
	// At most two shorts fit into the IFD. Otherwise, it is an offset into the file.
	if (count <= 2) {
	  for(i = 0;i < count;i++) {
	    tmp[i] = GetWord();
	  }
	} else {
	  Seek(GetLong()); // is an offset.
	  for(i = 0;i < count;i++) {
	    tmp[i] = GetWord();
	  }
	}
	break;
      case 4:
	if (count <= 1) {
	  for(i = 0;i < count;i++) { // or zero...
	    tmp[i] = GetLong();
	  }
	} else {
	  Seek(GetLong()); // is an offset.
	  for(i = 0;i < count;i++) {
	    tmp[i] = GetLong();
	  }
	}
	break;
      default:
	ImageLayout::PostError("Expected a numeric integer type when parsing tag %d in file %s, invalid TIFF",matchtag,m_pcFilename);
	break;
      }
      size   = count;
      vector = tmp;
      return true;
    } else {
      // Not found, do nothing.
      return false;
    }
  } catch(...) {
    delete[] tmp;
    throw;
  }
  return false;
}
///

/// TiffParser::GetImageWidth
ULONG TiffParser::GetImageWidth()
{
  ULONG width;

  if (!GetScalarTag(TiffTag::IMAGEWIDTH,width)) {
    ImageLayout::PostError("%s is invalid: image width not specified",m_pcFilename);
    return 0;
  }
  return width;
}
///

/// TiffParser::GetImageHeight
ULONG TiffParser::GetImageHeight()
{ 
  ULONG height;

  if (!GetScalarTag(TiffTag::IMAGELENGTH,height)) {
    ImageLayout::PostError("%s is invalid: image length not specified",m_pcFilename);
    return 0;
  }
  return height;
}
///

/// TiffParser::GetPhotometricInterpretation
// Return the photometric interpretation of the file.
// If this is *NOT* present, assume RGB.
ULONG TiffParser::GetPhotometricInterpretation()
{
  ULONG photo;

  if (GetScalarTag(TiffTag::PHOTOMETRIC,photo))
    return photo;

  return TiffTag::Photometric::RGB;
}
///

/// TiffParser::GetImageDepth
// Return the depth of the image in components. Depends on the
// photometric interpretation or the tag.
ULONG TiffParser::GetImageDepth()
{
  ULONG depth;
  ULONG photo;

  if (GetScalarTag(TiffTag::PHOTOMETRIC,photo)) {
    if (GetScalarTag(TiffTag::SAMPLESPERPIXEL,depth)) {
      ULONG extra = 0;
      if (GetScalarTag(TiffTag::EXTRASAMPLES,extra)) {
	extra = 1; // The extra sample type does not matter here.
	if (extra >= depth)
	  ImageLayout::PostError("%s has no image samples, only extra samples",m_pcFilename);
      }
      // Check for consistency.
      switch(photo) {
      case TiffTag::Photometric::MINISWHITE:
      case TiffTag::Photometric::MINISBLACK:
	if (depth - extra != 1)
	  ImageLayout::PostError("%s indicates black&white content, but has more than one sample per pixel",m_pcFilename);
	break;
      case TiffTag::Photometric::RGB:
	if (depth - extra < 3)
	  ImageLayout::PostError("%s indicates RGB content, but has less than three samples per pixel",m_pcFilename);
	break;
      case TiffTag::Photometric::PALETTE:
	if (depth - extra != 1)
	  ImageLayout::PostError("%s indicates palette mapping, but has more than one sample per pixel",m_pcFilename);
	break;
      case TiffTag::Photometric::YCBCR:
	if (depth - extra < 3)
	  ImageLayout::PostError("%s indicates YCbCr content, but has less than three samples per pixel",m_pcFilename);
	break;
	// Everything else is not checked, and unknown.
      }
      return depth;
    } else {
      switch(photo) {
      case TiffTag::Photometric::MINISWHITE:
      case TiffTag::Photometric::MINISBLACK:
      case TiffTag::Photometric::PALETTE:
	return 1;
      case TiffTag::Photometric::RGB:
      case TiffTag::Photometric::YCBCR:
	return 3;
      }
    }
  } else if (GetScalarTag(TiffTag::SAMPLESPERPIXEL,depth)) {
    return depth;
    // Cannot check, photo tag is missing.
  }
  //
  // Neither is there. Bummer!
  ImageLayout::PostError("%s includes neither photometric tag nor an image dimensions",m_pcFilename);
  return 0; // not valid.
}
///

/// TiffParser::GetBitsPerPixel 
// Return an array of the number of bits per pixel. Its
// size is identical to the image depths.
const ULONG *TiffParser::GetBitsPerPixel(void)
{
  ULONG count;
  ULONG depth = GetImageDepth(); // required to test for validity.
 
  if (m_pulBitsPerPixel)
    return m_pulBitsPerPixel; // is buffered.

  if (GetVectorTag(TiffTag::BITSPERSAMPLE,count,m_pulBitsPerPixel)) {
    if (depth != count)
      ImageLayout::PostError("%s does not indicate the sample precision for each channels",m_pcFilename);
    return m_pulBitsPerPixel;
  }
  
  // Assume 8bpp
  m_pulBitsPerPixel = new ULONG[depth];
  
  for(count = 0;count < depth;count++) {
    m_pulBitsPerPixel[count] = 8;
  }
  
  return m_pulBitsPerPixel;
}
///

/// TiffParser::GetCompression
// Return the compression type. Actually, not much can be
// handled by simpletiff anyhow.
ULONG TiffParser::GetCompression(void)
{
  ULONG type;

  if (!GetScalarTag(TiffTag::COMPRESSION,type)) {
    // Hmpf. Assume none.
    return TiffTag::Compression::NONE;
  }
  return type;
}
///

/// TiffParser::GetPredictor
// Get the prediction mode, only for LZW
ULONG TiffParser::GetPredictor(void)
{
  ULONG type;

  if (!GetScalarTag(TiffTag::PREDICTOR,type)) {
    // Assume none.
    return TiffTag::Predictor::NONE;
  }
  return type;
}
///

/// TiffParser::GetColorMap
// Return the colormap of the TIFF file if it is a lookup model.
// Otherwise, return null.
const ULONG *TiffParser::GetColorMap(void)
{
  const ULONG *precs;
  ULONG count;

  if (m_pulColorMap)
    return m_pulColorMap;

  if (GetPhotometricInterpretation() != TiffTag::Photometric::PALETTE)
    return NULL;

  precs = GetBitsPerPixel(); // required to verify the size of the array.
  if (precs[0] >= 32 || precs[0] == 0)
    ImageLayout::PostError("%s defines an invalid sample precision, must be > 0 and <32",m_pcFilename);

  if (GetVectorTag(TiffTag::COLORMAP,count,m_pulColorMap)) {
    if (count != 3 * (1UL << precs[0]))
      ImageLayout::PostError("the palette size of %s does not coincide with the sample precision",m_pcFilename);

    return m_pulColorMap;
  }

  ImageLayout::PostError("%s is a palette mapped file, but does not define a color map",m_pcFilename);
  return NULL;
}
///

/// TiffParser::GetSubsampling
// Return the subsampling factors in X and Y direction for
// YCbCr images. This is an array of two LONGs, or NULL
// in case the image is not in YCbCr.
const ULONG *TiffParser::GetSubsampling(void)
{
  ULONG count;
  
  if (m_pulSubsampling)
    return m_pulSubsampling;

  if (GetPhotometricInterpretation() != TiffTag::Photometric::YCBCR)
    return NULL;

  if (GetVectorTag(TiffTag::YCBCRSUBSAMPLING,count,m_pulSubsampling)) {
    if (count != 2)
      ImageLayout::PostError("%s is a YCbCr encoded image, but does not define exactly two subsampling factors",m_pcFilename);
    return m_pulSubsampling;
  }
  
  // The default is 2,2.
  m_pulSubsampling = new ULONG[2];
  m_pulSubsampling[0] = 2;
  m_pulSubsampling[1] = 2;

  return m_pulSubsampling;
}
///

/// TiffParser::GetPlanarConfig
// Get the planar configuration of the file, whether samples
// are organized in separate planes or interleaved.
ULONG TiffParser::GetPlanarConfig(void)
{
  ULONG config;

  if (GetScalarTag(TiffTag::PLANARCONFIG,config)) {
    if (config != TiffTag::Planarconfig::CONTIG && config != TiffTag::Planarconfig::SEPARATE)
      ImageLayout::PostError("%s defines an invalid planar configuration",m_pcFilename);
    return config;
  }
  
  // The default is contiguous, i.e. interleaved.
  return TiffTag::Planarconfig::CONTIG;
}
///

/// TiffParser::GetSampleFormat
// Return the sample format for the pixels. Note that this is defined separately
// for each channel.
const ULONG *TiffParser::GetSampleFormat(void)
{  
  ULONG count,i;

  if (m_pulSampleFormats)
    return m_pulSampleFormats;

  if (GetVectorTag(TiffTag::SAMPLEFORMAT,count,m_pulSampleFormats)) {
    ULONG depth = GetImageDepth();
    if (count == 1 && depth != 1) {
      ULONG fmt = m_pulSampleFormats[0];
      fprintf(stderr,"Warning: Invalid TIFF, must provide %d sample formats, not just one.\n",
	      depth);
      delete[] m_pulSampleFormats;m_pulSampleFormats = NULL;
      m_pulSampleFormats = new ULONG[depth];
      for (i = 0;i < depth;i++) {
	m_pulSampleFormats[i] = fmt;
      }
      return m_pulSampleFormats;
    }
    if (count != depth)
      ImageLayout::PostError("%s defines sample formats, but not one per channel",m_pcFilename);
    return m_pulSampleFormats;
  }

  // Otherwise, the default is all UINT.
  count = GetImageDepth();
  m_pulSampleFormats = new ULONG[count];
  for(i = 0;i < count;i++) {
    m_pulSampleFormats[i] = TiffTag::Sampleformat::UINT;
  }

  return m_pulSampleFormats;
}
///

/// TiffParser::GetRowsPerStrip
// Return the number of rows per strip for striped images.
ULONG TiffParser::GetRowsPerStrip(void)
{
  ULONG rows;

  if (GetScalarTag(TiffTag::ROWSPERSTRIP,rows)) {
    ULONG height = GetImageHeight();
    if (rows == 0)
      ImageLayout::PostError("%s defines an invalid number of rows per strip, must be > 0",m_pcFilename);
    if (rows > height)
      rows = height; // is one strip anyhow...
    return rows;
  }

  return MAX_ULONG; // the default value.
}
///

/// TiffParser::GetStripsPerImage
// Return the number of strips per image.
ULONG TiffParser::GetStripsPerImage(void)
{
  ULONG rows;

  if (GetScalarTag(TiffTag::ROWSPERSTRIP,rows)) {
    ULONG height = GetImageHeight();
    if (rows == 0)
      ImageLayout::PostError("%s defines an invalid number of rows per strip, must be > 0",m_pcFilename);
    
    if (rows > height)
      return 1;
    return (height + rows - 1) / rows;
  }

  // Otherwise, one strip.
  return 1;
}
///

/// TiffParser::GetAddressableStrips
// If the configuration is "SEPARATE", then each channel goes into
// a separate storage strip, and thus while the number of
// geometrical strips remains the same, the number of addressable
// units change. This is also the size of the array returned below.
ULONG TiffParser::GetAddressableStrips(void)
{
  if (m_ulUnits == 0) {
    switch(GetPlanarConfig()) {
    case TiffTag::Planarconfig::CONTIG: // Interleaved: The easy case.
      return m_ulUnits = GetStripsPerImage();
    case TiffTag::Planarconfig::SEPARATE:
      return m_ulUnits = GetImageDepth() * GetStripsPerImage();
    }
    return 0; // code never goes here.
  }
  return m_ulUnits;
}
///

/// TiffParser::GetStripByteCount
// Return the number of bytes for each strip.
const ULONG *TiffParser::GetStripByteCount(void)
{
  ULONG count;
  
  if (m_pulStripByteCount)
    return m_pulStripByteCount;

  if (GetVectorTag(TiffTag::STRIPBYTECOUNTS,count,m_pulStripByteCount)) {
    ULONG strips = GetAddressableStrips();
    if (count != strips)
      ImageLayout::PostError("%s does not define the proper number of strip byte counts, %d are specified, %d expected",
		m_pcFilename,strips,count);
    return m_pulStripByteCount;
  }

  ImageLayout::PostError("%s does not define the number of bytes for each strip, invalid TIFF",m_pcFilename);
  return NULL;
}
///

/// TiffParser::GetStripOffset
// Return the file offsets into the strips.
const ULONG *TiffParser::GetStripOffset(void)
{
  ULONG count;

  if (m_pulStripOffset)
    return m_pulStripOffset;

  if (GetVectorTag(TiffTag::STRIPOFFSETS,count,m_pulStripOffset)) {
    ULONG strips = GetAddressableStrips();
    if (count != strips)
      ImageLayout::PostError("%s does not define the proper number of strip byte counts, %d are specified, %d expected",
		m_pcFilename,strips,count);
    return m_pulStripOffset;
  }
  ImageLayout::PostError("%s does not define the file offsets for each strip, invalid TIFF",m_pcFilename);
  return NULL;
}
///

/// TiffParser::isTiled
// Return an indicator whether this TIFF file contains tiles.
bool TiffParser::isTiled(void)
{
  ULONG tw;
  
  if (GetScalarTag(TiffTag::TILEWIDTH,tw))
    return true;
  
  return false;
}
///

/// TiffParser::GetTileWidth
// Get the width of all tiles.
ULONG TiffParser::GetTileWidth(void)
{
  ULONG tw;
  
  if (GetScalarTag(TiffTag::TILEWIDTH,tw))
    return tw;
  
  ImageLayout::PostError("%s is a tiled image but does not define the tile width",m_pcFilename);
  return 0;
}
///

/// TiffParser::GetTileHeight
// Get the height of all tiles.
ULONG TiffParser::GetTileHeight(void)
{
  ULONG th;
  
  if (GetScalarTag(TiffTag::TILELENGTH,th))
    return th;
  
  ImageLayout::PostError("%s is a tiled image but does not define the tile height",m_pcFilename);
  return 0;
}
///

/// TiffParser::GetScaleFactor
// Get the sample value to NITS conversion factor, or 1.0 if it is
// not recorded.
DOUBLE TiffParser::GetScaleFactor(void)
{
  DOUBLE scale = 1.0;

  if (GetScalarTag(TiffTag::STONITS,scale))
    return scale;

  return 1.0;
}
///

/// TiffParser::GetTilesPerImage
// Return the number of tiles in the image.
ULONG TiffParser::GetTilesPerImage(void)
{
  ULONG w     = GetImageWidth();
  ULONG h     = GetImageHeight();
  ULONG tw    = GetTileWidth();
  ULONG th    = GetTileHeight();
  UQUAD tiles = ((w + tw - 1) / tw) * ((h + th - 1) / th);

  if (tiles > MAX_ULONG)
    ImageLayout::PostError("%s defines too many tiles, cannot process",m_pcFilename);

  return ULONG(tiles);
}
///

/// TiffParser::GetAddressableTiles
// If the configuration is "SEPARATE", then each channel goes into
// a separate storage tile, and thus while the number of
// geometrical tiles remains the same, the number of addressable
// units change. This is also the size of the array returned below.
ULONG TiffParser::GetAddressableTiles(void)
{  
  if (m_ulUnits == 0) {
    switch(GetPlanarConfig()) {
    case TiffTag::Planarconfig::CONTIG: // Interleaved: The easy case.
      return m_ulUnits = GetTilesPerImage();
    case TiffTag::Planarconfig::SEPARATE:
      return m_ulUnits = GetImageDepth() * GetTilesPerImage();
    }
    return 0; // code never goes here.
  }
  return m_ulUnits;
}
///

/// TiffParser::GetTileByteCount
// Return the array of tile byte counts, one entry per tile.
const ULONG *TiffParser::GetTileByteCount(void)
{
  ULONG cnt;
  
  if (m_pulStripByteCount)
    return m_pulStripByteCount;

  if (GetVectorTag(TiffTag::TILEBYTECOUNTS,cnt,m_pulStripByteCount)) {
    ULONG tiles = GetAddressableTiles();
    if (tiles != cnt)
      ImageLayout::PostError("%s does not define the proper number of byte counts for all tiles, expected %d found %d",
		m_pcFilename,tiles,cnt);
    return m_pulStripByteCount;
  } else if (GetVectorTag(TiffTag::STRIPBYTECOUNTS,cnt,m_pulStripByteCount)) {
    // Bummer! Some images store this in the STRIPBYTECOUNTS!
    ULONG tiles = GetAddressableTiles();
    if (tiles != cnt)
      ImageLayout::PostError("%s does not define the proper number of byte counts for all tiles, expected %d found %d",
		m_pcFilename,tiles,cnt);
    return m_pulStripByteCount;
  }

  ImageLayout::PostError("%s does not define the tile byte counts, invalid TIFF",m_pcFilename);
  return NULL;
}
///

/// TiffParser::GetTileOffset
// Return the array of tile file offsets, one entry per tile.
const ULONG *TiffParser::GetTileOffset(void)
{
  ULONG cnt;
  
  if (m_pulStripOffset)
    return m_pulStripOffset;

  if (GetVectorTag(TiffTag::TILEOFFSETS,cnt,m_pulStripOffset)) {
    ULONG tiles = GetAddressableTiles();
    if (tiles != cnt)
      ImageLayout::PostError("%s does not define the proper number of offsets for all tiles, expected %d found %d",
		m_pcFilename,tiles,cnt);
    return m_pulStripOffset;
  } else if (GetVectorTag(TiffTag::STRIPOFFSETS,cnt,m_pulStripOffset)) {
    // Bummer! Some images store this in the strip offsets!
    ULONG tiles = GetAddressableTiles();
    if (tiles != cnt)
      ImageLayout::PostError("%s does not define the proper number of offsets for all tiles, expected %d found %d",
		m_pcFilename,tiles,cnt);
    return m_pulStripOffset;
  }

  ImageLayout::PostError("%s does not define the tile offsets, invalid TIFF",m_pcFilename);
  return NULL;
}
///

/// TiffParser::GetDataOfUnit
// Return the data for addressable unit "i" (where i is either
// a tile, tile component, stripe or stripe component). The
// data is *not* endian-corrected, but read in in raw.
UBYTE *TiffParser::GetDataOfUnit(ULONG i,ULONG &size)
{
  ULONG bufsiz;
  
  if (m_pulStripOffset == NULL || m_pulStripByteCount == NULL) {
    if (isTiled()) {
      GetTileByteCount();
      GetTileOffset();
    } else {
      GetStripByteCount();
      GetStripOffset();
    }
  }
  assert(m_ulUnits > 0);
  assert(i < m_ulUnits);

  bufsiz = m_pulStripByteCount[i];
  if (bufsiz > m_ulBufferSize || m_pucBuffer == NULL) {
    delete[] m_pucBuffer;m_pucBuffer = NULL;
    m_pucBuffer = new UBYTE[m_ulBufferSize = bufsiz];
  }

  Seek(m_pulStripOffset[i]);

  errno = 0;
  if (fread(m_pucBuffer,sizeof(UBYTE),bufsiz,m_pFile) != bufsiz) {
    if (errno) {
      ImageLayout::PostError("%s: while reading the TIFF file %s",strerror(errno),m_pcFilename);
    } else {
      ImageLayout::PostError("unexpected EOF, file %s is truncated",m_pcFilename);
    }
  }

  size = bufsiz;
  return m_pucBuffer;
}
///
