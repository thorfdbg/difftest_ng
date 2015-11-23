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
** Fast Fourier Transform
**
** $Id: fft.hpp,v 1.4 2014/01/04 11:35:29 thor Exp $
**
** This class implements a complex fast fourier transformation.
**
*/

#ifndef TOOLS_FFT_HPP
#define TOOLS_FFT_HPP

/// Includes
#include "interface/types.hpp"
#ifdef USE_GSL
#include <gsl/gsl_fft_complex.h>
///

/// Class FFT
class FFT {
  // This class works, unlike most other classes here, on floating point data.
  //
  // Dimensions of the array here.
  ULONG   m_ulWidth;
  ULONG   m_ulHeight;
  //
  // The data itself.
  double *m_pdData;
  //
  // The horizontal window function.
  double *m_pdHWindow;
  // 
  // The vertical window function.
  double *m_pdVWindow;
  //
  // Apply windowing?
  bool    m_bWindow;
  //
  // The wave tables for horizontal and vertical transformation.
  gsl_fft_complex_wavetable *m_pHorizontalTable;
  gsl_fft_complex_wavetable *m_pVerticalTable;
  //
  // Workspaces
  gsl_fft_complex_workspace *m_pHorizontalWorkspace;
  gsl_fft_complex_workspace *m_pVerticalWorkspace;
  //
  // Create and initialize the prequisites for the FFT.
  void initTables(void);
  //
  // Apply the hamming window function
  void Window(double *data,double *window,ULONG stride,ULONG dimension);
  //
public:
  // Create an FFT class for a window of the given dimensions.
  FFT(ULONG width,ULONG height,bool window);
  //
  // Destroy the FFT again.
  ~FFT(void);
  //
  // Get access to the origin of the FFT window. This contains real/imaginary components
  // interleaved
  double *DataOf(void)
  {
    return m_pdData;
  }
  //
  // The modulo/stride of the above array. This is in elements in the array entries, not in numbers
  // because each number takes two slots.
  ULONG ModuloOf(void)
  {
    return m_ulWidth << 1;
  }
  //
  // Run the forwards FFT. The result is then again in DataOf().
  void ForwardsFFT(void);
  //
  // Run the backwards FFT.
  void BackwardsFFT(void);
  //
  // Return width and height of the array. Note that this counts complex entries, not real ones.
  ULONG WidthOf(void) const
  {
    return m_ulWidth;
  }
  //
  ULONG HeightOf(void) const
  {
    return m_ulHeight;
  }
};
///

///
#endif
#endif
  
