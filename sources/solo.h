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

#ifndef SOLO_H__
#define SOLO_H__

#define LEVELS_COUNT 12

typedef struct profil_t
{
    u8 version;
	char name[33]; //32 + 0
	char levelRacesStatus[LEVELS_COUNT][64]; //0 = locked 1 = unlocked 2 = bronze 3 = silver 4 = gold
	uint32 numberOfPoints;
	uint32 numberOfRacesDone;
	float totalDistanceDone;
	uint32 totalRaceTime; // in seconds
	//float averageSpeed; // computed
	uint32 totalBonusTaken; 
	uint32 totalSpeedBonusTaken;
	uint32 totalPerBonus[32];
	uint32 totalEnemyShipsDestroyed;
	uint32 totalAbortedRaces;
	uint32 totalNumberOfPointInCover;
	uint32 totalRacesDonePerGameType[16];
    u16 evolutions[4];
    u8 cameraMode[2];
    
    unsigned int GetEvolutionIndex( int teamIndex );
    unsigned int GetPointNeededForEvolution( int evolutionIndex );
    static unsigned int GetEvolutionCount();
    
    
} profil_t;

typedef struct race_t
{
	race_t() { type = 0; }    
    race_t(int _type) { type = static_cast<char>(_type); }
    
	char type; // 0 = none 1 = standard race
	unsigned short track;
	u8 nbLaps;
    float speed;
    
    // results related
	float goldTime, silverTime, bronzeTime;
	u32 coverPoints;
    
    // for tagging
    int mLevel, mRaceIndex;
} race_t;

// solo choose race
class Solo
{
public:
    static void DistributeAllLevels( );
    static void CreateProfil( );

    static void InitMenu( );
    static void Tick( float aTimeEllapsed );
    static void UninitMenu( );

    static profil_t playerProfil;
    
    static void ApplyCurrentRace( );
    
    // race done and return the number of points to add to team (1 if win something, 0 otherwise)
    static int TagRaceDone( race_t *pRace, int aRank ); 
    
    // add evolution points to the team. returns true is got new evolution
    static bool AddEvolutionPoints( unsigned int aTeamIndex, unsigned int nbPoints );
};
// --

extern bool soloUp, soloDown, soloLeft, soloRight, soloBack, soloEnter;

///////////////////////////////////////////////////////////////////////////////////////////////////


#endif