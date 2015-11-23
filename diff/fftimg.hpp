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
** $Id: fftimg.hpp,v 1.6 2014/01/04 11:35:28 thor Exp $
**
** This class saves the FFT of the difference image as a normalized 8bpp image
** with the same number of components as the original.
*/

#ifndef DIFF_FFTIMG_HPP
#define DIFF_FFTIMG_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#ifdef USE_GSL
///

/// Forwards
class FFT;
///

/// class FFTImg
// This class saves the FFT of the difference image as a normalized 8bpp image
// with the same number of components as the original.
class FFTImg : public Meter, private ImageLayout {
  //
  // The file name under which the difference image shall be saved.
  const char *m_pTargetFile;
  //
  // The component memory itself.
  UBYTE     **m_ppucImage;
  //
  // The FFTs used here.
  class FFT **m_ppFFT;
  //
  // The window-flag for the FFT.
  bool        m_bWindow;
  //
  template<typename T>
  void CopyToFFT(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
		 T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		 double *trg  ,ULONG stide,ULONG w,ULONG h);
  //
public:
  //
  // Construct the difference image. Takes a file name.
  FFTImg(const char *filename,bool window)
    : m_pTargetFile(filename), m_ppucImage(NULL), m_ppFFT(NULL), m_bWindow(window)
  {
  }
  //
  virtual ~FFTImg(void);
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    return NULL;
  }
};
///

///
#endif
#endif
