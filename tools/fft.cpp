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
** Fast Fourier Transform
**
** $Id: fft.cpp,v 1.8 2017/01/31 11:58:05 thor Exp $
**
** This class implements a complex fast fourier transformation.
**
*/

/// Includes
#include "interface/types.hpp"
#include "tools/fft.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
#ifdef USE_GSL
#include <new>
///

/// FFT::FFT
// Create an FFT class for a window of the given dimensions.
FFT::FFT(ULONG width,ULONG height,bool window)
  : m_ulWidth(width), m_ulHeight(height), m_pdData(new double[(width << 1) * height]),
    m_pdHWindow(window?new double[width]:NULL), m_pdVWindow(window?new double[height]:NULL), m_bWindow(window),
    m_pHorizontalTable(NULL), m_pVerticalTable(NULL),
    m_pHorizontalWorkspace(NULL), m_pVerticalWorkspace(NULL)
{
  ULONG i;
  
  if (window) {
    for(i = 0;i < width;i++) {
      m_pdHWindow[i] = sin(M_PI * i / (width - 1));
    }
    for(i = 0;i < height;i++) {
      m_pdVWindow[i] = sin(M_PI * i / (height - 1));
    }
  }
  //memset(m_pdData,0,(width << 1) * height * sizeof(double));
}
///

/// FFT::~FFT
// Dispose the FFT again.
FFT::~FFT(void)
{
  delete[] m_pdData;
  delete[] m_pdHWindow;
  delete[] m_pdVWindow;

  if (m_pHorizontalTable)
    gsl_fft_complex_wavetable_free(m_pHorizontalTable);

  if (m_pVerticalTable)
    gsl_fft_complex_wavetable_free(m_pVerticalTable);

  if (m_pHorizontalWorkspace)
    gsl_fft_complex_workspace_free(m_pHorizontalWorkspace);

  if (m_pVerticalWorkspace)
    gsl_fft_complex_workspace_free(m_pVerticalWorkspace);
}
///

/// FFT::initTables
// Create and initialize the prequisites for the FFT.
void FFT::initTables(void)
{
  std::bad_alloc err;
  
  if (m_pHorizontalTable == NULL) {
    m_pHorizontalTable = gsl_fft_complex_wavetable_alloc(m_ulWidth);
    if (m_pHorizontalTable == NULL)
      throw err;
  }  
  if (m_pVerticalTable == NULL) {
    m_pVerticalTable = gsl_fft_complex_wavetable_alloc(m_ulHeight);
    if (m_pVerticalTable == NULL)
      throw err;
  }

  if (m_pHorizontalWorkspace == NULL) {
    m_pHorizontalWorkspace = gsl_fft_complex_workspace_alloc(m_ulWidth);
    if (m_pHorizontalWorkspace == NULL)
      throw err;
  }
  
  if (m_pVerticalWorkspace == NULL) {
    m_pVerticalWorkspace = gsl_fft_complex_workspace_alloc(m_ulHeight);
    if (m_pVerticalWorkspace == NULL)
      throw err;
  }
}
///

/// FFT::Window
// Apply the hamming window function
void FFT::Window(double *data,double *window,ULONG stride,ULONG dimension)
{
  ULONG x;

  for(x = 0;x < dimension;x++) {
    *data *= *window;
    data  += stride;
    window++;
  }
}
///

/// FFT::ForwardsFFT
// Run the forwards FFT. The result is then again in DataOf().
void FFT::ForwardsFFT(void)
{
  ULONG x,y;

  initTables();

  // First horizontally,
  for(y = 0;y < m_ulHeight;y++) {
    if (m_bWindow)
      Window(m_pdData + (y * ModuloOf()),m_pdHWindow,2,m_ulWidth);
    if (gsl_fft_complex_forward(m_pdData + (y * ModuloOf()),1,m_ulWidth,m_pHorizontalTable,m_pHorizontalWorkspace))
      throw "FFT forwards transformation failed";
  }
  // then vertically.
  for(x = 0;x < m_ulWidth;x++) {
    if (m_bWindow)
      Window(m_pdData + (x << 1),m_pdVWindow,ModuloOf(),m_ulHeight);
    if (gsl_fft_complex_forward(m_pdData + (x << 1),ModuloOf() >> 1,m_ulHeight,m_pVerticalTable,m_pVerticalWorkspace))
      throw "FFT forwards transformation failed";
  }
}
///

/// FFT::BackwardsFFT
// Run the backwards FFT.
void FFT::BackwardsFFT(void)
{ 
  ULONG x,y;
  
  initTables();

  for(x = 0;x < m_ulWidth;x++) {
    if (m_bWindow)
      Window(m_pdData + (x << 1),m_pdVWindow,ModuloOf(),m_ulHeight);
    if (gsl_fft_complex_backward(m_pdData + (x << 1),ModuloOf() >> 1,m_ulHeight,m_pVerticalTable,m_pVerticalWorkspace))
      throw "FFT backwards transformation failed";
  }
  for(y = 0;y < m_ulHeight;y++) {
    if (m_bWindow)
      Window(m_pdData + (y * ModuloOf()),m_pdHWindow,2,m_ulWidth);
    if (gsl_fft_complex_backward(m_pdData + (y * ModuloOf()),1,m_ulWidth,m_pHorizontalTable,m_pHorizontalWorkspace))
      throw "FFT backwards transformation failed";
  }
  
}
///

#endif
