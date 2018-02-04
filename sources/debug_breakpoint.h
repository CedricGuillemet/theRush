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

#ifndef DEBUG_BREAKPOINT_H__
#define DEBUG_BREAKPOINT_H__


#if !defined(DEBUG_ENABLE)
#   error "define_macros.h and debug_common.h should be included first."
#endif

#if IS_OS_WINDOWS

//This hardware breakpoint implementation relies on: http://en.wikipedia.org/wiki/X86_debug_register
//Implementation based on code from Mike Morearty: http://www.morearty.com/code/breakpoint
//And some code from Michael Chourdakis: http://www.codeproject.com/Articles/28071/Toggle-hardware-data-read-execute-breakpoints-prog

//Supports up to 4 hardware breakpoints per thread

class HwBreakpoint
{
public:
    HwBreakpoint() : m_Status(E_Break_Not_Set) {}
    ~HwBreakpoint() { Clear(); }

    // 2-bit values used by x86 and x64 debug registers to specify the break condition
    enum E_Break_On_Condition
    {
        E_Break_On_None = -1,
        E_Break_On_Execute = 0,
        E_Break_On_Write = 1,
//      E_Break_On_IO_Read_Write = 2,   // Unsupported by hardware apparently
        E_Break_On_ReadWrite = 3
    };

    // 2-bit values used by x86 and x64 debug registers to specify the break watch size
    enum E_Break_Watch_Size
    {
        E_Break_Watch_None = -1,
        E_Break_Watch_One_Byte = 0,
        E_Break_Watch_Two_Bytes = 1,
        E_Break_Watch_Four_Bytes = 3,
        E_Break_Watch_Eight_Bytes = 2
    };

    enum E_Break_Status
    {
        E_Break_Not_Set = -1,
        E_Break_Register_0 = 0,
        E_Break_Register_1 = 1,
        E_Break_Register_2 = 2,
        E_Break_Register_3 = 3,
    };

    enum { E_Break_Register_Count = 4 };

    bool Set(void* _pAddress, int _WatchSizeInBytes, int _BreakCondition);
    bool Clear();

    static bool IsValidWatchSize( int _WatchSizeInBytes );
    static bool IsValidBreakCondition( int _BreakCondition );

protected:

    void SetBits(DWORD_PTR& dw, int lowBit, int bits, int newValue)
    {
        DWORD_PTR mask = (1 << bits) - 1; // e.g. 1 becomes 0001, 2 becomes 0011, 3 becomes 0111

        dw = (dw & ~(mask << lowBit)) | (newValue << lowBit);
    }

    E_Break_Status m_Status;
};

bool debug_initHwBrkPts( int _RegisterIdx, void* _pAddress, HwBreakpoint::E_Break_Watch_Size _WatchSizeInBytes, HwBreakpoint::E_Break_On_Condition _BreakCondition );
void hack_clearHwBrkPts();
void hack_setHwBrkPts();
void hack_EnableHwBrkPts();
void hack_DisableHwBrkPts();

#endif  //  IS_OS_WINDOWS

#endif	//	DEBUG_BREAKPOINT_H__
