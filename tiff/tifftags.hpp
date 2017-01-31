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
** This file defines tiff tags for usage in the parser and writer.
**
** $Id: tifftags.hpp,v 1.9 2017/01/31 11:58:05 thor Exp $
**
*/

#ifndef TIFF_TIFFTAGS_HPP
#define TIFF_TIFFTAGS_HPP

/// Includes
///

/// struct TiffTag
// This structure encapsulates all tiff tags.
struct TiffTag {
  enum {
    IMAGEWIDTH       = 256,
    IMAGELENGTH      = 257,
    BITSPERSAMPLE    = 258,
    COMPRESSION      = 259,
    PHOTOMETRIC      = 262,
    FILLORDER        = 266, // do we need this?
    STRIPOFFSETS     = 273,
    SAMPLESPERPIXEL  = 277,
    ROWSPERSTRIP     = 278,
    STRIPBYTECOUNTS  = 279,
    PLANARCONFIG     = 284,
    PREDICTOR        = 317,
    COLORMAP         = 320,
    TILEWIDTH        = 322,
    TILELENGTH       = 323,
    TILEOFFSETS      = 324,
    TILEBYTECOUNTS   = 325,
    EXTRASAMPLES     = 338,
    SAMPLEFORMAT     = 339,
    YCBCRSUBSAMPLING = 530,
    STONITS          = 37439 // sample value to nits conversion factor
  };
  //
  // Possible values for compression we support.
  struct Compression {
    enum {
      NONE     = 1,
      LZW      = 5,
      PACKBITS = 32773
    };
  };
  //
  // Possible values for the predictor (only for LZW)
  struct Predictor {
    enum {
      NONE  = 1,
      HDIFF = 2 // horizontal differences
    };
  };
  // Possible values for photometric we support
  struct Photometric {
    enum {
      MINISWHITE = 0,
      MINISBLACK = 1,
      RGB        = 2,
      PALETTE    = 3,
      SEPARATED  = 5,
      YCBCR      = 6
    };
  };
  //
  // Possible fill order values
  struct Fillorder {
    enum {
      MSB2LSB = 1,
      LSB2MSB = 2
    };
  };
  //
  // Possible organizations
  struct Planarconfig {
    enum {
      CONTIG   = 1, // contigous
      SEPARATE = 2
    };
  };
  //
  // Possible sample formats.
  struct Sampleformat {
    enum {
      UINT          = 1,  // unsigned integers
      INT           = 2,  // signed integers
      IEEEFP        = 3,  // floating point
      VOID          = 4,  // undefined
      COMPLEXINT    = 5,  // pairs of integers defining complex numbers
      COMPLEXIEEEFP = 6   // pairs of floating point defining complex numbers
    };
  };
};
///

///
#endif

    
