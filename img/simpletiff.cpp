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
** An image class to load and save TIFF images (or some of them).
**
** $Id: simpletiff.cpp,v 1.32 2017/01/31 11:58:04 thor Exp $
*/

/// Includes
#include "img/simpletiff.hpp"
#include "std/assert.hpp"
#include "std/string.hpp"
#include "std/stdio.hpp"
#include "tools/file.hpp"
#include "tools/halffloat.hpp"
#include "tiff/tiffparser.hpp"
#include "tiff/tiffwriter.hpp"
#include "tiff/tifftags.hpp"
#include "tiff/lzwdecoder.hpp"
#include "tiff/packbitsdecoder.hpp"
#include "tiff/trivialdecoder.hpp"
#include "img/imgspecs.hpp"
///

/// SimpleTiff::SimpleTiff
// default constructor
SimpleTiff::SimpleTiff(void)
  : m_ppComponents(NULL), m_usCount(0)
{
}
///

/// SimpleTiff::SimpleTiff
// copy the layout and reference from a
// different layout.
SimpleTiff::SimpleTiff(const class ImageLayout &layout)
  : ImageLayout(layout), m_ppComponents(NULL), m_usCount(0)
{
}
///

/// SimpleTiff::~SimpleTiff
// Destroy the tiff class and all the memory associated with it.
SimpleTiff::~SimpleTiff(void)
{

  if (m_ppComponents) {
    UWORD comp,d = m_usCount;
    
    for(comp = 0;comp < d;comp++)
      delete m_ppComponents[comp];

    delete[] m_ppComponents;
  }
}
///

/// SimpleTiff::SaveImage
// Save an image to a level 1 file descriptor, given its
// width, height and depth. We only support grey level and
// RGB here, no palette images.
void SimpleTiff::SaveImage(const char *filename,const struct ImgSpecs &specs)
{
  UBYTE *buffer = 0;
  UWORD comp,comq,t;
  ULONG x,y;
  ULONG w   = WidthOf();
  ULONG h   = HeightOf();
  UWORD d   = DepthOf();
  UBYTE sx  = 1;
  UBYTE sy  = 1;
  UBYTE bps = BitsOf(0);
  ULONG bpp = 0;
  bool  ycc      = false;
  bool  separate = false;
  bool  isfloat  = false;
  ULONG bytesperrow;
  
  for(comp = 0;comp < d;comp++) {
    if (isFloat(comp))
      isfloat = true;
    if (BitsOf(comp) != bps || isFloat(comp) != isFloat(0)) {
      bps = 0; // bit depths vary.
    }
    bpp += BitsOf(comp);
    if (comp == 0 || comp > 2) {
      if (m_pComponent[comp].m_ucSubX != 1 ||
	  m_pComponent[comp].m_ucSubY != 1)
	throw "the TIFF saver does not support subsampling in any component but 1 or 2, sorry.";
    } else {
      if (comp == 1) {
	sx = m_pComponent[comp].m_ucSubX;
	sy = m_pComponent[comp].m_ucSubY;
      } else if (comp == 2) {
	if (sx != m_pComponent[comp].m_ucSubX || sy != m_pComponent[comp].m_ucSubY) {
	  throw "the TIFF saver does not support different subsampling in U and V components, sorry.";
	}
      }
    }
  }

  if (sx != 1 || sy != 1) {
    if (comp < 3)
      throw "the TIFF safer does not support one or two components with subsampling, sorry.";
    ycc = true;
  } else if (specs.YUVEncoded == ImgSpecs::Yes) {
    if (comp < 3)
      throw "the TIFF safer does not support one or two components in YCC color space, sorry.";
    ycc = true;
  }

  if (specs.Interleaved == ImgSpecs::No) {
    separate = true;
  }

  class TiffWriter writer(filename,(specs.LittleEndian == ImgSpecs::No)?(true):(false));

  writer.DefineScalarTag(TiffTag::COMPRESSION,TiffTag::Compression::NONE);
  writer.DefineScalarTag(TiffTag::IMAGEWIDTH,w);
  writer.DefineScalarTag(TiffTag::IMAGELENGTH,h);
  writer.DefineScalarTag(TiffTag::ROWSPERSTRIP,h); // one strip is enough.
  writer.DefineScalarTag(TiffTag::SAMPLESPERPIXEL,d);

  if (isfloat && specs.RadianceScale != 1.0) {
    writer.DefineFloatTag(TiffTag::STONITS,specs.RadianceScale);
  }
  
  if (ycc) {
    UWORD depth = d;
    writer.DefineScalarTag(TiffTag::PHOTOMETRIC,TiffTag::Photometric::YCBCR);
    if (depth > 3) {
      void *tag = writer.DefineTag(TiffTag::EXTRASAMPLES,3,d - 3);
      while(depth > 3) {
	depth--;
	writer.DefineTagValue(tag,depth - 3,2);
      }
    }
    void *sub = writer.DefineTag(TiffTag::YCBCRSUBSAMPLING,3,2);
    writer.DefineTagValue(sub,0,sx);
    writer.DefineTagValue(sub,1,sy);
    separate = true;
  } else switch(d) {
  case 1:
    writer.DefineScalarTag(TiffTag::PHOTOMETRIC,TiffTag::Photometric::MINISBLACK);
    break;
  case 2:
    writer.DefineScalarTag(TiffTag::PHOTOMETRIC ,TiffTag::Photometric::MINISBLACK);
    writer.DefineScalarTag(TiffTag::EXTRASAMPLES,2); // assume that this is an alpha channel
    break;    
  case 3:
    writer.DefineScalarTag(TiffTag::PHOTOMETRIC,TiffTag::Photometric::RGB);
    break;
  case 4:
    // Assume RGB plus alpha. Could also be CMYK. Yuck!
    writer.DefineScalarTag(TiffTag::PHOTOMETRIC,TiffTag::Photometric::RGB);
    writer.DefineScalarTag(TiffTag::EXTRASAMPLES,2); // assume that this is an alpha channel
    break;
  case 5:
    // Assume CMYK plus alpha.
    writer.DefineScalarTag(TiffTag::PHOTOMETRIC,TiffTag::Photometric::SEPARATED);
    writer.DefineScalarTag(TiffTag::EXTRASAMPLES,2); // assume that this is an alpha channel
    break;
  default:
    // Hmm. Unclear what to do otherwise.
    break;
  }
  
  void *bpt = writer.DefineTag(TiffTag::BITSPERSAMPLE,3,d);
  void *fmt = writer.DefineTag(TiffTag::SAMPLEFORMAT,3,d);
  void *ofs = writer.DefineTag(TiffTag::STRIPOFFSETS,4,(separate)?(d):(1));

  for(comp = 0;comp < d;comp++) {
    if (isFloat(comp)) {
      writer.DefineTagValue(fmt,comp,TiffTag::Sampleformat::IEEEFP);
    } else if (isSigned(0)) {
      writer.DefineTagValue(fmt,comp,TiffTag::Sampleformat::INT);
    } else {
      writer.DefineTagValue(fmt,comp,TiffTag::Sampleformat::UINT);
    }
    writer.DefineTagValue(bpt,comp,BitsOf(comp));
  }

  if (separate) {
    void *bcn    = writer.DefineTag(TiffTag::STRIPBYTECOUNTS,4,d);
    writer.DefineScalarTag(TiffTag::PLANARCONFIG,TiffTag::Planarconfig::SEPARATE);
    ULONG offset = writer.LayoutTags();
    for (comp = 0;comp < d;comp++) {
      if (comp == 1 || comp == 2) {
	bytesperrow  = (BitsOf(comp)  * ((w + sx - 1) / sx) + 7) >> 3;
	bytesperrow *= (h + sy - 1) / sy;
      } else {
	bytesperrow  = ((BitsOf(comp) * w) + 7) >> 3;
	bytesperrow *= h;
      }
      writer.DefineTagValue(bcn,comp,bytesperrow);
      writer.DefineTagValue(ofs,comp,offset);
      if (offset + bytesperrow < offset)
	throw "TIFF image growing too large";
      offset += bytesperrow;
    }
  } else {
    bytesperrow = ((bpp * w) + 7) >> 3;
    writer.DefineScalarTag(TiffTag::PLANARCONFIG,TiffTag::Planarconfig::CONTIG);
    writer.DefineScalarTag(TiffTag::STRIPBYTECOUNTS,h * bytesperrow);
    writer.DefineTagValue(ofs,0,writer.LayoutTags());
  }
  
  // Tags are now complete. Now write the IFD.
  writer.WriteIFD();

    
  if (separate) {
    t = d;
    d = 1;
  } else {
    t = 1;
  }

  for(comq = 0;comq < t;comq ++) { 
    
    if (separate) {
      if (comq == 1 || comq == 2) {
	w          = (WidthOf()  + sx - 1) / sx;
	h          = (HeightOf() + sy - 1) / sy;
      } else {
	w          = WidthOf();
	h          = HeightOf();
      }
      bytesperrow  = (BitsOf(comq)  * w + 7) >> 3;
      buffer       = writer.GetStripBuffer(bytesperrow * h);
    } else { 
      w            = WidthOf();
      h            = HeightOf();
      bytesperrow  = ((bpp * w) + 7) >> 3;
      buffer       = writer.GetStripBuffer(bytesperrow * h);
    }
    
    for(y = 0;y < h;y++) {
      UBYTE *bptr = buffer;
      switch(bps) {
      case 8:
	for(x = 0;x < w;x++) {
	  for(comp = 0;comp < d;comp++) {
	    struct ComponentLayout *cl = m_pComponent + comp + comq;
	    *bptr = *(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x));
	    bptr++;
	  }
	}
	break;
      case 16:
	// Special hack for half-float which is internally represented as FLOAT
	if (isFloat(0)) {
	  for(x = 0;x < w;x++) {
	    for(comp = 0;comp < d;comp++) {
	      struct ComponentLayout *cl = m_pComponent + comp + comq;
	      writer.PutUWORD(bptr,F2H(*(FLOAT *)(((UBYTE *)cl->m_pPtr)+(cl->m_ulBytesPerRow * y)+(cl->m_ulBytesPerPixel * x))));
	    }
	  }
	} else {
	  for(x = 0;x < w;x++) {
	    for(comp = 0;comp < d;comp++) {
	      struct ComponentLayout *cl = m_pComponent + comp + comq;
	      writer.PutUWORD(bptr,*(UWORD *)(((UBYTE *)cl->m_pPtr)+(cl->m_ulBytesPerRow * y)+(cl->m_ulBytesPerPixel * x)));
	    }
	  }
	}
	break;
      case 32:
	for(x = 0;x < w;x++) {
	  for(comp = 0;comp < d;comp++) {
	    struct ComponentLayout *cl = m_pComponent + comp + comq;
	    writer.PutULONG(bptr,*(ULONG *)(((UBYTE *)cl->m_pPtr)+(cl->m_ulBytesPerRow * y)+(cl->m_ulBytesPerPixel * x)));
	  }
	}
	break;
      case 64:
	for(x = 0;x < w;x++) {
	  for(comp = 0;comp < d;comp++) {
	    struct ComponentLayout *cl = m_pComponent + comp + comq;
	    writer.PutUQUAD(bptr,*(UQUAD *)(((UBYTE *)cl->m_pPtr)+(cl->m_ulBytesPerRow * y)+(cl->m_ulBytesPerPixel * x)));
	  }
	}
	break;
      default: // bit-packing, and bit depths vary.
	// Bit-packing.
	memset(buffer,0,bytesperrow);
	{
	  UBYTE bitpos = 8;
	  for(x = 0;x < w;x++) {
	    for(comp = 0;comp < d;comp++) {
	      struct ComponentLayout *cl = m_pComponent + comp + comq;
	      UBYTE b = BitsOf(comp);
	      
	      if (b <= 8) {
		writer.PutBits(bptr,bitpos,b,
			       *(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)));
	      } else if (b < 16) {
		writer.PutBits(bptr,bitpos,b,
			       *(UWORD *)(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)));
	      } else if (b == 16) {
		if (isFloat(comp)) {
		  writer.PutBits(bptr,bitpos,16,
				 F2H(*(FLOAT *)(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x))));
		} else {
		  writer.PutBits(bptr,bitpos,16,
				 *(UWORD *)(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)));
		}
	      } else if (b <= 32) {
		writer.PutBits(bptr,bitpos,32,
			       *(ULONG *)(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)));
	      } else {
		throw "cannot write image files with varying bit depths containing more than 32 bits per pixel, sorry";
	      }
	    }
	  }
	  if (bitpos < 8) {
	    // bitpos = 8; // superflous, done anyhow.
	    bptr++;
	  }
	}
      }
      buffer += bytesperrow;
    }
    writer.PushStripBuffer();
  }
}
///

/// SimpleTiff::UnpackDataYCbCr
// Unpack the data from the source buffer into the destination component,
// including the subsampling of the Cb and Cr components as indicated.
// Y components are in a larger subbox of dimensions sx and sy.
// xofs and yofs are offsets into the target plane where the data is to be
// copied to. This always copies three components.
// width and height are the dimensions of the rectangle to be copied.
// bits is the number of bits per sample, size the buffer size.
template<class Decoder>
void SimpleTiff::UnpackDataYCbCr(UBYTE *buffer,bool bigendian,bool prediction,
				 ULONG xofs,ULONG yofs,
				 ULONG width ,ULONG height,
				 UBYTE bitcnt,ULONG bytes,
				 UBYTE sx    ,UBYTE sy)
{
  ULONG xs,ys;
  UBYTE xb,yb;
  ULONG x,y;
  ULONG xe = width  + xofs;
  ULONG ye = height + yofs;
  UWORD c;
  UBYTE *dst;
  Decoder d(buffer,bytes,bigendian);
  
  if (xofs  % sx != 0 || yofs   % sy != 0)
    throw "TIFF tile or stripe positions are not divisble by the subsampling factors";
  if (prediction)
    throw "Prediction not available in YCbCr decoding mode";

  xofs   /= sx;
  yofs   /= sy;
  width   = (width  + sx - 1) / sx;
  height  = (height + sy - 1) / sy;

  for(ys = 0;ys < height;ys++) {
    for(xs = 0;xs < width;xs++) {
      struct ComponentLayout *cl = m_pComponent;
      for(yb = 0;yb < sy;yb++) {
	for(xb = 0;xb < sx;xb++) {
	  x    = (xs + xofs) * sx + xb;
	  y    = (ys + yofs) * sy + yb;
	  if (x < xe && y < ye) {
	    dst  = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	    if (bitcnt <= 8) {
	      *dst = d.GetBits(bitcnt,cl->m_bSigned);
	    } else if (bitcnt <= 16) {
	      *(UWORD *)dst = d.GetBits(bitcnt,cl->m_bSigned);
	    } else if (bitcnt <= 32) {
	      *(ULONG *)dst = d.GetBits(bitcnt,cl->m_bSigned);
	    }
	  } else {
	    // The corresponding luma sample is not available, but still represented in the buffer.
	    // This image would be invalid according to the TIFF specs, but such images exist.
	    d.GetBits(bitcnt,cl->m_bSigned);
	  }
	}
      }
      // Unpack Cb and Cr.
      for(c = 0;c < 2;c++) {
	cl++;
	x    = xs + xofs;
	y    = ys + yofs;
	dst  = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	if (bitcnt <= 8) {
	  *dst = d.GetBits(bitcnt,cl->m_bSigned);
	} else if (bitcnt <= 16) {
	  *(UWORD *)dst = d.GetBits(bitcnt,cl->m_bSigned);
	} else if (bitcnt <= 32) {
	  *(ULONG *)dst = d.GetBits(bitcnt,cl->m_bSigned);
	}
      }
    }
    d.ByteAlign(); // Padding at end of row according
  }
}
///

/// SimpleTiff::UnpackData
// Unpack data that is in encoded form in the given buffer, with the given
// buffer size, and possible horizontal prediction.
template<class Decoder>
void SimpleTiff::UnpackData(UBYTE *buffer,bool bigendian,bool hdiff,
			    UWORD comp,UWORD cnt,ULONG xofs,ULONG y,
			    ULONG width,ULONG height,
			    UBYTE b,const ULONG *bits,const ULONG *fmt,
			    ULONG bytes,ULONG inv,DOUBLE scale)
{
  Decoder d(buffer,bytes,bigendian); // stripes/tiles are decoded independently.
  ULONG xs,ys,x;
  UWORD c;

  switch(b) {
  case 8:
    for(ys = 0;ys < height;ys++,y++) {
      for(xs = 0,x = xofs;xs < width;xs++,x++) {
	for(c = 0;c < cnt;c++) {
	  struct ComponentLayout *cl = m_pComponent + c + comp;
	  UBYTE *dst = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	  ULONG dt = d.GetUBYTE();
	  if (hdiff && xs > 0)
	    dt += *(dst-cl->m_ulBytesPerPixel)^inv;
	  *dst = dt^inv;
	}
      }
    }
    break;
  case 16: // this can be both float and short.
    if (fmt[comp] == TiffTag::Sampleformat::IEEEFP) {
      for(ys = 0;ys < height;ys++,y++) {
	for(xs = 0,x = xofs;xs < width;xs++,x++) {
	  for(c = 0;c < cnt;c++) {
	    struct ComponentLayout *cl = m_pComponent + c + comp;
	    UBYTE *dst = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	    FLOAT dt   = H2F(d.GetUWORD());
	    if (hdiff && xs > 0)
	      dt += *(FLOAT *)(dst-cl->m_ulBytesPerPixel);
	    *(FLOAT *)dst = scale * dt;
	  }
	}
      }
    } else {
      for(ys = 0;ys < height;ys++,y++) {
	for(xs = 0,x = xofs;xs < width;xs++,x++) {
	  for(c = 0;c < cnt;c++) {
	    struct ComponentLayout *cl = m_pComponent + c + comp;
	    UBYTE *dst = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	    ULONG dt   = d.GetUWORD();
	    if (hdiff && xs > 0)
	      dt += *(UWORD *)(dst-cl->m_ulBytesPerPixel)^inv;
	    *(UWORD *)dst = dt^inv;
	  }
	}
      }
    }
    break;
  case 32: // this can be both FLOAT and 32 bit. 
    if (fmt[comp] == TiffTag::Sampleformat::IEEEFP) {
      union {
	FLOAT f;
	ULONG u;
      } U2F;
      for(ys = 0;ys < height;ys++,y++) {
	for(xs = 0,x = xofs;xs < width;xs++,x++) {
	  for(c = 0;c < cnt;c++) {
	    struct ComponentLayout *cl = m_pComponent + c + comp;
	    UBYTE *dst = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	    U2F.u = d.GetULONG();
	    if (hdiff && xs > 0)
	      U2F.f += *(FLOAT *)(dst-cl->m_ulBytesPerPixel);
	    *(FLOAT *)dst = scale * U2F.f;
	  }
	}
      }
    } else {
      for(ys = 0;ys < height;ys++,y++) {
	for(xs = 0,x = xofs;xs < width;xs++,x++) {
	  for(c = 0;c < cnt;c++) {
	    struct ComponentLayout *cl = m_pComponent + c + comp;
	    UBYTE *dst = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	    ULONG dt   = d.GetULONG();
	    if (hdiff && xs > 0)
	      dt += *(ULONG *)(dst-cl->m_ulBytesPerPixel)^inv;
	    *(ULONG *)dst = dt^inv;
	  }
	}
      }
    }
    break;  
  case 64: // this can be only DOUBLE.
    for(ys = 0;ys < height;ys++,y++) {
      for(xs = 0,x = xofs;xs < width;xs++,x++) {
	for(c = 0;c < cnt;c++) {
	  union {
	    DOUBLE d;
	    UQUAD  u;
	  } U2D;
	  struct ComponentLayout *cl = m_pComponent + c + comp;
	  UBYTE *dst = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	  U2D.u = d.GetUQUAD();
	  if (hdiff && xs > 0)
	    U2D.d += *(DOUBLE *)(dst-cl->m_ulBytesPerPixel);
	  *(DOUBLE *)dst = scale * U2D.d;
	}
      }
    }
    break;
  default: // bit depths vary.
    for(ys = 0;ys < height;ys++,y++) {
      for(xs = 0,x = xofs;xs < width;xs++,x++) {
	for(c = 0;c < cnt;c++) {
	  UBYTE bitcnt = bits[c + comp];
	  if (bitcnt > 32)
	    throw "Varying bit depth with double precision numbers is not supported";
	  struct ComponentLayout *cl = m_pComponent + c + comp;
	  ULONG dt   = d.GetBits(bitcnt,cl->m_bSigned);
	  UBYTE *dst = ((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x);
	  if (bitcnt <= 8) {
	    if (hdiff && xs > 0)
	      dt += *((UBYTE *)(dst - cl->m_ulBytesPerPixel))^inv;
	    *(UBYTE *)dst = dt^inv;
	  } else if (bitcnt < 16) {
	    if (hdiff && xs > 0)
	      dt += *((UWORD *)(dst - cl->m_ulBytesPerPixel))^inv;
	    *(UWORD *)dst = dt^inv;
	  } else if (bitcnt == 16) {
	    if (fmt[c + comp] == TiffTag::Sampleformat::IEEEFP) {
	      FLOAT f = H2F(dt);
	      if (hdiff && xs > 0)
		f += *((FLOAT *)(dst - cl->m_ulBytesPerPixel));
	      *((FLOAT *)dst) = scale * f;
	    } else {
	      if (hdiff && xs > 0)
		dt += *((UWORD *)(dst - cl->m_ulBytesPerPixel))^inv;
	      *(UWORD *)dst = dt^inv;
	    }
	  } else if (bitcnt == 32) {  
	    if (fmt[c + comp] == TiffTag::Sampleformat::IEEEFP) { 
	      union {
		FLOAT f;
		ULONG u;
	      } U2F;
	      U2F.u = dt;
	      if (hdiff && xs > 0)
		U2F.f += *((FLOAT *)(dst - cl->m_ulBytesPerPixel));
	      *((FLOAT *)dst) = scale * U2F.f;
	    } else {
	      if (hdiff && xs > 0)
		dt += *((ULONG *)(dst - cl->m_ulBytesPerPixel))^inv;
	      *(ULONG *)dst = dt^inv;
	    }
	  } else {
	    if (hdiff && xs > 0)
	      dt += *((ULONG *)(dst - cl->m_ulBytesPerPixel))^inv;
	    *(ULONG *)dst = dt^inv;
	  }
	}
      }
      d.ByteAlign(); // Padding bits at end of row.
    }
    break;
  }
  
}
///

/// SimpleTiff::UnpackDataPaletized
// Unpack the data from the source buffer into the destination component.
// buffer is the data source, comp the component and cnt the number of
// components to copy. 
// xofs and yofs are offsets into the target plane where the data is to be
// copied to.
// width and height are the dimensions of the rectangle to be copied.
// bits is the number of bits per sample, size the buffer size.
template<class Decoder>
void SimpleTiff::UnpackDataPaletized(UBYTE *buffer,bool bigendian,bool prediction,
				     ULONG xofs,ULONG y,
				     ULONG width,ULONG height,UBYTE bits,ULONG bytes,
				     const ULONG *r,const ULONG *g,const ULONG *b)
{
  ULONG xs,ys,x;
  Decoder d(buffer,bytes,bigendian);

  if (prediction)
    throw "prediction mode not available for palettized data in TIFF decoder";
  
  switch(bits) {
  case 8:
    for(ys = 0;ys < height;ys++,y++) {
      for(xs = 0,x = xofs;xs < width;xs++,x++) {
	struct ComponentLayout *cl;
	UBYTE idx = d.GetUBYTE();
	cl = m_pComponent + 0;
	*(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)) = r[idx] >> 8;
	cl = m_pComponent + 1;
	*(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)) = g[idx] >> 8;
	cl = m_pComponent + 2;
	*(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)) = b[idx] >> 8;
      }
    }
    break;
  default:
    if (bits > 8)
      throw "TIFF palette indices longer than 8 bits not supported";
    for(ys = 0;ys < height;ys++,y++) {
      for(xs = 0,x = xofs;xs < width;xs++,x++) {
	struct ComponentLayout *cl;
	UWORD idx = d.GetBits(bits,false);
	cl = m_pComponent + 0;
	*(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)) = r[idx] >> 8;
	cl = m_pComponent + 1;
	*(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)) = g[idx] >> 8;
	cl = m_pComponent + 2;
	*(((UBYTE *)cl->m_pPtr) + (cl->m_ulBytesPerRow * y) + (cl->m_ulBytesPerPixel * x)) = b[idx] >> 8;
      }
      d.ByteAlign(); // Padding at end of row according
    }
    break;
  }
}
///

/// SimpleTiff::ReadTiled
// Read tiled data through the tiff interface.
void SimpleTiff::ReadTiled(class TiffParser &parser,int lzw,bool hdiff,
			   UWORD imgconfig,
			   ULONG tw,ULONG th,ULONG inv,
			   const ULONG *bits,const ULONG *fmt,
			   const ULONG *rm,const ULONG *gm,const ULONG *bm,
			   DOUBLE scale)
{
  ULONG   tilecount = parser.GetAddressableTiles();
  ULONG   tile;
  UWORD   comp      = 0; // current plane = tile.
  ULONG   x         = 0;
  ULONG   y         = 0;
  ULONG   iwidth    = WidthOf();
  ULONG   iheight   = HeightOf();
  UWORD   d         = DepthOf();
  UBYTE   b         = bits[0];
  UBYTE   sx        = (d > 1)?(m_pComponent[1].m_ucSubX):(1);
  UBYTE   sy        = (d > 1)?(m_pComponent[1].m_ucSubY):(1);

  for(comp = 0;comp < d;comp++) {
    if (bits[comp] != b || fmt[comp] != fmt[0]) {
      b = 0;
      break;
    }
  }
  comp = 0;
  
  for(tile = 0;tile < tilecount;tile++) {
    if (comp >= d) {
      throw "extra data at end of TIFF file";
    } else {
      ULONG  width  = tw;
      ULONG  height = th;
      ULONG  bytes;
      UBYTE *buffer;
      
      if (x + width  > iwidth) {
	if (x >= iwidth)
	  throw "extra data at end of TIFF file";
	width  = iwidth - x;
      }
      if (y + height > iheight) {
	if (y >= iheight)
	  throw "extra data at end of TIFF file";
	height = iheight - y;
      }
      
      if (width == 0 || height == 0)
	throw "extra data at end of TIFF file";
      
      buffer = parser.GetDataOfUnit(tile,bytes);
      
      if (rm) {
	assert(comp == 0 && d == 3);
	switch(lzw) {
	case TiffTag::Compression::NONE:
	  UnpackDataPaletized<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,x,y,width,height,bits[comp],bytes,
					      rm,gm,bm);
	  break;
	case TiffTag::Compression::PACKBITS:
	  UnpackDataPaletized<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,x,y,width,height,bits[comp],bytes,
					       rm,gm,bm);
	  break;
	case TiffTag::Compression::LZW:
	  UnpackDataPaletized<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,x,y,width,height,bits[comp],bytes,
					  rm,gm,bm);
	  break;
	}
      } else {
	switch(imgconfig) {
	case TiffTag::Planarconfig::SEPARATE: 
	  if ((comp == 1 || comp == 2) && (sx > 1 || sy > 1)) {
	    // Sizes must be divisible by the subsampling factors to be valid.
	    if (width % sx != 0 || height % sx != 0 || x % sx != 0 || y % sy != 0)
	      throw "invalid TIFF tile dimensions not divisible by subsampling factors";
	    switch(lzw) {
	    case TiffTag::Compression::LZW:
	      UnpackData<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,x / sx,y / sy,width / sx,height / sy,
				     bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackData<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,x / sx,y / sy,width / sx,height / sy,
					  bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::NONE:
	      UnpackData<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,x / sx,y / sy,width / sx,height / sy,
					 bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    }
	  } else { 
	    switch(lzw) {
	    case TiffTag::Compression::LZW:
	      UnpackData<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,x     ,y     ,width     ,height     ,
				     bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackData<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,x     ,y     ,width     ,height     ,
					  bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::NONE:
	      UnpackData<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,x     ,y     ,width     ,height     ,
					 bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    }
	  }
	  break;
	case TiffTag::Planarconfig::CONTIG: 
	  if (sx > 1 || sy > 1) {
	    switch(lzw) {
	    case TiffTag::Compression::NONE:
	      UnpackDataYCbCr<TrivialDecoder>(buffer  ,parser.isBigEndian(),hdiff,x,y,width,height     ,b,bytes,sx,sy);
	      break;
	    case TiffTag::Compression::LZW:
	      UnpackDataYCbCr<LZWDecoder>(buffer  ,parser.isBigEndian(),hdiff,x,y,width,height     ,b,bytes,sx,sy);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackDataYCbCr<PackBitsDecoder>(buffer  ,parser.isBigEndian(),hdiff,x,y,width,height     ,b,bytes,sx,sy);
	      break;
	    }
	  } else { 
	    switch(lzw) {
	    case TiffTag::Compression::LZW:
	      UnpackData<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,0,d,x,y,width,height,b,bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackData<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,0,d,x,y,width,height,b,bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::NONE:
	      UnpackData<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,0,d,x,y,width,height,b,bits,fmt,bytes,inv,scale);
	      break;
	    }
	  }
	  break;
	}
      }
      
      x += tw;
      if (x >= iwidth) {
	x  = 0;
	y += th;
	if (y >= iheight) {
	  if (imgconfig == TiffTag::Planarconfig::SEPARATE) {
	    comp++;
	    x = 0;
	    y = 0;
	  } 
	}
      }
    }
  }
}
///

/// SimpleTiff::ReadStriped
// Read striped data through the tiff interface.
void SimpleTiff::ReadStriped(class TiffParser &parser,int lzw,bool hdiff,
			     UWORD imgconfig,
			     ULONG inv,const ULONG *bits,const ULONG *fmt,
			     const ULONG *rm,const ULONG *gm,const ULONG *bm,
			     DOUBLE scale)
{
  ULONG rps = 0;  // rows per strip
  ULONG nos;      // number of strips
  ULONG strip;    // strip counter.
  UWORD comp = 0; // component counter (was: plane)
  ULONG y = 0;
  ULONG h;
  ULONG width   = WidthOf();
  ULONG height  = HeightOf();
  UWORD d       = DepthOf();
  UBYTE b       = bits[0];
  UBYTE sx      = (d > 1)?(m_pComponent[1].m_ucSubX):(1);
  UBYTE sy      = (d > 1)?(m_pComponent[1].m_ucSubY):(1);
  
  rps     = parser.GetRowsPerStrip();
  nos     = parser.GetAddressableStrips();

  for(comp = 0;comp < d;comp++) {
    if (bits[comp] != b || fmt[comp] != fmt[0]) {
      b = 0;
      break;
    }
  }
  comp = 0;
  
  for(strip = 0;strip < nos;strip++) {
    if (comp >= d) {
      throw "unexpected extra data in TIFF image";
    } else {
      UBYTE *buffer;
      ULONG  bytes;

      // Compute the expected stripe height.
      if (y + rps < height) {
	h = rps;
      } else {
	if (height <= y)
	  throw "unexpected extra data in TIFF image";
	h = height - y;
      }
      
      buffer = parser.GetDataOfUnit(strip,bytes);
      if (rm) {
	assert(comp == 0 && d == 3);
	switch(lzw) {
	  case TiffTag::Compression::NONE:
	    UnpackDataPaletized<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,0,y,width,h,bits[comp],bytes,rm,gm,bm);
	    break;
	  case TiffTag::Compression::LZW:
	    UnpackDataPaletized<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,0,y,width,h,bits[comp],bytes,rm,gm,bm);
	    break;
	  case TiffTag::Compression::PACKBITS:
	    UnpackDataPaletized<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,0,y,width,h,bits[comp],bytes,rm,gm,bm);
	    break;
	}
      } else {
	switch(imgconfig) {
	case TiffTag::Planarconfig::SEPARATE:
	  if ((comp == 1 || comp == 2) && (sx > 1 || sy > 1)) {
	    // Sizes must be divisible by the subsampling factors to be valid.
	    if (width % sx != 0 || h % sx != 0 || y % sy != 0)
	      throw "invalid TIFF stripe dimensions not divisible by subsampling factors";
	    switch(lzw) {
	    case TiffTag::Compression::LZW:
	      UnpackData<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,0,y / sy,width / sx,h / sy,
				     bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackData<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,0,y / sy,width / sx,h / sy,
					  bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::NONE:
	      UnpackData<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,0,y / sy,width / sx,h / sy,
					 bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    }
	  } else {
	    switch(lzw) {
	    case TiffTag::Compression::LZW:
	      UnpackData<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,0,y     ,width     ,h     ,
				     bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackData<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,0,y     ,width     ,h     ,
					  bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::NONE:
	      UnpackData<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,comp,1,0,y     ,width     ,h     ,
					 bits[comp],bits,fmt,bytes,inv,scale);
	      break;
	    }
	  }
	  break;
	case TiffTag::Planarconfig::CONTIG:
	  if (sx > 1 || sy > 1) {
	    switch(lzw) {
	    case TiffTag::Compression::NONE:
	      UnpackDataYCbCr<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,0,y,width,h     ,b,bytes,sx,sy);
	      break;
	    case TiffTag::Compression::LZW:
	      UnpackDataYCbCr<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,0,y,width,h     ,b,bytes,sx,sy);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackDataYCbCr<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,0,y,width,h     ,b,bytes,sx,sy);
	      break;
	    }
	  } else {
	    switch(lzw) {
	    case TiffTag::Compression::LZW:
	      UnpackData<LZWDecoder>(buffer,parser.isBigEndian(),hdiff,0   ,d,0,y,width,h,b,bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::PACKBITS:
	      UnpackData<PackBitsDecoder>(buffer,parser.isBigEndian(),hdiff,0   ,d,0,y,width,h,b,bits,fmt,bytes,inv,scale);
	      break;
	    case TiffTag::Compression::NONE:
	      UnpackData<TrivialDecoder>(buffer,parser.isBigEndian(),hdiff,0   ,d,0,y,width,h,b,bits,fmt,bytes,inv,scale);
	      break;
	    }
	  }
	  break;
	}
      }
      y  += rps;
      if (y >= height) {
	y = 0;
	if (imgconfig == TiffTag::Planarconfig::SEPARATE) {
	  comp++;
	}
      }
    }
  }
}
///

/// SimpleTiff::LoadImage
// Load an image from a level 1 file descriptor, keep it within
// the internals of this class. The accessor methods below
// should be used to find out more about this image.
void SimpleTiff::LoadImage(const char *basename,struct ImgSpecs &specs)
{ 
  class TiffParser parser(basename);
  ULONG w     = parser.GetImageWidth();
  ULONG h     = parser.GetImageHeight();
  ULONG photo = parser.GetPhotometricInterpretation();
  ULONG depth = parser.GetImageDepth();
  const ULONG *rpal = NULL;
  const ULONG *gpal = NULL;
  const ULONG *bpal = NULL;
  const ULONG *fmt  = parser.GetSampleFormat();
  const ULONG *bps  = parser.GetBitsPerPixel();
  ULONG cnf   = parser.GetPlanarConfig();
  ULONG subh  = 1;
  ULONG subv  = 1; // subsampling for YCbCr and related.
  ULONG  inv  = 0;
  UWORD  comp;
  ULONG  i;
  int    lzw   = TiffTag::Compression::NONE;
  bool   hdiff = false;
  DOUBLE scale = 1.0;

  if (parser.isBigEndian()) {
    specs.LittleEndian = ImgSpecs::No;
  } else {
    specs.LittleEndian = ImgSpecs::Yes;
  }

  switch(parser.GetCompression()) {
  case TiffTag::Compression::NONE:
  case TiffTag::Compression::LZW:
  case TiffTag::Compression::PACKBITS:
    lzw = parser.GetCompression();
    break;
  default:
    throw "unsupported TIFF compression type, sorry";
    break;
  }

  switch(parser.GetPredictor()) {
  case TiffTag::Predictor::NONE:
    break;
  case TiffTag::Predictor::HDIFF:
    hdiff = true;
    break;
  default:
    throw "unknown predictor type found in TIFF image";
    break;
  }

  scale = parser.GetScaleFactor();
  if (specs.AbsoluteRadiance != ImgSpecs::Yes) {
    // Here keep the scale in the specs, write it out later.
    specs.RadianceScale = scale;
    scale = 1.0;
  }
  
  for (i = 0;i < depth;i++) {
    if (fmt[i] == TiffTag::Sampleformat::VOID) {
      fprintf(stderr,"TIFF WARNING: sample format is unspecified, assuming UINT\n");
    }
    if (fmt[i] != TiffTag::Sampleformat::UINT && 
	fmt[i] != TiffTag::Sampleformat::INT  &&
	fmt[i] != TiffTag::Sampleformat::VOID &&
	fmt[i] != TiffTag::Sampleformat::IEEEFP)
      throw "the TIFF file contains samples in an unsupported sample format";
    if (fmt[i] == TiffTag::Sampleformat::IEEEFP) {
      if (bps[i] != 16 && bps[i] != 32 && bps[i] != 64)
	throw "unsupported sample precision, must be 32 or 64 for floating point samples";
    } else if (bps[i] < 1 || bps[i] > 32) {
      throw "unsupported sample precision, must be >= 1 and <= 16 for integer samples";
    }
  }
  
  switch(photo) {
  case TiffTag::Photometric::PALETTE:
    // Palette mapped. Convert to RGB.
    if (depth != 1)
      throw "TIFF unsupported color map with more than one sample per pixel";
    rpal = parser.GetColorMap();
    gpal = rpal + (1UL << bps[0]);
    bpal = gpal + (1UL << bps[0]);
    if (fmt[0] != TiffTag::Sampleformat::UINT && 
	fmt[0] != TiffTag::Sampleformat::VOID)
      throw "TIFF unsupported sample format for palette images, must be UINT";
    if (bps[0] > 8)
      throw "TIFF unsupported number of bits per sample for palette, must be <= 8";
    depth = 3;
    specs.Palettized = ImgSpecs::Yes;
    specs.YUVEncoded = ImgSpecs::No;
    break;
  case TiffTag::Photometric::YCBCR:
    {
      const ULONG *subs = parser.GetSubsampling();
      subh        = subs[0];
      subv        = subs[1];
      if (subh == 0 || subv == 0)
	throw "found invalid subsampling factors in TIFF file";
      //
      // The depth must be three here (what about extra samples??)
      if (depth < 3 || (depth != 3 && cnf == 1))
	throw "TIFF YCbCr images must have three components to be supported";
      if (bps[0] != bps[1] || bps[1] != bps[2])
	throw "TIFF YCbCr images with varying bit depths are not supported";
      if (bps[0] > 32)
	throw "TIFF YCbCr images with more than 32bpp are not supported";
      // 
      specs.Palettized = ImgSpecs::No;
      specs.YUVEncoded = ImgSpecs::Yes;
      // Problem is that I currently don't support the re-ordering of
      // the samples, see the TIFF specs.
    }
    break;
  case TiffTag::Photometric::MINISWHITE:
    // Also grey-scale, just inverted.
    inv = (1UL << bps[0]) - 1;
    if (fmt[0] == TiffTag::Sampleformat::IEEEFP) 
      throw "TIFF MINISWHITE not supported for floating point formats";   
    //
    specs.Palettized = ImgSpecs::No;
    specs.YUVEncoded = ImgSpecs::No;
    break;
  default:
    //
    specs.Palettized = ImgSpecs::No;
    if (specs.YUVEncoded == ImgSpecs::Unspecified)
      specs.YUVEncoded = ImgSpecs::No;
    break;
  }
  //
  specs.ASCII        = ImgSpecs::No;
  specs.Interleaved  = (cnf == TiffTag::Planarconfig::SEPARATE)?(ImgSpecs::No):(ImgSpecs::Yes);
  //
  // All data is now ready. Create the components.
  CreateComponents(w,h,depth);
  m_ppComponents = new struct TiffComponent *[m_usCount = depth];
  memset(m_ppComponents,0,sizeof(struct TiffComponent *) * depth);
  
  for(comp = 0;comp < depth;comp++) {
    struct TiffComponent *c;
    struct ComponentLayout *cl = m_pComponent + comp;
    m_ppComponents[comp] = c = new struct TiffComponent;
    UBYTE bitsperpixel   = (photo  == TiffTag::Photometric::PALETTE)?(8):(bps[comp]);
    ULONG bytesperpixel  = (bitsperpixel + 7) >> 3; // is one, also for palette index.
    bool  flt            = (photo  == TiffTag::Photometric::PALETTE)?(false):
      (fmt[comp] == TiffTag::Sampleformat::IEEEFP);
    // Special cludge: half-float is converted to float.
    if (flt && bitsperpixel == 16)
      bytesperpixel = 4;
    //
    if (comp == 0 || comp > 2) {
      c->m_ulWidth       = w;
      c->m_ulHeight      = h;
    } else {
      c->m_ulWidth       = (w + (subh - 1)) / subh;
      c->m_ulHeight      = (h + (subv - 1)) / subv;
    }
    c->m_ucDepth         = bitsperpixel;
    c->m_bSigned         = (photo  == TiffTag::Photometric::PALETTE)?(false):
      (fmt[comp] != TiffTag::Sampleformat::UINT && 
       fmt[comp] != TiffTag::Sampleformat::VOID);
    c->m_pData           = new UBYTE[c->m_ulWidth * c->m_ulHeight * bytesperpixel];
    cl->m_ulWidth        = c->m_ulWidth;
    cl->m_ulHeight       = c->m_ulHeight;
    cl->m_ucBits         = c->m_ucDepth;
    cl->m_bSigned        = c->m_bSigned;
    cl->m_bFloat         = flt;
    if (comp == 0) {
      cl->m_ucSubX       = 1;
      cl->m_ucSubY       = 1;
    } else {
      cl->m_ucSubX       = subh;
      cl->m_ucSubY       = subv;
    }
    cl->m_ulBytesPerPixel= bytesperpixel;
    cl->m_ulBytesPerRow  = c->m_ulWidth * cl->m_ulBytesPerPixel;
    cl->m_pPtr           = c->m_pData;
  }
  
  if (parser.isTiled()) {
    ULONG tw        = parser.GetTileWidth();
    ULONG th        = parser.GetTileHeight();
    //
    ReadTiled(parser,lzw,hdiff,cnf,tw,th,inv,bps,fmt,rpal,gpal,bpal,scale);
  } else {
    //
    ReadStriped(parser,lzw,hdiff,cnf,inv,bps,fmt,rpal,gpal,bpal,scale);
  }
}
///
