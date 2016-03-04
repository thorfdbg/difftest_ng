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
 * This class saves and loads images in any header-less format.
 *
 * $Id: simpleraw.cpp,v 1.15 2016/03/04 17:09:43 thor Exp $
 */

/// Includes
#include "interface/types.hpp"
#include "std/string.hpp"
#include "std/stdlib.hpp"
#include "tools/halffloat.hpp"
#include "tools/file.hpp"
#include "img/imglayout.hpp"
#include "img/imgspecs.hpp"
#include "img/simpleraw.hpp"
///

/// SimpleRaw::SimpleRaw
// default constructor
SimpleRaw::SimpleRaw(void)
  : m_pcFilename(NULL), m_pRawList(NULL), 
    m_ulNominalWidth(0), m_ulNominalHeight(0), m_usNominalDepth(0), 
    m_usFields(0), m_bSeparate(false), m_ucBit(0), m_ulBitBuffer(0)
{
}
///

/// SimpleRaw::SimpleRaw
// Copy the image from another source for later saving.
SimpleRaw::SimpleRaw(const class ImageLayout &org)
  : ImageLayout(org), m_pcFilename(NULL), m_pRawList(NULL), 
    m_ulNominalWidth(0), m_ulNominalHeight(0), m_usNominalDepth(0), 
    m_usFields(0), m_bSeparate(false), m_ucBit(0), m_ulBitBuffer(0)
{
}
///

/// SimpleRaw::~SimpleRaw
// destructor
SimpleRaw::~SimpleRaw(void)
{
  struct RawLayout *rl;

  delete[] m_pcFilename;

  while((rl = m_pRawList)) {
    UBYTE *mem = (UBYTE *)rl->m_pPtr;
    m_pRawList = rl->m_pNext;
    delete[] mem;
    delete rl;
  }
}
///

/// SimpleRaw::ComponentLayoutFromFileName
// Create the component layouts from the file name (Rather complicated, though).
void SimpleRaw::ComponentLayoutFromFileName(const char *filename)
{
  const char *sep;
  char *end;
  char planetype = 0;
  struct RawLayout *rl = NULL,*last = NULL,*packstart = NULL;
  int len;
  UBYTE packedbits = 0;
  bool havepacking = false;
  bool lefty       = false;
  
  delete[] m_pcFilename;
  m_pcFilename = NULL;

  sep = strchr(filename,'@'); /* find the separator */
  if (sep == NULL) {
    PostError("format specifier '@' following the file name is missing, invalid raw format");
    return;
  } else if (sep <= filename) {
    PostError("no filename specified to load from, format specifier must not be the first character");
    return;
  }

  len               = sep - filename;
  m_pcFilename      = new char[len + 1];
  memcpy(m_pcFilename,filename,len);
  m_pcFilename[len] = '\0'; // terminate here.
  sep++;
  
  if (*sep != ':') {
    long data;
    // Not immediately the field specifications, read dimensions.
    data = strtol(sep,&end,0);
    if (end == sep) {
      PostError("expected the width as first data in the format specification following '@'");
      return;
    }
    if (data <= 0) {
      PostError("the width must be strictly positive");
      return;
    }
    if (*end != 'x') {
      PostError("the width must be separated from the height by 'x'");
      return;
    }
    m_ulNominalWidth = ULONG(data);
    //
    //
    sep  = end + 1; // skip the 'x'.
    data = strtol(sep,&end,0);
    if (end == sep) {
      PostError("expected the height as second data in the format specification following 'x'");
      return;
    }
    if (data <= 0) {
      PostError("the height must be strictly positive");
      return;
    }
    if (*end != 'x') {
      PostError("the height must be separated from the number of components by 'x'");
      return;
    }
    m_ulNominalHeight = ULONG(data);
    //
    //
    sep  = end + 1; // skip the 'x'.
    data = strtol(sep,&end,0);
    if (end == sep) {
      PostError("expected the depth as third data in the format specification following 'x'");
      return;
    }
    if (data <= 0 || data > MAX_WORD) {
      PostError("the depth must be strictly positive and smaller than 32768");
      return;
    }
    if (*end != ':') {
      PostError("the depth must be separated from the field specifications by ':'");
      return;
    }
    //
    m_usNominalDepth = UWORD(data);
    //
    // Points now to the colon.
    sep = end;
  } else {
    if (m_ulNominalWidth == 0 || m_ulNominalHeight == 0 || m_usNominalDepth == 0) {
      PostError("image dimensions must be provided for loading in raw format");
      return;
    }
  }
  //
  // Dimensions (if available) have now been parsed off. Now go for the field list.
  sep++;
  //
  while(*sep) {
    long data;
    //
    // Check on the left-right bit-sequence.
    if (havepacking == false) {
      if (*sep == '-') {
	lefty = true;
	sep++;
      } else if (*sep == '+') {
	lefty = false;
	sep++;
      }
      havepacking = true;
    }
    //
    if (*sep != '[' && *sep != '{') {
      PostError("the field must start with a '[' or '{' bracket to indicate separate or interleaved planes");
      return;
    }
    //
    if (planetype == 0) {
      planetype = *sep;
      if (planetype == '[')
	m_bSeparate = true;
    } else if (planetype != *sep) {
      PostError("the plane type must be consistent for all fields, cannot mix '{' and '[' fields");
      return;
    }
    sep++;
    //
    // Create a new raw layout now.
    rl = new struct RawLayout();
    if (last == NULL) {
      assert(m_pRawList == NULL);
      m_pRawList    = rl;
    } else {
      assert(m_pRawList != NULL);
      last->m_pNext = rl;
    }
    m_usFields++;
    last = rl;
    //
    // Insert the layout as far as we have it.
    rl->m_ulWidth  = m_ulNominalWidth;
    rl->m_ulHeight = m_ulNominalHeight;
    rl->m_bLefty   = lefty;
    //
    // Parse off the number of bits in the field.
    data = strtol(sep,&end,0);
    if (end == sep) {
      PostError("expected the field size in bits as first field element following '{' or '['");
      return;
    }
    if (data == 0) {
      PostError("the field size cannot be zero bits");
      return;
    }
    if (data > 64 || data < -64) {
      PostError("the field size must be smaller or equal to 64 bits");
      return;
    }
    if (data > 0) {
      rl->m_ucBits  = UBYTE(data);
    } else {
      rl->m_ucBits  = UBYTE(-data);
      rl->m_bSigned = true;
    }
    sep = end;
    //
    // Is this possibly a floating point plane?
    if (*sep == 'f' || *sep == 'F') {
      rl->m_bFloat  = true;
      sep++;
    }
    //
    // Is there an endian indicator?
    if (*sep == '-') {
      rl->m_bLittleEndian = true;
      sep++;
    } else if (*sep == '+') {
      rl->m_bLittleEndian = false;
      sep++;
    }
    //
    // Is there a target channel?
    if (*sep == '=') {
      sep++;
      data = strtol(sep,&end,0);
      if (end == sep) {
	PostError("expected the target channel behind the '=' sign");
	return;
      }
      if (data < 0) {
	PostError("the field target channel must be non-negative");
	return;
      }
      if (data > MAX_WORD) {
	PostError("the target channel is out of range, it must be smaller than 32768");
	return;
      }
      if (data >= m_usNominalDepth) {
	PostError("the target channel does not exist, i.e. is outside the number of specified components");
	return;
      }
      rl->m_usTargetChannel = UWORD(data);
      rl->m_bIsPadding      = false;
      sep = end;
    } else {
      // Must be a padding channel.
      rl->m_bIsPadding      = true;
    }
    // At this point, the field description is over. We should either find a '}' or a ']'
    if (planetype == '{' && *sep != '}') {
      PostError("expected a '}' at the end of the field description");
      return;
    } else if (planetype == '[' && *sep != ']') {
      PostError("expected a ']' at the end of the field description");
      return;
    } 
    sep++;
    // There may be an optional subsampling specification. This only goes for separate planes.
    if (*sep == '/') {
      long data;
      if (planetype != '[') {
	PostError("subsampling specifications are only available for separate planes, use '[' for the field");
	return;
      }
      sep++;
      // Read subX.
      data = strtol(sep,&end,0);  
      if (end == sep) {
	PostError("expected the subsampling factors behind '/'");
	return;
      }
      if (data <= 0) {
	PostError("the subsampling factor must be strictly positive");
	return;
      } else if (data > MAX_UBYTE) {
	PostError("the subsampling factor must be smaller or equal than 255");
	return;
      }
      rl->m_ucSubX = UBYTE(data);
      sep = end;
      //
      if (*sep != 'x') {
	PostError("subsampling factors must be separated by 'x'");
	return;
      }
      sep++;
      // Read subY
      data = strtol(sep,&end,0);      
      if (end == sep) {
	PostError("expected a second subsampling factor behind 'x'");
	return;
      }
      if (data <= 0) {
	PostError("the subsampling factor must be strictly positive");
	return;
      } else if (data > MAX_UBYTE) {
	PostError("the subsampling factor must be smaller or equal than 255");
	return;
      }
      rl->m_ucSubY = UBYTE(data);
      sep = end;
    } else {
      rl->m_ucSubX = 1;
      rl->m_ucSubY = 1;
    }
    //
    // End of field. This can be either the end of the string, or a comma separating the fields.
    switch(*sep) {
    case ',':
      // start or possibly continuation of a packed sequence.
      if (packstart == NULL) {
	assert(packedbits == 0);
	packstart           = rl;
	rl->m_bStartPacking = true;
      } 
      if (sep[1]) {
	packedbits += rl->m_ucBits;
	sep++;
	break;
      }
    case ':':
      lefty       = false;
      havepacking = false;
      sep++;
    case '\0':
      // End of a bit-packing sequence, possibly. Count bits.
      // But this one is still included.
      if (packstart) {
	packedbits += rl->m_ucBits;
      }
      if (packedbits > 32) {
	PostError("cannot pack more than 32 bits into a field");
	return;
      } else if (packedbits != 0 && packedbits != 8 && packedbits != 16 && packedbits != 32) {
	PostError("the number of bits packed together in interleaved planes must be either 8,16 or 32");
	return;
      }
      
      while(packstart) {
	assert(packedbits > 0);
	if (packstart->m_bLittleEndian != rl->m_bLittleEndian) {
	  PostError("the endianness of fields bit-packed together must be consistent");
	  return;
	}
	// Check whether subsampling is consistent.
	if (m_bSeparate) {
	  if (packstart->m_ucSubX != rl->m_ucSubX || packstart->m_ucSubY != rl->m_ucSubY)
	    PostError("the subsampling factors must be consistent within a group of packed pixels");
	}
	packstart->m_ucBitsPacked = packedbits;
	packstart = packstart->m_pNext;
      }
      packedbits = 0;
      break;
    default:
      PostError("the end of the field must be terminated by either ':' or ','");
      return;
    }
  }
  //
  // Now check whether there are any fields at all.
  if (m_usFields == 0) {
    PostError("at least one field must be specified");
    return;
  }
  //
  // In case we are interleaved, try to figure the subsampling factors out.
  if (m_bSeparate == false) {
    UWORD counter[MAX_UWORD];
    UWORD max = 0;
    memset(counter,0,sizeof(counter));
    // Now count the number of samples in each channel and adjust
    // subsampling factors accordingly.
    for(rl = m_pRawList;rl;rl = rl->m_pNext) {
      if (rl->m_bIsPadding == false) {
	counter[rl->m_usTargetChannel]++;
	if (counter[rl->m_usTargetChannel] > max)
	  max = counter[rl->m_usTargetChannel];
      }
    }
    if (max == 0)
      PostError("at least one field must be specified");
    // Check whether the subsampling fields are all divisible by the maximum
    // count. If not, generate an error.
    for(rl = m_pRawList;rl;rl = rl->m_pNext) {
      if (rl->m_bIsPadding == false) {
	UWORD total = counter[rl->m_usTargetChannel];
	if (max % total) {
	  PostError("the number of samples for cannel %d is not divisible by the number of total samples",
		    rl->m_usTargetChannel);
	  return;
	}
	rl->m_ucSubX = max / total;
	rl->m_ucSubY = 1;
      }
    }
  }
}
///

/// SimpleRaw::ReadData
// Read a single pixel from the specified file.
UQUAD SimpleRaw::ReadData(FILE *in,UBYTE bitsize,UBYTE packsize,bool littleendian,bool issigned,bool lefty)
{
  if (bitsize == 8) {
    int d1 = getc(in);
    if (d1 < 0) {
      PostError("unexpected EOF while reading %s",m_pcFilename);
    }
    return d1;
  } else if (bitsize == 16) {
    int d1,d2;
    if (littleendian) {
      d2 = getc(in);
      d1 = getc(in);
    } else {
      d1 = getc(in);
      d2 = getc(in);
    }
    if (d1 < 0 || d2 < 0) {
      PostError("unexpected EOF while reading %s",m_pcFilename);
    }
    return (UQUAD(d1) << 8) | (UQUAD(d2));
  } else if (bitsize == 32) {
    int d1,d2,d3,d4;
    if (littleendian) {
      d4 = getc(in);
      d3 = getc(in);
      d2 = getc(in);
      d1 = getc(in);
    } else {
      d1 = getc(in);
      d2 = getc(in);
      d3 = getc(in);
      d4 = getc(in);
    }
    //
    if (d1 < 0 || d2 < 0 || d3 < 0 || d4 < 0) {
      PostError("unexpected EOF while reading %s",m_pcFilename);
    }
    return ((UQUAD(d1) << 24) |
	    (UQUAD(d2) << 16) |
	    (UQUAD(d3) <<  8) |
	    (UQUAD(d4) <<  0));
  } else if (bitsize == 64) {
    int d1,d2,d3,d4,d5,d6,d7,d8;
    if (littleendian) {
      d8 = getc(in);
      d7 = getc(in);
      d6 = getc(in);
      d5 = getc(in);
      d4 = getc(in);
      d3 = getc(in);
      d2 = getc(in);
      d1 = getc(in);
    } else {
      d1 = getc(in);
      d2 = getc(in);
      d3 = getc(in);
      d4 = getc(in);
      d5 = getc(in);
      d6 = getc(in);
      d7 = getc(in);
      d8 = getc(in);
    }
    //
    if (d1 < 0 || d2 < 0 || d3 < 0 || d4 < 0 || d5 < 0 || d6 < 0 || d7 < 0 || d8 < 0) {
      PostError("unexpected EOF while reading %s",m_pcFilename);
    }
    return ((UQUAD(d1) << 56) |
	    (UQUAD(d2) << 48) |
	    (UQUAD(d3) << 40) |
	    (UQUAD(d4) << 32) |
	    (UQUAD(d5) << 24) |
	    (UQUAD(d6) << 16) |
	    (UQUAD(d7) <<  8) |
	    (UQUAD(d8) <<  0));
  } else if (bitsize < 32) {
    ULONG res   = 0;
    UBYTE sign  = bitsize;
    UBYTE shift = 0;
    
    do {
      if (m_ucBit == 0) {
	if (packsize == 0)
	  PostError("Pixels must be packed into units of 8,16 or 32 bits, add dummy channels to discard unused bits");
	m_ulBitBuffer = ReadData(in,packsize,packsize,littleendian,issigned,false);
	m_ucBit       = packsize;
      }
      UBYTE avail = m_ucBit;
      if (avail > bitsize)
	avail = bitsize; // do not remove more bits than requested.

      // m_ucBit is the number of valid bits.
      if (lefty) {
	res     |= ((m_ulBitBuffer >> (packsize - m_ucBit)) & ((1UL << avail) - 1)) << shift;
	shift   += avail;
      } else {
	// remove avail bits from the byte
	res      = (res << avail) | ((m_ulBitBuffer >> (m_ucBit - avail)) & ((1UL << avail) - 1));
      }
      bitsize -= avail;
      m_ucBit -= avail;
    } while(bitsize);
    
    if (issigned) {
      if (res & (1 << (sign - 1))) { // result is negative
	res |= ULONG(-1) << sign;
      }
    }
    return res;
  } else {
    assert(0);
    return 0;
  }
}
///

/// SimpleRaw::LoadImage
// Load an image from a level 1 file descriptor, keep it within
// the internals of this class. The accessor methods below
// should be used to find out more about this image.
void SimpleRaw::LoadImage(const char *nameandspecs,struct ImgSpecs &specs)
{
  struct RawLayout *rl;
  //
  m_ulNominalWidth  = 0;
  m_ulNominalHeight = 0;
  m_usNominalDepth  = 0;
  //
  // First, parse off the filename.
  ComponentLayoutFromFileName(nameandspecs);
  //
  if (m_ulNominalWidth == 0 || m_ulNominalHeight == 0 || m_usNominalDepth == 0) {
    PostError("image dimensions must be specified when loading a raw image");
    return;
  }
  File in       = File(m_pcFilename,"rb");
  m_ucBit       = 0;
  m_ulBitBuffer = 0;
  //
  // Setup the component of the master layout.
  CreateComponents(m_ulNominalWidth,m_ulNominalHeight,m_usNominalDepth);
  //
  for(rl = m_pRawList;rl;rl = rl->m_pNext) {
    if (!rl->m_bIsPadding) {
      UWORD i = rl->m_usTargetChannel; // This has been checked to be valid.
      struct ComponentLayout *cl = m_pComponent + i;
      size_t bpp = 8;
      //
      // Several components can mapped multiple times now...
      cl->m_ucBits   = rl->m_ucBits;
      cl->m_bSigned  = rl->m_bSigned;
      cl->m_bFloat   = rl->m_bFloat;
      cl->m_ucSubX   = rl->m_ucSubX;
      cl->m_ucSubY   = rl->m_ucSubY;
      cl->m_ulWidth  = (rl->m_ulWidth  + rl->m_ucSubX - 1) / (rl->m_ucSubX);
      cl->m_ulHeight = (rl->m_ulHeight + rl->m_ucSubY - 1) / (rl->m_ucSubY);
      //
      // Now compute the memory demand.
      if (rl->m_ucBits <= 8) {
	bpp = sizeof(UBYTE);
      } else if (rl->m_ucBits <= 16) {
	if (rl->m_bFloat) {
	  // Allocate half-float as float for simplicity.
	  bpp = sizeof(FLOAT);
	} else {
	  bpp = sizeof(UWORD);
	}
      } else if (rl->m_ucBits <= 32) {
	bpp = sizeof(ULONG);
      } else if (rl->m_ucBits <= 64) {
	bpp = sizeof(UQUAD);
      } else {
	assert(0);
      }
      //
      cl->m_ulBytesPerPixel = ULONG(bpp);
      rl->m_ulBytesPerPixel = ULONG(bpp);
      if (UQUAD(bpp) * cl->m_ulWidth > MAX_ULONG || UQUAD(bpp) * cl->m_ulWidth * cl->m_ulHeight > MAX_ULONG) {
	PostError("image is too large, cannot load");
	return;
      }
      cl->m_ulBytesPerRow   = ULONG(bpp * cl->m_ulWidth);
      rl->m_ulBytesPerRow   = ULONG(bpp * cl->m_ulWidth);
      if (cl->m_pPtr == NULL) {
	cl->m_pPtr          = new UBYTE[bpp * cl->m_ulWidth * cl->m_ulHeight];
	rl->m_pPtr          = cl->m_pPtr;
      }
      //
    }
  }
  //
  // Layout?
  specs.Interleaved = (m_bSeparate)?(ImgSpecs::No):(ImgSpecs::Yes);
  specs.Palettized  = ImgSpecs::No;
  //
  // Now read the stuff.
  if (m_bSeparate) {
    ULONG x,y;
    for(rl = m_pRawList;rl;rl = rl->m_pNext) {
      UBYTE *rptr  = (UBYTE *)rl->m_pPtr;
      ULONG width  = (rl->m_ulWidth  + rl->m_ucSubX - 1) / (rl->m_ucSubX);
      ULONG height = (rl->m_ulHeight + rl->m_ucSubY - 1) / (rl->m_ucSubY);
      //
      // Are these multiple components packed together?
      if (rl->m_bStartPacking) {
	for(y = 0;y < height;y++) {
	  for(x = 0;x < width;x++) {
	    struct RawLayout *ro = rl;
	    do {
	      UQUAD data = ReadData(in,ro->m_ucBits,ro->m_ucBitsPacked,ro->m_bLittleEndian,
				    ro->m_bSigned,ro->m_bLefty);
	      if (!ro->m_bIsPadding) {
		struct ComponentLayout *cl = m_pComponent + ro->m_usTargetChannel;
		UBYTE *ptr = ((UBYTE *)(cl->m_pPtr)) + (y * cl->m_ulBytesPerRow) + (x * cl->m_ulBytesPerPixel);
		if (ro->m_ucBits <= 8) {
		  *(UBYTE *)ptr = UBYTE(data);
		} else if (ro->m_ucBits <= 16) {
		  if (ro->m_bFloat) {
		    FLOAT dt = H2F(data);
		    // Half float is stored as float.
		    *(FLOAT *)ptr = ULONG(dt);
		  } else {
		    *(UWORD *)ptr = UWORD(data);
		  }
		} else if (ro->m_ucBits <= 32) {
		  *(ULONG *)ptr = ULONG(data);
		} else if (ro->m_ucBits <= 64) {
		  *(UQUAD *)ptr = UQUAD(data);
		} else {
		  assert(0);
		}
	      }
	    } while((ro = ro->m_pNext) && ro->m_bStartPacking == false && ro->m_ucBitsPacked);
	  }
	  BitAlignIn();
	}
	//
	// Advance over the packed sequence.
	while(rl->m_pNext && rl->m_pNext->m_bStartPacking == false && rl->m_pNext->m_ucBitsPacked)
	  rl = rl->m_pNext;
	//
      } else {
	for(y = 0;y < height;y++) {
	  UBYTE *ptr = rptr;
	  for(x = 0;x < width;x++) {
	    UQUAD data = ReadData(in,rl->m_ucBits,8,rl->m_bLittleEndian,rl->m_bSigned,rl->m_bLefty);
	    if (!rl->m_bIsPadding) {
	      if (rl->m_ucBits <= 8) {
		*(UBYTE *)ptr = UBYTE(data);
	      } else if (rl->m_ucBits <= 16) {
		if (rl->m_bFloat) {
		  FLOAT dt = H2F(data);
		  // Half float is stored as float.
		  *(FLOAT *)ptr = ULONG(dt);
		} else {
		  *(UWORD *)ptr = UWORD(data);
		}
	      } else if (rl->m_ucBits <= 32) {
		*(ULONG *)ptr = ULONG(data);
	      } else if (rl->m_ucBits <= 64) {
		*(UQUAD *)ptr = UQUAD(data);
	      } else {
		assert(0);
	      }
	    }
	    ptr += rl->m_ulBytesPerPixel;
	  }
	  BitAlignIn();
	  rptr += rl->m_ulBytesPerRow;
	}
      }
    }
  } else {
    ULONG y;
    for(y = 0;y < m_ulHeight;y++) {
      ULONG x[MAX_UWORD];
      bool rowdone;
      memset(x,0,sizeof(x));
      do {
	for(rl = m_pRawList,rowdone = true;rl;rl = rl->m_pNext) {
	  UQUAD data = ReadData(in,rl->m_ucBits,rl->m_ucBitsPacked,
				rl->m_bLittleEndian,rl->m_bSigned,rl->m_bLefty);

	  if (!rl->m_bIsPadding) {
	    UWORD i = rl->m_usTargetChannel;
	    struct ComponentLayout *cl = m_pComponent + i;
	    if (x[i] < cl->m_ulWidth) {
	      UBYTE *ptr = (UBYTE *)cl->m_pPtr + (y * rl->m_ulBytesPerRow) + (x[i] * rl->m_ulBytesPerPixel);
	      //
	      if (rl->m_ucBits <= 8) {
		*(UBYTE *)ptr = UBYTE(data);
	      } else if (rl->m_ucBits <= 16) {
		if (rl->m_bFloat) {
		  FLOAT dt = H2F(data);
		  // Half float is stored as float.
		  *(FLOAT *)ptr = ULONG(dt);
		} else {
		  *(UWORD *)ptr = UWORD(data);
		}
	      } else if (rl->m_ucBits <= 32) {
		*(ULONG *)ptr = ULONG(data);
	      } else if (rl->m_ucBits <= 64) {
		*(UQUAD *)ptr = UQUAD(data);
	      } else {
		assert(0);
	      }
	      // Advance to the next display position for this channel.
	      x[i]++;
	    }
	  }
	  if (rl->m_ucBitsPacked && ((rl->m_pNext == NULL) || rl->m_pNext->m_bStartPacking ||
				     (rl->m_pNext->m_ucBitsPacked == 0))) {
	    BitAlignIn();
	  }
	}
	for(UWORD i = 0;i < m_usDepth;i++) {
	  if (x[i] < m_pComponent[i].m_ulWidth)
	    rowdone = false;
	}
      } while(rowdone == false);
    }
  }
}
///

/// SimpleRaw::BitAlignOut
// On writing, flush to the next byte boundary.
void SimpleRaw::BitAlignOut(FILE *out,UBYTE packsize,bool littleendian,bool lefty)
{
  if (m_ucBit < packsize) {
    WriteData(out,m_ulBitBuffer,packsize,packsize,littleendian,lefty);
    m_ucBit       = packsize;
    m_ulBitBuffer = 0;
  }
}
///

/// SimpleRaw::WriteData
// Write a single data item to the file.
void SimpleRaw::WriteData(FILE *out,UQUAD data,UBYTE bitsize,UBYTE packsize,bool littleendian,bool lefty)
{
  if (bitsize == 8) {
    putc(data,out);
  } else if (bitsize == 16) {
    if (littleendian) {
      putc(UBYTE(data >> 0),out);
      putc(UBYTE(data >> 8),out);
    } else {
      putc(UBYTE(data >> 8),out);
      putc(UBYTE(data >> 0),out);
    }
  } else if (bitsize == 32) {
    if (littleendian) {
      putc(UBYTE(data >>  0),out);
      putc(UBYTE(data >>  8),out);
      putc(UBYTE(data >> 16),out);
      putc(UBYTE(data >> 24),out);
    } else {
      putc(UBYTE(data >> 24),out);
      putc(UBYTE(data >> 16),out);
      putc(UBYTE(data >>  8),out);
      putc(UBYTE(data >>  0),out);
    }
  } else if (bitsize == 64) {
    if (littleendian) {
      putc(UBYTE(data >>  0),out);
      putc(UBYTE(data >>  8),out);
      putc(UBYTE(data >> 16),out);
      putc(UBYTE(data >> 24),out);
      putc(UBYTE(data >> 32),out);
      putc(UBYTE(data >> 40),out);
      putc(UBYTE(data >> 48),out);
      putc(UBYTE(data >> 56),out);
    } else {
      putc(UBYTE(data >> 56),out);
      putc(UBYTE(data >> 48),out);
      putc(UBYTE(data >> 40),out);
      putc(UBYTE(data >> 32),out);
      putc(UBYTE(data >> 24),out);
      putc(UBYTE(data >> 16),out);
      putc(UBYTE(data >>  8),out);
      putc(UBYTE(data >>  0),out);
    }
  } else if (bitsize < 32) {
    data &= (1UL << bitsize) - 1;
    
    if (lefty) {
      // Here we are filling from the right instead the left.
      // m_ucBit is the number of free bits that can be filled.
      while (bitsize >= m_ucBit) {
	// We have to write more bits than there is room in the bit buffer.
	// Hence, the complete buffer can be filled.
	m_ulBitBuffer |= (data & ((1UL << m_ucBit) - 1)) << (packsize - m_ucBit);
	WriteData(out,m_ulBitBuffer,packsize,packsize,littleendian,false);
	m_ulBitBuffer  = 0;
	// Remove lower bits.
	data         >>= m_ucBit;
	bitsize       -= m_ucBit;
	m_ucBit        = packsize;
      }
      // We have to write less than there is room in the bit buffer. Hence,
      // we must upshift the remaining bits to fit into the bit buffer.
      m_ulBitBuffer |= data << (packsize - m_ucBit);
      m_ucBit       -= bitsize;
    } else {
      while (bitsize >= m_ucBit) {
	// We have to write more bits than there is room in the bit buffer.
	// Hence, the complete buffer can be filled.
	m_ulBitBuffer |= data >> (bitsize - m_ucBit);
	WriteData(out,m_ulBitBuffer,packsize,packsize,littleendian,false);
	m_ulBitBuffer  = 0;
	bitsize       -= m_ucBit;
	m_ucBit        = packsize;
	// Remove upper bits.
	data          &= (1UL << bitsize) - 1;
      }
      // We have to write less than there is room in the bit buffer. Hence,
      // we must upshift the remaining bits to fit into the bit buffer.
      m_ulBitBuffer |= data << (m_ucBit - bitsize);
      m_ucBit       -= bitsize;
    }
  } else {
    assert(0);
  }
}
///

/// SimpleRaw::SaveImage
// Save an image to a level 1 file descriptor, given its
// width, height and depth.
void SimpleRaw::SaveImage(const char *nameandspecs,const struct ImgSpecs &)
{ 
  struct RawLayout *rl;
  //
  // Provide reasonable defaults
  m_ulNominalWidth  = m_ulWidth;
  m_ulNominalHeight = m_ulHeight;
  m_usNominalDepth  = m_usDepth;
  //
  // First, parse off the filename.
  ComponentLayoutFromFileName(nameandspecs);
  //
  if (m_usNominalDepth > m_usDepth) {
    PostError("not enough components available, requesting to write %d components while only %d are in the image",
	      m_usNominalDepth,m_usDepth);
    return;
  }
  // Try to associate the raw format data from the component data which is here already
  // available.
  for(rl = m_pRawList;rl;rl = rl->m_pNext) {
    if (!rl->m_bIsPadding) {
      UWORD i = rl->m_usTargetChannel;
      struct ComponentLayout *cl = m_pComponent + i;
      assert(rl->m_ulWidth > 0);
      if (rl->m_ulWidth > cl->m_ulWidth * rl->m_ucSubX) {
	PostError("trying to write field %i with width %d while only %d data is available",
		  i,rl->m_ulWidth,cl->m_ulWidth);
	return;
      }
      assert(rl->m_ulHeight > 0);
      if (rl->m_ulHeight > cl->m_ulHeight * rl->m_ucSubY) {
	PostError("trying to write field %i with height %d while only %d data is available",
		  i,rl->m_ulHeight,cl->m_ulHeight);
	return;
      }
 
    }
    if (!m_bSeparate) {
      if (rl->m_ulWidth != m_ulWidth || rl->m_ulHeight != m_ulHeight) {
	PostError("for interleaved field representations, the sizes of all components must be identical");
	return;
      }
    }
  }
  //
  File out      = File(m_pcFilename,"wb");
  m_ucBit       = 0;
  m_ulBitBuffer = 0;
  //
  if (m_bSeparate) {
    for(rl = m_pRawList;rl;rl = rl->m_pNext) {
      ULONG width  = (rl->m_ulWidth  + rl->m_ucSubX - 1) / (rl->m_ucSubX);
      ULONG height = (rl->m_ulHeight + rl->m_ucSubY - 1) / (rl->m_ucSubY);
      ULONG x,y;
      //
      // If multple fields are padded together, it is getting more complicated.
      // We need to write them interleaved into one plane.
      if (rl->m_bStartPacking) {
	m_ucBit = rl->m_ucBitsPacked;
	for(y = 0;y < height;y++) {
	  for(x = 0;x < width;x++) {
	    struct RawLayout *ro = rl;
	    do {
	      if (ro->m_bIsPadding) {
		WriteData(out,0,ro->m_ucBits,ro->m_ucBitsPacked,ro->m_bLittleEndian,ro->m_bLefty);
	      } else {
		struct ComponentLayout *cl = m_pComponent + ro->m_usTargetChannel;
		UBYTE *ptr = ((UBYTE *)(cl->m_pPtr)) + (y * cl->m_ulBytesPerRow) + (x * cl->m_ulBytesPerPixel);
		UQUAD data = 0;
		if (ro->m_ucBits <= 8) {
		  data = *ptr;
		} else if (ro->m_ucBits <= 16) {
		  if (ro->m_bFloat) {
		    FLOAT dt = *(FLOAT *)(ptr);
		    data = F2H(dt);
		  } else {
		    data = *(UWORD *)(ptr);
		  }
		} else if (ro->m_ucBits <= 32) {
		  data = *(ULONG *)(ptr);
		} else if (ro->m_ucBits <= 64) {
		  data = *(UQUAD *)(ptr);
		}
		WriteData(out,data,ro->m_ucBits,ro->m_ucBitsPacked,ro->m_bLittleEndian,ro->m_bLefty);
	      }
	    } while((ro = ro->m_pNext) && ro->m_bStartPacking == false && ro->m_ucBitsPacked);
	  }
	  BitAlignOut(out,8,rl->m_bLittleEndian,rl->m_bLefty);
	}
	// Advance over the packed sequence.
	while(rl->m_pNext && rl->m_pNext->m_bStartPacking == false && rl->m_pNext->m_ucBitsPacked)
	  rl = rl->m_pNext;
	//
      } else if (rl->m_bIsPadding) {
	m_ucBit = 8;
	for(y = 0;y < height;y++) {
	  for(x = 0;x < width;x++) {
	    WriteData(out,0,rl->m_ucBits,8,rl->m_bLittleEndian,rl->m_bLefty);
	  }
	  BitAlignOut(out,8,rl->m_bLittleEndian,rl->m_bLefty);
	}
      } else {
	UWORD i                    = rl->m_usTargetChannel;
	struct ComponentLayout *cl = m_pComponent + i;
	UBYTE *rptr                = (UBYTE *)cl->m_pPtr;
	m_ucBit = 8;
	for(y = 0;y < height;y++) {
	  UBYTE *ptr = rptr;
	  for(x = 0;x < width;x++) {
	    UQUAD data = 0;
	    if (rl->m_ucBits <= 8) {
	      data = *ptr;
	    } else if (rl->m_ucBits <= 16) {
	      if (rl->m_bFloat) {
		FLOAT dt = *(FLOAT *)(ptr);
		data = F2H(dt);
	      } else {
		data = *(UWORD *)(ptr);
	      }
	    } else if (rl->m_ucBits <= 32) {
	      data = *(ULONG *)(ptr);
	    } else if (rl->m_ucBits <= 64) {
	      data = *(UQUAD *)(ptr);
	    }
	    WriteData(out,data,rl->m_ucBits,8,rl->m_bLittleEndian,rl->m_bLefty);
	    ptr += cl->m_ulBytesPerPixel;
	  }
	  BitAlignOut(out,8,rl->m_bLittleEndian,rl->m_bLefty);
	  rptr += cl->m_ulBytesPerRow;
	}
      }
    }
  } else {
    ULONG y;
    for(y = 0;y < m_ulHeight;y++) {
      ULONG x[MAX_UWORD];
      bool rowdone;
      memset(x,0,sizeof(x));
      do {
	m_ucBit = m_pRawList->m_ucBitsPacked;
	for(rl = m_pRawList,rowdone = true;rl;rl = rl->m_pNext) {
	  if (rl->m_bIsPadding) {
	    WriteData(out,0,rl->m_ucBits,rl->m_ucBitsPacked,rl->m_bLittleEndian,rl->m_bLefty);
	  } else {
	    UWORD i = rl->m_usTargetChannel;
	    struct ComponentLayout *cl = m_pComponent + i;
	    if (x[i] < cl->m_ulWidth) {
	      UBYTE *ptr = ((UBYTE *)(cl->m_pPtr)) + (y * cl->m_ulBytesPerRow) + (x[i] * cl->m_ulBytesPerPixel);
	      UQUAD data = 0;
	      //
	      if (rl->m_ucBits <= 8) {
		data = *ptr;
	      } else if (rl->m_ucBits <= 16) {
		if (rl->m_bFloat) {
		  FLOAT dt = *(FLOAT *)(ptr);
		  data = F2H(dt);
		} else {
		  data = *(UWORD *)(ptr);
		}
	      } else if (rl->m_ucBits <= 32) {
		data = *(ULONG *)(ptr);
	      } else if (rl->m_ucBits <= 64) {
		data = *(UQUAD *)(ptr);
	      }
	      WriteData(out,data,rl->m_ucBits,rl->m_ucBitsPacked,rl->m_bLittleEndian,rl->m_bLefty);
	      //
	      // Advance to the next display position for this channel.
	      x[i]++;
	    } else {
	      // Write out dummy data to align it.
	      WriteData(out,0,rl->m_ucBits,rl->m_ucBitsPacked,rl->m_bLittleEndian,rl->m_bLefty);
	    }
	  }
	  if (rl->m_ucBitsPacked && ((rl->m_pNext == NULL) || rl->m_pNext->m_bStartPacking ||
				     (rl->m_pNext->m_ucBitsPacked == 0))) {
	    // Current position is the last field of the current bit-pack.
	    // Stop packing, next one is unpacked
	    BitAlignOut(out,rl->m_ucBitsPacked,rl->m_bLittleEndian,rl->m_bLefty);
	    // If the next field starts a new pack, reset the bit-counter for it.
	    if (rl->m_pNext && rl->m_pNext->m_bStartPacking)
	      m_ucBit = rl->m_pNext->m_ucBitsPacked;
	  } else if (rl->m_ucBitsPacked == 0 && rl->m_pNext && rl->m_pNext->m_ucBitsPacked) {
	    // Start bit packing, next one is packed.
	    m_ucBit = rl->m_pNext->m_ucBitsPacked;
	  }
	}
	for(UWORD i = 0;i < m_usDepth;i++) {
	  if (x[i] < m_pComponent[i].m_ulWidth)
	    rowdone = false;
	}
      } while(rowdone == false);
    }
  }
}
///
