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
 * Image specifications
 * 
 * $Id: imgspecs.cpp,v 1.3 2014/10/18 09:35:20 thor Exp $
 *
 * This structure defines a couple of specifications an image reader
 * may provide, and and image writer may acknowledge. 
 */

/// Includes
#include "interface/types.hpp"
#include "img/imgspecs.hpp"
///

/// ImgSpecs::MergeSpecs
// MergeSpecs: Merge this, and two other specs together. This one overrides all,
// spec one overrides spec2.
void ImgSpecs::MergeSpecs(struct ImgSpecs &spec1,struct ImgSpecs &spec2)
{
  ASCII            = MergeFeature(ASCII      ,spec1.ASCII      ,spec2.ASCII);
  Interleaved      = MergeFeature(Interleaved,spec1.Interleaved,spec2.Interleaved);
  YUVEncoded       = MergeFeature(YUVEncoded ,spec1.YUVEncoded ,spec2.YUVEncoded);
  Palettized       = MergeFeature(Palettized ,spec1.Palettized ,spec2.Palettized);
  AbsoluteRadiance = MergeFeature(AbsoluteRadiance,spec1.AbsoluteRadiance,spec2.AbsoluteRadiance);
  RadianceScale    = spec1.RadianceScale;
}
///

/// ImgSpecs::MergeFeature
ImgSpecs::BinaryFeature ImgSpecs::MergeFeature(ImgSpecs::BinaryFeature f1,
					       ImgSpecs::BinaryFeature f2,
					       ImgSpecs::BinaryFeature f3)
{
  if (f1 != Unspecified) 
    return f1;
  if (f2 != Unspecified)
    return f2;
  return f3;
}
///
