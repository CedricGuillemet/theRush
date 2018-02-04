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
#include "core_memory.h"

#if 0
//Delete template specialization example
template<>
void MemAllocInterface::Delete<YourObjectClass>( YourObjectClass *memory, const char *file, u32 line, const char* fonction )
{
	UNUSED_PARAMETER(file);
	UNUSED_PARAMETER(line);
	UNUSED_PARAMETER(fonction);

	if (memory != NULL)
	{
		debug_printf("DELETE: %p\n%s at line %d in function %s\n", static_cast<void*>(memory), file, line, fonction);

		delete memory;
	}
}
#endif
