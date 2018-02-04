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

#ifndef DEFINE_TYPES_H__
#define DEFINE_TYPES_H__


#if IS_OS_WINDOWS
typedef __int32 int32;
typedef unsigned __int32 uint32;
#elif IS_OS_LINUX
#include <stdint.h>
#include <limits.h>
typedef int32_t int32;
typedef uint32_t uint32;
#elif IS_OS_MACOSX
#include <sys/types.h>
typedef int32_t int32;
typedef u_int32_t uint32;
#endif

typedef unsigned char u8;
typedef unsigned char uint8;
typedef unsigned short u16;
typedef unsigned short uint16;
typedef uint32 u32;
typedef unsigned int uint;

#define U8_MIN  0
#define U8_MAX  UCHAR_MAX
#define U16_MIN 0
#define U16_MAX USHRT_MAX
#define U32_MIN 0
#define U32_MAX UINT_MAX

#endif	//	DEFINE_TYPES_H__
