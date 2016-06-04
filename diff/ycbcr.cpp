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
** $Id: ycbcr.cpp,v 1.7 2016/06/04 10:44:09 thor Exp $
**
** This class converts between RGB and YCbCr signals
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/ycbcr.hpp"
#include "std/math.hpp"
///

/// YCbCr::ToYCbCr
template<typename S,typename T>
void YCbCr::ToYCbCr(S *r,S *g,S *b,
		    double yoffset,double coffset,
		    double min,double max,
		    double cmin,double cmax,
		    ULONG bppr,ULONG bppg,ULONG bppb,
		    ULONG bprr,ULONG bprg,ULONG bprb,
		    ULONG w, ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    S *rrow = r;
    S *grow = g;
    S *brow = b;
    for(x = 0;x < w;x++) {
      double y  = *rrow * 0.299    + *grow * 0.587    + *brow * 0.114    + yoffset;
      double cb = *rrow * -0.16875 + *grow * -0.33126 + *brow * 0.5      + coffset;
      double cr = *rrow * 0.5      + *grow * -0.41869 + *brow * -0.08131 + coffset;
      //
      // clip to range.
      y  = (y  <  min)?( min):(( y >  max)?( max):( y));
      cb = (cb < cmin)?(cmin):((cb > cmax)?(cmax):(cb));
      cr = (cr < cmin)?(cmin):((cr > cmax)?(cmax):(cr));
      //
      *rrow        = S(y);
      *((T *)grow) = T(cb);
      *((T *)brow) = T(cr);
      //
      rrow  = (S *)((UBYTE *)(rrow) + bppr);
      grow  = (S *)((UBYTE *)(grow) + bppg);
      brow  = (S *)((UBYTE *)(brow) + bppb);
    }
    r  = (S *)((UBYTE *)(r) + bprr);
    g  = (S *)((UBYTE *)(g) + bprg);
    b  = (S *)((UBYTE *)(b) + bprb);
  }
}
/// 
/// YCbCr::FromYCbCr
template<typename S,typename T>
void YCbCr::FromYCbCr(S *yp,T *cb,T *cr,
		      double yoffset,double coffset,
		      double min,double max,
		      ULONG bppy,ULONG bppcb,ULONG bppcr,
		      ULONG bpry,ULONG bprcb,ULONG bprcr,
		      ULONG w, ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    S *yrow  = yp;
    T *cbrow = cb;
    T *crrow = cr;
    for(x = 0;x < w;x++) {
      double r  = (*yrow - yoffset) + (*crrow - coffset)*1.402;
      double g  = (*yrow - yoffset) + (*cbrow - coffset)*-0.34413 + (*crrow - coffset)*-0.71414;
      double b  = (*yrow - yoffset) + (*cbrow - coffset)*1.772;
      //
      // clip to range.
      r  = (r < min)?(min):((r > max)?(max):(r));
      g  = (g < min)?(min):((g > max)?(max):(g));
      b  = (b < min)?(min):((b > max)?(max):(b));
      //
      *yrow         = S(r);
      *((S *)cbrow) = S(g);
      *((S *)crrow) = S(b);
      //
      yrow   = (S *)((UBYTE *)(yrow ) + bppy);
      cbrow  = (T *)((UBYTE *)(cbrow) + bppcb);
      crrow  = (T *)((UBYTE *)(crrow) + bppcr);
    }
    yp  = (S *)((UBYTE *)(yp) + bpry);
    cb  = (T *)((UBYTE *)(cb) + bprcb);
    cr  = (T *)((UBYTE *)(cr) + bprcr);
  }
}
/// 
/// YCbCr::ToYbCr
// Convert a single image to YCbCr.
void YCbCr::ToYCbCr(class ImageLayout *img)
{
  int i;
  double yoffset,coffset;
  double ymin,ymax;
  double cmin,cmax;
  bool issigned = img->isSigned(0);
  bool isfloat  = img->isFloat(0);
  UBYTE bits    = img->BitsOf(0);
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  
  if (img->DepthOf() != 3)
    throw "source image for YCbCr conversion must have exactly three components";

  for(i = 1;i < 3;i++) {
    if (img->WidthOf(i)  != w ||
	img->HeightOf(i) != h)
      throw "component dimensions differ, use --cup to upsample source image before conversion to YCbCr";
    if (img->BitsOf(i)   != bits)
      throw "bit depth of all image components must be identical for conversion to YCbCr";
    if (img->isSigned(i) != issigned)
      throw "signedness of all image components must be identical for conversion to YCbCr";
    if (img->isFloat(i)  != isfloat)
      throw "data types of all image components must be identical for conversion to YCbCr";
  }
  //
  // Find the conversion offsets.
  if (m_bMakeSigned) {
    if (isfloat) {
      yoffset = coffset = 0.0; // no offsets for conversion to float
      cmin    = ymin    = -HUGE_VAL;
      cmax    = ymax    =  HUGE_VAL; // no clipping
    } else {
      coffset = 0.0; // no offsets here.
      yoffset = (issigned)?(0):(0.5);
      ymin    = (issigned)?(-(QUAD(1) << (bits-1)))  :  (0);
      ymax    = (issigned)?( (QUAD(1) << (bits-1))-1):( (QUAD(1) << (bits))-1);
      cmin    = -(QUAD(1) << (bits-1));
      cmax    =  (QUAD(1) << (bits-1))-1;
    }
  } else {
    if (isfloat) {
      yoffset = 0.0;
      coffset = 0.5;
      cmin    = ymin    = -HUGE_VAL;
      cmax    = ymax    =  HUGE_VAL; // no clipping
    } else {
      yoffset = 0.5;
      coffset = 0.5 + ((issigned)?(0):(QUAD(1) << (bits-1)));
      ymin    = (issigned)?(-(QUAD(1) << (bits-1)))  :  (0);
      ymax    = (issigned)?( (QUAD(1) << (bits-1))-1):( (QUAD(1) << (bits))-1);
      cmin    = (issigned)?(-(QUAD(1) << (bits-1)))  :  (0);
      cmax    = (issigned)?( (QUAD(1) << (bits-1))-1):( (QUAD(1) << (bits))-1);
    }
  }
  //
  if (isfloat) {
    if (bits <= 32) {
      ToYCbCr<FLOAT,FLOAT>((FLOAT *)img->DataOf(0),(FLOAT *)img->DataOf(1),(FLOAT *)img->DataOf(2),
			   yoffset,coffset,ymin,ymax,cmin,cmax,
			   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			   w,h);
    } else if (bits == 64) {
      ToYCbCr<DOUBLE,DOUBLE>((DOUBLE *)img->DataOf(0),(DOUBLE *)img->DataOf(1),(DOUBLE *)img->DataOf(2),
			   yoffset,coffset,ymin,ymax,cmin,cmax,
			   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			   w,h);
    } else throw "unsupported source format";
  } else {
    if (bits <= 8) {
      if (issigned) {
	ToYCbCr<BYTE,BYTE>((BYTE *)img->DataOf(0),(BYTE *)img->DataOf(1),(BYTE *)img->DataOf(2),
			   yoffset,coffset,ymin,ymax,cmin,cmax,
			   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			   w,h);
      } else if (m_bMakeSigned) {
	ToYCbCr<UBYTE,BYTE>((UBYTE *)img->DataOf(0),(UBYTE *)img->DataOf(1),(UBYTE *)img->DataOf(2),
			   yoffset,coffset,ymin,ymax,cmin,cmax,
			   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			   w,h);
      } else {
	ToYCbCr<UBYTE,UBYTE>((UBYTE *)img->DataOf(0),(UBYTE *)img->DataOf(1),(UBYTE *)img->DataOf(2),
			     yoffset,coffset,ymin,ymax,cmin,cmax,
			     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			     w,h);
      }
    } else if (bits <= 16) {
      if (issigned) {
	ToYCbCr<WORD,WORD>((WORD *)img->DataOf(0),(WORD *)img->DataOf(1),(WORD *)img->DataOf(2),
			   yoffset,coffset,ymin,ymax,cmin,cmax,
			   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			   w,h);
      } else if (m_bMakeSigned) {
	ToYCbCr<UWORD,WORD>((UWORD *)img->DataOf(0),(UWORD *)img->DataOf(1),(UWORD *)img->DataOf(2),
			    yoffset,coffset,ymin,ymax,cmin,cmax,
			    img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			    img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			    w,h);
      } else {
	ToYCbCr<UWORD,UWORD>((UWORD *)img->DataOf(0),(UWORD *)img->DataOf(1),(UWORD *)img->DataOf(2),
			     yoffset,coffset,ymin,ymax,cmin,cmax,
			     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			     w,h);
      }
    } else if (bits <= 32) {
      if (issigned) {
	ToYCbCr<LONG,LONG>((LONG *)img->DataOf(0),(LONG *)img->DataOf(1),(LONG *)img->DataOf(2),
			   yoffset,coffset,ymin,ymax,cmin,cmax,
			   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			   w,h);
      } else if (m_bMakeSigned) {
	ToYCbCr<ULONG,LONG>((ULONG *)img->DataOf(0),(ULONG *)img->DataOf(1),(ULONG *)img->DataOf(2),
			    yoffset,coffset,ymin,ymax,cmin,cmax,
			    img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			    img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			    w,h);
      } else {
	ToYCbCr<ULONG,ULONG>((ULONG *)img->DataOf(0),(ULONG *)img->DataOf(1),(ULONG *)img->DataOf(2),
			     yoffset,coffset,ymin,ymax,cmin,cmax,
			     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			     w,h);
      }
    } else throw "unsupported source format";
  }

  if (m_bMakeSigned) {
    img->isSigned(1) = true;
    img->isSigned(2) = true;
  }
}
///
/// YCbCr::FromYbCr
// Convert a single image from YCbCr to signed or unsigned RGB.
void YCbCr::FromYCbCr(class ImageLayout *img)
{
  int i;
  double yoffset,coffset;
  double min,max;
  bool issigned = img->isSigned(0);
  bool isfloat  = img->isFloat(0);
  UBYTE bits    = img->BitsOf(0);
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  bool  wassigned = img->isSigned(1);
  
  if (img->DepthOf() != 3)
    throw "source image for YCbCr conversion must have exactly three components";

  if (issigned && !wassigned)
    throw "cannot convert signed luma and unsigned chroma to RGB";

  for(i = 1;i < 3;i++) {
    if (img->WidthOf(i)  != w ||
	img->HeightOf(i) != h)
      throw "component dimensions differ, use --cup to upsample source image before conversion to RGB";
    if (img->BitsOf(i)   != bits)
      throw "bit depth of all image components must be identical for conversion to RGB";
    if (img->isSigned(i) != wassigned)
      throw "signedness of all image components must be identical for conversion to RGB";
    if (img->isFloat(i)  != isfloat)
      throw "data types of all image components must be identical for conversion to RGB";
  }
  //
  // Find the conversion offsets.
  if (wassigned) {
    if (isfloat) {
      yoffset = coffset = 0.0; // no offsets for conversion to float
      min     = -HUGE_VAL;
      max     =  HUGE_VAL;     // no clipping
    } else {
      coffset = 0.0; // no offsets here.
      yoffset = (issigned)?(0.0):(-0.5);
      min     = (issigned)?(-(QUAD(1) << (bits-1)))  :(0);
      max     = (issigned)?( (QUAD(1) << (bits-1))-1):( (QUAD(1) << (bits))-1);
    }
  } else {
    if (isfloat) {
      yoffset = 0.0;
      coffset = 0.5;
      min     = -HUGE_VAL;
      max     =  HUGE_VAL; // no clipping
    } else {
      yoffset = -0.5;
      coffset = (issigned)?(0)                       :(QUAD(1) << (bits-1));
      min     = (issigned)?(-(QUAD(1) << (bits-1)))  :(0);
      max     = (issigned)?( (QUAD(1) << (bits-1))-1):( (QUAD(1) << (bits))-1);
    }
  }  
  //
  // Now run the conversion
  if (isfloat) {
    if (bits <= 32) {
      FromYCbCr<FLOAT,FLOAT>((FLOAT *)img->DataOf(0),(FLOAT *)img->DataOf(1),(FLOAT *)img->DataOf(2),
			     yoffset,coffset,min,max,
			     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			     w,h);
    } else if (bits == 64) {
      FromYCbCr<DOUBLE,DOUBLE>((DOUBLE *)img->DataOf(0),(DOUBLE *)img->DataOf(1),(DOUBLE *)img->DataOf(2),
			       yoffset,coffset,min,max,
			       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			       w,h);
    } else throw "unsupported source format";
  } else {
    if (bits <= 8) {
      if (issigned) {
	FromYCbCr<BYTE,BYTE>((BYTE *)img->DataOf(0),(BYTE *)img->DataOf(1),(BYTE *)img->DataOf(2),
			     yoffset,coffset,min,max,
			     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			     w,h);
      } else if (wassigned) {
	FromYCbCr<UBYTE,BYTE>((UBYTE *)img->DataOf(0),(BYTE *)img->DataOf(1),(BYTE *)img->DataOf(2),
			      yoffset,coffset,min,max,
			      img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			      img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			      w,h);
      } else {
	FromYCbCr<UBYTE,UBYTE>((UBYTE *)img->DataOf(0),(UBYTE *)img->DataOf(1),(UBYTE *)img->DataOf(2),
			       yoffset,coffset,min,max,
			       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			       w,h);
      }
    } else if (bits <= 16) {
      if (issigned) {
	FromYCbCr<WORD,WORD>((WORD *)img->DataOf(0),(WORD *)img->DataOf(1),(WORD *)img->DataOf(2),
			     yoffset,coffset,min,max,
			     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			     w,h);
      } else if (wassigned) {
	FromYCbCr<UWORD,WORD>((UWORD *)img->DataOf(0),(WORD *)img->DataOf(1),(WORD *)img->DataOf(2),
			      yoffset,coffset,min,max,
			      img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			      img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			      w,h);
      } else {
	FromYCbCr<UWORD,UWORD>((UWORD *)img->DataOf(0),(UWORD *)img->DataOf(1),(UWORD *)img->DataOf(2),
			       yoffset,coffset,min,max,
			       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			       w,h);
      }
    } else if (bits <= 32) {
      if (issigned) {
	FromYCbCr<LONG,LONG>((LONG *)img->DataOf(0),(LONG *)img->DataOf(1),(LONG *)img->DataOf(2),
			     yoffset,coffset,min,max,
			     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			     w,h);
      } else if (wassigned) {
	FromYCbCr<ULONG,LONG>((ULONG *)img->DataOf(0),(LONG *)img->DataOf(1),(LONG *)img->DataOf(2),
			      yoffset,coffset,min,max,
			      img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			      img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			      w,h);
      } else {
	FromYCbCr<ULONG,ULONG>((ULONG *)img->DataOf(0),(ULONG *)img->DataOf(1),(ULONG *)img->DataOf(2),
			       yoffset,coffset,min,max,
			       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			       w,h);
      }
    } else throw "unsupported source format";
  }  

  img->isSigned(1) = issigned;
  img->isSigned(2) = issigned;
}
///
/// YCbCr::Measure
double YCbCr::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  if (m_bInverse) {
    FromYCbCr(src);
    FromYCbCr(dst);
  } else {
    ToYCbCr(src);
    ToYCbCr(dst);
  }
  return in;
}
///

