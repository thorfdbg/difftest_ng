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
** $Id: sim2.cpp,v 1.2 2020/10/27 13:27:33 thor Exp $
**
** This class converts between XYZ and SIM2 signals
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/sim2.hpp"
#include "std/math.hpp"
///

/// Sim2::~Sim2
// Destroy the class, release all memory
Sim2::~Sim2(void)
{
  delete[] m_pucSrc;
  delete[] m_pucDst;
}
///

/// Sim2::XYZToSim2
// Conversion core.
template<typename S>
void Sim2::XYZToSim2(const S *x,const S *y,const S *z,
		     ULONG bppx,ULONG bppy,ULONG bppz,
		     ULONG bprx,ULONG bpry,ULONG bprz,
		     UBYTE *buf,
		     ULONG width,ULONG height)
{
  ULONG xp,yp;
  const double alpha = 0.0376;
  const double Lscale = 32.0;
  const double log2 = log(2.0);

  for(yp = 0;yp < height;yp++) {
    const S *xr = x;
    const S *yr = y;
    const S *zr = z;
    for(xp = 0; xp < width;xp += 2) {
      double l1 = 0.0;
      double u1 = 512.0;
      double v1 = 512.0;
      double l2 = 0.0;
      double u2 = 512.0;
      double v2 = 512.0;
      double u,v;
      ULONG l1int,l2int;
      ULONG uint,vint;
      ULONG lhint,llint;
      ULONG chint,clint;
      ULONG r_val,g_val,b_val;
      S x1_val = *xr;
      S y1_val = *yr;
      S z1_val = *zr;
      S x2_val = *xr;
      S y2_val = *yr;
      S z2_val = *zr;

      if(y1_val >= 0.0001) {
        l1 = (alpha * log(y1_val) / log2) + 0.5;
        u1 = (((1626.6875   * x1_val) / (x1_val + 15.0 * y1_val + 3.0 * z1_val)) + 0.546875) * 4.0;
        v1 = (((3660.046875 * y1_val) / (x1_val + 15.0 * y1_val + 3.0 * z1_val)) + 0.546875) * 4.0;
      }

      xr = (const S *)(((const UBYTE *)(xr)) + bppx);
      yr = (const S *)(((const UBYTE *)(yr)) + bppy);
      zr = (const S *)(((const UBYTE *)(zr)) + bppz);

      if (xp + 1 < width) {
	x2_val = *xr;
	y2_val = *yr;
	z2_val = *zr;
	if (y1_val >= 0.0001) {
	  l2 = (alpha * log(y2_val) / log2) + 0.5;
	  u2 = (((1626.6875   * x2_val) / (x2_val + 15.0 * y2_val + 3.0 * z2_val)) + 0.546875) * 4.0;
	  v2 = (((3660.046875 * y2_val) / (x2_val + 15.0 * y2_val + 3.0 * z2_val)) + 0.546875) * 4.0;
	}
      }
      
      xr = (const S *)(((const UBYTE *)(xr)) + bppx);
      yr = (const S *)(((const UBYTE *)(yr)) + bppy);
      zr = (const S *)(((const UBYTE *)(zr)) + bppz);

      if(y1_val > (2.0 * y2_val)) {
        u = u1;
        v = v1;
      } else if(y2_val > (2.0 * y1_val)) {
        u = u2;
        v = v2;
      } else {
        u = (u1 + u2) / 2.0;
        v = (v1 + v2) / 2.0;
      }

      l1 = round((253.0 * l1 + 1.0) * Lscale);
      if (l1 < 32.0) {
	l1 = 32.0;
      } else if (l1 > 8159.0) {
	l1 = 8159.0;
      }
      l1int = l1;

      l2 = round((253.0 * l2 + 1.0) * Lscale);
      if (l2 < 32.0) {
	l2 = 32.0;
      } else if (l2 > 8159.0) {
	l2 = 8159.0;
      }
      l2int = l2;

      u = round(u);
      if (u < 4.0) {
	u = 4.0;
      } else if (u > 1019.0) {
	u = 1019.0;
      }
      uint = u;

      v = round(v);
      if (v < 4.0) {
	v = 4.0;
      } else if (v > 1019.0) {
	v = 1019.0;
      }
      vint = v;

      lhint = l1int >> 5;
      llint = l1int & 0x1f;

      chint = vint >> 2;
      clint = vint & 0x03;

      r_val = (llint << 3) | (clint << 1);
      if (r_val < 1)   r_val = 1;
      if (r_val > 254) r_val = 254;
      g_val = lhint;
      b_val = chint;

      *buf++ = r_val;
      *buf++ = g_val;
      *buf++ = b_val;

      lhint = l2int >> 5;
      llint = l2int & 0x1f;

      chint = uint >> 2;
      clint = uint & 0x03;

      r_val = chint;
      g_val = lhint;
      b_val = (llint << 3) | (clint << 1);
      if (b_val > 254) b_val = 254;
      if (b_val < 1)   b_val = 1;

      *buf++ = r_val;
      *buf++ = g_val;
      *buf++ = b_val;
    }
    x = (const S *)(((const UBYTE *)(x)) + bprx);
    y = (const S *)(((const UBYTE *)(y)) + bpry);
    z = (const S *)(((const UBYTE *)(z)) + bprz);
  }
}
///

/// Sim2::Convert
// Convert a single image.
void Sim2::Convert(class ImageLayout *img,bool dst)
{
  int i;
  bool issigned = img->isSigned(0);
  bool isfloat  = img->isFloat(0);
  UBYTE bits    = img->BitsOf(0);
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  UBYTE *buf    = NULL;

  if (img->DepthOf() != 3)
    throw "source image for SIM2 conversion must have exactly three components";

  if (!isfloat)
    throw "source image for SIM2 conversion must be floating point";

  for(i = 1;i < 3;i++) {
    if (img->WidthOf(i)  != w ||
	img->HeightOf(i) != h)
      throw "source image cannot be subsampled for conversion to SIM2";
    if (img->BitsOf(i)   != bits)
      throw "bit depth of all image components must be identical for conversion to SIM2";
    if (img->isSigned(i) != issigned)
      throw "signedness of all image components must be identical for conversion to SIM2";
    if (img->isFloat(i)  != isfloat)
      throw "data types of all image components must be identical for conversion to SIM2";
  }
  //
  // Create the storage for the target image.
  CreateComponents(w,h,3);
  if (dst) {
    assert(m_pucDst == NULL);
    buf = m_pucDst = new UBYTE[w * h * 3];
  } else {
    assert(m_pucSrc == NULL);
    buf = m_pucSrc = new UBYTE[w * h * 3];
  }
  assert(buf);
  for(i = 0;i < 3;i++) {
    m_pComponent[i].m_ucBits  = 8;
    m_pComponent[i].m_bSigned = false;
    m_pComponent[i].m_bFloat  = false;
    m_pComponent[i].m_ucSubX  = 1;
    m_pComponent[i].m_ucSubY  = 1;
    m_pComponent[i].m_ulBytesPerPixel = 3;
    m_pComponent[i].m_ulBytesPerRow   = 3 * w;
    m_pComponent[i].m_pPtr    = buf + i;
  }
  //
  if (bits <= 32) {
    XYZToSim2<FLOAT>((FLOAT *)img->DataOf(0),(FLOAT *)img->DataOf(1),(FLOAT *)img->DataOf(2),
		     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		     buf,w,h);
  } else if (bits == 64) {
    XYZToSim2<DOUBLE>((DOUBLE *)img->DataOf(0),(DOUBLE *)img->DataOf(1),(DOUBLE *)img->DataOf(2),
		      img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		      img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		      buf,w,h);
  } else throw "unsupported source format";
}
///

/// Sim2::Measure
double Sim2::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  Convert(src,false);
  Swap(*src);
  Convert(dst,true);
  Swap(*dst);

  return in;
}
///

