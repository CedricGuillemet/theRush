cmake_minimum_required(VERSION 2.6)

SET( CMAKE_VERBOSE_MAKEFILE on )

set( config_vars
    Debug
    RelWithDebInfo
    Release
)

set( CMAKE_CONFIGURATION_TYPES "${config_vars}" CACHE STRING "Specify build configurations" )
STRING( TOUPPER "${config_vars}" config_vars )

include(CMakeDependentOption)
include(CheckTypeSize)

project(R2)
#message( "Configuring R2 Solution" )

# Only instancing project(R2) at the end, so that whole solution name is 'R2'
# This also sets the variables R2_BINARY_DIR/PROJECT_BINARY_DIR and R2_SOURCE_DIR/PROJECT_SOURCE_DIR to the respective values.

check_type_size("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)
if (SIZEOF_VOID_P EQUAL 8)
    message( "64 bit platform" )
    set( IS_PLATFORM_64BIT TRUE )
    set( IS_PLATFORM_32BIT FALSE )
elseif (SIZEOF_VOID_P EQUAL 4)
    message( "32 bit platform" )
    set( IS_PLATFORM_64BIT FALSE )
    set( IS_PLATFORM_32BIT TRUE )
endif ()

if (WIN32)
    message( "Windows OS" )
    set( IS_OS_WINDOWS TRUE )
    set( IS_OS_MACOSX FALSE )
    set( IS_OS_LINUX FALSE )
elseif (APPLE)
    message( "Mac OSX" )
    set( IS_OS_WINDOWS FALSE )
    set( IS_OS_MACOSX TRUE )
    set( IS_OS_LINUX FALSE )
elseif (UNIX)
    message( "Linux" )
    set( IS_OS_WINDOWS FALSE )
    set( IS_OS_MACOSX FALSE )
    set( IS_OS_LINUX TRUE )
endif ()

#HACK
if (IS_OS_MACOSX)
    if (CMAKE_OSX_ARCHITECTURES STREQUAL "$(ARCHS_STANDARD_32_BIT)" OR CMAKE_OSX_ARCHITECTURES STREQUAL "i386" )
        message( "Forcing 32bit platform when targetting architecture: ${CMAKE_OSX_ARCHITECTURES}" )
        set( IS_PLATFORM_64BIT FALSE )
        set( IS_PLATFORM_32BIT TRUE )
    endif ()
    if (NOT CMAKE_OSX_DEPLOYEMENT_TARGET)
        set( CMAKE_OSX_DEPLOYEMENT_TARGET "${_CURRENT_OSX_VERSION}" )
    endif ()
    message( "Mac OSX deployement target: ${CMAKE_OSX_DEPLOYEMENT_TARGET}")

    set( CMAKE_OSX_ARCHITECTURES "i386" CACHE STRING ".theRush// is currently only supported on 32 bit platforms." )

elseif (IS_OS_LINUX)
    if (IS_PLATFORM_64BIT)
        message( "64bit platform isn't currently supported." )
        message( "Use a custom Cmake Toolchain File with:" )
        message( "set(CMAKE_C_COMPILER gcc -m32)" )
        message( "set(CMAKE_CXX_COMPILER g++ -m32)" )
    endif ()
    if (NOT CMAKE_BUILD_TYPE)
        set( CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Specify build type" FORCE )
    endif ()
endif ()

if (MSVC)
    set( IS_IDE_NONE FALSE )
    set( IS_IDE_MSVC TRUE )
    set( IS_IDE_XCODE FALSE )
    set( IS_IDE_CODEBLOCKS FALSE )
elseif (XCODE_VERSION)
    set( IS_IDE_NONE FALSE )
    set( IS_IDE_MSVC FALSE )
    set( IS_IDE_XCODE TRUE )
    set( IS_IDE_CODEBLOCKS FALSE )
elseif (CMAKE_EXTRA_GENERATOR STREQUAL "CodeBlocks")
    set( IS_IDE_NONE FALSE )
    set( IS_IDE_MSVC FALSE )
    set( IS_IDE_XCODE FALSE )
    set( IS_IDE_CODEBLOCKS TRUE )
else ()
    set( IS_IDE_NONE TRUE )
    set( IS_IDE_MSVC FALSE )
    set( IS_IDE_XCODE FALSE )
    set( IS_IDE_CODEBLOCKS FALSE )
endif ()

option(USE_COMPILER_RTTI "Enable Run Time Type Information (RTTI)" FALSE)

if (USE_COMPILER_RTTI)
    #RTTI is usually enable by default
else ()
    if (MSVC)
        add_definitions(/GR-)
    elseif (XCODE_VERSION)
        #for each target
        #set_target_properties( thetarget PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI "NO")
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        add_definitions(-fno-rtti)
    else ()
        #TODO: add other compiler specific definition to disable RTTI
    endif ()
endif ()

option(USE_COMPILER_EXCEPTION_HANDLING "Enable exception handling" FALSE)

if (USE_COMPILER_EXCEPTION_HANDLING)
    #HACK to check from code if exception is enable or not
    #this was needed to help compile tinythread 3rdParty code on Linux with g++
    add_definitions("CPP_EXCEPTION_ENABLE=1")
    #Exception handling is usually enable by default
else ()
    if (MSVC)
        string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    elseif (XCODE_VERSION)
        #for each target
        #set_target_properties( thetarget PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        add_definitions(-fno-exceptions)
    else ()
        #TODO: add other compiler specific definition to disable exception handling
    endif ()
endif ()

#Link Time Optimization
if (MSVC)
    set( LINK_TIME_OPTIM_COMPILE_FLAG "/GL" )
    set( LINK_TIME_OPTIM_LINK_FLAG "/LTCG" )
    set( LINK_TIME_OPTIM_LIBRARY_LINK_FLAG "/LTCG" )
    cmake_dependent_option(USE_LINK_TIME_OPTIMIZATION "Enable Whole Program Optimization (${LINK_TIME_OPTIM_COMPILE_FLAG}) and Link Time Code Generation (${LINK_TIME_OPTIM_LINK_FLAG}) in RELWITHDEBINFO and RELEASE builds" TRUE "MSVC" FALSE)
elseif (XCODE_VERSION)
    cmake_dependent_option(USE_LINK_TIME_OPTIMIZATION "Enable Link Time Optimization" TRUE "XCODE_VERSION" FALSE)
    #for each target
    #set_target_properties( thetarget PROPERTIES XCODE_ATTRIBUTE_LLVM_LTO "YES")
elseif (CMAKE_COMPILER_IS_GNUCXX)
    set( LINK_TIME_OPTIM_COMPILE_FLAG "-flto" )
    set( LINK_TIME_OPTIM_LINK_FLAG "-flto -fwhole-program" )
    set( LINK_TIME_OPTIM_LIBRARY_LINK_FLAG "" )
    cmake_dependent_option(USE_LINK_TIME_OPTIMIZATION "Enable Link Time Optimization and Whole Program Optimization (${LINK_TIME_OPTIM_LINK_FLAG}) in RELWITHDEBINFO and RELEASE builds" TRUE "CMAKE_COMPILER_IS_GNUCXX" FALSE)
else ()
    #TODO: Add similar options for other compilers
endif ()

set( compiler_C_CXX_flag_vars
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
)

if (MSVC)
    add_definitions(/nologo)

    #it will disable 'incremental linking' causing compilation failure when 'map file generation' is used
    #disabling 'incremental linking' will disable 'Edit and continue' option
    set( GENERATE_MAP_FILE_LINKER_FLAGS "/MAP /MAPINFO:EXPORTS /INCREMENTAL:NO" )
    cmake_dependent_option(GENERATE_MAP_FILE "Enable Linker map file generation (${GENERATE_MAP_FILE_LINKER_FLAGS})" TRUE "MSVC" FALSE)

    cmake_dependent_option(USE_MSVC_RUNTIME_LIBRARY_DLL "Dynamically link the MS CRT dlls (msvcrt) - uses /MD instead of /MT." FALSE "MSVC" FALSE)

    if (USE_MSVC_RUNTIME_LIBRARY_DLL)
        foreach (flag_var ${compiler_C_CXX_flag_vars})
            if (${flag_var} MATCHES "/MT")
                string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
            elseif (${flag_var} MATCHES "/MTd")
                string(REGEX REPLACE "/MTd" "/MDd" ${flag_var} "${${flag_var}}")
            endif ()
        endforeach ()
    else ()
        foreach (flag_var ${compiler_C_CXX_flag_vars})
            if (${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            elseif (${flag_var} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
            endif ()
        endforeach ()
    endif ()

    #back up C / CXX compiler flags
    foreach (flag_var ${compiler_C_CXX_flag_vars})
        set( ${flag_var}_DEFAULT ${${flag_var}} )
        #message( ${flag_var}: ${${flag_var}} )
    endforeach ()

    set( MP_COMPILATION_COMPILER_FLAGS "/MP" )
    cmake_dependent_option(USE_MP_COMPILATION "Enable multi-processor compilation (${MP_COMPILATION_COMPILER_FLAGS})" TRUE "MSVC" FALSE)
    if (USE_MP_COMPILATION)
        add_definitions( "${MP_COMPILATION_COMPILER_FLAGS}" )
    endif ()

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    set( WARNING_ALL_COMPILER_FLAGS "/Wall" )
    set( WARNING_LEVEL4_COMPILER_FLAGS "/W4" )
    set( WARNING_LEVEL3_COMPILER_FLAGS "/W3" )
    cmake_dependent_option(USE_COMPILER_WARNING_ALL "Enable all compiler warnings (${WARNING_ALL_COMPILER_FLAGS})" FALSE "MSVC" FALSE)
    cmake_dependent_option(USE_COMPILER_WARNING_LEVEL4 "Use compiler warning level 4 (${WARNING_LEVEL4_COMPILER_FLAGS}), default is level 3 (${WARNING_LEVEL3_COMPILER_FLAGS})." TRUE "MSVC;NOT USE_COMPILER_WARNING_ALL" FALSE)

    if (USE_COMPILER_WARNING_ALL)
        set( THERUSH_WARNING_COMPILER_FLAGS "${WARNING_ALL_COMPILER_FLAGS}" )
    elseif (USE_COMPILER_WARNING_LEVEL4)
        set( THERUSH_WARNING_COMPILER_FLAGS "${WARNING_LEVEL4_COMPILER_FLAGS}" )
    else ()
        set( THERUSH_WARNING_COMPILER_FLAGS "${WARNING_LEVEL3_COMPILER_FLAGS}" )
    endif ()

    set( THERUSH_DISABLED_WARNINGS_DETAIL
        "Disable several compiler warnings:
        4061 (enumerator in switch of enum is not explicitly handled by a case label)
        4201 (nonstandard extension used : nameless struct/union)
        4365 ('=' (or 'return', or 'initializing', or 'argument') : conversion from '' to '', signed/unsigned mismatch)
        4514 (unreferenced inline function has been removed)
        4640 (construction of local static object is not thread-safe)
        4710 (function not inlined)
        4820 (bytes padding added after data member)"
    )
    cmake_dependent_option(DISABLE_SPECIFIC_COMPILER_WARNINGS_THERUSH "${THERUSH_DISABLED_WARNINGS_DETAIL}" TRUE "USE_COMPILER_WARNING_ALL" FALSE)

    if (DISABLE_SPECIFIC_COMPILER_WARNINGS_THERUSH)
        set( THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS "/wd4061 /wd4201 /wd4365 /wd4514 /wd4640 /wd4710 /wd4820" )
    else ()
        #disabling by default warning 4201 (nonstandard extension used : nameless struct/union)
        set( THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS "/wd4201" )
    endif ()

    set( THERUSH_OPTIM_COMPILE_FLAGS_DEBUG "/Od /Ob0 /Oy-" )
    set( THERUSH_OPTIM_LINK_FLAGS_DEBUG "" )
    set( THERUSH_OPTIM_COMPILE_FLAGS_RELWITHDEBINFO "/Ox /Ob2 /Oi /Ot /Oy- /GF /Gy /arch:SSE" )
    set( THERUSH_OPTIM_LINK_FLAGS_RELWITHDEBINFO "/OPT:REF /OPT:ICF" )
    set( THERUSH_OPTIM_COMPILE_FLAGS_RELEASE "/Ox /Ob2 /Oi /Ot /Oy- /GF /Gy /arch:SSE /fp:except-" )
    set( THERUSH_OPTIM_LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF" )

    foreach (config_var ${config_vars})
        cmake_dependent_option(USE_CUSTOM_OPTIM_FLAGS_THERUSH_${config_var} "${config_var}: ${THERUSH_OPTIM_COMPILE_FLAGS_${config_var}} (compile) ${THERUSH_OPTIM_LINK_FLAGS_${config_var}} (link)" TRUE "MSVC" FALSE)
    endforeach ()

else ()
    #TODO: Add similar options for other compilers
endif ()

#Precompiled headers
if (MSVC)
    set( PCH_CREATE_COMPILER_FLAGS "/Yc" )
    set( PCH_USE_COMPILER_FLAGS "/Yu" )
    cmake_dependent_option(USE_PRECOMPILED_HEADERS "Enable precompiled headers (${PCH_CREATE_COMPILER_FLAGS}, ${PCH_USE_COMPILER_FLAGS})" TRUE "MSVC" FALSE)
elseif (XCODE_VERSION)
    message( "XCODE_VERSION: ${XCODE_VERSION}" )
    cmake_dependent_option(USE_PRECOMPILED_HEADERS "Enable precompiled headers" TRUE "XCODE_VERSION" FALSE)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    cmake_dependent_option(USE_PRECOMPILED_HEADERS "Enable precompiled headers" TRUE "CMAKE_COMPILER_IS_GNUCXX" FALSE)
else ()
    #TODO: Add similar options for other compilers
endif ()

if (USE_PRECOMPILED_HEADERS)
    set( PCH_HEADER_FILE stdafx.h )
    set( PCH_SOURCE_FILE stdafx.cpp )
endif ()

############ 3rdparty PROJECT

project(3rdparty)

#message( "Configuring 3rdparty Project" )

set( theRush_3rdparty_LIB_TYPE STATIC )

set( theRush_3rdparty_INCS
    3rdParty/fxaa3_8_minified.h
    3rdParty/GLee.h
    3rdParty/JSON_parser.h
    3rdParty/mongoose.h
    3rdParty/stb_image_write.h
    3rdParty/tinythread.h
)

set( theRush_3rdparty_SRCS
    3rdParty/GLee.cpp
    3rdParty/JSON_parser.c
    3rdParty/mongoose.c
    3rdParty/stb_vorbis.c
    3rdParty/tinythread.cpp
)

set( theRush_3rdparty_INCDIRS
    3rdParty
)

include_directories( ${theRush_3rdparty_INCDIRS} )
add_library( 3rdparty_lib ${theRush_3rdparty_LIB_TYPE} ${theRush_3rdparty_INCS} ${theRush_3rdparty_SRCS} )

if (NOT USE_COMPILER_RTTI)
    if (XCODE_VERSION)
        set_target_properties( 3rdparty_lib PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI "NO")
    endif ()
endif ()

if (NOT USE_COMPILER_EXCEPTION_HANDLING)
    if (XCODE_VERSION)
        set_target_properties( 3rdparty_lib PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
    endif ()
endif ()

if (USE_LINK_TIME_OPTIMIZATION)
    if (XCODE_VERSION)
        set_target_properties( 3rdparty_lib  PROPERTIES XCODE_ATTRIBUTE_LLVM_LTO "YES")
    else ()
        #set_property( TARGET 3rdparty_lib APPEND PROPERTY COMPILE_FLAGS_RELWITHDEBINFO ${LINK_TIME_OPTIM_COMPILE_FLAG} )
        #set_property( TARGET 3rdparty_lib APPEND PROPERTY COMPILE_FLAGS_RELEASE ${LINK_TIME_OPTIM_COMPILE_FLAG} )
        set_property( TARGET 3rdparty_lib APPEND PROPERTY STATIC_LIBRARY_FLAGS_RELWITHDEBINFO ${LINK_TIME_OPTIM_LIBRARY_LINK_FLAG} )
        set_property( TARGET 3rdparty_lib APPEND PROPERTY STATIC_LIBRARY_FLAGS_RELEASE ${LINK_TIME_OPTIM_LIBRARY_LINK_FLAG} )
    endif ()
endif ()

#if (MSVC)
#    foreach (config_var ${config_vars})
#        if (USE_CUSTOM_OPTIM_FLAGS_THERUSH_${config_var})
#            #adding custom optim linker flags to static library flags (needed at least for LTCG)
#            set_property( TARGET 3rdparty_lib APPEND PROPERTY STATIC_LIBRARY_FLAGS_${config_var} "${THERUSH_OPTIM_LINK_FLAGS_${config_var}}" )
#        endif ()
#    endforeach ()
#endif ()

############ theRush PROJECT

project(theRush)

#message( "Configuring theRush Project" )

if (USE_PRECOMPILED_HEADERS)
    set( theRush_PCH_INC sources/${PCH_HEADER_FILE} )
    set( theRush_PCH_SRC sources/${PCH_SOURCE_FILE} )
else ()
    #HACK
    set( theRush_sources_INCS sources/stdafx.h )
endif ()

set( theRush_bin_SRCS
    Bin/rushDatas.cpp
)
#Not compiling rushDatas.cpp directly, it is being included from include_rushDatas.cpp in RELEASE/RETAIL build
set_source_files_properties( ${theRush_bin_SRCS} PROPERTIES HEADER_FILE_ONLY true )

set( theRush_sources_INCS
    ${theRush_sources_INCS}
    sources/audio.h
    sources/bonus.h
    sources/camera.h
    sources/content.h
    sources/core_memory.h
    sources/core_smartpointer.h
    sources/debug_assert.h
    sources/debug_breakpoint.h
    sources/debug_common.h
    sources/define_macros.h
    sources/define_platform.h
    sources/define_types.h
    sources/fx.h
    sources/game.h
    sources/gui.h
    sources/include_Bullet.h
    sources/include_GL.h
    sources/include_libGizmo.h
    sources/include_OpenAL.h
    sources/include_RakNet.h
    sources/include_SDL.h
    sources/JSON_serializer.h
    sources/maths.h
    sources/menus.h
    sources/mesh.h
    sources/net.h
    sources/ocean.h
    sources/physics.h
    sources/render.h
    sources/solo.h
    sources/therush.h
    sources/toolbox.h
    sources/track.h
    sources/world.h
    sources/ZShipPhysics.h
)

set( theRush_sources_SRCS
    sources/audio.cpp
    sources/bonus.cpp
    sources/camera.cpp
    sources/content.cpp
    sources/core_memory.cpp
    sources/debug_assert.cpp
    sources/debug_breakpoint.cpp
    sources/debug_common.cpp
    sources/edit.cpp
    sources/fx.cpp
    sources/game.cpp
    sources/gui.cpp
    sources/include_rushDatas.cpp
    sources/JSON_serializer.cpp
    sources/maths.cpp
    sources/menus.cpp
    sources/mesh.cpp
    sources/net.cpp
    sources/ocean.cpp
    sources/physics.cpp
    sources/render.cpp
    sources/solo.cpp
    sources/testunit.cpp
    sources/therush.cpp
    sources/toolbox.cpp
    sources/track.cpp
    sources/world.cpp
    sources/ZShipPhysics.cpp
)

option(LINK_WITH_OPENAL_1_15 "Link with OpenAL Soft library version 1.15 instead of 1.14 (Only supported on Linux for now)" ${IS_OS_LINUX})

if ( LINK_WITH_OPENAL_1_15 )
    set( OPENAL_LIB_DIR_NAME OpenAL_1_15 )
else ()
    set( OPENAL_LIB_DIR_NAME OpenAL_1_14 )
endif ()

set( theRush_INCDIRS
    3rdParty/bullet_2_78/include/bullet
    3rdParty/LibGizmo_1_0/inc
    3rdParty/SDL_1_2_15/include
    3rdParty/RakNet_4_017/include/RakNet
    3rdParty/${OPENAL_LIB_DIR_NAME}/include
    ${theRush_3rdparty_INCDIRS}
    sources
)

set( theRush_INCS ${theRush_3rdparty_INCS} ${theRush_sources_INCS} ${theRush_PCH_INC} )
set( theRush_SRCS ${theRush_bin_SRCS} ${theRush_sources_SRCS} ${theRush_PCH_SRC} )


if (IS_OS_WINDOWS)
    set( LIB_DIR_PLATFORM "win" )
elseif (IS_OS_MACOSX)
    set( LIB_DIR_PLATFORM "osx" )
elseif (IS_OS_LINUX)
    set( LIB_DIR_PLATFORM "lin" )
endif ()

if (IS_PLATFORM_32BIT)
    set( LIB_DIR_PLATFORM "${LIB_DIR_PLATFORM}_x86" )
elseif (IS_PLATFORM_64BIT)
    set( LIB_DIR_PLATFORM "${LIB_DIR_PLATFORM}_x64" )
endif ()

set( LIB_DIR_NAME "lib_${LIB_DIR_PLATFORM}" )

#### OpenGL
if (IS_OS_WINDOWS)
    set( OPENGL_LIBS opengl32 )
elseif (IS_OS_MACOSX)
    set( OPENGL_LIBS "-framework OpenGL" )
elseif (IS_OS_LINUX)
    FIND_PACKAGE(OpenGL)
    set( OPENGL_LIBS "${OPENGL_gl_LIBRARY}" )
endif ()

#### Bullet Physics
set( BULLET_LIBS
    BulletMultiThreaded
    BulletSoftBody
    BulletDynamics
    BulletCollision
    LinearMath
)
set( BULLET_LIB_DIR "${PROJECT_SOURCE_DIR}/3rdParty/bullet_2_78/${LIB_DIR_NAME}" )
if (IS_OS_LINUX AND CMAKE_BUILD_TYPE)
    set( BULLET_LIB_DIR "${BULLET_LIB_DIR}/${CMAKE_BUILD_TYPE}" )
endif ()

#### LibGizmo
if (IS_OS_WINDOWS)
    set( LIBGIZMO_LIBS libgizmo )
    set( LIBGIZMO_LIB_DIR "${PROJECT_SOURCE_DIR}/3rdParty/libGizmo_1_0/${LIB_DIR_NAME}" )
else ()
    set( LIBGIZMO_LIBS "" )
    set( LIBGIZMO_LIB_DIR "" )
endif ()

#### OpenAL
if (IS_OS_WINDOWS)
    set( OPENAL_LIBS OpenAL32 )
else ()
    set( OPENAL_LIBS openal )
endif ()
if (IS_OS_MACOSX)
    set( OPENAL_LIBS ${OPENAL_LIBS} "-framework CoreAudio" "-framework AudioUnit" "-framework AudioToolbox" )
endif ()
set( OPENAL_LIB_DIR "${PROJECT_SOURCE_DIR}/3rdParty/${OPENAL_LIB_DIR_NAME}/${LIB_DIR_NAME}" )

#### Raknet
set( RAKNET_LIBS RakNetStatic )
set( RAKNET_LIB_DIR "${PROJECT_SOURCE_DIR}/3rdParty/RakNet_4_017/${LIB_DIR_NAME}" )

#### SDL
set( SDL_DIR "${PROJECT_SOURCE_DIR}/3rdParty/SDL_1_2_15")

if (IS_OS_WINDOWS)
    set( SDL_LIBS SDL SDLMain )
    set( SDL_LIB_DIR "${SDL_DIR}/${LIB_DIR_NAME}" )
elseif (IS_OS_MACOSX)
# can't get to link when using SDL static library, so for now only compiling with SDL.framework
#    #requires on MacOSX = 10.8 CoreGraphics, otherwise ApplicationServices (which includes CoreGraphics)
#    if (CMAKE_OSX_DEPLOYEMENT_TARGET STREQUAL "10.8")
#        set( CORE_GRAPHICS_FRAMEWORK_NAME "CoreGraphics" )
#    else ()
#        set( CORE_GRAPHICS_FRAMEWORK_NAME "ApplicationServices" )
#    endif ()
#    #requires HIToolbox included in Carbon framework
#    set( SDL_LIBS SDL SDLMain "-framework Cocoa" "-framework ${CORE_GRAPHICS_FRAMEWORK_NAME} -framework Foundation -framework AppKit" "-framework Carbon" "-framework CoreServices" "-framework CoreData" "-framework QuickTime" "-framework IOKit" )

    set( CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} "${SDL_DIR}" )
    include(FindSDL)

    set( SDL_LIBS "${SDL_LIBRARY}" )
    set( SDL_LIB_DIR "${SDL_DIR}" )
    set( theRush_SRCS ${theRush_SRCS} 3rdParty/SDL_1_2_15/src/macosx/SDLMain.h 3rdParty/SDL_1_2_15/src/macosx/SDLMain.m )
elseif (IS_OS_LINUX)
    set( SDL_LIBS SDL )
    set( SDL_LIB_DIR "${SDL_DIR}/${LIB_DIR_NAME}" )
else ()
    include(FindSDL)
    set( SDL_LIBS "${SDL_LIBRARY}" )
    set( SDL_LIB_DIR "${SDL_DIR}/${LIB_DIR_NAME}" )
endif()

#### WinSock
if (IS_OS_WINDOWS)
    set( WINSOCK_LIBS ws2_32 )
else ()
    set( WINSOCK_LIBS "" )
endif ()

#### Platform specific libraries

if (IS_OS_MACOSX)
    set( PLATFORM_LIBS "-framework CoreFoundation" )
else ()
    set( PLATFORM_LIBS "" )
endif ()

####
set( theRush_LIBS
    ${OPENGL_LIBS}
    ${BULLET_LIBS}
    ${LIBGIZMO_LIBS}
    ${OPENAL_LIBS}
    ${RAKNET_LIBS}
    ${SDL_LIBS}
    ${WINSOCK_LIBS}
    ${PLATFORM_LIBS}
    3rdparty_lib
)

set( theRush_LIBDIRS
    ${BULLET_LIB_DIR}
    ${LIBGIZMO_LIB_DIR}
    ${OPENAL_LIB_DIR}
    ${RAKNET_LIB_DIR}
    ${SDL_LIB_DIR}
)



include_directories( ${theRush_INCDIRS} )
link_directories( ${theRush_LIBDIRS} )

if (IS_OS_WINDOWS)
    # 'WIN32' parameter tells the executable point of entry is 'WinMain()' function
    set( THERUSH_EXTRA_EXE_PARAMS "WIN32" )
else ()
    set( THERUSH_EXTRA_EXE_PARAMS "" )
endif ()

#message("theRush_LIBS: ${theRush_LIBS}")

add_executable( theRush ${THERUSH_EXTRA_EXE_PARAMS} ${theRush_SRCS} ${theRush_INCS} )
target_link_libraries( theRush ${theRush_LIBS} )


#get_target_property( therush_property_debug theRush WHATEVER_FLAGS )
#message( therush_property_debug: "${therush_property_debug}" )

#OpenAL specific defines
add_definitions(-DAL_ALEXT_PROTOTYPES)

cmake_dependent_option(USE_OPENAL_STATIC_LIB_IN_DEBUG_RELWITHDEBINFO "Link to OpenAL static library in Debug and RelWithDebInfo builds" TRUE "NOT IS_OS_LINUX" FALSE)

if ( USE_OPENAL_STATIC_LIB_IN_DEBUG_RELWITHDEBINFO )
    set_property( TARGET theRush APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG "-DAL_LIBTYPE_STATIC" )
    set_property( TARGET theRush APPEND PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO "-DAL_LIBTYPE_STATIC" )
endif ()

set_property( TARGET theRush APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE "-DRETAIL")

if (USE_PRECOMPILED_HEADERS)
    if (MSVC)
        set_property( TARGET theRush APPEND PROPERTY COMPILE_FLAGS "${PCH_USE_COMPILER_FLAGS}${PCH_HEADER_FILE}" )
        set_property( SOURCE "${theRush_PCH_SRC}" APPEND PROPERTY COMPILE_FLAGS "${PCH_CREATE_COMPILER_FLAGS}${PCH_HEADER_FILE}" )
    elseif (XCODE_VERSION )
        set_target_properties( theRush PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${theRush_PCH_INC}" )
        set_target_properties( theRush PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES" )
    endif ()
endif ()

#enable logging to file in Debug and ReleaseWithDebInfo
set( THERUSH_ENABLE_LOGGING_TO_FILE "-DUSE_LOG_TO_FILE=1" )
set_property( TARGET theRush APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG "${THERUSH_ENABLE_LOGGING_TO_FILE}" )
set_property( TARGET theRush APPEND PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO "${THERUSH_ENABLE_LOGGING_TO_FILE}" )

set( THERUSH_PROFILING_COMPILE_DEFS "-DUSE_PROFILER=1" )
option(USE_PROFILING_THERUSH_RELWITHDEBINFO "Enable logging and profiling in RELWITHDEBINFO build: ${THERUSH_PROFILING_COMPILE_DEFS}" TRUE)

if (USE_PROFILING_THERUSH_RELWITHDEBINFO)
	#adding logging/profiling compile defs for RELWITHDEBINFO build
	set_property( TARGET theRush APPEND PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO "${THERUSH_PROFILING_COMPILE_DEFS}" )
endif ()

if (MSVC)
    if (GENERATE_MAP_FILE)
        set_property( TARGET theRush APPEND PROPERTY LINK_FLAGS "${GENERATE_MAP_FILE_LINKER_FLAGS}" )
    endif ()

    set_property( TARGET theRush APPEND PROPERTY COMPILE_FLAGS "${THERUSH_WARNING_COMPILER_FLAGS}" )
    #message( "THERUSH_WARNING_COMPILER_FLAGS: ${THERUSH_WARNING_COMPILER_FLAGS}" )

	set_property( TARGET theRush APPEND PROPERTY COMPILE_FLAGS "${THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS}" )
    #message( "THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS: ${THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS}" )

    foreach (config_var ${config_vars})
        if (USE_CUSTOM_OPTIM_FLAGS_THERUSH_${config_var})
            set( flag_var CMAKE_CXX_FLAGS_${config_var} )
            #message( "${flag_var}: ${${flag_var}}" )
            #removing default optim compiler flags
            string( REGEX REPLACE "/O. " "" ${flag_var} "${${flag_var}}" )
            string( REGEX REPLACE "/O.. " "" ${flag_var} "${${flag_var}}" )
            #adding custom optim compiler flags
            #set_property( TARGET theRush APPEND PROPERTY COMPILE_FLAGS_${config_var} ${THERUSH_OPTIM_COMPILE_FLAGS_${config_var}} )
            set( ${flag_var} "${${flag_var}} ${THERUSH_OPTIM_COMPILE_FLAGS_${config_var}}" )
            #message( "${flag_var}: ${${flag_var}}" )

            #adding custom optim linker flags
            set_property( TARGET theRush APPEND PROPERTY LINK_FLAGS_${config_var} "${THERUSH_OPTIM_LINK_FLAGS_${config_var}}" )
        endif ()
    endforeach ()
endif ()

if (XCODE_VERSION)
    set_target_properties( theRush PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${SDL_LIB_DIR}" )
endif ()

if (NOT USE_COMPILER_RTTI)
    if (XCODE_VERSION)
        set_target_properties( theRush PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI "NO")
    endif ()
endif ()

if (NOT USE_COMPILER_EXCEPTION_HANDLING)
    if (XCODE_VERSION)
        set_target_properties( theRush PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
    endif ()
endif ()

if (USE_LINK_TIME_OPTIMIZATION)
    if (XCODE_VERSION)
        set_target_properties( theRush PROPERTIES XCODE_ATTRIBUTE_LLVM_LTO "YES")
        message( "WARNING: 'Link-Time Optimization' is enabled for all configurations,")
        message( "if you want to debug, you will need to disable it in the Compiler 'Code Generation' part, in the 'Build settings' tab." )
    else ()
        #set_property( TARGET theRush APPEND PROPERTY COMPILE_FLAGS_RELWITHDEBINFO ${LINK_TIME_OPTIM_COMPILE_FLAG} )
        #set_property( TARGET theRush APPEND PROPERTY COMPILE_FLAGS_RELEASE ${LINK_TIME_OPTIM_COMPILE_FLAG} )
        set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${LINK_TIME_OPTIM_COMPILE_FLAG}" )
        set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${LINK_TIME_OPTIM_COMPILE_FLAG}" )
        set_property( TARGET theRush APPEND PROPERTY LINK_FLAGS_RELWITHDEBINFO ${LINK_TIME_OPTIM_LINK_FLAG} )
        set_property( TARGET theRush APPEND PROPERTY LINK_FLAGS_RELEASE ${LINK_TIME_OPTIM_LINK_FLAG} )
    endif ()
endif ()

if (USE_PRECOMPILED_HEADERS)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set( theRush_PCH_GCH "${PROJECT_SOURCE_DIR}/${theRush_PCH_INC}.gch" )

        set(_compiler_FLAGS ${CMAKE_CXX_FLAGS})

        string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
        list(APPEND _compiler_FLAGS ${${_flags_var_name}})

        get_target_property( _compile_DEFS theRush COMPILE_DEFINITIONS )
        if ( NOT _compile_DEFS STREQUAL "_compile_DEFS-NOTFOUND" )
            list(APPEND _compiler_FLAGS "${_compile_DEFS}" )
        endif ()

        string(TOUPPER "COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}" _defs_var_name )
        get_target_property( _compile_DEFS_${CMAKE_BUILD_TYPE} theRush ${_defs_var_name} )
        if ( NOT _compile_DEFS_${CMAKE_BUILD_TYPE} STREQUAL "_compile_DEFS_${CMAKE_BUILD_TYPE}-NOTFOUND" )
            list(APPEND _compiler_FLAGS "${_compile_DEFS_${CMAKE_BUILD_TYPE}}" )
        endif ()

        #GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
        #FOREACH(item ${_directory_flags})
            #LIST(APPEND _compiler_FLAGS "-I${item}")
        #ENDFOREACH(item)
        list(APPEND _compiler_FLAGS "-I${PROJECT_SOURCE_DIR}/sources")

        get_directory_property(_directory_flags DEFINITIONS)
        list(APPEND _compiler_FLAGS ${_directory_flags})

        string( REGEX REPLACE ";" " " _compiler_FLAGS "${_compiler_FLAGS}" )
        SEPARATE_ARGUMENTS(_compiler_FLAGS)

        #message("${CMAKE_CXX_COMPILER} -m32 ${_compiler_FLAGS} -x c++-header -o ${theRush_PCH_GCH} -c ${PROJECT_SOURCE_DIR}/${theRush_PCH_SRC}")
        ADD_CUSTOM_COMMAND(
            OUTPUT ${theRush_PCH_GCH}
            COMMAND ${CMAKE_CXX_COMPILER} -m32 ${_compiler_FLAGS} -x c++-header -o ${theRush_PCH_GCH} -c ${PROJECT_SOURCE_DIR}/${theRush_PCH_SRC}
            DEPENDS ${PROJECT_SOURCE_DIR}/${theRush_PCH_SRC} )
        ADD_CUSTOM_TARGET(theRush_gch DEPENDS ${theRush_PCH_GCH})
        ADD_DEPENDENCIES(theRush theRush_gch)
        set_property( TARGET theRush APPEND PROPERTY COMPILE_FLAGS "-Winvalid-pch" )
    endif ()
endif ()

get_target_property( THERUSH_COMPILE_DEFS theRush COMPILE_DEFINITIONS )
if ( NOT THERUSH_COMPILE_DEFS  STREQUAL "THERUSH_COMPILE_DEFS-NOTFOUND" )
    #HACK: removing "-D" from compile defs, cmake add it internally
    string( REGEX REPLACE "-D" "" THERUSH_COMPILE_DEFS "${THERUSH_COMPILE_DEFS}" )
    set_target_properties( theRush PROPERTIES COMPILE_DEFINITIONS "${THERUSH_COMPILE_DEFS}" )
    #message( "THERUSH_COMPILE_DEFS: ${THERUSH_COMPILE_DEFS}" )
endif ()

foreach (config_var ${config_vars})
    string(TOUPPER "COMPILE_DEFINITIONS_${config_var}" COMPILE_DEFINITIONS_var_name)
    get_target_property( THERUSH_COMPILE_DEFS_${config_var} theRush ${COMPILE_DEFINITIONS_var_name} )
    if ( NOT THERUSH_COMPILE_DEFS_${config_var}  STREQUAL "THERUSH_COMPILE_DEFS_${config_var}-NOTFOUND" )
        #HACK: removing "-D" from compile defs, cmake add it internally
        string( REGEX REPLACE "-D" "" THERUSH_COMPILE_DEFS_${config_var} "${THERUSH_COMPILE_DEFS_${config_var}}" )
        set_target_properties( theRush PROPERTIES ${COMPILE_DEFINITIONS_var_name} "${THERUSH_COMPILE_DEFS_${config_var}}" )
        #message( "THERUSH_COMPILE_DEFS_${config_var}: ${THERUSH_COMPILE_DEFS_${config_var}}" )
    endif ()
endforeach ()

#removing ';' from list of compile flags introduced when appending compile flags
get_target_property( THERUSH_COMPILE_FLAGS theRush COMPILE_FLAGS )
if ( NOT THERUSH_COMPILE_FLAGS  STREQUAL "THERUSH_COMPILE_FLAGS-NOTFOUND" )
    string( REGEX REPLACE ";" " " THERUSH_COMPILE_FLAGS "${THERUSH_COMPILE_FLAGS}" )
    set_target_properties( theRush PROPERTIES COMPILE_FLAGS "${THERUSH_COMPILE_FLAGS}" )
    #message( "THERUSH_COMPILE_FLAGS: ${THERUSH_COMPILE_FLAGS}" )
endif ()

#removing ';' from list of link flags introduced when appending link flags
get_target_property( THERUSH_LINK_FLAGS theRush LINK_FLAGS )
if ( NOT THERUSH_LINK_FLAGS  STREQUAL "THERUSH_LINK_FLAGS-NOTFOUND" )
    string( REGEX REPLACE ";" " " THERUSH_LINK_FLAGS "${THERUSH_LINK_FLAGS}" )
    set_target_properties( theRush PROPERTIES LINK_FLAGS "${THERUSH_LINK_FLAGS}" )
    #message( "THERUSH_LINK_FLAGS: ${THERUSH_LINK_FLAGS}" )
endif ()

foreach (config_var ${config_vars})
    get_target_property( THERUSH_LINK_FLAGS_${config_var} theRush LINK_FLAGS_${config_var} )
    if ( NOT THERUSH_LINK_FLAGS_${config_var}  STREQUAL "THERUSH_LINK_FLAGS_${config_var}-NOTFOUND" )
        string( REGEX REPLACE ";" " " THERUSH_LINK_FLAGS_${config_var} "${THERUSH_LINK_FLAGS_${config_var}}" )
        set_target_properties( theRush PROPERTIES LINK_FLAGS_${config_var} "${THERUSH_LINK_FLAGS_${config_var}}" )
        #message( "THERUSH_LINK_FLAGS_${config_var}: ${THERUSH_LINK_FLAGS_${config_var}}" )
    endif ()
endforeach ()

#display C / CXX compiler flags
foreach (flag_var ${compiler_C_CXX_flag_vars})
    #message( ${flag_var}: ${${flag_var}} )
endforeach ()

foreach (config_var ${config_vars})
    get_target_property( THERUSH_LINK_FLAGS_${config_var} theRush LINK_FLAGS_${config_var} )
    #message( "THERUSH_LINK_FLAGS_${config_var}: ${THERUSH_LINK_FLAGS_${config_var}}" )
endforeach ()

if (IS_IDE_MSVC)
    message( "Once you open *.sln file, in visual studio, right-click on '${PROJECT_NAME}' project and 'Set as StartUp Project'." )
    message( "In '${PROJECT_NAME}' project 'Debugging Properties', set 'Command Arguments' to either 'main' or 'edit'" )
    message( "Also in 'Debugging Properties', fixup the 'Working Directory' path to point to 'Bin' folder which should contain 'rushDatas.cpp'" )
    message( "You probably want to do all those changes in 'Debugging Properties' for 'All Configurations'" )
elseif (IS_IDE_XCODE)
    message( "Once you open *.xcodeproj file, in xcode, select 'theRush > My Mac 32-bit' as the active scheme.")
    message( "In 'Product' menu, select 'Edit Scheme...', then in 'Options' tab for 'Run theRush'" )
    message( "Check 'Use custom working directory' and fixup the 'Working Directory' path to point to 'Bin' folder which should contain 'rushDatas.cpp'" )
elseif (IS_IDE_CODEBLOCKS)
    message( "Once you open *.cbp file, in codeblocks:")
    message( "In 'Project' menu, select 'Properties...', then in 'Build Targets' tab, select '${PROJECT_NAME}'" )
    message( "Fixup 'Execution working directory' path to point to 'Bin' folder which should contain 'rushDatas.cpp'" )
elseif (IS_IDE_NONE)
    message( "Once compiled, you need to run the executable from the game project 'Bin' folder which should contain 'rushDatas.cpp'." )
endif ()

############ Build other sub-projects
set( DEFAULT_BUILD_ADDITIONAL_TARGETS ${IS_OS_WINDOWS} )
cmake_dependent_option(BUILD_ADDITIONAL_TARGETS "Build AssetsImport (windows only supported for now)" TRUE "DEFAULT_BUILD_ADDITIONAL_TARGETS" FALSE)

if ( BUILD_ADDITIONAL_TARGETS )
    add_subdirectory(build)
endif ()

############ R2 PROJECT


project( R2 )
# So that whole solution will be named R2
