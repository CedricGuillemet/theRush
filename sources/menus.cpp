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
#include "menus.h"

#include "gui.h"
#include "therush.h"
#include "render.h"
#include "solo.h"
#include "game.h"
#include "track.h"
#include "bonus.h"
#include "camera.h"
#include "ZShipPhysics.h"
#include "net.h"
#include "world.h"
#include "content.h"
#include "audio.h"
#include "physics.h"

#include "include_SDL.h"

MENU_IDS BackFromGameMenu;
server_t JoinServerStruct;
void writeConfig();
float padlx = 0.f, padly = 0.f;
bool xPOVLeft = false, xPOVRight = false;
extern float inactivityTime;
extern bool splitScreenEnabled;
int splitValidation = 0;
bool CreditsBackActive = false;

///////////////////////////////////////////////////////////////////////////////////////////////////

bool ControlLeft() 
{
	return ( KeyLeftDown() || POVLeft() || (padlx < -FLOAT_EPSILON) || xPOVLeft );
}

bool ControlRight()
{
	return ( KeyRightDown() || POVRight() || (padlx > FLOAT_EPSILON) || xPOVRight  );
}

bool ControlLeftReleased()
{
	return ( keysReleased(SDLK_LEFT) || POVLeftReleased() || xPOVLeft );
}

bool ControlRightReleased()
{
	return ( keysReleased(SDLK_RIGHT) || POVRightReleased() || xPOVRight );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
0 : main menu
1 : want to qui game
3 : setup from main menu
4 : stats
*/

static photoModeParams_t photoModeParams;
std::string screenShotFileName;
extern gui ggui;

static MENU_IDS mCurrentMenu = MENU_NONE;
MENU_IDS Menus::GetCurrent()
{
	return mCurrentMenu;
}

static int iServerBrowseNumPage;
static int iServerBrowsePagesCount;


// photo mode
guiCombo photoShip;
guiSlider photoDOF, photoFocal, photoHeight, photoLateral, photoRoll;
guiElt *photoElts[] = { &photoShip, &photoDOF, &photoFocal, &photoHeight, &photoLateral, &photoRoll };

// for config
u32 menuShowScreenResCount = 0;
config_t menuConfigBackup;
guiCombo setupResolution, setupFullscreen;
guiSlider setupMusicVol, setupFXVol;
guiCombo setupAA, setupShadows, setupOcean, setupReflection;
guiElt *setupElts[] = {&setupResolution, &setupFullscreen, &setupMusicVol, &setupFXVol, &setupAA, &setupShadows, &setupOcean, &setupReflection };

guiCombo quickMode, quickTrack, quickNumberOfLaps;
guiElt *quickElts[] = { &quickTrack, &quickNumberOfLaps, &quickMode };

// loading
guiProgress loadingProgress;

enum { E_Menu_Resolution_Count_Max = 128 };
const char *resos[E_Menu_Resolution_Count_Max] = {NULL};
const char *yesno[]={"no", "yes"};
const char *countText[]={"1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };

char *oneTo99[99];
const char *tracksNames[NBTracks];
const char **gameModesNames;


shipIdentifier_t currentShipIdentifiterSelected, currentShipIdentifiterSelected2;

void GuiToShipId( shipIdentifier_t& shipId )
{
	UNUSED_PARAMETER(shipId);
	/*
	shipId.mShipColor = namedColors[selShipBodyColor.currentItem].value;
	shipId.mMeshesID = (selShipNose.currentItem<<24) + (selShipMiddle.currentItem<<8) + (selShipBack.currentItem) + (selShipSide.currentItem<<16);
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void StartGameSolo( unsigned int gameMode = 0)
{
	GGame->Restart( gameMode );

	world.SetGameSpeed( 1.f );
	//track.progressMesh( 0.f, -1.f );
	setDOFBlur( 0.f );

	// go game in solo
	const int shipCountSolo = Game::E_Max_Ship_Count;
	for ( int i = 0 ; i < shipCountSolo ; ++i )
	{
		matrix_t mt;
		mt = track.GetSpawnMatrix( i );
		mt.position += mt.up * 1.5f;//ShipParameters.mEquilibriumHeight;//... 1.f;
		mt.position.w = 1.f;

		GameShip *ps = (GameShip *)GGame->Spawn(GO_Ship, mt, NULL );


		if (!i)
		{
			camera.SetCameraBehindShip( ps );
			GGame->SetPlayerShip( ps );
			ps->SetShipIdentifier( currentShipIdentifiterSelected );
		}
		else
		{
			ps->SetBot( 0.9f + static_cast<float>(i)*0.1f );
			shipIdentifier_t shipID;
			shipID.SetRandom();
			ps->SetShipIdentifier( shipID );
		}

		ps->mProps.ctrlEnabled = false;
	}

	Menus::Show(MENU_EMPTY_SOLO);
	GGame->SetState(GameStateCountDown);
}

void StartGameSplit( unsigned int gameMode = 0)
{
	splitScreenEnabled = true;
	GGame->Restart( gameMode );

	world.SetGameSpeed( 1.f );
	//track.progressMesh( 0.f, -1.f );
	setDOFBlur( 0.f );

	// go game in solo
	const int shipCountSolo = Game::E_Max_Ship_Count;
	for ( int i = 0 ; i < shipCountSolo ; ++i )
	{
		matrix_t mt;
		mt = track.GetSpawnMatrix( i );
		mt.position += mt.up * 1.5f;//ShipParameters.mEquilibriumHeight;//... 1.f;
		mt.position.w = 1.f;

		GameShip *ps = (GameShip *)GGame->Spawn(GO_Ship, mt, NULL );


		if ( i == 0 )
		{
			camera.SetCameraBehindShip( ps );
			GGame->SetPlayerShip( ps, 0 );
			ps->SetShipIdentifier( currentShipIdentifiterSelected );
			ps->SetName("Player 1");
		}
		else if ( i == 1 )
		{
			camera2.SetCameraBehindShip( ps );
			GGame->SetPlayerShip( ps, 1 );
			ps->SetShipIdentifier( currentShipIdentifiterSelected2 );
			ps->SetName("Player 2");
		}
		else
		{
			ps->SetBot( 0.9f + static_cast<float>(i)*0.1f );
			shipIdentifier_t shipID;
			shipID.SetRandom();
			ps->SetShipIdentifier( shipID );
		}

		ps->mProps.ctrlEnabled = false;
	}

	Menus::Show(MENU_EMPTY_SOLO);
	GGame->SetState(GameStateCountDown);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void Menus::Init()
{
	LOG("InitMenus\n");

	for (int i = 0; i< 99; i ++)
	{
		oneTo99[i] = new char [3];
		sprintf(oneTo99[i], "%d", i+1);
	}
	for (unsigned int i=0;i<NBTracks;i++)
	{
		tracksNames[i] = Tracks[i].mName.c_str();
	}
	gameModesNames = new const char * [NBGameTypes];
	for (unsigned int i = 0;i<NBGameTypes ; i++)
	{
		gameModesNames[i] = GameTypes[i].szName;
	}

	const std::vector<resolution_t>& supportedScreenRes = GetSupportedScreenResolutions();

	const u32 screenResCount = supportedScreenRes.size();
	menuShowScreenResCount = zmin( screenResCount, E_Menu_Resolution_Count_Max );

	for (u32 i = 0; i < menuShowScreenResCount; ++i)
	{
		resos[i] = supportedScreenRes[i].textValue.c_str();
	}
	LOG("Done\n");
}



void drawEndResultPosition(int x, int y, int number)
{
	static const char *posEndintTexs[]= {TEXTURE_POS1,TEXTURE_POS2,TEXTURE_POS3,TEXTURE_POS4, TEXTURE_POS5, TEXTURE_POS6,TEXTURE_POS7,TEXTURE_POS8};
	ggui.addBitmap(gui::guiBitmap(vec ( (float)x, (float)y, 12.f, 12.f ), textures[ posEndintTexs[number-1] ] ) );
}


void PutBannerText()
{
	ggui.putText(10, 34, "   __  .__                                 .__     ");
	ggui.putText(10, 33, " _/  |_|  |__   ____   _______ __ __  _____|  |__  ");
	ggui.putText(10, 32, " \\   __\\  |  \\_/ __ \\  \\_  __ \\  |  \\/  ___/  |  \\ ");
	ggui.putText(10, 31, "  |  | |   Y  \\  ___/   |  | \\/  |  /\\___ \\|   Y  \\");
	ggui.putText(10, 30, " /\\__| |___|  /\\___  >  |__|  |____//____  >___|  /");
	ggui.putText(10, 29, " \\/         \\/     \\/                    \\/     \\/ ");
	ggui.putText(10, 28, "                            - B E T A O N E - ");
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void Menus::Show(MENU_IDS menuNumber)
{
	ggui.clearText();
	ggui.addSelector(0,0);
	ggui.setBlackBox(0, 0, 0, 36);
	enableShowFocalCursor(false);
	Renderer::SetOverGUIMesh( NULL );

	switch (menuNumber)
	{

	case MENU_HOWTOPLAY_SOLO:
	case MENU_HOWTOPLAY_NETWORK:
	case MENU_HOWTOPLAY:
		{
			ggui.setBlackBox(2, 0, 60, 36);
			ggui.putText(4,32, "solo/quick");

			ggui.addBitmap(gui::guiBitmap(vec(4, 25, 24, 6), textures[TEXTURE_HOWTO_KBD0] ) );
			ggui.addBitmap(gui::guiBitmap(vec(36, 25, 24, 6), textures[TEXTURE_HOWTO_PAD] ) );

			ggui.putText(4,20, "split/player 1                  split/player 2");

			ggui.addBitmap(gui::guiBitmap(vec(4, 13, 24, 6), textures[TEXTURE_HOWTO_KBD2] ) );
			ggui.addBitmap(gui::guiBitmap(vec(36, 13, 24, 6), textures[TEXTURE_HOWTO_KBD1] ) );

			ggui.addBitmap(gui::guiBitmap(vec(4, 6, 24, 6), textures[TEXTURE_HOWTO_PAD] ) );
			ggui.addBitmap(gui::guiBitmap(vec(36, 6, 24, 6), textures[TEXTURE_HOWTO_PAD] ) );

			ggui.putText(6,3, "  ok");

			int menuPos[]={GC(6,3) };
			ggui.addSelector(menuPos, 1);
		}
		break;
	case MENU_EVOLUTION_INFO:
		break;
	case MENU_EVOLUTION_ACQUIRED:
		break;

	case MENU_SEL_TEAM_QUICK:
	case MENU_SEL_TEAM_CAREER:
	case MENU_SEL_TEAM_CREATESERVER:
	case MENU_SEL_TEAM_JOINSERVER:
		{
			ggui.setBlackBox(16-15, 0, 30, 36);
			ggui.putText(18-15,27, ".choose your team");

			for ( unsigned int i = 0 ; i < 4 ;i ++ )
				ggui.putText( 20-15, 23 - i, GetTeamName( i ) );


			int menuPos[]={GC(18-15,23), GC(18-15,22), GC(18-15,21), GC(18-15, 20) };
			ggui.addSelector(menuPos, 4);

			ggui.putText(21-15, 7, "[Enter] to validate");

			// ship/camera
			GameShip *ps = GGame->GetShip(0);
			const shipIdentifier_t& shipId = ps->GetShipIndentifier();
			currentShipIdentifiterSelected = shipId;
			camera.SetSelectOneShip( GGame->GetShip(0) );

		}
		break;
	case     MENU_SEL_TEAM_SPLIT:
		{
			splitValidation = 0;
			ggui.setBlackBox(0, 22-20, 64, 12);
			ggui.putText(21,32-20, ".choose your team");
			ggui.putText(21, 24-20, "[Enter] to validate");


			for ( unsigned int i = 0 ; i < 4 ;i ++ )
			{
				ggui.putText( 4, 30 - i-20, GetTeamName( i ) );
				ggui.putText( 36, 30 - i-20, GetTeamName( i ) );
			}

			int menuPos[]={GC( 2,30-20), GC( 2,29-20), GC( 2,28-20), GC( 2, 27-20) };
			int menuPos2[]={GC( 34,30-20), GC( 34,29-20), GC( 34,28-20), GC( 34, 27-20) };
			ggui.addSelector(menuPos, 4, 0);
			ggui.addSelector(menuPos2, 4, 1);


			// ship/camera
			GameShip *ps = GGame->GetShip(0);
			const shipIdentifier_t& shipId = ps->GetShipIndentifier();
			currentShipIdentifiterSelected = shipId;
			camera.SetCameraEndRace( GGame->GetShip(0) );

			GameShip *ps2 = GGame->GetShip(1);
			const shipIdentifier_t& shipId2 = ps2->GetShipIndentifier();
			currentShipIdentifiterSelected2 = shipId2;
			camera2.SetCameraEndRace( GGame->GetShip(1) );

		}
		break;
	case MENU_SEL_EVOLUTION_QUICK:
	case MENU_SEL_EVOLUTION_CAREER:
	case MENU_SEL_EVOLUTION_CREATESERVER:
	case MENU_SEL_EVOLUTION_JOINSERVER:
		{
			ggui.setBlackBox(5, 0, 25, 36);

			ggui.putText(7, 32, ".select your ship");

			unsigned int teamIndex = currentShipIdentifiterSelected.mTeamIndex;
			unsigned int nbPoints = 255;//Solo::playerProfil.evolutions[ teamIndex ];

			int menuPos[32];

			for ( unsigned int i = 0 ; i < profil_t::GetEvolutionCount() ; i++ )
			{
				unsigned int pointNeeded = Solo::playerProfil.GetPointNeededForEvolution(i);
				char tmps[512];
				sprintf( tmps, "evolution %d %s", i+1, (nbPoints >= pointNeeded)?"":"(locked)" );
				ggui.putText( 8, 23-i, tmps );
				menuPos[ i ] = GC( 6, 23-i );
			}

			unsigned int currentEvolution = Solo::playerProfil.GetEvolutionIndex( teamIndex );

			//menuPos[ 0 ] = GC( 6, 26/*26 - profil_t::GetEvolutionCount() - 1*/ );

			ggui.addSelector(menuPos, currentEvolution + 1 );

			ggui.putText(10, 7, "[Enter] to validate");


		}
		break;
	case MENU_SEL_EVOLUTION_SPLIT:
		{
			splitValidation = 0;
			ggui.setBlackBox(0, 2, 64, 16);


			ggui.putText(21,16, ".select your ship");
			unsigned int teamIndex = currentShipIdentifiterSelected.mTeamIndex;
			unsigned int nbPoints = 255;//Solo::playerProfil.evolutions[ teamIndex ];

			int menuPos[32];
			int menuPos2[32];

			for ( unsigned int i = 0 ; i < profil_t::GetEvolutionCount() ; i++ )
			{
				unsigned int pointNeeded = Solo::playerProfil.GetPointNeededForEvolution(i);
				char tmps[512];
				sprintf( tmps, "evolution %d %s", i+1, (nbPoints >= pointNeeded)?"":"(locked)" );
				ggui.putText( 4, 14-i, tmps );
				ggui.putText( 36, 14-i, tmps );
				menuPos[ i ] = GC( 2, 14-i );
				menuPos2[ i ] = GC( 34, 14-i );
			}

			unsigned int currentEvolution = Solo::playerProfil.GetEvolutionIndex( teamIndex );

			ggui.addSelector(menuPos, currentEvolution + 1, 0 );
			ggui.addSelector(menuPos2, currentEvolution + 1, 1 );

			ggui.putText(21, 4, "[Enter] to validate");
		}
		break;

	case MENU_CONNECTION_LOST:
		ggui.clearBitmaps();
		SoundGui("ButtonError");
		NetClient->Close();
		ggui.setBlackBox(0, 10, 64, 6);
		ggui.putText(6,14, ".connection to server lost//");
		ggui.putText(6,13, ".sorry for the inconvenience");
		ggui.putText(6,11, "> ok");
		break;
	case MENU_CONNECTED_WAITING:
		ggui.setBlackBox(0, 12, 64, 13);
		ggui.putText( 20, 23, ".waiting for game to start //");
		ggui.putText( 20, 22, ".connected buddies");
		break;
	case MENU_WAITING_BUDDIES:
		ggui.setBlackBox(0, 12, 64, 12);
		ggui.putText( 20, 22, ".waiting for buddies");
		break;
	case MENU_INTRO:
		ggui.setBlackBox(0, 0, 0, 36);
		break;
	case MENU_LOBBY_JOINING:
		{
			char tmps[512];
			sprintf(tmps,".joining server // %s", JoinServerStruct.mURL.c_str() );
			ggui.putText(4,19, tmps);

			sprintf(tmps,"enjoy your %s on %s", GameTypes[JoinServerStruct.mMode].szName, Tracks[JoinServerStruct.mTrack].mName.c_str());
			ggui.putText(4,17, tmps);
			ggui.putText(48,16, "[ESC] to cancel");
			ggui.setBlackBox(0, 15, 64, 6);

			// player props

			shipIdentifier_t shipId;
			GuiToShipId( shipId );
			NetClient->SetPlayerInfos( GetUserName().c_str(), shipId );

			// go connect

			NetClient->ConnectTo(JoinServerStruct.mURL);
		}
		break;
	case MENU_LOBBY_JOINERROR:
		{
			SoundGui("ButtonError");

			NetClient->Close();
			ggui.setBlackBox(0, 10, 64, 6);
			char tmps[512];
			sprintf(tmps,".server %s// NOT RESPONDING", JoinServerStruct.mURL.c_str() );
			ggui.putText(4,14, tmps);
			ggui.putText(18,12, "sorry for the inconvenience");
			ggui.putText(18,11, "  please try again later      > OK");
		}
		break;
	case MENU_SOLO_MAIN:
		{
			BackFromGameMenu = MENU_SOLO_MAIN;

			ClearKeysReleased();
			Solo::InitMenu();
		}
		break;
	case MENU_ENDRACE_NETWORK:
		{
			ggui.clearBitmaps();
			GameShip *ps = GGame->GetPlayerShip();
			if (ps)
			{
				camera.SetCameraEndRace(ps);
				ggui.setBlackBox(17, 0, 30, 36);
				ggui.addSelector(NULL,0);
				drawEndResultPosition(32, 24, ps->mProps.mRank);
			}
		}
		break;
	case MENU_ENDRACE_SOLO:
		{
			GameShip *ps = GGame->GetPlayerShip();
			camera.SetCameraEndRace(ps);
			ggui.setBlackBox(17, 0, 30, 36);

			drawEndResultPosition(32, 24, ps->mProps.mRank);

			if ( ( ps->mProps.mHealth <= 0.f)&&(GGame->GetType() != 1) )
			{
				ggui.putText(27, 26, ".wrecked");
			}

			ggui.putText(20, 6, "track selection");
			ggui.putText(20, 5, "restart");
			ggui.putText(20, 4, "main menu");            
			int menuPos[]={ GC(18,6), GC(18,5), GC(18,4) };
			ggui.addSelector( menuPos, 3 );
		}
		break;
	case MENU_ENDRACE_SPLIT:
		{
			ggui.clearBitmaps();
			GameShip *ps = GGame->GetPlayerShip(0);
			camera.SetCameraEndRace(ps);
			GameShip *ps2 = GGame->GetPlayerShip(1);
			camera2.SetCameraEndRace(ps2);

			ggui.setBlackBox(17, 0, 30, 36);

			drawEndResultPosition(19, 24, ps->mProps.mRank);
			drawEndResultPosition(51, 24, ps2->mProps.mRank);
			/*
			if ( ps->mProps.mHealth <= 0.f)
			{
			ggui.putText(27, 26, ".wrecked//");
			}
			*/
			ggui.putText(20, 7, "ship selection");
			ggui.putText(20, 6, "track selection");
			ggui.putText(20, 5, "restart");
			ggui.putText(20, 4, "main menu");            
			int menuPos[]={ GC(18,7), GC(18,6), GC(18,5), GC(18,4) };
			ggui.addSelector( menuPos, 4 );
		}
		break;
	case MENU_LOBBY_JOIN:
		{
			//PopMeshStack();
			//Renderer::setRenderMeshStack(GetMeshStack());
			//Renderer::newTransition();
			//camera.SetCameraTrackObserve();
			ggui.setBlackBox(10, 0, 44, 36);

			ggui.putText(12, 32, " Prev");
			ggui.putText(12, 30, "NET/LAN   [MODE]Track        Ping Players");
			ggui.putText(12, 7, " Next");

			iServerBrowseNumPage = 0;
			StartLanDiscovery();
		}
		break;
#if 0
	case MENU_SELSHIP_QUICK:
	case MENU_SELSHIP_CAREER:
	case MENU_SELSHIP_CREATESERVER:
	case MENU_SELSHIP_JOINSERVER:
		{
			/*
			PushMeshStack();
			Renderer::setRenderMeshStack(GetMeshStack());
			//Renderer::newTransition();
			buildWorldForChosingShip();
			*/
			ggui.setBlackBox(5, 0, 25, 36);

			ggui.putText(12, 32, ".select your ship");


			ggui.putText(8, 26, "middle");
			ggui.putText(8, 25, "side");
			ggui.putText(8, 24, "back");
			ggui.putText(8, 23, "nose");




			GameShip *ps = GGame->GetShip(0);
			const shipIdentifier_t& shipId = ps->GetShipIndentifier();

			currentShipIdentifiterSelected = shipId;




#define SELSHIPCOMBOWIDTH 13
			selShipMiddle.setFrame(16, 26, SELSHIPCOMBOWIDTH, 1);
			selShipSide.setFrame(16, 25, SELSHIPCOMBOWIDTH, 1);
			selShipBack.setFrame(16, 24, SELSHIPCOMBOWIDTH, 1);
			selShipNose.setFrame(16, 23, SELSHIPCOMBOWIDTH, 1);


			u8 noseId = (shipId.mMeshesID>>24)&0xFF;
			u8 middleId = (shipId.mMeshesID>>8)&0xFF;
			u8 backId = (shipId.mMeshesID)&0xFF;
			u8 sideId = (shipId.mMeshesID<<16)&0xFF;


			selShipMiddle.setItems( countText, middleId, GetMiddleCount() );
			selShipSide.setItems( countText, sideId, GetSideCount() );
			selShipBack.setItems( countText, backId, GetBackCount() );
			selShipNose.setItems( countText, noseId, GetNoseCount() );


			ggui.putText(8, 21, "color");
			/*
			ggui.putText(8, 20, "Reactor");
			ggui.putText(8, 21, "Trail");
			*/
			selShipBodyColor.setFrame(16, 21, SELSHIPCOMBOWIDTH, 1);
			/*
			selShipReactorColor.setFrame(16, 20, SELSHIPCOMBOWIDTH, 1);
			selShipTrailColor.setFrame(16, 21, SELSHIPCOMBOWIDTH, 1);
			*/


			selShipBodyColor.setItems( allNamedColors, GetColorIndexFromValue(shipId.mShipColor), MAX_NB_COLORS );
			/*
			selShipReactorColor.setItems( allNamedColors, 0, MAX_NB_COLORS );
			selShipTrailColor.setItems( allNamedColors, 0, MAX_NB_COLORS );
			*/



			ggui.putText(10, 7, "[Enter] to validate");


			int menuPos[]={GC(6,26), GC(6,25), GC(6,24), GC(6, 23), GC(6, 21), /*GC(6,20), GC(6, 19), */};
			ggui.addSelector(menuPos, 5);

			camera.SetCameraEndRace( GGame->GetShip(0) );



		}
		break;
#endif
	case MENU_SPLIT_PRECOUNTDOWN:
	case MENU_SOLO_PRECOUNTDOWN:
		{
			AudioMusic::PlayRandomSong();
			splitScreenEnabled = false;
			char tmps[512];
			sprintf( tmps, " .%02d %s // %s", GGame->GetTrack()+1, Tracks[GGame->GetTrack()].mName.c_str(), GameTypes[GGame->GetType()].szName );

			ggui.putText(1, 7, tmps );
			ggui.putText(1, 6, GameTypes[GGame->GetType()].szDescription );

			ggui.setBlackBox(0, 3, 64, 6);
			ggui.putText(31, 4, ".press [Enter] to start racing//");
			camera.SetPreRace();
			camera2.SetCameraEndRace(NULL);
			//	        track.progressMesh( 0.f, 100.f );
		}
		break;

	case MENU_LOBBY_CREATE:
		{
			ggui.setBlackBox(9, 0, 30, 36);
			ggui.putText(13, 30, ".create game");
			ggui.putText(12, 26,"Mode");
			ggui.putText(12, 27,"# laps");
			ggui.putText(12, 28,"track");
			ggui.putText(13, 22,".connected buddies");

			ggui.putText(12, 7,"go!");
			//ggui.putText(12, 6,"back");

			quickMode.setFrame(22, 26, 14, 1);
			quickNumberOfLaps.setFrame(22, 27, 14, 1);
			quickTrack.setFrame(22, 28, 14, 1);

			server_advert_data_t serverInfo;
			serverInfo.mTrack = 0;
			serverInfo.mMode = 0;
			serverInfo.mNbLaps = 3;
			serverInfo.mNbPlayers = 1;
			serverInfo.mMaxNbPlayers = Game::E_Max_Ship_Count;

			UpdateAdvertisedServerInfo( serverInfo );

			quickMode.setItems(gameModesNames, serverInfo.mTrack, NBGameTypes);
			quickTrack.setItems( tracksNames, serverInfo.mMode, GameSettings.TracksCountUsedInGame );
			quickTrack.currentItem = (int)(track.GetCurrentTrack()- Tracks);
			quickNumberOfLaps.setItems((const char**)oneTo99, serverInfo.mNbLaps - 1, 99);
		}
		break;
	case MENU_SPLIT_TRACKSEL:
	case MENU_QUICKRACE_TRACKSEL:
		{
			BackFromGameMenu = MENU_QUICKRACE_TRACKSEL;

			//buildTrackSelection();
			//Renderer::newTransition();

			ggui.setBlackBox(9+8, 0, 30, 36);
			ggui.putText(13+8, 30, ".select track");
			ggui.putText(12+8, 26,"Mode");
			ggui.putText(12+8, 27,"# laps");
			ggui.putText(12+8, 28,"track");

			ggui.putText(14+8, 5,"[Enter] to validate");
			//ggui.putText(12, 6,"back");

			quickMode.setFrame(22+8, 26, 14, 1);
			quickNumberOfLaps.setFrame(22+8, 17+10, 14, 1);
			quickTrack.setFrame(22+8, 28, 14, 1);




			quickMode.setItems(gameModesNames, 0, NBGameTypes);
			quickTrack.setItems( tracksNames, 0, GameSettings.TracksCountUsedInGame );
			quickNumberOfLaps.setItems((const char**)oneTo99, 2, 99);
			quickTrack.currentItem = (int)(track.GetCurrentTrack()- Tracks);

			int menuPos[]={GC(10+8,18+10), GC(10+8,17+10), GC(10+8,16+10)};
			ggui.addSelector(menuPos, 3);
		}
		break;
	case MENU_EMPTY_SOLO:
	case MENU_EMPTY_NETWORK:
		Audio::SetGroupVolume( AUDIO_GROUP_GAME3D, 1.f );
		Audio::SetGroupVolume( AUDIO_GROUP_GAME2D, 1.f );		
		break;
	case MENU_WANTRESTARTRACE_SOLO:
	case MENU_WANTRESTARTRACE_NETWORK:
		{
			ggui.setBlackBox(20, 0, 20, 36);
			ggui.putText(24,16, ".confirm restart?");
			ggui.putText(27, 12,"yes");
			ggui.putText(27, 13,"no");
			int menuPos[]={GC(25,13), GC(25,12)};
			ggui.addSelector(menuPos, 2);
		}
		break;
	case MENU_PHOTOMODE:
		{
			ggui.setBlackBox(2, 0, 24, 36);

			ggui.putText(3,20, ".photo mode//");
			ggui.putText(5, 17, "Player");
			ggui.putText(5, 16, "DOF");
			ggui.putText(5, 15, "Focal");
			ggui.putText(5, 14, "Height");
			ggui.putText(5, 13, "Lateral");
			ggui.putText(5, 12, "Roll");

			ggui.putText(5, 10, "Camera placement");
			ggui.putText(5,  9, "Save");

			//ggui.putText(5,  7, "ok");

			int menuPos[]={GC(3,17), GC(3,16), GC(3,15), GC(3,14), GC(3,13), GC(3,12), GC(3,10), 
				GC(3,9), GC(3,7)};
			ggui.addSelector(menuPos, 8);

			photoDOF.set(0.f, 1.f, 0.f, 0.3f);
			photoDOF.value = photoModeParams.mDOF;
			photoDOF.setFrame(12, 16, 12, 1);

			photoFocal.set(0.f, 30.f, 0.f, 0.5f);
			photoFocal.setFrame(12, 15, 12, 1);
			photoFocal.value = photoModeParams.mFocal;//5.f;

			photoHeight.set(-1.f, 3.f, 0.f, 1.f);
			photoHeight.value = photoModeParams.mHeight;
			photoHeight.setFrame(12, 14, 12, 1);


			photoLateral.set(-2.f, 2.f, 0.f, 1.f);
			photoLateral.value = photoModeParams.mLateral;
			photoLateral.setFrame(12, 13, 12, 1);

			photoRoll.set(-PI, PI, 0.f, 1.f);
			photoRoll.value = photoModeParams.mRoll;
			photoRoll.setFrame(12, 12, 12, 1);

			photoShip.setFrame(12, 17, 12, 1);
			photoShip.setItems(countText, GGame->GetPlayerShip()->GetIndex(), GGame->GetShipCount());

			camera.SetCameraCustom();
			enableShowFocalCursor(true);
		}
		break;
	case MENU_PHOTOMODE_PHOTOSAVED:
		{
			ggui.setBlackBox(0, 15, 64, 5);
			char tmps[512];
			sprintf(tmps, ".screen saved as %s", screenShotFileName.c_str() );
			ggui.putText(2, 18, tmps);
			ggui.putText(2, 16, "> ok");
		}
		break;
	case MENU_PHOTOMODE_CAMERAPLACEMENT:
		{
			ggui.setBlackBox(0, 2, 64, 5);
			char tmps[512];
			sprintf(tmps, ".use arrow to rotate around and change distance//" );
			ggui.putText(2, 5, tmps);
			ggui.putText(2, 3, "> ok");
		}
		break;
	case MENU_LOADING:
		{
			ggui.setBlackBox(0, 14, 64, 6);
			ggui.clearBitmaps();
			ggui.putText( 4, 16, "Generating track ..." );
			/*
			const track_t& tr = Tracks[GGame->GetGameProps().mTrackIndex];
			const gameType_t& gt = GameTypes[GGame->GetGameProps().mType];

			char tmps[512];
			sprintf(tmps, ".generating seed # %d // track .%s.", tr.seed, tr.mName.c_str());
			ggui.putText(3, 10, tmps);
			sprintf(tmps, ".mode {%s}", gt.szName);
			ggui.putText(3, 9, tmps);

			loadingProgress.setFrame(32, 5, 29, 1);
			loadingProgress.setValue(0.8f);
			*/
		}
		break;
	case MENU_MULTI:
		{
			ggui.setBlackBox(2, 0, 25, 36);

			ggui.putText(3,20, ".multiplayer");
			ggui.putText(5, 17, "join server");
			ggui.putText(5, 16, "create server");
			ggui.putText(5, 14, "2 players splitscreen");
			//ggui.putText(5, 15, "split screen for 2");

			int menuPos[]={GC(3,17), GC(3,16), GC(3,14)};//, GC(3,15)};
			ggui.addSelector(menuPos, 3);
		}
		break;
	case MENU_CREDITS:
		{
			ggui.clearBitmaps();
			camera.SetCameraTrackObserve();

			ggui.setBlackBox(0, 1, 64, 3);
			char tmps[512];
			sprintf(tmps, ".track %s // use any control", track.GetCurrentTrack()->mName.c_str() );
			ggui.putText(12,2, tmps );//".use any control.");
			ggui.addBitmap( gui::guiBitmap(vec(0, 0, 10, 5), textures[TEXTURE_LOGO] ) );
		}
		break;
	case MENU_MAIN://mainmenu
		{
			splitScreenEnabled = false;
			ggui.clearBitmaps();
			Audio::SetGroupVolume( AUDIO_GROUP_GAME3D, 0.f );
			Audio::SetGroupVolume( AUDIO_GROUP_GAME2D, 0.f );

			world.SetGameSpeed( 1.f );
			ggui.setBlackBox(10, 0, 20, 36);
			//camera.SetCameraTrackObserve();
			camera.SetPreRace();
			// PutBannerText();
			ggui.addBitmap( gui::guiBitmap(vec(12, 18, 40, 20), textures[TEXTURE_LOGO] ) );



			ggui.putText(14,15, "quick");
			ggui.putText(14,14, "solo");
			ggui.putText(14,13, "multi");
			ggui.putText(14,12, "setup");
			ggui.putText(14,11, "howto");
			ggui.putText(14,10, "quit");

			ggui.putText(18,1, "V0.8 Beta Two "__DATE__);

			int menuPos[]={GC(12,15), GC(12,14), GC(12,13), GC(12,12), GC(12,11), GC(12,10) };//, GC(12,11), GC(12,10)};
			ggui.addSelector(menuPos, 6);
			ggui.addSelector( NULL, 0, 1 );

			// create bots
			bool createBots = false;
			for (int i = 0;i<Game::E_Max_Ship_Count;i++)
			{
				if ( (!GGame->GetShip(i)) || (GGame->GetShip(i) && (!GGame->GetShip(i)->IsBot())) )
				{
					createBots = true;
					break;
				}
			}
			if (createBots)
			{
				GGame->SetRace( NULL );
				GGame->Restart();
				trackBonusInstance_t::ClearBonuses();

				const int shipCountMainMenu = Game::E_Max_Ship_Count;
				for (int i = 0; i < shipCountMainMenu; ++i)
				{
					matrix_t mtc;
					mtc = track.GetSpawnMatrix( i );

					GameShip *ps = (GameShip *)GGame->Spawn(GO_Ship, mtc, NULL );
					ps->SetBot();
					shipIdentifier_t shipID;
					shipID.SetRandom();
					ps->SetShipIdentifier( shipID );
					ps->mProps.ctrlEnabled = true;
				}
			}
		}
		break;
	case MENU_WANTOQUIT_NETWORK:
	case MENU_WANTOQUIT_SOLO:
	case MENU_WANTOQUIT_MAIN://want to quit
		{
			SoundGui("ButtonAsk");
			ggui.setBlackBox(20, 0, 20, 36);
			ggui.putText(24,16, ".want to quit?");
			ggui.putText(27, 12,"yes");
			ggui.putText(27, 13,"no");
			int menuPos[]={GC(25,13), GC(25,12)};
			ggui.addSelector(menuPos, 2);
		}
		break;
		/*
		case 2: //track spline
		{
		ggui.setBlackBox(40, 0, 20, 18);
		ggui.putText(42,0, "Hide");
		ggui.putText(42,1, "val1");
		ggui.putText(42,2, "val2");

		int menuPos[]={GC(40,2), GC(40,1), GC(40,0)};
		ggui.addSelector(menuPos, 3);
		}
		break;*
		*/
	case MENU_SETUP_SOLO:
	case MENU_SETUP_NETWORK:
	case MENU_SETUP_MAIN: // setup
		{
			ggui.setBlackBox(12, 0, 32, 36);
			ggui.putText(14, 20, ".setup");
			ggui.putText(16, 18,"resolution");
			ggui.putText(16, 17,"fullscreen");
			ggui.putText(16, 16,"music vol");
			ggui.putText(16, 15,"fx vol");
			ggui.putText(16, 14,"antialias");
			ggui.putText(16, 13,"render shadows");
			ggui.putText(16, 12,"render ocean");
			ggui.putText(16, 11,"render reflection");
			ggui.putText(18, 7,"[Enter] to validate");
			ggui.putText(18, 4,"Game needs restart to");
			ggui.putText(18, 3,"apply video changes.");

			setupResolution.setFrame(30, 18, 12, 1);
			setupFullscreen.setFrame(30, 17, 12, 1);
			setupMusicVol.setFrame(30, 16, 12, 1);
			setupFXVol.setFrame(30, 15, 12, 1);
			setupAA.setFrame(30, 14, 12, 1);
			setupShadows.setFrame( 30, 13, 12, 1 );
			setupOcean.setFrame( 30, 12, 12, 1 );
			setupReflection.setFrame( 30, 11, 12, 1 );

			menuConfigBackup = GetEngineConfig();
			ASSERT_GAME( IsValidEngineConfig(menuConfigBackup) );

			const bool bUseFullscreen = (menuConfigBackup.fullscreen != 0);
			const int screenResWidth = menuConfigBackup.desiredWidth;
			const int screenResHeight = menuConfigBackup.desiredHeight;
			const float fxVolume = menuConfigBackup.fxVolume;
			const float musicVolume = menuConfigBackup.musicVolume;
			const bool bUseAntiAliasing = (menuConfigBackup.AAactive != 0);
			const bool bRenderShadows = (menuConfigBackup.RenderShadows != 0);
			const bool bRenderOcean = (menuConfigBackup.RenderOcean != 0);
			const bool bRenderReflection = (menuConfigBackup.RenderReflection != 0);

			u32 menuShowScreenResIdx = 0;

			const std::vector<resolution_t>& resolutions = GetSupportedScreenResolutions();
			ASSERT_GAME( resolutions.size() <= menuShowScreenResCount );
			for (u32 i = 0; i < menuShowScreenResCount; ++i)
			{
				const resolution_t& res = resolutions[i];
				if ( res.Height == screenResHeight && res.Width == screenResWidth )
				{
					menuShowScreenResIdx = i;
					break;
				}
			}

			setupResolution.setItems(resos, menuShowScreenResIdx, menuShowScreenResCount);
			setupFullscreen.setItems(yesno, (bUseFullscreen?1:0), 2);

			setupMusicVol.set(0.f,1.f, musicVolume, 0.5f);
			setupFXVol.set(0.f,1.f, fxVolume, 0.5f);

			setupAA.setItems(yesno, (bUseAntiAliasing?1:0), 2);
			setupShadows.setItems(yesno, (bRenderShadows?1:0), 2);
			setupOcean.setItems(yesno, (bRenderOcean?1:0), 2);
			setupReflection.setItems(yesno, (bRenderReflection?1:0), 2);

			int menuPos[]={GC(14,18), GC(14,17), GC(14,16), GC(14,15), GC(14, 14), GC(14, 13), GC(14, 12), GC(14, 11)};
			ggui.addSelector(menuPos, 8);
		}
		break;
	case MENU_STATS: // stats
		{
#if 0
			ggui.setBlackBox(12, 0, 30, 36);
			ggui.putText(14, 34, ".statistics");
			// --

			/*
			uint32 totalBonusTaken; 
			uint32 totalSpeedBonusTaken;
			uint32 totalPerBonus[32];
			uint32 totalEnemyShipsDestroyed;
			uint32 totalAbortedRaces;
			uint32 totalRacesDonePerGameType[16];
			*/


			char tmps[512];
			sprintf(tmps, "%d races done", Solo::playerProfil.numberOfRacesDone);
			ggui.putText(15, 32, tmps);
			sprintf(tmps, "%6.2f Km runned", Solo::playerProfil.totalDistanceDone* 0.001f);
			ggui.putText(15, 31, tmps);
			sprintf(tmps, "%6.2f hours racing", Solo::playerProfil.totalRaceTime * (1.f/3600.f));
			ggui.putText(15, 30, tmps);

			float averageSpeed = 0.f;
			if (Solo::playerProfil.totalRaceTime > FLOAT_EPSILON)
				averageSpeed = (Solo::playerProfil.totalDistanceDone* 0.001f)/(Solo::playerProfil.totalRaceTime * (1.f/3600.f));
			sprintf(tmps, "%6.2f Km/h average speed", averageSpeed);
			ggui.putText(15, 29, tmps);

			sprintf(tmps, "%d bonus taken", playerProfil.totalBonusTaken);
			ggui.putText(15, 28, tmps);
			sprintf(tmps, "%d speed bonus taken", playerProfil.totalSpeedBonusTaken);
			ggui.putText(15, 27, tmps);
			sprintf(tmps, "%d vehicles destroyed", playerProfil.totalEnemyShipsDestroyed);
			ggui.putText(15, 26, tmps);
			sprintf(tmps, "%d aborted races", playerProfil.totalAbortedRaces);
			ggui.putText(15, 25, tmps);

			sprintf(tmps, "%d points made in Cover mode", playerProfil.totalNumberOfPointInCover);
			ggui.putText(15, 24, tmps);


			// --
			ggui.putText(16, 3,"ok");
			int menuPos[]={GC(14,3)};
			ggui.addSelector(menuPos, 1);
#endif
		}
		break;
	case MENU_INGAME_SOLO:
		{
			ggui.setBlackBox(10, 0, 20, 36);
			ggui.putText(12,20, ".paused//");
			ggui.putText(14,15, "resume");
			ggui.putText(14,14, "restart");
			ggui.putText(14,13, "setup");
			ggui.putText(14,12, "photo mode");
			ggui.putText(14,11, "howto");
			ggui.putText(14,10, "quit race");

			int menuPos[]={GC(12,15), GC(12,14), GC(12,13), GC(12,12), GC(12,11), GC(12,10) };
			ggui.addSelector(menuPos, 6);


		}
		break;
	case MENU_INGAME_NETWORK:
		{
			ggui.setBlackBox(10, 0, 20, 36);
			ggui.putText(12,20, ".paused//");
			ggui.putText(14,16, "resume");
			ggui.putText(14,15, "howto");
			if ( NetServer->Active() )
			{
				//ggui.putText(14,14, "restart");
				ggui.putText(14,14, "setup");
				ggui.putText(14,13, "quit race");

				int menuPos[]={GC(12,16), GC(12,15), GC(12,14), GC(12,13)};
				ggui.addSelector(menuPos, 4);
			}
			else
			{
				ggui.putText(14,14, "setup");
				ggui.putText(14,13, "quit race");

				int menuPos[]={GC(12,16), GC(12,15), GC(12,14), GC(12,13)};
				ggui.addSelector(menuPos, 4);
			}
		}
		break;
	default:
		break;

	}
	mCurrentMenu = menuNumber;
}

void Menus::Validate( int playerId )
{
	switch (mCurrentMenu)
	{
	case MENU_ENDRACE_NETWORK:
		{
			if (!ggui.getSelectorCount())
				break;
			ggui.clearBitmaps();

			SoundGui("ButtonValid");
			const int curIdx = ggui.getSelectorIndex();
			switch (curIdx) {
			case 0: // track sel
				Show(MENU_LOBBY_CREATE);
				break;
			case 1: // restart
				GGame->Restart();
				GGame->SetType( static_cast<u8>( quickMode.currentItem ) );
				GGame->SetTrack( static_cast<u8>( quickTrack.currentItem ) );
				GGame->SetNumberOfLapsToDo( static_cast<u8>( quickNumberOfLaps.currentItem + 1 ) );
				GGame->SetSplit( false );
				camera.SetCameraCustom();
				break;
			case 2:
				NetServer->Close();
				NetClient->Close();
				Show(MENU_MAIN);
				break;
			}
		}
		break;
	case MENU_SEL_TEAM_QUICK:
		SoundGui("ButtonValid");
		currentShipIdentifiterSelected.mTeamIndex = static_cast<u8>( ggui.getSelectorIndex() );
		Menus::Show( MENU_SEL_EVOLUTION_QUICK );
		break;
	case MENU_SEL_TEAM_CAREER:
		SoundGui("ButtonValid");
		currentShipIdentifiterSelected.mTeamIndex = static_cast<u8>( ggui.getSelectorIndex() );
		Menus::Show( MENU_SEL_EVOLUTION_CAREER );
		break;
	case MENU_SEL_TEAM_CREATESERVER:
		SoundGui("ButtonValid");
		currentShipIdentifiterSelected.mTeamIndex = static_cast<u8>( ggui.getSelectorIndex() );
		Menus::Show( MENU_SEL_EVOLUTION_CREATESERVER );
		break;
	case MENU_SEL_TEAM_JOINSERVER:
		SoundGui("ButtonValid");
		currentShipIdentifiterSelected.mTeamIndex = static_cast<u8>( ggui.getSelectorIndex() );
		Menus::Show( MENU_SEL_EVOLUTION_JOINSERVER );
		break;
	case MENU_SEL_TEAM_SPLIT:
		SoundGui("ButtonValid");
		if (!playerId)
			currentShipIdentifiterSelected.mTeamIndex = static_cast<u8>( ggui.getSelectorIndex() );
		else
			currentShipIdentifiterSelected2.mTeamIndex = static_cast<u8>( ggui.getSelectorIndex(1) );

		ggui.addSelector( NULL, 0, playerId );
		splitValidation |= (1<<playerId);
		ggui.clearText(32*playerId, 32, 6, 5);
		ggui.putText(32*playerId+6, 8, "Waiting other Player");
		if (splitValidation == 3)
			Menus::Show( MENU_SEL_EVOLUTION_SPLIT );
		break;
	case MENU_SEL_EVOLUTION_SPLIT:
		SoundGui("ButtonValid");
		ggui.addSelector( NULL, 0, playerId );
		splitValidation |= (1<<playerId);
		ggui.clearText(32*playerId, 32, 6, 9);
		ggui.putText(32*playerId+6, 9, "Waiting other Player");
		if (splitValidation == 3)
			Menus::Show( MENU_SPLIT_TRACKSEL );

		break;
		/*
		case MENU_SEL_EVOLUTION_QUICK:
		case MENU_SEL_EVOLUTION_CAREER:
		case MENU_SEL_EVOLUTION_CREATESERVER:
		case MENU_SEL_EVOLUTION_JOINSERVER:
		break;
		*/
	case MENU_INGAME_NETWORK:
		{
			SoundGui("ButtonValid");
			const int curIdx = ggui.getSelectorIndex();
			switch (curIdx) {
			case 0:
				//Audio::Push2DAudioEvent( 2 );
				Show(MENU_EMPTY_NETWORK);
				break;
			case 1:
				//Audio::Push2DAudioEvent( 2 );
				Show(MENU_HOWTOPLAY_NETWORK);
				break;
			case 2:
				//Audio::Push2DAudioEvent( 2 );
				Show(MENU_SETUP_NETWORK);
				break;
			case 3:
				//Audio::Push2DAudioEvent( 2 );
				Show(MENU_WANTOQUIT_NETWORK);
				break;
			default:
				break;
			}
		}
		break;
	case MENU_CONNECTION_LOST:
		Show(MENU_LOBBY_JOIN);
		break;
	case MENU_LOBBY_JOINERROR:
		Show(MENU_LOBBY_JOIN);
		break;
	case MENU_LOBBY_JOIN:
		{
			int curIdx = ggui.getSelectorIndex();
			bool isOnNext = (curIdx == (ggui.getSelectorCount()-1));

			if (curIdx == 0 && iServerBrowseNumPage>0)
			{
				iServerBrowseNumPage--;
			}
			else if (isOnNext && iServerBrowseNumPage < (iServerBrowsePagesCount-1))
			{
				iServerBrowseNumPage++;
			}
			else if (curIdx > 0 && (!isOnNext) )
			{
				// connect to server
				const std::vector<server_t> & srv = GetActiveServers();
				JoinServerStruct = srv[curIdx-1];
				StopLanDiscovery();
				Show(MENU_LOBBY_JOINING);
			}
		}
		break;
	case MENU_ENDRACE_SOLO:
		{
			ggui.clearBitmaps();
			SoundGui("ButtonValid");
			switch (ggui.getSelectorIndex())
			{
			case 0:
				// back to grid
				Show(BackFromGameMenu);
				break;
			case 1:
				// restart
				StartGameSolo();
				break;
			case 2: 
				// back to main
				GGame->Restart();
				Show( MENU_MAIN );
				break;

			}
		}
		break;
	case MENU_ENDRACE_SPLIT:
		ggui.clearBitmaps();
		SoundGui("ButtonValid");
		switch (ggui.getSelectorIndex())
		{
		case 0:
			// ship sel
			Show(MENU_SEL_TEAM_SPLIT);
			break;
		case 1:
			// back to grid
			Show(MENU_SPLIT_TRACKSEL);
			break;
		case 2:
			// restart
			StartGameSplit();
			break;
		case 3: 
			// back to main
			splitScreenEnabled = false;
			GGame->Restart();
			Show( MENU_MAIN );
			break;

		}
		break;
	case MENU_SEL_EVOLUTION_JOINSERVER:
		{            
			SoundGui("ButtonValid");
			Show(MENU_LOBBY_JOIN);
		}
		break;
	case MENU_SEL_EVOLUTION_CREATESERVER:
		{
			SoundGui("ButtonValid");

			// create network server

			NetServer->CreateServer();

			// player props

			shipIdentifier_t shipId;
			GuiToShipId( shipId );
			NetClient->SetPlayerInfos( GetUserName().c_str(), shipId );

			// net client go local!

			NetClient->ConnectTo( "127.0.0.1" );

			// Go create lobby GUI

			Show( MENU_LOBBY_CREATE );
		}
		break;
	case MENU_MULTI:
		SoundGui("ButtonValid");
		switch (ggui.getSelectorIndex())
		{
		case 0: Show(MENU_SEL_TEAM_JOINSERVER); break;
		case 1: Show(MENU_SEL_TEAM_CREATESERVER); break;
		case 2: 
			splitScreenEnabled = true; 
			Show(MENU_SEL_TEAM_SPLIT); 
			break;
		}
		break;
	case MENU_SOLO_PRECOUNTDOWN:
		{
			SoundGui("ButtonValid");
			StartGameSolo();
		}
		break;
	case MENU_SPLIT_PRECOUNTDOWN:
		{
			SoundGui("ButtonValid");
			StartGameSplit();
		}
		break;
	case MENU_PHOTOMODE:
		{

			switch (ggui.getSelectorIndex())
			{
			case 1: photoDOF.value = 0.f; SoundGui("ButtonValid"); 
				break;
			case 2: photoFocal.value = 5.f; SoundGui("ButtonValid"); 
				break;
			case 3: photoHeight.value = 0.f; SoundGui("ButtonValid"); 
				break;
			case 4: photoLateral.value = 0.f; SoundGui("ButtonValid"); 
				break;
			case 5: photoRoll.value = 0.f; SoundGui("ButtonValid"); 
				break;
			case 6: // placement
				Show(MENU_PHOTOMODE_CAMERAPLACEMENT);
				SoundGui("ButtonValid");
				break;
			case 7: // save
				{
					//extern sf::Clock GlobalClock;
					screenShotFileName = makeScreenShot((int)(GetApplicationTime() * 1000.f), false);
					Show(MENU_PHOTOMODE_PHOTOSAVED);

				}
				break;
			case 8: // ok
				setDOFBlur(0.f);
				//camera.lockOnMatrix( &GGame->GetPlayerShip()->GetPhysics()->GetTransform() );
				camera.SetCameraBehindShip( GGame->GetPlayerShip() );
				Show(MENU_INGAME_SOLO);
				SoundGui("ButtonValid");
				break;
			}
		}
		break;
	case MENU_PHOTOMODE_CAMERAPLACEMENT:
	case MENU_PHOTOMODE_PHOTOSAVED:
		Show(MENU_PHOTOMODE);
		SoundGui("ButtonValid");
		break;
	case MENU_MAIN: // mainmenu
		ggui.clearBitmaps();
		StopSequencePlayback();
		switch (ggui.getSelectorIndex())
		{
		case 0: // quick
			SoundGui("ButtonValid");
			Show( MENU_QUICKRACE_TRACKSEL );//MENU_SELSHIP_QUICK );//MENU_QUICKRACE_TRACKSEL);
			//Show( MENU_SOLO_MAIN );
			break;

		case 1: // solo
			SoundGui("ButtonValid");
			Show(MENU_SOLO_MAIN);
			break;

		case 2:	// lan
			//Game::CreateNewGameSolo(3);
			SoundGui("ButtonValid");
			Show(MENU_MULTI);
			break;
		case 3: // show setup
			SoundGui("ButtonValid");
			Show(MENU_SETUP_MAIN);
			break;
		case 4: // how to play
			SoundGui("ButtonValid");
			Show(MENU_HOWTOPLAY);
			break;
		case 5: // quit?
			SoundGui("ButtonValid");
			Show(MENU_WANTOQUIT_MAIN);
			break;
		default:

			break;
		}
		break;
	case MENU_HOWTOPLAY:
		ggui.clearBitmaps();
		Show(MENU_MAIN);
		break;
	case MENU_HOWTOPLAY_SOLO:
		ggui.clearBitmaps();
		Show(MENU_INGAME_SOLO);
		break;
	case MENU_HOWTOPLAY_NETWORK:
		ggui.clearBitmaps();
		Show(MENU_INGAME_NETWORK);
		break;
	case MENU_WANTOQUIT_MAIN: // want to quit
		SoundGui("ButtonAsk");
		if (ggui.getSelectorIndex() == 0)
		{
			Show(MENU_MAIN);
		}
		else
			QuitGameNow();
		break;
	case MENU_WANTOQUIT_SOLO:
		SoundGui("ButtonAsk");
		if (ggui.getSelectorIndex() == 0)
		{
			//		Show(MENU_MAIN);*
			Show(MENU_INGAME_SOLO);
		}
		else
		{
			// quit // back to main menu
			GGame->Restart();
			Show( MENU_MAIN );
		}
		break;
	case MENU_WANTOQUIT_NETWORK:
		SoundGui("ButtonAsk");
		if (ggui.getSelectorIndex() == 0)
		{
			Show(MENU_INGAME_NETWORK);
		}
		else
		{
			GGame->Restart();
			NetServer->Close();
			NetClient->Close();
			Show( MENU_MAIN );
		}
		break;
	case MENU_WANTRESTARTRACE_SOLO:
		{
			SoundGui("ButtonAsk");
			switch(ggui.getSelectorIndex())
			{
			case 0:
				Show(MENU_INGAME_SOLO);
				break;
			case 1:
				//GGame->Restart();
				StartGameSolo( GGame->GetType() );
				Show(MENU_EMPTY_SOLO);
				break;
			}
		}
		break;
	case MENU_WANTRESTARTRACE_NETWORK:
		{
			SoundGui("ButtonAsk");
			switch(ggui.getSelectorIndex())
			{
			case 0:
				Show(MENU_INGAME_NETWORK);
				break;
			case 1:
				////////////////////////////////////////////////////////////////////
				//FIXME
				//GGame->Restart();
				//StartGameSolo( GGame->GetType() );
				Show(MENU_EMPTY_NETWORK);
				break;
			}
		}
		break;
	case MENU_SPLIT_TRACKSEL:
	case MENU_SEL_EVOLUTION_QUICK: // ship selected, we create the game
		{
			SoundGui("ButtonValid");
			// clear game
			GGame->DestroyGameObjects();

			GGame->ResetNetIndexAllocator();

			physicWorld.ClearCollisions();

			Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME2D );
			Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME3D );

			// set game
			/*
			GGame->mProps.mTrackIndex = quickTrack.currentItem;
			GGame->mProps.mNumberOfLapsToDo = quickNumberOfLaps.currentItem +1;
			GGame->mProps.mType = quickMode.currentItem;
			*/
			GGame->SetType( static_cast<u8>( quickMode.currentItem ) );
			GGame->SetTrack( static_cast<u8>( quickTrack.currentItem ) );
			GGame->SetNumberOfLapsToDo( static_cast<u8>( quickNumberOfLaps.currentItem + 1 ) );

			bool bSplitScreen = (mCurrentMenu == MENU_SPLIT_TRACKSEL);
			GGame->SetSplit( bSplitScreen );

			Show( bSplitScreen?MENU_SPLIT_PRECOUNTDOWN:MENU_SOLO_PRECOUNTDOWN );
		}
		break;
	case MENU_SEL_EVOLUTION_CAREER:
		{ 
			SoundGui("ButtonValid");
			Solo::ApplyCurrentRace();

			Show(MENU_SOLO_PRECOUNTDOWN);
		}
		break;
	case MENU_LOBBY_CREATE:
		{
			const int nbBuddies = NetClient->GetConnectedPlayers();
			//if ( nbBuddies > 1 )
			{
				// validate the game we want, set it!
				/*
				GGame->mProps.mbDirty = true;
				GGame->mProps.mNumberOfLapsToDo = quickNumberOfLaps.currentItem +1;
				GGame->mProps.mTrackIndex = quickTrack.currentItem;
				GGame->mProps.mType = quickMode.currentItem;
				*/
				camera.SetCameraCustom();
				GGame->Restart();
				GGame->SetType( static_cast<u8>( quickMode.currentItem ) );
				GGame->SetTrack( static_cast<u8>( quickTrack.currentItem ) );
				GGame->SetNumberOfLapsToDo( static_cast<u8>( quickNumberOfLaps.currentItem + 1 ) );
				GGame->SetSplit( false );
				
			}
		}
		break;
	case MENU_QUICKRACE_TRACKSEL:
		{
			SoundGui("ButtonValid");
			Renderer::SetOverGUIMesh( NULL );
			Show( MENU_SEL_TEAM_QUICK );
		}
		break;
	case MENU_SETUP_SOLO:
	case MENU_SETUP_NETWORK:
	case MENU_SETUP_MAIN: // setup
		// apply settings
		{
			SoundGui("ButtonValid");

			const u32 screenResIdx = setupResolution.currentItem;

			const std::vector<resolution_t>& resolutions = GetSupportedScreenResolutions();
			ASSERT_GAME( screenResIdx < resolutions.size() );
			const resolution_t& res = resolutions[screenResIdx];

			config_t newConfig = GetEngineConfig();
			newConfig.fullscreen = (setupFullscreen.currentItem != 0)? 1 : 0;
			newConfig.desiredWidth = res.Width;
			newConfig.desiredHeight = res.Height;
			newConfig.musicVolume = setupMusicVol.value;
			newConfig.fxVolume = setupFXVol.value;
			newConfig.AAactive = (setupAA.currentItem != 0)? 1 : 0;
			newConfig.RenderShadows = (setupShadows.currentItem != 0)? 1 : 0;
			newConfig.RenderOcean = (setupOcean.currentItem != 0)? 1 : 0;
			newConfig.RenderReflection = (setupReflection.currentItem != 0)? 1 : 0;

			SetEngineConfig( newConfig );

			if ( AreEngineConfigEquals( newConfig, menuConfigBackup ) == false)
			{
				writeConfig();

				//HACK: change video mode doesn't work properly in fullscreen
				if ( !menuConfigBackup.fullscreen && !newConfig.fullscreen )
				{
					ChangeVideoModeAfterBufferSwap();
				}
				else
				{
					//FIXME: display message about new settings to be applied next game reboot
				}
			}
		}
		switch ( mCurrentMenu )
		{
		case MENU_SETUP_SOLO:
			Show( MENU_INGAME_SOLO );
			break;
		case MENU_SETUP_NETWORK:
			Show( MENU_INGAME_NETWORK );
			break;
		case MENU_SETUP_MAIN:
			Show( MENU_MAIN );
			break;
		default:
			break;
		}
		break;
	case MENU_STATS: // stats
		SoundGui("ButtonValid");
		Show(MENU_MAIN);
		break;
	case MENU_INGAME_SOLO:
		switch (ggui.getSelectorIndex())
		{
		case 0:// resume
			SoundGui("ButtonValid");
			Show(MENU_EMPTY_SOLO);
			break;
		case 1: // restart
			Show(MENU_WANTRESTARTRACE_SOLO);
			break;
		case 2: // setup
			SoundGui("ButtonValid");
			Show(MENU_SETUP_SOLO);
			break;
		case 3:
			SoundGui("ButtonValid");
			photoModeParams.mAngle = 0;//-PI*0.25f;
			photoModeParams.mDOF = 0.f;
			photoModeParams.mFocal = 5.f;
			photoModeParams.mRoll = 0.f;
			photoModeParams.mDistance = 6.f;
			photoModeParams.mShipMatrix = GGame->GetShip(0)->GetPhysics()->GetGroundMatrix();
			photoModeParams.mShipMatrix.position = GGame->GetShip(0)->GetPhysics()->GetTransform().position;
			photoModeParams.mHeight = 0.f;
			photoModeParams.mLateral = 0.f;
			Show(MENU_PHOTOMODE);
			break;
		case 4:
			Show(MENU_HOWTOPLAY_SOLO);
			break;
		case 5:
			Show(MENU_WANTOQUIT_SOLO);
			break;
		}
		break;
	default:
		break;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Menus::Tick( float aTimeEllapsed )
{
	switch(mCurrentMenu)
	{
	case MENU_SEL_TEAM_QUICK:
	case MENU_SEL_TEAM_CAREER:
	case MENU_SEL_TEAM_CREATESERVER:
	case MENU_SEL_TEAM_JOINSERVER:
		{
			//Renderer::SetOverGUIMesh( Tracks[ quickTrack.currentItem ].mMiniMesh );
			Renderer::SetOverGUIMeshRectangle( vec( 11, 1, 25, 25 ) );
			int teamIndex = ggui.getSelectorIndex();
			unsigned int nbPoints = Solo::playerProfil.evolutions[ teamIndex ];

			unsigned int nbPointsNeededBeforeEvo = Solo::playerProfil.GetPointNeededForEvolution( Solo::playerProfil.GetEvolutionIndex( teamIndex )+1 );

			guiProgress::directDraw( &ggui, 20-15, 13, 20, float( nbPoints ) / float( nbPointsNeededBeforeEvo ) );
			char tmps[128];
			sprintf( tmps, "evolution %d / %d       ", Solo::playerProfil.GetEvolutionIndex( teamIndex ), profil_t::GetEvolutionCount() );
			ggui.putText( 24-15, 15, tmps );

			int nbpb4 = nbPointsNeededBeforeEvo - nbPoints;
			sprintf( tmps, "%d point%s before evolution       ", nbpb4, (nbpb4>1)?"s":"" );
			ggui.putText( 19-15, 11, tmps );

			const int currentShipTeamIndex = currentShipIdentifiterSelected.mTeamIndex;
			if ( currentShipTeamIndex != teamIndex )
			{
				currentShipIdentifiterSelected.mTeamIndex = static_cast<u8>( teamIndex );
				GameShip *ps = GGame->GetShip(0);      

				matrix_t oldMAG = ps->GetMatrixForMissilesAndGun();
				matrix_t oldBeacon = ps->GetBeaconLocalY();
	

				ps->NetworkConstruct( ps->GetPhysics()->GetTransform(), currentShipIdentifiterSelected );
				ps->SetMatrixForMissilesAndGun( oldMAG );
				ps->SetBeaconLocalY( oldBeacon );
			}
		}
		break;
	case MENU_SEL_TEAM_SPLIT:
		{
			shipIdentifier_t *shipIds[]={&currentShipIdentifiterSelected, &currentShipIdentifiterSelected2 };
			for (int i=0;i<2;i++)
			{
				if (!ggui.getSelectorCount(i))
					continue;

				int teamIndex = ggui.getSelectorIndex(i);
				const int currentShipTeamIndex = shipIds[i]->mTeamIndex;
				if ( currentShipTeamIndex != teamIndex )
				{
					shipIds[i]->mTeamIndex = static_cast<u8>( teamIndex );
					GameShip *ps = GGame->GetShip(i);
					ps->NetworkConstruct( ps->GetPhysics()->GetTransform(), *shipIds[i] );
				}

			}
		}
		break;
	case MENU_SEL_EVOLUTION_QUICK:
	case MENU_SEL_EVOLUTION_CAREER:
	case MENU_SEL_EVOLUTION_CREATESERVER:
	case MENU_SEL_EVOLUTION_JOINSERVER:
		{            

			int selectorIndex = ggui.getSelectorIndex()-1;
			bool hasChanged = ggui.getSelectorIndex()&&( selectorIndex != currentShipIdentifiterSelected.mEvolutionIndex );

			// set ship
			if ( hasChanged ) 
			{
				if (selectorIndex != -1 )
					currentShipIdentifiterSelected.mEvolutionIndex = static_cast<u8>( selectorIndex );

				GameShip *ps = GGame->GetShip(0);
				ps->NetworkConstruct( ps->GetPhysics()->GetTransform(), currentShipIdentifiterSelected );
			}
		}
		break;
	case MENU_SEL_EVOLUTION_SPLIT:
		{
			shipIdentifier_t *shipIds[]={&currentShipIdentifiterSelected, &currentShipIdentifiterSelected2 };
			for (int i=0;i<2;i++)
			{
				if (!ggui.getSelectorCount(i))
					continue;

				int selectorIndex = ggui.getSelectorIndex(i)-1;
				bool hasChanged = ggui.getSelectorIndex(i)&&( selectorIndex != shipIds[i]->mEvolutionIndex );

				// set ship
				if ( hasChanged ) 
				{
					if (selectorIndex != -1 )
						shipIds[i]->mEvolutionIndex = static_cast<u8>( selectorIndex );

					GameShip *ps = GGame->GetShip(i);
					ps->NetworkConstruct( ps->GetPhysics()->GetTransform(), *shipIds[i] );
				}
			}
		}
		break;
	case MENU_CONNECTED_WAITING:
	case MENU_WAITING_BUDDIES:
		{
			const int connectedPlayerCount = NetClient->GetConnectedPlayers();
			const int connectedPlayerMax = Game::E_Max_Ship_Count;
			int readyCount = connectedPlayerCount;
			for (int  i = 0 ; i < connectedPlayerMax; ++i)
			{
				bool bDrawSpace = true;
				if ( i < connectedPlayerCount )
				{
					const playerNetInfos_t *plNfo = NetClient->GetPlayerInfos( i );
					if ( !plNfo->mbIsReady )
					{
						ggui.putText( 22, 20-i, plNfo->mNickName.c_str() );
						bDrawSpace = false;
						readyCount--;
					}
				}
				if ( bDrawSpace )
				{
					ggui.putText( 22, 20-i, "                                         " );
				}
			}
			if ( connectedPlayerCount && ( connectedPlayerCount == readyCount ) && (NetServer != NULL) )
			{
				NetServer->BroadcastStartRace();
			}
		}
		break;
	case MENU_LOBBY_CREATE:
		{
			int selectorIndex = ggui.getSelectorIndex();

			if (ControlLeftReleased())
				quickElts[selectorIndex]->applyPrevReleased(aTimeEllapsed);
			if (ControlRightReleased())
				quickElts[selectorIndex]->applyNextReleased(aTimeEllapsed);

			//updates advertised server info for broadcast
			server_advert_data_t serverInfo;
			ASSERT_GAME( quickTrack.currentItem < U8_MAX );
			serverInfo.mTrack = static_cast<u8>( quickTrack.currentItem );
			serverInfo.mMode = static_cast<u8>( quickMode.currentItem );
			serverInfo.mNbLaps = static_cast<u8>( quickNumberOfLaps.currentItem + 1 );
			serverInfo.mNbPlayers = static_cast<u8>( NetClient->GetConnectedPlayers() );
			serverInfo.mMaxNbPlayers = static_cast<u8>(Game::E_Max_Ship_Count);

			UpdateAdvertisedServerInfo( serverInfo );

			quickMode.draw(&ggui);
			quickTrack.draw(&ggui);
			quickNumberOfLaps.draw(&ggui);

			Renderer::SetOverGUIMesh( Tracks[ quickTrack.currentItem ].mMiniMesh );
			Renderer::SetOverGUIMeshRectangle( vec( 11, 1, 25, 25 ) );

			ggui.putText( 22, 25, GetEnvName(Tracks[quickTrack.currentItem].mEnvironmentIndex) );            

			// show connected buddies

			const int nbBuddies = NetClient->GetConnectedPlayers();
			for (int  i = 0 ; i < nbBuddies ; i++ )
			{
				const playerNetInfos_t *plNfo = NetClient->GetPlayerInfos( i );
				ggui.putText( 13, 20-i, plNfo->mNickName.c_str() );
			}
			const int connectedPlayerMax = Game::E_Max_Ship_Count;
			for (int  i = nbBuddies ; i < connectedPlayerMax ; ++i)
			{
				//const playerNetInfos_t *plNfo = NetClient->GetPlayerInfos( i );
				ggui.putText( 13, 20-i, "                                         " );
			}

			int menuPos[]={GC(10,28), GC(10,27), GC(10,26), GC(10, 7)};
			if ( nbBuddies > 1 )
			{
				ggui.putText(12, 7,"go!");
				int curi = ggui.getSelectorIndex();

				ggui.addSelector(menuPos, 4);
				ggui.setSelectorIndex( curi );
			}
			else
			{
				ggui.putText(12, 7,"   ");
				int curi = ggui.getSelectorIndex();

				ggui.addSelector(menuPos, 3);
				if (curi != 3)
					ggui.setSelectorIndex( curi );
			}
		}
		break;
	case MENU_SPLIT_PRECOUNTDOWN:
	case MENU_SOLO_PRECOUNTDOWN:
		{            
			static float progressAv = 0.f;
			progressAv += aTimeEllapsed * 60.f;
			//	        track.progressMesh( progressAv, 100.f );
		}
		break;
	case MENU_LOBBY_JOIN:
		{
			int curIdx = ggui.getSelectorIndex();
			bool isOnNext = (curIdx == (ggui.getSelectorCount()-1));

			int menuPos[22];//={GC(18,6), GC(18,5)};
			int menuPosAv = 0;
			menuPos[menuPosAv++] = GC(11, 32); // prev
			const std::vector<server_t> & srv = GetActiveServers();

			for (int i = 0;i<20;i++)
			{
				unsigned int idx = iServerBrowseNumPage * 20 + i;
				if (idx >= srv.size())
					ggui.putText(13, 28-i, "                                          ");
				else
				{
					ggui.putText(13, 28-i, srv[i].Format().c_str() );
					menuPos[menuPosAv++] = (GC(11, (28-i))); // items
				}

			}

			menuPos[menuPosAv++] = GC(11, 7);
			ggui.addSelector(menuPos, menuPosAv);
			ggui.setSelectorIndex(isOnNext? (menuPosAv-1):curIdx);


			iServerBrowsePagesCount = (int)ceilf( (float)srv.size() / 20.f );
			char tmps[512];
			sprintf(tmps, ".page %02d/%02d", iServerBrowseNumPage+1, iServerBrowsePagesCount );
			ggui.putText(42, 7, tmps);
		}
		break;
	case MENU_ENDRACE_SOLO:
		{
			char tmps[512];

			bool bInversed = !GameTypes[GGame->GetGameProps().mType].mbFirstInResultListHasWon;
			int nbString = GGame->GetEndResultsNbStrings();
			for (int i=0;i<nbString;i ++)
			{

				int idxx = bInversed?(nbString - i - 1):i;
				sprintf(tmps, ".%02d %s", bInversed?(9-nbString+i):(i+1), GGame->GetEndResultString(idxx).c_str() );
				ggui.putText(18, 22-i, tmps);
			}

			// show evolution

			int teamIndex = GGame->GetPlayerShip()->GetShipIndentifier().mTeamIndex;
			unsigned int nbPoints = Solo::playerProfil.evolutions[ teamIndex ];

			unsigned int nbPointsNeededBeforeEvo = Solo::playerProfil.GetPointNeededForEvolution( Solo::playerProfil.GetEvolutionIndex( teamIndex ) );


			guiProgress::directDraw( &ggui, 18, 10, 20, float( nbPoints ) / float( nbPointsNeededBeforeEvo ) );

			sprintf( tmps, "evolution %d /%d       ", Solo::playerProfil.GetEvolutionIndex( teamIndex ), profil_t::GetEvolutionCount() );
			ggui.putText( 18, 11, tmps );

			sprintf( tmps, "%d points before evolution       ", nbPointsNeededBeforeEvo - nbPoints );
			ggui.putText( 18, 9, tmps );


		}
		break;
	case MENU_ENDRACE_SPLIT:
		{
			int nbString = GGame->GetEndResultsNbStrings();
			char tmps[512];
			//bool InvertedList = GameTypes[GGame->GetType()].mbFirstInResultListHasWon;
			bool bInversed = !GameTypes[GGame->GetGameProps().mType].mbFirstInResultListHasWon;
			for (int i=0;i<nbString;i ++)
			{

				int idxx = bInversed?(nbString - i - 1):i;
				sprintf(tmps, ".%02d %s", bInversed?(9-nbString+i):(i+1), GGame->GetEndResultString(idxx).c_str() );
				ggui.putText(18, 22-i, tmps);
			}
		}
		break;
	case MENU_ENDRACE_NETWORK:
		{
			if ( NetServer->Active() )
			{
				if ( GGame->EveryBodyFinishedRacing() )
				{
					ggui.putText(20, 6, "track selection");
					ggui.putText(20, 5, "restart");
					ggui.putText(20, 4, "quit");
					int menuPos[]={GC(18,6), GC(18,5), GC(18,4)};
					if (!ggui.getSelectorCount())
						ggui.addSelector(menuPos, 3);
				}
				else
				{
					ggui.putText(20, 6, ".waiting for buddies");
				}
			}
			else
			{
				ggui.putText(20, 6, ".waiting for buddies");
			}
			int nbString = GGame->GetEndResultsNbStrings();
			bool bInversed = !GameTypes[GGame->GetGameProps().mType].mbFirstInResultListHasWon;
			for (int i=0;i<nbString;i ++)
			{
				char tmps[512];
				int idxx = bInversed?(nbString - i - 1):i;
				sprintf(tmps, ".%02d %s", bInversed?(9-nbString+i):(i+1), GGame->GetEndResultString(idxx).c_str() );
				ggui.putText(18, 22-i, tmps);
			}
		}
		break;
	case MENU_EMPTY_SOLO:
		{
			if (GGame->GetPlayerEndRaceInactivityTime() > 1.5f)
			{
				world.SetGameSpeed( 1.f );
				if ( GGame->GetPlayerShip(0) && GGame->GetPlayerShip(1) )
					Show(MENU_ENDRACE_SPLIT);
				else
					Show(MENU_ENDRACE_SOLO);
			}
			world.SetGameSpeed( 1.f );
		}
		break;
	case MENU_EMPTY_NETWORK:
		if ( GGame->GetPlayerEndRaceInactivityTime() >= 1.5f )
			Show( MENU_ENDRACE_NETWORK );
		break;
#if 0
	case MENU_SELSHIP_QUICK:
	case MENU_SELSHIP_CAREER:
	case MENU_SELSHIP_CREATESERVER:
	case MENU_SELSHIP_JOINSERVER:
		{
			int selectorIndex = ggui.getSelectorIndex();
			bool hasChanged = false;
			if (selectorIndex < 6 )
			{
				if (ControlLeftReleased())
					selShipElts[selectorIndex]->applyPrevReleased(aTimeEllapsed);
				if (ControlRightReleased())
					selShipElts[selectorIndex]->applyNextReleased(aTimeEllapsed);

				hasChanged = (ControlRightReleased()||ControlLeftReleased());
			}
			for (int i=0;i<5;i++)
				selShipElts[i]->draw( &ggui );	

			// set ship
			if ( hasChanged ) 
			{
				//shipIdentifier_t shipId;
				GuiToShipId( currentShipIdentifiterSelected );

				GameShip *ps = GGame->GetShip(0);
				/*
				ps->mInitialMatrix = ps->GetPhysics()->GetTransform();
				GGame->GetShip(0)->SetShipIdentifier( shipId );
				*/
				ps->NetworkConstruct( ps->GetPhysics()->GetTransform(), currentShipIdentifiterSelected );
			}
		}
		break;
#endif
	case MENU_SOLO_MAIN:
		{
			if (ControlLeftReleased())
				soloLeft = true;
			if (ControlRightReleased())
				soloRight = true;
			Solo::Tick(aTimeEllapsed);
		}
		break;
	case MENU_SETUP_SOLO:
	case MENU_SETUP_NETWORK:
	case MENU_SETUP_MAIN: // setup
		{
			setupResolution.draw(&ggui);
			setupFullscreen.draw(&ggui);
			setupMusicVol.draw(&ggui);
			setupFXVol.draw(&ggui);
			setupAA.draw(&ggui);
			setupShadows.draw(&ggui);
			setupOcean.draw(&ggui);
			setupReflection.draw(&ggui);

			int selectorIndex = ggui.getSelectorIndex();

			if (ControlLeft())
				setupElts[selectorIndex]->applyPrev(aTimeEllapsed);
			if (ControlRight())
				setupElts[selectorIndex]->applyNext(aTimeEllapsed);
			if (ControlLeftReleased())
				setupElts[selectorIndex]->applyPrevReleased(aTimeEllapsed);
			if (ControlRightReleased())
				setupElts[selectorIndex]->applyNextReleased(aTimeEllapsed);

			config_t newConfig = GetEngineConfig();
			newConfig.musicVolume = setupMusicVol.value;
			newConfig.fxVolume = setupFXVol.value;
			newConfig.AAactive = (setupAA.currentItem != 0)? 1 : 0;
			newConfig.RenderShadows = (setupShadows.currentItem != 0)? 1 : 0;
			newConfig.RenderOcean = (setupOcean.currentItem != 0)? 1 : 0;
			newConfig.RenderReflection = (setupReflection.currentItem != 0)? 1 : 0;
			AudioMusic::SetMusicVolume( setupMusicVol.value );
			SetEngineConfig( newConfig );
		}
		break;
	case MENU_SPLIT_TRACKSEL:
	case MENU_QUICKRACE_TRACKSEL:
		{
			//tickTrackSelection(aTimeEllapsed, quickTrack.currentItem);

			int selectorIndex = ggui.getSelectorIndex();
			if (selectorIndex < 3 )
			{
				if (ControlLeftReleased())
					quickElts[selectorIndex]->applyPrevReleased(aTimeEllapsed);
				if (ControlRightReleased())
					quickElts[selectorIndex]->applyNextReleased(aTimeEllapsed);
			}

			quickMode.draw(&ggui);
			quickTrack.draw(&ggui);
			quickNumberOfLaps.draw(&ggui);

			Renderer::SetOverGUIMesh( Tracks[ quickTrack.currentItem ].mMiniMesh );
			Renderer::SetOverGUIMeshRectangle( vec( 11+8, 6, 25, 25 ) );

			ggui.putText( 22+8, 24, GetEnvName(Tracks[quickTrack.currentItem].mEnvironmentIndex) );
		}
		break;
	case MENU_LOADING:
		{
			/*
			loadingProgress.draw(&ggui);
			static float avTxt = 0;
			avTxt += aTimeEllapsed*6.f;

			int avTxtI = (int)avTxt;

			const gameType_t& gt = GameTypes[GGame->GetGameProps().mType];
			int s = strlen(gt.szDescription);
			for (int i=3;i<61;i++)
			{
				ggui.putChar(i, 7, gt.szDescription[ (i + avTxtI) % s ]);
			}
			*/
		}
		break;

	case MENU_INGAME_SOLO:
		world.SetGameSpeed( 0.f );
		break;
	case MENU_PHOTOMODE:
		{
			int selectorIndex = ggui.getSelectorIndex();
			for (int i = 0;i<6; i++)
			{
				photoElts[i]->draw(&ggui);
			}
			if (selectorIndex > 0)
			{
				if (ControlLeft())
					photoElts[selectorIndex]->applyPrev(aTimeEllapsed);
				if (ControlRight())
					photoElts[selectorIndex]->applyNext(aTimeEllapsed);
			}
			else
			{
				// combo ship
				bool changeShip= false;
				if (ControlLeftReleased())
				{
					photoElts[selectorIndex]->applyPrevReleased(aTimeEllapsed);
					changeShip = true;
				}
				if (ControlRightReleased())
				{
					photoElts[selectorIndex]->applyNextReleased(aTimeEllapsed);
					changeShip = true;

				}

				if (changeShip)
				{
					IPhysicShip *phys = GGame->GetShip(photoShip.currentItem)->GetPhysics();
					photoModeParams.mShipMatrix = phys->GetGroundMatrix();
					photoModeParams.mShipMatrix.position =phys->GetTransform().position;
				}
			}


			photoModeParams.mHeight = photoHeight.value;
			photoModeParams.mLateral = photoLateral.value;
			photoModeParams.mRoll = photoRoll.value;
			photoModeParams.mDOF = photoDOF.value * 0.3f;
			photoModeParams.mFocal = photoFocal.value;
			setDOFFocus( photoFocal.value * photoFocal.value);
			setDOFBlur( photoDOF.value );

			camera.view(photoModeParams);
		}
		break;
	case MENU_PHOTOMODE_CAMERAPLACEMENT:
		{
			if (ControlLeft())
				photoModeParams.mAngle -= aTimeEllapsed;
			if (ControlRight())
				photoModeParams.mAngle += aTimeEllapsed;
			if (KeyUpDown())
				photoModeParams.mDistance -= aTimeEllapsed * 3.f;
			if (KeyDownDown())
				photoModeParams.mDistance += aTimeEllapsed * 3.f;

			photoModeParams.mDistance = Clamp(photoModeParams.mDistance, 2.f, 8.f);
			camera.view(photoModeParams);
		}
		break;
	case MENU_MAIN:
		if ( inactivityTime > 20.f)
		{
			CreditsBackActive = false;
			Menus::Show(MENU_CREDITS);
		}
		break;
	case MENU_CREDITS:
		if ( inactivityTime < 0.1f )
		{
			CreditsBackActive = true;
		}
		if ( CreditsBackActive && inactivityTime > 0.5f )
		{
			Menus::Show(MENU_MAIN);
		}

		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// E S C   IN MENU

void Menus::Esc()
{
	switch (mCurrentMenu)
	{
	case MENU_SEL_TEAM_CAREER:
		SoundGui("ButtonBack");
		ggui.clearBitmaps();
		Show(MENU_SOLO_MAIN);
		break;
	case MENU_SEL_EVOLUTION_CAREER:
		SoundGui("ButtonBack");
		ggui.clearBitmaps();
		Show(MENU_SEL_TEAM_CAREER);
		break;
	case MENU_HOWTOPLAY:
		SoundGui("ButtonBack");
		ggui.clearBitmaps();
		Show(MENU_MAIN);
		break;
	case MENU_HOWTOPLAY_SOLO:
		SoundGui("ButtonBack");
		ggui.clearBitmaps();
		Show(MENU_INGAME_SOLO);
		break;
	case MENU_HOWTOPLAY_NETWORK:
		SoundGui("ButtonBack");
		ggui.clearBitmaps();
		Show(MENU_INGAME_NETWORK);
		break;
	case MENU_SEL_EVOLUTION_QUICK:
		SoundGui("ButtonBack");
		Show(MENU_SEL_TEAM_QUICK);
		break;
	case MENU_SEL_TEAM_QUICK:
		SoundGui("ButtonBack");
		Show(MENU_QUICKRACE_TRACKSEL);
		break;
	case MENU_SPLIT_TRACKSEL:
		SoundGui("ButtonBack");
		Show(MENU_MAIN);
		break;
	case MENU_SEL_EVOLUTION_CREATESERVER:
		SoundGui("ButtonBack");
		Show(MENU_SEL_TEAM_CREATESERVER);
		break;
	case MENU_SEL_EVOLUTION_JOINSERVER:
		SoundGui("ButtonBack");
		Show(MENU_SEL_TEAM_JOINSERVER);
		break;
	case MENU_SEL_TEAM_CREATESERVER:
		SoundGui("ButtonBack");
		Show(MENU_MULTI);
		break;
	case MENU_SEL_TEAM_JOINSERVER:
		SoundGui("ButtonBack");
		Show(MENU_MULTI);
		break;
	case MENU_CONNECTED_WAITING:
	case MENU_LOBBY_JOINING:
		NetClient->Close();
		Show(MENU_LOBBY_JOIN);
		ASSERT_GAME( IsLanDiscoveryInProgress() );
		break;
	case MENU_LOBBY_CREATE:
		GGame->Restart();
		NetServer->Close();
		NetClient->Close();
		ASSERT_GAME( !IsLanDiscoveryInProgress() );
		SoundGui("ButtonBack");
		Show(MENU_MAIN);
		break;
	case MENU_LOBBY_JOIN:
		StopLanDiscovery();
		SoundGui("ButtonBack");
		Show(MENU_MAIN);
		break;
	case MENU_STATS: //stats
		SoundGui("ButtonBack");
		Show(MENU_MAIN);
		break;
	case MENU_SETUP_MAIN: // setup
		SoundGui("ButtonBack");
		SetEngineConfig(menuConfigBackup);
		Show(MENU_MAIN);
		break;
	case MENU_MAIN: // mainmenu
		ggui.clearBitmaps();
		SoundGui("ButtonBack");
		Show(MENU_WANTOQUIT_MAIN);
		break;
	case MENU_WANTOQUIT_MAIN: // quit?
		SoundGui("ButtonAsk");
		Show(MENU_MAIN);
		break;
	case MENU_MULTI:
		SoundGui("ButtonBack");
		Show(MENU_MAIN);
		break;
		// du main menu
	case MENU_QUICKRACE_TRACKSEL:
		SoundGui("ButtonBack");
		//Renderer::newTransition();
		//quitTrackSelection();
		Show(MENU_MAIN);
		Renderer::SetOverGUIMesh( NULL );
		break;
	case MENU_SOLO_MAIN:
		{
			extern bool bMicroView;
			if (!bMicroView)
			{
				SoundGui("ButtonBack");

				Solo::UninitMenu();

				Show(MENU_MAIN);
			}
		}
		break;
		// in game menu
	case MENU_EMPTY_SOLO:
		SoundGui("ButtonBack");
		Audio::PlayPauseGroup( AUDIO_GROUP_GAME3D, false ); 
		Audio::PlayPauseGroup( AUDIO_GROUP_GAME2D, false ); 
		Show(MENU_INGAME_SOLO);
		break;
	case MENU_INGAME_SOLO:
		SoundGui("ButtonBack");
		Show(MENU_EMPTY_SOLO);
		Audio::PlayPauseGroup( AUDIO_GROUP_GAME3D, true ); 
		Audio::PlayPauseGroup( AUDIO_GROUP_GAME2D, true ); 
		break;
	case MENU_EMPTY_NETWORK:
		SoundGui("ButtonBack");
		Show(MENU_INGAME_NETWORK);
		break;
	case MENU_INGAME_NETWORK:
		SoundGui("ButtonBack");
		Show(MENU_EMPTY_NETWORK);
		break;
	case MENU_SETUP_SOLO:
		SoundGui("ButtonBack");
		Show(MENU_INGAME_SOLO);
		break;
	case MENU_SETUP_NETWORK:
		SoundGui("ButtonBack");
		Show(MENU_INGAME_NETWORK);
		break;
	case MENU_WANTOQUIT_SOLO:
		SoundGui("ButtonBack");
		Show(MENU_INGAME_SOLO);
		break;
	case MENU_WANTOQUIT_NETWORK:
		SoundGui("ButtonBack");
		Show(MENU_INGAME_NETWORK);
		break;
	case MENU_WANTRESTARTRACE_SOLO:
		SoundGui("ButtonBack");
		Show(MENU_INGAME_SOLO);
		break;
		// photo mode
	case MENU_PHOTOMODE:
		SoundGui("ButtonBack");
		Show(MENU_INGAME_SOLO);
		setDOFBlur(0.f);
		//camera.lockOnMatrix( &GGame->GetPlayerShip()->GetPhysics()->GetTransform() );
		camera.SetCameraBehindShip( GGame->GetPlayerShip() );
		break;
	case MENU_PHOTOMODE_CAMERAPLACEMENT:
	case MENU_PHOTOMODE_PHOTOSAVED:
		SoundGui("ButtonBack");
		Show(MENU_PHOTOMODE);
		break;
	case MENU_SEL_TEAM_SPLIT:
	case MENU_SEL_EVOLUTION_SPLIT:
		splitScreenEnabled = true;
		SoundGui("ButtonBack");
		Show(MENU_MAIN);
		break;

#if 0
	case MENU_SELSHIP_QUICK:
		Show(MENU_MAIN);
		break;
	case MENU_SELSHIP_CAREER:
	case MENU_SELSHIP_CREATESERVER:
	case MENU_SELSHIP_JOINSERVER:
		SoundGui("ButtonBack");
		Show( MENU_MAIN );
		break;
#endif
	default:
		break;
	}
}
