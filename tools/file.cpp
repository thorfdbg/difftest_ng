/*************************************************************************
** Copyright (c) 2003-2016 Accusoft 				        **
**									**
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
** $Id: file.cpp,v 1.5 2016/06/04 10:44:10 thor Exp $
**
*/

/// Includes
#include "std/string.hpp"
#include "std/errno.hpp"
#include "tools/file.hpp"
#include "img/imglayout.hpp"
///

/// File::File
File::File(const char *filename,const char *mode)
  : m_pFile(NULL)
{
  m_pFile = fopen(filename,mode);
  if (m_pFile == NULL) {
    ImageLayout::PostError("unable to open %s: %s\n",filename,strerror(errno));
  }
}
///
