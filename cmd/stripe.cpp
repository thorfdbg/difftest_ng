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
 * Add an almost invisible striping to an image file to create a distortion.
 * 
 * $Id: stripe.cpp,v 1.4 2016/06/04 10:44:08 thor Exp $
 */

/// Includes
#include "img/imglayout.hpp"
#include "std/assert.hpp"
#include <new>
///

/// Stripe
template<typename T>
void StripeImage(T *data,ULONG w,ULONG h,ULONG bpp,ULONG bpr)
{
  ULONG x,y;

  for(y = 0;y < h;y++) {
    T *d = data;
    for(x = 0;x < w;x += 32) {
      *d &= 0xfffc;
      d   = (T *)(((UBYTE *)d) + (32 * bpp));
    }
    data = (T *)(((UBYTE *)data) + bpr);
  }
}
///

/// main
int main(int argc,char **argv)
{ 
  class ImageLayout *orgimg = NULL;
  const char *org = NULL;
  const char *dst = NULL;
  int rc = 0;
  
  try {
    if (argc == 3) {
      org = argv[1];
      dst = argv[2];
    } else {
      throw "requires exactly two mandatory arguments, original and destination image";
    }
    assert(org && dst);
    orgimg = ImageLayout::LoadImage(org);
    if (orgimg->isSigned(0)) {
      if (orgimg->BitsOf(0) > 8) {
	WORD *data = (WORD *)orgimg->DataOf(0);
	StripeImage(data,orgimg->WidthOf(0),orgimg->HeightOf(0),orgimg->BytesPerPixel(0),orgimg->BytesPerRow(0));
      } else {
	BYTE *data = (BYTE *)orgimg->DataOf(0);
	StripeImage(data,orgimg->WidthOf(0),orgimg->HeightOf(0),orgimg->BytesPerPixel(0),orgimg->BytesPerRow(0));
      }
    } else {
      if (orgimg->BitsOf(0) > 8) {
	UWORD *data = (UWORD *)orgimg->DataOf(0);
	StripeImage(data,orgimg->WidthOf(0),orgimg->HeightOf(0),orgimg->BytesPerPixel(0),orgimg->BytesPerRow(0));
      } else {
	UBYTE *data = (UBYTE *)orgimg->DataOf(0);
	StripeImage(data,orgimg->WidthOf(0),orgimg->HeightOf(0),orgimg->BytesPerPixel(0),orgimg->BytesPerRow(0));
      }
    }
    orgimg->SaveImage(dst);
  } catch(const char *error) {
    fprintf(stderr,"Program failed: %s\n",error);
    rc = 10;
  } catch(const std::bad_alloc &err) {
    fprintf(stderr,"Program run out of memory\n");
    rc = 15;
  } catch(...) {
    fprintf(stderr,"Caught unknown exception\n");
    rc = 20;
  }

  if (orgimg) 
    delete orgimg;

  return rc;
}
///
