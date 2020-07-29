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
 * An image class to load and save uncompressed PGX images.
 * This is the image file format that is defined by JPEG2000 part 4
 * for encoding the test streams.
 *
 * $Id: simplepgx.cpp,v 1.23 2020/07/29 11:10:47 thor Exp $
 */

/// Includes
#include "std/string.hpp"
#include "std/ctype.hpp"
#include "std/stdlib.hpp"
#include "tools/file.hpp"
#include "tools/halffloat.hpp"
#include "img/imgspecs.hpp"
#include "img/simplepgx.hpp"
///

/// Path separator
#ifdef _WINDLL // wrong-way path separator on windows...
# define PATH_SEP '\\'
#else
# define PATH_SEP '/'
#endif
///

/// SimplePgx::SimplePgx
// Default constructor.
SimplePgx::SimplePgx(void)
  : m_pucImage(NULL), m_pusImage(NULL), m_pNameList(NULL)
{
}
///

/// SimplePgx::SimplePgx
// Copy constructor, reference a PPM image.
SimplePgx::SimplePgx(const class ImageLayout &org)
  : ImageLayout(org), m_pucImage(NULL), m_pusImage(NULL), m_pNameList(NULL)
{
}
///

/// SimplePgx::~SimplePgx
// Dispose the object, delete the image
SimplePgx::~SimplePgx(void)
{
  struct ComponentName *n;
  //
  delete[] m_pucImage;
  delete[] m_pusImage;
  //
  // Dispose all PGX names.
  while((n = m_pNameList)) {
    m_pNameList = n->m_pNext;
    delete n;
  }
}
///

/// SimplePgx::LoadImage
// Load an image from an already open (binary) PPM or PGM file
// Throw in case the file should be invalid.
void SimplePgx::LoadImage(const char *basename,struct ImgSpecs &specs)
{
  struct ComponentName   *name;
  struct ComponentLayout *layout;
  ULONG w,h;
  UWORD depth = 0;
  char buffer[256 + 4];
  bool embedded = false; // if the header is embedded in the file and the file names are generated.
  bool single   = false;
  bool yuv      = false;
  File file(basename,"r");
  //
  if (m_pComponent) {
    PostError("Image is already loaded.\n");
  }
  //
  // Read the names, one after another.
  do {
    size_t len;
    if (!embedded) {
      if (fgets(buffer,255,file) == NULL) {
	break;
      }
    }
    //
    // Is this probably not the header file but the first data file?
    if (depth == 0 && buffer[0] == 'P' && (buffer[1] == 'G' || buffer[1] == 'F') && buffer[2] == ' ') {
      const char *dot = strrchr(basename,'.');
      if (dot && !strcmp(dot,".pgx") && dot - 1 > basename && dot < basename + 256) {
	if (dot[-1] == '0') {
	  embedded = true;
	} else {
	  embedded = true;
	  single   = true;
	}
      }
    }
    // If embedded, try to algorithmically determine the next file name by replacing the digits before
    // the basename.
    if (embedded) {
      if (single) {
	if (depth != 0)
	  break;
	strcpy(buffer,basename);
      } else {
	FILE *tmp;
	const char *dot  = strrchr(basename,'.');
	int dotpos = dot - 1 - basename;
	memcpy(buffer,basename,dotpos);
	sprintf(buffer+dotpos,"%d.pgx",depth);
	tmp = fopen(buffer,"rb");
	if (!tmp)
	  break;
	fclose(tmp);
      }
    }
    //
    // Check for a terminating /n and remove it.
    len = strlen(buffer);
    while(len > 0) {
      if (isspace(buffer[len-1])) {
	buffer[--len] = '\0';
      } else break;
    }
    if (len > 0) {
      struct ComponentName **last = &m_pNameList;
      // In case the buffer does not contain any relative component, assume that
      // the name is relative to the base name and construct the path name
      // accordingly.
      if (!strchr(buffer,PATH_SEP)) {
	const char *last = strrchr(basename,PATH_SEP);
	if (last) {
	  memmove(&buffer[last - basename + 1],buffer,strlen(buffer)+1);
	  memcpy(buffer,basename,last - basename + 1);
	}
      }
      name  = new struct ComponentName(buffer);
      // Yup, there's really a file name left, attach it to the end.
      while(*last) {
	last = &((*last)->m_pNext);
      }
      *last = name;
      depth++;
    }
  } while(true);
  //
  if (ferror(file)) {
    PostError("failure on reading the pgx component list file %s\n",basename);
  }
  //
  // Now parse the headers.
  name = m_pNameList;
  while(name) {
    char *data,*last,*dptr;
    FILE *header;
    size_t len;
    int depth;
    // Copy the name over, and replace the .raw with .h.
    strncpy(buffer,name->m_pName,256);
    if (!embedded) {
      char *cr = strchr(buffer,'\r');
      // Windows binary reader needs to get rid of the \r
      if (cr)
	*cr = '\0';
      len = strlen(buffer);
      if (len > 4) {
	if (!strcmp(buffer + len - 4,".raw"))
	  buffer[len - 4] = 0;
      }
      strcat(buffer,".h");
    }
    // Note that this is the header which is opened for ascii
    // transport.
    header = fopen(buffer,"r");

    if (header == NULL) {
      PostError("cannot open the pgx header file %s\n",buffer);
    }
    //
    // Read the header into the file. This should be one
    // single stream.
    data = fgets(buffer,255,header);
    fclose(header);
    if (data == NULL) {
      PostError("failed to read the pgx header file.\n");
    }
    //
    // Check whether the file header is fine. We only
    // support big endian PGX files, this is enough for
    // part4 compliance. We could also support
    // little endian files.
    if (!memcmp(buffer,"PF LM ",6)) {
      name->m_bLE        = true;
      specs.LittleEndian = ImgSpecs::Yes;
      name->m_bFloat     = true;
    } else if (!memcmp(buffer,"PF ML ",6)) {
      specs.LittleEndian = ImgSpecs::No;
      name->m_bFloat     = true;
    } else if (!memcmp(buffer,"PG LM ",6)) {
      specs.LittleEndian = ImgSpecs::Yes;
      name->m_bLE        = true;
    } else if (!memcmp(buffer,"PG ML ",6)) {
      specs.LittleEndian = ImgSpecs::No;
    } else {
      PostError("invalid PGX file, PGX identifier %s broken",buffer);
    }
    // The next must be + or -, defining the sign.
    dptr = buffer + 7;
    if (buffer[6] == '+' || buffer[6] == ' ') {
      name->m_bSigned = false;
    } else if (buffer[6] == '-') {
      name->m_bSigned = true;
    } else if (buffer[6] >= '0' && buffer[6] <= '9') {
      // Signedness not indicated, this is the depth. Assume unsigned.
      name->m_bSigned = false;
      dptr--;
    } else {
      PostError("invalid PGX file, PGX signedness %c broken\n",buffer[6]);
    }
    // Get the bit depth of the component.
    depth = strtol(dptr,&last,10);
    // Currently, not more than 16bpp.
    if (last <= dptr || last[0] != ' ' || depth <= 0 || depth > 64) {
      PostError("invalid PGX file, bit depth invalid\n");
    }
    name->m_ucDepth = depth;
    //
    data = last + 1;
    name->m_ulWidth = strtol(data,&last,10);
    if (last <= data || last[0] != ' ') {
      PostError("invalid PGX file, width invalid\n");
    }
    data = last + 1;
    name->m_ulHeight = strtol(data,&last,10);
    if (last <= data || !isspace(last[0])) {
      PostError("invalid PGX file, height invalid\n");
    }
    //
    // All done with this file. Get the next.
    name = name->m_pNext;
  }
  //
  // Find the maximum width, height as base for the
  // subsampling. 
  name = m_pNameList;
  w    = 0;
  h    = 0;
  while(name) {
    if (name->m_ulWidth > w)
      w = name->m_ulWidth;
    if (name->m_ulHeight > h)
      h = name->m_ulHeight;
    //
    name = name->m_pNext;
  }
  //
  // Setup the component list and the subsampling.
  CreateComponents(w,h,depth);
  //
  specs.ASCII      = ImgSpecs::No;
  specs.Palettized = ImgSpecs::No;
  //
  // Fill in the subsampling.
  name   = m_pNameList;
  layout = m_pComponent;
  while(name) {
    ULONG size = name->m_ulWidth * name->m_ulHeight;
    UBYTE bypp = ImageLayout::SuggestBPP(name->m_ucDepth,name->m_bFloat);
    FILE *raw;
    //
    layout->m_ucBits          = name->m_ucDepth;
    layout->m_bSigned         = name->m_bSigned;
    layout->m_bFloat          = name->m_bFloat;
    // this is only an approximation. Urgh. We don't know the left edge...
    layout->m_ucSubX          = w / name->m_ulWidth;
    // ditto. Same problem.  
    layout->m_ucSubY          = h / name->m_ulHeight;
    // Make a best guess wether this is yuv.
    if (depth >= 3 && 
	(layout->m_ucSubX > m_pComponent->m_ucSubX ||
	 layout->m_ucSubY > m_pComponent->m_ucSubY)) {
      yuv                     = true;
    }
    layout->m_ulWidth         = name->m_ulWidth;
    layout->m_ulHeight        = name->m_ulHeight;
    layout->m_ulBytesPerRow   = bypp * name->m_ulWidth;
    layout->m_ulBytesPerPixel = bypp;
    // allocate memory for this component.
    layout->m_pPtr            = name->m_pData = new UBYTE[name->m_ulWidth * name->m_ulHeight * bypp];
    //
    // Now read the data from the raw file.
    raw  = fopen(name->m_pName,"rb");
    if (raw == NULL) {
      PostError("unable to open the PGX raw data file %s\n",name->m_pName);
    }
    if (embedded) {
      int c;
      // Read off the first line.
      while((c = fgetc(raw)) != -1 && c != '\n'){}
      //
      if (c == -1) {
	fclose(raw);
	PostError("invalid data header in embedded PGX file %s\n",name->m_pName);
      }
    }
    // If we have here single bytes, we can parse off the file completely.
    if (name->m_ucDepth <= 8) {
      if (fread(name->m_pData,1,size,raw) != size) {
	fclose(raw);
	PostError("incomplete PGX data file %s\n",name->m_pName);
      }
    } else if (name->m_ucDepth == 16 && name->m_bFloat) {
      // The hacky case for half-float support.
      // We must read the data byte by byte due to endian issues.
      FLOAT *data = (FLOAT *)name->m_pData;
      while(size) {
	int in1,in2;
	in1 = fgetc(raw);
	in2 = fgetc(raw);
	if (in1 < 0 || in2 < 0) {
	  fclose(raw);
	  PostError("incomplete PGX data file %s\n",name->m_pName);
	}
	if (name->m_bLE) {
	  *data++ = H2F((in2 << 8) | in1); // is little endian
	} else {
	  *data++ = H2F((in1 << 8) | in2); // is big endian
	}
	size--;
      }
    } else if (name->m_ucDepth <= 16) {
      // We must read the data byte by byte due to endian issues.
      UWORD *data = (UWORD *)name->m_pData;
      while(size) {
	int in1,in2;
	in1 = fgetc(raw);
	in2 = fgetc(raw);
	if (in1 < 0 || in2 < 0) {
	  fclose(raw);
	  PostError("incomplete PGX data file %s\n",name->m_pName);
	}
	if (name->m_bLE) {
	  *data++ = (in2 << 8) | in1; // is little endian
	} else {
	  *data++ = (in1 << 8) | in2; // is big endian
	}
	size--;
      }
    } else if (name->m_ucDepth <= 32) { 
      ULONG *data = (ULONG *)name->m_pData;
      while(size) {
	int in1,in2,in3,in4;
	in1 = fgetc(raw);
	in2 = fgetc(raw);
	in3 = fgetc(raw);
	in4 = fgetc(raw);
	if (in1 < 0 || in2 < 0 || in3 < 0 || in4 < 0) {
	  fclose(raw);
	  PostError("incomplete PGX data file %s\n",name->m_pName);
	}
	if (name->m_bLE) {
	  *data++ = (ULONG(in4) << 24) | (ULONG(in3) << 16) | (ULONG(in2) << 8) | ULONG(in1); // is little endian
	} else {
	  *data++ = (ULONG(in1) << 24) | (ULONG(in2) << 16) | (ULONG(in3) << 8) | ULONG(in4); // is big endian
	}
	size--;
      }
    } else if (name->m_ucDepth <= 64) {  
      UQUAD *data = (UQUAD *)name->m_pData;
      while(size) {
	int in1,in2,in3,in4,in5,in6,in7,in8;
	in1 = fgetc(raw);
	in2 = fgetc(raw);
	in3 = fgetc(raw);
	in4 = fgetc(raw);
	in5 = fgetc(raw);
	in6 = fgetc(raw);
	in7 = fgetc(raw);
	in8 = fgetc(raw);
	if (in1 < 0 || in2 < 0 || in3 < 0 || in4 < 0 || in5 < 0 || in6 < 0 || in7 < 0 || in8 < 0) {
	  fclose(raw);
	  PostError("incomplete PGX data file %s\n",name->m_pName);
	}
	if (name->m_bLE) {
	  *data++ = (UQUAD(in8) << 56) | (UQUAD(in7) << 48) | (UQUAD(in6) << 40) | (UQUAD(in5) << 32) |
	    (UQUAD(in4) << 24) | (UQUAD(in3) << 16) | (UQUAD(in2) << 8) | UQUAD(in1); // is little endian
	} else {
	  *data++ = (UQUAD(in1) << 56) | (UQUAD(in2) << 48) | (UQUAD(in3) << 40) | (UQUAD(in4) << 32) |
	    (UQUAD(in5) << 24) | (UQUAD(in6) << 16) | (UQUAD(in7) << 8) | UQUAD(in8); // is big endian
	}
	size--;
      }
    }
    if (ferror(raw)) {
      fclose(raw);
      PostError("unable to read PGX data file %s\n",name->m_pName);
    }
    fclose(raw);
    name = name->m_pNext;
    layout++;
  }
  //
  // Insert the yuv flag into the specs.
  if (specs.YUVEncoded == ImgSpecs::Unspecified)
    specs.YUVEncoded = yuv?(ImgSpecs::Yes):(ImgSpecs::No);
}
///

/// SimplePgx::SaveImage
// Save the image to a PGM/PPM file, throw in case of error.
void SimplePgx::SaveImage(const char *basename,const struct ImgSpecs &specs)
{
  File output(basename,"w");
  int i;
  char buffer[256+4];
  bool le = (specs.LittleEndian == ImgSpecs::Yes)?(true):(false);
  //
  // Write the file header containing the references to
  // to all the components.
  for(i = 0;i < m_usDepth; i++) {
    struct ComponentLayout *cl = m_pComponent + i;
    ULONG w,h;
    FILE *raw,*hdr;
    sprintf(buffer,"%s_%d.raw",basename,i);
    fprintf(output,"%s\n",buffer);
    //
    // Compute width and height of this component.
    w = cl->m_ulWidth;
    h = cl->m_ulHeight;
    //
    // Write the raw output file.
    raw = fopen(buffer,"wb");
    if (raw) {
      ULONG x,y;
      UBYTE *p = (UBYTE *)cl->m_pPtr;
      for(y = 0;y < h;y++) {
	for(x = 0; x < w; x++) {
	  if (cl->m_ucBits > 64) {
	    PostError("Data size is too large, at most 64 bits per pixel\n");
	  } else if (cl->m_ucBits > 32) {
	    UQUAD data = *(QUAD *)p; // we have a quad pointer.
	    if (le) {
	      putc(data      ,raw);
	      putc(data >> 8 ,raw);
	      putc(data >> 16,raw);
	      putc(data >> 24,raw);
	      putc(data >> 32,raw);
	      putc(data >> 40,raw);
	      putc(data >> 48,raw);
	      putc(data >> 56,raw);
	    } else {
	      putc(data >> 56,raw);
	      putc(data >> 48,raw);
	      putc(data >> 40,raw);
	      putc(data >> 32,raw);
	      putc(data >> 24,raw);
	      putc(data >> 16,raw);
	      putc(data >> 8 ,raw);
	      putc(data      ,raw);
	    }
	  } else if (cl->m_ucBits > 16) {
	    ULONG data = *(ULONG *)p; // we have a long pointer.
	    // Write the data in big-endianness.
	    if (le) {
	      putc(data      ,raw);
	      putc(data >> 8 ,raw);
	      putc(data >> 16,raw);
	      putc(data >> 24,raw);
	    } else {
	      putc(data >> 24,raw);
	      putc(data >> 16,raw);
	      putc(data >> 8 ,raw);
	      putc(data      ,raw);
	    }
	  } else if (cl->m_ucBits == 16 && cl->m_bFloat) {
	    // Special hack: Half-floats are internally stored as floats.
	    HALF data  = F2H(*(FLOAT *)p);
	    if (le) {
	      putc(data     ,raw);
	      putc(data >> 8,raw);
	    } else {
	      putc(data >> 8,raw);
	      putc(data     ,raw);
	    }
	  } else if (cl->m_ucBits > 8) {
	    UWORD data = *(UWORD *)p; // we have a word pointer.
	    if (le) {
	      putc(data     ,raw);
	      putc(data >> 8,raw);
	    } else {
	      putc(data >> 8,raw);
	      putc(data     ,raw);
	    }
	  } else {
	    putc(*p,raw);
	  }
	  p += cl->m_ulBytesPerPixel;
	}
	p += cl->m_ulBytesPerRow - w * cl->m_ulBytesPerPixel;
      }
      if (ferror(raw)) {
	fclose(raw);
	PostError("failed to write the raw image data to %s\n",buffer);
      }
      fclose(raw);
    } else {
      PostError("failed to open %s\n",buffer);
    }
    //
    // Now write the image stats to the header file.
    sprintf(buffer,"%s_%d.h",basename,i);
    hdr = fopen(buffer,"w");
    if (hdr) {
      fprintf(hdr,"P%c %s %c%d %lu %lu\n",
	      cl->m_bFloat?('F'):('G'),	 
	      le?("LM"):("ML"),
	      cl->m_bSigned?('-'):('+'),cl->m_ucBits,
	      (unsigned long)(w),(unsigned long)(h));
      if (ferror(hdr)) {
	fclose(hdr);
	PostError("failed to write the image header to %s\n",buffer);
      }
      fclose(hdr);
    } else {
      PostError("failed to open %s\n",buffer);
    }
  }
}
///
