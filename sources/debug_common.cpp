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

#include "stdafx.h"
#include "define_platform.h"
#include "debug_common.h"

#if DEBUG_ENABLE

void debug_printf(const char * format, ...)
{
    va_list args;
    va_start(args, format);

    debug_vprintf(format, args);

    va_end(args);
}

void debug_vprintf(const char* format, va_list args)
{
#if IS_OS_WINDOWS
    char buffer[2048] = {0};
    vsnprintf(buffer, sizeof(buffer), format, args);

    OutputDebugString(buffer);
    OutputDebugString("\n");
#else
    FILE* stream = stdout;
    vfprintf(stream, format, args);
#endif
}

#endif  //  DEBUG_ENABLE

bool debug_ContainsNaN( const vec_t& v )
{
    return ( IS_NAN(v.x) || IS_NAN(v.y) || IS_NAN(v.z) || IS_NAN(v.w) );
}

bool debug_ContainsNaN( const vec3& v )
{
    return ( IS_NAN(v.x) || IS_NAN(v.y) || IS_NAN(v.z) );
}

bool debug_ContainsNaN (const matrix_t& m )
{
    return ( debug_ContainsNaN(m.right) || debug_ContainsNaN(m.up) || debug_ContainsNaN(m.dir) || debug_ContainsNaN(m.position) );
}
