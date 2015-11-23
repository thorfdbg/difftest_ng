/*************************************************************************
** Copyright (c) 2003-2010 Accusoft/Pegasus 				**
** THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE				**
**									**
** Written by Thomas Richter (THOR Software) for Accusoft/Pegasus	**
** All Rights Reserved							**
*************************************************************************/

/*
**
** $Id: convertimg.hpp,v 1.2 2011-03-09 15:52:42 thor Exp $
**
** This class runs a debayer filter on the image converting it from grey scale
** to RGB. This is mostly experimental.
*/

#ifndef DIFF_DEBAYER_HPP
#define DIFF_DEBAYER_HPP

/// Includes
#include "diff/meter.hpp"
#include "img/imglayout.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// class Debayer
// This class saves the original image unmodified for conversion and testing
// purposes.
class Debayer : public Meter, private ImageLayout {
  //
  // The file name under which the difference image shall be saved.
  const char            *m_pcTargetFile;
  //
  // The specificiations of the output file.
  const struct ImgSpecs &m_TargetSpecs;
  //
  // The image itself.
  void                  *m_pImage[3];
  //
  // The actual Debayer algorithm.
  template<typename T>
  static void DebayerImg<T>(const T *src,LONG bytesperpixel,LONG bytesperrow,ULONG width,ULONG height,
			    T *r,T *g,T *b,T min,T max);
  //
public:
  //
  // Construct the difference image. Takes a file name.
  Debayer(const char *filename,const struct ImgSpecs &spec)
    : m_pcTargetFile(filename), m_TargetSpecs(spec)
  {
    memset(m_pImage,0,sizeof(m_pImage));
  }
  //
  virtual ~Debayer(void);
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
