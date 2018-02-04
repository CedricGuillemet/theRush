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

#if IS_OS_LINUX
#undef None
#endif


#include "gui.h"
#include "physics.h"
#include "mesh.h"
#include "menus.h"
#include "camera.h"
#include "render.h"
#include "world.h"
#include "physics.h"
#include "content.h"
#include "solo.h"
#include "therush.h"
#include "game.h"
#include "net.h"
#include "fx.h"
#include "track.h"
#include "bonus.h"
#include "ZShipPhysics.h"
#include "solo.h"
#include "audio.h"
//#include <SDL/SDL_main.h>
#include "tinythread.h"
#include "JSON_serializer.h"

#if IS_OS_MACOSX
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#elif IS_OS_LINUX
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#elif IS_OS_WINDOWS
#include <MMSystem.h>
#include <Objbase.h>
#include <DShow.h>
#endif


#include "include_GL.h"
#include "include_SDL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#if EDITOR_ENABLE
void InitEditor();
void TickEditor( float aTimeEllapsed );
void DrawEditor( float aTimeEllapsed );
void UninitEditor();
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

float WIDTH = 1280.f;
float HEIGHT = 720.f;
float SCREENWIDTH = 1280.f;
float SCREENHEIGHT = 720.f;
bool splitScreenEnabled = false;

float GFixedTimeStamp = 0.0f;
bool GQuitAsked = false;
float GInputGlobalSpeed = 1.f;

float inactivityTime = 0.f;

float GetApplicationTime()
{
    return static_cast<float>(SDL_GetTicks())/1000.f;
}

float GWorldTime = 0.f;
float GetWorldTime()
{
    return GWorldTime;
}
void DrawFullScreenQuad();

bool AADesactivatedByCommandLine(false), ReflDesactivatedByCommandLine(false), ShadowsDesactivatedByCommandLine(false);

///////////////////////////////////////////////////////////////////////////////////////////////////

#if IS_OS_WINDOWS

#include <XInput.h>

#pragma comment(lib, "XInput.lib")


enum GamePadButton
{
	GAMEPAD_DPAD_UP          =0x0001,
	GAMEPAD_DPAD_DOWN        =0x0002,
	GAMEPAD_DPAD_LEFT        =0x0004,
	GAMEPAD_DPAD_RIGHT       =0x0008,
	GAMEPAD_START            =0x0010,
	GAMEPAD_BACK             =0x0020,
	GAMEPAD_LEFT_THUMB       =0x0040,
	GAMEPAD_RIGHT_THUMB      =0x0080,
	GAMEPAD_LEFT_SHOULDER    =0x0100,
	GAMEPAD_RIGHT_SHOULDER   =0x0200,
	GAMEPAD_A                =0x1000,
	GAMEPAD_B                =0x2000,
	GAMEPAD_X                =0x4000,
	GAMEPAD_Y                =0x8000
};


// handles XInput gamepads
class XInputManager
{

public:
	XInputManager()
	{
		memset(&state, 0, sizeof(XINPUT_STATE));
		memset(&stateOld, 0, sizeof(XINPUT_STATE));
		connectedPad[0]  =connectedPad[1] = connectedPad[2] = connectedPad[3] = 0;
	}

	void Update()
	{
		DWORD dwResult;

		for(int i = 0; i<4;i++)
		{
			memcpy(&stateOld[i], &state[i], sizeof(XINPUT_STATE));
			ZeroMemory( &state[i], sizeof(XINPUT_STATE) );

			dwResult = XInputGetState(i, &state[i]);
			connectedPad[i] = false;
			if(dwResult == ERROR_SUCCESS)
			{
				connectedPad[i] = true;
			}
		}
	}

	bool PadPresent( int pad)
	{
		return ( (pad < 3) && connectedPad[pad] );
	}

	bool ButDown(int pad, GamePadButton but)
	{
		return ( (pad < 4) && (state[pad].Gamepad.wButtons & but) != 0 );
	}

	bool ButReleased(int pad, GamePadButton but)
	{
		return ((pad < 4) && ( (state[pad].Gamepad.wButtons & but) == 0 ) && ( (stateOld[pad].Gamepad.wButtons & but) != 0 ) );
	}

	void GetLeftStick(int pad, float &x, float &y)
	{
		if (pad >3 || ( state[pad].Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && state[pad].Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
			x = 0.0;
		else
			x = state[pad].Gamepad.sThumbLX / 32768.f;

		if (pad >3 || ( state[pad].Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && state[pad].Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
			y = 0.0f;
		else
			y = state[pad].Gamepad.sThumbLY / 32768.f;
	}

	void GetRightStick(int pad, float &x, float &y)
	{
		if (pad >3 || ( state[pad].Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && state[pad].Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ))
			x = 0.0f;
		else
			x = state[pad].Gamepad.sThumbRX / 32768.f;

		if (pad >3 || ( state[pad].Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && state[pad].Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ))
			y = 0.0f;
		else
			y = state[pad].Gamepad.sThumbRY / 32768.f;
	}

	float GetRightTrigger( int pad )
	{
		if(pad <4 && state[pad].Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			return (state[pad].Gamepad.bRightTrigger / 255.0f);
		return 0.0f;
	}

	float GetLeftTrigger( int pad )
	{
		if( pad <4 && state[pad].Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			return (state[pad].Gamepad.bLeftTrigger / 255.0f);
		return 0.0f;
	}

	void SetVibration(int pad, float left, float right) // set vibration strength
	{
		// we can't have more than 4 pads!
		if(pad > 3)
			return;

		XINPUT_VIBRATION vibration;
		ZeroMemory( &vibration, sizeof(XINPUT_VIBRATION) );
		vibration.wLeftMotorSpeed = static_cast<WORD>(65535.f * left); // use any value between 0-65535 here
		vibration.wRightMotorSpeed = static_cast<WORD>(65535.f * right); // use any value between 0-65535 here
		XInputSetState( pad, &vibration );
	}
private:
	XINPUT_STATE state[4];         // xinput states
	XINPUT_STATE stateOld[4];         // xinput states

	bool connectedPad[4];
};

#endif  //  IS_OS_WINDOWS

unsigned short previousKeyDown[SDLK_LAST] = {0};//sf::Key::Count];
unsigned short currentKeyDown[SDLK_LAST] = {0};//sf::Key::Count];

float mPreviousPov = -1.f;
float mCurrentPov = -1.f;

bool PreviousJoyButtonDown[32];

bool POVUpReleased() { return false; }//( ( fabsf( mCurrentPov ) < 0.1f ) &&  (mPreviousPov < 0.f) ); }
bool POVDownReleased() { return false; }//( ( fabsf( mCurrentPov - 180.f ) < 0.1f ) && (mPreviousPov < 0.f) ); }
bool POVLeftReleased() { return false; }//( ( fabsf( mCurrentPov - 270.f ) < 0.1f ) && (mPreviousPov < 0.f) ); }
bool POVRightReleased() { return false; }//( ( fabsf( mCurrentPov - 90.f ) < 0.1f ) && (mPreviousPov < 0.f) ); }
bool POVLeft() { return false; }//(fabsf( mCurrentPov - 270.f ) < 0.1f ); }
bool POVRight() { return false; }//(fabsf( mCurrentPov - 90.f ) < 0.1f ); }


bool JoyButReleased( int iBut ) { return false; } //(App.GetInput().IsJoystickButtonDown( 0, iBut) && (!PreviousJoyButtonDown[ iBut ]) ); }

int MouseDeltaX = 0, MouseDeltaY = 0;
int PreviousMouseX, PreviousMouseY;
int MouseX = 0, MouseY = 0;
bool mbMouseLeftDown = false;
bool mbMouseRightDown = false;
bool mbPreviousMouseLeftDown = false;
bool mbPreviousMouseRightDown = false;

bool GBDrawCameraPosition = false;

bool MouseLButDown() { return mbMouseLeftDown; }
bool MouseLButReleased() { return ((!mbMouseLeftDown)&&(mbPreviousMouseLeftDown)); }
bool MouseLButPressed() { return ((mbMouseLeftDown)&&(!mbPreviousMouseLeftDown)); }

bool MouseRButDown() { return mbMouseRightDown; }

#ifndef RETAIL

float GTimeRecordKeys = 0.f;
#define KeyLogFileName "KeyLog.dat"
float GLastTimePlayBack = 0.f;
FILE *GPlaybackKeysFile = NULL;
bool GBRecordKeysIsActive = false;
bool GBPlaybackKeysIsActive = false;
static const unsigned int keysDataSize = sizeof( unsigned short) * SDLK_LAST;

void SaveRecordBuffer()
{
    FILE *fp = fopen( KeyLogFileName, "a+b" );
    if ( fp )
    {
        fwrite ( &GTimeRecordKeys, sizeof(float), 1, fp );
        fwrite ( currentKeyDown, keysDataSize, 1, fp );
        fclose ( fp );
    }
}

void InitRecordKeys()
{
    if (!GBRecordKeysIsActive)
        return;

    FILE *fp = fopen( KeyLogFileName, "wb" );
    if ( fp )
    {
        fclose ( fp );
    }
    SaveRecordBuffer();
}

void TickRecordKeys( float aTimeEllapsed )
{
    if (!GBRecordKeysIsActive)
        return;

    GTimeRecordKeys += aTimeEllapsed;


    if ( memcmp( previousKeyDown, currentKeyDown, keysDataSize ) )
    {
        SaveRecordBuffer();
        if ( currentKeyDown[0/*sf::Key::X*/] )
            exit(0);
    }
}

float GPBKBufferTime = 0.f;
unsigned short GPBKBufferValues[SDLK_LAST];

void ReadPlaybackEntry()
{
    fread( &GPBKBufferTime, sizeof(float), 1, GPlaybackKeysFile );
    fread( GPBKBufferValues, keysDataSize, 1, GPlaybackKeysFile );
}


void ApplyPlaybackBuffer()
{
    memcpy ( currentKeyDown, GPBKBufferValues, keysDataSize );
}

void InitPlayBackKeys()
{
    if (!GBPlaybackKeysIsActive)
        return;

    GPlaybackKeysFile = fopen( KeyLogFileName, "rb");
    ReadPlaybackEntry();
    GLastTimePlayBack = 0.f;
}

void PlayBackKeys( float aTimeEllapsed )
{
    if ( !GBPlaybackKeysIsActive)
        return;

    if ( !GPlaybackKeysFile )
        return;

    GLastTimePlayBack += aTimeEllapsed;
    if ( GLastTimePlayBack > GPBKBufferTime )
    {
        ApplyPlaybackBuffer();

        if ( currentKeyDown[SDLK_x] )
            exit(0);

        ReadPlaybackEntry();


    }
}

#endif

void ClearKeysReleased()
{
	memset(previousKeyDown, 0, sizeof(unsigned short) * SDLK_LAST);
}

bool KeyUpDown()
{
	return (currentKeyDown[SDLK_UP] != 0 );
}

bool KeyDownDown()
{
	return (currentKeyDown[SDLK_DOWN] != 0 );
}
bool KeyLeftDown()
{
	return (currentKeyDown[SDLK_LEFT] != 0 );
}
bool KeyRightDown()
{
	return (currentKeyDown[SDLK_RIGHT] != 0 );
}

bool KeyUseDown()
{
    return ( currentKeyDown[SDLK_SPACE] != 0 );
}

bool KeyUseAlternateDown()
{
    return ( currentKeyDown[SDLK_LCTRL] != 0 );
}


bool keysReleased(int code)
{
	return ( (!currentKeyDown[code]) && (previousKeyDown[code]) );
}

bool KeyShiftDown() { return (currentKeyDown[SDLK_LSHIFT] || currentKeyDown[SDLK_RSHIFT]); }
bool KeyCtrlDown() { return (currentKeyDown[SDLK_LCTRL] || currentKeyDown[SDLK_RCTRL]); }

bool keyUndo()
{
	return (keysReleased(SDLK_z) && KeyCtrlDown());
}
bool keyRedo()
{
	return (keysReleased(SDLK_y) && KeyCtrlDown());
}


bool keysEditingReleased()
{
	return keysReleased(SDLK_TAB);
}

bool keyUpdateDatasReleased()
{
	return keysReleased(SDLK_o);
}

bool keyScreenCaptureReleased()
{
	return keysReleased(SDLK_p);
}

void QuitGameNow()
{
	GQuitAsked = true;
}

void StartupTickPylonsBoxes(float aTimeEllapsed);
void tickInputs(float aTimeEllapsed)
{
	int validatePressed = 0;
	memcpy(previousKeyDown, currentKeyDown, sizeof(unsigned short) * SDLK_LAST );


    Uint8 *keystate = SDL_GetKeyState(NULL);

	for (int i=0;i<SDLK_LAST;i++)
	{
		currentKeyDown[i] = keystate[i];
	}


    /*
    char zop[512];
    sprintf( zop, "POV : %5.2f X:%5.2f Y:%5.2f Z:%5.2f", App.GetInput().GetJoystickAxis( 0, sf::Joy::AxisPOV),
        App.GetInput().GetJoystickAxis( 0, sf::Joy::AxisX),
        App.GetInput().GetJoystickAxis( 0, sf::Joy::AxisY),
        App.GetInput().GetJoystickAxis( 0, sf::Joy::AxisZ));
    ggui.putText(9, 18, zop);
    */
	PreviousMouseX = MouseX;
	PreviousMouseY = MouseY;
    /*
	MouseX = 0;//App.GetInput().GetMouseX();
	MouseY = 0;//App.GetInput().GetMouseY();
    */
    Uint8 mouseButState = SDL_GetMouseState( &MouseX, &MouseY );

	MouseDeltaX = MouseX - PreviousMouseX;
	MouseDeltaY = MouseY - PreviousMouseY;

    mbPreviousMouseLeftDown = mbMouseLeftDown;
    mbPreviousMouseRightDown = mbMouseRightDown;

	mbMouseLeftDown = ((mouseButState & SDL_BUTTON_LMASK ) != 0);//App.GetInput().IsMouseButtonDown(0);//sf::Mouse::Left);
	mbMouseRightDown = ((mouseButState & SDL_BUTTON_RMASK ) != 0);//App.GetInput().IsMouseButtonDown(0);//sf::Mouse::Right);

	if ( keysReleased( SDLK_RETURN ) || JoyButReleased(0) )
	{
		validatePressed = 1;
	}

    if ( keysReleased( SDLK_ESCAPE ) || JoyButReleased(1))
	{
		Menus::Esc();
		soloBack = true;
        //App.Close();
	}


    if ( keysReleased(SDLK_UP) || POVUpReleased() )
	{
		ggui.selectorPrevious();
		soloUp = true;
	}
	if ( keysReleased(SDLK_DOWN) || POVDownReleased() )
	{
		ggui.selectorNext();
		soloDown = true;
	}

	// ships
    shipControls_t ctrl, ctrl2;
    /*
	if (physicWorld.mShip)
	{
	physicWorld.mShip->NoRun();
	physicWorld.mShip->NoTurn();
     */

    if (KeyUpDown() )//|| ( App.GetInput().GetJoystickAxis( 0, 0/*sf::Joy::AxisZ*/ ) < -20.f ))
        ctrl.mRun = 0xFF;
    if (KeyRightDown() )// || ( App.GetInput().GetJoystickAxis( 0, 0/*sf::Joy::AxisX*/ ) > 50.f ) )
        ctrl.mLeft = 0xFF;
    if (KeyLeftDown() )//|| ( App.GetInput().GetJoystickAxis( 0, 0/*sf::Joy::AxisX*/ ) < -50.f ) )
        ctrl.mRight = 0xFF;

    if (KeyUseDown() || JoyButReleased(0) )
        ctrl.mUse = 1;

    if (KeyUseAlternateDown() || JoyButReleased(1) )
        ctrl.mUseAlternate = 1;


#ifndef RETAIL
	// Update datas for ships
    /*
	if (keyUpdateDatasReleased())
	{
		parseJSONShipsData();
		if (GGame)
			GGame->UpdateProperties();
                Renderer::UpdateRenderSettings();
	}
        */
	if (keyScreenCaptureReleased())
	{
		makeScreenShot( SDL_GetTicks( ) );
	}
	if (keysReleased(0/*sf::Key::L*/))
	{
		if (GGame && GGame->GetPlayerShip())
			GGame->GetPlayerShip()->mProps.mNumberLapsDone += 1.f;
	}
    /*
	if (keysReleased(sf::Key::Y))
	{
		//StartupPylon::ResetAll();
		if ( GGame->GetPlayerShip()->mProps.mBoost < 1.f)
			GGame->GetPlayerShip()->mProps.mBoost += 8.0f;
	}
    */
	if (keysReleased(SDLK_w))
	{
        vec_t hitPos, hitNorm;
        float len =-1.f;
        if (physicWorld.rayCast( camera.mViewInverse.position, camera.mViewInverse.position + camera.mViewInverse.dir, hitPos, hitNorm) )
            len = (hitPos-camera.mViewInverse.position).length();

        LOG(" vec(%5.4ff, %5.4ff, %5.4ff), vec(%5.4ff, %5.4ff, %5.4ff), %5.4ff\n", camera.mViewInverse.position.x,
            camera.mViewInverse.position.y,
            camera.mViewInverse.position.z,

            camera.mViewInverse.dir.x,
            camera.mViewInverse.dir.y,
            camera.mViewInverse.dir.z,
            len
            );
	}
	if ( GGame && GGame->GetPlayerShip() )
	{
		int bn = -1;
		for (int i=0;i<12;i++)
		{
			if (keysReleased( SDLK_F1 + i/*(sf::Key::Code)(sf::Key::F1+i)*/))
			{
				bn = i;
				break;
			}
		}
		if ( bn>0)
			{
			// give bonus to everyone
			for ( unsigned int i= 0;i<GGame->GetShipCount();i++)
			{
				//ForceBonusToShip( GGame->GetShip( i ), bn );
				Renderer::newVelvet( vec(1.f), 0);
				GGame->GetShip(0)->GetPhysics()->SetZeroSpeed();
			}
		}
	}
	/*
	if ( keysReleased( SDLK_F2 ) )
	{
		Renderer::newVelvet( vec(1.f,0.05f,0.05f), 0);
		GGame->GetPlayerShip(0)->GetPhysics()->SetZeroSpeed();
	}
	*/
	/*
	if (keysReleased(SDLK_v))
	{
		extern bool GUsePrePass;
		GUsePrePass = !GUsePrePass;
		if (GUsePrePass)
			MessageBox(NULL,"prepass on","prepass on", MB_OK);
		else
			MessageBox(NULL,"prepass off","prepass off", MB_OK);
	}
	*/
	if (keysReleased(0/*sf::Key::U*/))
		GBDrawCameraPosition = !GBDrawCameraPosition;
#endif

#ifndef RETAIL
    TickRecordKeys( aTimeEllapsed );
    PlayBackKeys( aTimeEllapsed );
#endif

    mPreviousPov = mCurrentPov;
    mCurrentPov = 0;//App.GetInput().GetJoystickAxis( 0, sf::Joy::AxisPOV);

    for ( int i = 0 ; i < 32 ; i++ )
    {
        PreviousJoyButtonDown[i] = 0;//App.GetInput().IsJoystickButtonDown( 0, i );
    }

    // camera switch
    if ( keysReleased( SDLK_TAB ) )
    {
        ++Solo::playerProfil.cameraMode[0] %= 3;
    }

	// debug AI mode
#ifndef RETAIL
	if ( keysReleased( SDLK_TAB ) && KeyShiftDown() )
	{
		static int behindShip = 0;
		camera.SetCameraBehindShip( GGame->GetShip(behindShip) );
		behindShip++;
	}

#endif
    /*
bool POVUpReleased() { return ( App.GetInput().GetJoystickAxis( 0, sf::Joy::AxisPOV) < 0.f && fabsf(mPreviousPov )<0.1f ); }
bool POVDownReleased() { return ( App.GetInput().GetJoystickAxis( 0, sf::Joy::AxisPOV) < 0.f && fabsf(mPreviousPov- 180.f)<0.1f ); }
bool JoyButReleased( int iBut ) { return (App.GetInput().IsJoystickButtonDown( 0, iBut) && (!PreviousJoyButtonDown[ iBut ]) ); }
*/
#if IS_OS_WINDOWS

    // handles XInput gamepads
    static XInputManager xim;
	xim.Update();

    extern float padlx, padly;
    padlx = 0.f;
    padly = 0.f;
	shipControls_t * shipControlsPtr[]={ &ctrl, &ctrl2};
	for (int i = 0;i<2;i++)
	{
		if ( xim.PadPresent( i ) )
		{
			shipControls_t& myCtrl = *shipControlsPtr[i];

			float lx, ly, rx, ry;
			xim.GetLeftStick(i, lx, ly );
			xim.GetRightStick(i, rx, ry );

			padlx = lx;
			padly = ly;

			myCtrl.mRun = static_cast<u8>( Clamp(myCtrl.mRun + xim.GetRightTrigger(i)*255.f, 0.f, 255.f) );
			//myCtrl.mLeft = myCtrl.mRight = 0;
			if ( lx < 0.f )
				myCtrl.mRight = static_cast<u8>( Clamp(myCtrl.mRight+(-lx)*255.f, 0.f, 255.f) );
			if ( lx > 0.f )
				myCtrl.mLeft = static_cast<u8>( Clamp(myCtrl.mLeft+lx*255.f, 0.f, 255.f) );

			myCtrl.mUse |= xim.ButDown( i, GAMEPAD_A )?0xFF:0;

			if ( xim.ButReleased( i, GAMEPAD_DPAD_UP ) )
			{
				ggui.selectorPrevious(i);
				soloUp = true;
			}

			if ( xim.ButReleased( i, GAMEPAD_DPAD_DOWN ) )
			{
				ggui.selectorNext(i);
				soloDown = true;
			}

			if ( xim.ButReleased( i, GAMEPAD_B ) || xim.ButReleased( 0, GAMEPAD_START ) || xim.ButReleased( i, GAMEPAD_BACK ) )
			{
				Menus::Esc();
				soloBack = true;
			}
			
			// in menus, for each selecting player
			if ( ggui.getSelectorCount(i) && xim.ButReleased( i, GAMEPAD_A ) )
			{
				validatePressed = i+1;
			}
			// no menus/selector, any input counts
			if ( (!ggui.getSelectorCount(0))&&(!ggui.getSelectorCount(1))&& xim.ButReleased( i, GAMEPAD_A ) )
			{
				validatePressed = i+1;
			}

			// camera switch
			if ( xim.ButReleased( i, GAMEPAD_Y ) )
			{
				++Solo::playerProfil.cameraMode[i] %= 3;
			}
			/*
			if (xim.ButReleased( i, GAMEPAD_START ) )
			{
				Menus::Esc();
				soloBack = true;
			}
			*/
			if (!i)
			{
				extern bool xPOVLeft, xPOVRight;
				xPOVLeft = xim.ButReleased( i, GAMEPAD_DPAD_LEFT );
				xPOVRight = xim.ButReleased( i, GAMEPAD_DPAD_RIGHT );
			}

			extern float inactivityTime;

			if (xim.ButDown(i,GAMEPAD_DPAD_UP       )|
				xim.ButDown(i,GAMEPAD_DPAD_DOWN     )|
				xim.ButDown(i,GAMEPAD_DPAD_LEFT     )|
				xim.ButDown(i,GAMEPAD_DPAD_RIGHT    )|
				xim.ButDown(i,GAMEPAD_START         )|
				xim.ButDown(i,GAMEPAD_BACK          )|
				xim.ButDown(i,GAMEPAD_LEFT_THUMB    )|
				xim.ButDown(i,GAMEPAD_RIGHT_THUMB   )|
				xim.ButDown(i,GAMEPAD_LEFT_SHOULDER )|
				xim.ButDown(i,GAMEPAD_RIGHT_SHOULDER)|
				xim.ButDown(i,GAMEPAD_A             )|
				xim.ButDown(i,GAMEPAD_B             )|
				xim.ButDown(i,GAMEPAD_X             )|
				xim.ButDown(i,GAMEPAD_Y             ))
			{
				inactivityTime = 0.f;
			}
		}
    }

#endif  //  IS_OS_WINDOWS


    if ( GGame && GGame->GetPlayerShip() )
        GGame->GetPlayerShip()->SetCurrentControls(ctrl);
    if ( GGame && GGame->GetPlayerShip(1) )
        GGame->GetPlayerShip(1)->SetCurrentControls(ctrl2);

	if (validatePressed)
	{
		soloEnter = true;
		Menus::Validate( validatePressed - 1 );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// root Object
typedef struct rootObject_t : public serialisableObject_t
{
    SERIALIZABLE(rootObject_t,"Root of editing tree.")
    rootObject_t()
    {
        mRenderSettings = &RenderSettings;
        mGameSettings = &GameSettings;
        //mShipParameters = &ShipParameters;
    }
    virtual ~rootObject_t() {}

    renderSettings_t *mRenderSettings;
    gameSettings_t *mGameSettings;
    //shipParameters_t *mShipParameters;
    track_t mTracks[MAX_NB_TRACKS];
} rootObject_t;

serializableField_t rootObject_t::mSerialisableFields[] = {SED(rootObject_t,mRenderSettings,"Rendering and shaders"),
                                                           SED(rootObject_t,mGameSettings,"Game and content values"),
                                                           //SED(rootObject_t,mShipParameters,"Ship Physics"),
                                                           SED(rootObject_t,mTracks,"The tracks.")
                                                           //SED(rootObject_t,Sounds,"All the sounds")
                                                         };
NBFIELDS(rootObject_t);

rootObject_t GRootObject;
serialisableObject_t *GRootSerialisableObjectPtr = &GRootObject;
track_t *Tracks = GRootObject.mTracks;

track_t *GetTrack0() { return &GRootObject.mTracks[0]; }
//std::vector<soundEntry_t >* GSounds = &GRootObject.Sounds;

///////////////////////////////////////////////////////////////////////////////////////////////////

bool IsScreenResolutionSupported( const int Width, const int Height );
bool GetDefaultScreenResolution( int& Width, int& Height );
void ChangeVideoMode( const int Width, const int Height, bool bFS, bool bEditMode );

#define WINDOWNAME ".the rush//"

void computeVisibleSurface(int Width, int Height)
{
	SCREENWIDTH = WIDTH = (float)Width;
	extern bool GBEditorInited;

	if ( GBEditorInited )
		HEIGHT = SCREENHEIGHT = (float)Height;
	else
		HEIGHT = (float)WIDTH * 0.5625f;

	SCREENHEIGHT = (float)Height;
}

void InitVideoMode( int Width, int Height, bool bFS, bool editMode )
{
    ASSERT_GAME( Width > 0 && Height > 0 );

    if ( IsScreenResolutionSupported( Width, Height ) == false )
    {
        debug_printf("InitVideoMode with unsupported screen resolution: w(%d), h(%d)", Width, Height);

        GetDefaultScreenResolution( Width, Height );
        ASSERT_GAME( IsScreenResolutionSupported( Width, Height ) );

        debug_printf("Instead using default screen resolution: w(%d), h(%d)", Width, Height);
    }

    const int bitsPerPixel = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
    SDL_SetVideoMode( Width, Height, bitsPerPixel, (editMode?SDL_RESIZABLE:0) );

    ChangeVideoMode( Width, Height, bFS, editMode );
}

void ChangeVideoMode( const int Width, const int Height, bool bFS, bool editMode )
{
    ASSERT_GAME( Width > 0 && Height > 0 );
    ASSERT_GAME( IsScreenResolutionSupported( Width, Height ) );

    const int sdl_flags = SDL_OPENGL | (bFS? SDL_FULLSCREEN : 0) | (editMode?SDL_RESIZABLE:0);
    const int bitsPerPixel = 32;    //SDL_GetVideoInfo()->vfmt->BitsPerPixel;
    if( SDL_SetVideoMode( Width, Height, bitsPerPixel, sdl_flags ) == 0 )
    {
    }

    config_t newConfig = GetEngineConfig();
    newConfig.desiredWidth = Width;
    newConfig.desiredHeight = Height;
    newConfig.fullscreen = bFS?1:0;

    SetEngineConfig( newConfig );

    computeVisibleSurface(Width, Height);

#if EDITOR_ENABLE
    extern bool GBEditorInited;
#else
    bool GBEditorInited = false;
#endif
    if ( bFS && (!GBEditorInited) )
        SDL_ShowCursor(SDL_DISABLE);
    else
        SDL_ShowCursor(SDL_ENABLE);
}

static bool GBChangeVideoModeAfterBufferSwap = false;
void ChangeVideoModeAfterBufferSwap()
{
    GBChangeVideoModeAfterBufferSwap = true;
}

std::vector <resolution_t> resolutions;

const std::vector<resolution_t>& GetSupportedScreenResolutions()
{
    return resolutions;
}

bool IsScreenResolutionSupported( const int Width, const int Height )
{
    bool bIsSupported = false;

    const u32 resolutionCount = resolutions.size();
    for (u32 i = 0; i < resolutionCount; ++i)
    {
        const resolution_t& res = resolutions[i];
        if ( res.Width == Width && res.Height == Height )
        {
            bIsSupported = true;
            break;
        }
    }

    return bIsSupported;
}

bool GetDefaultScreenResolution( int& Width, int& Height )
{
    //NOTE: selecting the largest supported screen resolution

    int defaultWidth = 0;
    int defaultHeight = 0;

    const u32 resolutionCount = resolutions.size();
    for (u32 i = 0; i < resolutionCount; ++i)
    {
        const resolution_t& res = resolutions[i];
        if ( res.Width >= defaultWidth && res.Height >= defaultHeight )
        {
            defaultWidth = res.Width;
            defaultHeight = res.Height;
        }
    }

    const bool bValidDefaultScreenRes = ( defaultWidth > 0 ) && ( defaultHeight > 0 );
    if ( bValidDefaultScreenRes )
    {
        Width = defaultWidth;
        Height = defaultHeight;
    }

    return bValidDefaultScreenRes;
}

void buildResolutionsList()
{
    SDL_Rect** modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

	/* Check if there are any modes available */
	if (modes == (SDL_Rect**)0) {
		//printf("No modes available!\n");
		exit(-1);
	}

	/* Check if our resolution is restricted */
	if (modes == (SDL_Rect**)-1) {
		//printf("All resolutions available.\n");
		return ;
	}
	else{
		/* Print valid modes */
		//printf("Available Modes\n");
		//int NbModes = 0;
        u32 nextModeIdx = 0;
        const SDL_Rect* nextMode = modes[nextModeIdx];
        while ( nextMode != NULL )
        {
            int width = nextMode->w;
            int height = nextMode->h;

            char tmps[512];
#if IS_OS_WINDOWS
            sprintf_s (tmps, 512, "%dx%d", width, height);
#else
            sprintf(tmps, "%dx%d", width, height);
#endif
            LOG("Add resolution %s\n", tmps );
            resolutions.push_back(resolution_t(width, height, tmps));

            nextMode = modes[++nextModeIdx];
            //NbModes++;//printf("  %d x %d\n", modes[i]->w, modes[i]->h);
        }
        //return NbModes;
	}

#if 0
	int ind = 0;
	int awidth = -1, aheight = -1;
	for (int rs = sf::VideoMode::GetModesCount ()-1;rs>=0; rs--)
	{
		if (sf::VideoMode::GetMode(rs).BitsPerPixel != 32)
			continue;

		char tmps[512];
		unsigned int width, height;
		//GDD->GetResolution(rs, width, height);
		width = sf::VideoMode::GetMode(rs).Width;
		height = sf::VideoMode::GetMode(rs).Height;
		if ((awidth != width) || (aheight != height))
		{
			awidth = width;
			aheight = height;
#if IS_OS_WINDOWS
			sprintf_s (tmps, 512, "%dx%d", width, height);
#else
            sprintf(tmps, "%dx%d", width, height);
#endif
			LOG("Add resolution %s\n", tmps );
			if ((width == GConfig.desiredWidth)&&(height == GConfig.desiredHeight))
				activeResolution = ind;

			resolutions.push_back(resolution_t(width, height, tmps));
			ind ++;
		}
	}
#endif
}



///////////////////////////////////////////////////////////////////////////////////////////////////

config_t GConfig;

const config_t& GetEngineConfig()
{
    return GConfig;
}

void SetEngineConfig( const config_t& Config )
{
    GConfig = Config;
}

bool IsValidEngineConfig( const config_t& Config )
{
    bool bIsValid = IsScreenResolutionSupported( Config.desiredWidth, Config.desiredHeight );
    bIsValid = bIsValid && ( Config.fullscreen == 0 || Config.fullscreen == 1 );
    bIsValid = bIsValid && ( Config.AAactive == 0 || Config.AAactive == 1 );
    bIsValid = bIsValid && ( Config.RenderShadows == 0 || Config.RenderShadows == 1 );
    bIsValid = bIsValid && ( Config.RenderOcean == 0 || Config.RenderOcean == 1 );
    bIsValid = bIsValid && ( Config.RenderReflection == 0 || Config.RenderReflection == 1 );

    return bIsValid;
}

bool AreEngineConfigEquals( const config_t& ConfigA, const config_t& ConfigB )
{
    const int compareResult = memcmp( &ConfigA, &ConfigB, sizeof(config_t) );

    return (compareResult == 0);
}

#if IS_OS_MACOSX
std::string GetMacUserPrefs();
#endif

void readConfig()
{
	// defaults config
    config_t defaultConfig;
    defaultConfig.fullscreen = 1;
    defaultConfig.desiredWidth = 1280;
    defaultConfig.desiredHeight = 720;
    defaultConfig.fxVolume = 0.8f;
    defaultConfig.musicVolume = 0.8f;
    defaultConfig.AAactive = true;
    defaultConfig.RenderShadows = true;//ShouldRenderShadows();
    defaultConfig.RenderOcean = true;//ShouldRenderOcean();
    defaultConfig.RenderReflection = true;//ShouldRenderReflection();
	defaultConfig.mouseSensibility = 1.f;

    SetEngineConfig( defaultConfig );

	// defaults profil

	Solo::CreateProfil();

	std::string configDirectory = GetHomeDirectoy();
#if IS_OS_MACOSX
	configDirectory = GetMacUserPrefs();
	configDirectory += "/";
#elif IS_OS_LINUX
    configDirectory += "/";
#endif

	FILE *fp;

	// config
	std::string configName = configDirectory + "therush/config.cfg";
	LOG("Reading config %s\n", configName.c_str() );
	fp = fopen(configName.c_str(), "r");
	if (fp)
	{
		fread(&GConfig, sizeof(config_t), 1, fp);
		fclose(fp);
		LOG("Done\n");
	}

	// profil
	std::string profilName = configDirectory + "therush/profil.cfg";
	LOG("Reading profile %s \n", profilName.c_str() );
	fp = fopen(profilName.c_str(), "r");
	if (fp)
	{
		memset(&Solo::playerProfil, 0, sizeof(profil_t) );
		fread(&Solo::playerProfil, sizeof(profil_t), 1, fp);
		fclose(fp);
		LOG("Done\n");
	}
}


void writeConfig()
{
    ASSERT_GAME( IsValidEngineConfig(GConfig) );

	// write
	std::string configDirectory = GetHomeDirectoy();
#if IS_OS_MACOSX
	configDirectory = GetMacUserPrefs();
	configDirectory += "/";
#elif IS_OS_LINUX
	configDirectory += "/";
#endif
	std::string dirName = configDirectory + "therush";

#if IS_OS_WINDOWS
	_mkdir(dirName.c_str());
#elif IS_OS_MACOSX
	/*int err = */mkdir(dirName.c_str(), 0777);
	/*
	if (err)
	{
		LOG(" Err %d\n", errno);
	}
	else {
		LOG(" OK\n");
	}
	*/
#else
/*int err =*/ mkdir(dirName.c_str(), 01777);
#endif

	// config
	std::string configName = configDirectory + "/therush/config.cfg";
	LOG("Writing Config %s \n", configName.c_str() );
	FILE *fp;
	fp = fopen(configName.c_str(), "wb");
	if (fp)
	{

		fwrite(&GConfig, sizeof(config_t), 1, fp);
		fclose(fp);
		LOG("Done\n");
	}

	// profil
	std::string profilName = configDirectory + "/therush/profil.cfg";
    LOG("Writing Config %s \n", profilName.c_str() );
	fp = fopen(profilName.c_str(), "wb");
	if (fp)
	{
		fwrite(&Solo::playerProfil, sizeof(profil_t), 1, fp);
		fclose(fp);
		LOG("Done\n");
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////

void testUnit1();
void testUnit2();
void testUnit6();



void InitMainGame()
{
    camera.project(90.0f, WIDTH/HEIGHT, 0.1f, 1000.0f);
    camera.view(vec(4,1,4), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));
	camera2.project(90.0f, WIDTH/HEIGHT, 0.1f, 1000.0f);
	camera2.view(vec(100,100,100), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));

    Game::CreateNewGameSolo( 0, 3 );

    LOG("Main started\n");

#if IS_OS_WINDOWS
    g_seed = timeGetTime();//static_cast<int>(GetElapsedTime() * 12312.123f);
#else
    g_seed = 0;
#endif

#ifdef _DEBUG
    track_t* intoTrack = &Tracks[10];
#else
    track_t* intoTrack = &Tracks[fastrand()&0x1F];
#endif
    track.GoWithTrack( intoTrack );

    Renderer::setRenderMeshStack(GetMeshStack());

    GGame->ClearFlagPropsDirtyWorld();
    GGame->ClearFlagPropsDirtyNetwork();

#ifdef _DEBUG
    Menus::Show( MENU_MAIN );
#else
    Menus::Show( MENU_INTRO );

    switch ( intoTrack->mEnvironmentIndex )
    {
    case 0:
        PlaySequence( introSceneArctic );
        break;
    case 1:
        PlaySequence( introSceneCity );
        break;
    case 2:
        PlaySequence( introSceneDesert );
        break;
    case 3:
        PlaySequence( introSceneSpace );
        break;
    }
#endif

    LOG("Rock&Roll!\n");
}

void InitGame( int gameType )										// All Setup For OpenGL Goes Here
{
    PROFILER_START(InitGame);

	LOG("Init Game (%d):: Player named %s signed-in.\n", gameType, GetUserName().c_str());

    Solo::DistributeAllLevels( );

    Menus::Init();

#if defined(RETAIL)
    InitMainGame();
#else
	switch (gameType)
	{
	case 0: // blocs
		testUnit1();
		break;
    case 2: // features full track
        testUnit2();
        break;
	case 5:
        InitMainGame();
		break;
#if EDITOR_ENABLE
    case 8:
        InitEditor();
        break;
#endif
	}
#endif

    LOG("InitGame Done\n");

    PROFILER_END(); // InitGame
}

///////////////////////////////////////////////////////////////////////////////////////////////////


int TickAndDrawScene(float ate)									// Here's Where We Do All The Drawing
{
	PROFILER_START(TickAndDrawScene);

    NetClient->Tick( ate );
    NetServer->Tick( ate );



	camera.computeMatricesAndFrustums();
	if (splitScreenEnabled)
		camera2.computeMatricesAndFrustums();


	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	glLoadMatrixf(camera.mProjection.m16);
	//gluPerspective(90.0f, (GLfloat)Width/(GLfloat)Height, 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	glLoadMatrixf(camera.mView.m16);

	glClearDepth(1.f);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glClear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	// Clear The Screen And The Depth Buffer

    GWorldTime += ate * world.GetGameSpeed();

        static float aFixedTimeStepLocalTime = 0.f;
        static const float UPDATE_FREQ = FREQ60Hz;

        aFixedTimeStepLocalTime += ate * world.GetGameSpeed();
        //aFixedTimeStepLocalTime += UPDATE_FREQ;//
        while ( aFixedTimeStepLocalTime >= UPDATE_FREQ )
        {
            camera.tick(UPDATE_FREQ);
			if (splitScreenEnabled)
				camera2.tick(UPDATE_FREQ);
            if (GGame)
                GGame->Tick( UPDATE_FREQ );
            physicWorld.tick( UPDATE_FREQ );

            aFixedTimeStepLocalTime -= UPDATE_FREQ;
        };
        if (GGame)
        {
            GGame->Interpolate( aFixedTimeStepLocalTime*(1.f/UPDATE_FREQ) );
            camera.tick(0.001f);
			if (splitScreenEnabled)
				camera2.tick(0.001f);	
            GGame->TickForRender( ate );
        }
        TickLanDiscovery( ate );
        Menus::Tick( ate );
        Ribbon::TickAll(world.GetGameSpeed() * ate );

    TickSequencePlayback( ate );
	ggui.tick( ate );

	updateAnimations( ate );

#if EDITOR_ENABLE
    TickEditor( ate );
#endif

	if (splitScreenEnabled)
	{
		// billboard/splitscreen
		if ( GGame && GGame->GetPlayerShip(1) && camera.GetMode() == CM_BEHINDSHIP )
			Renderer::AddBillBoard( GGame->GetPlayerShip(1)->GetPhysics()->GetTransform().position + vec(0.f,0.33f,0.f), 0.5f, textures[TEXTURE_PLAYER2]);
		Renderer::Render( ate, &camera, 1 );
		Audio::SetListener( camera.mViewInverse, ate );
		Audio::Tick( ate );
		if ( GGame && GGame->GetPlayerShip(0) && camera2.GetMode() == CM_BEHINDSHIP  )
			Renderer::AddBillBoard( GGame->GetPlayerShip(0)->GetPhysics()->GetTransform().position + vec(0.f,0.33f,0.f), 0.5f, textures[TEXTURE_PLAYER1]);
		Renderer::Render( ate, &camera2, 2 );
	}
	else
	{
		
		Renderer::Render( ate, &camera, 0 );
		Audio::SetListener( camera.mViewInverse, ate );
		Audio::Tick( ate );
	}
	Renderer::endOfRendering( ate, &camera, splitScreenEnabled );

#if EDITOR_ENABLE
    DrawEditor( ate );
#endif

	PROFILER_END(); // TickAndDrawScene
	return true;										// Keep Going
}

///////////////////////////////////////////////////////////////////////////////////////////////////
volatile bool EverythingIsInited = false;
volatile int MiniPhase = 0;
volatile float ThingsInited = 0.f;
void GameBaseInit( void * arg)
{
	buildMemoryFilesFromDirectory( "Datas/" );
	ThingsInited++;
    StartThreadedInits();
	ThingsInited++;
	initTracks();
	ThingsInited++;

    LOG(" Reading JSON Content ... ");
    std::string jsonstr = FileToString( "Datas/rushContent.json" );
    if (!jsonstr.empty())
    {
        JSONSerializer jsonser;
        jsonser.ParseJSON( GRootSerialisableObjectPtr, jsonstr.c_str(), jsonstr.length() );
        LOG(" JSON Parsed.\n");
    }

	MiniPhase = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool gHasFocus = true;
void distributeAllLevels( );

#if IS_OS_WINDOWS

INT WINAPI WinMain(HINSTANCE Instance, HINSTANCE, LPSTR lpCmdLine, INT cmdl)
#else
int main(int argc, char **argv)
#endif
{
    //Only supporting 32bit platforms
#if IS_OS_WINDOWS
    COMPILE_TIME_ASSERT( sizeof(lpCmdLine) == 4 );
#else
    COMPILE_TIME_ASSERT( sizeof(argv) == 4 );
#endif


    PROFILER_ENABLE;


	int gameType = 0;

	// command line desactivation
	if (strstr(lpCmdLine, "noAA"))
		AADesactivatedByCommandLine = true;
	if (strstr(lpCmdLine, "noRefl"))
		ReflDesactivatedByCommandLine = true;
	if (strstr(lpCmdLine, "noShadows"))
		ShadowsDesactivatedByCommandLine = true;

#ifndef RETAIL
    startWeb( NULL );
#if IS_OS_WINDOWS
	if (strstr(lpCmdLine, "main"))
		gameType = 5;
	if (strstr(lpCmdLine, "edit"))
		gameType = 8;
#else
    gameType = 5;
	if (argc>=2)
	{
        gameType = 5;// main
	}
#endif
#else
	gameType = 5;
#endif

#ifndef RETAIL
#if IS_OS_WINDOWS
    bool bSaveScreen = ( strstr( lpCmdLine, "-savepictures" ) != NULL);
    GBRecordKeysIsActive = ( strstr( lpCmdLine, "-recordkeys" ) != NULL);
    GBPlaybackKeysIsActive = ( strstr( lpCmdLine, "-playkeys" ) != NULL);

    InitRecordKeys();
    InitPlayBackKeys();
#endif
#endif



	// SDL Init
    if( SDL_Init( SDL_INIT_VIDEO|SDL_INIT_JOYSTICK  ) < 0 || !SDL_GetVideoInfo() )
		return 0;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	// Center window
	char env[] = "SDL_VIDEO_CENTERED=1";
#if IS_OS_WINDOWS
	_putenv(env);
#else
    putenv(env);
#endif
	SDL_Surface *iconImage = SDL_LoadBMP("therush.bmp");
	SDL_SetColorKey(iconImage, SDL_SRCCOLORKEY, 0xFFFFFF); 
    SDL_WM_SetIcon(iconImage, NULL);
   
    SDL_WM_SetCaption( WINDOWNAME, 0 );

	// music
	AudioMusic::InitMusic();
	    
	AudioMusic::BuildSongsList();




    // Create the main window
    readConfig();
    buildResolutionsList();

    const config_t& config = GetEngineConfig();
    int screenWidth = config.desiredWidth;
    int screenHeight = config.desiredHeight;
    bool isFS = (config.fullscreen != 0)&&(gameType != 8);
#ifndef RETAIL
#if IS_OS_WINDOWS
    if (bSaveScreen)
    {
        screenWidth = 1920;
        screenHeight = 1080;
        isFS = true;
        GFixedTimeStamp = 0.01668f;//0.016666f;
    }
#endif
#endif

    InitVideoMode( screenWidth, screenHeight, isFS,(gameType == 8) );
    LOG("Init render @ %5.0f x %5.0f in screen of @ %5.0f x %5.0f\n", WIDTH, HEIGHT, SCREENWIDTH, SCREENHEIGHT);

	LOG("Window Inited\n");
	glViewport( 0, 0, (int)WIDTH, (int)HEIGHT );
	// base init
	ggui.init();
	ggui.setResolution(64, 36);

	tthread::thread *gameBaseInit = new  tthread::thread( GameBaseInit, lpCmdLine );
	

	guiProgress progress;
	extern shader_t textRenderingShader;
	textRenderingShader.LoadShader( FileToString( "Datas/Shaders/text.vs" ).c_str(), FileToString( "Datas/Shaders/text.fs" ).c_str() );
	float progressValue = 0.f;
	while (!EverythingIsInited)
	{
		
		glClearColor(0.5f, 0.5f, 0.5f, 0.f);
		glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		
		tthread::this_thread::sleep_for(tthread::chrono::milliseconds(50));
		//ggui.clearText();
		ggui.putText(0, 19, "Loading...");
		progress.setFrame(0,18,64,1);
		progress.setValue( progressValue );
		progress.draw( &ggui );
		
		switch(MiniPhase)
		{
		case 0:
			progressValue = ThingsInited/20.f;
			ggui.putText(44, 17, "Generating gizmos    ");
			break;
		case 1:
			Audio::Init();
			MiniPhase++;
			progressValue = 0.5f;
			ggui.putText(44, 17, "Processing karma     ");
			break;
		case 2:
			Renderer::Init();
			MiniPhase++;
			progressValue+= 0.1f;
			ggui.putText(44, 17, "Dekrunching textures ");
			break;
		case 3:
			InitFX();
			MiniPhase++;
			progressValue+= 0.1f;
			ggui.putText(44, 17, "Compiling Lisp       ");
			break;
		case 4:
			InitMeshes();
			MiniPhase++;
			progressValue+= 0.1f;
			ggui.putText(44, 17, "Debugging lamp       ");
			break;
		case 5:
			physicWorld.init();
			MiniPhase++;
			progressValue+= 0.1f;
			ggui.putText(44, 17, "Unswizzling register ");
			break;
		case 6:
			initTracksMiniMesh();
			
			MiniPhase = 10;
			progressValue = 1.0f;
			ggui.putText(44, 17, "Compacting worlds    ");
			break;
		case 10:
			EverythingIsInited = true;
			ggui.putText(44, 17, "Linking features     ");
			Renderer::newVelvet( vec(1.f ), 0 );
			break;
		}
		
		ggui.tick( 0.05f );
		ggui.draw( 0.05f );

		extern GLuint mGuiTexture;
		textRenderingShader.Bind();
		glBindTexture( GL_TEXTURE_2D, mGuiTexture );
		glDisable(GL_BLEND);
		DrawFullScreenQuad( );
		
		SDL_Event event;
		while(SDL_PollEvent(&event)) {}
		if (event.type == SDL_VIDEORESIZE)
			computeVisibleSurface( event.resize.w, event.resize.h );

		SDL_GL_SwapBuffers( );
	}
    delete gameBaseInit;

	ggui.clearText();
	
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	// base init
    PushMeshStack();

	InitGame(gameType);

	AudioMusic::PlayRandomSong();
	AudioMusic::SetMusicVolume( GetEngineConfig().musicVolume );

	int nbFrameRendered = 0;
	float timeRenderingFrames = 0.f;


    LOG(" *** Memory stats\n");
    LOG("Sizeof(Game) = %d\n", static_cast<int>(sizeof(Game)));
    LOG("Sizeof(World) = %d\n", static_cast<int>(sizeof(World)));
    LOG("Sizeof(GameShip) = %d\n", static_cast<int>(sizeof(GameShip)));
    LOG("Sizeof(ZShipPhysics) = %d\n", static_cast<int>(sizeof(ZShipPhysics)));
    LOG("Sizeof(PhysicWorld) = %d\n", static_cast<int>(sizeof(PhysicWorld)));
    LOG("Sizeof(mesh_t) = %d\n", static_cast<int>(sizeof(mesh_t)));
    LOG("Sizeof(ZIndexArrayOGL) = %d\n", static_cast<int>(sizeof(ZIndexArrayOGL)));
    LOG("Sizeof(ZVertexArrayOGL) = %d\n", static_cast<int>(sizeof(ZVertexArrayOGL)));
    LOG("Sizeof(GameShip::shipProps_t) = %d\n", static_cast<int>(sizeof(GameShip::shipProps_t) ));
    LOG(" *** %d ArrayPools active with a total of %d bytes\n", GetNumberOfArrayPools(), GetNumberOfBytesInArrayPools() );

    // wait for the init threads
    WaitForThreadedInits();

	LOG("Starting Game Loop\n");

    Uint32 prevTicks = SDL_GetTicks();
	// Start game loop
    while (!GQuitAsked)//App.IsOpened())
    {
		PROFILER_START(MainLopp);
        // Process events

        Uint32 currentTicks = SDL_GetTicks();
		float Time = static_cast<float>( currentTicks  - prevTicks ) * 0.001f;
#ifndef RETAIL
		if (GFixedTimeStamp > FLOAT_EPSILON)
			Time = GFixedTimeStamp;
#endif

		
		tickInputs(Time);

		nbFrameRendered++;
		timeRenderingFrames += Time;
		if (timeRenderingFrames > 1.f)
		{
			float currentNbFPS = ((float)nbFrameRendered)/timeRenderingFrames;
			timeRenderingFrames = 0.f;
			nbFrameRendered = 0;
#ifndef RETAIL
#if IS_OS_WINDOWS
        if (!bSaveScreen)
#endif

        {
			char tmps[512];
            snprintf( tmps, sizeof(tmps), "%5.2f FPS", currentNbFPS);
            //ggui.putText( 0,0/*64-strlen(tmps),35*/, tmps );
        }
#endif
		}
#ifndef RETAIL
		if (GBDrawCameraPosition)
		{
			char tmps[512];
			snprintf( tmps, sizeof(tmps), "X:%5.2f Y:%5.2f Z:%5.2f",
				camera.mViewInverse.position.x, camera.mViewInverse.position.y, camera.mViewInverse.position.z );
			ggui.putText( 0,33/*64-strlen(tmps),35*/, tmps );
		}
#endif
		inactivityTime += Time;
		TickAndDrawScene(Time);


		Renderer::Tick( Time );
		AudioMusic::TickMusic();
		

		PROFILER_END(); // MainLopp

#ifndef RETAIL
#if IS_OS_WINDOWS
    if (bSaveScreen)
    {
        static int screenNb = 0;
        char tmpsfin[512];
        sprintf_s( tmpsfin, sizeof(tmpsfin), "recPictures/pic%06d.png", screenNb ++ );
        Renderer::DoSaveScreenShot( tmpsfin );
    }
#endif
#endif

        // Finally, display rendered frame on screen
        PROFILER_START(FrameSwap);

        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
			if ( event.type == SDL_KEYDOWN || event.type == SDL_KEYUP )
			{
				inactivityTime = 0.f;
			}
			else
			if (event.type == SDL_VIDEORESIZE)
			{
				computeVisibleSurface( event.resize.w, event.resize.h );
			}
            else if (event.type == SDL_QUIT)
			{
				GQuitAsked = true;
			}
			else if (event.type ==  SDL_MOUSEBUTTONUP)
			{
				if (event.button.button == 4 )
				{
					GInputGlobalSpeed += 0.1f;
				}
				else if (event.button.button == 5 )
				{
					GInputGlobalSpeed -= 0.1f;
				}
				GInputGlobalSpeed = Clamp( GInputGlobalSpeed, 0.f, 2.f);
            }          
        }

        SDL_GL_SwapBuffers( );

        if ( GBChangeVideoModeAfterBufferSwap )
        {
            GBChangeVideoModeAfterBufferSwap = false;

            const config_t& config = GetEngineConfig();
			extern bool GBEditorInited;
            ChangeVideoMode( config.desiredWidth, config.desiredHeight, (config.fullscreen != 0), GBEditorInited );
            Renderer::Reset();
        }

        prevTicks = currentTicks;
		PROFILER_END(); // FrameSwap
    }
    PROFILER_START(UninitGame);

	LOG("Leaving Game Loop\n");
    LogProfiler();

	Game::DestroyGame();

#ifndef RETAIL
#if IS_OS_WINDOWS
	// json write
    {
        JSONSerializer jsonser;
        StringToFile( "Datas/rushContent.json", jsonser.GenerateJSON( GRootSerialisableObjectPtr ) );
    }
#endif
#endif

    // config
	writeConfig();

    // uninit components
	UninitFX();
	physicWorld.uninit();
    Audio::Uninit();

    // back on engine/base meshes
	PopMeshStack();
	ClearMeshStack(); // Mesh createdat init time
    Renderer::clearAllLights();
	Renderer::portals.clear();
    ClearFlashLights();
    Renderer::CheckIndexVertexArrayInstanceCount();

	AudioMusic::UninitMusic();

    PROFILER_END(); //UninitGame

    PROFILER_END();

	LOG("Exit Success\n");


    return EXIT_SUCCESS; // Game
}


void SteamMusic( const char *szFileName);
void PlayPauseMusic();
void SetMusicVolume( float volume );