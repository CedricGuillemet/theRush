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
#include "solo.h"

#include "mesh.h"
#include "camera.h"
#include "world.h"
#include "gui.h"
#include "render.h"
#include "therush.h"
#include "content.h"
#include "fx.h"
#include "track.h"
#include "game.h"
#include "track.h"
#include "menus.h"
#include "audio.h"
#include "physics.h"

#include "include_SDL.h"

/*
#define _C(x, y) race_t(1, x, y)
// accelerator (track, temps gold, temps silver, temps bronze)
#define _A(x,y,z,w) race_t(2, x, y, z, w)
// eleminator (track)
#define _E(x) race_t(3, x)
// Destruction (track, nblaps)
#define _D(x,y) race_t(4, x, y)
// Cover (track, nbPoints)
#define _K(x, y) race_t(5, x, y, 4)
// YY (track)
#define _Y(x) race_t(6, x)
*/

// evolutions

const unsigned int evolutionThresholds[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128 };

unsigned int profil_t::GetEvolutionCount()
{
    return ( sizeof(evolutionThresholds) / sizeof( unsigned int ) );
}

//FIXME: evolutions array size shouldn't be hardcoded but use a const variable / enum
//'TeamCount' already defined in content.cpp, but should be declared somewhere visible from other *.cpp / *.h files
unsigned int profil_t::GetEvolutionIndex( int teamIndex )
{
    ASSERT_GAME( 0 <= teamIndex && teamIndex < 4);

	// stunfest points unlocking
    const unsigned int nbPoints = 255;//evolutions[teamIndex];
    
    const unsigned int evolutionCount = GetEvolutionCount();
    for ( unsigned int i = 0 ; i < evolutionCount ; i ++ )
    {
        if ( nbPoints <= evolutionThresholds[i] )
            return i;
    }
    return ( evolutionCount - 1 );
}

unsigned int profil_t::GetPointNeededForEvolution( int evolutionIndex )
{
    const int evolutionCount = GetEvolutionCount();

    if (evolutionIndex >= evolutionCount )
        evolutionIndex = evolutionCount-1;

    return evolutionThresholds[ evolutionIndex ];
}

// races



typedef struct level_t
{
	const char* name;
	unsigned short numberOfPointsToUnlock;
	unsigned int startupX, startupY;
	race_t races[64];
} level_t;


extern level_t levels[LEVELS_COUNT];
race_t* currentRace = NULL;


int GetRandomGameMode( int levelIndex )
{
    static int levelRandom = 0;
    levelRandom ++;

    if ( levelIndex == 0 )
        return levelRandom%5;
    else if ( levelIndex == 1 )
        return levelRandom%7;
    else
        return levelRandom%9;
}

int playCount[32]={0};
int envCount[4] = { 0, 0, 0, 0 };
void distributeLevel( int levelIndex, const u8* authorizedTracks, int authorizedTrackCount,
                     const u8* gameModes, int gameModesCount, u8 nbLapMin, u8 nbLapMax, float speedFactorMin,
                     float speedFactorMax)
{
    UNUSED_PARAMETER(gameModesCount);
    UNUSED_PARAMETER(gameModes);

    level_t *plevel = &levels[ levelIndex ];
    int playAv = 0;
    for (int i = 0;i<64;i++)
    {
        race_t& r = plevel->races[ i ];
        if ( r.type )
        {

            r.speed = LERP( speedFactorMin, speedFactorMax, r01() );
            r.nbLaps = nbLapMin + fastrand() % ( nbLapMax - nbLapMin );
            r.track = authorizedTracks[ (++playAv) % authorizedTrackCount ];
            r.type = static_cast<char>( GetRandomGameMode( levelIndex ) );//gameModes[ fastrand() % gameModesCount ] + 1;
            
            // index
            r.mLevel = levelIndex;
            r.mRaceIndex = i;
            
            playCount[r.track] ++;
            LOG(" tr %d laps %d type %d spd %5.2f\n", r.track, r.nbLaps, r.type, r.speed );
        }
    }
}
void Solo::DistributeAllLevels( )
{
    // 0 -6 : Artic
    // 7 - 16 : city
    // 16 - 23 : desert
    // 24 - 31 : space
    static const u8 envTracks[4][9]={
        {0,1,2,3,4,5,6}, // 7
        {7,8,9,10, 11,12,13,14,15}, // 9 
        {16,17,18,19,20,21,22,23},       //8  
        {24,25,26,27,28,29,30,31}//8
    };
    int trackPerEnvCount[]={ 7, 9, 8, 8 };
    
    int avPerEnv[4] = {0};
    
    vec_t envPars[12]=
    {
        vec(1.0f, 0.0f, 0.0f, 0.0f),
        vec(0.8f, 0.1f, 0.0f, 0.0f),
        vec(0.5f, 0.2f, 0.2f, 0.0f),
        vec(0.4f, 0.2f, 0.2f, 0.1f),        
        vec(0.2f, 0.4f, 0.1f, 0.2f),
        vec(0.1f, 0.45f, 0.3f, 0.2f),
        vec(0.1f, 0.2f, 0.4f, 0.3f),
        vec(0.0f, 0.1f, 0.4f, 0.4f),        
        vec(0.0f, 0.2f, 0.2f, 0.5f),
        vec(0.0f, 0.3f, 0.5f, 0.4f),
        vec(0.0f, 0.4f, 0.1f, 0.2f),
        vec(0.1f, 0.3f, 0.2f, 0.3f)
    };
    
    g_seed = 210177;
    for (int j=0;j<12;j++)
    {
        float avLevel = static_cast<float>(j)/11.f;
        u8 chosenTracks[8];
        const vec_t& ep = envPars[j];
        
        for (int i = 0;i<8;i++)
        {

            float envThresholds[4] = { ep.x+ep.y+ep.z, ep.x+ep.y, ep.x, 0.f };
            
            float renv = r01();
            int selectedEnv = 0;
            
            for (int k=0;k<4;k++)
            {
                if ( renv > envThresholds[k] )
                {
                    selectedEnv = 3-k;
                    break;
                }
            }
            envCount[selectedEnv] ++ ;
            int trackSelected = envTracks[selectedEnv][ avPerEnv[selectedEnv] ];
            ++avPerEnv[selectedEnv] %= trackPerEnvCount[selectedEnv];

            chosenTracks[i] = static_cast<u8>( trackSelected );
        }
        u8 authModes[]={0};
        
        static const u8 MinLapCount[12]={3,3,3,3,4,4,4,4,5,5,6,6};
        static const u8 MaxLapCount[12]={4,4,5,5,5,5,6,6,6,7,8,9};
        
        LOG("Level %d ------- \n",j);
        distributeLevel( j, chosenTracks, 7, authModes, 1, MinLapCount[j], MaxLapCount[j], LERP(0.8f, 1.2f, avLevel), LERP(0.85f, 1.3f, avLevel) );
        LOG("---------------- \n");
    }
    
    for (int i=0;i<32;i++)
    {
        LOG("Track %d played %d times\n", i, playCount[i] );
    }
    for (int i=0;i<4;i++)
    {
        LOG("Env %d played %d times\n", i, envCount[i] );
    }
    
    
}



#define _R race_t()
/*
// classic (track, nblaps)
#define _C(x, y) race_t(1, x, y)
// accelerator (track, temps gold, temps silver, temps bronze)
#define _A(x,y,z,w) race_t(2, x, y, z, w)
// eleminator (track)
#define _E(x) race_t(3, x)
// Destruction (track, nblaps)
#define _D(x,y) race_t(4, x, y)
// Cover (track, nbPoints)
#define _K(x, y) race_t(5, x, y, 4)
*/
// fake fill
#define _L race_t(1)
// YY (track)
//#define _Y(x) race_t(6, x)

///////////////////////////////////////////////////////////////////////////////////////////////////

level_t levels[12] = {
	{"Hucury",0,2,3,{
	_R   ,_L		,_L		,_R		,_R 	,_R   ,_R   ,_R   ,
	_R   ,_L		,_L,    _R		,_R		,_R   ,_R   ,_R   ,
	_R   ,_L		,_L		,_L		,_R		,_R   ,_R   ,_R   ,
	_R   ,_L		,_L		,_L		,_R		,_R   ,_R   ,_R   ,
	_R   ,_L		,_L		,_L		,_L		,_R   ,_R   ,_R   ,
	_R   ,_R		,_L		,_L		,_L		,_R   ,_R   ,_R   ,
	_R   ,_R		,_R		,_R		,_L		,_R   ,_R   ,_R   ,
	_R   ,_R		,_R		,_R		,_R		,_R   ,_R   ,_R   }},

	{"Uslone",1,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_R   ,_R   ,_R   ,_L,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_L,_L,_R   ,_R   ,
	_R   ,_L,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_L,_L,_R   ,_R   ,
	_R   ,_L,_R   ,_R   ,_R   ,_L,_R   ,_R   }},
	{"Acrone",2,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_L,_L,_R   ,_R   ,
	_R   ,_L,_R   ,_R   ,_R   ,_L,_R   ,_R   
	}},
	{"Vatune",3,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   }},
	{"Eichuna",4,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   }},
	{"Tadoubos",5,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   }},
	{"Seclite",6,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_R   ,_L,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_L,_L,_R   ,_R   ,
	_R   ,_L,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   }},
	{"Deruta",7,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,_R   }},
	{"Obroren",8,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_R   ,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,_R   }},
	{"Wolara",9,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_R   ,_L,_R   ,_L,_R   ,_R   ,
	_R   ,_L,_R   ,_L,_R   ,_L,_R   ,_R   ,
	_R   ,_L,_R   ,_L,_R   ,_L,_R   ,_R   ,
	_R   ,_L,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_R   ,_L,_L,_L,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   }},
	{"Utriri",10,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_L,_L,_R   ,_R   ,
	_R   ,_L,_L,_L,_R   ,_L,_R   ,_R   ,
	_R   ,_L,_R   ,_L,_L,_L,_R   ,_R   ,
	_R   ,_L,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   }},
	{"Caiturn",11,0,0,{
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_R   ,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_L,_L,_R   ,_R   ,_R   ,_R   ,
	_R   ,_L,_R   ,_L,_R   ,_R   ,_R   ,_R   ,
	_L,_L,_R   ,_L,_L,_R   ,_R   ,_R,
	_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R   ,_R      }
	}
};




mesh_t *hexagons[12][64];



profil_t Solo::playerProfil;

int currentProfileIndex = 0;
int currenLevel = 0;

bool bFirstRender = false;
int currentMacroSelectorIndex = 0;
matrix_t levelMatrices[12];
bool bMicroView = false;

static const vec_t soloMoveUp = vec(0.f, 2.f, 0.f);

static vec_t StatusColors[5] = { vec(0.4f, 0.4f, 0.4f, 0.43f), // locked
    vec(0.65f, 0.65f, 0.65f, 0.5f), // unlocked
    vec(186.f/255.f, 160.f/255.f, 73.f/255.f, 0.65f), // bronze,
    vec(186.f/255.f, 171.f/255.f, 119.f/255.f, 0.65f), // silver
    vec(245.f/255.f, 245.f/255.f, 29.f/255.f, 0.65f) // gold
    
};

// points

int getPointsDoneInLevel(int levelIndex);
int getPointsPossibleInLevel(int levelIndex);
int getPointsDoneInAllLevels();
int getPointsPossibleInAllLevels();

//extern sf::Sound *sndValid, *sndAsk, *sndBack, *sndLocked;
///////////////////////////////////////////////////////////////////////////////////////////////////

void unlockRacesAround(int levelIndex, int x, int y)
{
	char * status = Solo::playerProfil.levelRacesStatus[levelIndex];
	if (!status[y * 8 + x])
		status[y * 8 + x] = 1;
	if ( ( (y-1)>0) && (!status[(y-1) * 8 + x]) )
		status[(y-1) * 8 + x] = 1;
	if ( ((y+1)<8) && (!status[(y+1) * 8 + x]) )
		status[(y+1) * 8 + x] = 1;

	if ( ((y+1)<8) && ((x+1)<8) && (!status[(y+1) * 8 + (x+1)]) )
		status[(y+1) * 8 + (x+1)] = 1;
	if ( ((x+1)<8) && (!status[(y) * 8 + (x+1)]) )
		status[(y) * 8 + (x+1)] = 1;

	if ( ((y+1)<8) && ((x-1)>0) && (!status[(y+1) * 8 + (x-1)]) )
		status[(y+1) * 8 + (x-1)] = 1;
	if ( ((x-1)>0) && (!status[(y) * 8 + (x-1)]) )
		status[(y) * 8 + (x-1)] = 1;
}

void Solo::CreateProfil()
{
	memset (&playerProfil, 0, sizeof(profil_t) );
    playerProfil.cameraMode[1] = playerProfil.cameraMode[0] = 1;
    
	//profils[profileIndex].levelRacesStatus[0][3 * 8 + 2] = 1;
	unlockRacesAround(0, 2, 3);
}

int getPointsDoneInLevel(int levelIndex)
{
	int res = 0;
	char * status = Solo::playerProfil.levelRacesStatus[levelIndex];
	for (int i=0;i<64;i++)
	{
		if (status[i]>1)
		{
			res += status[i]-1;
		}
	}

	return res;
}

int getPointsPossibleInLevel(int levelIndex)
{
	int res = 0;
	race_t * racesT = levels[levelIndex].races;
	for (int i=0;i<64;i++)
	{
		res += racesT[i].type?3:0;
	}

	return res;
}

int getPointsDoneInAllLevels()
{
	int res = 0;
	for (int i=0;i<12;i++)
		res += getPointsDoneInLevel(i);

	return res;
}

int getPointsPossibleInAllLevels()
{
	int res = 0;
	for (int i=0;i<12;i++)
		res += getPointsPossibleInLevel(i);

	return res;
}


void buildLevelHexagons(level_t *pLevel, int levelIndex, const matrix_t& mat)
{
    float hheight = 0.6f;
    float hwidth = 1.f;
    float hrenf = 0.3f;
    mesh_t* ghexa = generateHexagon(hwidth * 0.9f, hheight * 0.9f, hrenf);
    
    
	for (int y = 0;y<8;y++)
	{
		for (int x= 0;x<8;x++)
		{
			int type = pLevel->races[y*8+x].type;
			
			if (!type)
            {
                hexagons[levelIndex][y*8+x] = NULL;
				continue;
            }
				

			
			matrix_t tr, sc, mvUp;

			tr.translation( vec((hwidth-hrenf) * (float)(7-x) - (4 * (hwidth-hrenf)), 
				1.f + (float)((7-y) * hheight) + ((x&1)?hheight*0.5f:0.f), 0.f) );
			mvUp.translation( soloMoveUp );
			sc.scale(1.f, 1.f, 0.1f);

            mesh_t* hexa = ghexa->clone();
            
			hexa->mWorldMatrix = hexa->mBaseMatrix = sc * tr * mat * mvUp;
			hexa->updateWorldBSphere();
			int raceStatus =  Solo::playerProfil.levelRacesStatus[levelIndex][y*8+x];

			hexagons[levelIndex][y*8+x] = hexa;

			hexa->color = StatusColors[ raceStatus ];//   vec(1.f, 1.f, 1.f, 0.5f);
			hexa->visible = true;
			hexa->physic = false;
		}
	}
    delete ghexa;
    ghexa = NULL;
}

void showMacroMenu()
{
	ggui.clearText();
	// menu
	int menuPos[12];//={GC(1,33), GC(1,32), GC(1,31), GC(1,30), GC(1,29), GC(1,28), GC(1,27), GC(1,26), GC(1,25), GC(1, 24), GC(1, 23)};
			
	ggui.putText(7, 34, "select league" );
	for (int i=0;i<12;i++)
	{
		ggui.putText(6, 20 + i, levels[11-i].name );
		menuPos[11-i] = GC(4, (20 + i));
	}
	ggui.addSelector(menuPos, 12);
	ggui.setSelectorIndex(currentMacroSelectorIndex);
	ggui.setBlackBox(2,0,22,36);

	char pointsInfos[512];
	sprintf(pointsInfos," %d points / %d      ", getPointsDoneInAllLevels(), getPointsPossibleInAllLevels());
	ggui.putText(6, 3, pointsInfos);
}

void showMicroMenu(race_t *pRace, char status)
{
	ggui.clearText();
	ggui.addSelector(0, 0);
	ggui.setBlackBox(34,0,25,36);


	char pointsInfos[512];
	sprintf(pointsInfos," %d points / %d      ", getPointsDoneInLevel(currenLevel), getPointsPossibleInLevel(currenLevel));
	ggui.putText(38, 3, pointsInfos);

	char tmps[512];
    sprintf(tmps,".track  %s", Tracks[pRace->track].mName.c_str() );
	ggui.putText( 36, 28, tmps);
    ggui.putText( 40, 27, GetEnvName(Tracks[pRace->track].mEnvironmentIndex) );    
	
	sprintf(tmps,".mode   %s", GameTypes[pRace->type-1].szName);
	ggui.putText(36, 25, tmps);

	const char *szStatusText;
	switch (status)
	{
		case 0: szStatusText = "locked"; break;
		case 1: szStatusText = "unlocked"; break;
		case 2: szStatusText = "bronze awarded"; break;
		case 3: szStatusText = "silver awarded"; break;
		case 4: szStatusText = "gold awarded"; break;
		default: szStatusText = NULL; ASSERT_GAME(0);
	}
	sprintf(tmps,".status %s", szStatusText);
	ggui.putText(36, 23, tmps);
	

    static const int startInfoLine = 21;
	switch(pRace->type-1)
	{
	case 0: // classic race
		sprintf(tmps,".%d laps", pRace->nbLaps);
		ggui.putText(36, startInfoLine, tmps);
		break;
	case 1: // Accelerator
		sprintf(tmps,".%.0f seconds // Gold", pRace->goldTime);
		ggui.putText(36, startInfoLine, tmps);
		sprintf(tmps,".%.0f seconds // Silver", pRace->silverTime);
		ggui.putText(36, startInfoLine-1, tmps);
		sprintf(tmps,".%.0f seconds // Bronze", pRace->bronzeTime);
		ggui.putText(36, startInfoLine-2, tmps);

		break;
	case 2: // Eliminator
		break;
	case 3: // Destruction
		sprintf(tmps,".%d laps", pRace->nbLaps);
		ggui.putText(36, startInfoLine, tmps);
		break;
	case 4: // Cover
		sprintf(tmps,".%d points // first", pRace->coverPoints);
		ggui.putText(36, startInfoLine, tmps);
		break;
	case 5: // YingYang
		break;
	}
    //mini mesh map
    Renderer::SetOverGUIMesh( Tracks[ pRace->track ].mMiniMesh );
    Renderer::SetOverGUIMeshRectangle( vec( 37, 5, 20, 20 ) );    

}

std::vector<unsigned int> backToNormal;
mesh_t* currentlyHighlighted = NULL;
int currentRaceX, currentRaceY;
bool soloUp(false), soloDown(false), soloLeft(false), soloRight(false), soloBack(false), soloEnter(false);
///////////////////////////////////////////////////////////////////////////////////////////////////

void Solo::InitMenu()
{
	soloUp = soloDown = soloLeft = soloRight = soloBack = soloEnter = false;
    PROFILER_START(buildWorldForSoloMenus);
    
	//PushMeshStack();
	//Renderer::setRenderMeshStack(GetMeshStack());
	// compute minimal points to unlock one level

	levels[0].numberOfPointsToUnlock = 0;
	for (int i = 1;i<12;i++)
	{
		int lvlm1 = getPointsPossibleInLevel(i-1)/3;
		int nbPart = lvlm1 /5;
		levels[i].numberOfPointsToUnlock = static_cast<unsigned short>( levels[i-1].numberOfPointsToUnlock + nbPart * 3 + nbPart * 2 + nbPart );
	}

	// --
	bFirstRender = true;

	currentlyHighlighted = NULL;
	camera.SetCameraCustom();


	for (int i=0;i<12;i++)
	{
        AIPoint_t aipt = track.getAIPoint( 20 + i * 5);
        
        matrix_t trup, aimat, res;
        aipt.BuildMatrix( aimat, false );
        trup.translation(0.f, -20.f, 0.f);
        
        res = aimat;

		buildLevelHexagons(&levels[i], i, res );
		levelMatrices[i] = res;
	}


	showMacroMenu();
	//enableDOF();
	setDOFBlur(0.08f);


    PROFILER_END(); // buildWorldForSoloMenus
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Solo::UninitMenu()
{
    for (int i=0;i<12;i++)
    {
        for (int y = 0;y<8;y++)
        {
            for (int x= 0;x<8;x++)
            {
                mesh_t* hexa = hexagons[i][y*8+x];
                if ( hexa )
                {
                    delete hexa;
                    hexa = NULL;
                    hexagons[i][y*8+x] = NULL;
                }
            }
        }
    }
    Renderer::SetOverGUIMesh( NULL );    
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Solo::Tick( float aTimeEllapsed )
{
	static float av = 0.f;
	static float parametricAv = 0.f;

	av= LERP( av, (float)currenLevel, aTimeEllapsed*5.f);
	parametricAv += aTimeEllapsed*0.03f;


	if (!bMicroView)
		currenLevel = ggui.getSelectorIndex();

	vec_t parametricMove = vec( cosf(3.f * parametricAv) * 2.f * sinf(parametricAv*0.5f), 
		5.f * cosf(2.f * parametricAv) * 1.5f * sinf(parametricAv*2.5f),
		1.2f * cosf(1.2f * parametricAv) * 0.8f * sinf(parametricAv*1.5f) ) * 0.02f;

	vec_t eyeMacro = vec(2.7f, 2.7f + av* 0.12f, -3.f) + parametricMove;
	vec_t targetMacro = vec(1.f, 3.f, 10.f) + parametricMove;
	

	vec_t eyeMicro = vec(-1.5f, 3.2f, -2.8f) + parametricMove;
	vec_t targetMicro = vec(-1.7f, 3.1f, 10.f) + parametricMove;


	int mat1 = (int)av;
	int mat2 = mat1+1;
	float frav = fmodf(av, 1.f);

	matrix_t it;
	it.lerp( levelMatrices[mat1], levelMatrices[mat2], frav );
	it.orthoNormalize();

	eyeMacro.TransformPoint( it );
	targetMacro.TransformPoint( it );

	eyeMicro.TransformPoint( it );
	targetMicro.TransformPoint( it );

	vec_t targetEye, TargetTarget;
	if (bMicroView)
	{
		targetEye = eyeMicro;
		TargetTarget = targetMicro;
	}
	else
	{
		targetEye = eyeMacro;
		TargetTarget = targetMacro;
	}
	static vec_t currentEye, currentTarget;
	if (bFirstRender)
	{
		currentEye = targetEye;
		currentTarget = TargetTarget;
		bFirstRender = false;
	}
	else
	{
		currentEye = LERP(currentEye, targetEye, aTimeEllapsed*5.f);
		currentTarget = LERP(currentTarget, TargetTarget, aTimeEllapsed*5.f);
	}

	setDOFFocus(bMicroView?2.9f:3.1f);

	camera.view(currentEye + soloMoveUp, currentTarget + soloMoveUp, it.up );//vec(0.f, 1.f, 0.f));

	if (!bMicroView)
	{
        Renderer::SetOverGUIMesh( NULL );
        
		if (playerProfil.numberOfPoints < levels[currenLevel].numberOfPointsToUnlock)
		{
			ggui.putText(8, 12, "//Locked\\\\");
			char tmps[512];
			sprintf(tmps, "%d points to unlock  ", levels[currenLevel].numberOfPointsToUnlock);
			ggui.putText(3, 11, tmps);


		}
		else
		{
			ggui.putText(8, 12, "               ");
			ggui.putText(3, 11, "                    ");

			if ( soloEnter )
			{
				SoundGui("ButtonValid");

				currentMacroSelectorIndex = ggui.getSelectorIndex();
				bMicroView = true;
				
				currentRaceX = levels[currenLevel].startupX;
				currentRaceY = levels[currenLevel].startupY;
                
                int idxBackTo = ( currenLevel << 16 ) + ( currentRaceY << 8 ) + currentRaceX;
				backToNormal.push_back( idxBackTo );
                
				currentlyHighlighted = hexagons[currenLevel][currentRaceY * 8 + currentRaceX];
			}
		}
	}
	else
	{


		if ( soloLeft )
		{
			if (((currentRaceX-1)>=0) && (levels[currenLevel].races[currentRaceY * 8 + currentRaceX-1].type))
			{
                int idxBackTo = ( currenLevel << 16 ) + ( currentRaceY << 8 ) + currentRaceX;
				backToNormal.push_back( idxBackTo );
				currentRaceX --;
				currentlyHighlighted = hexagons[currenLevel][currentRaceY * 8 + currentRaceX];
			}
		}
		if ( soloRight )
		{
			if (((currentRaceX+1)<8) && (levels[currenLevel].races[currentRaceY * 8 + currentRaceX+1].type))
			{
                int idxBackTo = ( currenLevel << 16 ) + ( currentRaceY << 8 ) + currentRaceX;
				backToNormal.push_back( idxBackTo );
				currentRaceX ++;
				currentlyHighlighted = hexagons[currenLevel][currentRaceY * 8 + currentRaceX];
			}
		}
		if (soloUp)
		{
			if (((currentRaceY-1)>=0) && (levels[currenLevel].races[(currentRaceY-1) * 8 + currentRaceX].type))
			{
                int idxBackTo = ( currenLevel << 16 ) + ( currentRaceY << 8 ) + currentRaceX;
				backToNormal.push_back( idxBackTo );
				currentRaceY --;
				currentlyHighlighted = hexagons[currenLevel][currentRaceY * 8 + currentRaceX];
			}
		}
		if (soloDown)
		{
			if (((currentRaceY+1)<8) && (levels[currenLevel].races[(currentRaceY+1) * 8 + currentRaceX].type))
			{
                int idxBackTo = ( currenLevel << 16 ) + ( currentRaceY << 8 ) + currentRaceX;
				backToNormal.push_back( idxBackTo );
				currentRaceY ++;
				currentlyHighlighted = hexagons[currenLevel][currentRaceY * 8 + currentRaceX];
			}
		}

		char playerRaceStatus = playerProfil.levelRacesStatus[currenLevel][currentRaceY * 8 + currentRaceX];

		if (soloBack)
		{
            SoundGui("ButtonBack");
			if (currentlyHighlighted)
            {
                int idxBackTo = ( currenLevel << 16 ) + ( currentRaceY << 8 ) + currentRaceX;
				backToNormal.push_back( idxBackTo );                
//				backToNormal.push_back(currentlyHighlighted);
            }

			currentlyHighlighted = NULL;
			// run race
			bMicroView = false;
			showMacroMenu();
		}
		else
		{
			
			showMicroMenu(&levels[currenLevel].races[currentRaceY * 8 + currentRaceX], playerRaceStatus);
		}

		if ( soloEnter )
		{
			
			if (playerRaceStatus)
			{
                SoundGui("ButtonValid");
                Solo::UninitMenu();
                
                // set game
                currentRace = &levels[currenLevel].races[currentRaceY * 8 + currentRaceX];                
                
                Menus::Show( MENU_SEL_TEAM_CAREER );
				return;
			}
			else
			{
                SoundGui("ButtonError");
				// son
				//sndLocked->Play();
			}
		}
	}

	if (currentlyHighlighted)
	{
		float lerpFactor = aTimeEllapsed * 7.f;
		matrix_t res = currentlyHighlighted->mBaseMatrix;
		
		matrix_t sc;
		sc.scale(1.35f, 1.35f, 0.1f);
		res.position -= res.dir * 1.5f;
		res = sc * res;
		
		currentlyHighlighted->mWorldMatrix.lerp(currentlyHighlighted->mWorldMatrix, res, lerpFactor);
        //currentlyHighlighted->mWorldMatrix.orthoNormalize();
		currentlyHighlighted->updateWorldBSphere();


		const vec_t& col = currentlyHighlighted->color;
		currentlyHighlighted->color.lerp(col, vec(col.x, col.y, col.z, 0.75f), lerpFactor);
	}

	if (!backToNormal.empty())
	{
		float lerpFactor = aTimeEllapsed * 7.f;
		if (lerpFactor >FLOAT_EPSILON)
		{
			std::vector<unsigned int>::iterator iter = backToNormal.begin();
			for (; iter != backToNormal.end() ; )
			{
                unsigned int backToIdx = (*iter);
                unsigned int backToLevel = backToIdx>>24;
                unsigned int backToY = (backToIdx>>8)&0xFF;
                unsigned int backToX = backToIdx&0xFF;
                
                int base64Idx = backToY*8+backToX;
				mesh_t *pm = hexagons[backToLevel][base64Idx];//(*iter);
			
				pm->mWorldMatrix.lerp(pm->mWorldMatrix, pm->mBaseMatrix, lerpFactor);
                //pm->mWorldMatrix.orthoNormalize();
				pm->updateWorldBSphere();

                
                int raceStatus =  playerProfil.levelRacesStatus[backToLevel][base64Idx];
                vec_t destCol = StatusColors[ raceStatus ];//   vec(1.f, 1.f, 1.f, 0.5f);

                
                
				const vec_t& col = pm->color;
				pm->color.lerp(col, destCol, lerpFactor);
			
                float distanceColor = (destCol-pm->color).length();
                if (distanceColor < 0.001f)
					iter = backToNormal.erase(iter);
				else
					++iter;
			}
		}
	}
	soloUp = soloDown = soloLeft = soloRight = soloBack = soloEnter = false;
}

void Solo::ApplyCurrentRace()
{
	Menus::Show(MENU_EMPTY_SOLO);
    GGame->DestroyGameObjects( );

    GGame->ResetNetIndexAllocator();

    physicWorld.ClearCollisions();

    Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME2D );
    Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME3D );

    GGame->SetType( currentRace->type - 1 );
    GGame->SetTrack( currentRace->track );
    GGame->SetNumberOfLapsToDo( currentRace->nbLaps );
	GGame->SetSplit( false );
    GGame->SetRace( currentRace );
}

int Solo::TagRaceDone( race_t *pRace, int aRank )
{
    // aRank is 0 indiced
    
    char previousRaceState = playerProfil.levelRacesStatus[pRace->mLevel][pRace->mRaceIndex];
    char newState = static_cast<char>( 4 - aRank );
    
    if ( newState > previousRaceState )
        playerProfil.levelRacesStatus[pRace->mLevel][pRace->mRaceIndex] = newState;
    
    if ( aRank <= 3 )
    {
        unlockRacesAround( pRace->mLevel, pRace->mRaceIndex&7, (pRace->mRaceIndex>>3) );
        if ( previousRaceState == 1 )
            return 1;
    }
    
    return 0;
}

bool Solo::AddEvolutionPoints( unsigned int aTeamIndex, unsigned int nbPoints )
{
    ASSERT_GAME( ( aTeamIndex < 4 ) );
    unsigned int currentEvolution = playerProfil.GetEvolutionIndex( aTeamIndex );
    playerProfil.evolutions[ aTeamIndex ] += static_cast<u16>( nbPoints );
    unsigned int newEvolution = playerProfil.GetEvolutionIndex( aTeamIndex );    
    ASSERT_GAME( ( currentEvolution <= newEvolution ) );
    return ( newEvolution != currentEvolution );
}
