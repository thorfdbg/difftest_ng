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
** $Id: ycbcr.hpp,v 1.16 2023/06/12 07:34:45 thor Exp $
**
** This class converts between RGB and YCbCr signals
*/

#ifndef DIFF_YCBCR_HPP
#define DIFF_YCBCR_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
#include "std/string.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class YCbCr
// This class converts images between rgb and ycbcr data.
class YCbCr : public Meter, private ImageLayout {
  //
  // This bool is set for backwards conversion, i.e. YCbCr->RGB
  bool  m_bInverse;
  //
  // This flag is set in case the chroma components shall be made
  // signed.
  bool  m_bMakeSigned;
  //
  // If this is set, the typical ITU-BT.XXX black level of 16 (scaled)
  // is added or subtracted from the signal levels.
  bool  m_bBlackLevel;
  //
  // The component memory itself. In case of the integer transformations,
  // the range is expanded, hence a new image has to be created.
  UBYTE *m_ppucSrcImage[4];
  UBYTE *m_ppucDstImage[4];
  //
public:
  //
  // The conversion to run
  enum Conversion {
    YCbCr_Trafo,     // this is actually a BT.601 conversion
    RCT_Trafo,
    YCgCo_Trafo,
    YCbCr709_Trafo,  // This is the BT.709 conversion
    YCbCr2020_Trafo, // This is the transformation for BT.2020
    RCTD_Trafo,      // Convert 4-component image with RGGB components into RCT plus delta-green
    YCgCoD_Trafo,    // Convert 4-component image with RGGB components into YCgCo plus delta-green
    Delta_Trafo,     // Convert 4-component image into R(avgG)B,delta-green
    RCT422_Trafo     // A transformation that looks almost like RCT, but operates in 422 space.
  };
  //
private:
  Conversion m_Conversion;
  //
  template<typename S>
  static void Copy(const S *src,S *dst,
		   ULONG srcbpp,ULONG srcbpr,
		   ULONG dstbpp,ULONG dstbpr,
		   ULONG w,ULONG h);
  //
  // Forwards conversion
  template<typename S,typename T>
  static void ToYCbCr(S *r,S *g,S *b,
		      double yoffset,double coffset,
		      double min,double max,
		      double cmin,double cmax,
		      ULONG bppr,ULONG bppg,ULONG bppb,
		      ULONG bprr,ULONG bprg,ULONG bprb,
		      ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void ToYCbCrBL(S *r,S *g,S *b,
			double yoffset,double coffset,
			double min,double max,
			double cmin,double cmax,
			ULONG bppr,ULONG bppg,ULONG bppb,
			ULONG bprr,ULONG bprg,ULONG bprb,
			ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void ToYCbCr709(S *r,S *g,S *b,
			 double yoffset,double coffset,
			 double min,double max,
			 double cmin,double cmax,
			 ULONG bppr,ULONG bppg,ULONG bppb,
			 ULONG bprr,ULONG bprg,ULONG bprb,
			 ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void ToYCbCr709BL(S *r,S *g,S *b,
			   double yoffset,double coffset,
			   double min,double max,
			   double cmin,double cmax,
			   ULONG bppr,ULONG bppg,ULONG bppb,
			   ULONG bprr,ULONG bprg,ULONG bprb,
			   ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void ToYCbCr2020(S *r,S *g,S *b,
			  double yoffset,double coffset,
			  double min,double max,
			  double cmin,double cmax,
			  ULONG bppr,ULONG bppg,ULONG bppb,
			  ULONG bprr,ULONG bprg,ULONG bprb,
			  ULONG w, ULONG h);  
  //
  // Forward conversion for RCT
  template<typename S,typename T>
  static void ToRCT(const S *r,const S *g,const S *b,S *y,T *cb,T *cr,
		    LONG yoffset,LONG coffset,
		    ULONG bppr,ULONG bppg, ULONG bppb,
		    ULONG bprr,ULONG bprg, ULONG bprb,
		    ULONG bppy,ULONG bppcb,ULONG bppcr,
		    ULONG bpry,ULONG bprcb,ULONG bprcr,
		    ULONG w, ULONG h);
  //
  // Forward conversion for the 422 RCT
  template<typename S,typename T>
  static void To422RCT(const S *r,const S *g,const S *b,S *y,T *cb,T *cr,
		       LONG yoffset,LONG coffset,
		       ULONG bppr,ULONG bppg, ULONG bppb,
		       ULONG bprr,ULONG bprg, ULONG bprb,
		       ULONG bppy,ULONG bppcb,ULONG bppcr,
		       ULONG bpry,ULONG bprcb,ULONG bprcr,
		       ULONG w, ULONG h);   
  //
  // Forward conversion for YCgCo
  template<typename S,typename T>
  static void ToYCgCo(const S *r,const S *g,const S *b,S *y,T *cg,T *co,
		      LONG yoffset,LONG coffset,
		      ULONG bppr,ULONG bppg, ULONG bppb,
		      ULONG bprr,ULONG bprg, ULONG bprb,
		      ULONG bppy,ULONG bppcg,ULONG bppco,
		      ULONG bpry,ULONG bprcg,ULONG bprco,
		      ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void ToDeltaGreen(const S *g1,const S *g2,S *a,T *d,
			   LONG doffset,
			   ULONG bppg1,ULONG bppg2,
			   ULONG bprg1,ULONG bprg2,
  			   ULONG bppa,ULONG bppd,
			   ULONG bpra,ULONG bprd,
			   ULONG w, ULONG h);
  //
  // Backwards conversion.
  template<typename S,typename T>
  static void FromYCbCr(S *y,T *cb,T *cr,
			double yoffset,double coffset,
			double min,double max,
			ULONG bppy,ULONG bppcb,ULONG bppcr,
			ULONG bpry,ULONG bprcb,ULONG bprcr,
			ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void FromYCbCrBL(S *y,T *cb,T *cr,
			  double yoffset,double coffset,
			  double min,double max,
			  ULONG bppy,ULONG bppcb,ULONG bppcr,
			  ULONG bpry,ULONG bprcb,ULONG bprcr,
			  ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void FromYCbCr709(S *y,T *cb,T *cr,
			   double yoffset,double coffset,
			   double min,double max,
			   ULONG bppy,ULONG bppcb,ULONG bppcr,
			   ULONG bpry,ULONG bprcb,ULONG bprcr,
			   ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void FromYCbCr709BL(S *y,T *cb,T *cr,
			     double yoffset,double coffset,
			     double min,double max,
			     ULONG bppy,ULONG bppcb,ULONG bppcr,
			     ULONG bpry,ULONG bprcb,ULONG bprcr,
			     ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void FromYCbCr2020(S *y,T *cb,T *cr,
			    double yoffset,double coffset,
			    double min,double max,
			    ULONG bppy,ULONG bppcb,ULONG bppcr,
			    ULONG bpry,ULONG bprcb,ULONG bprcr,
			    ULONG w, ULONG h);
  //
  template<typename S,typename T>
  static void FromRCT(const S *y,const T *cb,const T *cr,S *r,S *g,S *b,
		      LONG yoffset,LONG coffset,
		      ULONG bppy,ULONG bppcb,ULONG bppcr,
		      ULONG bpry,ULONG bprcb,ULONG bprcr,
		      ULONG bppr,ULONG bppg, ULONG bppb,
		      ULONG bprr,ULONG bprg, ULONG bprb,
		      ULONG w, ULONG h,
		      LONG min,LONG max);
  //
  template<typename S,typename T>
  static void From422RCT(const S *y,const T *cb,const T *cr,S *r,S *g,S *b,
			 LONG yoffset,LONG coffset,
			 ULONG bppy,ULONG bppcb,ULONG bppcr,
			 ULONG bpry,ULONG bprcb,ULONG bprcr,
			 ULONG bppr,ULONG bppg, ULONG bppb,
			 ULONG bprr,ULONG bprg, ULONG bprb,
			 ULONG w, ULONG h,
			 LONG min,LONG max);
  //
  template<typename S,typename T>
  static void FromYCgCo(const S *y,const T *cb,const T *cr,S *r,S *g,S *b,
			LONG yoffset,LONG coffset,
			ULONG bppy,ULONG bppcg,ULONG bppco,
			ULONG bpry,ULONG bprcg,ULONG bprco,
			ULONG bppr,ULONG bppg, ULONG bppb,
			ULONG bprr,ULONG bprg, ULONG bprb,
			ULONG w, ULONG h,LONG min,LONG max);
  //
  template<typename S,typename T>
  static void FromDeltaGreen(const S *a,const T *d,S *g1,S *g2,
			     LONG doffset,
			     ULONG bppa,ULONG bppd,
			     ULONG bpra,ULONG bprd,
			     ULONG bppg1,ULONG bppg2,
			     ULONG bprg1,ULONG bprg2,
			     ULONG w, ULONG h);
  //
  // Convert a single image to YCbCr.
  void ToYCbCr(class ImageLayout *img);
  //
  // Convert a single image from YCbCr to RGB
  void FromYCbCr(class ImageLayout *img);
  //
  // Convert from the external image to the image stored in
  // this structure
  template<typename S,typename T>
  void DispatchToRCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h);
  //
  // The dispatcher for the reverse transformation direction.
  template<typename S,typename T>
  void DispatchFromRCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h,
		       LONG min,LONG max);
  //
  // The dispatcher for the 422RCT (a weirdo)
  template<typename S,typename T>
  void DispatchTo422RCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h);
  //
  // The dispatcher for the reverse transformation direction.
  template<typename S,typename T>
  void DispatchFrom422RCT(const class ImageLayout *img,ULONG yoffset,ULONG coffset,ULONG w,ULONG h,
			  LONG min,LONG max);
  //
  template<typename S,typename T>
  void DispatchToYCbCr(const class ImageLayout *img,double offset,double coffset,
		       double ymin,double ymax,double cmin,double cmax,ULONG w,ULONG h);
  //
  // The dispatcher for the reverse transformation direction.
  template<typename S,typename T>
  void DispatchFromYCbCr(const class ImageLayout *img,double yoffset,double coffset,double min,double max,
			 ULONG w,ULONG h);
  //
  // Perform integer transformations, RCT and YCgCo
  // They are both range-expanding.
  //
  // Convert an image from RCT or YCgCo, creating a new image
  void ToRCT(class ImageLayout *img,UBYTE *(&membuf)[4]);
  //
  // Convert an image from RCT or YCgCo, creating a new image
  void FromRCT(class ImageLayout *img,UBYTE *(&membuf)[4]);
  //
  // Convert a 422 sampled image with the 422 RCT to YCbCr.
  void To422RCT(class ImageLayout *img,UBYTE *(&membuf)[4]);
  //
  // Convert a YCbCr 422 sampled image with the 422 RCT to RGB.
  void From422RCT(class ImageLayout *img,UBYTE *(&membuf)[4]);
  //
public:
  //
  // Forwards or backwards conversion to and from YCbCr
  YCbCr(bool inverse,bool makesigned,bool blacklevel,Conversion conv)
    : m_bInverse(inverse), m_bMakeSigned(makesigned), m_bBlackLevel(blacklevel), m_Conversion(conv)
  {
    memset(m_ppucSrcImage,0,sizeof(m_ppucSrcImage));
    memset(m_ppucDstImage,0,sizeof(m_ppucDstImage));
  }
  //
  virtual ~YCbCr(void);
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
