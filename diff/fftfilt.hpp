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
**
** $Id: fftfilt.hpp,v 1.7 2016/06/04 10:44:08 thor Exp $
**
** This class filters the image with an FFT and selects only frequences around a horizontal and
** vertical base frequency and a given radius.
*/

#ifndef DIFF_FFTFILT_HPP
#define DIFF_FFTFILT_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#ifdef USE_GSL
///

/// Forwards
class FFT;
///

/// class FFTFilt
// This class filters the image with an FFT and selects only frequences around a horizontal and
// vertical base frequency and a given radius.
class FFTFilt : public Meter, private ImageLayout {
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
  // Center and radius of the region to filter.
  ULONG       m_ulXC,m_ulYC,m_ulRadius;
  //
  // The filter itself.
  double     *m_pdFilter;
  //
  // Normalize the result?
  bool        m_bNormalize;
  //
  // The comb filter direction and period, if set.
  ULONG       m_ulCombX,m_ulCombY;
  //
  template<typename T>
  void CopyToFFT(T *org       ,ULONG obytesperpixel,ULONG obytesperrow,
		 T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		 double *trg  ,ULONG stide,ULONG w,ULONG h);
  //
  template<typename T>
  void CopyFromFFT(double *src,ULONG stride,ULONG w,ULONG h,
		   T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		   double min,double max,double scale,double shift);
  //
  // Create a filter in the given array with the given stride variable around the
  // given center with the given radius
  static void CreateFilter(double *filt,ULONG stride,ULONG w,ULONG h,ULONG xc,ULONG yc,ULONG xr,ULONG yr);
  //
  // Normalize the filter for l^1-norm 1.
  static void NormalizeFilter(double *filt,ULONG stride,ULONG w,ULONG h);
  //
  // Update minimum and maximum over the FFT output.
  static void FindMinMax(double *fft,ULONG stride,ULONG w,ULONG h,double &min,double &max);
  //
public:
  //
  // Construct the difference image. Takes a file name.
  FFTFilt(const char *filename,ULONG xc,ULONG yc,ULONG radius,bool normalize,
	  ULONG combx,ULONG comby)
    : m_pTargetFile(filename), m_ppucImage(NULL), m_ppFFT(NULL), 
      m_ulXC(xc), m_ulYC(yc), m_ulRadius(radius), m_pdFilter(NULL), m_bNormalize(normalize),
      m_ulCombX(combx), m_ulCombY(comby)
  {
  }
  //
  virtual ~FFTFilt(void);
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
