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
 * This class saves and loads images in the dpx format.
 *
 * $Id: simpledpx.cpp,v 1.22 2020/02/05 11:28:21 thor Exp $
 */

/// Includes
#include "interface/types.hpp"
#include "img/imglayout.hpp"
#include "img/simpledpx.hpp"
#include "img/imgspecs.hpp"
#include "std/stdio.hpp"
#include "tools/file.hpp"
///

/// SimpleDPX::SimpleDPX
// default constructor
SimpleDPX::SimpleDPX(void)
  : m_bLittleEndian(true), m_bLeftToRightScan(false), m_bFlipX(false), m_bFlipY(false), m_bFlipXY(false)
{
}
///

/// SimpleDPX::SimpleDPX
// Copy the image from another source for later saving.
SimpleDPX::SimpleDPX(const class ImageLayout &layout)
  : ImageLayout(layout), m_bLittleEndian(true), m_bLeftToRightScan(false), m_bFlipX(false), m_bFlipY(false), m_bFlipXY(false)
{
}
///

/// SimpleDPX::~SimpleDPX
// Destroy the simple DPX loader
SimpleDPX::~SimpleDPX(void)
{
  // Elements are destroyed as members of this class.
}
///

/// SimpleDPX::BuildScanPattern
// Build the scan pattern for the given element and element description
// Returns true for YUV scans.
bool SimpleDPX::BuildScanPattern(struct ImageElement *el)
{
  struct ScanElement **slp = &(el->m_pScanPattern);
  bool yuv = false;
  UBYTE i;

  switch(el->m_ucDescriptor) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 6:
  case 4:
  case 8: // Depth image. Ok, encode this as alpha for the time being.
    // This is alpha only
    // These are single-component elements.
    if (el->m_ucDescriptor == 4 || el->m_ucDescriptor == 8) {
      el->m_ucAlphaDepth = 1;
    } else {
      el->m_ucDepth      = 1;
    }
    //
    new ScanElement(slp,0);
    break;
  case 7: // This is CbCr, alternating. Is this also subsampled in Y direction?
    el->m_ucDepth      = 2;
    el->m_ucSubX       = 2;
    el->m_ucSubY       = 2; // I hope so...
    yuv                = true;
    //
    // Create two scan elements. The first scans Cb, the second Cr.
    new ScanElement(slp,0);
    new ScanElement(slp,1);
    /* Falls through. */
  case 9:
    // "Composite video". Largely underspecified. No idea what the samples are here.
    yuv = true;
    PostError("found composite video element in DPX file %s. "
	      "DPX specifications are unsufficient for implementation",m_pcFileName);
    break;
  case 50:
    // This is BGR (not RGB)
    el->m_ucDepth      = 3;
    // Create three scan elements in the order B,G,R.
    if (m_bLeftToRightScan) {
      // Strangely enough, programs fall back to RGB
      // if the scan order is wrong.
      new ScanElement(slp,0);
      new ScanElement(slp,1);
      new ScanElement(slp,2);
    } else {
      new ScanElement(slp,2);
      new ScanElement(slp,1);
      new ScanElement(slp,0);
    }
    break;
  case 51:
    // This is BGRA (not RGBA)
    el->m_ucDepth      = 3;
    el->m_ucAlphaDepth = 1;
    // Create three scan elements in the order B,G,R,A.
    if (m_bLeftToRightScan) {
      // Actually, I do not really know what programs do here...
      new ScanElement(slp,0);
      new ScanElement(slp,1);
      new ScanElement(slp,2);
      new ScanElement(slp,MAX_UWORD);
    } else {
      new ScanElement(slp,2);
      new ScanElement(slp,1);
      new ScanElement(slp,0);
      new ScanElement(slp,MAX_UWORD);
    }
    break;
  case 52:
    // This is ARGB
    el->m_ucDepth      = 3;
    el->m_ucAlphaDepth = 1;
    // Create three scan elements in the order A,R,G,B
    new ScanElement(slp,MAX_UWORD);
    new ScanElement(slp,0);
    new ScanElement(slp,1);
    new ScanElement(slp,2);
    break;
  case 100:
    // This is V210, CbYCrY repeated.
    el->m_ucDepth      = 3;
    el->m_ucSubX       = 2;
    yuv                = true;
    // Create three scan elements in the order Cb,Y,Cr,Y
    new ScanElement(slp,1);
    new ScanElement(slp,0);
    new ScanElement(slp,2);
    new ScanElement(slp,0);
    break;
  case 101:
    // This is CbYACrYA
    el->m_ucDepth      = 3;
    el->m_ucAlphaDepth = 1;
    el->m_ucSubX       = 2;
    yuv                = true;
    // Create scan elements in the order Cb,Y,A,Cr,Y,A
    new ScanElement(slp,1);
    new ScanElement(slp,0);
    new ScanElement(slp,MAX_UWORD);
    new ScanElement(slp,2);
    new ScanElement(slp,0);
    new ScanElement(slp,MAX_UWORD);
    break;
  case 102:
    // This is CbYCr repeated, non-subsampled.
    el->m_ucDepth      = 3;
    yuv                = true;
    // Create scan elements in the order Cb,Y,Cr
    new ScanElement(slp,1);
    new ScanElement(slp,0);
    new ScanElement(slp,2);
    break;
  case 103:
    // This is CbYCrA repeated, non-subsampled.
    el->m_ucDepth      = 3;
    el->m_ucAlphaDepth = 1;
    yuv                = true;
    new ScanElement(slp,1);
    new ScanElement(slp,0);
    new ScanElement(slp,2);
    new ScanElement(slp,MAX_UWORD);
    break;
  case 150:
  case 151:
  case 152:
  case 153:
  case 154:
  case 155:
  case 156:
    // This is a generic n-element image, with n = 2..8.
    el->m_ucDepth      = el->m_ucDescriptor - 150 + 2;
    for(i = 0;i < el->m_ucDepth;i++) {
      new ScanElement(slp,i);
    }
    break;
  default:
    PostError("found unspecified DPX element descriptor in file %s",m_pcFileName);
    break;
  }

  return yuv;
}
///

/// SimpleDPX::ParseElementHeader
// Parse a single element header from the given file.
bool SimpleDPX::ParseElementHeader(FILE *file,struct ImageElement *el)
{
  ULONG sign;
  UBYTE xfer,color;
  UWORD packing,encoding;
  bool yuv = false;
  //
  // Read an element.
  sign = GetLong(file);
  switch(sign) {
  case 0:
    el->m_bSigned = false;
    break;
  case 1:
    el->m_bSigned = true;
    break;
  default:
    PostError("found invalid signedness indicator in DPX file %s, must be either 1 or 0",m_pcFileName);
    break;
  }
  // Next four are luma scaling attributes we do not need to support.
  SkipBytes(file,4+4+4+4);
  //
  el->m_ucDescriptor = GetByte(file);
  //
  // Read the transfer characteristics. Not that we do anything about it.
  // thor fix: apparently, some files use 255 as indicator, probably as 'we do not know'.
  xfer = GetByte(file);
  if (xfer > 12 && xfer != 255)
    PostError("found unspecified DPX transfer characteristics in file %s",m_pcFileName);
  //
  // Read the colorimetric specification. Nothing we can make use of.
  color = GetByte(file);
  if (color > 10 && color != 255)
    PostError("found invalid colorimetric specification in DPX file %s",m_pcFileName);
  //
  el->m_ucBitDepth = GetByte(file);
  // Get the packing information. 0 is full packing,
  // 1 is with packing bits at the LSB side, 2 is with packing bits at
  // the MSB side.
  packing          = GetWord(file);
  if (packing > 2)
    PostError("found invalid DPX padding specification in file %s",m_pcFileName);
  //
  switch(el->m_ucBitDepth) {
  case 8:
  case 16:
    // These are all integer bit depths DPX allows. No padding, no packing.
    break;
  case 1:
    // 11 bit elements are packed to 32 bit LONGs
    // No padding bits.
    el->m_ucPackElements = 32;
    break;
  case 10:
    // 10 bit elements are packed to 32 bit LONGs
    // Padding is possible.
    switch(packing) {
    case 1:
      el->m_ucLSBPaddingBits = 2;
      break;
    case 2:
      el->m_ucMSBPaddingBits = 2;
      break;
    }
    el->m_ucPackElements = 3;
    break;
  case 12:
    // 12 bit elements are packed to 32 bit LONGs.
    // Three elements are packed into one 32 bit word.
    switch(packing) {
    case 1:
      el->m_ucLSBPaddingBits = 4;
      break;
    case 2:
      el->m_ucMSBPaddingBits = 4;
      break;
    }
    el->m_ucPackElements = 1;
    break;
  case 32:
  case 64:
    el->m_bFloat     = true;
    break;
  default:
    PostError("found invalid DPX bit depth in file %s",m_pcFileName);
    break;
  }
  //
  yuv = BuildScanPattern(el);
  //
  // Get the encoding information.
  encoding = GetWord(file);
  switch(encoding) {
  case 0:
    // no encoding.
    break;
  case 1:
    // RLE encoding.
    el->m_bRLE = true;
    // No idea what RLE and FLOAT is supposed to mean. How is the run length
    // encoded?
    if (el->m_bFloat)
      PostError("RLE encoding and floating point samples as found in file %s "
		"are undocumented in the DPX specs and not supported",m_pcFileName);
    break;
  default:
    PostError("found invalid DPX encoding specification in file %s",m_pcFileName);
    break;
  }
  //
  // Get the offset to the data.
  el->m_ulOffset = GetLong(file);
  //
  // Read end of line/end of frame padding.
  el->m_ulEndOfLinePadding  = GetLong(file);
  // Read end of frame padding.
  el->m_ulEndOfFramePadding = GetLong(file);
  //
  // Skip the 32 bytes description.
  SkipBytes(file,32);
  //
  return yuv;
}
///

/// SimpleDPX::ParseHeader
// Parse the file header of a DPX file
void SimpleDPX::ParseHeader(FILE *file,struct ImgSpecs &specs)
{
  ULONG hdr;
  char version[9];
  ULONG encryption;
  UWORD orientation;
  UWORD depth,alpha;
  ULONG width,height;
  UWORD i;
  bool yuv;
  struct ComponentLayout *cl;

  m_bLittleEndian = false;

  hdr = GetLong(file);

  if (hdr == MakeID('S','D','P','X')) {
    // This is big-endian.
    m_bLittleEndian = false;
  } else if (hdr == MakeID('X','P','D','S')) {
    m_bLittleEndian = true;
  } else {
    PostError("input file %s is not a valid DPX file",m_pcFileName);
  }

  // thor: ugly workaround, let's assume for a while that
  // bit endian files are stored such that we read from the MSB.
  // This is not following the specs, but apparently how it is done.
  m_bLeftToRightScan = !m_bLittleEndian;
  
  m_ulDataOffset = GetLong(file);
  
  version[8] = 0;
  GetString(file,version,8);

  if (strcmp(version,"V1.0") && strcmp(version,"V2.0"))
    PostError("unsupported DPX version detected, only V1.0 and V2.0 are supported");

  SkipBytes(file,8); // file size and ditto key are not really needed.
  //
  // Read sizes of data structures.
  m_ulGenericSize  = GetLong(file);
  m_ulIndustrySize = GetLong(file);
  m_ulUserSize     = GetLong(file);
  //
  // Skip over the bytes we do not care about.
  SkipBytes(file,100+24+100+200+200);
  //
  // Read the encyrption key.
  encryption       = GetLong(file);
  if (encryption  != ULONG(~0UL))
    fprintf(stderr,"Warning! - DPX file %s signals encryption, output may be wrong.\n",m_pcFileName);
  //
  // Skip over the rest of the header. not needed.
  SkipBytes(file,104);
  
  //
  // Read the image orientation
  orientation      = GetWord(file);
  if (orientation >= 8)
    PostError("unknown orientation specified in DPX file %s, must be between 0 and 7",m_pcFileName);
  m_bFlipX  = (orientation & 1)?true:false;
  m_bFlipY  = (orientation & 2)?true:false;
  m_bFlipXY = (orientation & 4)?true:false;

  m_usElements     = GetWord(file);
  if (m_usElements == 0 || m_usElements > 8)
    PostError("number of image elements must be between 1 and 8 in DPX file %s",m_pcFileName);

  width  = GetLong(file);
  height = GetLong(file);
  if (m_bFlipXY) {
    m_ulHeight       = width;
    m_ulWidth        = height;
  } else {
    m_ulWidth        = width;
    m_ulHeight       = height;
  }
  yuv                = false;
  specs.ASCII        = ImgSpecs::No;
  specs.Interleaved  = (m_usElements == 1)?(ImgSpecs::Yes):(ImgSpecs::No);
  specs.Palettized   = ImgSpecs::No;
  specs.LittleEndian = (m_bLittleEndian)?(ImgSpecs::Yes):(ImgSpecs::No);
  //
  // Image elements. All eight are always present, though only the first m_usElements contain
  // valid data.
  for(i = 0;i < 8;i++) {
    if (i < m_usElements) {
      m_Elements[i].m_ulWidth  = width;
      m_Elements[i].m_ulHeight = height;
      if (ParseElementHeader(file,m_Elements + i))
	yuv = true;
    } else {
      // Skip over elements we do not need
      SkipBytes(file,4*5 + 1*4 + 2*2 + 4*3 + 32);
    }
  }
  //
  // The rest is junk we do not need.
  if (specs.YUVEncoded == ImgSpecs::Unspecified)
    specs.YUVEncoded = yuv?(ImgSpecs::Yes):(ImgSpecs::No);
  //
  // Now compute the depth of the image.
  depth = 0;
  alpha = 0;
  for (i = 0;i < m_usElements;i++) {
    depth += m_Elements[i].m_ucDepth;
    alpha += m_Elements[i].m_ucAlphaDepth;
  }
  CreateComponents(m_ulWidth,m_ulHeight,depth + alpha);
  //
  // Now create the planes and allocate memory.
  cl = m_pComponent;
  for(i = 0;i < m_usElements;i++) {
    struct ImageElement *el = m_Elements + i;
    struct ScanElement  *sl = el->m_pScanPattern;
    while(sl) {
      struct ComponentLayout *cll; 
      UBYTE bytesperpixel      = ImageLayout::SuggestBPP(el->m_ucBitDepth,false);
      UBYTE subx               = el->m_ucSubX;
      UBYTE suby               = el->m_ucSubY;
      UWORD k                  = sl->m_usTargetChannel;
      //
      // If flipXY is set, then X and Y change their role, so flip them now.
      if (m_bFlipXY) {
	subx = el->m_ucSubY;
	suby = el->m_ucSubX;
      }
      //
      // LUMA and alpha is always encoded without subsampling.
      if (k == 0 || k == 3) {
	subx = suby = 1;
      }
      //
      // Is this an alpha component or not?
      if (k == MAX_UWORD) {
	cll   = cl + el->m_ucDepth;
	k     = el->m_ucDepth;
      } else {
	cll   = cl + k;
      }
      cll->m_ucBits            = m_Elements[i].m_ucBitDepth;
      cll->m_bSigned           = m_Elements[i].m_bSigned;
      cll->m_bFloat            = m_Elements[i].m_bFloat;
      cll->m_ucSubX            = subx;
      cll->m_ucSubY            = suby;
      cll->m_ulWidth           = (m_ulWidth  + subx - 1) / subx;
      cll->m_ulHeight          = (m_ulHeight + suby - 1) / suby;
      cll->m_ulBytesPerPixel   = bytesperpixel;
      cll->m_ulBytesPerRow     = cll->m_ulWidth * bytesperpixel;
      // Check whether we have already data for this channel. If not, allocate now.
      // As a channel may appear multiple times in one scan pattern, make sure to
      // allocate only once.
      if (el->m_pData[k] == NULL) {
	el->m_pData[k] = new UBYTE[cll->m_ulWidth * bytesperpixel * cll->m_ulHeight];
	cll->m_pPtr    = el->m_pData[k];
	sl->m_bFirst   = true;
      }
      sl->m_pData      = cll->m_pPtr;
      sl->m_pComponent = cll;
      sl = sl->m_pNext;
    }
    // Adjust the components.
    cl += el->m_ucDepth + el->m_ucAlphaDepth;
  }
}
///

/// SimpleDPX::ReadData
// Read a single pixel from the specified file.
UQUAD SimpleDPX::ReadData(FILE *in,const struct ImageElement *el,bool issigned)
{
  // The specs seem to indicate that data is always packed into 32-bit LONGs,
  // no matter what. Interestingly, not all software seems to follow this,
  // but for this time, accept only correctly formulated data.
  switch(el->m_ucBitDepth) {
  case 32:
    return GetLong(in);
  case 64:
    {
      UQUAD q1,q2;
      q1 = GetLong(in);
      q2 = GetLong(in);
      if (m_bLittleEndian) {
	return q1 | (q1 << 32);
      } else {
	return q2 | (q1 << 32);
      }
    }
  default:
    if (el->m_ucBitDepth < 32) {
      ULONG res   = 0;
      UBYTE sign  = el->m_ucBitDepth;
      UBYTE bits  = el->m_ucBitDepth;
      UBYTE shift = 0;

      //
      // Unfortunately, some DPX files scan left to right rather than
      // right to left, reverse from what the specs say.
      if (m_bLeftToRightScan) {
	do {
	  // Refill the buffer if only the padding bits are left, or no bits are left.
	  if (m_cBit <= el->m_ucLSBPaddingBits) {
	    // Fill up the bit-buffer.
	    m_ulBitBuffer = GetLong(in);
	    m_cBit        = 32;
	    //
	    // If LSB padding is on, remove now the LSB bits.
	    m_cBit       -= el->m_ucMSBPaddingBits;
	  }
	  UBYTE avail = m_cBit;
	  if (avail > bits)
	    avail = bits; // do not remove more bits than requested.
	  
	  // remove avail bits from the long word
	  res    <<= avail;
	  res     |= (m_ulBitBuffer >> (m_cBit - avail)) & ((1UL << avail) - 1);
	  bits    -= avail;
	  m_cBit  -= avail;
	} while(bits);
      } else {
	do {
	  // Refill the buffer if only the padding bits are left, or no bits are left.
	  if (m_cBit <= el->m_ucMSBPaddingBits) {
	    // Fill up the bit-buffer.
	    m_ulBitBuffer = GetLong(in);
	    m_cBit        = 32;
	    //
	    // If LSB padding is on, remove now the LSB bits.
	    m_cBit       -= el->m_ucLSBPaddingBits;
	  }
	  UBYTE avail = m_cBit;
	  if (avail > bits)
	    avail = bits; // do not remove more bits than requested.
	  
	  // remove avail bits from the long word
	  res     |= ((m_ulBitBuffer >> (32 - m_cBit)) & ((1UL << avail) - 1)) << shift;
	  bits    -= avail;
	  shift   += avail;
	  m_cBit  -= avail;
	} while(bits);
      }

      if (el->m_ucPackElements == 1)
	m_cBit -= el->m_ucLSBPaddingBits + el->m_ucMSBPaddingBits;
      
      if (issigned) {
	if (res & (1 << (sign - 1))) { // result is negative
	  res |= ULONG(-1) << sign;
	}
      }
      return res;
    } else {
      assert(!"invalid bit depth for DPX file");
      return 0;
    }
  }
}
///

/// SimpleDPX::ParseElement
// Parse a single element of a dpx file
void SimpleDPX::ParseElement(FILE *file,struct ImageElement *el)
{
  ULONG rcnt = 0; // repeat count
  ULONG icnt = 0; // individual count
  bool linedone  = false;
  bool framedone = false;

  if (fseek(file,el->m_ulOffset,SEEK_SET) < 0)
    PostError("unable to seek to the element data in the DPX file %s",m_pcFileName);

  // Start at a new byte boundary.
  m_cBit = 0;
  do {
    framedone = true;
    do {
      //
      // First check whether we have to read a new pixel.
      if (el->m_bRLE && rcnt == 0 && icnt == 0) {
	ULONG s = ReadData(file,el,false);
	if (s & 1) {
	  rcnt = s >> 1; // all equal pixels.
	  icnt = 1; // read one pixel, write all pixels.
	} else {
	  icnt = s >> 1; // all different pixels.
	  rcnt = icnt;
	}
      } else {
	icnt = 1;
	rcnt = 1;
      }
      if (icnt > 0) {
	struct ScanElement *sl = el->m_pScanPattern;
	// Get a new individual pixel. For that, first get the pixel position.
	do {
	  // Now read the pixel.
	  sl->m_uqPrev = ReadData(file,el,el->m_bSigned);
	} while((sl = sl->m_pNext));
	icnt--;
      }
      if (rcnt > 0) {
	struct ScanElement *sl = el->m_pScanPattern;
	do {
	  struct ComponentLayout *cl = sl->m_pComponent;
	  ULONG  x = el->m_ulX[sl->m_usTargetChannel & 0x0f]; // map alpha to 15
	  ULONG  y = el->m_ulY[sl->m_usTargetChannel & 0x0f];
	  ULONG  w = cl->m_ulWidth;
	  ULONG  h = cl->m_ulHeight;
	  ULONG tx = x;
	  ULONG ty = y;
	  // Potentially flip the image.
	  if (m_bFlipX)
	    tx = w - 1 - tx;
	  if (m_bFlipY)
	    ty = h - 1 - ty;
	  if (m_bFlipXY) {
	    ULONG t = tx;
	    tx = ty;
	    ty = t;
	  }
	  // Check whether the pixel is in range.
	  if (tx < w && ty < h) {
	    APTR data;
	    // And write the data out.
	    data         = ((UBYTE *)(sl->m_pData)) + (tx * cl->m_ulBytesPerPixel) + (ty * cl->m_ulBytesPerRow);
	    if (el->m_ucBitDepth <= 8) {
	      *(UBYTE *)(data) = sl->m_uqPrev;
	    } else if (el->m_ucBitDepth <= 16) {
	      *(UWORD *)(data) = sl->m_uqPrev;
	    } else if (el->m_ucBitDepth <= 32) {
	      *(ULONG *)(data) = sl->m_uqPrev;
	    } else {
	      *(UQUAD *)(data) = sl->m_uqPrev;
	    }
	    //
	    // Advance to the next x.
	    el->m_ulX[sl->m_usTargetChannel & 0x0f]++;
	  }
	} while((sl = sl->m_pNext));
	//
	// Check whether we are at the end of the line.
	for(sl = el->m_pScanPattern,linedone = true;sl;sl = sl->m_pNext) {
	  ULONG dimension = (m_bFlipXY)?(sl->m_pComponent->m_ulHeight):(sl->m_pComponent->m_ulWidth);
	  if (el->m_ulX[sl->m_usTargetChannel & 0x0f] < dimension)
	    linedone = false;
	}
	rcnt--;
      }
    } while(!linedone);
    //
    // RLE accross lines is not supported and not allowed by the specs.
    //
    // An entire line is done. Skip the padding at the end of the line.
    SkipBytes(file,el->m_ulEndOfLinePadding);
    // Also start at a new bit boundary. Padding is broken at scan line
    // boundaries.
    m_cBit       = 0;
    //
    // Increment the Y position.
    {
      struct ScanElement *sl = el->m_pScanPattern;
      do {
	struct ComponentLayout *cl = sl->m_pComponent;
	ULONG  y = el->m_ulY[sl->m_usTargetChannel & 0x0f];
	ULONG  h = cl->m_ulHeight;
	if (y + 1 < h)
	  framedone = false;
	if (sl->m_bFirst)
	  el->m_ulY[sl->m_usTargetChannel & 0x0f]++;
	el->m_ulX[sl->m_usTargetChannel & 0x0f] = 0;
      } while((sl = sl->m_pNext));
    }
  } while(!framedone);
}
///

/// SimpleDPX::LoadImage
// Load an image from a level 1 file descriptor, keep it within
// the internals of this class. The accessor methods below
// should be used to find out more about this image.
void SimpleDPX::LoadImage(const char *name,struct ImgSpecs &specs)
{
  UWORD i;
  File input(name,"rb");
  
  m_pcFileName = name;

  ParseHeader(input,specs);
  for(i = 0;i < m_usElements;i++) {
    ParseElement(input,m_Elements + i);
  }
}
///

/// SimpleDPX::CreateElementLayout
// Generate the layout of elements given the information in
// the component layout.
void SimpleDPX::CreateElementLayout(const struct ImgSpecs &specs)
{ 
  bool yuv        = false;
  bool interleave = false;
  UWORD nextco = 0;
  struct ImageElement *el = m_Elements;
  UWORD i;
  //
  m_usElements = 0;
  // Generate here the elements to estimate the size of the file.
  // First check whether we have an explicit YCbCr image. If so, check whether
  // the subsampling factors are fine.
  if (specs.YUVEncoded == ImgSpecs::Yes) {
    yuv = true;
  } else if (specs.YUVEncoded == ImgSpecs::No) {
    yuv = false;
  } else {
    // Test whether this is something that might be YUV
    // by testing the subsampling factors.
    if (m_usDepth >= 3) {
      if (m_pComponent[1].m_ucSubX == 2 || m_pComponent[2].m_ucSubX == 2)
	yuv = true;
    }
  }
  //
  if (m_usDepth > 3) {
    if (m_pComponent[3].m_ucSubX != 1 || m_pComponent[3].m_ucSubY != 1)
      PostError("Cannot create DPX file : alpha component cannot be subsampled");
  } 
  //
  // If this is not YUV, all components must have 1-1 subsampling.
  // Otherwise, only the components 1 and 2 can have subsampling, namely
  // either 2x1 or 2x2. Everything else is out.
  if (yuv) {
    if (m_usDepth < 3)
      PostError("Cannot create DPX file : YUV data must have at least three components");
    // This requires luma to be non-subsampled.
    if (m_pComponent[0].m_ucSubX != 1 || m_pComponent[0].m_ucSubY != 1)
      PostError("Cannot create DPX file : luma component cannot be subsampled");
    if (m_pComponent[1].m_ucBits     != m_pComponent[2].m_ucBits)
      PostError("Cannot create DPX file : chroma components must have the same bitdepth");
    if (m_pComponent[1].m_bSigned    != m_pComponent[2].m_bSigned)
      PostError("Cannot create DPX file : chroma components must have the same signed-ness");
    if (m_pComponent[1].m_ucSubX     != m_pComponent[2].m_ucSubX)
      PostError("Cannot create DPX file : chroma components must have the same subsampling factors");
    if (m_pComponent[1].m_ucSubY     != m_pComponent[2].m_ucSubY)
      PostError("Cannot create DPX file : chroma components must have the same subsampling factors");
    if (m_pComponent[1].m_ucSubX > 2 || m_pComponent[1].m_ucSubY > 2)
      PostError("Cannot create DPX file : unsupported subsampling factors");
    //
    // Depending on the subsampling, we create either an interleaved or a non-interleaved component.
    // There is not much chance to respect the interleaved setting by the user.
    if (m_pComponent[1].m_ucSubY == 2) {
      // Create a separate VU-plane.
      el->SetDefault(6,m_pComponent[0]); // Luma
      // The rest of the data is derived from the descriptor.
      // Create now the chroma plane.
      el++,m_usElements++;
      if (m_pComponent[1].m_ucSubX != 2)
	PostError("Cannot create DPX file : unsupported subsampling factors");
      el->SetDefault(7,m_pComponent[1]); // CbCr, interleaved
      el++,m_usElements++;
      nextco             = 3; // components 0,1,2 handled.
    } else {
      // Bit depth of luma and chroma must be identical here.
      if (m_pComponent[1].m_ucBits  != m_pComponent[0].m_ucBits)
	PostError("Cannot create DPX file : luma and chroma components must have the same bitdepth");
      if (m_pComponent[1].m_bSigned != m_pComponent[0].m_bSigned)
	PostError("Cannot create DPX file : luma and chroma components must have the same signed-ness");
      //
      // Here we have to pack everything into one component. It is either 444 or 422.
      if (m_pComponent[1].m_ucSubX == 2) {
	// This is 422. If we have alpha, we may pack it here.
	if (m_usDepth > 3 && specs.Interleaved == ImgSpecs::Yes && 
	    m_pComponent[0].m_bSigned == m_pComponent[2].m_bSigned && 
	    m_pComponent[0].m_ucBits  == m_pComponent[2].m_ucBits) {
	  // YCbCr with Alpha
	  el->SetDefault(101,m_pComponent[0]);
	  el++,m_usElements++;
	  nextco             = 4;
	} else {
	  // Here: Separate alpha.
	  el->SetDefault(100,m_pComponent[0]);
	  el++,m_usElements++;
	  nextco             = 3;
	}
      } else {
	// This is 444. Possibly pack alpha into the same component.
	if (m_usDepth > 3 && specs.Interleaved == ImgSpecs::Yes && 
	    m_pComponent[0].m_bSigned == m_pComponent[2].m_bSigned && 
	    m_pComponent[0].m_ucBits  == m_pComponent[2].m_ucBits) {
	  // YCbCr in 444
	  el->SetDefault(103,m_pComponent[0]);
	  el++,m_usElements++;
	  nextco             = 4;
	} else {
	  // Here: Separate alpha.
	  el->SetDefault(102,m_pComponent[0]);
	  el++,m_usElements++;
	  nextco             = 3;
	}
      }
    }
  } else if (m_usDepth >= 3) {
    // RGB cases. Subsampling must be 1, no excuses.
    if (m_pComponent[0].m_ucSubX != 1 || m_pComponent[0].m_ucSubY != 1 ||
	m_pComponent[1].m_ucSubX != 1 || m_pComponent[1].m_ucSubY != 1 ||
	m_pComponent[2].m_ucSubX != 1 || m_pComponent[2].m_ucSubY != 1) {
      PostError("Cannot create DPX file : RGB images must not have subsampling");
    }
    // Separate or interleaved? Try interleaved first, if the depths allow this.
    if (specs.Interleaved == ImgSpecs::Yes &&
	m_pComponent[0].m_bSigned == m_pComponent[1].m_bSigned &&
	m_pComponent[0].m_bSigned == m_pComponent[2].m_bSigned &&
	m_pComponent[0].m_ucBits  == m_pComponent[1].m_ucBits  &&
	m_pComponent[0].m_ucBits  == m_pComponent[2].m_ucBits) {
      // Ok, RGB. Can we create ARGB as well?
      if (m_usDepth > 3 && 
	  m_pComponent[0].m_bSigned == m_pComponent[3].m_bSigned &&
	  m_pComponent[0].m_ucBits  == m_pComponent[3].m_ucBits) {
	// RGBA
	el->SetDefault(51,m_pComponent[0]);
	el++,m_usElements++;
	nextco             = 4;
      } else {
	// Separate alpha, but interleaved RGB.
	el->SetDefault(50,m_pComponent[0]);
	el++,m_usElements++;
	nextco             = 3;
      }
    } else {
      // Here separate RGB. Create three planes, alpha comes later.
      el->SetDefault(1,m_pComponent[0]);
      el++,m_usElements++;
      el->SetDefault(2,m_pComponent[1]);
      el++,m_usElements++;
      el->SetDefault(3,m_pComponent[2]);
      el++,m_usElements++;
      nextco             = 3;
    }
  } else if (m_usDepth == 1 || m_usDepth == 2) {
    // This is luma, or luma with alpha.
    // Always interleaved, no matter what.
    if (m_pComponent[0].m_ucSubX != 1 || m_pComponent[0].m_ucSubY != 1) {
      PostError("Cannot create DPX file : luma images must not have subsampling");
    }
    el->SetDefault(6,m_pComponent[0]);
    el++,m_usElements++;
    nextco             = 2;
    if (m_usDepth == 2) {
      el->SetDefault(4,m_pComponent[1]);
      el++,m_usElements++;
      nextco             = 3; 
      if (m_pComponent[0].m_ucSubX != 1 || m_pComponent[0].m_ucSubY != 1) {
	PostError("Cannot create DPX file : alpha component must not have subsampling");
      }
    }
  }
  // Insert alpha if that did not happen yet.
  if (m_usDepth > 3 && nextco == 3) {
    el->SetDefault(4,m_pComponent[0]);
    el++,m_usElements++;
    nextco             = 4; 
    if (m_pComponent[0].m_ucSubX != 1 || m_pComponent[0].m_ucSubY != 1) {
      PostError("Cannot create DPX file : alpha component must not have subsampling");
    }
  }
  //
  // Now handle the remaining components. First, subsampling must be all 1x1
  for(i = nextco;i < m_usDepth;i++) {
    if (m_pComponent[i].m_ucSubX != 1 || m_pComponent[i].m_ucSubY != 1) {
      PostError("Cannot create DPX file : non-yuv component must not have subsampling");
    }
  }
  //
  // Check whether we can interleave them.
  if (specs.Interleaved == ImgSpecs::Yes) {
    interleave = true;
    for(i = nextco;i < m_usDepth;i++) {
      if (m_pComponent[i].m_bSigned != m_pComponent[nextco].m_bSigned ||
	  m_pComponent[i].m_ucBits  != m_pComponent[nextco].m_ucBits) {
	interleave = false;
	break;
      }
    }
  }
  //
  // Build remaining components.
  while(nextco < m_usDepth) {
    UWORD count = m_usDepth;
    if (count > 8)
      count = 8;
    if (interleave == false)
      count = 1;
    if (m_usElements >= 8)
      PostError("Cannot create DPX file : too many elements necessary to cover all components");
    el->SetDefault((count == 1)?(0):(150 + count - 2),m_pComponent[nextco]);
    el++,m_usElements++;
    nextco += count;
  }
}
///

/// SimpleDPX::AssociateComponents
// Link the element data to the components in the system.
void SimpleDPX::AssociateComponents(ULONG offset,ULONG planes[9])
{
  struct ComponentLayout *cl = m_pComponent;
  UWORD i;
  
  for(i = 0;i < m_usElements;i++) {
    struct ImageElement *el = m_Elements + i;
    struct ScanElement  *sl = el->m_pScanPattern;
    while(sl) {
      struct ComponentLayout *cll = cl + sl->m_usTargetChannel; 
      UBYTE subx               = el->m_ucSubX;
      UBYTE suby               = el->m_ucSubY;
      UWORD k                  = sl->m_usTargetChannel;
      ULONG w                  = m_ulWidth;
      ULONG h                  = m_ulHeight;
      ULONG bytesperrow;
      //
      el->m_ulWidth            = w;
      el->m_ulHeight           = h;
      w                        = (w + subx - 1) / subx;
      h                        = (h + suby - 1) / suby;
      //
      // Compute the bytes per row as it appears in the file.
      if (el->m_ucLSBPaddingBits || el->m_ucMSBPaddingBits) {
	switch(el->m_ucBitDepth) {
	case 1:
	  // 32 units in of one bit.
	  bytesperrow = ((w + 31) >> 5) << 2;
	  break;
	case 10:
	  // Three units packed into one long word of size four.
	  bytesperrow = ((w + 2) / 3) << 2;
	  break;
	case 12:
	  // Each unit takes one UWORD
	  bytesperrow = w << 1;
	  break;
	default:
	  assert(!"padding set even though not required");
	  bytesperrow = w * el->m_ucDepth * el->m_ucBitDepth >> 3;
	  break;
	}
      } else {
	bytesperrow = w * el->m_ucDepth * el->m_ucBitDepth >> 3;
      }
      //
      // Compute the total bytesize, and add to the offset.
      if (sl == el->m_pScanPattern)
	planes[i] = offset;
      offset   += bytesperrow * h;
      //
      // Is this an alpha component or not?
      if (k == MAX_UWORD) { // here this is alpha.
	k              = el->m_ucDepth;
      }
      sl->m_pData      = cll->m_pPtr;
      sl->m_pComponent = cll;
      // Check whether this is the first time we touch this component.
      // If so, set the first flag.
      {
	struct ScanElement  *sl2 = el->m_pScanPattern;
	bool first = true;
	while(sl2 && sl2 != sl) {
	  if (sl2->m_pComponent == cll) {
	    first = false;
	    break;
	  }
	  sl2 = sl2->m_pNext;
	}
	sl->m_bFirst = first;
      }
      sl = sl->m_pNext;
    }
    // Adjust the components.
    cl += el->m_ucDepth + el->m_ucAlphaDepth;
  }
  //
  // And set the total size of the file.
  planes[8] = offset;
}
///

/// SimpleDPX::WriteElementHeader
// Write the element specific header to the file.
void SimpleDPX::WriteElementHeader(FILE *file,struct ImageElement *el,ULONG offset)
{
  PutLong(file,el->m_bSigned?1:0);
  // Reference low and high value remain undefined.
  PutLong(file,ULONG(~0UL));
  PutLong(file,ULONG(~0UL));
  PutLong(file,ULONG(~0UL));
  PutLong(file,ULONG(~0UL));
  // The descriptor.
  PutByte(file,el->m_ucDescriptor);
  // The transfer characteristics. We really do not know. Assume 601.
  PutByte(file,7);
  // The Colorimetric information. Also assume 601.
  PutByte(file,7);
  // BitDepth
  PutByte(file,el->m_ucBitDepth);
  // Packing.
  if (el->m_ucLSBPaddingBits) {
    PutWord(file,1);
  } else if (el->m_ucMSBPaddingBits) {
    PutWord(file,2);
  } else {
    PutWord(file,0);
  }
  // Encoding: None. No attempt to try RLE.
  PutWord(file,0);
  // The offset to the data.
  PutLong(file,offset);
  // No end of line nor end of image padding.
  PutLong(file,0);
  PutLong(file,0);
  // No element description either.
  PutString(file,"",32);
}
///

/// SimpleDPX::WriteHeader
// Write the complete DPX header with all specifications.
// pad data accordingly.
void SimpleDPX::WriteHeader(FILE *file,ULONG planes[9])
{
  UWORD i;
  union {
    FLOAT f;
    ULONG ul;
  } u;
  //
  // Put the magic word here.
  PutLong(file,MakeID('S','D','P','X'));
  PutLong(file,planes[0]);
  PutString(file,"V2.0",8); // version
  PutLong(file,planes[8]);
  PutLong(file,1L); // ditto-key
  // The generic header includes all element headers.
  PutLong(file,1664);
  // No industry-specific and user specific headers.
  PutLong(file,0);
  PutLong(file,0);
  PutString(file,m_pcFileName,100); // the file name. Just copy what we got from the user
  PutString(file,"",24); // creation time. Just leave blank.
  PutString(file,"difftest_ng",100); // creator
  PutString(file,"",200);    // leave the project name empty
  PutString(file,"",200);    // copyright statement
  PutLong(file,ULONG(~0UL)); // encryption: none.
  PutString(file,"",104);    // the reserved field
  //
  PutWord(file,0);            // image orientation is always upright
  PutWord(file,m_usElements); // number of elements
  PutLong(file,m_ulWidth);
  PutLong(file,m_ulHeight);
  //
  // Write now the image elements.
  for(i = 0;i < 8;i++) {
    if (i < m_usElements) {
      WriteElementHeader(file,m_Elements + i,planes[i]);
    } else {
      PutString(file,"",4*5 + 1*4 + 2*2 + 4*3 + 32);
    }
  }
  // Write the reserved data.
  PutString(file,"",52);
  // Image source information header.
  PutLong(file,0); // Xoffset
  PutLong(file,0); // Yoffset
  // Image center. Whatever that means.
  u.f = m_ulWidth * 0.5;
  PutLong(file,u.ul);
  u.f = m_ulHeight * 0.5;
  PutLong(file,u.ul);
  // Original size of the image. This is the original size.
  PutLong(file,m_ulWidth);
  PutLong(file,m_ulHeight);
  // Source image file name. As if I know...
  PutString(file,m_pcFileName,100);
  PutString(file,"",24); // source image date
  PutString(file,"",32); // input device name
  PutString(file,"",32); // input device serial number
  // Border validity: All borders are valid
  PutWord(file,0);
  PutWord(file,0);
  PutWord(file,0);
  PutWord(file,0);
  // Pixel aspect ratio. I do not know. Set this to 1:1
  PutLong(file,1);
  PutLong(file,1);
  // Additional source information.
  PutLong(file,m_ulWidth);  // X scanned size
  PutLong(file,m_ulHeight); // Y scanned size
  PutString(file,"",20);    // Reserved data
  // should be 1664 bytes so far.
  assert(ftell(file) == 1664);
  // Write the rest with zeros to fill the area up to the start of the
  // image data.
  assert(planes[0] > 1664);
  PutString(file,"",planes[0] - 1664);
}
///

/// SimpleDPX::WriteData
// Write out the data to the file, possibly align it.
void SimpleDPX::WriteData(FILE *out,struct ImageElement *el,UQUAD q)
{ 
  switch(el->m_ucBitDepth) {
  case 32:
    PutLong(out,q);
    return;
  case 64:
    if (m_bLittleEndian) {
      PutLong(out,q);
      PutLong(out,q >> 32);
    } else {
      PutLong(out,q >> 32);
      PutLong(out,q);
    }
    return;
  default:
    if (el->m_ucBitDepth < 32) {
      UBYTE bits  = el->m_ucBitDepth;
      
      do {
	UBYTE avail = 32 - m_cBit;
	// Compute the number of available bits.
	if (avail > bits)
	  avail = bits;
	// 
	// Fill the bits into the bit buffer.
	m_ulBitBuffer |= (q & ((1L << avail) - 1)) << m_cBit;
	m_cBit        += avail;
	bits          -= avail;
	q            >>= avail;
	if (el->m_ucPackElements == 1)
	  m_cBit += el->m_ucMSBPaddingBits + el->m_ucLSBPaddingBits;
	//
	// Empty the buffer if there is no room left.
	if (m_cBit >= 32 - el->m_ucMSBPaddingBits) {
	  // Fill up the bit-buffer. We can have packed sizes of 16 or 32.
	  PutLong(out,m_ulBitBuffer);
	  m_cBit        = el->m_ucLSBPaddingBits;
	  m_ulBitBuffer = 0;
	}
      } while(bits);
    } else {
      assert(!"invalid bit size in output buffer");
    }
  }
}
///

/// SimpleDPX::Flush
// Flush out the rest of the buffer if there is one.
void SimpleDPX::Flush(FILE *out,struct ImageElement *el)
{
  switch (el->m_ucBitDepth) {
  case 32:
  case 64:
    // Nothing to do here, is all aligned.
    break;
  default:
    if (el->m_ucBitDepth < 32) {
      if (m_cBit > el->m_ucLSBPaddingBits) {
	PutLong(out,m_ulBitBuffer);
      }
    } else {
      assert(!"invalid bit size");
    }
  }
  m_cBit        = el->m_ucLSBPaddingBits;
  m_ulBitBuffer = 0;
}
///

/// SimpleDPX::WriteElement
// Write out the target data to the components.
void SimpleDPX::WriteElement(FILE *out,struct ImageElement *el)
{
  bool linedone  = false;
  bool framedone = false;

  // Start at a new byte boundary.
  m_cBit        = el->m_ucLSBPaddingBits;
  m_ulBitBuffer = 0;
  do {
    framedone = true;
    do {
      struct ScanElement *sl = el->m_pScanPattern;
      linedone = true;
      do {
	struct ComponentLayout *cl = sl->m_pComponent;
	ULONG  x = el->m_ulX[sl->m_usTargetChannel & 0x0f]; // map alpha to 15
	ULONG  y = el->m_ulY[sl->m_usTargetChannel & 0x0f];
	ULONG  w = cl->m_ulWidth;
	ULONG  h = cl->m_ulHeight;
	UQUAD  q;
	// Check whether the pixel is in range.
	if (x < w && y < h) {
	  APTR data;
	  // And write the data out.
	  data         = ((UBYTE *)(sl->m_pData)) + (x * cl->m_ulBytesPerPixel) + (y * cl->m_ulBytesPerRow);
	  if (el->m_ucBitDepth <= 8) {
	    q = *(UBYTE *)(data);
	  } else if (el->m_ucBitDepth <= 16) {
	    q = *(UWORD *)(data);
	  } else if (el->m_ucBitDepth <= 32) {
	    q = *(ULONG *)(data);
	  } else {
	    q = *(UQUAD *)(data);
	  }
	  //
	  // Write out the data, keeping it aligned.
	  WriteData(out,el,q);
	  //
	  // Advance to the next x.
	  if (x + 1 < w) 
	    linedone = false;
	  el->m_ulX[sl->m_usTargetChannel & 0x0f]++;
	}
      } while((sl = sl->m_pNext));
    } while(!linedone);
    //
    // Increment the Y position. Packing is broken on scan line boundaries, so flush out now.
    Flush(out,el);
    {
      struct ScanElement *sl = el->m_pScanPattern;
      do {
	struct ComponentLayout *cl = sl->m_pComponent;
	ULONG  y = el->m_ulY[sl->m_usTargetChannel & 0x0f];
	ULONG  h = cl->m_ulHeight;
	if (y + 1 < h)
	  framedone = false;
	if (sl->m_bFirst)
	  el->m_ulY[sl->m_usTargetChannel & 0x0f]++;
	el->m_ulX[sl->m_usTargetChannel & 0x0f] = 0;
      } while((sl = sl->m_pNext));
    }
  } while(!framedone);
}
///

/// SimpleDPX::SaveImage
// Save an image to a level 1 file descriptor, given its
// width, height and depth.
void SimpleDPX::SaveImage(const char *name,const struct ImgSpecs &specs)
{
  UWORD i;
  ULONG planes[9];
  //
  // Set the offset to the image data to 4K. We could use any other offset
  // as well, though.
  // Default is big-endian.
  if (specs.LittleEndian == ImgSpecs::Yes) {
    m_bLittleEndian = true;
  } else {
    m_bLittleEndian = false;
  }
  //
  // Create the layout of the components
  CreateElementLayout(specs);
  //
  for(i = 0;i < m_usElements;i++) {
    BuildScanPattern(m_Elements + i);
  }
  //
  // Define an offset for the first element.
  planes[0] = 4096;
  // Link them to the components.
  AssociateComponents(planes[0],planes);
  //
  File out(name,"wb");
  //
  m_pcFileName = name;
  WriteHeader(out,planes);
  for(i = 0;i < m_usElements;i++) {
    WriteElement(out,m_Elements + i);
  }
}
///

