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

#ifndef THERUSH_H__
#define THERUSH_H__

///////////////////////////////////////////////////////////////////////////////////////////////////

bool KeyUpDown();
bool KeyDownDown();
bool KeyLeftDown();
bool KeyRightDown();
bool KeyCtrlDown();
bool KeyShiftDown();


void QuitGameNow();
void ClearKeysReleased();

bool MouseLButDown();
bool MouseLButPressed();
bool MouseLButReleased();
bool MouseRButDown();


bool POVLeft();
bool POVRight();
bool POVLeftReleased();
bool POVRightReleased();


bool keysReleased(int code);

struct track_t;

track_t *GetTrack0();

// visible surface
extern float WIDTH;
extern float HEIGHT;
// screen/window surface
extern float SCREENWIDTH;
extern float SCREENHEIGHT;

typedef struct resolution_t
{
	resolution_t(int _w, int _h, char* _t) 
	{
		Width = _w;
		Height = _h;
		textValue = _t;
	}
	int Width, Height;
	std::string textValue;
}resolution_t;

const std::vector<resolution_t>& GetSupportedScreenResolutions();

#pragma pack(push,1)
typedef struct config_t
{
	uint32 fullscreen;
	uint32 desiredWidth;
	uint32 desiredHeight;
	float fxVolume;
	float musicVolume;
	uint32 AAactive;
    uint32 RenderShadows;
    uint32 RenderOcean;
    uint32 RenderReflection;
	float mouseSensibility;
}config_t;
#pragma pack(pop)

const config_t& GetEngineConfig();
void SetEngineConfig( const config_t& Config );
bool IsValidEngineConfig( const config_t& Config );
bool AreEngineConfigEquals( const config_t& ConfigA, const config_t& ConfigB );

void ChangeVideoModeAfterBufferSwap();

std::string GetPictureDirectoy();
float GetApplicationTime();
float GetWorldTime();

#endif