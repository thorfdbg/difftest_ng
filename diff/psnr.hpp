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
** $Id: psnr.hpp,v 1.14 2020/09/15 09:45:49 thor Exp $
**
** This class measures the PSNR between two images, averaged over all samples
** and thus all components.
*/

#ifndef DIFF_PSNR_HPP
#define DIFF_PSNR_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class PSNR
class PSNR : public Meter {
  //
  // PSNR measurement type
  int  m_Type;
  //
  // Print result in log space/dB (false) or in linear space (true)
  bool m_bLinear;
  //
  // Print result in SNR (scale to max) rather than PSNR (scale to nominal max).
  bool m_bSNR;
  //
  // Instead of taking the max in the SNR computation, compute the energy of the source.
  bool m_bScaleToEnergy;
  //
  // Templated implementations
  template<typename T>
  double MSE(T *org,ULONG obytesperpixel,ULONG obytesperrow,
	     T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
	     ULONG w,ULONG h,double &max,double &energy);
public:
  //
  // Several options: Mean PSNR, minimum PSNR, and with YCbCr weights (yuck!)
  enum Type {
    Mean,
    Min,
    YCbCr,
    YUV,
    SamplingWeighted,
    RootMean
  };
  //
  PSNR(Type t,bool linear = false,bool snr = false,bool fromenergy = false)
    : m_Type(t), m_bLinear(linear), m_bSNR(snr), m_bScaleToEnergy(fromenergy)
  { }
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    if (m_bLinear) {
      if (m_Type == RootMean)
	return "RMSE";
      return "MSE";
    }
    if (m_bSNR)
      return "SNR";
    return "PSNR";
  }
};
///

///
#endif
