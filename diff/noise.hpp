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
** $Id: noise.hpp,v 1.1 2026/06/18 08:38:22 thor Exp $
**
** This class adds Gaussian or Poisson noise to images.
*/

#ifndef DIFF_NOISE_HPP
#define DIFF_NOISE_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imgspecs.hpp"
///

/// Forwards
class ImageLayout;
///

/// class Noise
class Noise : public Meter {
  //
  // Sigma value for Gaussian noise
  double m_dSigma;
  //
  // Set to true for shot noise (Poisson noise)
  bool   m_bPoisson;
  //
  // Templated implementations: Add Gaussian Noise
  template<typename T>
  static void addGaussian(T *org,T min,T max,ULONG bytesperpixel,ULONG bytesperrow,ULONG w,ULONG h,double sigma);
  //
  // Templated implementations: Add Poisson Noise
  template<typename T>
  static void PoissonNoise(T *org,T max,ULONG bytesperpixel,ULONG bytesperrow,ULONG w,ULONG h);
  //
  //
  void addGaussian(class ImageLayout *img) const;
  //
  void PoissonNoise(class ImageLayout *img) const;
  // 
public:
  //
  //
  Noise(bool poisson,double sigma)
    : m_dSigma(sigma), m_bPoisson(poisson)
  {
  }
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
