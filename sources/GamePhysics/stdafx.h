
#ifndef __STDAFX_H__
#define __STDAFX_H__

#if defined(__OBJC__)

#import <Cocoa/Cocoa.h>

#else

//IMPORTANT: list of warnings disabled in the project file (for Visual Studio: *.vcxproj)
//Note that this list might not be in sync or depends on whether its a debug build or not,
//so check the project build properties to be sure
//warning 4061: enumerator in switch of enum is not explicitly handled by a case label
//warning 4201: nonstandard extension used : nameless struct/union
//warning 4365: '=' (or 'return', or 'initializing', or 'argument') : conversion from '' to '', signed/unsigned mismatch
//warning 4514: unreferenced inline function has been removed
//warning 4640: construction of local static object is not thread-safe
//warning 4710: function not inlined
//warning 4820: bytes padding added after data member

#include "../define_platform.h"

#ifndef NACL

#if IS_OS_LINUX

#include <string.h>
#include <stddef.h>

#elif IS_OS_MACOSX

#include <strings.h>

#elif IS_OS_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     1   // Exclude rarely-used stuff from Windows headers
#endif

#pragma warning( push, 3 )
#include <windows.h>

#include <process.h>
#include <direct.h>
#pragma warning(disable:4917) // Disabling warning 4917 triggered in microsoft sdks\windows\v7.0a\include\ocidl.h(3112): a GUID can only be associated with a class, interface or namespace
#include <shlobj.h>
#pragma warning( pop )

#endif  //  defined(WIN32)

#endif  //  !defined(NACL)

#pragma warning( push, 3 )
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#pragma warning(disable:4530) //Disabling warning 4530 triggered in VC\include\xlocale(323): C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#include <list>
#include <vector>
#include <map>
#include <string>
#pragma warning( pop )

#include "../define_macros.h"
#include "../define_types.h"
#include "../debug_common.h"
#include "../debug_assert.h"
#include "../debug_breakpoint.h"

//#include "core_memory.h"
//#include "core_smartpointer.h"

#include "../maths.h"
#include "../toolbox.h"

#endif  // !__OBJC__

#endif  // __STDAFX_H__
