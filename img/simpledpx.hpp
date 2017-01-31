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
 * This class saves and loads images in the dpx format.
 *
 * $Id: simpledpx.hpp,v 1.14 2017/01/31 11:58:04 thor Exp $
 */

#ifndef SIMPLEDPX_HPP
#define SIMPLEDPX_HPP

/// Includes
#include "interface/types.hpp"
#include "img/imglayout.hpp"
#include "std/stdio.hpp"
#include "std/string.hpp"
///

/// class SimpleDPX
// This class saves and loads images in the dpx format. DPX is a SMTPE specification
// for mostly uncompressed frames.
class SimpleDPX : public ImageLayout {
  //
  // Components of the state machine of the file.
  // Bitbuffer buffers 32 bits in the incoming data,
  // bit is the bit counter.
  ULONG m_ulBitBuffer;
  BYTE  m_cBit;
  //
  // Endianness of the file. DPX is endian
  // agnostic and can be both little and big endian.
  // The following is TRUE for little-endian images.
  bool  m_bLittleEndian;
  //
  // The offset to the image data.
  ULONG m_ulDataOffset;
  // 
  // The generic section header size.
  ULONG m_ulGenericSize;
  //
  // The industry-specific header size
  ULONG m_ulIndustrySize;
  //
  // The user defined header size.
  ULONG m_ulUserSize;
  //
  // Image orientation. FlipX and FlipY reverse the direction
  // of X and Y scan. FlipXY exchanges the role of X and Y *after*
  // interpreting FlipX and FlipY. That is, if FlipXY is set, and
  // FlipX is set, then the fast scan direction is bottom to top,
  // the slow scan direction is left to right. FlipXY also swaps
  // the image dimensions.
  bool  m_bFlipX;
  bool  m_bFlipY;
  bool  m_bFlipXY;
  //
  // Number of image elements. An image element contains data for
  // an image plane, either interleaved from multiple components
  // or as a single component. This depends on the descriptor
  // of the element.
  UWORD m_usElements;
  //
  // The file name we are reading.
  const char *m_pcFileName;
  //
  // Since the scan pattern over an image element can be rather
  // unorthogonal, this structure describes a single element
  // for scanning.
  struct ScanElement {
    //
    // Pointer to the next element on this list.
    struct ScanElement     *m_pNext;
    //
    // The pointer to the bitplane.
    APTR                    m_pData;
    //
    // The image component this belongs to
    struct ComponentLayout *m_pComponent;
    //
    // The component/channel this goes to/comes from.
    UWORD                   m_usTargetChannel;
    //
    // The previous pixel value, used for runlength compression
    // This stores the pixel value for this element in the scan.
    // Note that a scan element represents a single component,
    // not an interleaved group of components.
    UQUAD                   m_uqPrev;
    //
    // A flag that indicates that this is the first scan element of
    // its component within a scan pattern.
    bool                    m_bFirst;
    //
    ScanElement(void) 
      : m_pNext(NULL), m_pData(NULL), m_pComponent(NULL),
	m_usTargetChannel(0), m_bFirst(false)
    { 
    }
    // Create a new scan element, enqueue it into a list.
    // The target channel is the channel number within the element, not
    // an absolute component number. If this is set to MAX_UWORD, the
    // scan element is an alpha channel.
    ScanElement(struct ScanElement **&prev,UWORD targetchannel)
      : m_pNext(NULL), m_pData(NULL), m_pComponent(NULL), 
	m_usTargetChannel(targetchannel)
    {
      *prev = this;
      prev  = &m_pNext;
    }
    //
  };
  //
  // A single image element. This element holds the data.
  // This stores the data for an interleaved group of pixels.
  struct ImageElement {
    //
    // Pointer to the image data. A pixel in an element
    // can up to eight components deep, we store them
    // separately because they can be subsampled.
    APTR  m_pData[8];
    //
    // The scan pattern for this element. This points
    // to the components of the element.
    struct ScanElement *m_pScanPattern;
    //
    // Dimensions of the data (unrotated)
    ULONG m_ulWidth;
    ULONG m_ulHeight;
    //
    // The descriptor of the element. The descriptor
    // identifies the data type of the pixels within the element,
    // and to some degree its color representation.
    UBYTE m_ucDescriptor;
    //
    // Number of components per element.
    UBYTE m_ucDepth;
    //
    // Number of alpha channels per element.
    UBYTE m_ucAlphaDepth;
    //
    // Subsampling in line (X) and frame (Y) direction.
    // This is subsampling before rotation.
    UBYTE m_ucSubX,m_ucSubY;
    //
    // Bit depth of the element
    UBYTE m_ucBitDepth;
    //
    // Elements of bitdepths 10 or 12 can be padded, either
    // at the LSB side or at the MSB side. For ten bits,
    // three numbers are padded (at LSB or MSB or none) and fit
    // into one 32 bit LONG.
    // For 16 bit data, two elements are fitted into one 32 bit
    // LONG, but individually padded. (Sigh!)
    // For one-bit data, 32 elements are fitted into the data,
    // LSB first, without padding.
    //
    // Number of padding bits at the LSB side
    UBYTE m_ucLSBPaddingBits;
    //
    // Number of padding bits at the MSB side
    UBYTE m_ucMSBPaddingBits;
    //
    // Number of elements packed together.
    UBYTE m_ucPackElements;
    //
    // Signed or unsigned?
    bool  m_bSigned;
    //
    // If this is set, the component is float.
    bool  m_bFloat;
    //
    // Set in case Runlength-coding is used.
    bool  m_bRLE;
    //
    // End of line padding. This is the number of bytes added to the end of
    // each line to pad to a given multiple of bytes.
    ULONG m_ulEndOfLinePadding;
    //
    // End of image padding. This is the number of bytes added to the end
    // of the frame. This is pretty much useless as individual element
    // offsets are recorded anyhow. But if we would want to read data
    // sequentially, here is the padding between elements.
    ULONG m_ulEndOfFramePadding;
    //
    // The offset to the data from the start of the file.
    ULONG m_ulOffset;
    //
    // X and Y position for scanning. Element #15 is reserved for alpha.
    // This is for internal housekeeping.
    ULONG m_ulX[15];
    ULONG m_ulY[15];
    //
    // Default constructor of an element: delete at least the pointer.
    ImageElement(void)
    : m_pScanPattern(NULL),
      m_ucDepth(0), m_ucAlphaDepth(0),
      m_ucSubX(1), m_ucSubY(1),
      m_ucBitDepth(0), m_ucLSBPaddingBits(0), m_ucMSBPaddingBits(0),
      m_ucPackElements(1),
      m_bSigned(false), m_bFloat(false), m_bRLE(false),
      m_ulEndOfLinePadding(0), m_ulEndOfFramePadding(0), m_ulOffset(0)
    { 
      memset(m_pData,0,sizeof(m_pData));
      memset(m_ulX,0,sizeof(m_ulX));
      memset(m_ulY,0,sizeof(m_ulY));
    }
    //
    // Destructor of the element
    ~ImageElement(void)
    {
      struct ScanElement *se;
      int i;

      while((se = m_pScanPattern)) {
	m_pScanPattern = se->m_pNext;
	delete se;
      }
      
      for(i = 0;i < 8;i++) {
	delete[] (UBYTE *)m_pData[i];
      }
    }
    //
    // Install default settings: Descriptor, depth, signedness
    void SetDefault(UBYTE descriptor,const struct ComponentLayout &c)
    {
      m_ucDescriptor = descriptor;
      m_ucBitDepth   = c.m_ucBits;
      m_bSigned      = c.m_bSigned;
      //
      // These two require padding.
      if (m_ucBitDepth == 10) {
	m_ucLSBPaddingBits = 2;
	m_ucPackElements   = 3;
      } else if (m_ucBitDepth == 12) {
	m_ucLSBPaddingBits = 4;
	m_ucPackElements   = 1;
      } else if (m_ucBitDepth == 1) {
	m_ucPackElements   = 32;
      }
    }
    //
  }   m_Elements[8];
  //
  //
  // A little helper to make IDs readable.
  static ULONG MakeID(UBYTE c1,UBYTE c2,UBYTE c3,UBYTE c4)
  {
    return (c1 << 24) | (c2 << 16) | (c3 << 8) | c4;
  }
  //
  // Read a byte from a file, throw on EOF.
  UBYTE GetByte(FILE *f) const
  {
    int i = getc(f);

    if (i == EOF)
      PostError("unexpected error while reading a DPX file %s",m_pcFileName);

    return UBYTE(i);
  }
  //
  // Seek n bytes forward
  void SkipBytes(FILE *f,int b) const
  {
    if (fseek(f,b,SEEK_CUR) < 0)
      PostError("unexepcted error while skipping bytes in a DPX file %s",m_pcFileName);
  }
  //
  // Get an ASCII string of the given size.
  void GetString(FILE *f,char *buf,size_t len) const
  {
    while(len) {
      *buf++ = GetByte(f);
      len--;
    }
  }
  //
  // Put a string with at most len characters. Fill the rest with zeros, or
  // truncate if the string is too long.
  void PutString(FILE *f,const char *buf,size_t len) const
  {
    size_t slen = strlen(buf);
    size_t add;

    if (slen > len)
      slen = len;
    add = len - slen;

    while(slen) {
      PutByte(f,*buf++);
      slen--;
    }
    while(add) {
      PutByte(f,0);
      add--;
    }
  }
  //
  // Get a 16-bit WORD, endian-agnostic.
  UWORD GetWord(FILE *f) const
  {
    UBYTE b1,b2;

    b1 = GetByte(f);
    b2 = GetByte(f);

    if (m_bLittleEndian) {
      return b1 | (b2 << 8);
    } else {
      return b2 | (b1 << 8);
    }
  }
  //
  // Get a 32-bit LONG, endian-agnostic.
  ULONG GetLong(FILE *f) const
  {
    UWORD w1,w2;

    w1 = GetWord(f);
    w2 = GetWord(f);

    if (m_bLittleEndian) {
      return w1 | (w2 << 16);
    } else {
      return w2 | (w1 << 16);
    }
  }
  //
  // Put a single byte to the stream.
  void PutByte(FILE *f,UBYTE b) const
  {
    putc(b,f);
    
    if (ferror(f))
      PostError("error writing data to a DPX file %s",m_pcFileName);
  }
  //
  // Put a WORD to the stream, endian-agnostic.
  void PutWord(FILE *f,UWORD w) const
  {
    if (m_bLittleEndian) {
      PutByte(f,UBYTE(w >> 0));
      PutByte(f,UBYTE(w >> 8));
    } else {
      PutByte(f,UBYTE(w >> 8));
      PutByte(f,UBYTE(w >> 0));
    }
  }
  //
  // Put a LONG to the stream, endian-agnostic
  void PutLong(FILE *f,ULONG l) const
  {
    if (m_bLittleEndian) {
      PutWord(f,UWORD(l >>  0));
      PutWord(f,UWORD(l >> 16));
    } else {
      PutWord(f,UWORD(l >> 16));
      PutWord(f,UWORD(l >>  0));
    }
  }
  //
  // Parse the file header of a DPX file
  void ParseHeader(FILE *file,struct ImgSpecs &specs);
  //
  // Parse the header of a single element. Returns the yuv flag.
  bool ParseElementHeader(FILE *file,struct ImageElement *el);
  //
  // Write the element specific header to the file.
  void WriteElementHeader(FILE *file,struct ImageElement *el,ULONG offset);
  //
  // Parse an element of a dpx file, and fill its data into the
  // data container.
  void ParseElement(FILE *file,struct ImageElement *el);
  //
  // Write out the target data to the components.
  void WriteElement(FILE *out,struct ImageElement *el);
  //
  // Build the scan pattern for the given element and element description
  // Returns true for YUV scans.
  bool BuildScanPattern(struct ImageElement *el);
  //
  // Read a single pixel from the specified file.
  UQUAD ReadData(FILE *in,const struct ImageElement *el,bool sign);
  //
  // Write a single pixel value to the output file.
  void WriteData(FILE *out,struct ImageElement *el,UQUAD q);
  //
  // Flush the output buffer if there are bits waiting in it.
  void Flush(FILE *out,struct ImageElement *el);
  //
  // Write the complete DPX header with all specifications.
  // pad data accordingly.
  void WriteHeader(FILE *file,ULONG planes[9]);
  //
  // Generate the layout of elements given the information in
  // the component layout.
  void CreateElementLayout(const struct ImgSpecs &specs);
  //
  // Link the element data to the components in the system.
  // Compute the component offsets.
  void AssociateComponents(ULONG offset,ULONG planes[9]);
  //
public:
  // default constructor
  SimpleDPX(void);
  // Copy the image from another source for later saving.
  SimpleDPX(const class ImageLayout &layout);
  // destructor
  ~SimpleDPX(void);
  //
  // Save an image to a level 1 file descriptor, given its
  // width, height and depth.
  void SaveImage(const char *name,const struct ImgSpecs &specs);
  //
  // Load an image from a level 1 file descriptor, keep it within
  // the internals of this class. The accessor methods below
  // should be used to find out more about this image.
  void LoadImage(const char *name,struct ImgSpecs &specs);
  //
};
///

#endif
