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
 * Image layout
 * 
 * $Id: imglayout.hpp,v 1.23 2023/07/11 10:34:06 thor Exp $
 *
 * This class defines the image layout, width, height and the
 * image depth of the individual components. It is supplied by
 * the loading class (j2k in our case, or BMP, or PGM, or PGX)
 * and provided to the saving class.
 */

#ifndef IMGLAYOUT_HPP
#define IMGLAYOUT_HPP

/// Includes
#include "interface/types.hpp"
#include "std/stdio.hpp"
#include "std/assert.hpp"
///

/// Forwards
struct ImgSpecs;
///

/// Class ImageLayout
class ImageLayout {
  //
  // For multi-codestream support: Attach several image layouts in one row,
  // write them at once.
  class ImageLayout *m_pNext;  
  //
  // File storage so we don't have to close and
  // remove and close files in the main manually.
  FILE              *m_pFileStore;
  //
protected:
  //
  // Width and height of the image we administrate. If subsampling should be involved,
  // then this is without it.
  ULONG              m_ulWidth;
  ULONG              m_ulHeight;
  //
  // Depth of the image in components.
  UWORD              m_usDepth;
  //
  // Number of alpha channels in here.
  UWORD              m_usAlphaDepth;
  //
  // The component array. We hold depth+1 components, one is reserved for the ROI.
  // How and where the components are organized is the matter of the implementors.
  struct ComponentLayout {
    //
    // The depth of the component in bits. This bit count
    // includes the sign bit.
    UBYTE       m_ucBits;
    //
    // A boolean indicator whether this is signed or not.
    bool        m_bSigned;
    //
    // A boolean indicator whether this is a IEEE float format or not.
    bool        m_bFloat;
    //
    // Possible subsampling values, if we have one.
    UBYTE       m_ucSubX;
    UBYTE       m_ucSubY;
    //
    // If components have individual sizes, place them here.
    // Only pgx and j2k care here, all others use the global
    // width.
    ULONG       m_ulWidth;
    ULONG       m_ulHeight;
    //
    // Image organization: Bytes per Row, Bytes per Pixel
    ULONG       m_ulBytesPerPixel;
    ULONG       m_ulBytesPerRow;
    //
    // A pointer to the image itself. This does not
    // administrate the memory, this does the implementor.
    APTR        m_pPtr;
    //
    // Constructor.
    ComponentLayout(void)
      : m_ucBits(8), m_bSigned(false), m_bFloat(false),
	m_ucSubX(1), m_ucSubY(1),
	m_ulWidth(0), m_ulHeight(0),
	m_pPtr(NULL)
    { }
    //
    ~ComponentLayout(void)
    { }
  }            *m_pComponent;
  //
  // Allocate an image layout for the given width and height.
  // Should only be used for building up the component array.
  void CreateComponents(ULONG w,ULONG h,UWORD d);
  //
  // Create components from a template, using the same layout as the
  // original.
  void CreateComponents(const class ImageLayout &img);
  //
  // Compute a suitable bits per pixel value from a bitdepth. Note that
  // this is not the bpp value for this specific implementation, but
  // a helper function that returns a usable size for a given bitdepth
  static UBYTE SuggestBPP(UBYTE bits,bool isfloat);
  //
public:
  // This can be called by subclasses to indicate an
  // error
  static void PostError(const char *ftm,...);
  //
  // Constructor and destructor. This also includes a copy constructor
  // when making a clone of an image to save it later.
  ImageLayout(void);
  ImageLayout(const class ImageLayout &img);
  virtual ~ImageLayout(void);
  //
  // Assignment operator. Makes also a clone of the source,
  // installing it here.
  class ImageLayout &operator=(const class ImageLayout &img);
  //
  // Return some basic layout questions.
  ULONG WidthOf(void) const
  {
    return m_ulWidth;
  }
  ULONG HeightOf(void) const
  {
    return m_ulHeight;
  }
  UWORD DepthOf(void) const
  {
    return m_usDepth;
  }
  //
  // Return the width of the n-th component.
  ULONG WidthOf(UWORD comp) const
  {
    assert(comp < m_usDepth);
    assert(m_pComponent);
    
    return m_pComponent[comp].m_ulWidth;
  }
  //
  // Return the height of the n-th component.
  ULONG HeightOf(UWORD comp) const
  {
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_ulHeight;
  }
  //
  // Return the number of bits in the n-th component.
  UBYTE BitsOf(UWORD comp) const
  {
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_ucBits;
  }
  //
  // Return the signed-ness of the n-th component.
  bool isSigned(UWORD comp) const
  {
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_bSigned;
  }
  //
  // The signedness as an rvalue.
  bool &isSigned(UWORD comp)
  { 
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_bSigned;
  }
  //
  // Return whether the data is floating point or not.
  bool isFloat(UWORD comp) const
  {
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_bFloat;
  }
  //
  // Return the subsampling in X direction.
  UBYTE SubXOf(UWORD comp) const
  { 
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_ucSubX;
  }
  //
  // Return the subsampling in Y direction.
  UBYTE SubYOf(UWORD comp) const
  { 
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_ucSubY;
  }    
  //
  // Return the raw image data of the n-th component.
  void *DataOf(UWORD comp) const
  {
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_pPtr;
  }
  //
  // Return the number of bytes to skip from one pixel to the next.
  ULONG BytesPerPixel(UWORD comp) const
  { 
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_ulBytesPerPixel;
  }
  //
  // Return the number of bytes per row to skip from one row to the next.
  ULONG BytesPerRow(UWORD comp) const
  {
    assert(comp < m_usDepth);
    assert(m_pComponent);

    return m_pComponent[comp].m_ulBytesPerRow;
  }
  //
  // Perform an endian swap on the buffered image. This is nowhere
  // recorded, though. 
  bool SwapEndian(void);
  //
  // Swap this image layout internals with that of the given source.
  void Swap(class ImageLayout &o);
  //
  // Reduce the image to a single component, namely the given one.
  // Does not filter, etc...
  void Restrict(UWORD comp,UWORD count);
  //
  // Crop an image region from the image given the coordinates of the edges.
  void Crop(ULONG x1,ULONG y1,ULONG x2,ULONG y2);
  //
  // Limit to the given field of all components.
  void ExtractField(bool oddfield);
  //
  // Return or link the next image in here.
  class ImageLayout *&NextOf(void)
  {
    return m_pNext;
  }
  //
  // Unlink the image pointed to by the argument and update the list.
  virtual void Unlink(class ImageLayout *&head)
  {
    head = m_pNext;
  }
  //
  // Load an image from the specified filespec using the appropriate file type,
  // derived from the extension. Returns the proper loader.
  static class ImageLayout *LoadImage(const char *filename,struct ImgSpecs &specs);
  //
  // Clone the layout of an image and create an image of the same dimensions just
  // with no data.
  static class ImageLayout *CloneLayout(const class ImageLayout *org);
  //
  // Save an image back to a file
  void SaveImage(const char *filename,const struct ImgSpecs &specs);
  //
  // Save an image with default specifications.
  void SaveImage(const char *filename);
  //
  // Check whether the two images are compatible in dimension and depth
  // to allow a comparison. Throw if not.
  void TestIfCompatible(const class ImageLayout *dst) const;

};
///

///
#endif

