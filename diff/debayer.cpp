/*************************************************************************
** Copyright (c) 2003-2010 Accusoft/Pegasus 				**
** THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE				**
**									**
** Written by Thomas Richter (THOR Software) for Accusoft/Pegasus	**
** All Rights Reserved							**
*************************************************************************/

/*
**
** $Id: convertimg.hpp,v 1.2 2011-03-09 15:52:42 thor Exp $
**
** This class runs a debayer filter on the image converting it from grey scale
** to RGB. This is mostly experimental.
*/

/// Includes
#include "diff/debayer.hpp"
///

/// Debayer::~Debayer
Debayer::~Debayer(void)
{
  int i;

  for(i = 0;i < 3;i++) {
    delete[] (UBYTE *)(m_pImage[i]);
  }
}
///

/// Debayer::DebayerImg
// The actual debayer algorithm. The bayer pattern is here (currently) hardcoded to grbg
template<typename T,typename S>
void Debayer::DebayerImg<T,S>(const T *src,LONG bytesperpixel,LONG bytesperrow,ULONG width,ULONG height,
			      T *r,T *g,T *b,S min,S max)
{
#define AT(p,x,y) (*(const *T)(((const UBYTE *)p) + (x) * bytesperrow + (y) * bytesperpixel))
#define ABS(x)    ((x) > 0)?(x):(-(x));
  ULONG x,y;

  for(y = 0;y < h;y += 2) {
    const T *row  = src;
    T *rrow = r;
    T *grow = g;
    T *brow = b;
    for(x = 0;x < w;x += 2) {
      // Step 1: Fill in the green pixels we have.
      grow[0] = *row;
      if (x + 1 < w && y + 1 < h) grow[1 + w] = AT(row,1,1);
      //
      // Estimate g in the top right corner
      if (x + 1 < w) {
	// calculate horizontal gradient
	S g4 = *row;
	S r5 = (x + 1 < w)?AT(row, 1,0):g4;
	S g6 = (x + 2 < w)?AT(row, 2,0):r5;
	S r3 = (x     > 0)?AT(row,-1,0):g4;
	S r7 = (x + 3 < w)?AT(row, 3,0):g6;
	S g2 = (x + 1 < w && y     > 0)?AT(row,1,-1):r5;
	S g8 = (x + 1 < w && y + 1 < h)?AT(row,1, 1):r5;
	S r1 = (x + 1 < w && y     > 2)?AT(row,1,-3):g2;
	S r9 = (x + 1 < w && y +3  < h)?AT(row,1, 3):g8;
	//
	// Estimate g at the r-position.
	S dh = ABS(g4 - g6) + ABS(r5 - r3 + r5 - r7);
	S dv = ABS(g2 - g8) + ABS(r5 - r1 + r5 - r9);
	S g5;
	if (dh > dv) {
	  g5 = (g4 + g8)/2 + (r5 - r1 + r5 - r9)/4;
	} else if (dh < dv) {
	  g5 = (g4 + g6)/2 + (r5 - r3 + r5 - r7)/4;
	} else {
	  g5 = (g2 + g8 + g4 + g6)/4 + (r5 - r1 + r5 - r9 + r5 - r3 + r5 - r7)/8;
	}
	if (g5 < min) g5 = min;
	if (g5 > max) g5 = max;
	grow[1] = g5;
      }
      //
      // Estimate g in the bottom left corner
      if (y + 1 < h) {
	S g2 = *row;
	S b5 = (y + 1 < h)?(AT(row,0 ,1)):g2;
	S g8 = (y + 2 < h)?(AT(row,0 ,2)):b5;
	S b9 = (y + 3 < h)?(AT(row,0 ,3)):g8;
	S b1 = (y     > 0)?(AT(row,0,-1)):g2;
	S g4 = (y + 1 < h && x > 0)?(AT(row,-1,1)):b5;
	S b3 = (y + 1 < h && x > 1)?(AT(row,-2,1)):g4;
	S g6 = (y + 1 < h && x + 1 < w)?(AT(row,1,1)):b5;
	S b7 = (y + 1 < h && x + 2 < w)?(AT(row,2,1)):g6;
	//
	// Estimate g at the r-position.
	S dh = ABS(g4 - g6) + ABS(b5 - b3 + b5 - b7);
	S dv = ABS(g2 - g8) + ABS(b5 - b1 + b5 - b9);
	S g5;
	if (dh > dv) {
	  g5 = (g4 + g8)/2 + (b5 - b1 + b5 - b9)/4;
	} else if (dh < dv) {
	  g5 = (g4 + g6)/2 + (b5 - b3 + b5 - b7)/4;
	} else {
	  g5 = (g2 + g8 + g4 + g6)/4 + (b5 - b1 + b5 - b9 + b5 - b3 + b5 - b7)/8;
	}
	if (g5 < min) g5 = min;
	if (g5 > max) g5 = max;
	grow[w] = g5;
      }
      //
      // Green is done. Fill in the red channel.
      if (x + 1 < w) {
	rrow[1] = AT(row,1,0);
      }
      //
      // Red is done. Fill in the blue channel.
      if (y + 1 < h) {
	brow[1] = AT(row,0,1);
      }
      //
      // Increment the buffer positions.
      rrow++;
      grow++;
      brow++;
      row = (const *T)(((const UBYTE *)row) + bytesperpixel);
    }
    // Advance to the next row.
    r += w;
    g += w;
    b += w;
    src = (const *T)(((const UBYTE *)src) + bytesperrow);
  }
}
///

/// Debayer::Measure
double Debayer::Measure(class ImageLayout *src,class ImageLayout *,double in)
{
  int i;
  UBYTE bits;
  UBYTE bpp;
  ULONG width  = src->WidthOf();
  ULONG height = src->HeightOf();
  
  if (src->DepthOf() != 1)
    throw "Source image to be de-mosaiked must have only one component";

  bits = src->BitsOf(0);
  if (src->isFloat() && bits == 16) {
    // Is actually IEEE single precision.
    bits = 32;
  }
  bpp = (bits + 7) >> 3;

  CreateComponents(width,height,3); // the original depth must be one.
  
  for(i = 0;i < 3;i++) {
    m_pImage[i]                       = new UBYTE[bpp * width * height];
    m_pComponent[i].m_pPtr            = m_pImage[i];
    m_pComponent[i].m_ulBytesPerPixel = bpp;
    m_pComponent[i].m_ulBytesPerRow   = bpp * width;
  }
  
  if (src->isFloat(0)) {
    switch(src->BitsOf(0)) {
    case 16:
    case 32:
      DebayerImg<FLOAT,FLOAT>((FLOAT *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
			      (FLOAT *)m_pImage[0],(FLOAT *)m_pImage[1],(FLOAT *)m_pImage[2],-HUGE_VAL,HUGE_VAL);
      break;
    case 32:
      DebayerImg<DOUBLE,DOUBLE>((DOUBLE *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
				(DOUBLE *)m_pImage[0],(DOUBLE *)m_pImage[1],(DOUBLE *)m_pImage[2],-HUGE_VAL,HUGE_VAL);
      break;
    default:
      throw "unsupported source pixel type";
      break;
    }
  } else {
    if (src->isSigned(0)) {
      QUAD min = -(QUAD(1) << (src->BitsOf(0)-1));
      QUAD max = +(QUAD(1) << (src->BitsOf(1)-1)) - 1;
      switch((src->BitsOf(0) + 7) & -8) {
      case 8:
	DebayerImg<BYTE,LONG>((BYTE *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
			      (BYTE *)m_pImage[0],(BYTE *)m_pImage[1],(BYTE *)m_pImage[2],min,max);
	break;
      case 16:
	DebayerImg<WORD,LONG>((WORD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
			      (WORD *)m_pImage[0],(WORD *)m_pImage[1],(WORD *)m_pImage[2],min,max);
	break;
      case 32:
	DebayerImg<LONG,QUAD>((LONG *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
			      (LONG *)m_pImage[0],(LONG *)m_pImage[1],(LONG *)m_pImage[2],min,max);
	break;
      case 64:
	DebayerImg<QUAD,QUAD>((QUAD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
			      (QUAD *)m_pImage[0],(QUAD *)m_pImage[1],(QUAD *)m_pImage[2],min,max);
	break;
      default:
	throw "unsupported source pixel type";
	break;
      }
    } else {
      UQUAD max = +(UQUAD(1) << (src->BitsOf(1))) - 1;
      switch((src->BitsOf(0) + 7) & -8) {
      case 8:
	DebayerImg<UBYTE,ULONG>((UBYTE *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
				(UBYTE *)m_pImage[0],(UBYTE *)m_pImage[1],(UBYTE *)m_pImage[2],min,max);
	break;
      case 16:
	DebayerImg<UWORD,ULONG>((UWORD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
				(UWORD *)m_pImage[0],(UWORD *)m_pImage[1],(UWORD *)m_pImage[2],min,max);
	break;
      case 32:
	DebayerImg<ULONG,UQUAD>((ULONG *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
				(ULONG *)m_pImage[0],(ULONG *)m_pImage[1],(ULONG *)m_pImage[2],min,max);
	break;
      case 64:
	DebayerImg<UQUAD,UQUAD>((UQUAD *)(src->DataOf(0)),src->BytesPerPixel(0),src->BytesPerRow(0),width,height,
				(UQUAD *)m_pImage[0],(UQUAD *)m_pImage[1],(UQUAD *)m_pImage[2],min,max);
	break;
      default:
	throw "unsupported source pixel type";
	break;
      }
    }
  }

  SaveImage(m_pcTargetFile,m_TargetSpecs);
  
  return in;
}
///
