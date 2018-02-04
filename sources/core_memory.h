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

#ifndef CORE_MEMORY_H__
#define CORE_MEMORY_H__


#if !defined(MEMORY_DEBUG_ENABLE)
#   error "define_macros.h should be included first."
#endif


#if !MEMORY_DEBUG_ENABLE

#define NEW_MEM new
#define NEW_MEM_FROM(MemoryAllocator) new
#define DELETE_MEM(MemoryPtr) delete MemoryPtr
#define DELETE_MEM_ARRAY(MemoryPtr) delete[] MemoryPtr

#else   //  MEMORY_DEBUG_ENABLE

#define NEW_MEM new ( MemAllocInterface::E_Memory_Allocator_Default, __FILE__, __LINE__, __PRETTY_FUNCTION__ )
#define NEW_MEM_FROM(MemoryAllocator) new ( MemoryAllocator, __FILE__, __LINE__, __PRETTY_FUNCTION__ )
#define DELETE_MEM(MemoryPtr) MemAllocInterface::Delete( MemoryPtr, __FILE__, __LINE__, __PRETTY_FUNCTION__ )
#define DELETE_MEM_ARRAY(MemoryPtr) MemAllocInterface::DeleteArray( MemoryPtr, __FILE__, __LINE__, __PRETTY_FUNCTION__ )

class MemAllocInterface
{
public:
    enum E_Memory_Allocator
    {
        E_Memory_Allocator_Invalid = -1,
        E_Memory_Allocator_Default = 0,
    };

    template<typename T>
    static void	Delete( T *memory, const char *file, u32 line, const char* fonction )
    {
        UNUSED_PARAMETER(file);
        UNUSED_PARAMETER(line);
        UNUSED_PARAMETER(fonction);

        if (memory != NULL)
        {
            //debug_printf("DELETE: %p\n%s at line %d in function %s\n", static_cast<void*>(memory), file, line, fonction);

            delete memory;
        }
    }

    template<typename T>
    static void	DeleteArray( T *memory, const char *file, u32 line, const char* fonction )
    {
        UNUSED_PARAMETER(file);
        UNUSED_PARAMETER(line);
        UNUSED_PARAMETER(fonction);

        //debug_printf("DELETE[]: %p\n%s at line %d in function %s\n", static_cast<void*>(memory), file, line, fonction);

        delete[] memory;
    }
};

#if 0
//Delete template specialization example
template<>
static void	MemAllocInterface::Delete<YourObjectClass>( YourObjectClass *memory, const char *file, u32 line, const char* fonction );
#endif

inline void* operator new(size_t size, MemAllocInterface::E_Memory_Allocator memalloc, const char *file, u32 line, const char* fonction)
{
    UNUSED_PARAMETER(memalloc);
    UNUSED_PARAMETER(file);
    UNUSED_PARAMETER(line);
    UNUSED_PARAMETER(fonction);

    void *memory = malloc(size);
    //debug_printf("NEW: %p (size: %d)\n%s at line %d in function %s\n", memory, size, file, line, fonction);

    return memory;
}

inline void* operator new[](size_t size, MemAllocInterface::E_Memory_Allocator memalloc, const char *file, u32 line, const char* fonction)
{
    UNUSED_PARAMETER(memalloc);
    UNUSED_PARAMETER(file);
    UNUSED_PARAMETER(line);
    UNUSED_PARAMETER(fonction);

    void *memory = malloc(size);
    //debug_printf("NEW[]: %p (size: %d)\n%s at line %d in function %s\n", memory, size, file, line, fonction);

    return memory;
}

//NOTE: the matching 'delete' and 'delete[]' operators can't really be used but are required:
//warning C4291: no matching operator delete found; memory will not be freed if initialization throws an exception

inline void operator delete(void *memory, MemAllocInterface::E_Memory_Allocator memalloc, const char *file, u32 line, const char* fonction)
{
    UNUSED_PARAMETER(memalloc);
    UNUSED_PARAMETER(file);
    UNUSED_PARAMETER(line);
    UNUSED_PARAMETER(fonction);

    ASSERT_GAME(0);

    free(memory);
}

inline void operator delete[](void* memory, MemAllocInterface::E_Memory_Allocator memalloc, const char *file, u32 line, const char* fonction)
{
    UNUSED_PARAMETER(memalloc);
    UNUSED_PARAMETER(file);
    UNUSED_PARAMETER(line);
    UNUSED_PARAMETER(fonction);

    ASSERT_GAME(0);

    free(memory);
}

#endif //   MEMORY_DEBUG_ENABLE

#endif  //  CORE_MEMORY_H__
