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
** $Id: debayer.cpp,v 1.6 2019/03/01 10:15:56 thor Exp $
**
** This class runs a debayer filter on the image converting it from grey scale
** to RGB. This is mostly experimental.
*/

/// Includes
#include "diff/debayer.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// Debayer::~Debayer
Debayer::~Debayer(void)
{
  ReleaseComponents(m_ppucSource);
  ReleaseComponents(m_ppucDestination);
}
///

/// at
static int clip(int x,int max)
{
  max--;
  
  if (x < 0)
    x = -x;
  if (x > max)
    x = (max << 1) - x;
  if (x < 0)
    x = 0;
  if (x > max)
    x = max;

  return x;
}

template<typename T>
static T at(const T *src,LONG bytesperpixel,LONG bytesperrow,LONG x,LONG y,LONG w,LONG h)
{
  const UBYTE *p = (const UBYTE *)(src);

  x = clip(x,w);
  y = clip(y,h);

  p += x * bytesperpixel;
  p += y * bytesperrow;

  return *(const T*)(p);
}

#define AT(x,y) at<T>(src,bytesperpixel,bytesperrow,x,y,w,h)

// The gamma correction for cie.
static DOUBLE ciepow(DOUBLE x)
{
  if (x < 0.0)
    return 0.0;
  if (x > 0.008856)
    return pow(x,1.0/3.0);
  
  return 7.787*x + 16/116.0;
}

#define ABS(x) ((x) < 0)?(-(x)):(x)
#define SQR(x) ((x) * (x))
#define MIN(a,b) ((a) < (b))?(a):(b)
#define MAX(a,b) ((a) < (b))?(b):(a)
///

/// Debayer::ADHKernel
// The actual implementation of the ADH algorithm
template<typename T>
void Debayer::ADHKernel(const T *src,
			FLOAT *horr,FLOAT *verr,
			FLOAT *horg,FLOAT *verg,
			FLOAT *horb,FLOAT *verb,
			FLOAT *horl ,FLOAT *verl,
			FLOAT *horca,FLOAT *verca,
			FLOAT *horcb,FLOAT *vercb,
			LONG bytesperpixel,LONG bytesperrow,
			T *rp,T *gp,T *bp,FLOAT min,FLOAT max)
{
  LONG x,y,dy,dx;
  LONG w  = this->m_ulWidth;
  LONG h  = this->m_ulHeight;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;

  /*
  ** dcraw includes an additional affine scaling step here: it subtracts
  ** the black level (e.g. 512 for the FHG images) and scales the components
  ** by a component dependent value, (e.g. {12.009511,4.15093756,5.36335036} for the FHG images})
  */
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      // Step 1: Fill in the green pixels we have in the horizontal and vertical kernel.
      if (x + gx < w && y + gy < h) {
	horg[(x + gx) + w * (y + gy)] = AT(x + gx,y + gy);
	verg[(x + gx) + w * (y + gy)] = AT(x + gx,y + gy);
      }
      if (x + kx < w && y + ky < h) {
	horg[(x + kx) + w * (y + ky)] = AT(x + kx,y + ky);
	verg[(x + kx) + w * (y + ky)] = AT(x + kx,y + ky);
      }
      //
      // Filter green horizontally and vertically.
      if (x + gx < w && y + ky < h) {
	horg[(x + gx) + w * (y + ky)] = ((AT(x + gx - 1, y + ky) + AT(x + gx,y + ky) + AT(x + gx + 1,y + ky)) * 2.0 - AT(x + gx - 2, y + ky) - AT(x + gx + 2, y + ky)) / 4.0;
	verg[(x + gx) + w * (y + ky)] = ((AT(x + gx, y + ky - 1) + AT(x + gx,y + ky) + AT(x + gx,y + ky + 1)) * 2.0 - AT(x + gx, y + ky - 2) - AT(x + gx, y + ky + 2)) / 4.0;
	//dcraw also clamps the interpolated values between the two real values
      }
      //
      // Same for the alternative position.
      if (x + kx < w && y + gy < h) {
	horg[(x + kx) + w * (y + gy)] = ((AT(x + kx - 1, y + gy) + AT(x + kx,y + gy) + AT(x + kx + 1,y + gy)) * 2.0 - AT(x + kx - 2, y + gy) - AT(x + kx + 2, y + gy)) / 4.0;
	verg[(x + kx) + w * (y + gy)] = ((AT(x + kx, y + gy - 1) + AT(x + kx,y + gy) + AT(x + kx,y + gy + 1)) * 2.0 - AT(x + kx, y + gy - 2) - AT(x + kx, y + gy + 2)) / 4.0;
	//dcraw also clamps the interpolated values between the two real values
      }
    }
  }
  //
  // Now compute red and blue.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG ax,ay;
      //
      // Interpolate red at red sample positions
      if (x + rx < w && y + ry < h) {
	horr[(x + rx) + w * (y + ry)] = AT(x + rx, y + ry);
	verr[(x + rx) + w * (y + ry)] = AT(x + rx, y + ry);
      }
      //
      // Horizontal and vertical interpolation of red at rx ^ 1,ry, which is a green pixel.
      ax = rx ^ 1;
      if (x + ax < w && y + ry < h) {
	FLOAT nbs   = AT(x + ax - 1,y + ry) + AT(x + ax + 1,y + ry); // neighbouring red pixels.
	FLOAT ghor  = horg[clip(x + ax - 1,w) + w * (y + ry)] + horg[clip(x + ax + 1,w) + w * (y + ry)]; // interpolated green pixels left and right
	FLOAT gver  = verg[clip(x + ax - 1,w) + w * (y + ry)] + verg[clip(x + ax + 1,w) + w * (y + ry)];
	horr[(x + ax) + w * (y + ry)] = horg[(x + ax) + w * (y + ry)] - ghor / 2 + nbs / 2;
	verr[(x + ax) + w * (y + ry)] = verg[(x + ax) + w * (y + ry)] - gver / 2 + nbs / 2;
      }
      //
      ay = ry ^ 1;
      if (x + rx < w && y + ay < h) {
	FLOAT nbs   = AT(x + rx,y + ay - 1) + AT(x + rx,y + ay + 1); // interpolated from the pixels top and bottom.
	FLOAT ghor  = horg[x + rx + w * clip(y + ay - 1,h)] + horg[x + rx + w * clip(y + ay + 1,h)];
	FLOAT gver  = verg[x + rx + w * clip(y + ay - 1,h)] + verg[x + rx + w * clip(y + ay + 1,h)];
	horr[(x + rx) + w * (y + ay)] = horg[(x + rx) + w * (y + ay)] - ghor / 2 + nbs / 2;
	verr[(x + rx) + w * (y + ay)] = verg[(x + rx) + w * (y + ay)] - gver / 2 + nbs / 2;
      }
      //
      // Then diagonal.
      if (x + ax < w && y + ay < h) {
	FLOAT nbs   = AT(x + ax - 1,y + ay - 1) + AT(x + ax + 1,y + ay - 1) + AT(x + ax - 1,y + ay + 1) + AT(x + ax + 1,y + ay + 1); // the four surrounding red pixels.
	FLOAT ghor  = horg[clip(x + ax - 1,w) + w * clip(y + ay - 1,h)] + horg[clip(x + ax + 1,w) + w * clip(y + ay - 1,h)] +
	  horg[clip(x + ax - 1,w) + w * clip(y + ay + 1,h)] + horg[clip(x + ax + 1,w) + w * clip(y + ay + 1,h)];
	FLOAT gver  = verg[clip(x + ax - 1,w) + w * clip(y + ay - 1,h)] + verg[clip(x + ax + 1,w) + w * clip(y + ay - 1,h)] +
	  verg[clip(x + ax - 1,w) + w * clip(y + ay + 1,h)] + verg[clip(x + ax + 1,w) + w * clip(y + ay + 1,h)];
	horr[(x + ax) + w * (y + ay)] = horg[(x + ax) + w * (y + ay)] - ghor / 4 + nbs / 4;
	verr[(x + ax) + w * (y + ay)] = verg[(x + ax) + w * (y + ay)] - gver / 4 + nbs / 4;
      }
      //
      // Interpolate blue at blue sample positions
      if (x + bx < w && y + by < h) {
	horb[(x + bx) + w * (y + by)] = AT(x + bx, y + by);
	verb[(x + bx) + w * (y + by)] = AT(x + bx, y + by);
      }
      //
      // Horizontal and vertical interpolation of blue at bx ^ 1,by, which is a green pixel.
      ax = bx ^ 1;
      if (x + ax < w && y + by < h) {
	FLOAT nbs   = AT(x + ax - 1,y + by) + AT(x + ax + 1,y + by); // neighbouring red pixels.
	FLOAT ghor  = horg[clip(x + ax - 1,w) + w * (y + by)] + horg[clip(x + ax + 1,w) + w * (y + by)]; // interpolated green pixels left and right
	FLOAT gver  = verg[clip(x + ax - 1,w) + w * (y + by)] + verg[clip(x + ax + 1,w) + w * (y + by)];
	horb[(x + ax) + w * (y + by)] = horg[(x + ax) + w * (y + by)] - ghor / 2 + nbs / 2;
	verb[(x + ax) + w * (y + by)] = verg[(x + ax) + w * (y + by)] - gver / 2 + nbs / 2;
      }
      //
      ay = by ^ 1;
      if (x + bx < w && y + ay < h) {
	FLOAT nbs   = AT(x + bx,y + ay - 1) + AT(x + bx,y + ay + 1); // interpolated from the pixels top and bottom.
	FLOAT ghor  = horg[x + bx + w * clip(y + ay - 1,h)] + horg[x + bx + w * clip(y + ay + 1,h)];
	FLOAT gver  = verg[x + bx + w * clip(y + ay - 1,h)] + verg[x + bx + w * clip(y + ay + 1,h)];
	horb[(x + bx) + w * (y + ay)] = horg[(x + bx) + w * (y + ay)] - ghor / 2 + nbs / 2;
	verb[(x + bx) + w * (y + ay)] = verg[(x + bx) + w * (y + ay)] - gver / 2 + nbs / 2;
      }
      //
      // Then diagonal.
      if (x + ax < w && y + ay < h) {
	FLOAT nbs   = AT(x + ax - 1,y + ay - 1) + AT(x + ax + 1,y + ay - 1) + AT(x + ax - 1,y + ay + 1) + AT(x + ax + 1,y + ay + 1); // the four surrounding red pixels.
	FLOAT ghor  = horg[clip(x + ax - 1,w) + w * clip(y + ay - 1,h)] + horg[clip(x + ax + 1,w) + w * clip(y + ay - 1,h)] +
	  horg[clip(x + ax - 1,w) + w * clip(y + ay + 1,h)] + horg[clip(x + ax + 1,w) + w * clip(y + ay + 1,h)];
	FLOAT gver  = verg[clip(x + ax - 1,w) + w * clip(y + ay - 1,h)] + verg[clip(x + ax + 1,w) + w * clip(y + ay - 1,h)] +
	  verg[clip(x + ax - 1,w) + w * clip(y + ay + 1,h)] + verg[clip(x + ax + 1,w) + w * clip(y + ay + 1,h)];
	horb[(x + ax) + w * (y + ay)] = horg[(x + ax) + w * (y + ay)] - ghor / 4 + nbs / 4;
	verb[(x + ax) + w * (y + ay)] = verg[(x + ax) + w * (y + ay)] - gver / 4 + nbs / 4;
      }
    }
  }
  
  for(y = 0;y < h;y++) {
    for(x = 0;x < w;x++) {
      // Now horizontal and vertical interpolations are known. Convert from RGB to LAB.
      // This depends of course on the camera primaries, but for simplicity, we use the
      // same conversion matrix as in the original work.
      // 0.386275   0.334884   0.168971
      // 0.199173   0.703457   0.066264
      // 0.018107   0.118130   0.949690
      // In addition, we assume here a D65 white point. Also, scaling was not correct
      // in dcraw for the CIELab conversion.
      //
      // dcraw uses a camera depending conversion matrix, namely for the FHG images
      // 0.688983917  0.290616691  0.0203993637
      // 0.24259387   1.01152134   -0.254115313
      // 0.0101935146 -0.172740877 1.16254735
      // (this makes little sense as xyz coordinates are always positive)
      // and then adds an offset of (0.5,0.5,0.5)
      {
	FLOAT xh = ciepow((0.386275 * horr[x + w * y] + 0.334884 * horg[x + w * y] + 0.168971 * horb[x + w * y])/(0.95047 * max));
	FLOAT yh = ciepow((0.199173 * horr[x + w * y] + 0.703457 * horg[x + w * y] + 0.066264 * horb[x + w * y])/max);
	FLOAT zh = ciepow((0.018107 * horr[x + w * y] + 0.118130 * horg[x + w * y] + 0.949690 * horb[x + w * y])/(1.08833 * max));
	FLOAT xv = ciepow((0.386275 * verr[x + w * y] + 0.334884 * verg[x + w * y] + 0.168971 * verb[x + w * y])/(0.95047 * max));
	FLOAT yv = ciepow((0.199173 * verr[x + w * y] + 0.703457 * verg[x + w * y] + 0.066264 * verb[x + w * y])/max);
	FLOAT zv = ciepow((0.018107 * verr[x + w * y] + 0.118130 * verg[x + w * y] + 0.949690 * verb[x + w * y])/(1.08883 * max));
	// Convert from xyz to CIElab. dcraw includes an additional scaling factor of 64 here.
	horl[x + w * y]  = 116 * yh - 16; // L horizontal
	verl[x + w * y]  = 116 * yv - 16; // L vertical
	horca[x + w * y] = 500 * (xh - yh);
	verca[x + w * y] = 500 * (xv - yv);
	horcb[x + w * y] = 200 * (yh - zh);
	vercb[x + w * y] = 200 * (yv - zv);
      }
    }
  }

  //
  // Build homogenuity map and select the winning direction.
  // Quite like the dcraw implementation, there is no median filter.
  for(y = 0;y < h;y++) {
    for(x = 0;x < w;x++) {
      int homhor = 0;
      int homver = 0;
      for(dy = y - 1; dy <= y + 1;dy++) {
	for (dx = x - 1; dx <= x + 1;dx++) {
	  FLOAT ldiffhorn = ABS(horl[clip(dx,w) + w * clip(dy,h)] - horl[clip(dx,w) + w * clip(dy - 1,h)]);
	  FLOAT ldiffvern = ABS(verl[clip(dx,w) + w * clip(dy,h)] - verl[clip(dx,w) + w * clip(dy - 1,h)]);
	  FLOAT ldiffhors = ABS(horl[clip(dx,w) + w * clip(dy,h)] - horl[clip(dx,w) + w * clip(dy + 1,h)]);
	  FLOAT ldiffvers = ABS(verl[clip(dx,w) + w * clip(dy,h)] - verl[clip(dx,w) + w * clip(dy + 1,h)]);
	  FLOAT ldiffhorw = ABS(horl[clip(dx,w) + w * clip(dy,h)] - horl[clip(dx - 1,w) + w * clip(dy,h)]);
	  FLOAT ldiffverw = ABS(verl[clip(dx,w) + w * clip(dy,h)] - verl[clip(dx - 1,w) + w * clip(dy,h)]);
	  FLOAT ldiffhore = ABS(horl[clip(dx,w) + w * clip(dy,h)] - horl[clip(dx + 1,w) + w * clip(dy,h)]);
	  FLOAT ldiffvere = ABS(verl[clip(dx,w) + w * clip(dy,h)] - verl[clip(dx + 1,w) + w * clip(dy,h)]);
	  FLOAT cdiffhorn = SQR(horca[clip(dx,w) + w * clip(dy,h)] - horca[clip(dx,w) + w * clip(dy - 1,h)]) +
	    SQR(horcb[clip(dx,w) + w * clip(dy,h)] - horcb[clip(dx,w) + w * clip(dy - 1,h)]);
	  FLOAT cdiffvern = SQR(verca[clip(dx,w) + w * clip(dy,h)] - verca[clip(dx,w) + w * clip(dy - 1,h)]) +
	    SQR(vercb[clip(dx,w) + w * clip(dy,h)] - vercb[clip(dx,w) + w * clip(dy - 1,h)]);
	  FLOAT cdiffhors = SQR(horca[clip(dx,w) + w * clip(dy,h)] - horca[clip(dx,w) + w * clip(dy + 1,h)]) +
	    SQR(horcb[clip(dx,w) + w * clip(dy,h)] - horcb[clip(dx,w) + w * clip(dy + 1,h)]);
	  FLOAT cdiffvers = SQR(verca[clip(dx,w) + w * clip(dy,h)] - verca[clip(dx,w) + w * clip(dy + 1,h)]) +
	    SQR(vercb[clip(dx,w) + w * clip(dy,h)] - vercb[clip(dx,h) + w * clip(dy + 1,h)]);
	  FLOAT cdiffhorw = SQR(horca[clip(dx,w) + w * clip(dy,h)] - horca[clip(dx - 1,w) + w * clip(dy,h)]) +
	    SQR(horcb[clip(dx,w) + w * clip(dy,h)] - horcb[clip(dx - 1,w) + w * clip(dy,h)]);
	  FLOAT cdiffverw = SQR(verca[clip(dx,w) + w * clip(dy,h)] - verca[clip(dx - 1,w) + w * clip(dy,h)]) +
	    SQR(vercb[clip(dx,w) + w * clip(dy,h)] - vercb[clip(dx - 1,w) + w * clip(dy,h)]);
	  FLOAT cdiffhore = SQR(horca[clip(dx,w) + w * clip(dy,h)] - horca[clip(dx + 1,w) + w * clip(dy,h)]) +
	    SQR(horcb[clip(dx,w) + w * clip(dy,h)] - horcb[clip(dx + 1,w) + w * clip(dy,h)]);
	  FLOAT cdiffvere = SQR(verca[clip(dx,w) + w * clip(dy,h)] - verca[clip(dx + 1,w) + w * clip(dy,h)]) +
	    SQR(vercb[clip(dx,w) + w * clip(dy,h)] - vercb[clip(dx + 1,w) + w * clip(dy,h)]);
	  FLOAT leps      = MIN(MAX(ldiffhorw,ldiffhore),MAX(ldiffvern,ldiffvers));
	  FLOAT ceps      = MIN(MAX(cdiffhorw,cdiffhore),MAX(cdiffvern,cdiffvers));
	  homhor         += (ldiffhorn <= leps && cdiffhorn <= ceps) + (ldiffhors <= leps && cdiffhors <= ceps) +
	    (ldiffhorw <= leps && cdiffhorw <= ceps) + (ldiffhore <= leps && cdiffhore <= ceps);
	  homver         += (ldiffvern <= leps && cdiffvern <= ceps) + (ldiffvers <= leps && cdiffvers <= ceps) +
	    (ldiffverw <= leps && cdiffverw <= ceps) + (ldiffvere <= leps && cdiffvere <= ceps);
	}
      }
      FLOAT r,g,b;
      /*
      ** the dcraw version averages over a 3x3 neighbourhood, this algorithm
      ** only checks a single value
      */
      if (homhor > homver) {
	r = horr[x + y * w];
	g = horg[x + y * w];
	b = horb[x + y * w];
      } else if (homhor < homver) {
	r = verr[x + y * w];
	g = verg[x + y * w];
	b = verb[x + y * w];
      } else {
	r = (horr[x + y * w] + verr[x + y * w]) / 2;
	g = (horg[x + y * w] + verg[x + y * w]) / 2;
	b = (horb[x + y * w] + verb[x + y * w]) / 2;
      }
      if (r < min) r = min;
      if (r > max) r = max;
      if (g < min) g = min;
      if (g > max) g = max;
      if (b < min) b = min;
      if (b > max) b = max;
      
      rp[x + y * w] = r;
      gp[x + y * w] = g;
      bp[x + y * w] = b;
    }
  }
}
///

/// Debayer::BilinearKernel
// The actual debayer algorithm.
template<typename T,typename S>
void Debayer::BilinearKernel(const T *src,LONG bytesperpixel,LONG bytesperrow,
			     T *r,T *g,T *b,S min,S max)
{
  LONG x,y;
  LONG w  = this->m_ulWidth;
  LONG h  = this->m_ulHeight;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;

  for(y = 0;y < h;y += 2) {
    T *rrow = r;
    T *grow = g;
    T *brow = b;
    for(x = 0;x < w;x += 2) {
      LONG ax,ay;
      // Step 1: Fill in the green pixels we have.
      if (x + gx < w && y + gy < h)
	grow[gx + w * gy] = AT(x + gx,y + gy);
      if (x + kx < w && y + ky < h)
	grow[kx + w * ky] = AT(x + kx,y + ky);
      //
      // Estimate g in the (gx,ky) corner
      if (x + gx < w && y + ky < h) {
	S vw  = AT(x + gx - 1 ,y + ky);
	S vn  = AT(x + gx     ,y + ky - 1);
	S ve  = AT(x + gx + 1,y + ky);
	S vs  = AT(x + gx     ,y + ky + 1);
	S cn  = (vw + vn + ve + vs) / 4;
	// Clamp.
	if (cn < min) cn = min;
	if (cn > max) cn = max;
	grow[gx + w * ky] = cn;
      }
      //
      // Estimate g in the (kx,gy) corner
      if (x + kx < w && y + gy < h) {
	// calculate horizontal gradient
	S vw = AT(x + kx - 1,y + gy);
	S vn = AT(x + kx    ,y + gy - 1);
	S ve = AT(x + kx + 1,y + gy);
	S vs = AT(x + kx    ,y + gy + 1);
	S cn = (vw + vn + ve + vs) / 4;
	// Clamp.
	if (cn < min) cn = min;
	if (cn > max) cn = max;
	grow[kx + w * gy] = cn;
      }
      //
      // Green is done. Fill in the red channel.
      if (x + rx < w && y + ry < h)
	rrow[rx + w * ry] = AT(x + rx,y + ry);
      //
      // Horizontal interpolation of red at rx ^ 1,ry
      ax = rx ^ 1;
      if (x + ax < w && y + ry < h) {
	S ve = AT(x + ax - 1,y + ry);
	S vw = AT(x + ax + 1,y + ry);
	S cn = (ve + vw) / 2;
	if (cn < min) cn  = min;
	if (cn > max) cn  = max;
	rrow[ax + w * ry] = cn;
      }
      // Vertical interpolation of red at rx,ry ^ 1
      ay = ry ^ 1;
      if (x + rx < w && y + ay < h) {
	S vn = AT(x + rx,y + ay - 1);
	S vs = AT(x + rx,y + ay + 1);
	S cn = (vn + vs) / 2;
	if (cn < min) cn  = min;
	if (cn > max) cn  = max;
	rrow[rx + w * ay] = cn;
      }
      //
      // Horizontal and vertical interpolation at ax,ay.
      if (x + ax < w && y + ay < h) {
	S vnw = AT(x + ax - 1,y + ay - 1);
	S vne = AT(x + ax + 1,y + ay - 1);
	S vsw = AT(x + ax - 1,y + ay + 1);
	S vse = AT(x + ax + 1,y + ay + 1);
	S cn  = (vnw + vne + vsw + vse) / 4;
	if (cn < min) cn  = min;
	if (cn > max) cn  = max;
	rrow[ax + w * ay] = cn;
      }
      //
      // Red is done. Fill in the blue channel.
       if (x + bx < w && y + by < h)
	brow[bx + w * by] = AT(x + bx,y + by);
      //
      // Horizontal interpolation of blue at rx ^ 1,ry
      ax = bx ^ 1;
      if (x + ax < w && y + by < h) {
	S ve = AT(x + ax - 1,y + by);
	S vw = AT(x + ax + 1,y + by);
	S cn = (ve + vw) / 2;
	if (cn < min) cn  = min;
	if (cn > max) cn  = max;
	brow[ax + w * by] = cn;
      }
      // Vertical interpolation of blue at rx,ry ^ 1
      ay = by ^ 1;
      if (x + bx < w && y + ay < h) {
	S vn = AT(x + bx,y + ay - 1);
	S vs = AT(x + bx,y + ay + 1);
	S cn = (vn + vs) / 2;
	if (cn < min) cn  = min;
	if (cn > max) cn  = max;
	brow[bx + w * ay] = cn;
      }
      //
      // Horizontal and vertical interpolation at ax,ay.
      if (x + ax < w && y + ay < h) {
	S vnw = AT(x + ax - 1,y + ay - 1);
	S vne = AT(x + ax + 1,y + ay - 1);
	S vsw = AT(x + ax - 1,y + ay + 1);
	S vse = AT(x + ax + 1,y + ay + 1);
	S cn  = (vnw + vne + vsw + vse) / 4;
	if (cn < min) cn  = min;
	if (cn > max) cn  = max;
	brow[ax + w * ay] = cn;
      }
      //
      // Increment the buffer positions.
      rrow += 2;
      grow += 2;
      brow += 2;
    }
    // Advance to the next row.
    r += w << 1;
    g += w << 1;
    b += w << 1;
  }
}
///

/// Debayer::ReleaseComponents
// Release the memory for the target components
// that have been allocated.
void Debayer::ReleaseComponents(UBYTE **&p)
{
   int i;

  if (p) {
    for(i = 0;i < m_usAllocated;i++) {
      if (p[i])
        delete[] p[i];
    }
    delete[] p;
    p = NULL;
  }
}
///

/// Debayer::CreateImageData
// Create the image data from the dimensions computed
void Debayer::CreateImageData(UBYTE **&data,class ImageLayout *src)
{
  UWORD i;
  //
  assert(m_pComponent == NULL);
  assert(data == NULL);
  //
  // Allocate the component data pointers.
  m_ulWidth    = src->WidthOf();
  m_ulHeight   = src->HeightOf();
  m_usDepth    = 3;
  m_pComponent = new struct ComponentLayout[m_usDepth];
  //
  // Initialize the component dimensions.
  for(i = 0;i < m_usDepth;i++) {
    m_pComponent[i].m_ulWidth  = m_ulWidth;
    m_pComponent[i].m_ulHeight = m_ulHeight;
    m_pComponent[i].m_ucBits   = src->BitsOf(0);
    m_pComponent[i].m_bSigned  = src->isSigned(0);
    m_pComponent[i].m_ucSubX   = src->SubXOf(0);
    m_pComponent[i].m_ucSubY   = src->SubYOf(0);
  }
  //
  // Allocate the component pointers.
  data = new UBYTE *[m_usDepth];
  m_usAllocated = m_usDepth;
  memset(data,0,sizeof(UBYTE *) * m_usDepth);
  //
  // Fill up the component data pointers.
  for(i = 0;i < m_usDepth;i++) {
    UBYTE bps = ImageLayout::SuggestBPP(m_pComponent[i].m_ucBits,m_pComponent[i].m_bFloat);
    //
    data[i]                           = new UBYTE[m_pComponent[i].m_ulWidth * m_pComponent[i].m_ulHeight * bps];
    m_pComponent[i].m_ulBytesPerPixel = bps;
    m_pComponent[i].m_ulBytesPerRow   = bps * m_pComponent[i].m_ulWidth;
    m_pComponent[i].m_pPtr            = data[i];
  }
}
///

/// Debayer::ADHInterpolate
// This is a smarter interpolation ported from dcraw, which again
// uses the algorithm from Keigo Hirakawa and Thomas W. Parks:
// "Adapaptive Homogeneity directed demosaicing algorithm"
void Debayer::ADHInterpolate(UBYTE **&dest,class ImageLayout *src)
{
  FLOAT *horr = NULL,*verr = NULL;
  FLOAT *horg = NULL,*verg = NULL;
  FLOAT *horb = NULL,*verb = NULL;
  FLOAT *horl  = NULL,*verl  = NULL;
  FLOAT *horca = NULL,*verca = NULL;
  FLOAT *horcb = NULL,*vercb = NULL;
  //
  // Delete the old data
  delete[] m_pComponent;
  m_pComponent = NULL;
  ReleaseComponents(dest);
  
  if (src->DepthOf() != 1)
    throw "Source image to be de-mosaiked must have only one component";

  try {
    ULONG size = src->WidthOf() * src->HeightOf();

    horr  = new FLOAT[size];
    verr  = new FLOAT[size];
    horg  = new FLOAT[size];
    verg  = new FLOAT[size];
    horb  = new FLOAT[size];
    verb  = new FLOAT[size];
    horl  = new FLOAT[size];
    verl  = new FLOAT[size];
    horca = new FLOAT[size];
    verca = new FLOAT[size];
    horcb = new FLOAT[size];
    vercb = new FLOAT[size];
    
    CreateImageData(dest,src);
    
    if (src->isFloat(0)) {
      if (src->isSigned(0))
	throw "the ADH algorithm is not defined for signed pixel values";
      switch(src->BitsOf(0)) {
      case 16:
      case 32:
	ADHKernel<FLOAT>((FLOAT *)(src->DataOf(0)),
			 horr,verr,horg,verg,horb,verb,
			 horl,verl,horca,verca,horcb,vercb,
			 src->BytesPerPixel(0),src->BytesPerRow(0),
			 (FLOAT *)dest[0],(FLOAT *)dest[1],(FLOAT *)dest[2],-HUGE_VAL,HUGE_VAL);
	break;
      case 64:
	ADHKernel<DOUBLE>((DOUBLE *)(src->DataOf(0)),
			  horr,verr,horg,verg,horb,verb,
			  horl,verl,horca,verca,horcb,vercb,
			  src->BytesPerPixel(0),src->BytesPerRow(0),
			  (DOUBLE *)dest[0],(DOUBLE *)dest[1],(DOUBLE *)dest[2],-HUGE_VAL,HUGE_VAL);
      break;
      default:
	throw "unsupported source pixel type";
	break;
      }
    } else {
      if (src->isSigned(0)) {
	throw "the ADH algorithm is not defined for signed pixel values";
      } else {
	FLOAT min = 0;
	FLOAT max = +(UQUAD(1) << (src->BitsOf(0))) - 1;
	switch((src->BitsOf(0) + 7) & -8) {
	case 8:
	  ADHKernel<UBYTE>((UBYTE *)(src->DataOf(0)),
			   horr,verr,horg,verg,horb,verb,
			   horl,verl,horca,verca,horcb,vercb,
			   src->BytesPerPixel(0),src->BytesPerRow(0),
			   (UBYTE *)dest[0],(UBYTE *)dest[1],(UBYTE *)dest[2],min,max);
	  break;
	case 16:
	  ADHKernel<UWORD>((UWORD *)(src->DataOf(0)),
			   horr,verr,horg,verg,horb,verb,
			   horl,verl,horca,verca,horcb,vercb,
			   src->BytesPerPixel(0),src->BytesPerRow(0),
			   (UWORD *)dest[0],(UWORD *)dest[1],(UWORD *)dest[2],min,max);
	  break;
	case 32:
	  ADHKernel<ULONG>((ULONG *)(src->DataOf(0)),
			   horr,verr,horg,verg,horb,verb,
			   horl,verl,horca,verca,horcb,vercb,
			   src->BytesPerPixel(0),src->BytesPerRow(0),
			   (ULONG *)dest[0],(ULONG *)dest[1],(ULONG *)dest[2],min,max);
	  break;
	case 64:
	  ADHKernel<UQUAD>((UQUAD *)(src->DataOf(0)),
			   horr,verr,horg,verg,horb,verb,
			   horl,verl,horca,verca,horcb,vercb,
			   src->BytesPerPixel(0),src->BytesPerRow(0),
			   (UQUAD *)dest[0],(UQUAD *)dest[1],(UQUAD *)dest[2],min,max);
	  break;
	default:
	  throw "unsupported source pixel type";
	  break;
	}
      }
    }
    //
    Swap(*src);
    delete[] horr;
    delete[] verr;
    delete[] horg;
    delete[] verg;
    delete[] horb;
    delete[] verb;
    delete[] horl;
    delete[] verl;
    delete[] horca;
    delete[] verca;
    delete[] horcb;
    delete[] vercb;
  } catch(...) {
    delete[] horr;
    delete[] verr;
    delete[] horg;
    delete[] verg;
    delete[] horb;
    delete[] verb;
    delete[] horl;
    delete[] verl;
    delete[] horca;
    delete[] verca;
    delete[] horcb;
    delete[] vercb;
    throw;
  }
}
///

/// Debayer::BilinearInterpolate
// Bilinear interpolation
void Debayer::BilinearInterpolate(UBYTE **&dest,class ImageLayout *src)
{
  //
  // Delete the old data
  delete[] m_pComponent;
  m_pComponent = NULL;
  ReleaseComponents(dest);
  
  if (src->DepthOf() != 1)
    throw "Source image to be de-mosaiked must have only one component";

  CreateImageData(dest,src);
  
  if (src->isFloat(0)) {
    switch(src->BitsOf(0)) {
    case 16:
    case 32:
      BilinearKernel<FLOAT,DOUBLE>((FLOAT *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				   (FLOAT *)dest[0],(FLOAT *)dest[1],(FLOAT *)dest[2],-HUGE_VAL,HUGE_VAL);
      break;
    case 64:
      BilinearKernel<DOUBLE,DOUBLE>((DOUBLE *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				    (DOUBLE *)dest[0],(DOUBLE *)dest[1],(DOUBLE *)dest[2],-HUGE_VAL,HUGE_VAL);
      break;
    default:
      throw "unsupported source pixel type";
      break;
    }
  } else {
    if (src->isSigned(0)) {
      QUAD min = -(QUAD(1) << (src->BitsOf(0)-1));
      QUAD max = +(QUAD(1) << (src->BitsOf(0)-1)) - 1;
      switch((src->BitsOf(0) + 7) & -8) {
      case 8:
	BilinearKernel<BYTE,WORD>((BYTE *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				  (BYTE *)dest[0],(BYTE *)dest[1],(BYTE *)dest[2],min,max);
	break;
      case 16:
	BilinearKernel<WORD,LONG>((WORD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				  (WORD *)dest[0],(WORD *)dest[1],(WORD *)dest[2],min,max);
	break;
      case 32:
	BilinearKernel<LONG,QUAD>((LONG *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				  (LONG *)dest[0],(LONG *)dest[1],(LONG *)dest[2],min,max);
	break;
      case 64:
	BilinearKernel<QUAD,QUAD>((QUAD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				  (QUAD *)dest[0],(QUAD *)dest[1],(QUAD *)dest[2],min,max);
	break;
      default:
	throw "unsupported source pixel type";
	break;
      }
    } else {
      UQUAD min = 0;
      UQUAD max = +(UQUAD(1) << (src->BitsOf(0))) - 1;
      switch((src->BitsOf(0) + 7) & -8) {
      case 8:
	BilinearKernel<UBYTE,WORD>((UBYTE *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				   (UBYTE *)dest[0],(UBYTE *)dest[1],(UBYTE *)dest[2],min,max);
	break;
      case 16:
	BilinearKernel<UWORD,LONG>((UWORD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				   (UWORD *)dest[0],(UWORD *)dest[1],(UWORD *)dest[2],min,max);
	break;
      case 32:
	BilinearKernel<ULONG,QUAD>((ULONG *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				   (ULONG *)dest[0],(ULONG *)dest[1],(ULONG *)dest[2],min,max);
	break;
      case 64:
	BilinearKernel<UQUAD,UQUAD>((UQUAD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),
				    (UQUAD *)dest[0],(UQUAD *)dest[1],(UQUAD *)dest[2],min,max);
	break;
      default:
	throw "unsupported source pixel type";
	break;
      }
    }
  }
  //
  Swap(*src);
}
///

/// Debayer::Measure
double Debayer::Measure(class ImageLayout *src,class ImageLayout *dest,double in)
{
  switch(m_Method) {
  case Bilinear:
    BilinearInterpolate(m_ppucSource,src);
    BilinearInterpolate(m_ppucDestination,dest);
    break;
  case ADH:
    ADHInterpolate(m_ppucSource,src);
    ADHInterpolate(m_ppucDestination,dest);
    break;
  }
  
  return in;
}
///
