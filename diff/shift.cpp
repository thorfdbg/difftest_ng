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
** $Id: shift.cpp,v 1.6 2020/09/15 09:45:49 thor Exp $
**
** This class shifts images in X or Y direction.
*/

/// Includes
#include "img/imglayout.hpp"
#include "diff/shift.hpp"
///

/// Shift::shiftRight
// Templated implementations: Shift the image horizontally to the right
template<typename T>
void Shift::shiftRight(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx)
{
  ULONG y;

  for(y = 0;y < h;y++) {
    T *left = org;
    T *src  = (T *)((UBYTE *)(org) + obytesperpixel * (w - dx - 1));
    T *dst  = (T *)((UBYTE *)(org) + obytesperpixel * (w - 1 ));
    while(src >= left) {
      *dst = *src;
      src  = (T *)((UBYTE *)(src) - obytesperpixel);
      dst  = (T *)((UBYTE *)(dst) - obytesperpixel);
    }
    while(dst >= left) {
      *dst = boundary;
      dst  = (T *)((UBYTE *)(dst) - obytesperpixel);
    }
    org = (T *)((UBYTE *)(org) + obytesperrow);
  }
}
///

/// Shift::shiftLeft
// Templated implementations: Shift the image horizontally to the left
template<typename T>
void Shift::shiftLeft(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dx)
{
  ULONG y;

  for(y = 0;y < h;y++) {
    T *right = (T *)((UBYTE *)(org) + obytesperpixel * w);
    T *src   = (T *)((UBYTE *)(org) + obytesperpixel * dx);
    T *dst   = org;
    while(src < right) {
      *dst = *src;
      src  = (T *)((UBYTE *)(src) + obytesperpixel);
      dst  = (T *)((UBYTE *)(dst) + obytesperpixel);
    }
    while(dst < right) {
      *dst = boundary;
      dst  = (T *)((UBYTE *)(dst) + obytesperpixel);
    }
    org = (T *)((UBYTE *)(org) + obytesperrow);
  }
}
///

/// Shift::shiftDown
// Templated implementations: Shift the image vertically down
template<typename T>
void Shift::shiftDown(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dy)
{
  ULONG y;

  for(y = h - 1;y >= (ULONG)dy;y--) {
    T *src   = (T *)((UBYTE *)(org) + obytesperrow * (y - dy));
    T *dst   = (T *)((UBYTE *)(org) + obytesperrow * y);
    T *right = (T *)((UBYTE *)(dst) + obytesperpixel * w);
    while(dst < right) {
      *dst = *src;
      src  = (T *)((UBYTE *)(src) + obytesperpixel);
      dst  = (T *)((UBYTE *)(dst) + obytesperpixel);
    }
  }
  do {
    T *dst   = (T *)((UBYTE *)(org) + obytesperrow * y);
    T *right = (T *)((UBYTE *)(dst) + obytesperpixel * w);
    while(dst < right) {
      *dst = boundary;
      dst  = (T *)((UBYTE *)(dst) + obytesperpixel);
    }
  } while(y--);
}
///

/// Shift::shiftUp
// Templated implementations: Shift the image vertically up
template<typename T>
void Shift::shiftUp(T *org,T boundary,ULONG obytesperpixel,ULONG obytesperrow,ULONG w,ULONG h,int dy)
{
  ULONG y;

  for(y = 0;y < h - dy;y++) {
    T *src   = (T *)((UBYTE *)(org) + obytesperrow   * (y + dy));
    T *dst   = (T *)((UBYTE *)(org) + obytesperrow   * y);
    T *right = (T *)((UBYTE *)(dst) + obytesperpixel * w);
    while(dst < right) {
      *dst = *src;
      src  = (T *)((UBYTE *)(src) + obytesperpixel);
      dst  = (T *)((UBYTE *)(dst) + obytesperpixel);
    }
  }
  while(y < h) {
    T *dst   = (T *)((UBYTE *)(org) + obytesperrow   * y);
    T *right = (T *)((UBYTE *)(dst) + obytesperpixel * w);
    while(dst < right) {
      *dst = boundary;
      dst  = (T *)((UBYTE *)(dst) + obytesperpixel);
    }
    y++;
  }
}
///

/// Shift::shift
// Shift the image in the given direction
void Shift::shift(class ImageLayout *img,bool isyuv) const
{
  UWORD comp,d  = img->DepthOf();
  
  for(comp = 0;comp < d;comp++) {
    ULONG  w = img->WidthOf(comp);
    ULONG  h = img->HeightOf(comp);
    UBYTE sx = img->SubXOf(comp);
    UBYTE sy = img->SubYOf(comp);
    int   dx = this->dx;
    int   dy = this->dy;
    bool yuv = (isyuv && (comp == 1 || comp == 2) && img->isSigned(comp) == false)?true:false;
    UBYTE bd = img->BitsOf(comp);
    //
    if ((dx % sx) || (dy % sy))
      throw "shift distance is not divisible by the subsampling factors, cannot perform the shift";
    //
    dx /= sx;
    dy /= sy;
    //
    if (img->BitsOf(comp) <= 8) {
      UBYTE neutral = (yuv)?(1 << (bd - 1)):(0);
      Shift::shift<UBYTE>((UBYTE *)(img->DataOf(comp)),neutral,img->BytesPerPixel(comp),img->BytesPerRow(comp),
			  w,h,dx,dy);
    } else if (!img->isFloat(comp) && img->BitsOf(comp) <= 16) { // 16 bit float is internally stored as 32 bit.
      UWORD neutral = (yuv)?(1 << (bd - 1)):(0);
      Shift::shift<UWORD>((UWORD *)(img->DataOf(comp)),neutral,img->BytesPerPixel(comp),img->BytesPerRow(comp),
			  w,h,dx,dy);
    } else if (img->BitsOf(comp) <= 32) {
      if (img->isFloat(comp)) {
	// This is stored in float, actually.
	FLOAT neutral = (yuv)?(0.5f):(0.0f);
	Shift::shift<FLOAT>((FLOAT *)(img->DataOf(comp)),neutral,img->BytesPerPixel(comp),img->BytesPerRow(comp),
			    w,h,dx,dy);
      } else {
	ULONG neutral = (yuv)?(1 << (bd - 1)):(0);
	Shift::shift<ULONG>((ULONG *)(img->DataOf(comp)),neutral,img->BytesPerPixel(comp),img->BytesPerRow(comp),
			    w,h,dx,dy);
      }
    } else if (img->BitsOf(comp) <= 64 && img->isFloat(comp)) {
      DOUBLE neutral = (yuv)?(0.5):(0.0);
      Shift::shift<DOUBLE>((DOUBLE *)(img->DataOf(comp)),neutral,img->BytesPerPixel(comp),img->BytesPerRow(comp),
			  w,h,dx,dy);
    } else {
      throw "unsupported data type";
    }
  }
}
///

/// Shift::Measure
double Shift::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  shift(src,m_bSrcIsYUV);
  shift(dst,m_bDstIsYUV);

  return in;
}
///
