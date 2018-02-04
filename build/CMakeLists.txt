############ AssetsImport PROJECT

project(AssetsImport)

#message( "Configuring Assets Import Project" )

set( AssetsImport_3rdparty_INCS
    ${R2_SOURCE_DIR}/3rdParty/JSON_parser.h
    ${R2_SOURCE_DIR}/3rdParty/mongoose.h
)
set( AssetsImport_sources_INCS
    ${R2_SOURCE_DIR}/sources/debug_assert.h
    ${R2_SOURCE_DIR}/sources/debug_common.h
    ${R2_SOURCE_DIR}/sources/define_macros.h
    ${R2_SOURCE_DIR}/sources/define_platform.h
    ${R2_SOURCE_DIR}/sources/define_types.h
    ${R2_SOURCE_DIR}/sources/include_GL.h
    ${R2_SOURCE_DIR}/sources/maths.h
    ${R2_SOURCE_DIR}/sources/toolbox.h
)

set( AssetsImport_3rdparty_SRCS
    ${R2_SOURCE_DIR}/3rdParty/JSON_parser.c
    ${R2_SOURCE_DIR}/3rdParty/mongoose.c
)
set( AssetsImport_sources_SRCS
    ${R2_SOURCE_DIR}/sources/AssetsImport.cpp
    ${R2_SOURCE_DIR}/sources/debug_assert.cpp
    ${R2_SOURCE_DIR}/sources/debug_common.cpp
    ${R2_SOURCE_DIR}/sources/maths.cpp
    ${R2_SOURCE_DIR}/sources/toolbox.cpp
)

set( AssetsImport_INCDIRS ${R2_SOURCE_DIR}/3rdParty ${R2_SOURCE_DIR}/sources )
set( AssetsImport_INCS ${AssetsImport_3rdparty_INCS} ${AssetsImport_sources_INCS} )
set( AssetsImport_SRCS ${AssetsImport_3rdparty_SRCS} ${AssetsImport_sources_SRCS} )

#### OpenGL
if (IS_OS_WINDOWS)
    set( OPENGL_LIBS opengl32 )
elseif ( IS_OS_MACOSX )
    set( OPENGL_LIBS "-framework OpenGL" )
elseif ( IS_OS_LINUX )
    FIND_PACKAGE(OpenGL)
    set( OPENGL_LIBS "${OPENGL_gl_LIBRARY}" )
endif ()

#### WinSock
if (IS_OS_WINDOWS)
    set( WINSOCK_LIBS ws2_32 )
else ()
    set( WINSOCK_LIBS "" )
endif ()

set( AssetsImport_LIBS
    ${OPENGL_LIBS}
    ${WINSOCK_LIBS}
)

include_directories( ${AssetsImport_INCDIRS} )

add_executable( AssetsImport ${AssetsImport_SRCS} ${AssetsImport_INCS} )
target_link_libraries( AssetsImport ${AssetsImport_LIBS} )

if (MSVC)
    set_property( TARGET AssetsImport APPEND PROPERTY COMPILE_FLAGS "${THERUSH_WARNING_COMPILER_FLAGS}" )
    #message( "THERUSH_WARNING_COMPILER_FLAGS: ${THERUSH_WARNING_COMPILER_FLAGS}" )

	set_property( TARGET AssetsImport APPEND PROPERTY COMPILE_FLAGS "${THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS}" )
    #message( "THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS: ${THERUSH_DISABLED_WARNINGS_COMPILE_FLAGS}" )

    set( ASSETSIMPORT_OPTIM_COMPILE_FLAGS_DEBUG "/Od /Ob0 /Oy-" )
    set( ASSETSIMPORT_OPTIM_LINK_FLAGS_DEBUG "" )
    set( ASSETSIMPORT_OPTIM_COMPILE_FLAGS_RELWITHDEBINFO "/O2 /Oi /Oy- /GL /Gy" )
    set( ASSETSIMPORT_OPTIM_LINK_FLAGS_RELWITHDEBINFO "/OPT:REF /OPT:ICF /LTCG" )
    set( ASSETSIMPORT_OPTIM_COMPILE_FLAGS_RELEASE "/O2 /Oi /Oy- /GL /Gy" )
    set( ASSETSIMPORT_OPTIM_LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF /LTCG" )

    foreach (config_var ${therush_config_vars})
        if (USE_CUSTOM_OPTIM_FLAGS_THERUSH_${config_var})
            set( flag_var CMAKE_CXX_FLAGS_${config_var} )

            #restoring default optim compiler flags
            set( ${flag_var} "${${flag_var}_DEFAULT}" )

            #removing default optim compiler flags
            string( REGEX REPLACE "/O. " "" ${flag_var} "${${flag_var}}" )
            string( REGEX REPLACE "/O.. " "" ${flag_var} "${${flag_var}}" )

            #adding custom optim compiler flags
            set( ${flag_var} "${${flag_var}} ${ASSETSIMPORT_OPTIM_COMPILE_FLAGS_${config_var}}" )
            #message( ${flag_var}: ${${flag_var}} )

            #adding custom optim linker flags
            set_property( TARGET AssetsImport APPEND PROPERTY LINK_FLAGS_${config_var} "${ASSETSIMPORT_OPTIM_LINK_FLAGS_${config_var}}" )
        endif ()
    endforeach ()
endif ()

#removing ';' from list of compile flags introduced when appending compile flags
get_target_property( ASSETSIMPORT_COMPILE_FLAGS AssetsImport COMPILE_FLAGS )
if ( NOT ASSETSIMPORT_COMPILE_FLAGS  STREQUAL "ASSETSIMPORT_COMPILE_FLAGS-NOTFOUND" )
    string( REGEX REPLACE ";" " " ASSETSIMPORT_COMPILE_FLAGS "${ASSETSIMPORT_COMPILE_FLAGS}" )
    set_target_properties( AssetsImport PROPERTIES COMPILE_FLAGS "${ASSETSIMPORT_COMPILE_FLAGS}" )
    #message( "ASSETSIMPORT_COMPILE_FLAGS: ${ASSETSIMPORT_COMPILE_FLAGS}" )
endif ()

#removing ';' from list of link flags introduced when appending compile flags
get_target_property( ASSETSIMPORT_LINK_FLAGS AssetsImport LINK_FLAGS )
if ( NOT ASSETSIMPORT_LINK_FLAGS  STREQUAL "ASSETSIMPORT_LINK_FLAGS-NOTFOUND" )
    string( REGEX REPLACE ";" " " ASSETSIMPORT_LINK_FLAGS "${ASSETSIMPORT_LINK_FLAGS}" )
    set_target_properties( AssetsImport PROPERTIES LINK_FLAGS "${ASSETSIMPORT_LINK_FLAGS}" )
    #message( "ASSETSIMPORT_LINK_FLAGS: ${ASSETSIMPORT_LINK_FLAGS}" )
endif ()
