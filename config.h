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
** Config file for difftest_ng
** $Id: config.h,v 1.8 2017/01/31 11:57:58 thor Exp $
**
*/

#ifndef CONFIG_H
#define CONFIG_H

/// Autoconfig inclusion
//
// Remove the following comment if you do NOT want to use
// the automatically generated configuration but want to
// configure in this file.
//#undef USE_AUTOCONF
//
// Set the following to make use of the
// autoconf configuration
#ifdef USE_AUTOCONF
#include "autoconfig.h"
//
#if defined(HAVE_GSL_FFT_COMPLEX_BACKWARD) && defined(HAVE_GSL_FFT_COMPLEX_FORWARD) && defined(HAVE_GSL_FFT_COMPLEX_WORKSPACE_ALLOC) && defined(HAVE_GSL_FFT_COMPLEX_WORKSPACE_FREE) && defined(HAVE_GSL_GSL_FFT_COMPLEX_H)
# define USE_GSL
#endif
//
#if defined(HAVE_PNG_H) && defined(HAVE_PNG_INIT_IO)
# define USE_PNG
#endif
//
#if defined(HAVE_IMATHBOX_H) && defined(HAVE_IMFINPUTFILE_H) && defined(HAVE_IMFRGBAFILE_H) && defined(HAVE_HALF_H)
# define USE_EXR
#endif
//
//
// Pull additional settings in?
#ifdef ADDON_FILE
# include "addons/addons.h"
#endif
#ifdef HAVE_NORETURN
# define NORETURN       __attribute__ ((noreturn))
#else
# define NORETURN
#endif
#ifdef HAS_ALIGNED
# define FORCE_ALIGNED __attribute__ ((aligned))
#else
# define FORCE_ALIGNED
#endif
#ifdef HAVE_ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__ ((always_inline))
#define FORCEINLINE __attribute__ ((always_inline))
#else
#define ALWAYS_INLINE
#endif
#ifdef HAS_VISIBILITY_INTERNAL
# ifndef J2K_EXPORT
#  ifdef BUILD_LIB
#   define J2K_EXPORT __attribute__ ((visibility ("default")))
#  else
#   define J2K_EXPORT 
#  endif
# endif
#endif
#ifdef HAS_VISIBILITY_HIDDEN
# define J2K_HIDDEN __attribute__ ((visibility ("hidden")))
#endif
#ifndef FORCEINLINE
# define FORCEINLINE
#endif
#ifndef J2K_EXPORT
# define J2K_EXPORT
#endif
#ifndef TYPE_CDECL
# define TYPE_CDECL
#endif
#ifndef J2K_HIDDEN
# define J2K_HIDDEN
#endif
#if HAVE_RESTRICTED_PTRS
# define NONALIASED __restrict__
#else
# define NONALIASED
#endif
#else
///

/// Static type sizes
//
// Various project settings: Set the following define to enable
// special optimizations for little-endian machines.
//#define J2K_LIL_ENDIAN
//
// Compiler characteristics, edit to match your compiler and system.
// sizes in bytes.
#define SIZEOF_CHAR 1
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
// Stephen B: Make sure Solaris 64 bit code does the right thing.
#if defined(__64BIT__) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64)
# define SIZEOF_LONG 8
#else
# define SIZEOF_LONG 4
#endif
// In case your compiler doesn't like typedefs to void *, change the
// following line:
//#define J2K_NOVOIDPTR
///

/// VS settings
#if defined(WIN32) || defined(WIN64)
//
// Disable VCC warnings. Yes, it's evil...
#ifdef _MSC_VER
#   pragma warning(4: 4146 4244 4334)
#   define SIZEOF___INT64 8
#   define FORCEINLINE __forceinline
#   define TYPE_CDECL __cdecl
#else
#   define FORCEINLINE
#   define TYPE_CDECL
#endif
//
// Use the assembly-lock-prefix for atomic addition?
#ifdef WIN32
# define USE_I386_XADD
# define NATURAL_ALIGNMENT 4
#endif
#ifdef WIN64
# define USE_INTERLOCKED
# define NATURAL_ALIGNMENT 4
#endif
#define NORETURN
#define HAVE_IO_H 1
#define HAVE_ASSERT_H 1
#define HAVE_GETTICKCOUNT 1
#define HAVE_WINDOWS_H 1
#define HAVE__LSEEKI64 1
#define HAS_CONST_CAST 1
#define HAS_REINTERPRET_CAST 1
#define NONALIASED
#ifndef J2K_EXPORT
# ifdef BUILD_LIB
#  define J2K_EXPORT __declspec(dllexport)
# else
#  define J2K_EXPORT 
# endif
#endif
#ifndef J2K_HIDDEN
# define J2K_HIDDEN
#endif
#ifndef FORCE_ALIGNED
# define FORCE_ALIGNED
#endif
// Enable SDL depending on a macro.
#ifdef HAVE_SIMPLE_DIRECT_MEDIA
# define HAVE_SDL_INIT 1
# define HAVE_SDL_INITSUBSYSTEM 1
# define HAVE_SDL_OPENAUDIO 1
# define HAVE_SDL_SDL_H 1
# define HAVE_SDL_SETVIDEOMODE 1
#endif
// Enable workaround for missing vsnprintf. This is not
// mission-critical and just a debugging thing, luckely.
#if !(defined(_VS8_) || defined(_VS7_) || defined(_VS6_))
# define HAVE_VSNPRINTF 1
# define HAVE_SNPRINTF 1
#endif
//
// Enable C99 types
// Bummer! None of the VS compilers does have stdint.h
//
// Disable the bogus deprecation warnings of VS8
#ifdef _VS8_
#define _CRT_SECURE_NO_DEPRECATE(a)
#endif
// Disable this dumb VS 6 warning about a "this" identifier
// used in the constructor. This is all valid here...
#ifdef _VS6_
#pragma warning( disable : 4355 ) 
// Enable a VS6 bug workaround
#define AMBIGIOUS_NEW_BUG 1
#endif
// Define this if the clock() function is available
#define HAVE_CLOCK 1
// Define if we provide pegasus style cycle counting.
#define PCYCLES_AVAILABLE 1
// Set the following if we may use the pentium built-in
// cycle counter for performance counting.
#if !defined(USE_ASSEMBLY) || USE_ASSEMBLY
# define USE_PENTIUM_TSC 1
#endif
//
// Is this little-endian?
#ifndef J2K_LIL_ENDIAN
#define J2K_LIL_ENDIAN 1
#endif
#undef J2K_BIG_ENDIAN
//
// Is this big-endian?
// #ifndef J2K_BIG_ENDIAN
// #define J2K_BIG_ENDIAN 1
// #endif
// #undef J2K_LIL_ENDIAN
//
// Default header availibility
#define HAVE_CTYPE_H 1
#define HAVE_ISSPACE 1
#define HAVE_MATH_H 1
#define HAVE_SETJMP_H 1
#define HAVE_SETJMP 1
#define HAVE_LONGJMP 1
#define HAVE_STDARG_H 1
#define HAVE_ERRNO_H 1
#define HAVE_STDDEF_H 1
#define HAS_PTRDIFF_T 1
#define HAVE_STDIO_H 1
#define HAVE_STRTOL 1
#define HAVE_STRTOD 1
#define HAVE_STDLIB_H 1
#define STD_HEADERS 1
#define HAVE_STRING_H 1
#define HAVE_MEMCHR 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_STRCHR 1
#define HAVE_STRRCHR 1
#define HAVE_STRERROR 1
#define HAVE_OPEN 1
#define HAVE_RENAME 1
#define HAVE_CLOSE 1
#define HAVE_READ 1
#define HAVE_WRITE 1
#define HAVE_MALLOC 1
#define HAVE_FREE 1
#define HAVE_FCNTL_H 1
// Define for NT style semaphores and threading
#define USE_NT_SEMAPHORES 1
#define USE_NT_MUTEXES 1
#define USE_NT_THREADS 1

#else
///

/// Generic iX settings
// Define this if the MS/win32 header io.h is available.
//#define HAVE_IO_H 1
// Define this if the POSIX unistd.h header is available.
#define HAVE_UNISTD_H 1
#define HAVE_CYTPE_H 1
#define HAVE_ISSPACE 1
#define HAVE_MATH_H 1
#define HAVE_SETJMP_H 1
#define HAVE_SETJMP 1
#define HAVE_LONGJMP 1
#define HAVE_STDARG_H 1
#define HAVE_ERRNO_H 1
#define HAS_PTRDIFF_T 1
#define HAVE_STDIO_H 1
#define HAVE_STRTOL 1
#define HAVE_STRTOD 1
#define HAVE_STDLIB_H 1
#define STD_HEADERS 1
#define HAVE_STRING_H 1
#define HAVE_MEMCHR 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_STRCHR 1
#define HAVE_STRRCHR 1
#define HAVE_STRERROR 1
#define HAVE_FCNTL_H 1
#define HAVE_OPEN 1
#define HAVE_RENAME 1
#define HAVE_CLOSE 1
#define HAVE_SLEEP 1
#define HAVE_READ 1
#define HAVE_WRITE 1
#define HAVE_MALLOC 1
#define HAVE_FREE 1
#define HAVE_NETINET_IN_H 1
#define HAVE_HTONL 1
#define HAVE_HTONS 1
#define HAVE_NTOHL 1
#define HAVE_NTOHS 1
#define HAVE_ASSERT_H 1
#define HAVE_SYSTEM 1
#define HAVE_VSNPRINTF 1
#define FORCEINLINE
#define TYPE_CDECL
#define HAS_CONST_CAST 1
#define HAS_REINTERPRET_CAST 1
#define NONALIASED __restrict__
#define HAS_STDIN_FILENO 1
#define HAS_STDOUT_FILENO 1
#define HAS_STDERR_FILENO 1

// Define this if time.h and sys/time.h can be included
// commonly.
#define TIME_WITH_SYS_TIME 1

// Define this if TIME_H is available
#define HAVE_TIME_H 1

// Define this if the clock() function is available
#define HAVE_CLOCK 1

// Define this if gettimeofday is available
#define HAVE_GETTIMEOFDAY 1

// Define this if gettickcount is available
#undef HAVE_GETTICKCOUNT

// Define this if the Windows.h header is available
#undef HAVE_WINDOWS_H

// Define this if the compiler supports the noreturn attribute
#define HAVE_NORETURN 1

#endif
///

/// GNU settings
// GCC specific settings
#ifdef __GNUC__
# define SIZEOF_LONG_LONG 8
//
// Specify whether we can capture a segfault.
# define HAVE_SIGNAL_H 1
# define HAVE_SIGSEGV 1
# define HAVE_SIGNAL 1
# define HAS__NULL_TYPE 1
# ifndef NORETURN
#  define NORETURN       __attribute__ ((noreturn))
# endif
# ifndef ALWAYS_INLINE
#  define ALWAYS_INLINE __attribute__ ((always_inline))
# endif
# ifndef FORCEINLINE
#  define FORCEINLINE __attribute__ ((always_inline))
# endif
# ifndef FORCE_ALIGNED
#  define FORCE_ALIGNED __attribute__ ((aligned))
# endif
# ifndef J2K_EXPORT
#  define J2K_EXPORT
# endif
# ifndef J2K_HIDDEN
#  define J2K_HIDDEN
# endif
# define USE_GSL
#endif
///

/// ICC settings
// ICC specific settings.
#ifdef __ICC__
# define SIZEOF___INT64 8
# ifndef NORETURN
#  define NORETURN
# endif
# ifndef ALWAYS_INLINE
#  define ALWAYS_INLINE
# endif
# ifndef FORCEINLINE
#  define FORCEINLINE
# endif
# ifndef J2K_EXPORT
#  define J2K_EXPORT
# endif
# ifndef J2K_HIDDEN
#  define J2K_HIDDEN
# endif
# ifndef FORCE_ALIGNED
#  define FORCE_ALIGNED
# endif
#endif

#endif // of if USE_AUTOCONF
///

///
#endif
