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
#include "debug_assert.h"

#if ASSERT_ENABLE

#include <sstream>
#include <iostream>

uint32 GameAssertFailed(const char* condition, const char* file, int32 line, const char* format, ...)
{
    bool triggerDebugBreak = true;

	LOG("ASSERT %s at %d \n", file, line );

	debug_printf("ASSERT(%s) FAILED: %s:%d", condition, file, line);
    if ( format != NULL)
    {
        va_list args;
        va_start(args, format);

        debug_vprintf(format, args);

        va_end(args);
    }

    return triggerDebugBreak? 0 : 1;
}

uint32 ToolAssertFailed(const char* condition, const char* file, int32 line, const char* format, ...)
{
    bool triggerDebugBreak = true;

    LOG("ASSERT %s at %d \n", file, line );

    debug_printf("ASSERT(%s) FAILED: %s:%d", condition, file, line);

#if IS_OS_WINDOWS

    char buffer[2048] = {0};

    std::stringstream str;

    str << "The following assertion failed!\n\n"; 

    if ( condition )
    {
        str << "Error:\n     \"" << condition << "\"\n\n";
    }

    if ( format )
    {
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        const char* msg = buffer;
        str << "Error Message:\n      " << msg << "\n\n";
    }

    if ( file )
    {
        str << "File:\n     " << file << "\n\nLine:\n     " << line << "\n\n";
    }

    std::cerr << "==============================================\n"
        << "Assert failed!\n\n"
        << str.str() << "\n"
        << "==============================================\n" << std::endl;

    str << "Press Abort to exit, Retry to debug, Ignore to ignore this assert" << std::ends; 

    const char* msgCaption = "Assert Failed!";
    const int msgBoxResult = MessageBox( NULL, str.str().c_str(), msgCaption, MB_ABORTRETRYIGNORE | MB_ICONSTOP );

    bool exitProgram = false;
    switch(msgBoxResult)
    {
    case 0:
        exitProgram = true;
        break;

    case	IDRETRY:
        triggerDebugBreak = true;
        break;

    case	IDABORT:
        exitProgram = true;
        break;

    case	IDIGNORE:
        triggerDebugBreak = false;
        break;

    default:
        break;
    }

    if ( exitProgram )
    {
        exit(EXIT_FAILURE);
    }
#else   //!  IS_OS_WINDOWS
    if ( format != NULL)
    {
        va_list args;
        va_start(args, format);

        debug_vprintf(format, args);

        va_end(args);
    }
#endif  //!  IS_OS_WINDOWS

    return triggerDebugBreak? 0 : 1;
}

#endif  //  ASSERT_ENABLE
