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
** $Id: mask.cpp,v 1.2 2014/01/04 11:35:28 thor Exp $
**
** This tool replaces the pixels in the distored image by the pixels from the original
** providing a masking operation. The masking is an alpha-blending in case the mask
** is not binary.
*/

/// Includes
#include "diff/meter.hpp"
#include "diff/mask.hpp"
#include "img/imglayout.hpp"
#include "img/imgspecs.hpp"
///

/// Mask::MixDown
template<typename T,typename S>
void Mask::MixDown(const T *org,ULONG obytesperpixel,ULONG obytesperrow,
		   T       *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		   const S *msk,ULONG mbytesperpixel,ULONG mbytesperrow,
		   double min,double max,ULONG w,ULONG h)
{ 
  ULONG x,y;

  if (m_bInvert) {
    for(y = 0;y < h;y++) {
      const T *orgrow = org;
      T *dstrow = dst;
      const S *mskrow = msk;
      for(x = 0;x < w;x++) {
	S msk = *mskrow;
	if (msk <= min) {
	  *dstrow = *orgrow; // simple replace
	} else if (msk < max) {
	  double scale = (msk - min) / (max - min);
	  *dstrow = *orgrow * (1.0 - scale) + *dstrow * scale;
	}
	
	orgrow      = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
	dstrow      = (T       *)((      UBYTE *)(dstrow) + dbytesperpixel);
	mskrow      = (const S *)((const UBYTE *)(mskrow) + mbytesperpixel);
      }
      org = (const T *)((const UBYTE *)(org) + obytesperrow);
      dst = (T       *)((      UBYTE *)(dst) + dbytesperrow);
      msk = (const S *)((const UBYTE *)(msk) + mbytesperrow);
    }
  } else {
    for(y = 0;y < h;y++) {
      const T *orgrow = org;
      T *dstrow = dst;
      const S *mskrow = msk;
      for(x = 0;x < w;x++) {
	S msk = *mskrow;
	if (msk >= max) {
	  *dstrow = *orgrow; // simple replace
	} else if (msk > min) {
	  double scale = (msk - min) / (max - min);
	  *dst = *orgrow * scale + *dstrow * (1.0 - scale);
	}
	
	orgrow      = (const T *)((const UBYTE *)(orgrow) + obytesperpixel);
	dstrow      = (T       *)((      UBYTE *)(dstrow) + dbytesperpixel);
	mskrow      = (const S *)((const UBYTE *)(mskrow) + mbytesperpixel);
      }
      org = (const T *)((const UBYTE *)(org) + obytesperrow);
      dst = (T       *)((      UBYTE *)(dst) + dbytesperrow);
      msk = (const S *)((const UBYTE *)(msk) + mbytesperrow);
    }
  }
}
///

/// Mask::Blend
// Blend, but still with unknown mask type. This then goes into one of the above
// functions.
template<typename T>
void Mask::Blend(const T *org,ULONG obytesperpixel,ULONG obytesperrow,
		 T       *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		 UWORD comp,class ImageLayout *mask)
{
  double min,max;
  //
  // Default min/max come from the bitdepth. Float has a 0-1 natural range.
  if (mask->isFloat(comp)) {
    min = 0.0;
    max = 1.0;
  } else if (mask->isSigned(comp)) {
    min = 0.0; // Yes, really.
    max = double((1UL << (mask->BitsOf(comp) - 1)) - 1);
  } else {
    min = 0.0;
    max = double((1UL << (mask->BitsOf(comp)    )) - 1);
  }

  if (mask->isSigned(comp)) {
    if (mask->BitsOf(comp) <= 8) {
      MixDown<T,BYTE>(org,obytesperpixel,obytesperrow,
		    dst,dbytesperpixel,dbytesperrow,
		    (const BYTE *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		    min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (!mask->isFloat(comp) && mask->BitsOf(comp) <= 16) {
      MixDown<T,WORD>(org,obytesperpixel,obytesperrow,
		    dst,dbytesperpixel,dbytesperrow,
		    (const WORD *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		    min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (!mask->isFloat(comp) && mask->BitsOf(comp) <= 32) {
      MixDown<T,LONG>(org,obytesperpixel,obytesperrow,
		    dst,dbytesperpixel,dbytesperrow,
		    (const LONG *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		    min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (mask->BitsOf(comp) <= 32 && mask->isFloat(comp)) {
      MixDown<T,FLOAT>(org,obytesperpixel,obytesperrow,
		     dst,dbytesperpixel,dbytesperrow,
		     (const FLOAT *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		     min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (mask->BitsOf(comp) == 64 && mask->isFloat(comp)) {
      MixDown<T,DOUBLE>(org,obytesperpixel,obytesperrow,
		      dst,dbytesperpixel,dbytesperrow,
		      (const DOUBLE *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		      min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else {
      throw "unsupported data type";
    }
  } else {
    if (mask->BitsOf(comp) <= 8) {
      MixDown<T,UBYTE>(org,obytesperpixel,obytesperrow,
		     dst,dbytesperpixel,dbytesperrow,
		     (const UBYTE *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		     min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (!mask->isFloat(comp) && mask->BitsOf(comp) <= 16) {
      MixDown<T,UWORD>(org,obytesperpixel,obytesperrow,
		     dst,dbytesperpixel,dbytesperrow,
		     (const UWORD *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		     min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (!mask->isFloat(comp) && mask->BitsOf(comp) <= 32) {
      MixDown<T,ULONG>(org,obytesperpixel,obytesperrow,
		     dst,dbytesperpixel,dbytesperrow,
		     (const ULONG *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		     min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (mask->BitsOf(comp) <= 32 && mask->isFloat(comp)) {
      MixDown<T,FLOAT>(org,obytesperpixel,obytesperrow,
		     dst,dbytesperpixel,dbytesperrow,
		     (const FLOAT *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		     min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else if (mask->BitsOf(comp) == 64 && mask->isFloat(comp)) {
      MixDown<T,DOUBLE>(org,obytesperpixel,obytesperrow,
		      dst,dbytesperpixel,dbytesperrow,
		      (const DOUBLE *)mask->DataOf(comp),mask->BytesPerPixel(comp),mask->BytesPerRow(comp),
		      min,max,mask->WidthOf(comp),mask->HeightOf(comp));
    } else {
      throw "unsupported data type";
    }
  }
}
///

/// Mask::Measure
// Implement the masking algorithm as an image filter between source and destination.
double Mask::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  struct ImgSpecs specs; // actually, these aren't used.
  class ImageLayout *mask = NULL;
  UWORD comp,depth,mdepth;

  try {
    mask = ImageLayout::LoadImage(m_pcMaskName,specs);
    //
    // Check the image dimensions.
    if (dst->WidthOf() != mask->WidthOf() || dst->HeightOf() != mask->HeightOf())
      throw "Image dimensions and mask dimensions did not match, cannot apply image mask";
    //
    // Depth of the mask can be either one, or equal to the depth of the source image.
    // That the destination image and the source image match has already been tested in the
    // main program.
    depth  = dst->DepthOf();
    mdepth = mask->DepthOf();
    if (mdepth != 1 && mdepth != depth)
      throw "The image mask must either have one component, or match the number of components of the images";
    //
    // Now process all components.
    for(comp = 0;comp < depth;comp++) {
      UWORD mcomp = comp;
      //
      // If the mask has only one component, use only this component.
      if (mcomp >= mdepth)
	mcomp = 0;
      //
      // Check again the dimensions, now within the component.
      if (dst->WidthOf(comp) != mask->WidthOf(mcomp) || dst->HeightOf(comp) != mask->HeightOf(mcomp))
	throw "Image dimensions and mask dimensions did not match, cannot apply image mask";
      //
      if (dst->isSigned(comp)) {
	if (dst->BitsOf(comp) <= 8) {
	  Blend<BYTE>((const BYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		      (BYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      mcomp,mask);
	} else if (!dst->isFloat(comp) && dst->BitsOf(comp) <= 16) {
	  Blend<WORD>((const WORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		      (WORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      mcomp,mask);
	} else if (!dst->isFloat(comp) && dst->BitsOf(comp) <= 32) {
	  Blend<LONG>((const LONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		      (LONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		      mcomp,mask);
	} else if (dst->BitsOf(comp) <= 32 && dst->isFloat(comp)) {
	  Blend<FLOAT>((const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		       (FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       mcomp,mask);
	} else if (dst->BitsOf(comp) == 64 && dst->isFloat(comp)) {
	  Blend<DOUBLE>((const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			(DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			mcomp,mask);
	} else {
	  throw "unsupported data type";
	}
      } else {
	if (dst->BitsOf(comp) <= 8) {
	  Blend<UBYTE>((const UBYTE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		       (UBYTE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       mcomp,mask);
	} else if (!dst->isFloat(comp) && dst->BitsOf(comp) <= 16) {
	  Blend<UWORD>((const UWORD *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		       (UWORD *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       mcomp,mask);
	} else if (!dst->isFloat(comp) && dst->BitsOf(comp) <= 32) {
	  Blend<ULONG>((const ULONG *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		       (ULONG *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       mcomp,mask);
	} else if (dst->BitsOf(comp) <= 32 && dst->isFloat(comp)) {
	  Blend<FLOAT>((const FLOAT *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
		       (FLOAT *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
		       mcomp,mask);
	} else if (dst->BitsOf(comp) == 64 && dst->isFloat(comp)) {
	  Blend<DOUBLE>((const DOUBLE *)dst->DataOf(comp),dst->BytesPerPixel(comp),dst->BytesPerRow(comp),
			(DOUBLE *)src->DataOf(comp),src->BytesPerPixel(comp),src->BytesPerRow(comp),
			mcomp,mask);
	} else {
	  throw "unsupported data type";
	}
      }
    }
  } catch(...) {
    delete mask;
    
    throw; // deliver exception upwards.
  }

  delete mask;
  return in;
}
///

