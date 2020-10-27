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
** $Id: sim2.hpp,v 1.1 2020/10/26 10:06:06 thor Exp $
**
** This class converts between XYZ and SIM2 signals, the latter are used to drive
** the SIM2 monitor.
*/

#ifndef DIFF_SIM2_HPP
#define DIFF_ISM2_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Sim2
// This class converts images between xyz and sim2 data.
class Sim2 : public Meter, private ImageLayout {
  // 
public:
  //
  // There is currently only one conversion direction, that from XYZ to
  // sim2.
  //
private:
  //
  // Two buffers for data, source and target. This buffers the output data.
  UBYTE *m_pucSrc;
  UBYTE *m_pucDst;
  //
  // Conversion core.
  template<typename S>
  void XYZToSim2(const S *x,const S *y,const S *z,
		 ULONG bppx,ULONG bppy,ULONG bppz,
		 ULONG bprx,ULONG bpry,ULONG bprz,
		 UBYTE *buf,
		 ULONG width,ULONG height);
  //
  // Convert a single image.
  void Convert(class ImageLayout *img,bool dst);
  //
  //
public:
  //
  // Forwards or backwards conversion to and from XYZ
  Sim2(void)
    : m_pucSrc(NULL), m_pucDst(NULL)
  {
  }
  //
  virtual ~Sim2(void);
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
