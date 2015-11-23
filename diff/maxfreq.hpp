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
** $Id: maxfreq.hpp,v 1.5 2014/01/04 11:35:28 thor Exp $
**
** This class locates the maximum frequency and returns the absolute value of this frequency
** as result.
*/

#ifndef DIFF_MAXFREQ_HPP
#define DIFF_MAXFREQ_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#ifdef USE_GSL
///

/// Forwards
class FFT;
///

/// class MaxFreq
// This class saves the FFT of the difference image as a normalized 8bpp image
// with the same number of components as the original.
class MaxFreq : public Meter {
  //
  // The FFTs used here.
  class FFT **m_ppFFT;
  //
  // Number of FFT components.
  UWORD       m_usDepth;
  //
  // grey-scale absolute value FFT.
  double     *m_pdAbs;
  //
  // The type of measurement.
  int         m_Type;
  //
  template<typename T>
  void CopyToFFT(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
		 T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		 double *trg  ,ULONG stide,ULONG w,ULONG h);
  //
public:
  //
  // What should be measured.
  enum Type {
    MaxH,   // horizontal
    MaxV,   // vertical frequency
    MaxR,   // Radial frequency
    Var,    // maximum variance found
    Pattern // Pattern index
  };
  //
  // Construct the difference image. Takes a file name.
  MaxFreq(Type t)
    : m_ppFFT(NULL), m_usDepth(0), m_pdAbs(NULL), m_Type(t)
  {
  }
  //
  virtual ~MaxFreq(void);
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    switch(m_Type) {
    case MaxH:
      return "MaxHorizFreq";
    case MaxV:
      return "MaxVertFreq";
    case MaxR:
      return "MaxRadialFreq";
    case Var:
      return "MaxVariance";
    case Pattern:
      return "PatternIndex";
    }
    return NULL;
  }
};
///

///
#endif
#endif
