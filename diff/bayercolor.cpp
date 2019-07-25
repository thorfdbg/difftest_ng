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
** $Id: bayercolor.cpp,v 1.6 2019/03/06 14:38:31 thor Exp $
**
** This class implements various color transformations on CFA data,
** specific to CFA only. They generally depend on the Bayer layout,
** unlike the simpler transformations in the YCbCr class.
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "std/string.hpp"
#include "diff/bayercolor.hpp"
#include "img/imgspecs.hpp"
#include "std/assert.hpp"
///

/// BayerColor::~BayerColor
BayerColor::~BayerColor(void)
{
  delete[] m_pucSrcImage;
  delete[] m_pucDstImage;
}
///

/// Access/At function
template<typename T>
static inline T At(const T *in,LONG x,LONG y,LONG w,LONG h,LONG bpp,LONG bpr)
{
  if (x < 0)
    x = -x;
  if (x >= w)
    x = (w << 1) - 2 - x;
  if (y < 0)
    y = -y;
  if (y >= h)
    y = (h << 1) - 2 - y;

  return *((const T *)(((const UBYTE *)(in) + x * bpp + y * bpr)));
}

template<typename T>
static inline T &At(T *in,LONG x,LONG y,LONG w,LONG h,LONG bpp,LONG bpr)
{
  if (x < 0)
    x = -x;
  if (x >= w)
    x = (w << 1) - 2 - x;
  if (y < 0)
    y = -y;
  if (y >= h)
    y = (h << 1) - 2 - y;

  return *((T *)(((UBYTE *)(in) + x * bpp + y * bpr)));
}
///

/// BayerColor::ToRCTX
template<typename S,typename T>
void BayerColor::ToRCTX(const S *in,T *out,LONG chromaoffset,
			ULONG sbpp,ULONG sbpr,
			ULONG tbpp,ULONG tbpr,
			LONG w    ,LONG h)
{
  LONG x,y;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;

  // First step, lift the green channel. Place delta at position kx,ky
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(in,x + kx - 1,y + ky - 1,w,h,sbpp,sbpr) +
	At(in,x + kx + 1,y + ky - 1,w,h,sbpp,sbpr) +
	At(in,x + kx - 1,y + ky + 1,w,h,sbpp,sbpr) +
	At(in,x + kx + 1,y + ky + 1,w,h,sbpp,sbpr);
      At(out,x + kx,y + ky,w,h,tbpp,tbpr) = chromaoffset + (LONG)At(in,x + kx,y + ky,w,h,sbpp,sbpr) - ((gav + 2) >> 2);
    }
  }
  
  // Second step, lift the green channel. Place green average at position gx,gy
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + gx - 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx - 1,y + gy + 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy + 1,w,h,tbpp,tbpr);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = (LONG)At(in,x + gx,y + gy,w,h,sbpp,sbpr) + ((gav + 4) >> 3) - (chromaoffset >> 1);
    }
  }

  // Third step: Compute Cb,Cr and luma from the average green.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + gx,y + gy,w,h,tbpp,tbpr);
      LONG r   = At(in,x + rx,y + ry,w,h,sbpp,sbpr);
      LONG b   = At(in,x + bx,y + by,w,h,sbpp,sbpr);
      At(out,x +  1,y +  1,w,h,tbpp,tbpr) = At(out,x + kx,y + ky,w,h,tbpp,tbpr);      // Put the delta channel last.
      At(out,x +  0,y +  0,w,h,tbpp,tbpr) = ((r + b + (gav << 1)) >> 2) + (chromaoffset >> 1); // Y  at (0,0)
      At(out,x +  1,y +  0,w,h,tbpp,tbpr) = chromaoffset + b - gav;                   // Cb at (1,0)
      At(out,x +  0,y +  1,w,h,tbpp,tbpr) = chromaoffset + r - gav;                   // Cr at (0,1)
    }
  }
}
///

/// BayerColor::FromRCTX
template<typename S,typename T>
void BayerColor::FromRCTX(const S *in,T *out,LONG chromaoffset,
			  ULONG sbpp,ULONG sbpr,
			  ULONG tbpp,ULONG tbpr,
			  LONG w    ,LONG h)
{
  LONG x,y;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;

  //
  // Undo the computation of luma and recompute red,average green and blue.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      // Reshuffle components, they arrive as (Y,Cb,Cr,d) to be in line with the rctd transformation
      LONG d   = At(in,x + 1 ,y +  1,w,h,sbpp,sbpr);
      LONG yl  = At(in,x + 0 ,y +  0,w,h,sbpp,sbpr) - (chromaoffset >> 1);
      LONG cr  = At(in,x + 0 ,y +  1,w,h,sbpp,sbpr) - chromaoffset;
      LONG cb  = At(in,x + 1 ,y +  0,w,h,sbpp,sbpr) - chromaoffset;
      LONG gav = yl - ((cb + cr) >> 2);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = gav;
      At(out,x + rx,y + ry,w,h,tbpp,tbpr) = cr + gav;
      At(out,x + bx,y + by,w,h,tbpp,tbpr) = cb + gav;
      At(out,x + kx,y + ky,w,h,tbpp,tbpr) = d;
    }
  }
  //
  // Inverse lifting, compute the green1 channel from gav and the deltas.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + gx - 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx - 1,y + gy + 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy + 1,w,h,tbpp,tbpr);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = (LONG)At(out,x + gx,y + gy,w,h,tbpp,tbpr) - ((gav + 4) >> 3) + (chromaoffset >> 1);
    }
  }
  //
  // Inverse lifting, compute the green2 channel from green1
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + kx - 1,y + ky - 1,w,h,tbpp,tbpr) +
	At(out,x + kx + 1,y + ky - 1,w,h,tbpp,tbpr) +
	At(out,x + kx - 1,y + ky + 1,w,h,tbpp,tbpr) +
	At(out,x + kx + 1,y + ky + 1,w,h,tbpp,tbpr);
      At(out,x + kx,y + ky,w,h,tbpp,tbpr) = (LONG)At(out,x + kx,y + ky,w,h,tbpp,tbpr) + ((gav + 2) >> 2) - chromaoffset;
    }
  }
}
///

/// BayerColor::ToYDgCoCgX
// Conversion to and from YDgCoCg-X transformation
template<typename S,typename T>
void BayerColor::ToYDgCoCgX(const S *in,T *out,LONG chromaoffset,
			    ULONG sbpp,ULONG sbpr,
			    ULONG tbpp,ULONG tbpr,
			    LONG  w   ,LONG h)
{
  LONG x,y;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;
  LONG p1x,p1y;
  LONG p2x,p2y;

  // First step, lift the green channel. Place delta at position kx,ky
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(in,x + kx - 1,y + ky - 1,w,h,sbpp,sbpr) +
	At(in,x + kx + 1,y + ky - 1,w,h,sbpp,sbpr) +
	At(in,x + kx - 1,y + ky + 1,w,h,sbpp,sbpr) +
	At(in,x + kx + 1,y + ky + 1,w,h,sbpp,sbpr);
      At(out,x + kx,y + ky,w,h,tbpp,tbpr) = chromaoffset + (LONG)At(in,x + kx,y + ky,w,h,sbpp,sbpr) - ((gav + 2) >> 2);
    }
  }
  
  // Second step, lift the green channel. Place green average at position gx,gy
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + gx - 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx - 1,y + gy + 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy + 1,w,h,tbpp,tbpr);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = (LONG)At(in,x + gx,y + gy,w,h,sbpp,sbpr) + ((gav + 4) >> 3) - (chromaoffset >> 1);
    }
  }

  // Third lifting step: Predict red from blue and place the difference at the red sample positions.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG bav = At(in,x + rx - 1,y + ry - 1,w,h,sbpp,sbpr) +
	At(in,x + rx + 1,y + ry - 1,w,h,sbpp,sbpr) +
	At(in,x + rx - 1,y + ry + 1,w,h,sbpp,sbpr) +
	At(in,x + rx + 1,y + ry + 1,w,h,sbpp,sbpr);
      At(out,x + rx,y + ry,w,h,tbpp,sbpr) = chromaoffset + (LONG)At(in,x + rx,y + ry,w,h,sbpp,sbpr)  - ((bav + 2) >> 2);
    }
  }

  // Fourth lifting step: Update blue from the red average and place the updated result at bx,by
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG rav = At(out,x + bx - 1,y + by - 1,w,h,tbpp,tbpr) +
	At(out,x + bx + 1,y + by - 1,w,h,tbpp,tbpr) +
	At(out,x + bx - 1,y + by + 1,w,h,tbpp,tbpr) +
	At(out,x + bx + 1,y + by - 1,w,h,tbpp,tbpr);
      At(out,x + bx,y + by,w,h,tbpp,tbpr) = (LONG)At(in,x + bx,y + by,w,h,sbpp,sbpr) + ((rav + 4) >> 3) - (chromaoffset >> 1);
    }
  }

  //
  // Compute the blue channel close to green. This is either horizontally or vertically
  // aligned.
  if (gx == bx) {
    p1x = p2x = gx;
    p1y = gy - 1;
    p2y = gy + 1;
  } else {
    p1y = p2y = gy;
    p1x = gx - 1;
    p2x = gx + 1;
  }
  // Sixth lifting step: Predict the green average from the blue average and place the result into the green channel.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG bav = At(out,x + p1x,y + p1y,w,h,tbpp,tbpr) + At(out,x + p2x,y + p2y,w,h,tbpp,tbpr);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = chromaoffset + (LONG)At(out,x + gx,y + gy,w,h,tbpp,tbpr) - ((bav + 1) >> 1);
    }
  }

  //
  // Update the blue channel from the green channel, and adjust for the changed offset. The blue channel contains now
  // the luminance information.
  if (gx == bx) {
    p1x = p2x = bx;
    p1y = by - 1;
    p2y = by + 1;
  } else {
    p1y = p2y = by;
    p1x = bx - 1;
    p2x = bx + 1;
  }
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + p1x,y + p1y,w,h,tbpp,tbpr) + At(out,x + p2x,y + p2y,w,h,tbpp,tbpr);
      At(out,x + bx,y + by,w,h,tbpp,tbpr) = (LONG)At(out,x + bx,y + by,w,h,tbpp,tbpr) + ((gav + 2) >> 2);
    }
  }

  //
  // Reshuffle components into the order Y (from blue), cg (from g), co (from r), dg (from k)
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG yl = At(out,x + bx,y + by,w,h,tbpp,tbpr);
      LONG cg = At(out,x + gx,y + gy,w,h,tbpp,tbpr);
      LONG co = At(out,x + rx,y + ry,w,h,tbpp,tbpr);
      LONG dg = At(out,x + kx,y + ky,w,h,tbpp,tbpr);
      At(out,x + 0,y + 0,w,h,tbpp,tbpr) = yl;
      At(out,x + 1,y + 0,w,h,tbpp,tbpr) = cg;
      At(out,x + 0,y + 1,w,h,tbpp,tbpr) = co;
      At(out,x + 1,y + 1,w,h,tbpp,tbpr) = dg;
    }
  }
}
///

/// BayerColor::FromYDgCoCgX
template<typename S,typename T>
void BayerColor::FromYDgCoCgX(const S *in,T *out,LONG chromaoffset,
			      ULONG sbpp,ULONG sbpr,
			      ULONG tbpp,ULONG tbpr,
			      LONG  w   ,LONG h)
{
  LONG x,y;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;
  LONG p1x,p1y;
  LONG p2x,p2y;

  //
  // Reshuffle components into the order Y (to blue), cg (to g), co (to r), dg (to k)
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG yl = At(in,x + 0,y + 0,w,h,sbpp,sbpr);
      LONG cg = At(in,x + 1,y + 0,w,h,sbpp,sbpr);
      LONG co = At(in,x + 0,y + 1,w,h,sbpp,sbpr);
      LONG dg = At(in,x + 1,y + 1,w,h,sbpp,sbpr);
      At(out,x + bx,y + by,w,h,tbpp,tbpr) = yl;
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = cg;
      At(out,x + rx,y + ry,w,h,tbpp,tbpr) = co;
      At(out,x + kx,y + ky,w,h,tbpp,tbpr) = dg;
    }
  }

  //
  // Inverse update the blue channel from the green channel, and adjust for the changed offset. The blue channel contains now
  // the luminance information.
  if (gx == bx) {
    p1x = p2x = bx;
    p1y = by - 1;
    p2y = by + 1;
  } else {
    p1y = p2y = by;
    p1x = bx - 1;
    p2x = bx + 1;
  }
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + p1x,y + p1y,w,h,tbpp,tbpr) + At(out,x + p2x,y + p2y,w,h,tbpp,tbpr);
      At(out,x + bx,y + by,w,h,tbpp,tbpr) = (LONG)At(out,x + bx,y + by,w,h,tbpp,tbpr) - ((gav + 2) >> 2);
    }
  }
  //
  // Inverse predict the blue channel close to green. This is either horizontally or vertically
  // aligned.
  if (gx == bx) {
    p1x = p2x = gx;
    p1y = gy - 1;
    p2y = gy + 1;
  } else {
    p1y = p2y = gy;
    p1x = gx - 1;
    p2x = gx + 1;
  }
  // Sixth lifting step: Inverse predict the green average from the blue average and place the result into the green channel.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG bav = At(out,x + p1x,y + p1y,w,h,tbpp,tbpr) + At(out,x + p2x,y + p2y,w,h,tbpp,tbpr);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = (LONG)At(out,x + gx,y + gy,w,h,tbpp,tbpr) + ((bav + 1) >> 1) - chromaoffset;
    }
  }

  // Fourth lifting step: Inverse update blue from the red average and place the updated result at bx,by
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG rav = At(out,x + bx - 1,y + by - 1,w,h,tbpp,tbpr) +
	At(out,x + bx + 1,y + by - 1,w,h,tbpp,tbpr) +
	At(out,x + bx - 1,y + by + 1,w,h,tbpp,tbpr) +
	At(out,x + bx + 1,y + by - 1,w,h,tbpp,tbpr);
      At(out,x + bx,y + by,w,h,tbpp,tbpr) = (LONG)At(out,x + bx,y + by,w,h,tbpp,tbpr) - ((rav + 4) >> 3) + (chromaoffset >> 1);
    }
  }
  
  // Third lifting step: Inverse predict red from blue and place the difference at the red sample positions.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG bav = At(out,x + rx - 1,y + ry - 1,w,h,tbpp,tbpr) +
	At(out,x + rx + 1,y + ry - 1,w,h,tbpp,tbpr) +
	At(out,x + rx - 1,y + ry + 1,w,h,tbpp,tbpr) +
	At(out,x + rx + 1,y + ry + 1,w,h,tbpp,tbpr);
      At(out,x + rx,y + ry,w,h,tbpp,sbpr) = (LONG)At(out,x + rx,y + ry,w,h,sbpp,sbpr) + ((bav + 2) >> 2) - chromaoffset;
    }
  }

  // Second step, inverse lift the green channel. Input is green average at position gx,gy
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + gx - 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy - 1,w,h,tbpp,tbpr) +
	At(out,x + gx - 1,y + gy + 1,w,h,tbpp,tbpr) +
	At(out,x + gx + 1,y + gy + 1,w,h,tbpp,tbpr);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = (LONG)At(out,x + gx,y + gy,w,h,tbpp,tbpr) - ((gav + 4) >> 3) + (chromaoffset >> 1);
    }
  }

  // First step, lift the green channel. Place delta at position kx,ky
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG gav = At(out,x + kx - 1,y + ky - 1,w,h,tbpp,tbpr) +
	At(out,x + kx + 1,y + ky - 1,w,h,tbpp,tbpr) +
	At(out,x + kx - 1,y + ky + 1,w,h,tbpp,tbpr) +
	At(out,x + kx + 1,y + ky + 1,w,h,tbpp,tbpr);
      At(out,x + kx,y + ky,w,h,tbpp,tbpr) = (LONG)At(out,x + kx,y + ky,w,h,tbpp,tbpr) + ((gav + 2) >> 2) - chromaoffset;
    }
  }
}
///

/// BayerColor::ToRCTD
template<typename S,typename T>
void BayerColor::ToRCTD(const S *in,T *out,LONG chromaoffset,
			ULONG sbpp,ULONG sbpr,
			ULONG tbpp,ULONG tbpr,
			LONG w    ,LONG h)
{
  LONG x,y;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;
  
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      LONG g1  = At(in,x + gx,y + gy,w,h,sbpp,sbpr);
      LONG g2  = At(in,x + kx,y + ky,w,h,sbpp,sbpr);
      LONG r   = At(in,x + rx,y + ry,w,h,sbpp,sbpr);
      LONG b   = At(in,x + bx,y + by,w,h,sbpp,sbpr);
      LONG gav = (g1 + g2) >> 1;
      At(out,x +  1,y +  1,w,h,tbpp,tbpr) = g1 - g2 + chromaoffset;              // Put the delta channel last.
      At(out,x +  0,y +  0,w,h,tbpp,tbpr) = ((r + b + (gav << 1)) >> 2) + (chromaoffset >> 1);           // Y  at (0,0)
      At(out,x +  1,y +  0,w,h,tbpp,tbpr) = chromaoffset + b - gav;              // Cb at (1,0)
      At(out,x +  0,y +  1,w,h,tbpp,tbpr) = chromaoffset + r - gav;              // Cr at (0,1)
    }
  }
}
///

/// BayerColor::FromRCTD
template<typename S,typename T>
void BayerColor::FromRCTD(const S *in,T *out,LONG chromaoffset,
			  ULONG sbpp,ULONG sbpr,
			  ULONG tbpp,ULONG tbpr,
			  LONG w    ,LONG h)
{
  LONG x,y;
  LONG rx = this->m_lrx;
  LONG ry = this->m_lry;
  LONG gx = this->m_lgx;
  LONG gy = this->m_lgy;
  LONG kx = this->m_lkx;
  LONG ky = this->m_lky;
  LONG bx = this->m_lbx;
  LONG by = this->m_lby;

  //
  // Undo the computation of luma and recompute red,average green and blue.
  for(y = 0;y < h;y += 2) {
    for(x = 0;x < w;x += 2) {
      // Reshuffle components, they arrive as (Y,Cb,Cr,d) to be in line with the rctd transformation
      LONG d   = At(in,x + 1 ,y +  1,w,h,sbpp,sbpr) - chromaoffset;
      LONG yl  = At(in,x + 0 ,y +  0,w,h,sbpp,sbpr) - (chromaoffset >> 1);
      LONG cr  = At(in,x + 0 ,y +  1,w,h,sbpp,sbpr) - chromaoffset;
      LONG cb  = At(in,x + 1 ,y +  0,w,h,sbpp,sbpr) - chromaoffset;
      LONG gav = yl - ((cb + cr) >> 2);
      At(out,x + gx,y + gy,w,h,tbpp,tbpr) = gav + d - (d >> 1);
      At(out,x + rx,y + ry,w,h,tbpp,tbpr) = cr + gav;
      At(out,x + bx,y + by,w,h,tbpp,tbpr) = cb + gav;
      At(out,x + kx,y + ky,w,h,tbpp,tbpr) = gav - (d >> 1);
    }
  }
}
///

/// BayerColor::CreateImage
// Allocate the arrays and initialize this image to
// a layout that is identical to the source.
void BayerColor::CreateImage(UBYTE *&target,class ImageLayout *src,bool extendsrange)
{
  UBYTE bps,targetbits;
  assert(m_pComponent == NULL);
  assert(target == NULL);
  //
  // Allocate the component array.
  if (extendsrange) {
    targetbits = src->BitsOf(0) + 1;
  } else {
    targetbits = src->BitsOf(0) - 1;
  }
  m_usDepth    = 1;
  m_ulWidth    = src->WidthOf();
  m_ulHeight   = src->HeightOf();
  m_pComponent = new struct ComponentLayout[1];
  m_pComponent[0].m_ulWidth  = m_ulWidth;
  m_pComponent[0].m_ulHeight = m_ulHeight;
  m_pComponent[0].m_ucBits   = targetbits;
  m_pComponent[0].m_bSigned  = src->isSigned(0);
  m_pComponent[0].m_ucSubX   = src->SubXOf(0);
  m_pComponent[0].m_ucSubY   = src->SubYOf(0);
  //
  // Compute the number of bits per sample. 
  bps    = ImageLayout::SuggestBPP(targetbits,false);
  target = new UBYTE[m_ulWidth * m_ulHeight * bps];
  m_pComponent[0].m_ulBytesPerPixel = bps;
  m_pComponent[0].m_ulBytesPerRow   = bps * m_ulWidth;
  m_pComponent[0].m_pPtr            = target;
}
///

/// BayerColor::Decorrelate
// Forwards transform into the target argument given first from the
// single component image given as source.
void BayerColor::Decorrelate(UBYTE *&target,class ImageLayout *src)
{
  delete[] target;
  target = NULL;
  delete[] m_pComponent;
  m_pComponent = NULL;
  //
  // Check the depth. Bayer can only handle 1 component images.
  if (src->DepthOf() != 1)
    throw "the source image has to be a single-component CFA pattern";
  //
  // Dimensions must be even.
  if ((src->WidthOf() & 1) || (src->HeightOf() & 1))
    throw "the source image dimensions are odd, but must be even";
  //
  // Only integer is supported here.
  if (src->isFloat(0))
    throw "only integer samples are supported by the CFA decorrelation transformations";
  //
  // Update the dimensions of the target image, which is kept in
  // this class.
  CreateImage(target,src,true);
  //
  // Now perform the decorrelation. The component range may be larger.
  // This correlation is defined such that components never get signed.
  if (isSigned(0)) {
    if (src->BitsOf(0) <= 7) {
      DispatchDecorrelation<BYTE,BYTE>((const BYTE *)src->DataOf(0),(BYTE *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) == 8) {
      DispatchDecorrelation<BYTE,WORD>((const BYTE *)src->DataOf(0),(WORD *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) < 16) {
      DispatchDecorrelation<WORD,WORD>((const WORD *)src->DataOf(0),(WORD *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) == 16) {
      DispatchDecorrelation<WORD,LONG>((const WORD *)src->DataOf(0),(LONG *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) <= 32) {
      DispatchDecorrelation<LONG,LONG>((const LONG *)src->DataOf(0),(LONG *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else {
      throw "unsupported data type";
    }
  } else {
    LONG offset = (ULONG(1) << src->BitsOf(0));
    if (src->BitsOf(0) <= 7) {
      DispatchDecorrelation<UBYTE,UBYTE>((const UBYTE *)src->DataOf(0),(UBYTE *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) == 8) {
      DispatchDecorrelation<UBYTE,UWORD>((const UBYTE *)src->DataOf(0),(UWORD *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) < 16) {
      DispatchDecorrelation<UWORD,UWORD>((const UWORD *)src->DataOf(0),(UWORD *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) == 16) {
      DispatchDecorrelation<UWORD,ULONG>((const UWORD *)src->DataOf(0),(ULONG *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (src->BitsOf(0) <= 32) {
      DispatchDecorrelation<ULONG,ULONG>((const ULONG *)src->DataOf(0),(ULONG *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else {
      throw "unsupported data type";
    }
  }
  //
  // Swap in the new definition, replacing the old.
  Swap(*src);
}
///

/// BayerColor::InverseDecorrelate
// Backards transform into the target argument given first from
// the single component image given as source
void BayerColor::InverseDecorrelate(UBYTE *&target,class ImageLayout *src)
{
  delete[] target;
  target = NULL;
  delete[] m_pComponent;
  m_pComponent = NULL;
  //
  // Check the depth. Bayer can only handle 1 component images.
  if (src->DepthOf() != 1)
    throw "the source image has to be a single-component CFA pattern";
  //
  // Dimensions must be even.
  if ((src->WidthOf() & 1) || (src->HeightOf() & 1))
    throw "the source image dimensions are odd, but must be even";
  //
  // Only integer is supported here.
  if (src->isFloat(0))
    throw "only integer samples are supported by the CFA decorrelation transformations";
  //
  // Update the dimensions of the target image, which is kept in
  // this class.
  CreateImage(target,src,false);
  //
  // Now perform the decorrelation. The component range may be larger.
  // This correlation is defined such that components never get signed.
  if (isSigned(0)) {
    if (BitsOf(0) <= 7) {
      DispatchInverseDecorrelation<BYTE,BYTE>((const BYTE *)src->DataOf(0),(BYTE *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) == 8) {
      DispatchInverseDecorrelation<WORD,BYTE>((const WORD *)src->DataOf(0),(BYTE *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) < 16) {
      DispatchInverseDecorrelation<WORD,WORD>((const WORD *)src->DataOf(0),(WORD *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) == 16) {
      DispatchInverseDecorrelation<LONG,WORD>((const LONG *)src->DataOf(0),(WORD *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) <= 32) {
      DispatchInverseDecorrelation<LONG,LONG>((const LONG *)src->DataOf(0),(LONG *)target,0,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else {
      throw "unsupported data type";
    }
  } else {
    LONG offset = (LONG(1) << BitsOf(0));
    if (BitsOf(0) <= 7) {
      DispatchInverseDecorrelation<UBYTE,UBYTE>((const UBYTE *)src->DataOf(0),(UBYTE *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) == 8) {
      DispatchInverseDecorrelation<UWORD,UBYTE>((const UWORD *)src->DataOf(0),(UBYTE *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) < 16) {
      DispatchInverseDecorrelation<UWORD,UWORD>((const UWORD *)src->DataOf(0),(UWORD *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) == 16) {
      DispatchInverseDecorrelation<ULONG,UWORD>((const ULONG *)src->DataOf(0),(UWORD *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else if (BitsOf(0) <= 32) {
      DispatchInverseDecorrelation<ULONG,ULONG>((const ULONG *)src->DataOf(0),(ULONG *)target,offset,src->BytesPerPixel(0),src->BytesPerRow(0));
    } else {
      throw "unsupported data type";
    }
  }
  //
  // Swap in the new definition, replacing the old.
  Swap(*src);
}
///

/// BayerColor::DispatchDecorrelation
// Depending on the type of the decorrelation, depatch into the corresponding
// implementation.
template<typename S,typename T>
void BayerColor::DispatchDecorrelation(const S *src,T *dst,LONG offset,ULONG sbpp,ULONG sbpr)
{
  switch(this->m_Conversion) {
  case RCTX:
    ToRCTX(src,dst,offset,
	   sbpp,sbpr,
	   BytesPerPixel(0),BytesPerRow(0),
	   m_ulWidth,m_ulHeight);
    break;
  case RCTD:
    ToRCTD(src,dst,offset,
	   sbpp,sbpr,
	   BytesPerPixel(0),BytesPerRow(0),
	   m_ulWidth,m_ulHeight);
    break;
  case YDgCoCgX:
    ToYDgCoCgX(src,dst,offset,
	       sbpp,sbpr,
	       BytesPerPixel(0),BytesPerRow(0),
	       m_ulWidth,m_ulHeight);
    break;
  default:
    throw "invalid transformation specified";
  }
}
///

/// BayerColor::DispatchInverseDecorrelation
// Depending on the type of the decorrelation, depatch into the corresponding
// implementation.
template<typename S,typename T>
void BayerColor::DispatchInverseDecorrelation(const S *src,T *dst,LONG offset,ULONG sbpp,ULONG sbpr)
{
  switch(this->m_Conversion) {
  case RCTX:
    FromRCTX(src,dst,offset,
	     sbpp,sbpr,
	     BytesPerPixel(0),BytesPerRow(0),
	     m_ulWidth,m_ulHeight);
    break;
  case RCTD:
    FromRCTD(src,dst,offset,
	     sbpp,sbpr,
	     BytesPerPixel(0),BytesPerRow(0),
	     m_ulWidth,m_ulHeight);
    break;
  case YDgCoCgX:
    FromYDgCoCgX(src,dst,offset,
		 sbpp,sbpr,
		 BytesPerPixel(0),BytesPerRow(0),
		 m_ulWidth,m_ulHeight);
    break;
  default:
    throw "invalid transformation specified";
  }
}
///

/// BayerColor::Measure
// Perform the actual conversion.
double BayerColor::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  if (m_bInverse) {
    InverseDecorrelate(m_pucSrcImage,src);
    InverseDecorrelate(m_pucDstImage,dst);
  } else {
    Decorrelate(m_pucSrcImage,src);
    Decorrelate(m_pucDstImage,dst);
  }

  return in;
}
///
