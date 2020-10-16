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
 * Image specifications
 * 
 * $Id: imgspecs.hpp,v 1.8 2020/10/16 11:23:25 thor Exp $
 *
 * This structure defines a couple of specifications an image reader
 * may provide, and and image writer may acknowledge. 
 */

#ifndef IMGSPECS_HPP
#define IMGSPECS_HPP

/// Includes
#include "interface/types.hpp"
#include "std/stdio.hpp"
#include "std/assert.hpp"
///

/// struct ImgSpecs
struct ImgSpecs {
  //
  enum BinaryFeature {
    Yes,
    No,
    Unspecified
  };
  //
  // Is this image encoded in raw (binary) or ascii format?
  BinaryFeature ASCII;
  //
  // Is this image encoded in one or multiple planes (interleaved?)
  BinaryFeature Interleaved;
  //
  // Is this image in an RGB or YUV type of color space?
  BinaryFeature YUVEncoded;
  //
  // Does the image come from a palette encoding?
  BinaryFeature Palettized;
  //
  // Is this image little endian?
  BinaryFeature LittleEndian;
  //
  // Is the image scaled to absolute luminance.
  BinaryFeature AbsoluteRadiance;
  //
  // The radiance scale (if there is one)
  double        RadianceScale;
  //
  BinaryFeature FullRange;
  //
  ImgSpecs(void)
    : ASCII(Unspecified), Interleaved(Unspecified), YUVEncoded(Unspecified), 
      Palettized(Unspecified), LittleEndian(Unspecified), AbsoluteRadiance(Unspecified),
      RadianceScale(1.0), FullRange(Unspecified)
  { }
  //
  // MergeSpecs: Merge this, and two other specs together. This one overrides all,
  // spec one overrides spec2.
  void MergeSpecs(struct ImgSpecs &spec1,struct ImgSpecs &spec2);
  //
private:
  // Merge two features together.
  static BinaryFeature MergeFeature(BinaryFeature f1,BinaryFeature f2,BinaryFeature f3);
};
///

///
#endif
