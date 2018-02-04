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

#ifndef DEBUG_ASSERT_H__
#define DEBUG_ASSERT_H__


#if !defined(ASSERT_ENABLE)
#   error "define_macros.h and debug_common.h should be included first."
#endif


#if ASSERT_ENABLE

#define COMPILE_TIME_ASSERT(cond)   \
    DECLARE_MACRO_BEGIN     \
    typedef char CompileTimeAssert[(cond) ? 1 : -1]; \
    DECLARE_MACRO_END

// returns 0 to trigger DEBUG_BREAK, 1 otherwise
uint32 GameAssertFailed(const char* condition, const char* file, int32 line, const char* format, ...);

#define ASSERT_GAME(cond)					(void)((cond) || GameAssertFailed(#cond, __FILE__, __LINE__, NULL) || (DEBUG_BREAK(), 1))

#define ASSERT_GAME_MSG(cond, fmt, ...)		(void)((cond) || GameAssertFailed(#cond, __FILE__, __LINE__, fmt, __VA_ARGS__) || (DEBUG_BREAK(), 1))

// returns 0 to trigger DEBUG_BREAK, 1 otherwise
uint32 ToolAssertFailed(const char* condition, const char* file, int32 line, const char* format, ...);

#define ASSERT_TOOL(cond)					(void)((cond) || ToolAssertFailed(#cond, __FILE__, __LINE__, NULL) || (DEBUG_BREAK(), 1))

#define ASSERT_TOOL_MSG(cond, fmt, ...)		(void)((cond) || ToolAssertFailed(#cond, __FILE__, __LINE__, fmt, __VA_ARGS__) || (DEBUG_BREAK(), 1))

#else

#define COMPILE_TIME_ASSERT(cond)           (void)sizeof(cond)

#define ASSERT_GAME(cond)					(void)sizeof(cond)

#define ASSERT_GAME_MSG(cond, fmt, ...)		\
    DECLARE_MACRO_BEGIN \
        (void)sizeof(cond); \
        (void)sizeof(fmt); \
    DECLARE_MACRO_END

#define ASSERT_TOOL(cond)					(void)sizeof(cond)

#define ASSERT_TOOL_MSG(cond, fmt, ...)		\
    DECLARE_MACRO_BEGIN \
    (void)sizeof(cond); \
    (void)sizeof(fmt); \
    DECLARE_MACRO_END

#endif	//!	ASSERT_ENABLE


#endif	//	DEBUG_ASSERT_H__
