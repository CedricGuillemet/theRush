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

#ifndef GAME_H__
#define GAME_H__


#include "content.h"
//#include "ZShipPhysics.h"
#include "physics.h"
#include "render.h"

struct Channel;
class IPhysicShip;
struct omniLight_t;



///////////////////////////////////////////////////////////////////////////////////////////////////
// game constants 

struct CameraSetup_t : public serialisableObject_t
{
    SERIALIZABLE(gameSettings_t,"Camera setup")
    CameraSetup_t();
    float CameraBehindDistance, CameraTensionFactor, CameraTargetDistance, CameraUpGroundShipFactor, CameraLocalUp,CameraBehindDistanceMax;
    float CameraAboveDistanceMin, CameraAboveDistanceMax, CameraSideDistance;
    float cameraRollingInterpolationFactor; // interpolation quand la caméra est en mode rolling (on suit le vaisseau )
    float cameraRollingSwitch; // DOT a partir duquel on passe en mode rolling    
};

typedef struct gameSettings_t : public serialisableObject_t
{
    SERIALIZABLE(gameSettings_t,"Game and content values.")
    gameSettings_t();

    float TrackBonusSpeedBoost;
    float ColorBonusAmount;
    int NumberOfMinesToDrop;
    float TimeBetwen2MinesDrop;
    float mMineRadius;
    float MineHealthDamage,MineBoostDecrease;
    float MissileHealthDamage,MissileBoostDecrease;
    float ShipRadius;

    float BonusPreHoleEcartementFactor;
    float BonusPreHoleEcartementMinimum;
    float BonusRadius;
    float BonusPositionTrackWidthFactor;

    float ReactorLightRadius;
    float MissileRadius;
    float MissileSpeed;
    float MissileTTL;
    float MissileLocalReactorBackZ;
    float InExplosionHealthDamage, InExplosionBoostDecrease;
    float ExplosionDuration, ExplosionMaxRadius;

    float MissileDotForAI;
    float MissileTimeBeforeFiredAnyway;
    float MissileHomingTimeBeforeFiredAnyway;
    float MissileMinimalTargetDistance;

    float AutoPilotTime;
    float ShieldTime;
    float GravityGripTime;
    int MachineGunBullets;
    float MachineGunTimeBetween2Bullets;
    float BulletHealthDamage;
    float BulletBoostDecrease;
    float DestructionShieldTime;
    float SpeedBoosterTime;
    float SpeedBoosterIncPerSecond;
    float BoostDecPerSecond;

    float BonusRepairInc;
    float BonusMissileSpread;
    float BonusHomingTimeBeforeArmed;
    float BonusHomingMinimalDistance;
    float BonusHomingMaximalDistance;    
    float TeslaHookDuration;
    float TeslaHookMinimalDistance;
    float TeslaHookMaximalDistance;
    float TeslaHookedBrakeFactor;
    float TeslaHookingAccelFactor;
    float TeslaHookTimeBeforeUsedAnyway;

    float BulletTTL;
    float BulletSpeed;
    float BulletWidth,BulletRed,BulletGreen,BulletBlue;

    float TrackBonusInactivityTime;

    float AcceleratorStartupSpeed;
    float AcceleratorDistanceChange;
    float AcceleratorSpeedInc;

    float HealthDecrementShockFactor;
    float DefaultHealth;

    float EliminatorTime;

    float DestructionLapHealth;
    float DestructionWeaponsFactor;

    float CoverPointsPerSecondFor1st;
    float CoverPointsPerSecondFor8th;

    float YYPowerPerTrackBonus;
    float YYPointsPerMissile;
    float YYPointsCapacity;

    float BoostFOVClamp;
    float BoostFOVMuliplication;
    float BoostFOVLerpFactor;

    float TrailColorSpeedFactor;
    float TrailWidthSpeedFactor;
    float TrailWidth;
    // camera
    CameraSetup_t cameraSetups[2];
    // gravity grip light
    float GravityGripLightRed,GravityGripLightGreen,GravityGripLightBlue,GravityGripLightRadius;

    float TeslaHookLightRed,TeslaHookLightGreen,TeslaHookLightBlue,TeslaHookLightRadius;
    float ExplosionLightRed,ExplosionLightGreen,ExplosionLightBlue,ExplosionLightRadius;
    float MissileLightRed,MissileLightGreen,MissileLightBlue,MissileLightRadius;

    int TracksCountUsedInGame;
        
} gameSettings_t;

extern gameSettings_t GameSettings;

typedef struct bonusGenerationParameters_t : public serialisableObject_t
{
    bonusGenerationParameters_t()
    {
        bonusVirageLength = 4.2f;
        bonusVirageRaide = 7.f;
        bonusTimeBeforeSpeed = 2.1f;
        bonusTimeBeforeWeapon = 1.3f;
        bonusSpeedLocalDensity = 0.8f;
        bonusWeaponLocalDensity = 1.5f;
    }

    SERIALIZABLE( bonusGenerationParameters_t, "Bonus generation parameters." )

    float bonusVirageLength;
    float bonusVirageRaide;
    float bonusTimeBeforeSpeed;
    float bonusTimeBeforeWeapon;
    float bonusSpeedLocalDensity;
    float bonusWeaponLocalDensity;
} bonusGenerationParameters_t;

typedef struct shipControls_t
{
    shipControls_t() { reset(); }
    void reset() { mUseAlternate = mLeft = mRight = mRun = mUse = 0; }

    u8 mLeft;
    u8 mRight;
    u8 mRun;
    u8 mUse;
    u8 mUseAlternate;
}shipControls_t;

typedef struct shipIdentifier_t
{
    shipIdentifier_t() { id = 0; }
    struct
    {
        union 
        {
            u32 id;
            struct
            {
                u8 mTeamIndex;
                u8 mEvolutionIndex;
                u8 padding0;
                u8 padding1;
            };
        };
    };
    void SetRandom()
    {
        id = getRandomShipCompound();
    }
} shipIdentifier_t;


#define NB_LIGHTNING_PER_SHIP 50

class ZShipPhysics;
class Bonus;
class Game;
class GameShip;
class Trail;
class Lightning;
class MachineGun;
class GameObjectNet;

struct mesh_t;
struct track_t;
struct occlusionQuery_t;
struct race_t;

enum GameObjectTypes
{
    GO_Ship,
    GO_Missile,
    GO_Mine,
    GO_Explosion,
    GO_Game,
	GO_Portal,
    GO_LastType
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class HUD
{
public:
    HUD()
    {
        Reset();
    }
    ~HUD()
    {
    }
    void Reset() { mNbLaps = 0; mLapTimes[0] = 0.f; }
    void PushNewLap() { mLapTimes[++mNbLaps] = 0.f; }
    void Tick(float aTimeEllapsed)
    {
        mLapTimes[mNbLaps] += aTimeEllapsed;
    }
    void Draw(Game *pGame, GameShip *pShip, int iSplitScreen );
protected:
    float mLapTimes[99];
    int mNbLaps;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GameObject
{
public:
    // type
    GameObject(u8 type, const matrix_t& mat);
    virtual ~GameObject();

    u8 GetType() const { return mType; }

    // behaviour
    virtual void Tick(float aTimeEllapsed) { UNUSED_PARAMETER(aTimeEllapsed); }

    bool ShouldBeDestroyed() const { return mbShouldBeDestroyed; }
    void SetShouldBeDestroyed() { mbShouldBeDestroyed = true; }

    // render
    virtual void InitForRender() {}
    virtual void TickForRender(float aTimeEllapsed) { UNUSED_PARAMETER(aTimeEllapsed); }
    virtual void UninitForRender() {}

    const matrix_t& GetMatrix() const { return mMatrix; }
    void SetMatrix( const matrix_t& mat ) { mMatrix = mat; }

    virtual u32 GetOwnerNetIndex() const  = 0;
    virtual void SetOwnerNetIndex(u32 OwnerNetIndex)  = 0;

    void HandleNetworkInstance();
    void HandleNetDestruction();

protected:
    u8 mType;
    matrix_t mMatrix;
    bool mbShouldBeDestroyed;
    GameObjectNet *mNetInstance;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
#define NBFIREFLIES 5

class GameShip : public GameObject
{
public:
    GameShip(const matrix_t& mat, unsigned int idx);
    virtual ~GameShip();

    IPhysicShip *GetPhysics() { return mPhysic; }

    // ship props streamed N times a sec with position/orientation

    typedef struct shipProps_t
    {
        float mHealth;
        float mPoints;
        vec3 mBoost; // dec to 0. added to steer
        //float mEnergy; // color energy
        float mNumberLapsDone;
        
        enum { Invalid_Bonus_type = 255 };
        u8 mBonusType; // 255 means none
        
        u8 lockingProgress;
        
        u8 mAutoPilot	 : 1; // autodirection chooser
        u8 mAutoUseBonus : 1; // use bonus at will. Only for Bots
        u8 mShield		 : 1;
        u8 mGravityGrip  : 1;
        
        u8 mTeslaHooking : 2; // 0: inactive, 1:tesla activated but none hooked, 2 : tesla hooking
        u8 mTeslaHooked  : 1;
        u8 mShooting	 : 1;
        
        u8 mCurrentColor : 1; // 0 is black 1 is white (for YY mode)
        u8 mDestrShield  : 1;
        u8 ctrlEnabled   : 1;
        u8 physicEnabled : 1;

        u8 lockedBy		 : 4; // 1 based . 0 = none
        u8 locking		 : 4; // 1 based . 0 = none

        u8 mRank		 : 4;
        u8 mNetIndex	 : 4; // 1 based

		u8 mCommandJitter: 1;

        // game modes
        u8 mGotOverEnduroSpeedThresold : 1;
        /*
        union
        {
            struct 
            {
                u8 mIsRed : 1;
                u8 mIsGreen : 1;
            } YY;
        };
        */

        //Locking Net Index is stored on 4 bits
        enum{ Locking_Net_Index_Max = 15 };

        //LockedBy  Net Index is stored on 4 bits
        enum{ LockedBy_Net_Index_Max = 15 };

        //Ship Owner Net Index is stored on 4 bits
        enum{ Owner_Net_Index_Max = 15 };

        shipProps_t(void)
        {
            memset(this, 0, sizeof(shipProps_t));
            mBonusType = Invalid_Bonus_type;
            ctrlEnabled = 0;
        }

    } shipProps_t;
    shipProps_t mProps;

    void ClearBonus();
    Bonus * GetAcquiredBonus() { return mCurrentAcquiredBonus; }
    void SetAcquiredBonus(Bonus *bn, int bonusType);

    virtual void Tick(float aTimeEllapsed);
    void Interpolate( float aLerp ) { mPhysic->Interpolate( aLerp ); }

    void SetBot( float aSkillFactor = 1.f ) { mProps.mAutoPilot = true; mProps.mAutoUseBonus = true; GetPhysics()->SetSkill( aSkillFactor ); }
	bool IsBot() const { return mProps.mAutoPilot; }

    void InitPhysics();

    virtual void InitForRender();
    virtual void TickForRender(float aTimeEllapsed);
    virtual void UninitForRender();


    void SetCurrentControls(shipControls_t ctrl) { mCurrentControls = ctrl; }
    const shipControls_t& GetCurrentControls() const { return mCurrentControls; }
    unsigned int GetIndex() const { return mIndex; }

    const char * GetName() const { return mName.c_str(); }
    void SetName(const char *szName) { mName = szName; mbNameIsDirty = true;}

    void Reset();

    // FireFlies
    void SetFFIntensity(float aIntens);
    void SetFFColor(const vec_t& col);
    void SetFFScale(float aScale);
    void SetFFLocalTimeFactor(float aTime);
    void SetFFDestinationMatrix( const matrix_t *pDestMatrix, float aInterp);

    float GetBonusLocalTime()const { return mBonusLocalTime; }
    float GetInstantSpeed() const { return mSpeed; }
    float GetSmoothedSpeed() const { return mSmoothedSpeed; }

    matrix_t GetMatrixForMissilesAndGun() const;
	void SetMatrixForMissilesAndGun( matrix_t& mat ) { mGunAndMissileMatrix = mat; }

	matrix_t GetBeaconLocalY() const { return beaconLocalY; }
	void SetBeaconLocalY( matrix_t& mat ) { beaconLocalY = mat; }

	



    virtual u32 GetOwnerNetIndex() const;
    virtual void SetOwnerNetIndex(u32 OwnerNetIndex);
    void ClearOwnerNetIndex();
    int GetCollisionMaskIndex() const;

    u32 GetLockingNetIndex() const;
    void SetLockingNetIndex(u32 LockingNetIndex);
    void ClearLockingNetIndex();

    u32 GetLockedByNetIndex() const;
    void SetLockedByNetIndex(u32 LockedByNetIndex);
    void ClearLockedByNetIndex();

    const vec_t& GetWorldReactorPosition() const { return mWorldReactorPosition; }
    const occlusionQuery_t *const GetOcclusion() const { return mOcclusion; }

    void SetShipIdentifier( const shipIdentifier_t& shipID);
    const shipIdentifier_t& GetShipIndentifier() const { return mShipIdentifier; }

    void NetworkConstruct( const matrix_t& mat, const shipIdentifier_t& shipId);
    
    float GetCompletedTrackLaps() const { return static_cast<float>(mCompletedTrackLaps); }
    void IncrementCompletedTrackLaps() { mCompletedTrackLaps ++; }
    mesh_t *GetShipMesh() const { return mShipMesh; }

    void ResetPID() { mShipPID.error = mShipPID.previouserror = mShipPID.I = 0.f;/*.SetIPD( 1.f, 1.f, 1.f );*/ }
    
    bool ShipIsInTunnel() const;
    float GetHealthForRendering() const { return mHealthForRendering; }
    float GetPointsForRendering() const { return mPointsForRendering; }

    GameShip * GetClosest( float aMaxDistance );
	const matrix_t& GetDrunkMatrix() const { return mDrunkMatrix; }

	void FastForward( int BrickCount, float portalOffset );
protected:
    // AI
    PIDf mShipPID;
    int mCurrentWayPoint;
    shipControls_t ComputeAI( float aTimeEllapsed );

    static int findBestCurveStartIndex( const vec_t& v );

    //
    shipIdentifier_t mShipIdentifier;
    matrix_t mInitialMatrix;
    IPhysicShip *mPhysic;
    Bonus *mCurrentAcquiredBonus;

    unsigned int mCompletedTrackLaps;

    // mesh
    mesh_t *mShipMesh;

    vec_t mLocalReactor;
    vec_t mWorldReactorPosition;

    // drunk/hover effect
    float mDrunkTime;
	matrix_t mDrunkMatrix;
    // controls
    shipControls_t mCurrentControls;

    // idx
    unsigned int mIndex;

    //name
    std::string mName;
    bool mbNameIsDirty;

    // trail
    Trail *mTrail;
    //vec_t mTrailColor;

	float mGravityGripTime;
	float mCommandJitterTime;

    // speed computed in tick
    float mSpeed, mSmoothedSpeed;
    float mHealthForRendering;
    float mPointsForRendering;

    // lightning
#define NBGRDM 100
    Lightning *mLightningEffects[NB_LIGHTNING_PER_SHIP];
    omniLight_t *mLightningOmnis[NB_LIGHTNING_PER_SHIP];

    // reactor light
    omniLight_t *mReactorLight;
    //vec_t mReactorColorRadius;

    // shields/YY
    Trail *mFireFlies[NBFIREFLIES];
    float mFireFliesAv;
    matrix_t mFireFliesBlurredMatrix;
    float mFFScale;
    float mFFLocalTimeFactor;
    const matrix_t* mFFDestMatrix;
    float mFFDestMatrixInterp;
    omniLight_t *mShieldLight;

    // YY Attack
    void ApplyYYAttack();
    GameShip *mYYAttackedShip;
    float mYYAttackInterpValue;

    // game mode properties
    float mYYShipRotationAngle;

    // Bonus Local Time
    float mBonusLocalTime;

    u32 GetPreviousLockingNetIndex() const;
    void SetPreviousLockingNetIndex(u32 LockingNetIndex);
    void ClearPreviousLockingNetIndex();

    //Previous Locking Net Index is stored in u8
    enum{ Previous_Locking_Net_Index_Max = U8_MAX };

    // previous props
    bool mbPreviousAutopilot;
    bool mbPreviousGravityGrip;
    float mPreviousBoost;
    u8 mPreviousLocking;
    u8 mbPreviousTeslaHooking;
    float mPreviousHealth;
    float mPreviousSpeed;
    bool mPreviousEnduroThresholdReached;

    // visual/machine gun
    MachineGun *mMachineGunBonus;

    // sounds
    Channel *mGravityGripSound;
    Channel *mTeslaSound;
    Channel *mEngineSound;
    Channel *mShieldSoundOn, *mShieldSoundLoop ;
    Channel *mChanSparklesSound, *mChanMetalFrictionSound ;


    float mImpactSoundPlaying;
    float mFrictionSoundStrength;
    // tracking/tracked
    Channel *mTrackingSound, *mTrackedSound;

    // occlusion
    occlusionQuery_t *mOcclusion;

    // game related stuff
    void SetGameRelatedProperties();

    matrix_t mGunAndMissileMatrix, beaconLocalY;

	friend class Game;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GameMissile : public GameObject
{
public:
    GameMissile( const matrix_t& mat );
    virtual ~GameMissile();

    void SetDirection(const vec_t& dir) { UNUSED_PARAMETER(dir); }

    virtual void Tick(float aTimeEllapsed);
    void Interpolate( float aLerp ) { UNUSED_PARAMETER(aLerp); }
    virtual void InitForRender();
    virtual void TickForRender(float aTimeEllapsed);
    virtual void UninitForRender();

    virtual u32 GetOwnerNetIndex() const;
    virtual void SetOwnerNetIndex(u32 OwnerNetIndex);

    u32 GetTargeting() const;
    void SetTargeting(u32 TargetNetIndex);
	void SetRoadTargeting( u16 roadBrickIndex ) { mRoadBrickTargeting = roadBrickIndex; }
    const occlusionQuery_t *const GetOcclusion() const { return mOcclusion; }
    vec_t GetWorldReactorPosition() const;
    void SetBrickIndex( int brickIdx ) { maCurIdx = brickIdx; }

protected:
    // missile owner // to replicate
    union
    {
		
        u8 missileProps;
        struct
        {
            u8 mOwner			: 4;
            u8 mTargeting		: 4;
        };
    };
    //Ship Owner Net Index is stored on 4 bits
    enum{ Owner_Net_Index_Max = 15 };

    //Targeting Net Index is stored on 4 bits
    enum{ Targeting_Net_Index_Max = 15 };

    occlusionQuery_t *mOcclusion;
    mesh_t *mMissile;
    Trail *mTrail;
    float mTTL;
    int maCurIdx;

    bool mbFreedFromRoad;
    // reactor light
    omniLight_t *mReactorLight;
	u16 mRoadBrickTargeting;
    // sounds
    //sf::Sound *mLaunchedSound, *mPropulsorSound;
    
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GameMine : public GameObject
{
public:
    GameMine( const matrix_t& mat );
    virtual ~GameMine();

    virtual void InitForRender();
    virtual void TickForRender(float aTimeEllapsed);
    virtual void UninitForRender();

    virtual u32 GetOwnerNetIndex() const;
    virtual void SetOwnerNetIndex(u32 OwnerNetIndex);

    unsigned int mBrickIndex; // for obstacles/AI

protected:
    // ro replicate
    u8	mOwner;
    //Ship Owner Net Index is stored on u8
    enum{ Owner_Net_Index_Max = U8_MAX };

    mesh_t *mMine;
    //  light
    omniLight_t *mLight;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GameExplosion : public GameObject
{
public:
    GameExplosion( const matrix_t& mat );
    virtual ~GameExplosion();

    virtual void Tick(float aTimeEllapsed);
    virtual void InitForRender();
    virtual void TickForRender(float aTimeEllapsed);
    virtual void UninitForRender();

    float GetRadius() const { return mExpRadius; }

    virtual u32 GetOwnerNetIndex() const;
    virtual void SetOwnerNetIndex(u32 OwnerNetIndex);

	void SetExplosionType( ExplosionType atype );
	ExplosionType GetExplosionType() const { return (ExplosionType)mExplosionType; }
	void SetBrick( u16 brickIndex ) { mBrickIndex = brickIndex; }

	int GetCurrentBrickIndex() const;
protected:
    // ro replicate
    u8	mOwner;
    //Ship Owner Net Index is stored on u8
    enum{ Owner_Net_Index_Max = U8_MAX };

    //mesh_t *mExploSphere;
    float mLifeSpan;
    float mTTL, mTTLRT;
    float mExpRadius;
    float mRadius;

	u8 mExplosionType;
	u16 mBrickIndex;
	float mBrickOffset;
    // reactor light
    omniLight_t *mExplosionLight;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GamePortal : public GameObject
{
public:
    GamePortal( const matrix_t& mat );
    virtual ~GamePortal();

    virtual void Tick(float aTimeEllapsed);
    virtual void InitForRender();
    virtual void TickForRender(float aTimeEllapsed);
    virtual void UninitForRender();

	void SetBricksInfos(int brickStart, int brickCount, float offset, bool bForward );

    virtual u32 GetOwnerNetIndex() const;
    virtual void SetOwnerNetIndex(u32 OwnerNetIndex);

	
	bool IsInPortal( GameShip *pShip ) const;
	bool IsForward() const { return mIsForward; }
	float GetOffset() const { return mOffset; }
protected:
    // ro replicate
    u8	mOwner;
    //Ship Owner Net Index is stored on u8
    enum{ Owner_Net_Index_Max = U8_MAX };


    float mLifeSpan;
    float mTTL;
	float mOffset;

	bool mIsForward;
	int mBrickStart, mBrickCount;
	Renderer::portal_t *mPortalRender;

    // reactor light
    omniLight_t *mPortalLight;
};
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct gameProperties_t
{
    u8 mType;
    u16 mTrackIndex;
    u8 mbDirtyForNetwork:4;
    u8 mbDirtyForWorld:4;
	//u8 mbDirtyForEndString:2;
    u8 mNumberOfLapsToDo;
	u8 mSplitScreen;

} gameProperties_t;


typedef struct bullet_t
{
    bullet_t(const vec_t& pos, const vec_t& dir);

    vec_t mPos, mDir;
    float mTTL;
    bool mbDelete;
} bullet_t;

enum GameState
{
    GameStateWaiting,
    GameStateCountDown,
    GameStatePlaying
};

class Game
{
protected:
    // Game life
    Game();
    virtual ~Game();
public:

    // Creation
    static void CreateNewGameSolo(int aType, int aNbLaps);
    static void DestroyGame();
    void DestroyGameObjects();

    // update
    virtual void Tick(float aTimeEllapsed);
    void TickForRender(float aTimeEllapsed);
    void Interpolate( float aLerp );

    // game props
    void SetType(u8 aType);
    u8 GetType() const { return mProps.mType; }
    void SetTrack(u16 aTrack);
    u16 GetTrack() const { return mProps.mTrackIndex; }
    void SetNumberOfLapsToDo(u8 aNbLaps);
	void SetSplit( bool bSplit );

    const gameProperties_t& GetGameProps() const { return mProps; }
    void ClearFlagPropsDirtyNetwork() { mProps.mbDirtyForNetwork = 0; }
    void ClearFlagPropsDirtyWorld() { mProps.mbDirtyForWorld = 0; }
    

    // game instance capability
    bool	SupportsRendering() const { return mbSupportsRendering; }
    bool	IsAuth() const { return mbIsAuth; }
    void EnableAuthoritative() { mbIsAuth = true; }
    void DisableAuthoritative() { mbIsAuth = false; }

    // objects&game props
    virtual GameObject* Spawn( u8 objectType, const matrix_t& mat, GameObject* parent = NULL );
    GameObject* SpawnShip( const matrix_t& mat, const shipIdentifier_t& shipID, u8 iNetIndex );

    virtual void Destroy(GameObject* pObj);

    // bonus status for replication
    void AddInvalidatedBonus(int idx) { if (IsAuth()) mInvalidatedBonus.push_back(static_cast<u16>(idx)); }
    void AddValidatedBonus(int idx) { if (IsAuth()) mValidatedBonus.push_back(static_cast<u16>(idx)); }
    const std::vector<u16>& GetInvalidatedBonuses() const { return mInvalidatedBonus; }
    const std::vector<u16>& GetValidatedBonuses() const { return mValidatedBonus; }
    void ClearValidatedAndInvalidatedBonusesList() { mValidatedBonus.clear(); mInvalidatedBonus.clear(); }

    static void GenerateBonuses(const bonusGenerationParameters_t& params, track_t & tr, bool bSpeed, bool bWeapon, bool bColors );

    // Bullets
    void AddBullet(const vec_t& pos, const vec_t& dir);

    // Game ship
    GameShip *GetPlayerShip( int playerID = 0 ) const { return mPlayerShip[playerID]; }
	void SetPlayerShip( GameShip *pShip,  int playerID = 0 ) { mPlayerShip[playerID] = pShip; mPlayerShip[playerID]->SetName(GetUserName().c_str()); }
    float GetPlayerEndRaceInactivityTime() const { return mPlayerEndRaceInactiveTime; }
    GameShip *GetShip(unsigned int idx) 
	{ 
		if (idx >= mGameObjectTyped[GO_Ship].size() )
			return NULL;
		return (GameShip*)mGameObjectTyped[GO_Ship][idx]; 
	}
    unsigned int GetShipCount() const { return mGameObjectTyped[GO_Ship].size(); }
    unsigned int GetStillAliveShipCount() const; 
    GameShip *GetLastRankedOnLapsCount() const;

    enum { E_Max_Ship_Count = 8 };

    // 1 base
    unsigned int GetRankBasedOnPoints( GameShip *pShip ) const;
    // game settings
    //const gameSettings_t & Settings() { return mSettings; }


    // restart
    virtual void Restart( int aMode = -1 );

    // state
    void SetState(GameState aState);
    GameState GetState() const { return mGameState; }

    // end results
    int GetEndResultsNbStrings() const { return mEndResultsStringsCount; }
    const std::string& GetEndResultString(int idx) const { ASSERT_GAME(idx < E_Max_Ship_Count); return mEndResultsString[idx]; }

    void SetEndResultsNbStrings( int nbResults ) { mEndResultsStringsCount = nbResults; }
    void SetEndResultString( int idx, const char *szStr ) { ASSERT_GAME(idx < E_Max_Ship_Count); mEndResultsString[idx] = szStr; }
    int GetPositionForShip(GameShip *pShip);
    bool EveryBodyFinishedRacing() const;
    // net index
    GameShip *GetShipByNetIndex( int NetIdx ) const; // index is 1 based

    void InitStartupPylons(const matrix_t mats[4]);
    // obstacles
    typedef struct obstacle_t
    {
        matrix_t mat;
        float radius;
        unsigned int brickindex;
        GameObject *relatedObject;
    } obstacle_t;


    void BuildObstacleList();
    int GetFirstObstacleIndexForBrickIndex( unsigned int aBrickIdx ) const;
    const obstacle_t& GetObstacleAtIndex( int idx ) const { return mObstacles[idx&0xFF]; }
    const int GetObstaclesCount() const { return mNbGlobalObstacles; }
    
    void SetRace( race_t * pRace ) { mCurrentRace = pRace; }
    float GetRacingTime() const { return mRacingTime; }
    // enduro
    void SetEnduroTimeAvailable( float aTime ) { mEnduroTotalTimeAvailable = aTime; }
    float GetEnduroTimeAvailable() const { return mEnduroTotalTimeAvailable; }

    void SetEnduroSpeedThreshold( float aSpeed ) { mEnduroSpeedThreshold = aSpeed; }
    float GetEnduroSpeedThreshold() const { return mEnduroSpeedThreshold; }

    static const char* RankSuffixText( unsigned int aRank );

    // net index
private:
    enum { First_Object_Net_Index = 1, Last_Object_Net_Index = 15 };
public:
    enum { Invalid_Object_Net_Index = 0 };
    static bool IsValidObjectNetIndex( u32 NetIndex )
    {
        return (First_Object_Net_Index <= NetIndex) && (NetIndex <= Last_Object_Net_Index);
    }
    void ResetNetIndexAllocator() { mNetIndex = First_Object_Net_Index; }
	bool EndStringDirty() const { return mEndResultsStringsDirty; }

	bool ShipExists(GameShip *ps) const
	{
		for (unsigned int i=0;i<mGameObjectTyped[GO_Ship].size();i++)
		{
			if ( (GameShip*)(mGameObjectTyped[GO_Ship][i]) == ps )
				return true;
		}
		return false;
	}

protected:
    // obstacles
    #define MAXNBOBSTACLES 256
    obstacle_t mObstacles[MAXNBOBSTACLES];
    int mNbGlobalObstacles;
    static int sortByBrickIndex(const void *p1, const void *p2);

    // flares


    void RenderFlare( const vec_t& worldReactorPosition, float reactorWidth, const vec_t& reactorInvDirection, unsigned int nbPixelsRendered  );

    std::vector<GameObject*> mGameObjects;
    std::vector<GameObject*> mGameObjectTyped[GO_LastType];

    // end result strings
    std::string mEndResultsString[E_Max_Ship_Count];
    int mEndResultsStringsCount;
    bool mEndResultsStringsDirty;
    void PushEndRaceStringForShip(GameShip *ps);

    // run-time props
    bool	mbSupportsRendering;
    bool	mbIsAuth;

    // bonus change
    std::vector<u16> mInvalidatedBonus;
    std::vector<u16> mValidatedBonus;
    // auth ticks


    // player ship
    GameShip *mPlayerShip[2];
    float mPlayerEndRaceInactiveTime;

    /*gameSettings_t mSettings;
    void UpdateGameProperties();
*/
    // bullets
    std::vector<bullet_t> mBullets;
    void TickBullets(float aTimeEllapsed);
    // collision metal response (ship, bullet, walls)



    // gameState
    GameState mGameState;
    float mCountDown;
    float mRacingTime;
    void TickGamePlay(float aTimeEllapsed);

    // net index
    u32 mNetIndex;
    u32 AllocateNetIndex() { ASSERT_GAME( IsValidObjectNetIndex(mNetIndex) ); return mNetIndex++; }

    // HUD
    HUD mHUD;

    // pylons
    StartupPylon mStartupPylons[4];
	float mStartupLocalTime;

    // game props
    gameProperties_t mProps;
    
    // enduro
    float mEnduroTotalTimeAvailable, mEnduroSpeedThreshold;
    // end of race for ship ps
    void HandleEndOfRace( GameShip *ps );
    race_t *mCurrentRace;
    // eliminator
    float mTimeBeforeElimination;

#if IS_OS_WINDOWS
	HINSTANCE physicDLLLibrary;
	typedef IPhysicShip* (*NewGamePhysics_t)( Game *_GGame, PhysicWorld *_physicWorld, Track *_track );
public:
	NewGamePhysics_t NewGamePhysics ;
	void LoadPhysicExtension();
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// game types

typedef struct gameType_t
{
    const char *szName;
    const char *szDescription;
    bool mbFirstInResultListHasWon; // true for classic race. False for Eliminator
    int mHUDNumber;
    bool bSpeedTrackBonus, bWeaponTrackBonus, bColorTrackBonus;
}gameType_t;

extern gameType_t GameTypes[];
extern unsigned int NBGameTypes;

// 


extern Game *GGame;

#endif
