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
** $Id: xyz.cpp,v 1.5 2017/01/31 11:58:04 thor Exp $
**
** This class converts between RGB and YCbCr signals
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/xyz.hpp"
#include "std/math.hpp"
///

/// XYZ::Multiply
template<typename S>
void XYZ::Multiply(S *r,S *g,S *b,
		   double min,double max,
		   ULONG bppr,ULONG bppg,ULONG bppb,
		   ULONG bprr,ULONG bprg,ULONG bprb,
		   ULONG w, ULONG h,
		   const double matrix[9])
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    S *rrow = r;
    S *grow = g;
    S *brow = b;
    for(x = 0;x < w;x++) {
      double xc = *rrow * matrix[0] + *grow * matrix[1] + *brow * matrix[2];
      double yc = *rrow * matrix[3] + *grow * matrix[4] + *brow * matrix[5];
      double zc = *rrow * matrix[6] + *grow * matrix[7] + *brow * matrix[8];
      //
      // clip to range.
      xc  = (xc  <  min)?( min):(( xc >  max)?( max):( xc));
      yc  = (yc  <  min)?( min):(( yc >  max)?( max):( yc));
      zc  = (zc  <  min)?( min):(( zc >  max)?( max):( zc));
      //
      *rrow        = S(xc);
      *grow        = S(yc);
      *brow        = S(zc);
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

/// XYZ::Multiply
// Convert a single image.
void XYZ::Multiply(class ImageLayout *img)
{
  int i;
  double min,max;
  bool issigned = img->isSigned(0);
  bool isfloat  = img->isFloat(0);
  UBYTE bits    = img->BitsOf(0);
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  const double *matrix = NULL;

  if (m_bInverse) {
    switch(m_Direction) {
    case RGBtoXYZ:
      {
	static const double m[] = {+3.2404542,-1.5371385,-0.4985314,
				   -0.9692660,+1.8760108,+0.0415560,
				   +0.0556434,-0.2040259,+1.0570000};
	matrix = m;
      }
      break;
    case XYZtoLMS:
      {
	static const double m[] = {+1.8600666,-1.1294801,+0.2198983,
				   +0.3612229,+0.6388043,-0.0000071,
				   0,         0,+1.0890873};
	matrix = m;
      }
      break;
    case RGBtoLMS:
      {
	static const double m[] = {+5.4722121,-4.6419601,+0.16963708,
				   -1.1252419,+2.2931709,-0.16789520,
				   +0.0298017,-0.1931807,+1.16364789};
	matrix = m;
      }
      break;
    }
  } else {
    switch(m_Direction) {
    case RGBtoXYZ:
      {
	static const double m[] = {+0.4124564,+0.3575761,+0.1804375,
				   +0.2126729,+0.7151522,+0.0721750,
				   +0.0193339,+0.1191920,+0.9503041};
	matrix = m;
      }
      break;
    case XYZtoLMS:
      {
	static const double m[] = {+0.4002,+0.7076,-0.0808,
				   -0.2263,+1.1653,+0.0457,
				   0,      0,+0.9182};
	matrix = m;
      }
      break;
    case RGBtoLMS:
      {
	static const double m[] = {+0.3139902,+0.6395129,+0.0464975,
				   +0.1553724,+0.7578945,+0.0867014,
				   +0.0177524,+0.1094421,+0.8725692};
	matrix = m;
      }
      break;
    }
  }
  
  if (matrix == NULL)
    throw "unsupported conversion";
  
  if (img->DepthOf() != 3)
    throw "source image for XYZ conversion must have exactly three components";

  for(i = 1;i < 3;i++) {
    if (img->WidthOf(i)  != w ||
	img->HeightOf(i) != h)
      throw "source image cannot be subsampled for conversion to XYZ";
    if (img->BitsOf(i)   != bits)
      throw "bit depth of all image components must be identical for conversion to XYZ";
    if (img->isSigned(i) != issigned)
      throw "signedness of all image components must be identical for conversion to XYZ";
    if (img->isFloat(i)  != isfloat)
      throw "data types of all image components must be identical for conversion to XYZ";
  }
  //
  // Find the conversion offsets.
  if (isfloat) {
    min  = -HUGE_VAL;
    max  =  HUGE_VAL; // no clipping
  } else {
    min    = (issigned)?(-(QUAD(1) << (bits-1)))  :  (0);
    max    = (issigned)?( (QUAD(1) << (bits-1))-1):( (QUAD(1) << (bits))-1);
  }
  //
  if (isfloat) {
    if (bits <= 32) {
      Multiply<FLOAT>((FLOAT *)img->DataOf(0),(FLOAT *)img->DataOf(1),(FLOAT *)img->DataOf(2),
		      min,max,
		      img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		      img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		      w,h,matrix);
    } else if (bits == 64) {
      Multiply<DOUBLE>((DOUBLE *)img->DataOf(0),(DOUBLE *)img->DataOf(1),(DOUBLE *)img->DataOf(2),
		       min,max,
		       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		       w,h,matrix);
    } else throw "unsupported source format";
  } else {
    if (bits <= 8) {
      if (issigned) {
	Multiply<BYTE>((BYTE *)img->DataOf(0),(BYTE *)img->DataOf(1),(BYTE *)img->DataOf(2),
		       min,max,
		       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		       w,h,matrix);
      } else {
	Multiply<UBYTE>((UBYTE *)img->DataOf(0),(UBYTE *)img->DataOf(1),(UBYTE *)img->DataOf(2),
			min,max,
			img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			w,h,matrix);
      }
    } else if (bits <= 16) {
      if (issigned) {
	Multiply<WORD>((WORD *)img->DataOf(0),(WORD *)img->DataOf(1),(WORD *)img->DataOf(2),
		       min,max,
		       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		       w,h,matrix);
      } else {
	Multiply<UWORD>((UWORD *)img->DataOf(0),(UWORD *)img->DataOf(1),(UWORD *)img->DataOf(2),
			min,max,
			img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			w,h,matrix);
      }
    } else if (bits <= 32) {
      if (issigned) {
	Multiply<LONG>((LONG *)img->DataOf(0),(LONG *)img->DataOf(1),(LONG *)img->DataOf(2),
		       min,max,
		       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		       w,h,matrix);
      } else {
	Multiply<ULONG>((ULONG *)img->DataOf(0),(ULONG *)img->DataOf(1),(ULONG *)img->DataOf(2),
			min,max,
			img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
			img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
			w,h,matrix);
      }
    } else throw "unsupported source format";
  }
}
///

/// XYZ::Measure
double XYZ::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  Multiply(src);
  Multiply(dst);

  return in;
}
///

