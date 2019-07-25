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
** $Id: scale.cpp,v 1.18 2019/03/01 10:15:56 thor Exp $
**
** This class scales images, converts them from and to float
*/

/// Includes
#include "diff/scale.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// Scale::Convert
template<typename S,typename T>
void Scale::Convert(const S *org ,ULONG obytesperpixel,ULONG obytesperrow,
		    T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		    ULONG w, ULONG h,
		    double scale ,double shift,double min,double max)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    const S *orgrow = org;
    T *dstrow       = dst;
    for(x = 0;x < w;x++) {
      double v    = (*orgrow - shift) * scale;
      if (v < min) {
	*dstrow   = min;
      } else if (v > max) {
	*dstrow   = max;
      } else {
	*dstrow   = v;
      }
      orgrow      = (const S *)((const UBYTE *)(orgrow) + obytesperpixel);
      dstrow      = (T *)((UBYTE *)(dstrow) + dbytesperpixel);
    }
    org = (const S *)((const UBYTE *)(org) + obytesperrow);
    dst = (T *)((UBYTE *)(dst) + dbytesperrow);
  }
}
///

/// Scale::~Scale
Scale::~Scale(void)
{
  int i;

  if (m_ppucImage) {
    for(i = 0;i < m_usDepth;i++) {
      if (m_ppucImage[i])
        delete[] m_ppucImage[i];
    }
    delete[] m_ppucImage;
  }

  delete m_pDest;
}
///

/// Scale::ApplyScaling
// Apply a scaling from the source to the image stored here.
void Scale::ApplyScaling(class ImageLayout *src)
{
  UWORD comp;

  CreateComponents(*src);
  m_ppucImage = new UBYTE *[src->DepthOf()];
  memset(m_ppucImage,0,sizeof(UBYTE *) * src->DepthOf());

  for(comp = 0;comp < src->DepthOf();comp++) {
    ULONG  w    = src->WidthOf(comp);
    ULONG  h    = src->HeightOf(comp);
    UBYTE *mem;
    UBYTE bps      = 8; 
    UBYTE sbps     = src->BitsOf(comp);
    bool  tofloat  = false;
    bool  tosigned = false;
    double min     = 0.0;
    double max     = 1.0;
    double scale   = 1.0;
    double shift   = 0.0;
    ULONG dbpp;
    ULONG dbpr;
    //
    // Get a bits per sample value. If the target is integer, get specified bitdepth.
    // Otherwise, use existing bitdepth. Otherwise, use eight.
    if (m_bMakeFloat == false && (m_bMakeInt || src->isFloat(comp) == false)) {
      if (m_ucTargetDepth) {
	bps = m_ucTargetDepth;
      } else if (src->isFloat(comp) == false) {
	bps = src->BitsOf(comp);
      } else {
	bps = 8;
      }
      if (bps > 16)
	throw "invalid target bit depth specified, must be <= 16 for integer";
      tofloat = false;
    } else if (m_bMakeInt == false && (m_bMakeFloat || src->isFloat(comp) == true)) {
      // Default is float, unless the target depth is 64.
      if (m_ucTargetDepth == 0 || m_ucTargetDepth == 32) {
	bps = 32;
      } else if (m_ucTargetDepth == 64) {
	bps = 64;
      } else if (m_ucTargetDepth == 16) {
	bps = 16;
      } else {
	throw "invalid target bit depth specified, must be 32 or 64 for floating point";
      }
      tofloat = true;
      min     = 0;
      max     = 1;
    } else {
      throw "invalid conversion specified, cannot convert to int and float at the same time";
    }
    if (m_bMakeSigned == false && (m_bMakeUnsigned || src->isSigned(comp) == false)) {
      tosigned = false;
    } else if (m_bMakeUnsigned == false && (m_bMakeSigned || src->isSigned(comp) == true)) {
      tosigned = true;
    } else {
      throw "invalid conversion specified, cannot convert to unsigned and signed at the same time";
    }
    //
    // Now install the parameters.
    dbpp = ImageLayout::SuggestBPP(bps,tofloat);
    mem  = new UBYTE[w * h * dbpp];
    m_ppucImage[comp]                    = mem;
    m_pComponent[comp].m_ucBits          = bps;
    m_pComponent[comp].m_bSigned         = tosigned;
    m_pComponent[comp].m_bFloat          = tofloat;
    m_pComponent[comp].m_ulWidth         = w;
    m_pComponent[comp].m_ulHeight        = h;
    m_pComponent[comp].m_ulBytesPerPixel = dbpp;
    m_pComponent[comp].m_ulBytesPerRow   = dbpr = w * dbpp;
    m_pComponent[comp].m_pPtr            = mem;
    //
    if (tofloat) {
      min = -HUGE_VAL;
      max =  HUGE_VAL; // never clip
    } else {
      if (tosigned) {
	min = -(QUAD(1)  << (bps - 1));
	max =  (QUAD(1)  << (bps - 1)) - 1;
      } else {
	min = 0;
	max =  (UQUAD(1) <<  bps     ) - 1;
      }
    }
    //
    if (tofloat) {
      shift = 0.0; // represent unshifted.
      if (src->isFloat(comp)) {
	scale = 1.0;
      } else {
	if (src->isSigned(comp)) {
	  scale = 1.0 / ((UQUAD(1) << (sbps - 1)) - 1); // scale max to 1.0
	} else {
	  scale = 1.0 / ((UQUAD(1) << sbps) - 1);
	}
      }
    } else { // target is a integer type.
      UQUAD e = (bps > sbps)?1:0; // one on upscaling.
      if (tosigned) { // target is a signed integer type.
	if (src->isFloat(comp)) {
	  shift = 0.0;
	  scale = (UQUAD(1) << (bps - 1)) - 1; // scale to maximum
	} else if (src->isSigned(comp)) {
	  shift = 0.0;
	  scale = double((UQUAD(1) << (bps - 1)) - e) / (((UQUAD(1) << (sbps - 1)) - e));
	} else {
	  shift = QUAD(1) << (sbps - 1);
	  scale = double((UQUAD(1) << (bps - 1)) - e) / (((UQUAD(1) << (sbps - 1)) - e));
	}
      } else { // target is an unsigned integer type
	if (src->isFloat(comp)) {
	  shift = 0.0;
	  scale = (UQUAD(1) << bps) - 1; // scale to maximum
	} else if (src->isSigned(comp)) {
	  shift = -(QUAD(1) << (sbps - 1));
	  scale = double((UQUAD(1) << bps) - e) / ((UQUAD(1) << sbps) - e);
	} else {
	  shift = 0.0;
	  scale = double((UQUAD(1) << bps) - e) / ((UQUAD(1) << sbps) - e);
	}
      }
    }
    // Do not multiply if padding is requested.
    if (m_bPad)
      scale = 1.0;
    //
    //
    if (tofloat) {
      if (bps == 32 || bps == 16) {
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,FLOAT>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(FLOAT *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,FLOAT>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (FLOAT *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,FLOAT>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(FLOAT *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,FLOAT>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (FLOAT *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,FLOAT>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(FLOAT *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,FLOAT>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (FLOAT *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,FLOAT>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (FLOAT *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,FLOAT>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(FLOAT *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else if (bps == 64) {
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,DOUBLE>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (DOUBLE *)mem            ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,DOUBLE>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (DOUBLE *)mem             ,dbpp                    ,dbpr,
				  w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,DOUBLE>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (DOUBLE *)mem            ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,DOUBLE>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (DOUBLE *)mem             ,dbpp                    ,dbpr,
				  w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,DOUBLE>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (DOUBLE *)mem            ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,DOUBLE>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				  (DOUBLE *)mem             ,dbpp                    ,dbpr,
				  w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,DOUBLE>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(DOUBLE *)mem              ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,DOUBLE>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (DOUBLE *)mem               ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else {
	throw "unsupported destination data format";
      }
    } else if (tosigned) {
      if (bps <= 8) { // convert to BYTE
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,BYTE>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (BYTE *)mem             ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,BYTE>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(BYTE *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,BYTE>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (BYTE *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,BYTE>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(BYTE *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,BYTE>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (BYTE *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,BYTE>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(BYTE *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,BYTE>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (BYTE *)mem               ,dbpp                    ,dbpr,
			      w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,BYTE>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (BYTE *)mem                ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else if (bps <= 16) { // convert to WORD
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,WORD>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (WORD *)mem             ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,WORD>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(WORD *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,WORD>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (WORD *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,WORD>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(WORD *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,WORD>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (WORD *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,WORD>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(WORD *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,WORD>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (WORD *)mem               ,dbpp                    ,dbpr,
			      w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,WORD>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (WORD *)mem                ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else if (bps <= 32) { // convert to LONG
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,LONG>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (LONG *)mem             ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,LONG>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(LONG *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,LONG>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (LONG *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,LONG>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(LONG *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,LONG>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (LONG *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,LONG>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(LONG *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,LONG>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			      (LONG *)mem               ,dbpp                    ,dbpr,
			      w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,LONG>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (LONG *)mem                ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else {
	throw "unsupported target format";
      }
    } else { // to unsigned
      if (bps <= 8) { // convert to UBYTE
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,UBYTE>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UBYTE *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,UBYTE>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UBYTE *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,UBYTE>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UBYTE *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,UBYTE>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UBYTE *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,UBYTE>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UBYTE *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,UBYTE>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UBYTE *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,UBYTE>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (UBYTE *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,UBYTE>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UBYTE *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else if (bps <= 16) { // convert to UWORD 
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,UWORD>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UWORD *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,UWORD>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UWORD *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,UWORD>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UWORD *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,UWORD>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UWORD *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,UWORD>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UWORD *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,UWORD>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (UWORD *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,UWORD>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (UWORD *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,UWORD>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(UWORD *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else if (bps <= 32) { // convert to ULONG
	if (src->BitsOf(comp) <= 8) {
	  if (src->isSigned(comp)) {
	    Convert<BYTE,ULONG>((BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(ULONG *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UBYTE,ULONG>((UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (ULONG *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 16) {
	  if (src->isSigned(comp)) {
	    Convert<WORD,ULONG>((WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(ULONG *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<UWORD,ULONG>((UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (ULONG *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (!src->isFloat(comp) && src->BitsOf(comp) <= 32) {
	  if (src->isSigned(comp)) {
	    Convert<LONG,ULONG>((LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(ULONG *)mem             ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	  } else {
	    Convert<ULONG,ULONG>((ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				 (ULONG *)mem              ,dbpp                    ,dbpr,
				 w,h,scale,shift,min,max);
	  }
	} else if (src->BitsOf(comp) <= 32 && src->isFloat(comp)) {
	  Convert<FLOAT,ULONG>((FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			       (ULONG *)mem              ,dbpp                    ,dbpr,
			       w,h,scale,shift,min,max);
	} else if (src->BitsOf(comp) == 64 && src->isFloat(comp)) {
	  Convert<DOUBLE,ULONG>((DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
				(ULONG *)mem               ,dbpp                    ,dbpr,
				w,h,scale,shift,min,max);
	} else {
	  throw "unsupported source data format";
	}
      } else {
	throw "unsupported target data format";
      }
    }
  }
}
///

/// Scale::Measure
double Scale::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  //
  // Apply as a filter?
  if (m_pTargetFile == NULL) {
    // First apply on this image.
    ApplyScaling(src);
    //
    // Create a destination image.
    assert(m_pDest == NULL);
    m_pDest = new class Scale(m_pTargetFile,m_bMakeInt,m_bMakeFloat,m_bMakeUnsigned,m_bMakeSigned,
			      m_ucTargetDepth,m_bPad,m_TargetSpecs);
    //
    // Also map the destination image.
    m_pDest->ApplyScaling(dst);
    //
    // Replace the original images with the modified versions.
    src->Swap(*this);
    dst->Swap(*m_pDest);
  } else {
    ApplyScaling(src);
    
    SaveImage(m_pTargetFile,m_TargetSpecs);
  }
  return in;
}
///

