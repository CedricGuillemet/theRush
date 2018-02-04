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

#pragma warning(push)
#pragma warning(disable:4005)   //Disabling warning 4005: macro redefinition, because GLee always define 'WIN32_LEAN_AND_MEAN'
#include "GLee.h"
#pragma warning(pop)

#if IS_OS_LINUX
#include <GL/gl.h>
#include <GL/glx.h>
#elif IS_OS_MACOSX
#include <OpenGL/gl.h>
#elif IS_OS_WINDOWS
#include <gl/gl.h>
#include <gl/glu.h>
#endif
