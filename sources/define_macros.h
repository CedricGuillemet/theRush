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

#ifndef DEFINE_MACROS_H__
#define DEFINE_MACROS_H__


#if !defined(PLATFORM_OS)
#   error "define_platform.h should be included first."
#endif


//NOTE: introducing macros to help declaring a multi line macro (See http://cnicholson.net/2009/03/stupid-c-tricks-dowhile0-and-c4127/)
#define DECLARE_MACRO_BEGIN             \
    do {

#if IS_OS_WINDOWS
//NOTE: disabling the conditional expression is constant warning caused by 'while(0)'
#define DECLARE_MACRO_END               \
    __pragma(warning(push))             \
    __pragma(warning(disable:4127))     \
    } while(0)                          \
    __pragma(warning(pop))

#else
#define DECLARE_MACRO_END   } while(0)
#endif


//NOTE: introducing macro to help getting rid of warning C4100: unreferenced formal parameter
#define UNUSED_PARAMETER(x)         ((void)(x))
//NOTE: some other implementation suggestions I found on the web:
//( *(volatile typeof(x) *)&(x) = (x); )
//for GCC: UNUSED_ ## x __attribute__((unused))


//NOTE: isnan() and isfinite() might only be available with C99
//See http://stackoverflow.com/questions/570669/checking-if-a-double-or-float-is-nan-in-c
#if IS_COMPILER_MSVC
#   define IS_NAN(x) ( _isnan(x) || !_finite(x) )
#elif IS_COMPILER_GCC
#   if IS_OS_LINUX
#       define IS_NAN(x) ( isnan(x) || !isfinite(x) )
#   elif IS_OS_MACOSX
#       define IS_NAN(x) ( __inline_isnand(x) )
#   endif
#endif


//NOTE: most compilers define __PRETTY_FUNCTION__ except MSVC
#if IS_COMPILER_MSVC
#   if !defined(__PRETTY_FUNCTION__)
#       define __PRETTY_FUNCTION__ __FUNCSIG__
//#       define __PRETTY_FUNCTION__ __FUNCDNAME__
//#       define __PRETTY_FUNCTION__ __FUNCTION__
#   endif
#endif


#if !defined(DEBUG_ENABLE)
#   if defined(_DEBUG) || !defined(RETAIL)
#       define DEBUG_ENABLE   1
#   else
#       define DEBUG_ENABLE   0
#   endif
#endif


#if !defined(ASSERT_ENABLE)
#   define ASSERT_ENABLE	DEBUG_ENABLE
#endif

#if !defined(MEMORY_DEBUG_ENABLE)
#   define MEMORY_DEBUG_ENABLE	DEBUG_ENABLE
#endif

//HACK: this is introduced to check from code if exception handling is enabled
//Needed to compile tinythread 3rdparty code on Linux with g++
#if !defined(CPP_EXCEPTION_ENABLE)
#   define CPP_EXCEPTION_ENABLE     0
#endif

#if !defined(EDITOR_ENABLE)
#   if !defined(RETAIL) && IS_OS_WINDOWS
#       define EDITOR_ENABLE   1
#   else
#       define EDITOR_ENABLE   0
#   endif
#endif

#if !defined(LOG_ENABLE)
#   if !defined(RETAIL)
#       define LOG_ENABLE   1
#   else
#       define LOG_ENABLE   0
#   endif
#endif

#if !defined(USE_LOG_TO_FILE)
#   define USE_LOG_TO_FILE 0
#endif

#if !defined(USE_PROFILER)
#   define USE_PROFILER 0
#endif

#if !defined(RENDER_SHADOWS_ENABLE)
#   define RENDER_SHADOWS_ENABLE	!IS_OS_MACOSX
#endif

#if !defined(RENDER_OCEAN_ENABLE)
#   define RENDER_OCEAN_ENABLE	!IS_OS_MACOSX
#endif

#if !defined(RENDER_REFLECTION_ENABLE)
#   define RENDER_REFLECTION_ENABLE	!IS_OS_MACOSX
#endif

#endif	//	DEFINE_MACROS_H__
