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

#ifndef DEBUG_COMMON_H__
#define DEBUG_COMMON_H__


#if !defined(DEBUG_ENABLE)
#   error "define_macros.h should be included first."
#endif


#if DEBUG_ENABLE

#if IS_OS_WINDOWS
//#   define DEBUG_BREAK()	DebugBreak()    //NOTE: was breaking at a lower level in callstack
#   define DEBUG_BREAK()	__debugbreak()
#elif IS_OS_LINUX
//#   define DEBUG_BREAK()    asm("int3\n")
#define DEBUG_BREAK() __builtin_trap()
#elif IS_OS_MACOSX
#   define DEBUG_BREAK()    __builtin_trap()
#else
#   define DEBUG_BREAK()    abort()
#endif

void debug_printf(const char* format, ...);
void debug_vprintf(const char* format, va_list args);

#else   //! DEBUG_ENABLE
#   define DEBUG_BREAK()

#define debug_printf(fmt, ...)		\
    DECLARE_MACRO_BEGIN \
    (void)sizeof(fmt); \
    DECLARE_MACRO_END

#define debug_vprintf(fmt, args)	\
    DECLARE_MACRO_BEGIN \
    (void)sizeof(fmt); \
    (void)sizeof(args); \
    DECLARE_MACRO_END

#endif  //! DEBUG_ENABLE

struct vec_t;
struct vec3;
struct matrix_t;
bool debug_ContainsNaN( const vec_t& v );
bool debug_ContainsNaN( const vec3& v );
bool debug_ContainsNaN (const matrix_t& m );

template<typename T>
int debug_GetElementIndexInArray( const T* element, const T elements[] )
{
    return ( (uintptr_t)(element) - (uintptr_t)(&elements[0]) ) / sizeof(T);
}

inline bool debug_IsIndexInRange( int index, int min, int max )
{
    return (min <= index) && (index <= max);
}

#endif	//	DEBUG_COMMON_H__
