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
** $Id: mapping.hpp,v 1.11 2020/09/15 09:45:49 thor Exp $
**
** This class works like the scaler, but more elaborate as it allows a couple
** of less trivial conversions: gamma mapping, log mapping and half-log mapping.
*/

#ifndef DIFF_MAPPING_HPP
#define DIFF_MAPPING_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Mapping
// This class takes floating point images and converts them to unsigned integers by various methods.
class Mapping : public Meter, private ImageLayout {
  //
public:
  enum MappingType {
    Gamma,   // perform a gamma mapping
    HalfLog, // perform a half-logarithmic map
    Log,     // perform a logarithmic map with clamp value.
    PU2,     // Rafal's percetually uniform map.
    PQ,      // backwards PQ aka SMTPE-2084, compute PQ values from luminances
    GammaToe,// gamma with toe region as in sRGB with adjustable exponent and toe region
    HLG      // hybrid log gamma
  };
  //
private:
  // The file name under which the difference image shall be saved.
  const char    *m_pTargetFile;
  //
  // The component memory itself.
  UBYTE        **m_ppucImage;
  //
  // In case this acts as a filter, keep the second (destination) image here.
  class Mapping *m_pDest;
  //
  // The lookup table for the perceptual uniform map.
  double        *m_PU_Lut;
  //
  // The mapping type to use.
  MappingType    m_Type;
  //
  // The value of gamma.
  double         m_dGamma;
  //
  // For the toe-region input: The slope of the toe region.
  double         m_dToeSlope;
  //
  // The desired output bitdepth. Only if ToInt is given,
  // zero if no scaling is considered.
  UBYTE          m_ucTargetDepth;
  //
  // Inverse operation, i.e. from half-log/gamma integer to float.
  bool           m_bInverse;
  //
  // Apply as a filter, do not save an output image.
  bool           m_bFilter;
  //
  // Output specifications of the destination file.
  const struct ImgSpecs &m_TargetSpecs;
  //
  // Create the PU-Lookup table.
  void CreatePUMap(void);
  //
  // Convert to int using a gamma mapping.
  template<typename S,typename T>
  void ToGamma(const S *org ,ULONG obytesperpixel,ULONG obytesperrow,
	       T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
	       ULONG w, ULONG h, double scale, double limF, double gamma);
  //
  // apply the inverse map.
  template<typename S,typename T>
  void InvGamma(const S *src  ,ULONG obytesperpixel,ULONG obytesperrow,
		T *dst        ,ULONG dbytesperpixel,ULONG dbytesperrow,
		ULONG w, ULONG h, double scale, double outscale, double gamma);
  //
  // Convert to int using a gamma mapping.
  template<typename S,typename T>
  void ToToeGamma(const S *org ,ULONG obytesperpixel,ULONG obytesperrow,
		  T *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		  ULONG w, ULONG h, double scale, double offset, double slope,
		  double threshold, double gamma,double min, double max);

  //
  // apply the inverse map.
  template<typename S,typename T>
  void InvToeGamma(const S *src  ,ULONG obytesperpixel,ULONG obytesperrow,
		   T *dst        ,ULONG dbytesperpixel,ULONG dbytesperrow,
		   ULONG w, ULONG h, double scale, double offset, double slope,
		   double threshold, double gamma, double min, double max);
  //
  // Compute the limF value as the 95% percentile of the luminance for
  // greyscale.
  double ComputeLimF(const FLOAT *org,ULONG obytesperpixel,ULONG obytesperrow,
		     ULONG w,ULONG h);
  //
  // Compute limF for RGB images.
  double ComputeLimF(const FLOAT *r,ULONG rbytesperpixel,ULONG rbytesperrow,
		     const FLOAT *g,ULONG gbytesperpixel,ULONG gbytesperrow,
		     const FLOAT *b,ULONG bbytesperpixel,ULONG bbytesperrow,
		     ULONG w,ULONG h);
  //
  // Convert to int using a half-log map.
  void ToHalfLog(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
		 UWORD *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		 ULONG w, ULONG h);
  //
  // Inverse computation from half-log to int. 
  void ToHalfExp(const UWORD *org ,ULONG obytesperpixel,ULONG obytesperrow,
		 FLOAT *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
		 ULONG w, ULONG h);
  //
  // Apply a logarithmic map, clamp at a minimum value. The clamp value is in m_bGamma.
  void ToLog(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
	     FLOAT *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
	     ULONG w, ULONG h);
  //
  // Apply mapping to a perceptually uniform space with Rafal's PU-Map. The output
  // is identical to the matlab script except that it is not scaled by 255. If you
  // want to do that, use --touns 8.
  void ToPU2(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
	     FLOAT *dst       ,ULONG dbytesperpixel,ULONG dbytesperrow,
	     ULONG w, ULONG h);
  //
  // Apply a mapping from luminances to PQ, this is the backwards PQ map.
  template<typename T>
  void ToPQ(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
	    T *dst           ,ULONG dbytesperpixel,ULONG dbytesperrow,
	    ULONG w, ULONG h, double scale);
  //
  // Apply a mapping from PQ values to luminances, this is the forwards PQ map.
  template<typename T>
  void FromPQ(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
	      FLOAT *dst   ,ULONG dbytesperpixel,ULONG dbytesperrow,
	      ULONG w, ULONG h, double scale);
  //
  // Apply a mapping from luminances to HLG.
  template<typename T>
  void ToHLG(const FLOAT *org ,ULONG obytesperpixel,ULONG obytesperrow,
	     T *dst           ,ULONG dbytesperpixel,ULONG dbytesperrow,
	     ULONG w, ULONG h, double scale);
  //
  // Apply a mapping from HLG to luminances
  template<typename T>
  void FromHLG(const T *org ,ULONG obytesperpixel,ULONG obytesperrow,
	       FLOAT *dst   ,ULONG dbytesperpixel,ULONG dbytesperrow,
	       ULONG w, ULONG h, double scale);
  //
  // Apply a map from the source image to the target image that must be
  // already initialized.
  void ApplyMap(class ImageLayout *src,class ImageLayout *dst);
  //
  // Create a buffer for the target image.
  void CreateTargetBuffer(class ImageLayout *templte);
  //
public:
  //
  // Scale the difference image. Takes a file name.
  Mapping(const char *filename,MappingType type,double gamma,bool inverse,UBYTE targetdepth,
	  bool filter,const struct ImgSpecs &specs,double slope = 0.0)
    : m_pTargetFile(filename), m_ppucImage(NULL), m_pDest(NULL), m_PU_Lut(NULL),
      m_Type(type), m_dGamma(gamma), m_dToeSlope(slope), m_ucTargetDepth(targetdepth), 
      m_bInverse(inverse), m_bFilter(filter), m_TargetSpecs(specs)
  {
  }
  //
  virtual ~Mapping(void);
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
