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

#ifndef CONTENT_H__
#define CONTENT_H__

//unsigned int generateDebugTexture();
class btCollisionShape;

void loadContent();
void saveContent();
struct mesh_t;

mesh_t *getSprite(unsigned int idx);

// slices
typedef struct shapeSegment_t
{
	float x1, y1;
	float x2, y2;
	u32 color;
} shapeSegment_t;
	
typedef struct shape_t
{
	float width;
	shapeSegment_t *mSegs[4];
	int mNbSegs[4];
} shape_t;

typedef struct shapePattern_t
{
	shape_t *shapeNear;
	shape_t *shapeFar;
	int mTriangulation;
	u32 colorSolidMesh;
	u32 colorTransparentMesh;
	/*
	on file les pointeurs, on les incrémente
	et on bake les vertex/index dans 1 array par type de mesh
	-> non, a cause du clip
	return NbVertices
	*/
	int GenerateMesh(u8 *vtDest, int fmt, u16* indDest,
		int &nbIndices,
		const matrix_t& mtNear, const matrix_t& mtFar,
		int triangulation, int shapeSegmentArray, /* 0= solid, 1=transparent,...*/
        float aWidthRatioNear, float aWidthRatioFar, 
        u32 lightColor = 0, u32 darkColor = 0, 
        int leftDarkStart = 0, int leftDarkCount = 0,
        int rightDarkStart = 0, int rightDarkCount = 0,
                     u32 wallColor = 0xFFFFFFFF
        ) const;
}shapePattern_t;

extern shapePattern_t shapePatterns[];


typedef struct shapeSequence_t
{
        typedef struct shapeSequenceEntry_t
        {
                shapePattern_t* pattern;
                bool bRepeatable;
        }shapeSequenceEntry_t;

        int nbEntries;
        bool isTransparent;
		unsigned int maxLength;
        shapeSequenceEntry_t entries[20];
} shapeSequence_t;

// datas
/*
int GetMiddleCount();
int GetSideCount();
int GetBackCount();
int GetNoseCount();
*/
void InitMeshes();
u32 getRandomShipCompound();
//mesh_t *getCompoundShip( u32 shipNb, vec_t& reactorPosition);

const mesh_t* getRenderingCompoundShip( u32 shipNb, vec_t& reactorPosition );
btCollisionShape* GetCompoundPhysicMesh( u32 shipNb );

mesh_t *getReactorMesh( u32 shipNb );

// ships
extern std::vector<std::string> GShipsName;
void CreateTankerOverCity( const vec_t& center, float r, float R, float d, float timeFactor );

// credits mesh
mesh_t * getCreditsMesh(unsigned int idx);
unsigned int getNextCreditsMeshIndex(unsigned int idx);
void initCreditMeshes();

// sounds
void LoadSounds();

// meshes

mesh_t *GetTankerMesh();
mesh_t *GetEnv4Anchor();
mesh_t *GetEnv4();
mesh_t *GetGeoSpherePrefab( int aLevel );
mesh_t *GetGeoSmoothedSpherePrefab( int aLevel );


// env names
const char *GetEnvName( unsigned int envIdx );
const char * GetTeamName( unsigned int idx );

// Startup Pylons


class StartupPylon
{
public:
	StartupPylon();
	~StartupPylon();

	void Init(const matrix_t & mat);
    void Clear();
    
	void Reset();
	void Tick( float mStartupLocalTime, float mNextSuLT, float aTimeEllapsed);
    
protected:
	static vec_t FireColors[];

	mesh_t *startupBoxes[4];

	
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// memory files
/*
struct memoryFiles_t
{
    uint32 dataSize;    
    u8* datas;
};
*/
// map of hash name and data

extern std::map< uint32, std::string > memoryFiles;
void buildMemoryFilesFromDirectory( const char *szDirectory, bool addJSON = true );
void buildMemoryFilesFromMemory( const unsigned char *memoryDatas );
const std::string& getFileFromMemory( const char *szFileName );
const std::string& getFileFromMemory( uint32 hash );

///////////////////////////////////////////////////////////////////////////////////////////////////
// track settup functions
class TrackTopology;
struct track_t;
struct PerSliceInfo_t;

void TrackSetupGuenin( track_t *pTrack, TrackTopology& , matrix_t*, matrix_t*, PerSliceInfo_t *, PerSliceInfo_t * );
void TrackSetupWorld( track_t *pTrack, TrackTopology& , matrix_t*, matrix_t*, PerSliceInfo_t *, PerSliceInfo_t * );


extern unsigned int GLensTexture;
extern unsigned int GSparkleTexture;
extern unsigned int GExplosionTexture;
extern unsigned int GPlasmaTexture;
extern unsigned int GElectricWallTexture;
extern unsigned int GRockNormalTexture;
extern unsigned int GRockDiffuseTexture[8];

void BindScreenAdsShader( int idx );
int GetScreenAdsShaderCount();

void StartThreadedInits();
void WaitForThreadedInits();

///////////////////////////////////////////////////////////////////////////////////////////////////

mesh_t* GetAdvertisingMesh( unsigned int idx );
unsigned int GetAdvertisingMeshCount( );
matrix_t GetAdvertisingAnimationMatrix( unsigned int idx, float frame );
extern unsigned int GAdTextTexture;

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "menus.h"

typedef struct cutScenePlan_t
{
    vec_t eyeStart, dirStart;
    vec_t eyeEnd, dirEnd;
    float duration; // seconds
    float smoothEdge1, smoothEdge2;
    vec_t velvetColor; // alpha == 0 -> no velvet
    float DOFStrength, DOFFocalDistance;
    float trackProgressStart;
    u8 cameraMode;
    u8 indexOnEscKey;
    MENU_IDS menuToSet;
    //d on key(X)/goto_index, camera mode(0 = no change, 1=menu track observe), trackprogress value (-1 = not set)
}cutScenePlan_t;

typedef struct cutScene_t
{
    unsigned int nbPlans;
    cutScenePlan_t *plans;
} cutScene_t;

// sequences

extern cutScene_t introSceneArctic;
extern cutScene_t introSceneCity;
extern cutScene_t introSceneDesert;
extern cutScene_t introSceneSpace;

// play sequences

void PlaySequence( const cutScene_t& seq );
bool SequenceIsBeingPlayed();
void TickSequencePlayback( float aTimeEllapsed );
void StopSequencePlayback();

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string GetRandomSentence();
const char* GetRandomBotName();

///////////////////////////////////////////////////////////////////////////////////////////////////

extern std::map<std::string, mesh_t*> prefabs;
extern std::map<std::string, unsigned int> textures;
void LoadPrefabs();

#define TEXTURE_LOGO "Datas/Images/TheRushLogo.png"
#define TEXTURE_PLAYER1 "Datas/Images/player1.png"
#define TEXTURE_PLAYER2 "Datas/Images/player2.png"

#define TEXTURE_POS1 "Datas/Images/1rstPos.png"
#define TEXTURE_POS2 "Datas/Images/2ndPos.png"
#define TEXTURE_POS3 "Datas/Images/3rdPos.png"
#define TEXTURE_POS4 "Datas/Images/4thPos.png"
#define TEXTURE_POS5 "Datas/Images/5thPos.png"
#define TEXTURE_POS6 "Datas/Images/6thPos.png"
#define TEXTURE_POS7 "Datas/Images/7thPos.png"
#define TEXTURE_POS8 "Datas/Images/8thPos.png"
#define TEXTURE_BEACONWEAPON "Datas/Textures/bonus.png"

#define TEXTURE_HOWTO_KBD0 "Datas/Images/claviersolo.png"
#define TEXTURE_HOWTO_KBD1 "Datas/Images/clavier1.png"
#define TEXTURE_HOWTO_KBD2 "Datas/Images/clavier2.png"
#define TEXTURE_HOWTO_PAD "Datas/Images/pad.png"

#define TEXTURE_PORTAL "Datas/Textures/portal.png"

#endif

