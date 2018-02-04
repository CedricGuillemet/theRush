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

#ifndef MENUS_H__
#define MENUS_H__

///////////////////////////////////////////////////////////////////////////////////////////////////

enum MENU_IDS
{
	MENU_NONE,
	MENU_MAIN,
	MENU_WANTOQUIT_MAIN,
	MENU_SETUP_MAIN,
	MENU_STATS,
	MENU_EMPTY_SOLO,
	MENU_EMPTY_NETWORK,
	MENU_INGAME_SOLO,
	MENU_INGAME_NETWORK,
	MENU_PHOTOMODE,
	MENU_SETUP_SOLO,
	MENU_SETUP_NETWORK,
	MENU_WANTOQUIT_SOLO,
    MENU_WANTOQUIT_NETWORK,
	MENU_LOADING,
	MENU_PHOTOMODE_CAMERAPLACEMENT,
	MENU_PHOTOMODE_PHOTOSAVED,
	MENU_MULTI,
	MENU_SOLO_MAIN,
	MENU_WANTRESTARTRACE_SOLO,
    MENU_WANTRESTARTRACE_NETWORK,
	MENU_QUICKRACE_TRACKSEL,
	MENU_SPLIT_TRACKSEL,
	MENU_SOLO_PRECOUNTDOWN,

	MENU_SPLIT_PRECOUNTDOWN,
    /*
	MENU_SELSHIP_QUICK,
	MENU_SELSHIP_CAREER,
	MENU_SELSHIP_CREATESERVER,
	MENU_SELSHIP_JOINSERVER,
    */
	MENU_LOBBY_JOIN,
	MENU_LOBBY_CREATE,

	MENU_LOBBY_JOINING,
	MENU_LOBBY_JOINERROR,

	MENU_ENDRACE_SOLO,
    MENU_ENDRACE_NETWORK,
	MENU_ENDRACE_SPLIT,
    MENU_INTRO,

    MENU_WAITING_BUDDIES,
    MENU_CONNECTED_WAITING,

    MENU_CONNECTION_LOST,
    
    MENU_EVOLUTION_INFO,
    MENU_EVOLUTION_ACQUIRED,
    
    MENU_SEL_TEAM_QUICK,
    MENU_SEL_EVOLUTION_QUICK,

    MENU_SEL_TEAM_CAREER,
    MENU_SEL_EVOLUTION_CAREER,

    MENU_SEL_TEAM_CREATESERVER,
    MENU_SEL_EVOLUTION_CREATESERVER,

    MENU_SEL_TEAM_JOINSERVER,
    MENU_SEL_EVOLUTION_JOINSERVER,

    MENU_SEL_TEAM_SPLIT,
    MENU_SEL_EVOLUTION_SPLIT,

	MENU_CREDITS,
	MENU_HOWTOPLAY,

	MENU_HOWTOPLAY_SOLO,
	MENU_HOWTOPLAY_NETWORK,
};

class Menus
{
public:
    static void Init();
    static void Show( MENU_IDS menuNumber );
    static void Validate( int playerId = 0 );
    static void Esc();
    static void Tick(float aTimeEllapsed);
    static MENU_IDS GetCurrent();
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif