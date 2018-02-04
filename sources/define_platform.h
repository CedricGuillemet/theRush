///////////////////////////////////////////////////////////////////////////////////////////////////
//
//      __   __                              __
//     |  |_|  |--.-----. .----.--.--.-----.|  |--.
//   __|   _|     |  -__| |   _|  |  |__ --||     |
//  |__|____|__|__|_____| |__| |_____|_____||__|__|
//
//  Copyright (C) 2007-2013 Cedric Guillemet
//
// This file is part of .the rush//.
//
//    .the rush// is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    .the rush// is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with .the rush//.  If not, see <http://www.gnu.org/licenses/>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DEFINE_PLATFORM_H__
#define DEFINE_PLATFORM_H__

#define PLATFORM_OS_WINDOWS    1
#define PLATFORM_OS_LINUX      2
#define PLATFORM_OS_MACOSX     3

//NOTE: all predefine C/C++ compiler macros: http://sourceforge.net/apps/mediawiki/predef/

#define PLATFORM_COMPILER_MSVC  1
#define PLATFORM_COMPILER_GCC   2

#if defined( _MSC_VER )
#   define PLATFORM_COMPILER            PLATFORM_COMPILER_MSVC
#   define PLATFORM_COMPILER_VERSION    _MSC_VER
#   define IS_COMPILER_MSVC     1
#   define IS_COMPILER_GCC      0

#   define PRAGMA_MESSAGE(x) __pragma( message(#x) )
    PRAGMA_MESSAGE("Platform Compiler is Microsoft Visual C++.")
#elif defined( __GNUC__ )
#   define PLATFORM_COMPILER            PLATFORM_COMPILER_GCC
#   define PLATFORM_COMPILER_VERSION    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)
#   define IS_COMPILER_MSVC     0
#   define IS_COMPILER_GCC      1

#   define DO_PRAGMA(x)      _Pragma(#x)
#   define PRAGMA_MESSAGE(x) DO_PRAGMA(message(#x))
   PRAGMA_MESSAGE("Platform Compiler is GCC.")
#else
#   error "This compiler is not supported."
#endif


#if defined( __WIN32__ ) || defined( _WIN32 ) || defined( __WIN64__ ) || defined( _WIN64 ) || defined( WIN32 )
#   define IS_OS_WINDOWS    1
#   define IS_OS_LINUX      0
#   define IS_OS_MACOSX     0
#   define PLATFORM_OS      PLATFORM_OS_WINDOWS
    PRAGMA_MESSAGE("Platform OS is Windows.")
#elif defined(__linux__) || defined( LINUX )
#   define IS_OS_WINDOWS    0
#   define IS_OS_LINUX      1
#   define IS_OS_MACOSX     0
#   define PLATFORM_OS      PLATFORM_OS_LINUX
    PRAGMA_MESSAGE("Platform OS is Linux.")
#elif ( defined(__APPLE__) && defined(__MACH__) )  || defined( MACOSX )
#   define IS_OS_WINDOWS    0
#   define IS_OS_LINUX      0
#   define IS_OS_MACOSX     1
#   define PLATFORM_OS      PLATFORM_OS_MACOSX
    PRAGMA_MESSAGE("Platform OS is MacOSX.")
#else
#   error "This platform is not supported."
#endif


#define PLATFORM_MEMORY_ADDRESS_SPACE_32BIT  1
#define PLATFORM_MEMORY_ADDRESS_SPACE_64BIT  2

#if defined(__x86_64__) || defined(_M_X64) || defined(__powerpc64__)
#   define IS_PLATFORM_64BIT                1
#   define IS_PLATFORM_32BIT                0
#   define PLATFORM_MEMORY_ADDRESS_SPACE    PLATFORM_MEMORY_ADDRESS_SPACE_64BIT
    PRAGMA_MESSAGE("Using 64bit memory address space.")
#else
#   define IS_PLATFORM_64BIT                0
#   define IS_PLATFORM_32BIT                1
#   define PLATFORM_MEMORY_ADDRESS_SPACE    PLATFORM_MEMORY_ADDRESS_SPACE_32BIT
    PRAGMA_MESSAGE("Using 32bit memory address space.")
#endif

#if IS_OS_WINDOWS
    #if !defined(snprintf)
        #define snprintf sprintf_s
    #endif
#endif

#if IS_OS_WINDOWS
    #if !defined(vnsprintf)
        #define vnsprintf vsprintf_s
    #endif
#elif IS_OS_LINUX
    #if !defined(vnsprintf)
        #define vnsprintf vsnprintf
    #endif
#endif

#endif	//	DEFINE_PLATFORM_H__
