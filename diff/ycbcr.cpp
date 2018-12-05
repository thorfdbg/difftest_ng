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
** $Id: ycbcr.cpp,v 1.14 2018/12/05 09:21:53 thor Exp $
**
** This class converts between RGB and YCbCr signals
*/

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "diff/ycbcr.hpp"
#include "std/math.hpp"
///

/// YCbCr::~YCbCr
// Destructor, also gets rid of the image memory
YCbCr::~YCbCr(void)
{
  int i;
  
  for(i = 0;i < 4;i++) {
    delete[] m_ppucSrcImage[i];
    delete[] m_ppucDstImage[i];
  }
}
///

/// YCbCr::ToDeltaGreen
template<typename S,typename T>
void YCbCr::ToDeltaGreen(const S *g1,const S *g2,S *a,T *d,
			 LONG doffset,
			 ULONG bppg1,ULONG bppg2,
			 ULONG bprg1,ULONG bprg2,
			 ULONG bppa,ULONG bppd,
			 ULONG bpra,ULONG bprd,
			 ULONG w,ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    const S *g1row = g1;
    const S *g2row = g2;
    S *arow        = a;
    T *drow        = d;
    for(x = 0;x < w;x++) {
      LONG avg   = *g1row + *g2row;
      LONG delta = LONG(*g1row) - LONG(*g2row);
      //
      *arow      = S(avg) >> 1;
      *drow      = T(delta + doffset);
      //
      g1row  = (const S *)((const UBYTE *)(g1row)  + bppg1);
      g2row  = (const S *)((const UBYTE *)(g2row)  + bppg2);
      arow   = (S *)((UBYTE *)(arow) + bppa);
      drow   = (T *)((UBYTE *)(drow) + bppd);
    }
    g1 = (const S *)((const UBYTE *)(g1) + bprg1);
    g2 = (const S *)((const UBYTE *)(g2) + bprg2);
    a  = (S *)((UBYTE *)(a) + bpra);
    d  = (T *)((UBYTE *)(d) + bprd);
  }
}
///

/// YCbCr::FromDeltaGreen
template<typename S,typename T>
void YCbCr::FromDeltaGreen(const S *a,const T *d,S *g1,S *g2,
			   LONG doffset,
			   ULONG bppa,ULONG bppd,
			   ULONG bpra,ULONG bprd,
			   ULONG bppg1,ULONG bppg2,
			   ULONG bprg1,ULONG bprg2,
			   ULONG w,ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    const S *arow = a;
    const T *drow = d;
    S *g1row      = g1;
    S *g2row      = g2;
    for(x = 0;x < w;x++) {
      LONG delta = *drow - doffset;
      LONG k     = *arow - (delta >> 1);
      LONG g     = delta + k;
      *g1row     = S(g);
      *g2row     = S(k);
      g1row  = (S *)((UBYTE *)(g1row)  + bppg1);
      g2row  = (S *)((UBYTE *)(g2row)  + bppg2);
      arow   = (const S *)((const UBYTE *)(arow) + bppa);
      drow   = (const T *)((const UBYTE *)(drow) + bppd);
    }
    g1 = (S *)((UBYTE *)(g1) + bprg1);
    g2 = (S *)((UBYTE *)(g2) + bprg2);
    a  = (const S *)((const UBYTE *)(a) + bpra);
    d  = (const T *)((const UBYTE *)(d) + bprd);
  }
}
///

/// YCbCr::Copy
template<typename S>
void YCbCr::Copy(const S *src,S *dst,
		 ULONG srcbpp,ULONG srcbpr,
		 ULONG dstbpp,ULONG dstbpr,
		 ULONG w,ULONG h)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    const S *srow = src;
    S *drow       = dst;
    for(x = 0;x < w;x++) {
      *drow = *srow;
      srow  = (const S *)((const UBYTE *)(srow) + srcbpp);
      drow  = (S *)((UBYTE *)(drow) + dstbpp);
    }
    src = (const S *)((const UBYTE *)(src) + srcbpr);
    dst = (S *)((UBYTE *)(dst) + dstbpr);
  }
}
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

/// YCbCr::ToYCbCr709
template<typename S,typename T>
void YCbCr::ToYCbCr709(S *r,S *g,S *b,
		       double yoffset,double coffset,
		       double min,double max,
		       double cmin,double cmax,
		       ULONG bppr,ULONG bppg,ULONG bppb,
		       ULONG bprr,ULONG bprg,ULONG bprb,
		       ULONG w, ULONG h)
{
  ULONG x,y;
  const double kb = 0.0722;
  const double kr = 0.2126;
  const double kg = 1.0 - kb - kr;

  for(y = 0;y < h;y++) {
    S *rrow = r;
    S *grow = g;
    S *brow = b;
    for(x = 0;x < w;x++) {
      double y  = *rrow * kr    + *grow * kg    + *brow * kb;
      double cb = 0.5 * (*brow  - y) / (1.0 - kb) + coffset;
      double cr = 0.5 * (*rrow  - y) / (1.0 - kr) + coffset;
      //
      // clip to range.
      y += yoffset;
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


/// YCbCr::ToYCbCr2020
template<typename S,typename T>
void YCbCr::ToYCbCr2020(S *r,S *g,S *b,
		       double yoffset,double coffset,
		       double min,double max,
		       double cmin,double cmax,
		       ULONG bppr,ULONG bppg,ULONG bppb,
		       ULONG bprr,ULONG bprg,ULONG bprb,
		       ULONG w, ULONG h)
{
  ULONG x,y;
  const double kb = 0.0593;
  const double kr = 0.2627;
  const double kg = 1.0 - kb - kr;

  for(y = 0;y < h;y++) {
    S *rrow = r;
    S *grow = g;
    S *brow = b;
    for(x = 0;x < w;x++) {
      double y  = *rrow * kr    + *grow * kg    + *brow * kb    + yoffset;
      double cb = 0.5 * (*brow  - y) / (1.0 - kb) + coffset;
      double cr = 0.5 * (*rrow  - y) / (1.0 - kr) + coffset;
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

/// YCbCr::FromYCbCr709
template<typename S,typename T>
void YCbCr::FromYCbCr709(S *yp,T *cb,T *cr,
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
      double r  = (*yrow - yoffset) + (*crrow - coffset)*1.5748;
      double g  = (*yrow - yoffset) + (*cbrow - coffset)*-0.1873242729306488 + (*crrow - coffset)*-0.4681242729306488;
      double b  = (*yrow - yoffset) + (*cbrow - coffset)*1.8556;
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

/// YCbCr::FromYCbCr2020
template<typename S,typename T>
void YCbCr::FromYCbCr2020(S *yp,T *cb,T *cr,
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
      double r  = (*yrow - yoffset) + (*crrow - coffset)*1.4746;
      double g  = (*yrow - yoffset) + (*cbrow - coffset)*-0.1645531268436578 + (*crrow - coffset)*-0.5713531268436578;
      double b  = (*yrow - yoffset) + (*cbrow - coffset)*1.8814;
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

/// YCbCr::ToRCT
template<typename S,typename T>
void YCbCr::ToRCT(const S *r,const S *g,const S *b,S *y,T *cb,T *cr,
		  LONG yoffset,LONG coffset,
		  ULONG bppr,ULONG bppg, ULONG bppb,
		  ULONG bprr,ULONG bprg, ULONG bprb,
		  ULONG bppy,ULONG bppcb,ULONG bppcr,
		  ULONG bpry,ULONG bprcb,ULONG bprcr,
		  ULONG w, ULONG h)
{
  ULONG xi,yi;

  for(yi = 0;yi < h;yi++) {
    const S *rrow  = r;
    const S *grow  = g;
    const S *brow  = b;
    S *yrow  = y;
    T *cbrow = cb;
    T *crrow = cr;
    for(xi = 0;xi < w;xi++) {
      LONG r  = *rrow;
      LONG g  = *grow;
      LONG b  = *brow;
      LONG y  = ((r + (g << 1) + b) >> 2) + yoffset;
      LONG cb =  (b - g) + coffset;
      LONG cr =  (r - g) + coffset;
      //
      *yrow  = S(y);
      *cbrow = T(cb);
      *crrow = T(cr);
      //
      rrow   = (const S *)((const UBYTE *)(rrow)  + bppr);
      grow   = (const S *)((const UBYTE *)(grow)  + bppg);
      brow   = (const S *)((const UBYTE *)(brow)  + bppb);
      yrow   = (S *)((UBYTE *)(yrow)  + bppy);
      cbrow  = (T *)((UBYTE *)(cbrow) + bppcb);
      crrow  = (T *)((UBYTE *)(crrow) + bppcr);
    }
    r  = (const S *)((const UBYTE *)(r)  + bprr);
    g  = (const S *)((const UBYTE *)(g)  + bprg);
    b  = (const S *)((const UBYTE *)(b)  + bprb);
    y  = (S *)((UBYTE *)(y)  + bpry);
    cb = (T *)((UBYTE *)(cb) + bprcb);
    cr = (T *)((UBYTE *)(cr) + bprcr);
  }
}
///

/// YCbCr::To422RCT
template<typename S,typename T>
void YCbCr::To422RCT(const S *r,const S *g,const S *b,S *y,T *cb,T *cr,
		     LONG yoffset,LONG coffset,
		     ULONG bppr,ULONG bppg, ULONG bppb,
		     ULONG bprr,ULONG bprg, ULONG bprb,
		     ULONG bppy,ULONG bppcb,ULONG bppcr,
		     ULONG bpry,ULONG bprcb,ULONG bprcr,
		     ULONG w, ULONG h)
{
  ULONG xi,yi;

  for(yi = 0;yi < h;yi++) {
    const S *rrow  = r;
    const S *grow  = g;
    const S *brow  = b;
    S *yrow  = y;
    T *cbrow = cb;
    T *crrow = cr;
    for(xi = 0;xi < w;xi += 2) {
      LONG r  = *rrow;
      LONG g1 = *grow;
      LONG b  = *brow;
      LONG y1 = ((r + (g1 << 1) + b) >> 1) + yoffset;
      LONG cb =  (b - g1) + coffset;
      LONG cr =  (r - g1) + coffset;
      grow    = (const S *)((const UBYTE *)(grow)  + bppr);
      LONG g2 = *grow;
      LONG y2 = ((r + (g2 << 1) + b) >> 1) + yoffset;
      //
      *yrow  = S(y1);
      yrow   = (S *)((UBYTE *)(yrow)  + bppy);
      *yrow  = S(y2);
      *cbrow = T(cb);
      *crrow = T(cr);
      //
      rrow   = (const S *)((const UBYTE *)(rrow)  + bppr);
      grow   = (const S *)((const UBYTE *)(grow)  + bppg);
      brow   = (const S *)((const UBYTE *)(brow)  + bppb);
      yrow   = (S *)((UBYTE *)(yrow)  + bppy);
      cbrow  = (T *)((UBYTE *)(cbrow) + bppcb);
      crrow  = (T *)((UBYTE *)(crrow) + bppcr);
    }
    r  = (const S *)((const UBYTE *)(r)  + bprr);
    g  = (const S *)((const UBYTE *)(g)  + bprg);
    b  = (const S *)((const UBYTE *)(b)  + bprb);
    y  = (S *)((UBYTE *)(y)  + bpry);
    cb = (T *)((UBYTE *)(cb) + bprcb);
    cr = (T *)((UBYTE *)(cr) + bprcr);
  }
}
///

/// YCbCr::FromRCT
template<typename S,typename T>
void YCbCr::FromRCT(const S *yp,const T *cb,const T *cr,S *r,S *g,S *b,
		    LONG yoffset,LONG coffset,
		    ULONG bppy,ULONG bppcb,ULONG bppcr,
		    ULONG bpry,ULONG bprcb,ULONG bprcr,
		    ULONG bppr,ULONG bppg, ULONG bppb,
		    ULONG bprr,ULONG bprg, ULONG bprb,
		    ULONG w, ULONG h,
		    LONG min,LONG max)
{
  ULONG xi,yi;

  for(yi = 0;yi < h;yi++) {
    const S *yrow  = yp;
    const T *cbrow = cb;
    const T *crrow = cr;
    S *rrow  = r;
    S *grow  = g;
    S *brow  = b;
    for(xi = 0;xi < w;xi++) {
      LONG y  = *yrow  - yoffset;
      LONG cb = *cbrow - coffset;
      LONG cr = *crrow - coffset;
      LONG g  = y - ((cb + cr) >> 2);
      LONG r  = cr + g;
      LONG b  = cb + g;
      // Clamp to range
      if (r < min) r = min;
      if (r > max) r = max;
      if (g < min) g = min;
      if (g > max) g = max;
      if (b < min) b = min;
      if (b > max) b = max;
      //
      *rrow = S(r);
      *grow = S(g);
      *brow = S(b);
      //
      yrow   = (const S *)((const UBYTE *)(yrow ) + bppy);
      cbrow  = (const T *)((const UBYTE *)(cbrow) + bppcb);
      crrow  = (const T *)((const UBYTE *)(crrow) + bppcr);
      rrow   = (S *)((UBYTE *)(rrow ) + bppr);
      grow   = (S *)((UBYTE *)(grow)  + bppg);
      brow   = (S *)((UBYTE *)(brow)  + bppb);
    }
    yp  = (const S *)((const UBYTE *)(yp) + bpry);
    cb  = (const T *)((const UBYTE *)(cb) + bprcb);
    cr  = (const T *)((const UBYTE *)(cr) + bprcr);
    r   = (S *)((UBYTE *)(r)  + bprr);
    g   = (S *)((UBYTE *)(g)  + bprg);
    b   = (S *)((UBYTE *)(b)  + bprb);
  }
}
///

/// YCbCr::From422RCT
template<typename S,typename T>
void YCbCr::From422RCT(const S *yp,const T *cb,const T *cr,S *r,S *g,S *b,
		       LONG yoffset,LONG coffset,
		       ULONG bppy,ULONG bppcb,ULONG bppcr,
		       ULONG bpry,ULONG bprcb,ULONG bprcr,
		       ULONG bppr,ULONG bppg, ULONG bppb,
		       ULONG bprr,ULONG bprg, ULONG bprb,
		       ULONG w, ULONG h,
		       LONG min,LONG max)
{
  ULONG xi,yi;

  for(yi = 0;yi < h;yi++) {
    const S *yrow  = yp;
    const T *cbrow = cb;
    const T *crrow = cr;
    S *rrow  = r;
    S *grow  = g;
    S *brow  = b;
    for(xi = 0;xi < w;xi += 2) {
      LONG y1 = *yrow  - yoffset;
      LONG cb = *cbrow - coffset;
      LONG cr = *crrow - coffset;
      LONG g1 = (y1 - ((cb + cr) >> 1)) >> 1;
      LONG r  = cr + g1;
      LONG b  = cb + g1;
      yrow    = (const S *)((const UBYTE *)(yrow ) + bppy);
      LONG y2 = *yrow  - yoffset;
      LONG g2 = y2 - ((r + b) >> 1);
      //
      // Clamp to range
      if (r  < min) r  = min;
      if (r  > max) r  = max;
      if (g1 < min) g1 = min;
      if (g1 > max) g1 = max;
      if (g2 < min) g2 = min;
      if (g2 > max) g2 = max;
      if (b  < min) b  = min;
      if (b  > max) b  = max;
      //
      *rrow  = S(r);
      *grow  = S(g1);
      grow   = (S *)((UBYTE *)(grow)  + bppg);
      *grow  = S(g2);
      *brow  = S(b);
      //
      yrow   = (const S *)((const UBYTE *)(yrow ) + bppy);
      cbrow  = (const T *)((const UBYTE *)(cbrow) + bppcb);
      crrow  = (const T *)((const UBYTE *)(crrow) + bppcr);
      rrow   = (S *)((UBYTE *)(rrow ) + bppr);
      grow   = (S *)((UBYTE *)(grow)  + bppg);
      brow   = (S *)((UBYTE *)(brow)  + bppb);
    }
    yp  = (const S *)((const UBYTE *)(yp) + bpry);
    cb  = (const T *)((const UBYTE *)(cb) + bprcb);
    cr  = (const T *)((const UBYTE *)(cr) + bprcr);
    r   = (S *)((UBYTE *)(r)  + bprr);
    g   = (S *)((UBYTE *)(g)  + bprg);
    b   = (S *)((UBYTE *)(b)  + bprb);
  }
}
///

/// YCbCr::ToYCgCo
template<typename S,typename T>
void YCbCr::ToYCgCo(const S *r,const S *g,const S *b,S *y,T *cg,T *co,
		    LONG yoffset,LONG coffset,
		    ULONG bppr,ULONG bppg,ULONG bppb,
		    ULONG bprr,ULONG bprg,ULONG bprb,
		    ULONG bppy,ULONG bppcg,ULONG bppco,
		    ULONG bpry,ULONG bprcg,ULONG bprco,
		    ULONG w, ULONG h)
{
  ULONG xi,yi;

  for(yi = 0;yi < h;yi++) {
    const S *rrow = r;
    const S *grow = g;
    const S *brow = b;
    S *yrow  = y;
    T *cgrow = cg;
    T *corow = co;
    for(xi = 0;xi < w;xi++) {
      LONG r  = *rrow;
      LONG g  = *grow;
      LONG b  = *brow;
      LONG co = r - b;
      LONG t  = b + (co >> 1);
      LONG cg = g - t;
      LONG y  = t + (cg >> 1);
      //
      *yrow   = S(y  + yoffset);
      *cgrow  = T(cg + coffset);
      *corow  = T(co + coffset);
      //
      rrow  = (const S *)((const UBYTE *)(rrow) + bppr);
      grow  = (const S *)((const UBYTE *)(grow) + bppg);
      brow  = (const S *)((const UBYTE *)(brow) + bppb);
      yrow  = (S *)((UBYTE *)(yrow)  + bppy);
      cgrow = (T *)((UBYTE *)(cgrow) + bppcg);
      corow = (T *)((UBYTE *)(corow) + bppco);
    }
    r  = (const S *)((const UBYTE *)(r) + bprr);
    g  = (const S *)((const UBYTE *)(g) + bprg);
    b  = (const S *)((const UBYTE *)(b) + bprb);
    y  = (S *)((UBYTE *)(y)  + bpry);
    cg = (T *)((UBYTE *)(cg) + bprcg);
    co = (T *)((UBYTE *)(co) + bprco);
  }
}
///

/// YCbCr::FromYCgCo
template<typename S,typename T>
void YCbCr::FromYCgCo(const S *yp,const T *cg,const T *co,S *r,S *g,S *b,
		      LONG yoffset,LONG coffset,
		      ULONG bppy,ULONG bppcg,ULONG bppco,
		      ULONG bpry,ULONG bprcg,ULONG bprco,
		      ULONG bppr,ULONG bppg, ULONG bppb,
		      ULONG bprr,ULONG bprg, ULONG bprb,
		      ULONG w, ULONG h,
		      LONG min,LONG max)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    const S *yrow  = yp;
    const T *cgrow = cg;
    const T *corow = co;
    S *rrow  = r;
    S *grow  = g;
    S *brow  = b;
    for(x = 0;x < w;x++) {
      LONG y  = *yrow  - yoffset;
      LONG cg = *cgrow - coffset;
      LONG co = *corow - coffset;
      LONG t  = y - (cg >> 1);
      LONG g  = cg + t;
      LONG b  = t - (co >> 1);
      LONG r  = b + co;
      // Clamp to range
      if (r < min) r = min;
      if (r > max) r = max;
      if (g < min) g = min;
      if (g > max) g = max;
      if (b < min) b = min;
      if (b > max) b = max;
      //
      *rrow  = S(r);
      *grow  = S(g);
      *brow  = S(b);
      //
      yrow   = (const S *)((const UBYTE *)(yrow ) + bppy);
      cgrow  = (const T *)((const UBYTE *)(cgrow) + bppcg);
      corow  = (const T *)((const UBYTE *)(corow) + bppco);
      rrow   = (S *)((UBYTE *)(rrow ) + bppr);
      grow   = (S *)((UBYTE *)(grow)  + bppg);
      brow   = (S *)((UBYTE *)(brow)  + bppb);
    }
    yp  = (const S *)((const UBYTE *)(yp) + bpry);
    cg  = (const T *)((const UBYTE *)(cg) + bprcg);
    co  = (const T *)((const UBYTE *)(co) + bprco);
    r   = (S *)((UBYTE *)(r)  + bprr);
    g   = (S *)((UBYTE *)(g)  + bprg);
    b   = (S *)((UBYTE *)(b)  + bprb);
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
      if (m_bBlackLevel) {
	yoffset += (QUAD(1) << bits) >> 4;
      }
    }
  }
  //
  if (isfloat) {
    if (bits <= 32) {
      DispatchToYCbCr<FLOAT,FLOAT>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
    } else if (bits == 64) {
      DispatchToYCbCr<DOUBLE,DOUBLE>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
    } else throw "unsupported source format";
  } else {
    if (bits <= 8) {
      if (issigned) {
	DispatchToYCbCr<BYTE,BYTE>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      } else if (m_bMakeSigned) {
	DispatchToYCbCr<UBYTE,BYTE>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      } else {
	DispatchToYCbCr<UBYTE,UBYTE>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      }
    } else if (bits <= 16) {
      if (issigned) {
	DispatchToYCbCr<WORD,WORD>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      } else if (m_bMakeSigned) {
	DispatchToYCbCr<UWORD,WORD>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      } else {
	DispatchToYCbCr<UWORD,UWORD>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      }
    } else if (bits <= 32) {
      if (issigned) {
	DispatchToYCbCr<LONG,LONG>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      } else if (m_bMakeSigned) {
	DispatchToYCbCr<ULONG,LONG>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
      } else {
	DispatchToYCbCr<ULONG,ULONG>(img,yoffset,coffset,ymin,ymax,cmin,cmax,w,h);
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
      if (m_bBlackLevel) {
	yoffset += (QUAD(1) << bits) >> 4;
      }
    }
  }  
  //
  // Now run the conversion
  if (isfloat) {
    if (bits <= 32) {
      DispatchFromYCbCr<FLOAT,FLOAT>(img,yoffset,coffset,min,max,w,h);
    } else if (bits == 64) {
      DispatchFromYCbCr<DOUBLE,DOUBLE>(img,yoffset,coffset,min,max,w,h);
    } else throw "unsupported source format";
  } else {
    if (bits <= 8) {
      if (issigned) {
	DispatchFromYCbCr<BYTE,BYTE>(img,yoffset,coffset,min,max,w,h);
      } else if (wassigned) {
	DispatchFromYCbCr<UBYTE,BYTE>(img,yoffset,coffset,min,max,w,h);
      } else {
	DispatchFromYCbCr<UBYTE,UBYTE>(img,yoffset,coffset,min,max,w,h);
      }
    } else if (bits <= 16) {
      if (issigned) {
	DispatchFromYCbCr<WORD,WORD>(img,yoffset,coffset,min,max,w,h);
      } else if (wassigned) {
	DispatchFromYCbCr<UWORD,WORD>(img,yoffset,coffset,min,max,w,h);
      } else {
	DispatchFromYCbCr<UWORD,UWORD>(img,yoffset,coffset,min,max,w,h);
      }
    } else if (bits <= 32) {
      if (issigned) {
	DispatchFromYCbCr<LONG,LONG>(img,yoffset,coffset,min,max,w,h);
      } else if (wassigned) {
	DispatchFromYCbCr<ULONG,LONG>(img,yoffset,coffset,min,max,w,h);
      } else {
	DispatchFromYCbCr<ULONG,ULONG>(img,yoffset,coffset,min,max,w,h);
      }
    } else throw "unsupported source format";
  }  

  img->isSigned(1) = issigned;
  img->isSigned(2) = issigned;
}
///

/// YCbCr::DispatchToRCT
// Convert from the external image to the image stored in
// this structure
template<typename S,typename T>
void YCbCr::DispatchToRCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h)
{
  switch(m_Conversion) {
  case RCT_Trafo:
    ToRCT<S,T>((const S *)img->DataOf(0),(const S *)img->DataOf(1),(const S *)img->DataOf(2),
	       (S *)this->DataOf(0),(T *)this->DataOf(1),(T *)this->DataOf(2),
	       yoffset,coffset,
	       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
	       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
	       this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(2),
	       this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(2),
	       w,h);
    break;
  case YCgCo_Trafo:
    ToYCgCo<S,T>((const S *)img->DataOf(0),(const S *)img->DataOf(1),(const S *)img->DataOf(2),
		 (S *)this->DataOf(0),(T *)this->DataOf(1),(T *)this->DataOf(2),
		 yoffset,coffset,
		 img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		 img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		 this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(2),
		 this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(2),
		 w,h);
    break;
  case RCTD_Trafo:
    ToDeltaGreen<S,T>((const S *)img->DataOf(1),(const S *)img->DataOf(2),
		      (S *)this->DataOf(1),(T *)this->DataOf(3),
		      coffset,
		      img->BytesPerPixel(1),img->BytesPerPixel(2),
		      img->BytesPerRow(1)  ,img->BytesPerRow(2),
		      this->BytesPerPixel(1),this->BytesPerPixel(3),
		      this->BytesPerRow(1)  ,this->BytesPerRow(3),
		      w,h);
    ToRCT<S,T>((const S *)img->DataOf(0),(const S *)this->DataOf(1),(const S *)img->DataOf(3),
	       (S *)this->DataOf(0),(T *)this->DataOf(1),(T *)this->DataOf(2),
	       yoffset,coffset,
	       img->BytesPerPixel(0),this->BytesPerPixel(1),img->BytesPerPixel(3),
	       img->BytesPerRow(0)  ,this->BytesPerRow(1)  ,img->BytesPerRow(3),
	       this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(2),
	       this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(2),
	       w,h);
    break;
  case YCgCoD_Trafo:
    ToDeltaGreen<S,T>((const S *)img->DataOf(1),(const S *)img->DataOf(2),
		      (S *)this->DataOf(1),(T *)this->DataOf(3),
		      coffset,
		      img->BytesPerPixel(1),img->BytesPerPixel(2),
		      img->BytesPerRow(1)  ,img->BytesPerRow(2),
		      this->BytesPerPixel(1),this->BytesPerPixel(3),
		      this->BytesPerRow(1)  ,this->BytesPerRow(3),
		      w,h);
    ToYCgCo<S,T>((const S *)img->DataOf(0),(const S *)this->DataOf(1),(const S *)img->DataOf(3),
		 (S *)this->DataOf(0),(T *)this->DataOf(1),(T *)this->DataOf(2),
		 yoffset,coffset,
		 img->BytesPerPixel(0),this->BytesPerPixel(1),img->BytesPerPixel(3),
		 img->BytesPerRow(0)  ,this->BytesPerRow(1)  ,img->BytesPerRow(3),
		 this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(2),
		 this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(2),
		 w,h);
    break;
  case Delta_Trafo:
    ToDeltaGreen<S,T>((const S *)img->DataOf(1),(const S *)img->DataOf(2),
		      (S *)this->DataOf(1),(T *)this->DataOf(3),
		      coffset,
		      img->BytesPerPixel(1),img->BytesPerPixel(2),
		      img->BytesPerRow(1)  ,img->BytesPerRow(2),
		      this->BytesPerPixel(1),this->BytesPerPixel(3),
		      this->BytesPerRow(1)  ,this->BytesPerRow(3),
		      w,h);
    Copy<S>((const S *)img->DataOf(0),(S *)this->DataOf(0),
	    img->BytesPerPixel(0),img->BytesPerRow(0),
	    this->BytesPerPixel(0),this->BytesPerRow(0),
	    w,h);
    Copy<S>((const S *)img->DataOf(3),(S *)this->DataOf(2),
	    img->BytesPerPixel(3),img->BytesPerRow(3),
	    this->BytesPerPixel(2),this->BytesPerRow(2),
	    w,h);
    break;
  default:
    throw "unknown conversion specified";
  }
}
///

/// YCbCr::ToRCT
// Convert an image with the RCT or the YCgCo transformation.
void YCbCr::ToRCT(class ImageLayout *img,UBYTE *(&membuf)[4])
{
  LONG yoffset  = 0; // always zero
  LONG coffset  = 0; // the chroma component offset.
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  bool  ysign   = img->isSigned(0);
  bool  csign   = img->isSigned(0) || m_bMakeSigned;
  UBYTE bits    = img->BitsOf(0);
  UWORD comp;
  UWORD mindepth;

  switch(m_Conversion) {
  case RCTD_Trafo:
  case YCgCoD_Trafo:
  case Delta_Trafo:
    mindepth = 4;
    break;
  default:
    mindepth = 3;
    break;
  }
  
  CreateComponents(*img);
  
  if (!m_bMakeSigned) {
    coffset = 0 + ((ysign)?(0):(LONG(1) << (img->BitsOf(0))));
  }
  //
  // Now build the components. Floating point is not supported.
  // Everything else is copied over. The entry point tested that
  // at least three components are available.
  for(comp = 0;comp < img->DepthOf();comp++) {
    if (comp < mindepth) {
      ULONG w     = img->WidthOf(comp);
      ULONG h     = img->HeightOf(comp);
      UBYTE bits  = img->BitsOf(comp); // input bits.
      UBYTE obits = bits;
      UBYTE bpc;
      UBYTE *mem;
      //
      // If this is float, then we are not supported.
      if (img->isFloat(comp))
	throw "RCT and YCgCo color spaces are not supported for floating point data";
      //
      // Depth must be all the same.
      if (bits != img->BitsOf(0))
	throw "RCT and YCgCo require all the same bit depths as input";
      if (img->isSigned(comp) != img->isSigned(0))
	throw "RCT and YCgCo require all the same signedness as input";
      //
      // If this is a chroma component, expand the bitdepth.
      if (m_Conversion == Delta_Trafo) {
	if (comp == mindepth - 1)
	  obits++;
      } else {
	if (comp > 0)
	  obits++;
      }
      bpc = (obits + 7) >> 3;
      mem = new UBYTE[w * h * bpc];
      //
      // Store the pointer to be able to release it later.
      membuf[comp]                         = mem;
      m_pComponent[comp].m_ucBits          = obits;
      m_pComponent[comp].m_bSigned         = (comp > 0)?(csign):(ysign);
      m_pComponent[comp].m_bFloat          = false;
      m_pComponent[comp].m_ulWidth         = w;
      m_pComponent[comp].m_ulHeight        = h;
      m_pComponent[comp].m_ulBytesPerPixel = bpc;
      m_pComponent[comp].m_ulBytesPerRow   = bpc * w;
      m_pComponent[comp].m_pPtr            = mem;
    } else {
      // Otherwise, just copy the data over.
      m_pComponent[comp].m_ucBits          = img->BitsOf(comp);
      m_pComponent[comp].m_bSigned         = img->isSigned(comp);
      m_pComponent[comp].m_bFloat          = img->isFloat(comp);
      m_pComponent[comp].m_ulWidth         = img->WidthOf(comp);
      m_pComponent[comp].m_ulHeight        = img->HeightOf(comp);
      m_pComponent[comp].m_ulBytesPerPixel = img->BytesPerPixel(comp);
      m_pComponent[comp].m_ulBytesPerRow   = img->BytesPerRow(comp);
      m_pComponent[comp].m_pPtr            = img->DataOf(comp);
    }
  }

  // If this includes a fourth component, it is one of the
  // transformations using delta. If so, preprocess
  // to compute the delta-component first. Delta is formed
  // between the two greens, i.e. source components 1 and 2.
  // The result goes into target components 1 and 3.
  if (bits <= 8) {
    if (bits + 1 <= 8) {
      if (ysign) {
	DispatchToRCT<BYTE,BYTE>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchToRCT<UBYTE,BYTE>(img,yoffset,coffset,w,h);
      } else {
	DispatchToRCT<UBYTE,UBYTE>(img,yoffset,coffset,w,h);
      }
    } else {
      if (ysign) {
	DispatchToRCT<BYTE,WORD>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchToRCT<UBYTE,WORD>(img,yoffset,coffset,w,h);
      } else {
	DispatchToRCT<UBYTE,UWORD>(img,yoffset,coffset,w,h);
      }
    }
  } else if (bits <= 16) {
    if (bits + 1 <= 16) {
      if (ysign) {
	DispatchToRCT<WORD,WORD>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchToRCT<UWORD,WORD>(img,yoffset,coffset,w,h);
      } else {
	DispatchToRCT<UWORD,UWORD>(img,yoffset,coffset,w,h);
      }
    } else {
      if (ysign) {
	DispatchToRCT<WORD,LONG>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchToRCT<UWORD,LONG>(img,yoffset,coffset,w,h);
      } else {
	DispatchToRCT<UWORD,ULONG>(img,yoffset,coffset,w,h);
      }
    }
  } else if (bits + 1 <= 32) {
    if (ysign) {
      DispatchToRCT<LONG,LONG>(img,yoffset,coffset,w,h);
    } else if (m_bMakeSigned) {
      DispatchToRCT<ULONG,LONG>(img,yoffset,coffset,w,h);
    } else {
      DispatchToRCT<ULONG,ULONG>(img,yoffset,coffset,w,h);
    }
  } else throw "unsupported source format";
}
///

/// YCbCr::DispatchToYCbCr
template<typename S,typename T>
void YCbCr::DispatchToYCbCr(const class ImageLayout *img,double yoffset,double coffset,
			    double ymin,double ymax,double cmin,double cmax,ULONG w,ULONG h)
{
  switch(m_Conversion) {
  case YCbCr_Trafo:     // this is actually a BT.601 conversion
    ToYCbCr<S,T>((S *)img->DataOf(0),(S *)img->DataOf(1),(S *)img->DataOf(2),
		 yoffset,coffset,ymin,ymax,cmin,cmax,
		 img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		 img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		 w,h);
    break;
  case YCbCr709_Trafo:  // This is the BT.709 conversion
    ToYCbCr709<S,T>((S *)img->DataOf(0),(S *)img->DataOf(1),(S *)img->DataOf(2),
		    yoffset,coffset,ymin,ymax,cmin,cmax,
		    img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		    img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		    w,h);
    break;
  case YCbCr2020_Trafo: // This is the transformation for BT.2020
    ToYCbCr2020<S,T>((S *)img->DataOf(0),(S *)img->DataOf(1),(S *)img->DataOf(2),
		     yoffset,coffset,ymin,ymax,cmin,cmax,
		     img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		     img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		     w,h);
    break;
  default:
    throw "unknown conversion specified";
  }
}
///

/// YCbCr::DispatchFromRCT
// This is a stub-function to simplify the dispatching of the conversion from RTC/YCgCo to RGB
template<typename S,typename T>
void YCbCr::DispatchFromRCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h,
			    LONG min,LONG max)
{
  switch(m_Conversion) {
  case RCT_Trafo:
    FromRCT<S,T>((const S *)img->DataOf(0),(const T *)img->DataOf(1),(const T *)img->DataOf(2),
		 (S *)this->DataOf(0),(S *)this->DataOf(1),(S *)this->DataOf(2),
		 yoffset,coffset,
		 img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		 img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		 this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(2),
		 this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(2),
		 w,h,min,max);
    break;
  case YCgCo_Trafo:
    FromYCgCo<S,T>((const S *)img->DataOf(0),(const T *)img->DataOf(1),(const T *)img->DataOf(2),
		   (S *)this->DataOf(0),(S *)this->DataOf(1),(S *)this->DataOf(2),
		   yoffset,coffset,
		   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		   this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(2),
		   this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(2),
		   w,h,min,max);
    break;
  case RCTD_Trafo:
    FromRCT<S,T>((const S *)img->DataOf(0),(const T *)img->DataOf(1),(const T *)img->DataOf(2),
		 (S *)this->DataOf(0),(S *)this->DataOf(1),(S *)this->DataOf(3),
		 yoffset,coffset,
		 img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		 img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		 this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(3),
		 this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(3),
		 w,h,min,max);
    FromDeltaGreen<S,T>((const S *)this->DataOf(1),(const T *)img->DataOf(3),
			(S *)this->DataOf(1),(S *)this->DataOf(2),
			coffset,
			this->BytesPerPixel(1),img->BytesPerPixel(3),
			this->BytesPerRow(1),img->BytesPerRow(3),
			this->BytesPerPixel(1),this->BytesPerPixel(2),
			this->BytesPerRow(1),this->BytesPerRow(2),
			w,h);
    break;
  case YCgCoD_Trafo:
     FromYCgCo<S,T>((const S *)img->DataOf(0),(const T *)img->DataOf(1),(const T *)img->DataOf(2),
		    (S *)this->DataOf(0),(S *)this->DataOf(1),(S *)this->DataOf(3),
		    yoffset,coffset,
		    img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		    img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		    this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(3),
		    this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(3),
		    w,h,min,max);
     FromDeltaGreen<S,T>((const S *)this->DataOf(1),(const T *)img->DataOf(3),
			 (S *)this->DataOf(1),(S *)this->DataOf(2),
			 coffset,
			 this->BytesPerPixel(1),img->BytesPerPixel(3),
			 this->BytesPerRow(1),img->BytesPerRow(3),
			 this->BytesPerPixel(1),this->BytesPerPixel(2),
			 this->BytesPerRow(1),this->BytesPerRow(2),
			 w,h);
    break;
  case Delta_Trafo:
    FromDeltaGreen<S,T>((const S *)img->DataOf(1),(const T *)img->DataOf(3),
			(S *)this->DataOf(1),(S *)this->DataOf(2),
			coffset,
			img->BytesPerPixel(1),img->BytesPerPixel(3),
			img->BytesPerRow(1),img->BytesPerRow(3),
			this->BytesPerPixel(1),this->BytesPerPixel(2),
			this->BytesPerRow(1),this->BytesPerRow(2),
			w,h);
    Copy<S>((const S *)img->DataOf(0),(S *)this->DataOf(0),
	    img->BytesPerPixel(0),img->BytesPerRow(0),
	    this->BytesPerPixel(0),this->BytesPerRow(0),
	    w,h);
    Copy<S>((const S *)img->DataOf(2),(S *)this->DataOf(3),
	    img->BytesPerPixel(2),img->BytesPerRow(2),
	    this->BytesPerPixel(3),this->BytesPerRow(3),
	    w,h);
    break;
  default:
    throw "unknown conversion specified";
  }
}
///

/// YCbCr::DispatchFromYCbCr
// The dispatcher for the reverse transformation direction.
template<typename S,typename T>
void YCbCr::DispatchFromYCbCr(const class ImageLayout *img,double yoffset,double coffset,double min,double max,ULONG w,ULONG h)
{
  switch(m_Conversion) {
  case YCbCr_Trafo:     // this is actually a BT.601 conversion
    FromYCbCr<S,T>((S *)img->DataOf(0),(T *)img->DataOf(1),(T *)img->DataOf(2),
		   yoffset,coffset,min,max,
		   img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		   img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		   w,h);
    break;
  case YCbCr709_Trafo:  // This is the BT.709 conversion
    FromYCbCr709<S,T>((S *)img->DataOf(0),(T *)img->DataOf(1),(T *)img->DataOf(2),
		      yoffset,coffset,min,max,
		      img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		      img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		      w,h);
    break;
  case YCbCr2020_Trafo: // This is the transformation for BT.2020
    FromYCbCr2020<S,T>((S *)img->DataOf(0),(T *)img->DataOf(1),(T *)img->DataOf(2),
		       yoffset,coffset,min,max,
		       img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		       img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		       w,h);
    break;
  default:
    throw "unknown conversion specified";
  }
}
///

/// YCbCr::FromRCT
// Convert back from RCT or YCgCo to RGB
void YCbCr::FromRCT(class ImageLayout *img,UBYTE *(&membuf)[4])
{
  LONG yoffset  = 0; // always zero
  LONG coffset  = 0; // the chroma component offset.
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  bool  ysign   = img->isSigned(0);
  bool  csign   = img->isSigned(1);
  UBYTE ybits   = img->BitsOf(0);
  UBYTE cbits   = img->BitsOf(1);
  LONG min,max;
  UWORD comp;
  UWORD mindepth;

  switch(m_Conversion) {
  case Delta_Trafo:
    cbits    = img->BitsOf(3);
    csign    = img->isSigned(3);
  case RCTD_Trafo:
  case YCgCoD_Trafo:
    mindepth = 4;
    break;
  default:
    mindepth = 3;
    break;
  }
  
  if (cbits != ybits + 1)
    throw "RCT and YCgCo input requires one additional bit in the chroma components";

  if (ysign && !csign)
    throw "RCT and YCgCo do not allow signed luma with unsigned chroma";
  
  CreateComponents(*img);

  // Add an inverse chroma shift in case chroma is unsigned.
  if (!csign)
    coffset = 1L << ybits;
  //
  // The signed-ness depends on the signedness of the y-coordinate,
  // and so does the clamping.
  if (ysign) {
    min = -(1L << (ybits - 1));
    max = -(min + 1);
  } else {
    min = 0;
    max = (1L << ybits) - 1;
  }
  
  // Now build the components. Floating point is not supported.
  // Everything else is copied over. The entry point tested that
  // at least three components are available. Note that the
  // chroma components must include one extra bit.
  for(comp = 0;comp < img->DepthOf();comp++) {
    if (comp < mindepth) {
      ULONG w     = img->WidthOf(comp);
      ULONG h     = img->HeightOf(comp);
      UBYTE bpc;
      UBYTE *mem;
      //
      // If this is float, then we are not supported.
      if (img->isFloat(comp))
	throw "RCT and YCgCo color spaces are not supported for floating point data";
      //
      // Depth must be all the same.
      if (m_Conversion != Delta_Trafo) {
	if (comp > 0 && cbits != img->BitsOf(comp))
	  throw "RCT and YCgCo require all the same chroma bit depths as input";
	if (comp > 0 && img->isSigned(comp) != csign)
	  throw "RCT and YCgCo require that all chroma components have the same signedness";
      }
      //
      bpc = (ybits + 7) >> 3;
      mem = new UBYTE[w * h * bpc];
      //
      // Store the pointer to be able to release it later.
      membuf[comp]                         = mem;
      m_pComponent[comp].m_ucBits          = ybits;
      m_pComponent[comp].m_bSigned         = ysign; // signed-ness comes from the luma component.
      m_pComponent[comp].m_bFloat          = false;
      m_pComponent[comp].m_ulWidth         = w;
      m_pComponent[comp].m_ulHeight        = h;
      m_pComponent[comp].m_ulBytesPerPixel = bpc;
      m_pComponent[comp].m_ulBytesPerRow   = bpc * w;
      m_pComponent[comp].m_pPtr            = mem;
    } else {
      // Otherwise, just copy the data over.
      m_pComponent[comp].m_ucBits          = img->BitsOf(comp);
      m_pComponent[comp].m_bSigned         = img->isSigned(comp);
      m_pComponent[comp].m_bFloat          = img->isFloat(comp);
      m_pComponent[comp].m_ulWidth         = img->WidthOf(comp);
      m_pComponent[comp].m_ulHeight        = img->HeightOf(comp);
      m_pComponent[comp].m_ulBytesPerPixel = img->BytesPerPixel(comp);
      m_pComponent[comp].m_ulBytesPerRow   = img->BytesPerRow(comp);
      m_pComponent[comp].m_pPtr            = img->DataOf(comp);
    }
  }

  if (ybits <= 8) {
    if (cbits <= 8) {
      if (ysign) {
	DispatchFromRCT<BYTE,BYTE>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFromRCT<UBYTE,BYTE>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFromRCT<UBYTE,UBYTE>(img,yoffset,coffset,w,h,min,max);
      }
    } else {
      if (ysign) {
	DispatchFromRCT<BYTE,WORD>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFromRCT<UBYTE,WORD>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFromRCT<UBYTE,UWORD>(img,yoffset,coffset,w,h,min,max);
      }
    }
  } else if (ybits <= 16) {
    if (cbits <= 16) {
      if (ysign) {
	DispatchFromRCT<WORD,WORD>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFromRCT<UWORD,WORD>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFromRCT<UWORD,UWORD>(img,yoffset,coffset,w,h,min,max);
      }
    } else {
      if (ysign) {
	DispatchFromRCT<WORD,LONG>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFromRCT<UWORD,LONG>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFromRCT<UWORD,ULONG>(img,yoffset,coffset,w,h,min,max);
      }
    }
  } else if (ybits <= 32) {
    if (ysign) {
      DispatchFromRCT<LONG,LONG>(img,yoffset,coffset,w,h,min,max);
    } else if (csign) {
      DispatchFromRCT<ULONG,LONG>(img,yoffset,coffset,w,h,min,max);
    } else {
      DispatchFromRCT<ULONG,ULONG>(img,yoffset,coffset,w,h,min,max);
    }
  } else throw "unsupported source format";
}
///

/// YCbCr::To422RCT
// Convert a 422 sampled image with the 422 RCT to YCbCr.
void YCbCr::To422RCT(class ImageLayout *img,UBYTE *(&membuf)[4])
{
  LONG yoffset  = 0; // always zero
  LONG coffset  = 0; // the chroma component offset.
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  bool  ysign   = img->isSigned(0);
  bool  csign   = img->isSigned(0) || m_bMakeSigned;
  UBYTE bits    = img->BitsOf(0);
  UWORD comp;
  int i;

  if (w & 1)
    throw "The 422RCT transformation does not support odd image widths";

  for(i = 1;i < 3;i++) {
    if (img->WidthOf(i) != (w >> 1) || img->HeightOf(i) != h)
      throw "Chroma components must be subsampled in horizontal direction for the 422 RCT";
  }
  
  CreateComponents(*img);
  
  if (!m_bMakeSigned) {
    coffset = 0 + ((ysign)?(0):(LONG(1) << (img->BitsOf(0))));
  }
  //
  // Now build the components. Floating point is not supported.
  // Everything else is copied over. The entry point tested that
  // at least three components are available.
  for(comp = 0;comp < img->DepthOf();comp++) {
    if (comp < 3) {
      ULONG w     = img->WidthOf(comp);
      ULONG h     = img->HeightOf(comp);
      UBYTE bits  = img->BitsOf(comp); // input bits.
      UBYTE obits = bits;
      UBYTE bpc;
      UBYTE *mem;
      //
      // If this is float, then we are not supported.
      if (img->isFloat(comp))
	throw "The 422RCT transformation does not support floating point data";
      //
      // Depth must be all the same.
      if (bits != img->BitsOf(0))
	throw "The 422RCT requires all input bith depths to be identical";
      if (img->isSigned(comp) != img->isSigned(0))
	throw "The 422RCT requires all input components to have the same signedness";
      //
      // Expand the bitdepths of all components. Otherwise, this transformation
      // would not be reversible.
      obits++;
      bpc = (obits + 7) >> 3;
      mem = new UBYTE[w * h * bpc];
      //
      // Store the pointer to be able to release it later.
      membuf[comp]                         = mem;
      m_pComponent[comp].m_ucBits          = obits;
      m_pComponent[comp].m_bSigned         = (comp > 0)?(csign):(ysign);
      m_pComponent[comp].m_bFloat          = false;
      m_pComponent[comp].m_ulWidth         = w;
      m_pComponent[comp].m_ulHeight        = h;
      m_pComponent[comp].m_ulBytesPerPixel = bpc;
      m_pComponent[comp].m_ulBytesPerRow   = bpc * w;
      m_pComponent[comp].m_pPtr            = mem;
    } else {
      // Otherwise, just copy the data over.
      m_pComponent[comp].m_ucBits          = img->BitsOf(comp);
      m_pComponent[comp].m_bSigned         = img->isSigned(comp);
      m_pComponent[comp].m_bFloat          = img->isFloat(comp);
      m_pComponent[comp].m_ulWidth         = img->WidthOf(comp);
      m_pComponent[comp].m_ulHeight        = img->HeightOf(comp);
      m_pComponent[comp].m_ulBytesPerPixel = img->BytesPerPixel(comp);
      m_pComponent[comp].m_ulBytesPerRow   = img->BytesPerRow(comp);
      m_pComponent[comp].m_pPtr            = img->DataOf(comp);
    }
  }

  if (bits <= 8) {
    if (bits + 1 <= 8) {
      if (ysign) {
	DispatchTo422RCT<BYTE,BYTE>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchTo422RCT<UBYTE,BYTE>(img,yoffset,coffset,w,h);
      } else {
	DispatchTo422RCT<UBYTE,UBYTE>(img,yoffset,coffset,w,h);
      }
    } else {
      if (ysign) {
	DispatchTo422RCT<BYTE,WORD>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchTo422RCT<UBYTE,WORD>(img,yoffset,coffset,w,h);
      } else {
	DispatchTo422RCT<UBYTE,UWORD>(img,yoffset,coffset,w,h);
      }
    }
  } else if (bits <= 16) {
    if (bits + 1 <= 16) {
      if (ysign) {
	DispatchTo422RCT<WORD,WORD>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchTo422RCT<UWORD,WORD>(img,yoffset,coffset,w,h);
      } else {
	DispatchTo422RCT<UWORD,UWORD>(img,yoffset,coffset,w,h);
      }
    } else {
      if (ysign) {
	DispatchTo422RCT<WORD,LONG>(img,yoffset,coffset,w,h);
      } else if (m_bMakeSigned) {
	DispatchTo422RCT<UWORD,LONG>(img,yoffset,coffset,w,h);
      } else {
	DispatchTo422RCT<UWORD,ULONG>(img,yoffset,coffset,w,h);
      }
    }
  } else if (bits + 1 <= 32) {
    if (ysign) {
      DispatchTo422RCT<LONG,LONG>(img,yoffset,coffset,w,h);
    } else if (m_bMakeSigned) {
      DispatchTo422RCT<ULONG,LONG>(img,yoffset,coffset,w,h);
    } else {
      DispatchTo422RCT<ULONG,ULONG>(img,yoffset,coffset,w,h);
    }
  } else throw "unsupported source format";
}
///

/// YCbCr::From422RCT
// Convert a YCbCr 422 sampled image with the 422 RCT to RGB.
void YCbCr::From422RCT(class ImageLayout *img,UBYTE *(&membuf)[4])
{
  LONG yoffset  = 0; // always zero
  LONG coffset  = 0; // the chroma component offset.
  ULONG w       = img->WidthOf(0);
  ULONG h       = img->HeightOf(0);
  bool  ysign   = img->isSigned(0);
  bool  csign   = img->isSigned(1);
  UBYTE ybits   = img->BitsOf(0);
  UBYTE cbits   = img->BitsOf(1);
  LONG min,max;
  UWORD comp;
  int i;
  
  if (cbits != ybits)
    throw "The 422RCT requires identical bitdepths for all components";

  if (ysign && !csign)
    throw "The 422RCT does not allow signed luma with unsigned chroma";

  if (w & 1)
    throw "The 422RCT transformation does not support odd image widths";

  for(i = 1;i < 3;i++) {
    if (img->WidthOf(i) != (w >> 1) || img->HeightOf(i) != h)
      throw "Chroma components must be subsampled in horizontal direction for the 422 RCT";
  }
  
  CreateComponents(*img);

  // Add an inverse chroma shift in case chroma is unsigned.
  if (!csign)
    coffset = (1L << ybits) >> 1;

  //
  // The signed-ness depends on the signedness of the y-coordinate,
  // and so does the clamping.
  if (ysign) {
    min = -(1L << (ybits - 2));
    max = -(min + 1);
  } else {
    min = 0;
    max = (1L << (ybits - 1)) - 1;
  }
  
  // Now build the components. Floating point is not supported.
  // Everything else is copied over. The entry point tested that
  // at least three components are available. Note that the
  // chroma components must include one extra bit.
  for(comp = 0;comp < img->DepthOf();comp++) {
    if (comp < 3) {
      ULONG w     = img->WidthOf(comp);
      ULONG h     = img->HeightOf(comp);
      UBYTE bpc;
      UBYTE *mem;
      //
      // If this is float, then we are not supported.
      if (img->isFloat(comp))
	throw "The 422RCT transformation does not support floating point data";
      //
      // Depth must be all the same.
      if (cbits != img->BitsOf(comp))
	throw "The 422RCT requires the same bith depths for all components";
      if (comp > 0 && img->isSigned(comp) != csign)
	throw "The 422RCT requires that all chroma components have the same signedness";
      //
      bpc = (ybits - 1 + 7) >> 3;
      mem = new UBYTE[w * h * bpc];
      //
      // Store the pointer to be able to release it later.
      membuf[comp]                         = mem;
      m_pComponent[comp].m_ucBits          = ybits - 1;
      m_pComponent[comp].m_bSigned         = ysign; // signed-ness comes from the luma component.
      m_pComponent[comp].m_bFloat          = false;
      m_pComponent[comp].m_ulWidth         = w;
      m_pComponent[comp].m_ulHeight        = h;
      m_pComponent[comp].m_ulBytesPerPixel = bpc;
      m_pComponent[comp].m_ulBytesPerRow   = bpc * w;
      m_pComponent[comp].m_pPtr            = mem;
    } else {
      // Otherwise, just copy the data over.
      m_pComponent[comp].m_ucBits          = img->BitsOf(comp);
      m_pComponent[comp].m_bSigned         = img->isSigned(comp);
      m_pComponent[comp].m_bFloat          = img->isFloat(comp);
      m_pComponent[comp].m_ulWidth         = img->WidthOf(comp);
      m_pComponent[comp].m_ulHeight        = img->HeightOf(comp);
      m_pComponent[comp].m_ulBytesPerPixel = img->BytesPerPixel(comp);
      m_pComponent[comp].m_ulBytesPerRow   = img->BytesPerRow(comp);
      m_pComponent[comp].m_pPtr            = img->DataOf(comp);
    }
  }

  if (ybits <= 8) {
    if (cbits <= 8) {
      if (ysign) {
	DispatchFrom422RCT<BYTE,BYTE>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFrom422RCT<UBYTE,BYTE>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFrom422RCT<UBYTE,UBYTE>(img,yoffset,coffset,w,h,min,max);
      }
    } else {
      if (ysign) {
	DispatchFrom422RCT<BYTE,WORD>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFrom422RCT<UBYTE,WORD>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFrom422RCT<UBYTE,UWORD>(img,yoffset,coffset,w,h,min,max);
      }
    }
  } else if (ybits <= 16) {
    if (cbits <= 16) {
      if (ysign) {
	DispatchFrom422RCT<WORD,WORD>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFrom422RCT<UWORD,WORD>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFrom422RCT<UWORD,UWORD>(img,yoffset,coffset,w,h,min,max);
      }
    } else {
      if (ysign) {
	DispatchFrom422RCT<WORD,LONG>(img,yoffset,coffset,w,h,min,max);
      } else if (csign) {
	DispatchFrom422RCT<UWORD,LONG>(img,yoffset,coffset,w,h,min,max);
      } else {
	DispatchFrom422RCT<UWORD,ULONG>(img,yoffset,coffset,w,h,min,max);
      }
    }
  } else if (ybits <= 32) {
    if (ysign) {
      DispatchFrom422RCT<LONG,LONG>(img,yoffset,coffset,w,h,min,max);
    } else if (csign) {
      DispatchFrom422RCT<ULONG,LONG>(img,yoffset,coffset,w,h,min,max);
    } else {
      DispatchFrom422RCT<ULONG,ULONG>(img,yoffset,coffset,w,h,min,max);
    }
  } else throw "unsupported source format";
}
///

/// YCbCr::DispatchTo422RCT
// The dispatcher for the 422RCT (a weirdo)
template<typename S,typename T>
void YCbCr::DispatchTo422RCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h)
{
  switch(m_Conversion) {
  case RCT422_Trafo:
    To422RCT<S,T>((const S *)img->DataOf(1),(const S *)img->DataOf(0),(const S *)img->DataOf(2),
		  (S *)this->DataOf(0),(T *)this->DataOf(1),(T *)this->DataOf(2),
		  yoffset,coffset,
		  img->BytesPerPixel(1),img->BytesPerPixel(0),img->BytesPerPixel(2),
		  img->BytesPerRow(1)  ,img->BytesPerRow(0)  ,img->BytesPerRow(2),
		  this->BytesPerPixel(0),this->BytesPerPixel(1),this->BytesPerPixel(2),
		  this->BytesPerRow(0)  ,this->BytesPerRow(1)  ,this->BytesPerRow(2),
		  w,h);
    break;
  default:
    throw "unknown conversion specified";
  }
}
///

/// YCbCr::DispatchFrom422RCT
// The dispatcher for the reverse transformation direction.
template<typename S,typename T>
void YCbCr::DispatchFrom422RCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h,
			       LONG min,LONG max)
{
  switch(m_Conversion) {
  case RCT422_Trafo:
    From422RCT<S,T>((const S *)img->DataOf(0),(const T *)img->DataOf(1),(const T *)img->DataOf(2),
		    (S *)this->DataOf(1),(S *)this->DataOf(0),(S *)this->DataOf(2),
		    yoffset,coffset,
		    img->BytesPerPixel(0),img->BytesPerPixel(1),img->BytesPerPixel(2),
		    img->BytesPerRow(0)  ,img->BytesPerRow(1)  ,img->BytesPerRow(2),
		    this->BytesPerPixel(1),this->BytesPerPixel(0),this->BytesPerPixel(2),
		    this->BytesPerRow(1)  ,this->BytesPerRow(0)  ,this->BytesPerRow(2),
		    w,h,min,max);
    break;
  default:
    throw "unknown conversion specified";
  }
}
///

/// YCbCr::Measure
double YCbCr::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  int i;
  UWORD mindepth;

  switch(m_Conversion) {
  case RCTD_Trafo:
  case YCgCoD_Trafo:
  case Delta_Trafo:
    mindepth = 4;
    break;
  default:
    mindepth = 3;
    break;
  }
  
  if (src->DepthOf() < mindepth || dst->DepthOf() < mindepth)
    throw "insufficient number of components for color space conversion";

  // Check the dimensions of the components. Must all have the same size for all
  // transformations but the 422 trafo, there we test separately.
  if (m_Conversion != RCT422_Trafo) {
    for(i = 0;i < mindepth;i++) {
      if (src->WidthOf(i) != src->WidthOf(0) || src->HeightOf(i) != src->HeightOf(0) ||
	  dst->WidthOf(i) != dst->WidthOf(0) || dst->HeightOf(i) != dst->HeightOf(0))
	throw "component dimensions differ, probably due to subsampling. Use --cup to upsample chroma first";
    }
  }
  
  switch(m_Conversion) {
  case YCbCr_Trafo:
  case YCbCr709_Trafo:
  case YCbCr2020_Trafo:
    if (m_bInverse) {
      FromYCbCr(src);
      FromYCbCr(dst);
    } else {
      ToYCbCr(src);
      ToYCbCr(dst);
    }
    break;
  case RCTD_Trafo:
  case YCgCoD_Trafo:
  case Delta_Trafo:
  case RCT_Trafo:
  case YCgCo_Trafo:
    if (m_bInverse) {
      FromRCT(src,m_ppucSrcImage);
      Swap(*src);
      FromRCT(dst,m_ppucDstImage);
      Swap(*dst);
    } else {
      ToRCT(src,m_ppucSrcImage);
      Swap(*src);
      ToRCT(dst,m_ppucDstImage);
      Swap(*dst);
    }
    break;
  case RCT422_Trafo:
    if (m_bInverse) {
      From422RCT(src,m_ppucSrcImage);
      Swap(*src);
      From422RCT(dst,m_ppucDstImage);
      Swap(*dst);
    } else {
      To422RCT(src,m_ppucSrcImage);
      Swap(*src);
      To422RCT(dst,m_ppucDstImage);
      Swap(*dst);
    }
    break;
  default:
    throw "unknown conversion type specified";
  }
  return in;
}
///

