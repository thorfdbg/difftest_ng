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
 * Type definition: Some system independent type definitions
 * (thor's pecularities)
 * $Id: types.cpp,v 1.2 2014/01/04 11:35:29 thor Exp $
 *
 * The following header defines basic types to be used in the J2K interface
 * routines. Especially, this file must be adapted if your compiler has
 * different ideas what a "unsigned long" is as we *MUST* fix the width
 * of elementary data types. Especially, do not use types not defined here for
 * interface glue routines.
 *
 * This is the "internal" header file defining internal types, importing the
 * types from the external "j2ktypes" header.
 */

