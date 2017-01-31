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
** This class provides a RAII access to stdio files.
**
** $Id: file.hpp,v 1.7 2017/01/31 11:58:05 thor Exp $
**
*/

#ifndef TOOLS_FILE_HPP
#define TOOLS_FILE_HPP

/// Includes
#include "std/stdio.hpp"
///

/// Class File
class File {
  //
  // The file pointer itself.
  FILE *m_pFile;
  //
public:
  File(const char *filename,const char *mode);
  //
  ~File(void)
  {
    if (m_pFile)
      fclose(m_pFile);
  }
  //
  // Return the raw file pointer.
  operator FILE* (void) const
  {
    return m_pFile;
  }

  FILE * operator-> (void) const
  {
    return m_pFile;
  }
};
///

///
#endif
