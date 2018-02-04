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
#include "debug_breakpoint.h"

#if IS_OS_WINDOWS

#define HW_BREAKPOINT_SUSPEND_ENABLE    0

bool HwBreakpoint::Set(void* _pAddress, int _WatchSizeInBytes, int _BreakCondition)
{
    if ( m_Status != E_Break_Not_Set )
    {
        Clear();
    }

    if ( _pAddress == NULL
        || HwBreakpoint::IsValidWatchSize(_WatchSizeInBytes) == false
        || HwBreakpoint::IsValidBreakCondition(_BreakCondition) == false )
    {
        return false;
    }

    const HANDLE currentThread = GetCurrentThread();
    if ( currentThread == NULL )
    {
        DWORD lastError =  GetLastError();
        wprintf(L"Format message failed with 0x%x\n", lastError );

        return false;
    }

#if HW_BREAKPOINT_SUSPEND_ENABLE
    const DWORD suspendResult = SuspendThread( currentThread );
    if ( suspendResult == (DWORD)-1 )
    {
        DWORD lastError =  GetLastError();
        wprintf(L"Format message failed with 0x%x\n", lastError );

        return false;
    }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

    CONTEXT context = {0};
    context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    // Read the debug registers value
    const DWORD getContextResult = GetThreadContext( currentThread, &context );
    if ( getContextResult == 0)
    {
        DWORD lastError =  GetLastError();
        wprintf(L"GetThreadContext() failed with 0x%x\n", lastError );

#if HW_BREAKPOINT_SUSPEND_ENABLE
        const DWORD resumeResult = ResumeThread( currentThread );
        if ( resumeResult == (DWORD)-1 )
        {
            lastError =  GetLastError();
            wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
        }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

        return false;
    }

    COMPILE_TIME_ASSERT( E_Break_Register_Count == 4 );

    // Checking DR7 bits (1,3,5,7) for available global tasks level debug register
    DWORD registerIndex;
#if IS_PLATFORM_32BIT
    DWORD bitOne = 1;
#elif IS_PLATFORM_64BIT
    DWORD64 bitOne = 1;
#endif
    for (registerIndex = 0; registerIndex < E_Break_Register_Count; ++registerIndex)
    {
        if ((context.Dr7 & (bitOne << (registerIndex*2))) == 0)
        {
            break;
        }
    }

    if ( registerIndex == E_Break_Register_Count )
    {
#if HW_BREAKPOINT_SUSPEND_ENABLE
        const DWORD resumeResult = ResumeThread( currentThread );
        if ( resumeResult == (DWORD)-1 )
        {
            DWORD lastError =  GetLastError();
            wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
        }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

        return false;
    }

    E_Break_Status tmpBreakStatus = E_Break_Not_Set;
    switch (registerIndex)
    {
    case 0:
        context.Dr0 = (DWORD_PTR) _pAddress;
        tmpBreakStatus = E_Break_Register_0;
        break;
    case 1:
        context.Dr1 = (DWORD_PTR) _pAddress;
        tmpBreakStatus = E_Break_Register_1;
        break;
    case 2:
        context.Dr2 = (DWORD_PTR) _pAddress;
        tmpBreakStatus = E_Break_Register_2;
        break;
    case 3:
        context.Dr3 = (DWORD_PTR) _pAddress;
        tmpBreakStatus = E_Break_Register_3;
        break;
    default:
        ASSERT_GAME( false );
    }

    int watchSizeCode = E_Break_Watch_None;
    switch ( _WatchSizeInBytes )
    {
    case 1:
        watchSizeCode = E_Break_Watch_One_Byte;
        break;
    case 2:
        watchSizeCode = E_Break_Watch_Two_Bytes;
        break;
    case 4:
        watchSizeCode = E_Break_Watch_Four_Bytes;
        break;
    case 8:
        watchSizeCode = E_Break_Watch_Eight_Bytes;
        break;
    default:
        ASSERT_GAME( false );
    }

    if ( watchSizeCode == E_Break_Watch_None )
    {
#if HW_BREAKPOINT_SUSPEND_ENABLE
        const DWORD resumeResult = ResumeThread( currentThread );
        if ( resumeResult == (DWORD)-1 )
        {
            DWORD lastError =  GetLastError();
            wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
        }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

        return false;
    }

    SetBits( context.Dr7, 16 + registerIndex*4, 2, _BreakCondition );
    SetBits( context.Dr7, 18 + registerIndex*4, 2, watchSizeCode );
    SetBits( context.Dr7, registerIndex*2, 1, 1 );

    const DWORD setContextResult = SetThreadContext( currentThread, &context );
    if ( setContextResult == 0)
    {
        DWORD lastError =  GetLastError();
        wprintf(L"SetThreadContext() failed with 0x%x\n", lastError );

#if HW_BREAKPOINT_SUSPEND_ENABLE
        const DWORD resumeResult = ResumeThread( currentThread );
        if ( resumeResult == (DWORD)-1 )
        {
            lastError =  GetLastError();
            wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
        }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

        return false;
    }

    m_Status = tmpBreakStatus;

#if HW_BREAKPOINT_SUSPEND_ENABLE
    const DWORD resumeResult = ResumeThread( currentThread );
    if ( resumeResult == (DWORD)-1 )
    {
        lastError =  GetLastError();
        wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
    }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

    return true;
}


bool HwBreakpoint::Clear()
{
    if ( m_Status != E_Break_Not_Set)
    {
        const HANDLE currentThread = GetCurrentThread();
        if ( currentThread == NULL )
        {
            DWORD lastError =  GetLastError();
            wprintf(L"Format message failed with 0x%x\n", lastError );

            return false;
        }

#if HW_BREAKPOINT_SUSPEND_ENABLE
        const DWORD suspendResult = SuspendThread( currentThread );
        if ( suspendResult == (DWORD)-1 )
        {
            DWORD lastError =  GetLastError();
            wprintf(L"Format message failed with 0x%x\n", lastError );

            return false;
        }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

        CONTEXT context = {0};
        context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

        // Read the debug registers value
        const DWORD getContextResult = GetThreadContext( currentThread, &context );
        if ( getContextResult == 0)
        {
            DWORD lastError =  GetLastError();
            wprintf(L"GetThreadContext() failed with 0x%x\n", lastError );

#if HW_BREAKPOINT_SUSPEND_ENABLE
            const DWORD resumeResult = ResumeThread( currentThread );
            if ( resumeResult == (DWORD)-1 )
            {
                lastError =  GetLastError();
                wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
            }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

            return false;
        }

        DWORD registerIndex = m_Status;
        ASSERT_GAME( registerIndex < E_Break_Register_Count );

        SetBits( context.Dr7, registerIndex*2, 1, 0 );

        const DWORD setContextResult = SetThreadContext( currentThread, &context );
        if ( setContextResult == 0)
        {
            DWORD lastError =  GetLastError();
            wprintf(L"SetThreadContext() failed with 0x%x\n", lastError );

#if HW_BREAKPOINT_SUSPEND_ENABLE
            const DWORD resumeResult = ResumeThread( currentThread );
            if ( resumeResult == (DWORD)-1 )
            {
                lastError =  GetLastError();
                wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
            }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

            return false;
        }

        m_Status = E_Break_Not_Set;

#if HW_BREAKPOINT_SUSPEND_ENABLE
        const DWORD resumeResult = ResumeThread( currentThread );
        if ( resumeResult == (DWORD)-1 )
        {
            lastError =  GetLastError();
            wprintf(L"ResumeThread() failed with 0x%x\n", lastError );
        }
#endif  //  HW_BREAKPOINT_SUSPEND_ENABLE

    }

    return true;
}

bool HwBreakpoint::IsValidWatchSize( int _WatchSizeInBytes )
{
    const int watchSizeInBytes = _WatchSizeInBytes;
    ASSERT_GAME( watchSizeInBytes != E_Break_Watch_None );

    return ( watchSizeInBytes == E_Break_Watch_One_Byte
        || watchSizeInBytes == E_Break_Watch_Two_Bytes
        || watchSizeInBytes == E_Break_Watch_Four_Bytes
        || watchSizeInBytes == E_Break_Watch_Eight_Bytes );
}

bool HwBreakpoint::IsValidBreakCondition( int _BreakCondition )
{
    const int breakCondition = _BreakCondition;
    ASSERT_GAME( breakCondition != E_Break_On_None );

    return ( breakCondition == E_Break_On_Execute
        || breakCondition == E_Break_On_Write
 //       || breakCondition == E_Break_On_IO_Read_Write // Unsupported by hardware apparently
        || breakCondition == E_Break_On_ReadWrite );
}

HwBreakpoint g_HwBrkPts[HwBreakpoint::E_Break_Register_Count];
void* g_HwBrkPtAddresses[HwBreakpoint::E_Break_Register_Count] = {NULL};
int g_HwBrkPtWatchSizes[HwBreakpoint::E_Break_Register_Count] = {HwBreakpoint::E_Break_Watch_None};
int g_HwBrkPtConditions[HwBreakpoint::E_Break_Register_Count] = {HwBreakpoint::E_Break_On_None};
bool g_bEnableHwBrkPts = false;

bool debug_initHwBrkPts( int _RegisterIdx, void* _pAddress, HwBreakpoint::E_Break_Watch_Size _WatchSizeInBytes, HwBreakpoint::E_Break_On_Condition _BreakCondition )
{
    const int watchSizeInBytes = _WatchSizeInBytes;
    const int breakCondition = _BreakCondition;

    ASSERT_GAME( _RegisterIdx != HwBreakpoint::E_Break_Not_Set );
    
    if ( _pAddress == NULL
        || (_RegisterIdx < 0)
        || (_RegisterIdx >= HwBreakpoint::E_Break_Register_Count)
        || HwBreakpoint::IsValidWatchSize(watchSizeInBytes) == false
        || HwBreakpoint::IsValidBreakCondition(breakCondition) == false )
    {
        return false;
    }
    
    
    const bool bHwBrkPtClear = g_HwBrkPts[_RegisterIdx].Clear();
    ASSERT_GAME( bHwBrkPtClear );
    UNUSED_PARAMETER( bHwBrkPtClear );  //HACK: needed when ASSERT_GAME() is void, to prevent warning 4189 : local variable is initialized but not referenced

    g_HwBrkPtAddresses[_RegisterIdx] = _pAddress;
    g_HwBrkPtWatchSizes[_RegisterIdx] = watchSizeInBytes;
    g_HwBrkPtConditions[_RegisterIdx] = breakCondition;

    return true;
}

void hack_EnableHwBrkPts()
{
    g_bEnableHwBrkPts = true;
}

void hack_DisableHwBrkPts()
{
    g_bEnableHwBrkPts = false;
}

void hack_setHwBrkPts()
{
    if ( g_bEnableHwBrkPts )
    {
        for ( int i = 0; i < HwBreakpoint::E_Break_Register_Count; ++i )
        {
            if ( g_HwBrkPtAddresses[i] != NULL && g_HwBrkPtWatchSizes[i] > 0 )
            {
                const bool bHwBrkPtSet = g_HwBrkPts[i].Set( g_HwBrkPtAddresses[i], g_HwBrkPtWatchSizes[i], g_HwBrkPtConditions[i] );
                ASSERT_GAME( bHwBrkPtSet );
                UNUSED_PARAMETER( bHwBrkPtSet );  //HACK: needed when ASSERT_GAME() is void, to prevent warning 4189 : local variable is initialized but not referenced
            }
        }
    }
}

void hack_clearHwBrkPts()
{
    if (g_bEnableHwBrkPts)
    {
        for ( int i = 0; i < HwBreakpoint::E_Break_Register_Count; ++i )
        {
            if ( g_HwBrkPtAddresses[i] != NULL )
            {
                const bool bHwBrkPtClear = g_HwBrkPts[i].Clear();
                ASSERT_GAME( bHwBrkPtClear );
                UNUSED_PARAMETER( bHwBrkPtClear );  //HACK: needed when ASSERT_GAME() is void, to prevent warning 4189 : local variable is initialized but not referenced
            }
        }
    }
}

#if 0
void hack_checkMeshStack()
{
    const mesh_t*pm = GMeshes[GRenderStackIndex].first;
    const mesh_t* prev_pm = NULL;
    int ParsedMeshCount = 0;
    while(pm)
    {
        ASSERT_GAME( pm->mStackIndex == GRenderStackIndex );
        const int IA_element_size = pm->mIA->GetElementSize();
        ASSERT_GAME( IA_element_size == 2 || IA_element_size == 4 );
        ++ParsedMeshCount;
        prev_pm = pm;
        pm = pm->mNext;
    }
}
#endif  //  0

#endif  //  IS_OS_WINDOWS
