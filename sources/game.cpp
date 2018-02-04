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
#include "game.h"
#include "content.h"
#include "ZShipPhysics.h"
#include "physics.h"
#include "net.h"
#include "bonus.h"
#include "mesh.h"
#include "world.h"
#include "menus.h"
#include "gui.h"
#include "fx.h"
#include "render.h"
#include "track.h"
#include "camera.h"
#include "therush.h"
#include "solo.h"
#include "audio.h"

#include "include_SDL.h"

#include <algorithm>    //used in some asserts

///////////////////////////////////////////////////////////////////////////////////////////////////




serializableField_t bonusGenerationParameters_t::mSerialisableFields[] = {
    SED(bonusGenerationParameters_t,bonusVirageLength,"Threshold after which a curve is set to be lengthy. A lengthy or hard curve allows more bonus at its exit."),
    SED(bonusGenerationParameters_t,bonusVirageRaide,"Angular factor for determining hard curves."),
    SED(bonusGenerationParameters_t,bonusTimeBeforeSpeed,"Bricks count before speed bonus. The higher it is, less frequent speed bonuses are."),
    SED(bonusGenerationParameters_t,bonusTimeBeforeWeapon,"Same as bonusSpeedLocalDensity but for weapons bonus."),
    SED(bonusGenerationParameters_t,bonusSpeedLocalDensity,"Speed bonus count factor. The higher it is, the more bonus you have."),
    SED(bonusGenerationParameters_t,bonusWeaponLocalDensity,"Same as bonusSpeedLocalDensity but for weapons bonus." )
};


NBFIELDS(bonusGenerationParameters_t);




gameSettings_t GameSettings;

serializableField_t CameraSetup_t::mSerialisableFields[] = {
    // camera
    SED(CameraSetup_t,CameraBehindDistance, "Distance behind ship"),
    SED(CameraSetup_t,CameraBehindDistanceMax,"Distance behind ship max"),
    SED(CameraSetup_t,CameraTensionFactor, "Tension. The higher the more rigid"),
    SED(CameraSetup_t,CameraTargetDistance, "Eye target distance in front of ship"),
    SED(CameraSetup_t,CameraUpGroundShipFactor, "0:same up as ground.1:same up as ship"),
    SED(CameraSetup_t,CameraLocalUp, ""),
    SED(CameraSetup_t,CameraAboveDistanceMin, "Minimal relative height"),
    SED(CameraSetup_t,CameraAboveDistanceMax, "Maximal relative height"),
    SED(CameraSetup_t,CameraSideDistance, "Maximal relative left-right distance"),
    SED(CameraSetup_t,cameraRollingInterpolationFactor, "Rolling up factor"), // interpolation quand la camÈra est en mode rolling (on suit le vaisseau )
    SED(CameraSetup_t,cameraRollingSwitch,"DOT value threshold for switching in rolling mode") // DOT a partir duquel on passe en mode rolling
};
NBFIELDS(CameraSetup_t);

CameraSetup_t::CameraSetup_t()
{
    CameraBehindDistance = 4.8000f;
    CameraBehindDistanceMax = 5.1000f;
    CameraTensionFactor = 12.0000f;
    CameraTargetDistance = 30.0000f;
    CameraUpGroundShipFactor = 0.2500f;
    CameraLocalUp = 1.0000f;
    CameraAboveDistanceMin = 0.8000f;
    CameraAboveDistanceMax = 1.5000f;
    CameraSideDistance = 0.0800f;
    cameraRollingInterpolationFactor = 0.0800f;
    cameraRollingSwitch = 0.8000f;
}

serializableField_t gameSettings_t::mSerialisableFields[] = {
    SE(gameSettings_t,TrackBonusSpeedBoost),
    // general
    SED(gameSettings_t,TracksCountUsedInGame,"Number of tracks used in the game."),
    // bonus
    SED(gameSettings_t,BonusRadius,"Bonus collision sphere radius"),
    SED(gameSettings_t,BonusPositionTrackWidthFactor, "Bonus drop factor on track.0=middle of track. 1=border"),

    SED(gameSettings_t,TrackBonusSpeedBoost,"track speed bonus boost value"),
    SED(gameSettings_t,NumberOfMinesToDrop,""),
    SED(gameSettings_t,TimeBetwen2MinesDrop,""),
    SED(gameSettings_t,mMineRadius,""),
    SED(gameSettings_t,MineHealthDamage,""),
    SED(gameSettings_t,MineBoostDecrease,""),
    SED(gameSettings_t,ShipRadius,""),

    SED(gameSettings_t,BulletTTL,""),
    SED(gameSettings_t,BulletSpeed,""),
    SED(gameSettings_t,BulletWidth,""),
    SED(gameSettings_t,BulletRed,""),
    SED(gameSettings_t,BulletGreen,""),
    SED(gameSettings_t,BulletBlue,""),

    SED(gameSettings_t,TrackBonusInactivityTime,""),

    // FOV
    SE(gameSettings_t, BoostFOVClamp ),
    SE(gameSettings_t, BoostFOVMuliplication ),

    // camera
    SED(gameSettings_t,cameraSetups,"close and futher camera setups")
};


NBFIELDS(gameSettings_t);

gameSettings_t::gameSettings_t()
{
    TrackBonusSpeedBoost = 10.0f;
    ColorBonusAmount = 0.05f;
    NumberOfMinesToDrop = 5;
    TimeBetwen2MinesDrop = 0.5f;
    mMineRadius = 0.10f;
    MineHealthDamage = 0.01f;
    MineBoostDecrease = 0.2f;
    MissileRadius = 0.05f;
    MissileSpeed = 200.0f;
    MissileTTL = 10.0f;
    MissileHealthDamage = 0.01f;
    MissileBoostDecrease = 0.2f;
    ExplosionDuration = 0.8f;
    ExplosionMaxRadius = 30.0f;
    InExplosionHealthDamage = 0.01f;
    InExplosionBoostDecrease = 0.3f;
    AutoPilotTime = 5.0f;
    ShieldTime = 5.0f;
    GravityGripTime = 5.0f;
    MachineGunBullets = 30;
    MachineGunTimeBetween2Bullets = 0.1f;
    BulletHealthDamage = 0.005f;
    BulletBoostDecrease = 0.1f;
    DestructionShieldTime = 5.0f;
    SpeedBoosterTime = 2.0f;
    SpeedBoosterIncPerSecond = 20.0f;
    BoostDecPerSecond = 5.0f;
    BonusRepairInc = 0.2f;
    BonusMissileSpread = 0.1f;
    BonusHomingTimeBeforeArmed = 2.0f;
    BonusHomingMinimalDistance = 4.0f;
    BonusHomingMaximalDistance = 50.0f;
    TeslaHookDuration = 5.0f;
    TeslaHookMinimalDistance = 4.0f;
    TeslaHookMaximalDistance = 50.0f;
    TeslaHookedBrakeFactor = 0.2f;
    TeslaHookingAccelFactor = 0.3f;
    BulletTTL = 15.0f;
    BulletSpeed = 50.0f;
    BulletWidth = 0.3f;
    BulletRed = 1.0f;
    BulletGreen = 0.5f;
    BulletBlue = 0.5f;
    TrackBonusInactivityTime = 5.0f;
    AcceleratorStartupSpeed = 100.0f;
    AcceleratorDistanceChange = 1000.0f;
    AcceleratorSpeedInc = 10.0f;
    EliminatorTime = 25.0f;
    DestructionLapHealth = 10.0f;
    DestructionWeaponsFactor = 2.0f;
    CoverPointsPerSecondFor1st = 10.0f;
    CoverPointsPerSecondFor8th = 3.0f;
    BoostFOVClamp = 10.0f;
    BoostFOVMuliplication = 5.0f;
    BoostFOVLerpFactor = 6.0f;
    HealthDecrementShockFactor = 1.0f;
    YYPowerPerTrackBonus = 10.0f;
    YYPointsPerMissile = 15.0f;
    YYPointsCapacity = 120.0f;
    TrailColorSpeedFactor = 0.025f;
    GravityGripLightRed = 0.8f;
    GravityGripLightGreen = 0.8f;
    GravityGripLightBlue = 1.0f;
    GravityGripLightRadius = 1.5f;
    TeslaHookLightRed = 0.8f;
    TeslaHookLightGreen = 0.8f;
    TeslaHookLightBlue = 1.0f;
    TeslaHookLightRadius = 0.5f;
    ExplosionLightRed = 1.8f;
    ExplosionLightGreen = 0.8f;
    ExplosionLightBlue = 1.0f;
    ExplosionLightRadius = 50.5f;
    MissileLightRed = 1.8f;
    MissileLightGreen = 0.8f;
    MissileLightBlue = 1.0f;
    MissileLightRadius = 5.5f;
    /*
    CameraBehindDistance = 3.5f;
    CameraBehindDistanceMax = 4.5f;

    CameraAboveDistanceMax = 1.5f;
    CameraAboveDistanceMin = 0.8f;

    CameraSideDistance = 0.1f;

    cameraRollingInterpolationFactor = 0.08f;
    cameraRollingSwitch = 0.8f;

    CameraTensionFactor = 6.0f;
    CameraTargetDistance = 30.0f;
    CameraUpGroundShipFactor = 0.25f;
    CameraLocalUp = 0.85f;
    */

    DefaultHealth = 1.0f;

    BonusRadius = 5.f;
    BonusPositionTrackWidthFactor = 0.45f;

    // trail
    TrailWidth = 0.35f;

}
///////////////////////////////////////////////////////////////////////////////////////////////////

gameType_t GameTypes[] = {
    {"Race", // 0
    "//Finish all the laps before your oponents",
    true,
    0,
    true, // speed
    true,  // weapon
    false // color
    },

    /*
    {"Accelerator",
    "//Keep racing the more you can as the speed increases each second!",
    false,
    1
    },
    */

    {"Eliminator", // 1
    "//After 1st lap, the worst pilot is eliminated every 30 seconds!",
    false,
    2,
    true, // speed
    true,  // weapon
    false // color
    },


    {"Destruction", // 2
    "//More destructive bonus. Some more energy for each finished lap.",
    false,
    3,
    true, // speed
    true,  // weapon
    false // color
    },


    {"Cover", // 3
    "//Get points each second depending on your place. The more you get, the best you are.",
    true,
    4,
    true, // speed
    true,  // weapon
    false // color
    },


    {"YingYang", // 4
    "//Switch your ship color the same color the track bonus. Get energy. Use it to destroy oponents!",
    false,
    5,
    false, // speed
    false,  // weapon
    true // color
    },

    {"Bird Shooting", // 5
    "//No weapon except unlimited machine gun.",
    false,
    0,
    true, // speed
    false,  // weapon
    false // color
    },

    {"Survivor", // 6
    "//First is blue and weaponless. Others are red and ready to shoot.",
    false,
    0,
    true, // speed
    true,  // weapon
    false // color
    },

    {"Collector", // 7
    "//Get points for each speedboost. The highest score wins",
    false,
    0,
    true, // speed
    false,  // weapon
    false // color
    },

    {"Enduro", // 8
    "//Keep your speed over 400. Below, your remaining time is decreased.",
    false,
    0,
    true, // speed
    true,  // weapon
    false // color
    },

};
unsigned int NBGameTypes = sizeof(GameTypes)/sizeof(gameType_t);

Game *GGame = NULL;

bullet_t::bullet_t(const vec_t& pos, const vec_t& dir)
{
    /*
    char tmps[512];
    sprintf(tmps, "BP : %5.2f %5.2f %5.2f\n", pos.x, pos.y, pos.z);
    OutputDebugString(tmps);
    */

    mPos = pos;
    mDir = dir;
    mTTL = GameSettings.BulletTTL;
    mbDelete = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GameShip::GameShip(const matrix_t& mat, unsigned int idx) : GameObject( GO_Ship, mat ), mShipPID(1.f, 1.f, 1.f), mGravityGripTime( -1.f ), mCommandJitterTime( -1.f )
{
    ASSERT_GAME( (mProps.mBonusType == shipProps_t::Invalid_Bonus_type) && (mProps.ctrlEnabled == 0) );
    ASSERT_GAME( mShipIdentifier.id == 0 );
	mDrunkMatrix.identity();

    ClearOwnerNetIndex();
    ClearLockingNetIndex();
    ClearLockedByNetIndex();

    mCurrentAcquiredBonus = NULL;

    mHealthForRendering = mProps.mHealth = 1.f;
    mPointsForRendering = 0.f;

    mbNameIsDirty = false;
    mName = GetRandomBotName();

    mLocalReactor = vec_t::zero;
    mWorldReactorPosition = vec_t::zero;

    mShipMesh = NULL;
    mTrail = NULL;

    mBonusLocalTime = 0.f;
    mImpactSoundPlaying = -1.f;

    mPhysic = GGame->NewGamePhysics( GGame, &physicWorld, &track);//new ZShipPhysics; //physicWorld.spawn(mat);
    mInitialMatrix = mat;
    InitPhysics();

    mCompletedTrackLaps = 0;
    mIndex = idx;

    // shields/YY
    mReactorLight = NULL;
    for (int i=0; i < NBFIREFLIES; ++i)
    {
        mFireFlies[i] = NULL;
    }
    mFireFliesAv = 0.f;
    mFireFliesBlurredMatrix.identity();
    mFFScale = 0.f;
    mFFLocalTimeFactor = 0.f;
    mFFDestMatrix = NULL;
    mFFDestMatrixInterp = 0.f;

    mShieldLight = NULL;

    mYYShipRotationAngle = 0.f;

    // sounds instances

    mTeslaSound = NULL;
    mShieldSoundOn = NULL;
    mShieldSoundLoop = NULL;
    mGravityGripSound = NULL;
    mEngineSound = NULL;
    mChanSparklesSound = NULL;
    mChanMetalFrictionSound = NULL;

    mFrictionSoundStrength = 0.f;

    // prev bonus states
    mbPreviousAutopilot = false;
    mbPreviousGravityGrip = false;
    mPreviousBoost = 0.f;
    ClearPreviousLockingNetIndex();
    mbPreviousTeslaHooking = false;
    mPreviousSpeed = 0.f;
    mPreviousHealth = mProps.mHealth = GameSettings.DefaultHealth;
    mPreviousEnduroThresholdReached = false;

    // machine gun
    mMachineGunBonus = NULL;

    mSmoothedSpeed = mSpeed = 0.f;

    // lightning
    for (int i=0;i<NB_LIGHTNING_PER_SHIP;i++)
    {
        mLightningEffects[i] = NULL;
        mLightningOmnis[i] = NULL;
    }
    mOcclusion = NULL;

    // ai
    mCurrentWayPoint = -1;

    // drunk/hover effect
    mDrunkTime = static_cast<float>( fastrand() );

    SetGameRelatedProperties();

    mYYAttackedShip = NULL;
    mYYAttackInterpValue = -1;

    mTrackingSound = NULL;
    mTrackedSound = NULL;

    mGunAndMissileMatrix = matrix_t::Identity;
    beaconLocalY = matrix_t::Identity;
}


GameShip::~GameShip()
{
    delete mPhysic;
	mPhysic = NULL;
    ClearBonus();
	
    delete mMachineGunBonus;
    mMachineGunBonus = NULL;

    UninitForRender();
}

void GameShip::ClearBonus()
{
	mGravityGripTime = -1.f;
	mCommandJitterTime = -1.f;
    delete mCurrentAcquiredBonus;
    mCurrentAcquiredBonus = NULL;
    mProps.mBonusType = shipProps_t::Invalid_Bonus_type;
}

void GameShip::SetAcquiredBonus(Bonus *bn, int bonusType)
{
    ASSERT_GAME( bn != NULL && bn->GetBonusType() == bonusType );
    ASSERT_GAME_MSG( mCurrentAcquiredBonus == NULL, "%s", "Leaking memory: current acquired bonus should be deleted first, before setting a new one." );

    mCurrentAcquiredBonus = bn;

    ASSERT_GAME( 0 <= bonusType && bonusType <= U8_MAX );
    mProps.mBonusType = static_cast<u8>(bonusType);

    ASSERT_GAME( mProps.mBonusType != shipProps_t::Invalid_Bonus_type );
}

bool GameShip::ShipIsInTunnel() const
{
    return ( ( track.getAIPoint( mPhysic->GetCurrentRoadBlockIndex() ).topology[0]&TrackTopology::TOPO_INGROUND) != 0 );
}

u32 GameShip::GetOwnerNetIndex() const
{
    return mProps.mNetIndex;
}
void GameShip::SetOwnerNetIndex(u32 OwnerNetIndex)
{
    ASSERT_GAME(OwnerNetIndex <= shipProps_t::Owner_Net_Index_Max);
    mProps.mNetIndex = static_cast<u8>(OwnerNetIndex);
}
void GameShip::ClearOwnerNetIndex()
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= shipProps_t::Owner_Net_Index_Max );
    mProps.mNetIndex = Game::Invalid_Object_Net_Index;
}
int GameShip::GetCollisionMaskIndex() const
{
    return (mProps.mNetIndex&7);
}
u32 GameShip::GetLockingNetIndex() const
{
    return mProps.locking;
}
void GameShip::SetLockingNetIndex(u32 LockingNetIndex)
{
    ASSERT_GAME(LockingNetIndex <= shipProps_t::Locking_Net_Index_Max);
    mProps.locking = static_cast<u8>(LockingNetIndex);
}
void GameShip::ClearLockingNetIndex()
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= shipProps_t::Locking_Net_Index_Max );
    mProps.locking = Game::Invalid_Object_Net_Index;
}
u32 GameShip::GetLockedByNetIndex() const
{
    return mProps.lockedBy;
}
void GameShip::SetLockedByNetIndex(u32 LockedByNetIndex)
{
    ASSERT_GAME(LockedByNetIndex <= shipProps_t::LockedBy_Net_Index_Max);
    mProps.lockedBy = static_cast<u8>(LockedByNetIndex);
}
void GameShip::ClearLockedByNetIndex()
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= shipProps_t::LockedBy_Net_Index_Max );
    mProps.lockedBy = Game::Invalid_Object_Net_Index;
}
u32 GameShip::GetPreviousLockingNetIndex() const
{
    return mPreviousLocking;
}
void GameShip::SetPreviousLockingNetIndex(u32 LockingNetIndex)
{
    ASSERT_GAME(LockingNetIndex <= Previous_Locking_Net_Index_Max);
    mPreviousLocking = static_cast<u8>(LockingNetIndex);
}
void GameShip::ClearPreviousLockingNetIndex()
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= Previous_Locking_Net_Index_Max );
    mPreviousLocking = Game::Invalid_Object_Net_Index;
}

void GameShip::NetworkConstruct( const matrix_t& mat, const shipIdentifier_t& shipId)
{
    //mShipIdentifier = shipId;
    //mPhysic->LoadShip( mat, GetCompoundPhysicMesh( mShipIdentifier.mMeshesID ), this );
    mInitialMatrix = mat;
    mShipIdentifier.id = static_cast<u32>(-1);
    SetShipIdentifier( shipId );
}


void GameShip::SetShipIdentifier( const shipIdentifier_t& shipID)
{
    bool bSetNewShipMesh = (mShipIdentifier.id != shipID.id);
    mShipIdentifier = shipID;

    if ( GGame->SupportsRendering() )
        InitForRender();

    if (mShipMesh)
        mShipMesh->color = vec ( 1.f );//namedColors[mShipIdentifier.mShipColor].value );

    if ( GGame->SupportsRendering() )
        TickForRender( 0.f );

    if ( bSetNewShipMesh )
    {
        //HACK: apparently needs to slightly change initial matrix when resetting the ship physics
        const float initial_pos_y = mInitialMatrix.position.y;

        const float tiny_translation_y = 0.2f;
        mInitialMatrix.position.y += tiny_translation_y;


		/*
		matrix_t oldbeaconLocalY(beaconLocalY),oldmGunAndMissileMatrix(mGunAndMissileMatrix), oldnewGrdMatrix(newGrdMatrix);
    beaconLocalY.lerp( beaconLocalY, tmpbeaconLocalY, 0.1f );
    beaconLocalY.orthoNormalize();

    mGunAndMissileMatrix = beaconLocalY * newGrdMatrix;
    mGunAndMissileMatrix.orthoNormalize();
	*/

		matrix_t oldsmooth = GetPhysics()->GetGroundOrientationSmoothed();

        InitPhysics();

		GetPhysics()->SetGroundOrientationSmoothed( oldsmooth );

        mInitialMatrix.position.y = initial_pos_y;
    }
}

void GameShip::Tick( float aTimeEllapsed )
{
    vec_t previousPositionForSpeed = mPhysic->GetTransform().position;

    mPhysic->NoRun();
    mPhysic->NoTurn();

									
	if (mGravityGripTime>0.f)
	{
		mGravityGripTime -= aTimeEllapsed;
		if ( mGravityGripTime <= 0.f )
		{
			mProps.mGravityGrip = false;
		}
	}


	if (mCommandJitterTime>0.f)
	{
		mCommandJitterTime -= aTimeEllapsed;
		if ( mCommandJitterTime <= 0.f )
		{
			mProps.mCommandJitter = false;
		}
	}

    if (mProps.mAutoPilot)
    {
        mCurrentControls.reset();
        mCurrentControls = ComputeAI( aTimeEllapsed );
    }
	/*
	if ( 1 )//mProps.mCommandJitter )
	{
		static float jitterTime = 0.f;
		static int jitterValue = 0;
		jitterTime += aTimeEllapsed;
		if (jitterTime > 0.05f)
		{
			jitterTime = -0.05f;
			jitterValue = fastrand();
		}
		else if (jitterTime < 0.f )
		{
			if ( (jitterValue&3) ==3)
				mCurrentControls.mLeft = 0xFF;
			else if ( (jitterValue&3) ==2)
				mCurrentControls.mRight = 0xFF;
			else
				mCurrentControls.mRight = mCurrentControls.mLeft = 0;	

			if ( mCurrentControls.mRun > 20 )
				mCurrentControls.mRun -= jitterValue&0xF;
		}
		
	}
	*/
    if (mProps.mAutoUseBonus && mCurrentAcquiredBonus)
        mCurrentControls.mUse = mCurrentAcquiredBonus->AIShouldUseIt( this, aTimeEllapsed );

    if ( mCurrentControls.mUseAlternate )
    {
        if (GGame->GetGameProps().mType == 4 ) // YY
        {
            bool bPlaySwitchSound = false;
            if ( (mProps.mCurrentColor==0) && ( mYYShipRotationAngle<0.1f ) )
            {
                mProps.mCurrentColor = 1;
                bPlaySwitchSound = true;
            }
            else if ( (mProps.mCurrentColor==1) && ( fabsf(mYYShipRotationAngle - PI_MUL_2) < 0.1f ) )
            {
                mProps.mCurrentColor = 0;
                bPlaySwitchSound = true;
            }

            if ( bPlaySwitchSound && GGame->SupportsRendering() )
                Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/switchcolor.ogg"), AUDIO_GROUP_GAME2D );
        }
    }

    if (!mProps.ctrlEnabled)
        mCurrentControls.reset();
    else
    {
        if ( NetClient->Active() &&
            ( ( Menus::GetCurrent() == MENU_INGAME_NETWORK ) ||
            ( Menus::GetCurrent() == MENU_SETUP_NETWORK ) )
            )
        {
            mCurrentControls.reset();
        }
    }

    //if (mCurrentControls.mRun || (mProps.mBoost > 0.f) )
    // run factor
    float teslaHookedBrake = mProps.mTeslaHooked?GameSettings.TeslaHookedBrakeFactor:0.f;
    float teslaHookingAccel = (mProps.mTeslaHooking>1)?GameSettings.TeslaHookingAccelFactor:0.f;
    float runFactor = (mCurrentControls.mRun?(1.f - teslaHookedBrake + teslaHookingAccel):0.f );
    /*
    if (mProps.mBoost > 0.f)
    {
    LOG(" Boost : %5.2f \n", mProps.mBoost );
    }
    */

    // YY Speed factor decreased if health < 0.5f, increased a bit if > 0.5f
    float YYSpeedFactor = 1.f;
    if (GGame->GetGameProps().mType == 4)
    {
        if ( mProps.mHealth < 0.5f )
        {
             YYSpeedFactor = 1.f - ( (0.5f - mProps.mHealth) * 0.4f ) ;
        }
        else
        {
            YYSpeedFactor = (mProps.mHealth-0.5f) * 0.2f; // a 5th of health improved -> max 10%
        }
    }
    // set values to physic integrator
    mPhysic->Run( runFactor * YYSpeedFactor, mProps.mBoost.getVec() );

    // turn
    if (mCurrentControls.mRight)
        mPhysic->TurnRight( (float)(mCurrentControls.mRight)*(1.f/255.f) );
    else
        if (mCurrentControls.mLeft)
            mPhysic->TurnLeft( (float)(mCurrentControls.mLeft)*(1.f/255.f) );

    if ( mCurrentControls.mUse && GGame->IsAuth() && mCurrentAcquiredBonus)
        mCurrentAcquiredBonus->Apply(this);
    else if ( mCurrentControls.mUse && GGame->IsAuth() && GGame->GetType() == 4 )
    {
        ApplyYYAttack();
    }
    else if ( mCurrentControls.mUse && (GGame->SupportsRendering()) && GGame->GetType() == 4 )
    {
        mYYAttackedShip = GetClosest( 60.f ); // potential network unsync
        mYYAttackInterpValue = 1.f;
    }

    // use YY
    //mProps.mBoost = LERP(mProps.mBoost, 0.f, aTimeEllapsed*GameSettings.BoostDecPerSecond);
    mProps.mBoost.lerp( 0.f, aTimeEllapsed*GameSettings.BoostDecPerSecond);

    mPhysic->Tick( aTimeEllapsed, mProps.mCommandJitter, mProps.mGravityGrip );

    if (aTimeEllapsed> FLOAT_EPSILON)
    {
        vec_t difDist = mPhysic->GetTransform().position - previousPositionForSpeed;
        difDist -= mPhysic->GetTransform().up * difDist.dot(mPhysic->GetTransform().up);

        mSpeed = difDist.length() / aTimeEllapsed;
        mSmoothedSpeed = LERP(mSmoothedSpeed, mSpeed, aTimeEllapsed * 2.f);
		// TODO: smooth speed in ship physics
        //mPhysic->m_Speed = mSmoothedSpeed;
    }

    if ( mProps.mHealth <= 0.f )
    {
        mProps.mShield		 = 0;
        mProps.mGravityGrip  = 0;
        mProps.mDestrShield  = 0;

        mProps.mTeslaHooking = 0;
        mProps.mTeslaHooked  = 0;
        mProps.mShooting	 = 0;
    }

    // missile and gun matrix

    //matrix_t viewProjBeacon = camera.mView * beaconProj ;
    const matrix_t & shptrMat = GetPhysics()->GetTransform();
    // from ground to smoother ground with position
    matrix_t newGrdMatrix = GetPhysics()->GetGroundOrientationSmoothed();

    float distanceOverGround = (newGrdMatrix.position-GetPhysics()->GetGroundMatrix().position).dot(newGrdMatrix.up);
	if (!GetPhysics()->HasGroundRaycast())
		newGrdMatrix.position = shptrMat.position;// - shptrMat.up * distanceOverGround;
	else
		newGrdMatrix.position = shptrMat.position - shptrMat.up * distanceOverGround;

    newGrdMatrix.position.w = 1.f;

    // get projected angle and compute a local Y rotation

    float ngDotDir = shptrMat.dir.dot( newGrdMatrix.dir );
    float ngDotRight = shptrMat.dir.dot( newGrdMatrix.right );

    ASSERT_GAME( (-1.f < ngDotDir && ngDotDir < 1.f) || IsNearlyEqual(ngDotDir, -1.f) || IsNearlyEqual(ngDotDir, 1.f) ) ;
    ngDotDir = Clamp(ngDotDir, -1.f, 1.f);
    float Yng = -acosf( ngDotDir );
    if ( ngDotRight>0.f) Yng = -Yng;

    matrix_t tmpbeaconLocalY;
    tmpbeaconLocalY.rotationY( Yng );

    beaconLocalY.lerp( beaconLocalY, tmpbeaconLocalY, 0.1f );
    beaconLocalY.orthoNormalize();

    mGunAndMissileMatrix = beaconLocalY * newGrdMatrix;
    mGunAndMissileMatrix.orthoNormalize();
}

void GameShip::InitPhysics()
{
    mPhysic->LoadShip( mInitialMatrix, GetCompoundPhysicMesh( mShipIdentifier.id ), this );
}

void GameShip::InitForRender()
{
    //u32 rndCompound = getRandomCompound();

    if ( mShipMesh != NULL )
    {
        //FIXME: might be a good idea to have ReinitForRender() or make sure InitForRender() only needs to be called once after the correct Ship id has been set?
        delete mShipMesh;
        mShipMesh = NULL;
    }

    //void getCompoundShip(u32 shipNb, vec_t& reactorPosition, mesh_t* &renderingMesh, mesh_t* &collisionMesh  )
    const mesh_t* OriginalShipMesh = getRenderingCompoundShip( mShipIdentifier.id, mLocalReactor );
    mShipMesh = OriginalShipMesh->clone();//getSprite(mSpriteNumber)->clone();
    mShipMesh->color = vec( 1.2f );

    if ( mOcclusion == NULL)
        mOcclusion = Renderer::NewOcclusionQuery();

    mOcclusion->mMesh = getReactorMesh( mShipIdentifier.id );

    if ( mTrail == NULL )
    {
        mTrail = new Trail;
        //const shipProperties_t& props = getShipPropertiesPackage(mModelName);

        for (int i=0;i<NBFIREFLIES;i++)
        {
            ASSERT_GAME( mFireFlies[i] == NULL );
            mFireFlies[i] = new Trail;
            mFireFlies[i]->SetColor(vec(1.f, 0.f, 0.f, 0.f));
            mFireFlies[i]->SetCurrentPoint( vec ( cosf(0.f * 3.f), sinf( 0.f * 3.f) , cosf(0.f )*2.f ) ) ;
            mFireFlies[i]->SetMinimalDistance(0.1f);
            mFireFlies[i]->SetWidth(0.2f);
            mFireFlies[i]->SetWorldMatrix( mPhysic->GetTransform() );
        }

        ASSERT_GAME( mTrackingSound == NULL );
        mTrackingSound = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/tracking.ogg" ), AUDIO_GROUP_GAME2D );
        ASSERT_GAME( mTrackedSound == NULL );
        mTrackedSound = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/MissileAlarm.ogg" ), AUDIO_GROUP_GAME2D );
        ASSERT_GAME( mTeslaSound == NULL );
        mTeslaSound = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/Electric.ogg" ), AUDIO_GROUP_GAME3D );
        ASSERT_GAME( mEngineSound == NULL );
        mEngineSound = Channel::AllocateChannel( Audio::GetSound("Datas/Sounds/fx/flamme_bruleur.ogg"), AUDIO_GROUP_GAME3D );
        if (mEngineSound)
        {
            mEngineSound->SetLoop(true);
            mEngineSound->SetVolume(0.f);
            mEngineSound->Play();
        }

        ASSERT_GAME( mReactorLight == NULL );
        mReactorLight = Renderer::newOmni();

        ASSERT_GAME( mShieldLight == NULL );
        mShieldLight = Renderer::newOmni();
        mShieldLight->mColor = vec(0.f);

    }
    //SetSoundPresets(mTrackingSound, hashTS );
}

void GameShip::TickForRender(float aTimeEllapsed)
{
    const vec_t& shpPos = mPhysic->GetPosition();


    //mTrailColor = vec(0.8f, 0.8f, 1.f, 1.f );//vec( mShipIdentifier.mTrailColor );
    //mReactorColorRadius = vec(0.8f, 0.8f, 1.f, 1.f );//vec ( mShipIdentifier.mReactorLightColor );

    // drunk effect

    mDrunkTime += aTimeEllapsed*2.f*world.GetGameSpeed();
    matrix_t drunkOffset;
    vec_t decalValue =  vec( cosf( mDrunkTime * 0.422f ) + sinf( mDrunkTime * 0.666f ), cosf( mDrunkTime * 0.353f ) + sinf( mDrunkTime * 0.125f ), cosf( mDrunkTime * 0.23f ) + sinf( mDrunkTime * 0.8f ) );
    vec_t rotateValue =  vec( cosf( mDrunkTime * 0.422f ) + sinf( mDrunkTime * 0.666f ), cosf( mDrunkTime * 0.353f ) + sinf( mDrunkTime * 0.125f ), cosf( mDrunkTime * 0.23f ) + sinf( mDrunkTime * 0.8f ) );
    rotateValue *= 0.01f;
    drunkOffset.translation( decalValue * 0.05f );
    matrix_t drunkRotate1, drunkRotate2;
    drunkRotate1.rotationYawPitchRoll( rotateValue.x, rotateValue.y, rotateValue.z );

	mDrunkMatrix = drunkRotate1 * drunkOffset;
    // drunk effect

    mShipMesh->mWorldMatrix = mDrunkMatrix * mPhysic->GetTransform() ;

    mShipMesh->updateWorldBSphere();

    mShipMesh->visible = true;
    mShipMesh->physic = false;

    mWorldReactorPosition.TransformPoint( mLocalReactor, mShipMesh->mWorldMatrix );
    //mWorldReactorPosition = mShipMesh->mWorldMatrix.position;

    mShieldLight->mPosition = mShipMesh->mWorldMatrix.position;
    mShieldLight->mPosition.w = 30.f;

    mOcclusion->mWorldMatrix = mShipMesh->mWorldMatrix;

    if (mPhysic->HasBeenReseted())
    {
        this->ClearBonus();
        mTrail->TeleportTo( mWorldReactorPosition );

		// player 1
        if ( ( GGame->GetPlayerShip() == this) && ( camera.GetMode() == CM_BEHINDSHIP ) )
            Renderer::newBlackVelvet(0);
        if ( ( GGame->GetPlayerShip(1) == this) && ( camera2.GetMode() == CM_BEHINDSHIP ) )
            Renderer::newBlackVelvet(1);

        SetGameRelatedProperties();
    }
    // trail

    mTrail->SetWidth( GameSettings.TrailWidth );
    mTrail->SetColor( vec(0.8f, 0.8f, 1.f, 1.f ) );//mTrailColor );
    float trailStrength = smootherstep(0.25f, 0.8f, Clamp( (mSmoothedSpeed * GameSettings.TrailColorSpeedFactor), 0.f, 1.f) );
    mTrail->SetCurrentPoint(mWorldReactorPosition, trailStrength );
	/*
    if ( ( GGame->GetPlayerShip() == this) && ( camera.GetMode() == CM_BEHINDSHIP ) )
        mTrail->SetVisible( false );
    else
	*/
        mTrail->SetVisible( true );

    // reactor light
    mReactorLight->mPosition = vec( mWorldReactorPosition.x, mWorldReactorPosition.y, mWorldReactorPosition.z, 10.f );//GameSettings.ReactorLightRadius );
    mReactorLight->mColor = vec(0.8f, 0.8f, 1.f, 1.f );//mReactorColorRadius;
    //	mTrail->SetWidth( Clamp(mSpeed * GameSettings.TrailWidthSpeedFactor, 0.f, mMaxTrailWidth) );

    bool mustZeroLocalBonusTime = true;

    // shield
    if ( mProps.mShield || mProps.mDestrShield )
    {
        if (mBonusLocalTime<= FLOAT_EPSILON)
        {
            uint32 hash = hashFast( "shieldOn" );
            mShieldSoundOn = Channel::AllocateChannel( Audio::GetSound("Datas/Sounds/fx/ShieldOn.ogg"), AUDIO_GROUP_GAME3D );//new sf::Sound( *GetSound( hash ) );
            if ( mShieldSoundOn != NULL )
            {
                mShieldSoundOn->SetDestroyOnStop( false );
                mShieldSoundOn->Play();
            }

            hash = hashFast( "shieldLoop" );
            mShieldSoundLoop =  Channel::AllocateChannel( Audio::GetSound("Datas/Sounds/fx/ShieldLoop.ogg"), AUDIO_GROUP_GAME3D );//new sf::Sound( *GetSound( hash ) );
            if ( mShieldSoundLoop != NULL )
            {
                mShieldSoundLoop->SetLoop(true);
                mShieldSoundLoop->SetDestroyOnStop( false );
                mShieldSoundLoop->Play();
            }
        }
        // sounds
        if (mShieldSoundOn)
        {
            mShieldSoundOn->SetPosition( mPhysic->GetTransform().position );
        }

        if (mShieldSoundLoop)
        {
            mShieldSoundLoop->SetPosition( mPhysic->GetTransform().position );
        }

        mBonusLocalTime += aTimeEllapsed;
        mustZeroLocalBonusTime = false;
        // set FFlies
        if (mBonusLocalTime<=1.f)
        {

            SetFFColor(mProps.mShield?vec(0.2f, 0.2f, 1.f, 0.f):vec(1.0f, 0.2f, 0.2f, 0.f));
            SetFFIntensity(mBonusLocalTime);
            SetFFScale(1.f);
            SetFFLocalTimeFactor(1.f);
        }

        if ( mProps.mShield )
            mShieldLight->mColor.lerp( vec( 0.f, 0.f, 1.f, 1.f ), aTimeEllapsed * 5.f);
        else
            mShieldLight->mColor.lerp( vec( 1.f, 0.f, 0.f, 1.f ), aTimeEllapsed * 5.f);
    }
    else
    {
        if (mShieldSoundOn)
        {
            mShieldSoundOn->Stop();
            Channel::DestroyChannel(mShieldSoundOn);
            mShieldSoundOn = NULL;

            Channel *shieldSoundOff = Channel::AllocateChannel( Audio::GetSound("Datas/Sounds/fx/ShieldOff.ogg"), AUDIO_GROUP_GAME3D );
            if (shieldSoundOff)
            {
                shieldSoundOff->SetPosition( mPhysic->GetTransform().position );
                shieldSoundOff->Play();
            }
        }

        if (mShieldSoundLoop)
        {
            mShieldSoundLoop->Stop();
            Channel::DestroyChannel(mShieldSoundLoop);
            mShieldSoundLoop = NULL;
        }

        // hide FF
        SetFFIntensity(0.f);
        SetFFScale(0.f);
        SetFFLocalTimeFactor(0.f);
        mShieldLight->mColor.lerp( vec(0.f), aTimeEllapsed * 5.f);
    }

    if (GGame->GetGameProps().mType == 4)
    {
        // attack
        if ( mYYAttackedShip && mYYAttackInterpValue > 0.f )
        {
            mYYAttackInterpValue  -= aTimeEllapsed * 5.f;
            if ( mYYAttackInterpValue < 0.f )
                mYYAttackInterpValue = 0.f;

            SetFFDestinationMatrix( &mYYAttackedShip->GetPhysics()->GetTransform(), 1.f - mYYAttackInterpValue);
        }
        if ( mYYAttackedShip &&  mYYAttackInterpValue < 0.f )
        {
            mYYAttackedShip = NULL;
            SetFFDestinationMatrix( NULL, 0.f );
        }

        SetFFLocalTimeFactor(1.f);
        SetFFScale(1.f);
    }


    // autopilot
	for (int i=0;i<2;i++)
	{
		if (this->mProps.mAutoPilot && GGame->GetPlayerShip(i) == this)
		{
			if ( (mBonusLocalTime <= GameSettings.AutoPilotTime-3.0f) &&
				( (mBonusLocalTime+aTimeEllapsed) > GameSettings.AutoPilotTime-3.0f) )
			{
				Audio::PlaySound( Audio::GetSound("Datas/Sounds/droid/DisengagementInTwoSeconds.ogg"), AUDIO_GROUP_GAME2D );
			}

			mBonusLocalTime += aTimeEllapsed;
			mustZeroLocalBonusTime = false;
		}
	}
    /*
    if (mbPreviousAutopilot && (!mProps.mAutoPilot) && GGame->GetPlayerShip() == this)
    {
    sf::Sound *psnd = FAFSound(  GetSound( hashFast( "Disengagement" ) ) );
    psnd->SetRelativeToListener(true);
    psnd->SetPosition(0.f, 0.f, 0.f);
    psnd->Play();
    }
    */

    // engine sound

    if ( mEngineSound != NULL )
    {
        //mEngineSound->SetRelativeToListener(false);
        mEngineSound->SetVolume( ( 0.02f + zmin(mSpeed*0.0002f, 0.03f) ) * 2.f );
        mEngineSound->SetPitch(1.0f + zmin(mSpeed*0.001f, 0.25f) );
    }

    if ( GGame->GetPlayerShip() == this )
    {
        if ( this->ShipIsInTunnel() )
        {
            Channel::SetEffect( 1, AUDIO_GROUP_GAME3D );
        }
        else
        {
            Channel::SetEffect( 0, AUDIO_GROUP_GAME3D );
        }
        //mEngineSound->SetPosition( camera.mViewInverse.position );
    }
    //else

    if ( mEngineSound != NULL )
    {
        mEngineSound->SetPosition( mPhysic->GetCurrentPosition() );

        vec_t shipVelocity = (mPhysic->GetPreviousPosition() - mPhysic->GetCurrentPosition() ) * (1.f/ aTimeEllapsed ) * 0.003f;
        mEngineSound->SetVelocity( shipVelocity );
    //LOG("Ship Vel : %5.2f %5.2f %5.2f \n", shipVelocity.x, shipVelocity.y, shipVelocity.z );
    //LOG("snd pos: %5.2f %5.2f %5.2f \n", mMatrix.position.x, mMatrix.position.y, mMatrix.position.z );
    /*
    mEngineSound->SetPosition( shpPos.x, shpPos.y, shpPos.z);
    mEngineSound->SetMinDistance(1000.f);
    mEngineSound->SetAttenuation(0.f);
    */

    //float speedDelta = mPreviousSpeed - mSmoothedSpeed;
    /*
    mEngineSound->SetPitch( 1.0f );//fabsf(speedDelta)*50.f);
    mEngineSound->SetVolume( 50.0f + fabsf(speedDelta)*50.f);
    */
    }

    // gravity grip
    if ( (!mbPreviousGravityGrip) && mProps.mGravityGrip )
    {
        mGravityGripSound = Channel::AllocateChannel( Audio::GetSound("Datas/Sounds/fx/GravityGripFX.ogg"), AUDIO_GROUP_GAME3D );
        if ( mGravityGripSound != NULL )
        {
            mGravityGripSound->SetLoop( true );
            mGravityGripSound->SetPosition( mPhysic->GetTransform().position );
            mGravityGripSound->Play();
        }
    }
    else if ( mbPreviousGravityGrip && (!mProps.mGravityGrip) )
    {
        if ( mGravityGripSound != NULL )
        {
            mGravityGripSound->Stop();
            mGravityGripSound = NULL;
        }
    }
    else if ( mbPreviousGravityGrip )
    {
        if ( mGravityGripSound != NULL )
        {
            mGravityGripSound->SetPosition( mPhysic->GetTransform().position );
        }
    }

    //
    if (mustZeroLocalBonusTime)
        mBonusLocalTime = 0.f;

    // fireflies
    if (mFFLocalTimeFactor >0.f)
    {
        mFireFliesAv += aTimeEllapsed * mFFLocalTimeFactor;

        matrix_t ffBaseMatrix;
        if (!mFFDestMatrix)
        {
            ffBaseMatrix = mPhysic->GetTransform();
        }
        else
        {
            ffBaseMatrix.lerp(mPhysic->GetTransform() , *mFFDestMatrix, mFFDestMatrixInterp);
            ffBaseMatrix.orthoNormalize();
        }

        mFireFliesBlurredMatrix.up.lerp(ffBaseMatrix.up, aTimeEllapsed);
        mFireFliesBlurredMatrix.up.normalize();

        mFireFliesBlurredMatrix.position = ffBaseMatrix.position;
        mFireFliesBlurredMatrix.dir = mPhysic->GetTransform().dir;

        mFireFliesBlurredMatrix.right.cross(mFireFliesBlurredMatrix.dir, mFireFliesBlurredMatrix.up);
        mFireFliesBlurredMatrix.right.normalize();

        for (int i=0;i<NBFIREFLIES;i++)
        {
            vec_t fireFlypos = vec(cosf(mFireFliesAv * 3.f*i*1.3f), sinf( mFireFliesAv * 3.f*i*1.7814f) , cosf(mFireFliesAv*i*1.1247f )*0.85f*i*0.9784f) * mFFScale;

            mFireFlies[i]->SetCurrentPoint( fireFlypos );
            mFireFlies[i]->SetWorldMatrix( mFireFliesBlurredMatrix );
        }
    }
    // hide things when in cockpit camera
    for (int i=0;i<NBFIREFLIES;i++)
    {
        mFireFlies[i]->SetVisible( !mShipMesh->onlyVisibleForShadows );
    }

    // gravity grip


    if (mProps.mGravityGrip)
    {

        int meshav = 0;
        int blkidx = mPhysic->GetCurrentRoadBlockIndex();
        for (float ligne = -2.f; ligne<3.f; ligne += 0.5f)
        {
            matrix_t ggmat, ggmat1, ggmat2;
            track.getAIPoint(blkidx + ((int)ligne) ).BuildMatrix(ggmat1, false);
            track.getAIPoint(blkidx + ((int)ligne) +1 ).BuildMatrix(ggmat2, false);

            float roadWidth = track.getAIPoint(blkidx + ((int)ligne) ).mAIwidth[0];

            ggmat.lerp( ggmat1, ggmat2, fmodf(ligne, 1.f));
            //FIXME: does it need to be ortho normalized?

            for (float column = -roadWidth; column<= roadWidth; column += 2.0f)
            {
                const matrix_t& shpMat = mPhysic->GetTransform();
                //const vec_t& shpPos = mPhysic->GetPosition();
                vec_t ggpos = ggmat.position + ggmat.right * column;
                float distSq = (ggpos - shpPos).lengthSq();
                if ((distSq < (6.f * 6.f)) && (meshav<NB_LIGHTNING_PER_SHIP))
                {
                    if (!mLightningEffects[meshav])
                        mLightningEffects[meshav] = new Lightning (32);
                    if (!mLightningOmnis[meshav])
                        mLightningOmnis[meshav] = Renderer::newOmni();

                    vec_t secondPoint = ggmat.up*(r01() * 0.5f + 0.1f);// + ggmat.right * (r01()*0.1f- 0.05f) + ggmat.right * (r01()+0.1f - 0.05f);
                    mLightningEffects[meshav]->SetPoints(ggpos, shpPos, secondPoint);
                    mLightningEffects[meshav]->SetWidth(0.2f);
                    vec_t lightingOmniPos = ggpos + ggmat.up * 0.5f;
                    const gameSettings_t& gsettings = GameSettings;
                    lightingOmniPos.w = gsettings.GravityGripLightRadius;

                    mLightningOmnis[meshav]->mPosition = lightingOmniPos;
                    mLightningOmnis[meshav]->mColor = vec( gsettings.GravityGripLightRed, gsettings.GravityGripLightGreen, gsettings.GravityGripLightBlue, 1.f );

                    meshav++;
                }
            }
        }
        for (int i = meshav;i <NB_LIGHTNING_PER_SHIP ; i++)
        {
            delete mLightningEffects[i];
            mLightningEffects[i] = NULL;

            Renderer::destroyOmni( mLightningOmnis[i] );
            mLightningOmnis[i] = NULL;
        }
    }
    else
    {
        if (mbPreviousGravityGrip)
        {
            for (int i=0;i<NB_LIGHTNING_PER_SHIP;i++)
            {
                if (mLightningEffects[i])
                {
                    delete mLightningEffects[i];
                    mLightningEffects[i] = NULL;

                    Renderer::destroyOmni( mLightningOmnis[i] );
                    mLightningOmnis[i] = NULL;
                }
            }
        }
    }

    // boost FOV
	Camera *cameras[]={&camera, &camera2};
	for (int i=0;i<2;i++)
	{
		if ( ( GGame->GetPlayerShip(i) == this ) && ( cameras[i]->GetObservedShip() == this ) && ( cameras[i]->GetMode() == CM_BEHINDSHIP ) )
		{
			vec_t localBoost = mProps.mBoost.getVec();
			float boostOnShip = localBoost.dot( GetPhysics()->GetTransform().dir );
			float desiredFOV = 90.0f + Clamp( boostOnShip, 0.f, GameSettings.BoostFOVClamp) * GameSettings.BoostFOVMuliplication;
			float computedFov = LERP(camera.mFov, desiredFOV, aTimeEllapsed * GameSettings.BoostFOVLerpFactor);
			cameras[i]->project(computedFov, camera.mRatio, camera.mNear, camera.mFar);
		}
	}

    // boost sounds
    if ( (mProps.mBoost.length() - mPreviousBoost) > 4.f)
    {
        // sound
        /*
        uint32 hash = hashFast( "trackSpeed" ) ;

        sf::Sound * psnd = FAFSound( GetSound( hash ) );
        SetSoundPresets(psnd, hash );
        psnd->SetRelativeToListener(false);
        //vec_t relSndPos = mMatrix.position - camera.mViewInverse.position;
        psnd->SetPosition( mMatrix.position.x, mMatrix.position.y, mMatrix.position.z );
        psnd->Play();
        */
    }

    // target heading
	for (int i=0;i<2;i++)
	{
		if (GGame->GetPlayerShip(i) == this)
		{
			const int LockingNetIndex = GetLockingNetIndex();
			if ( Game::IsValidObjectNetIndex(LockingNetIndex) )
			{
				const int PreviousLockingNetIndex = mPreviousLocking;
				if ( LockingNetIndex != PreviousLockingNetIndex )
				{
					//mTrackingSound->Play();
				}

				GameShip * shp = GGame->GetShipByNetIndex(LockingNetIndex);
				if (shp)
				{
					Renderer::DrawTarget( shp->GetPhysics()->GetPosition() , (float)(mProps.lockingProgress)* (1.f/255.f) );
				}
			}
			else
				Renderer::DrawTarget( vec(0.f) , -1.f );

			const int LockedByNetIndex = GetLockedByNetIndex();
			if ( Game::IsValidObjectNetIndex(LockedByNetIndex) )
			{
				// sounds + warning
	#if 0
				if (mTrackedSound->GetStatus() != sf::Sound::Playing)
				{
					mTrackedSound->SetLoop(true);
					mTrackedSound->Play();
				}
	#endif
			}
			else
			{
	#if 0
				if (mTrackedSound->GetStatus() == sf::Sound::Playing)
					mTrackedSound->Stop();
	#endif
			}
		}
	}
    // tesla hook
    if ( mProps.mTeslaHooking )
    {


        int lightningAv = 0;
        /*
        // few lightnings around
        for (int i=0;i<3;i++)
        {
        if (!mLightningEffects[lightningAv])
        mLightningEffects[lightningAv] = new Lightning (32);

        vec_t ggpos = shpPos + normalized(vec(r01() * 2.f -1.f, r01() * 2.f -1.f, r01() * 2.f -1.f) ) * (0.8f + r01());
        mLightningEffects[lightningAv]->SetPoints(ggpos, shpPos);
        mLightningEffects[lightningAv]->SetWidth(0.2f);
        lightningAv++;
        }
        */
        if ( mProps.mTeslaHooking > 1)
        {
            const float maxTeslaDist = GameSettings.TeslaHookMaximalDistance;

            for (unsigned int i=0;i<GGame->GetShipCount();i++)
            {
                GameShip *shp = GGame->GetShip(i);
                if ( shp == this)
                    continue;

                const vec_t& otherPos = shp->GetPhysics()->GetPosition();
                float dist = (otherPos - shpPos).length();
                if (dist <= maxTeslaDist)
                {
                    float aStrength = ((maxTeslaDist - dist)/maxTeslaDist);
                    int nbLightnings = (int) ( aStrength * 3.f + 1.f );
                    for (int j=0;j<nbLightnings;j++)
                    {
                        if (lightningAv == NB_LIGHTNING_PER_SHIP)
                            break;

                        if (!mLightningEffects[lightningAv])
                            mLightningEffects[lightningAv] = new Lightning (32);

                        vec_t ggOther = otherPos + normalized(vec(r01() * 2.f -1.f, r01() * 2.f -1.f, r01() * 2.f -1.f) ) * (0.2f + r01()*0.5f);
                        vec_t ggShip  = shpPos + normalized(vec(r01() * 2.f -1.f, r01() * 2.f -1.f, r01() * 2.f -1.f) ) * (0.2f + r01()*0.5f);
                        mLightningEffects[lightningAv]->SetPoints( ggOther, ggShip );
                        mLightningEffects[lightningAv]->SetWidth( 0.1f + 0.3f * aStrength );
                        mLightningEffects[lightningAv]->SetColor( vec( 1.f, 1.f, 1.f, 0.3f + aStrength*0.7f ) );
                        lightningAv++;
                    }
                }
            }
        }
        for (int i=lightningAv;i<NB_LIGHTNING_PER_SHIP;i++)
        {
            delete mLightningEffects[i];
            mLightningEffects[i] = NULL;
        }
    }

    if ( mbPreviousTeslaHooking && (!mProps.mTeslaHooking) )
    {
        for (int i=0;i<NB_LIGHTNING_PER_SHIP;i++)
        {
            delete mLightningEffects[i];
            mLightningEffects[i] = NULL;
        }
    }
#if 0
    if ( (mProps.mTeslaHooking>1) || mProps.mTeslaHooked)
    {
        if (mTeslaSound->GetStatus() != sf::Sound::Playing)
        {
            mTeslaSound->SetRelativeToListener(false);
            mTeslaSound->SetLoop(true);
            mTeslaSound->Play();
        }

        mTeslaSound->SetPosition(shpPos.x, shpPos.y, shpPos.z);

    }
#endif
    if ( (!mProps.mTeslaHooking) && (!mProps.mTeslaHooked))
    {
        /*
        if (mTeslaSound->GetStatus() == sf::Sound::Playing)
        {
        mTeslaSound->Stop();
        }
        */
    }
    // health
	for (int i=0;i<2;i++)
	{
		if (GGame->GetPlayerShip(i) == this)
		{
			if ( mPreviousHealth < mProps.mHealth)
			{
				// play health sound
				if ( GGame->GetType() == 0 )
					Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/repair.ogg"), AUDIO_GROUP_GAME2D );

				/*
				sf::Sound *psnd = FAFSound(  GetSound( hashFast( "repair" ) ) );
				psnd->SetRelativeToListener(true);
				psnd->SetPosition(0.f, 0.f, 0.f);
				psnd->Play();
				*/
			}
			else if (mPreviousHealth > mProps.mHealth )
			{
				//loosing health -> shocked
				//ggui.applyShock( (mPreviousHealth - mProps.mHealth) * GameSettings.HealthDecrementShockFactor );
				//if ( ( GetCurrentMenu() == MENU_EMPTY_SOLO ) || ( GetCurrentMenu() == MENU_EMPTY_NETWORK) )
			}
		}
	}

    //machine gun
    if ( (!mMachineGunBonus) && (mProps.mBonusType == BT_MachineGun) )
    {
        mMachineGunBonus = new MachineGun;
    }

    if ( mMachineGunBonus )
    {
        if ( mCurrentControls.mUse )
            mMachineGunBonus->Apply( this );

        mMachineGunBonus->Tick( this, aTimeEllapsed );
    }

    if ( mMachineGunBonus && (mProps.mBonusType != BT_MachineGun) )
    {
        delete mMachineGunBonus;
        mMachineGunBonus = NULL;
    }
    // wrecked
    if ( mProps.mHealth <= 0.f )
    {
        bool bEmit = ((fastrand()&63)==63);
        if ( bEmit )
        {
            int emitCount = fastrand()&0xF;
            for (int i = 0;i<emitCount;i++)
            {
                vec_t sparkSpawn = vec( r01()*2.f - 1.f, 1.f, r01()*2.f - 1.f );
                sparkSpawn.TransformVector( mShipMesh->mWorldMatrix );

                GSparkles->Spawn( mShipMesh->mWorldMatrix.position + mShipMesh->mWorldMatrix.dir * (r01()*3.f-1.5f), normalized(sparkSpawn)*0.3f, 0xFFFFFFFF, 0.32f);
            }
        }
    }


    // collisions

    // collisions
    const PhysCollision_t *pc = physicWorld.GetCollisions().GetFirst();
    const PhysCollision_t *pcEnd = physicWorld.GetCollisions().GetEndUsed();

    //float maxFriction = 0.f;

    for (; pc != pcEnd; pc++)
    {
        float impactValue = fabsf(pc->mDistance);
        static const float fullImpactValue = 0.175f;
        if ( ( ( pc->mObject1  == this) || ( pc->mObject2  == this) ) &&
            ( impactValue > fullImpactValue ) && ( mImpactSoundPlaying <= 0.f ) )
        {
            Channel *pChan = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/WallCollisionDefault.ogg" ), AUDIO_GROUP_GAME3D );
            if ( pChan != NULL )
            {
                pChan->SetPosition( pc->mWorldPosition );
                pChan->SetVolume(1.f);
                pChan->SetPitch(1.f);
                pChan->Play();
                mImpactSoundPlaying = 0.3f;
            }
        }

        if ( ( ( pc->mObject1  == this) || ( pc->mObject2  == this) ) && ( impactValue <= fullImpactValue ) )
        {
            if ( mChanSparklesSound == NULL )
            {
                mChanSparklesSound = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/forge_outil.ogg" ), AUDIO_GROUP_GAME3D );
                if ( mChanSparklesSound != NULL )
                {
                    mChanSparklesSound->SetLoop(true);
                    mChanSparklesSound->Play();
                }
            }

            if ( mChanMetalFrictionSound == NULL )
            {
                mChanMetalFrictionSound = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/grincement.ogg" ), AUDIO_GROUP_GAME3D );
                if ( mChanMetalFrictionSound != NULL )
                {
                    mChanMetalFrictionSound->SetLoop(true);
                    mChanMetalFrictionSound->Play();
                }
            }

            if ( mChanSparklesSound != NULL )
            {
                mChanSparklesSound->SetPosition( pc->mWorldPosition );
            }

            if ( mChanMetalFrictionSound != NULL )
            {
                mChanMetalFrictionSound->SetPosition( pc->mWorldPosition );
            }

            mFrictionSoundStrength = impactValue * (2.f / fullImpactValue );
            mFrictionSoundStrength = Clamp( mFrictionSoundStrength, 0.f, 1.f );
        }
        //LOG("Col %5.4f \n", impactValue );
    }
    mImpactSoundPlaying -= aTimeEllapsed;

    // friction lower
    mFrictionSoundStrength -= aTimeEllapsed*2.f;

    if ( mChanSparklesSound )
    {
        if (mFrictionSoundStrength<0.f)
        {
            mChanSparklesSound->Stop();
            mChanSparklesSound = NULL;
        }
        else
        {
            mChanSparklesSound->SetVolume( mFrictionSoundStrength );
        }
    }
    if ( mChanMetalFrictionSound )
    {
        if (mFrictionSoundStrength<0.f)
        {
            mChanMetalFrictionSound->Stop();
            mChanMetalFrictionSound = NULL;
        }
        else
        {
            mChanMetalFrictionSound->SetVolume( mFrictionSoundStrength );
        }
    }

    // health for render
    if ( fabs(mHealthForRendering - mProps.mHealth) <= aTimeEllapsed )
    {
        mHealthForRendering = mProps.mHealth;
    }
    else
    {
        if ( mHealthForRendering > mProps.mHealth )
            mHealthForRendering -= aTimeEllapsed;
        else
            mHealthForRendering += aTimeEllapsed;
    }
    mPointsForRendering = LERP( mPointsForRendering, mProps.mPoints, 0.5f);
    if ( fabs(mPointsForRendering - mProps.mPoints) <= 0.01f )
        mPointsForRendering = mProps.mPoints;

    // game modes

    switch( GGame->GetGameProps().mType )
    {

    case 4: // YY
        {
            float timeSwitchFactor = aTimeEllapsed * 10.f;
            if ( mProps.mCurrentColor == 1 )
            {
                mYYShipRotationAngle = LERP( mYYShipRotationAngle, PI_MUL_2, timeSwitchFactor );
                mShipMesh->color.lerp( vec(0.f, 1.f, 0.f, 1.f ), timeSwitchFactor );
            }
            else
            {
                mYYShipRotationAngle = LERP( mYYShipRotationAngle, 0.f, timeSwitchFactor );
                mShipMesh->color.lerp( vec(1.f, 0.f, 0.f, 1.f ), timeSwitchFactor );
            }
            SetFFColor( vec(mShipMesh->color.x, mShipMesh->color.y, mShipMesh->color.z, 0.f ) );

            matrix_t localZRot;
            localZRot.rotationZ( mYYShipRotationAngle );

            matrix_t shipZRot = localZRot * mShipMesh->mWorldMatrix;
            mShipMesh->mWorldMatrix = shipZRot;


            SetFFIntensity( Clamp( (mProps.mHealth-0.5f)* 2.f , 0.f, 1.f ) );
        }
        break;
    case 8:
        if ( (!mPreviousEnduroThresholdReached) && mProps.mGotOverEnduroSpeedThresold )
            Audio::PlaySound( Audio::GetSound("Datas/Sounds/droid/SpeedThresholdReached.ogg"), AUDIO_GROUP_GAME2D );
        break;
    default:
        break;
    }

    // prev state
    mbPreviousAutopilot = mProps.mAutoPilot;
    mbPreviousGravityGrip = mProps.mGravityGrip;
    mbPreviousTeslaHooking = mProps.mTeslaHooking;
    mPreviousBoost = mProps.mBoost.length();
    SetPreviousLockingNetIndex( GetLockingNetIndex() ); //mPreviousLocking = mProps.locking;
    mPreviousHealth = mProps.mHealth;
    mPreviousSpeed = mSmoothedSpeed;
    mPreviousEnduroThresholdReached = mProps.mGotOverEnduroSpeedThresold;


}

// FireFlies
void GameShip::SetFFIntensity(float aIntens)
{
    vec_t ffcol = mFireFlies[0]->GetColor();
    aIntens *= (float)NBFIREFLIES;
    int i = 0;
    while (aIntens >= 0.f)
    {
        if (aIntens >= 1.f)
            mFireFlies[i]->SetColor(vec(ffcol.x, ffcol.y, ffcol.z, 1.f));
        else if (aIntens > 0.f)
            mFireFlies[i]->SetColor(vec(ffcol.x, ffcol.y, ffcol.z, aIntens));


        aIntens -= 1.f;
        i++;
    }
}
void GameShip::SetFFColor(const vec_t& col)
{
    for (int i=0;i<NBFIREFLIES;i++)
        mFireFlies[i]->SetColor(col);
}
void GameShip::SetFFScale(float aScale)
{
    mFFScale = aScale;
}
void GameShip::SetFFLocalTimeFactor(float aTime)
{
    mFFLocalTimeFactor = aTime;
}
void GameShip::SetFFDestinationMatrix( const matrix_t *pDestMatrix, float aInterp)
{
    mFFDestMatrix = pDestMatrix;
    mFFDestMatrixInterp = aInterp;
}

void GameShip::UninitForRender()
{
    for (int i = 0;i <NB_LIGHTNING_PER_SHIP ; i++)
    {
        delete mLightningEffects[i];
        mLightningEffects[i] = NULL;

        Renderer::destroyOmni( mLightningOmnis[i] );
        mLightningOmnis[i] = NULL;
    }

    Renderer::destroyOmni( mReactorLight );
    mReactorLight = NULL;
    Renderer::destroyOmni( mShieldLight );
    mShieldLight = NULL;

    //Channel::DestroyChannel( mGravityGripSound );
    mGravityGripSound = NULL;
    //Channel::DestroyChannel( mShieldSoundOn );
    mShieldSoundOn = NULL;
    //Channel::DestroyChannel( mShieldSoundLoop );
    mShieldSoundLoop = NULL;
    //Channel::DestroyChannel( mTrackingSound );
    mTrackingSound = NULL;
    //Channel::DestroyChannel( mTrackedSound );
    mTrackedSound = NULL;
    //Channel::DestroyChannel( mTeslaSound );
    mTeslaSound = NULL;
    //Channel::DestroyChannel( mEngineSound );
    mEngineSound = NULL;
    //Channel::DestroyChannel( mChanSparklesSound );
    mChanSparklesSound = NULL;
    //Channel::DestroyChannel( mChanMetalFrictionSound );
    mChanMetalFrictionSound = NULL;

    for (int i=0; i < NBFIREFLIES; ++i)
    {
        delete mFireFlies[i];
        mFireFlies[i] = NULL;
    }

    delete mTrail;
    mTrail = NULL;

    if ( mOcclusion != NULL )
    {
        Renderer::DeleteOcclusionQuery( mOcclusion );
        mOcclusion = NULL;
    }

    delete mShipMesh;
    mShipMesh = NULL;
}

void GameShip::SetGameRelatedProperties()
{
    switch( GGame->GetGameProps().mType )
    {
    case 5: // bird shooting
        {
            Bonus *bn = Bonus::Instanciate( BT_MachineGun );
            ASSERT_GAME( bn->GetBonusType() == BT_MachineGun );

            MachineGun* mg = static_cast<MachineGun*>(bn);
            mg->SetShellCount( 10000000 );

            this->SetAcquiredBonus( bn, BT_MachineGun );
        }
        break;
    case 4: // YY
        {
            mYYShipRotationAngle = 0;
            mHealthForRendering = mProps.mHealth = 0.5f;
        }
        break;
    case 8: // enduro
        {
            mPreviousEnduroThresholdReached = false;
            mProps.mGotOverEnduroSpeedThresold = false;
        }
        break;
    }
}

void GameShip::Reset()
{
	mGravityGripTime = -1.f;
	mCommandJitterTime = -1.f;
    mCompletedTrackLaps = 0;
	mPhysic->Reset(mPhysic->GetTime0RestartMatrix());
    ClearBonus();

    SetGameRelatedProperties();
}

matrix_t GameShip::GetMatrixForMissilesAndGun() const
{
#if 0
    const matrix_t& misMat = mPhysic->GetTransform();
    matrix_t shpGroundMat = mPhysic->GetGroundMatrix();
    //misMat.position = shpGroundMat.position + shpGroundMat.up * 2.f;


    float dotup = misMat.up.dot(shpGroundMat.up);
    shpGroundMat.dir = misMat.dir - shpGroundMat.up * dotup;
    shpGroundMat.dir.normalize();

    shpGroundMat.right.cross(shpGroundMat.dir, shpGroundMat.up);
    shpGroundMat.right.normalize();
    shpGroundMat.position += shpGroundMat.up * 5.f;
    //    shpGroundMat.position = misMat.position;
    //shpGroundMat.up = -shpGroundMat.up;
    return shpGroundMat;
    return mPhysic->GetTransform();
    return misMat;
#endif


    return mGunAndMissileMatrix;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GameObject::GameObject(u8 type, const matrix_t& mat) :
    mType(type),
    mMatrix(mat),
    mbShouldBeDestroyed(false),
    mNetInstance(NULL)
{
}

GameObject::~GameObject()
{
	mMatrix = matrix_t::Zero;
	mNetInstance = NULL;
}

void GameObject::HandleNetworkInstance()
{
    mNetInstance = NetServer->Spawn( this->GetType(), this );
}

void GameObject::HandleNetDestruction()
{
    NetServer->Destroy( mNetInstance );
}


///////////////////////////////////////////////////////////////////////////////////////////////////

GameMissile::GameMissile(const matrix_t& mat) : GameObject( GO_Missile, mat )
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= GameMissile::Owner_Net_Index_Max );
    mOwner = Game::Invalid_Object_Net_Index;

    //const shipProperties_t& props = getShipPropertiesPackage("Game");
    mTTL = GameSettings.MissileTTL;//getShipProperty(props, "MissileTTL");
    //mLaunchedSound = mPropulsorSound = NULL;
    mOcclusion = NULL;
    mMissile = NULL;
    mTrail = NULL;
    mReactorLight = NULL;

    this->missileProps = 0;
    maCurIdx = UNASSIGNED_BRICK_INDEX;
    mbFreedFromRoad = false;

	mRoadBrickTargeting = 0xFFFF;
}

GameMissile::~GameMissile()
{
    UninitForRender();
}

vec_t GameMissile::GetWorldReactorPosition() const
{
    vec_t res = vec(0.f, 0.f, GameSettings.MissileLocalReactorBackZ, 0.f);
    res.TransformPoint( mMatrix );
    return res;
}

u32 GameMissile::GetOwnerNetIndex() const
{
    return mOwner;
}
void GameMissile::SetOwnerNetIndex(u32 OwnerNetIndex)
{
    ASSERT_GAME( OwnerNetIndex <= GameMissile::Owner_Net_Index_Max );
    mOwner = static_cast<u8>(OwnerNetIndex);
}
u32 GameMissile::GetTargeting() const
{
    return mTargeting;
}
void GameMissile::SetTargeting(u32 TargetNetIndex)
{
    ASSERT_GAME( TargetNetIndex <= GameMissile::Targeting_Net_Index_Max );
    mTargeting = static_cast<u8>(TargetNetIndex);
}

void GameMissile::Tick(float aTimeEllapsed)
{
    vec_t prevPos = mMatrix.position;
    mMatrix.position += mMatrix.dir * aTimeEllapsed * GameSettings.MissileSpeed;
    mMatrix.position.w = 1.f;

    vec_t hitPosition, hitNormal;

    // wall check
    vec_t chkPos = prevPos+mMatrix.dir*3.f;
    bool missileCollide = world.IsThereVoxelAtWorldPos( chkPos );
    missileCollide |= physicWorld.rayCast(prevPos, chkPos, hitPosition, hitNormal);

	
    if ( missileCollide )
    {
        SetShouldBeDestroyed();
        GGame->Spawn(GO_Explosion, mMatrix, this);
        return;
    }
    else
    {
        // ground check
        /*if (physicWorld.rayCast(mMatrix.position, mMatrix.position-mMatrix.up*10.f, hitPosition, hitNormal))
        {
        float distToGround = (hitPosition-mMatrix.position).length();
        float difDist = distToGround-2.f;
        if (difDist<0.f)
        {

        vec_t newPos = mMatrix.position - mMatrix.up * difDist;
        mMatrix.LookAt(prevPos, newPos, mMatrix.up);
        mMatrix.position = newPos;
        }
        }
        */
        /*vec_t closestPoint;
        tquaternion closestQuat;
        */
        vec_t trackMiddle;
        matrix_t trackMat;
        float leftrightfactor;

        float distanceToBorder = 0;

        //int prevmacuridx = maCurIdx;




        float mUpDownFactor; // unused for now
        int maCurFourche = 0;

        if (!mbFreedFromRoad)
        {
            if (track.getClosestSampledPoint(mMatrix.position, trackMat, trackMiddle, maCurIdx, leftrightfactor,-mMatrix.up, mUpDownFactor, distanceToBorder, maCurFourche, false))
            {

                vec_t mCurrentPlan = buildPlan(trackMat.position, trackMat.up);
                float distToGround = mCurrentPlan.signedDistanceTo( mMatrix.position );
                float difDist = distToGround-1.5f;
                /*
                printf(" idx : %d pos(%5.4f,%5.4f,%5.4f) d2g: %5.4f LR: %5.4f \n", maCurIdx, mMatrix.position.x,mMatrix.position.y,mMatrix.position.z,
                distToGround, leftrightfactor );
                */
                if (difDist<0.f)
                {
                    vec_t newPos = mMatrix.position - mMatrix.up * difDist;
                    mMatrix.LookAt(prevPos, newPos, mMatrix.up);
                    mMatrix.position = newPos;
                }
                if ( (leftrightfactor<0.f) || (leftrightfactor>1.f) )
                    mbFreedFromRoad = true;
            }
            else
            {
                mbFreedFromRoad = true;
            }
        }
    }
	if (mRoadBrickTargeting == 0xFFFF)
	{
		const int TargetingNetIndex = GetTargeting();
		if ( Game::IsValidObjectNetIndex(TargetingNetIndex) )
		{
			GameShip *pShip = GGame->GetShipByNetIndex(TargetingNetIndex);
			if (pShip)
			{
				matrix_t goodWayToGo;
				goodWayToGo.LookAt(mMatrix.position, pShip->GetPhysics()->GetPosition(), vec(0.f, 1.f, 0.f));
				mMatrix.lerp(mMatrix, goodWayToGo, aTimeEllapsed * 5.f);
				mMatrix.orthoNormalize();

				// distance to ship explode
				vec_t distance = (mMatrix.position - pShip->GetPhysics()->GetPosition() );
				distance.w = 0;
				if ( distance.lengthSq() < 10.f )
				{
					SetShouldBeDestroyed();
					GGame->Spawn(GO_Explosion, mMatrix, this);
					return;
				}
			}
		}
	}
	else
	{
		// road brick index
		const AIPoint_t& pt = track.getAIPoint( mRoadBrickTargeting );
		
		matrix_t goodWayToGo;
		goodWayToGo.LookAt(mMatrix.position, pt.mAIPos[0], vec(0.f, 1.f, 0.f));
		mMatrix.lerp(mMatrix, goodWayToGo, aTimeEllapsed*10.f);
		mMatrix.orthoNormalize();
		// distance to ship explode
		vec_t distance = (mMatrix.position - pt.mAIPos[0] );
		distance.w = 0;
		if ( distance.lengthSq() < 10.f )
		{
			SetShouldBeDestroyed();
			GGame->Spawn(GO_Explosion, mMatrix, this);
			return;
		}
	}

    mTTL -= aTimeEllapsed;
    if (mTTL <= 0.f)
    {
        SetShouldBeDestroyed();
        GGame->Spawn(GO_Explosion, mMatrix, this);
    }

}

void GameMissile::InitForRender()
{
    ASSERT_GAME( mMissile == NULL );

    mMissile = getSprite(6)->clone();
    mMissile->color = vec(1.f);
    mMissile->visible = true;

    ASSERT_GAME( mOcclusion == NULL );

    mOcclusion = Renderer::NewOcclusionQuery();
    mOcclusion->mMesh = mMissile;

    ASSERT_GAME( mTrail == NULL );
    mTrail = new Trail;
    /*
    const shipProperties_t& props = getShipPropertiesPackage(mModelName);
    vec_t trailColor = vec(getShipProperty(props, "TrailRed"),
    getShipProperty(props, "TrailGreen"),
    getShipProperty(props, "TrailBlue"), 1.f);
    */
    mTrail->SetColor(vec(0.5f, 0.5f, 0.5f, 1.f));
    mTrail->SetCurrentPoint( mMatrix.position, 0.f, true ) ;
    mTrail->SetCurrentPoint( mMatrix.position, 1.f, true ) ;
    mTrail->SetMinimalDistance(1.5f);
    mTrail->SetWidth(0.15f);
    mTrail->SetLossPerSecond(2.0f);
    /*
    uint32 hash = hashFast( "missileLaunch" );
    mLaunchedSound = new sf::Sound( *GetSound( hash ) );
    SetSoundPresets(mLaunchedSound, hash );
    mLaunchedSound->Play();
    mLaunchedSound->SetRelativeToListener(false);

    hash = hashFast( "enginesMissile" );
    mPropulsorSound = new sf::Sound( *GetSound( hash ) );
    SetSoundPresets(mPropulsorSound, hash );
    mPropulsorSound->SetLoop(true);
    mPropulsorSound->Play();
    mPropulsorSound->SetRelativeToListener(false);
    */
    const gameSettings_t& gsettings = GameSettings;

    ASSERT_GAME( mReactorLight == NULL );
    mReactorLight = Renderer::newOmni();
    mReactorLight->mColor = vec( gsettings.MissileLightRed, gsettings.MissileLightGreen, gsettings.MissileLightBlue, 1.f );

}

void GameMissile::TickForRender(float aTimeEllapsed)
{
    UNUSED_PARAMETER(aTimeEllapsed);

    matrix_t resc, scalc;
    resc.translation(-7.f, 0.f, -7.f);
    scalc.scale(GameSettings.MissileRadius, GameSettings.MissileRadius, GameSettings.MissileRadius);

    mOcclusion->mWorldMatrix = mMissile->mWorldMatrix = resc * scalc * mMatrix;
    mMissile->updateWorldBSphere();

    vec_t smg = vec(6.f, 7.f, 3.f);
    smg.TransformPoint(mMissile->mWorldMatrix);
    mTrail->SetCurrentPoint( smg ) ;

    // sounds
    /*
    if (mPropulsorSound)
    {
    mLaunchedSound->SetPosition( smg.x, smg.y, smg.z );
    mPropulsorSound->SetPosition( smg.x, smg.y, smg.z );
    }
    */
    mReactorLight->mPosition = vec( smg.x, smg.y, smg.z, GameSettings.MissileLightRadius );
}

void GameMissile::UninitForRender()
{
    Renderer::destroyOmni( mReactorLight );
    mReactorLight = NULL;

    /*
    delete mLaunchedSound;
    delete mPropulsorSound;
    */

    delete mTrail;
    mTrail = NULL;

    if (mOcclusion)
    {
        Renderer::DeleteOcclusionQuery( mOcclusion );
        mOcclusion = NULL;
    }

    delete mMissile;
    mMissile = NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

GameMine::GameMine(const matrix_t& mat) : GameObject(GO_Mine, mat)
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= GameMine::Owner_Net_Index_Max );
    mOwner = Game::Invalid_Object_Net_Index;

    mLight = NULL;
    mMine = NULL;
}

GameMine::~GameMine()
{
    UninitForRender();
}

void GameMine::InitForRender()
{
    matrix_t scalc;

    float mineWidth = getSprite(5)->bSphere.w;
    float aMineScaleUniform = GameSettings.mMineRadius * (1.f/mineWidth);
    scalc.scale( -aMineScaleUniform, aMineScaleUniform, aMineScaleUniform );
    mMine = getSprite(5)->clone();
    mMine->color = vec(1.f);
    mMine->visible = true;
    mMine->mBaseMatrix =  scalc * mMatrix;
    mMine->updateWorldBSphere();

    // anim
    meshAnimation_t *pa = mMine->addAnimation();
    pa->mDuration = 3.f;
    pa->mThreshold = 0.1f;


    pa->mTranslationStart = vec( 0.f, mineWidth*0.5f, 0.f );
    pa->mTranslationEnd = vec( 0.f, mineWidth*1.1f, 0.f );
    pa->mTranslationType = 2;
    pa->mTranslationInterpolation = 1;


    pa->mRotationStart = vec(0.f, 0.f, 0.f);
    pa->mRotationEnd = vec(0.f, 2.f*PI, 0.f);
    pa->mRotationType = 1;
    pa->mRotationInterpolation = 0;

    // sound
    Channel *pChan = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/dropmine.ogg" ), AUDIO_GROUP_GAME3D );
    if ( pChan != NULL )
    {
        pChan->SetPosition( mMatrix.position );
        pChan->SetVolume(1.f);
        pChan->SetPitch(1.f);
        pChan->Play();
    }

    mLight = Renderer::newOmni();
    mLight->mColor = vec( 1.0f, 0.3f, 0.3f, 20.f );
    mLight->mPosition = mMatrix.position;
    mLight->mPosition.w = 20.f;
}

void GameMine::TickForRender(float aTimeEllapsed)
{
    UNUSED_PARAMETER(aTimeEllapsed);
}

void GameMine::UninitForRender()
{
    Renderer::destroyOmni( mLight );
    mLight = NULL;

    delete mMine;
    mMine = NULL;
}

u32 GameMine::GetOwnerNetIndex() const
{
    return mOwner;
}
void GameMine::SetOwnerNetIndex(u32 OwnerNetIndex)
{
    ASSERT_GAME( OwnerNetIndex <= GameMine::Owner_Net_Index_Max );
    mOwner = static_cast<u8>(OwnerNetIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GameExplosion::GameExplosion( const matrix_t& mat ) : GameObject(GO_Explosion, mat ), mBrickIndex(0xFFFF), mBrickOffset(0.f)
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= GameExplosion::Owner_Net_Index_Max );
    mOwner = Game::Invalid_Object_Net_Index;

    //const shipProperties_t& props = getShipPropertiesPackage("Game");
    mTTLRT = mTTL = GameSettings.ExplosionDuration;//getShipProperty(props, "ExplosionDuration");
    mExpRadius = GameSettings.ExplosionMaxRadius;//getShipProperty(props, "ExplosionMaxRadius");
    mLifeSpan = 0.f;
    mRadius = 1.f;
	mExplosionType = FIRE_BALL;
    mExplosionLight = NULL;
}

GameExplosion::~GameExplosion()
{
    UninitForRender();
}

void GameExplosion::SetExplosionType( ExplosionType atype )
 { 
	mExplosionType = (u8)atype; 
	switch (atype)
	{	
	case FIRE_BALL:
	// defaulted in c-tor
	break;
	case FIRE_WALL:
	case PLASMA_WALL:
	case ELECTRIC_WALL:
	mTTLRT = mTTL = 3.f;
	break;
	}
 }

 int GameExplosion::GetCurrentBrickIndex() const
 {
	return mBrickIndex + (int)mBrickOffset;
 }

void GameExplosion::Tick(float aTimeEllapsed)
{
    mLifeSpan += aTimeEllapsed;
	mTTLRT -= aTimeEllapsed;
    if (mLifeSpan >= mTTL)
    {
        SetShouldBeDestroyed();
    }
	if ( mBrickIndex != 0xFFFF )
	{
		const AIPoint_t& pt = track.getAIPoint( mBrickIndex + (int)mBrickOffset );
		const AIPoint_t& pt2 = track.getAIPoint( mBrickIndex + (int)mBrickOffset + 1);
		matrix_t matrix1, matrix2;
		pt.BuildMatrix( matrix1, 0 );
		pt2.BuildMatrix( matrix2, 0 );
		float alerp = fmodf(mBrickOffset, 1.f);
		mMatrix.lerp( matrix1, matrix2, alerp );

		float endOfLifeScale = zmin( mTTL*2.f, 1.f );

		matrix_t sca;
		sca.scale( vec( LERP( pt.mAIwidth[0], pt2.mAIwidth[0], alerp) * 2.f, 20.f, 12.f) * endOfLifeScale  );
		sca *= mMatrix;

		mMatrix = sca;

		mBrickOffset += aTimeEllapsed * 30.f;
	}
}

void GameExplosion::InitForRender()
{
#if 0
    static mesh_t *ExploSphere = NULL;
    if (!ExploSphere)
        ExploSphere = generateSuper( 0.5f, 0.5f, 0.5f, 4);

    mExploSphere = ExploSphere->clone();//
    mExploSphere->visible = true;
    mExploSphere->mbTransparent = true;


    /*    sf::Sound * psnd = FAFSound( GetSound( hashFast( "explode" ) ) );
    SetSoundPresets(psnd, hashFast( "explode" ) );
    psnd->SetRelativeToListener(false);
    psnd->SetPosition( mMatrix.position.x, mMatrix.position.y, mMatrix.position.z );
    psnd->Play();
    */
#endif
    Channel *pChan = Channel::AllocateChannel( Audio::GetSound( "Datas/Sounds/fx/explode.ogg" ), AUDIO_GROUP_GAME3D );
    if ( pChan != NULL )
    {
        pChan->SetPosition( mMatrix.position );
        pChan->SetVolume(1.f);
        pChan->SetPitch(1.f);
        pChan->Play();
    }

    const gameSettings_t& gsettings = GameSettings;
    ASSERT_GAME( mExplosionLight == NULL );
    mExplosionLight = Renderer::newOmni();
    mExplosionLight->mColor = vec( gsettings.ExplosionLightRed, gsettings.ExplosionLightGreen, gsettings.ExplosionLightBlue, 1.f );
    mExplosionLight->mPosition = vec( mMatrix.position.x, mMatrix.position.y, mMatrix.position.z, gsettings.ExplosionLightRadius );

    for (int i=0;i<100;i++)
    {
        vec_t sparkSpawn = vec( r01()*2.f - 1.f, 1.f, r01()*2.f - 1.f );
        sparkSpawn.TransformVector( mMatrix );

        GSparkles->Spawn( mMatrix.position, normalized(sparkSpawn)*1.35f, 0xFFFFFFFF, 1.5f);
    }
}

void GameExplosion::TickForRender(float aTimeEllapsed)
{
    float expStrength = 1.f - mLifeSpan/mTTL;

    //FIXME: 'decr' isn't used, should it be removed?
    //float decr = Clamp( expStrength * 4.f, 0.f, 1.f);

    

	if (mExplosionType == FIRE_BALL)
	{
		matrix_t sca;
		sca.scale( 10.f * Clamp ( mLifeSpan + 0.8f, 0.8f, 2.f )  );
		sca.position = mMatrix.position;
		matrix_t sca2;
		mRadius = LERP(mRadius, 50.f, aTimeEllapsed*3.f);

		sca2.scale( mRadius );
		sca2.position = mMatrix.position;

		Renderer::mExplosions.push_back( Renderer::explosion_t( sca, sca2, expStrength ) );
		mExplosionLight->mPosition = sca.position;
		mExplosionLight->mPosition.w = 30.f;
	}
	else
	{
		Renderer::mExplosions.push_back( Renderer::explosion_t( mMatrix, 1.f, (ExplosionType)mExplosionType, true ) );

		mExplosionLight->mPosition = mMatrix.position;
		mExplosionLight->mPosition += mMatrix.up;
		mExplosionLight->mPosition.w = 30.f;
	}

	switch ( mExplosionType)
	{	
		case FIRE_BALL:
		case FIRE_WALL:
			mExplosionLight->mColor = vec(1.f, 0.1f, 0.f, 1.f);//0.8f, 0.1f , 1.f );
			break;
		case PLASMA_WALL:
			mExplosionLight->mColor = vec(1.f, 0.4f, 1.f , 1.f );
			break;
		case ELECTRIC_WALL:
			mExplosionLight->mColor = vec(0.6f, 0.6f, 1.f , 1.f );
			break;
	}
    //const gameSettings_t& gsettings = GameSettings;
    //mExplosionLight->mColor /* = vec( gsettings.ExplosionLightRed, gsettings.ExplosionLightGreen, gsettings.ExplosionLightBlue, 1.f ) **/ expStrength;
    mExplosionLight->mColor *= expStrength * 5.f;
	
}

void GameExplosion::UninitForRender()
{
    //    delete mExploSphere;
    Renderer::destroyOmni( mExplosionLight );
    mExplosionLight = NULL;
}

u32 GameExplosion::GetOwnerNetIndex() const
{
    return mOwner;
}
void GameExplosion::SetOwnerNetIndex(u32 OwnerNetIndex)
{
    ASSERT_GAME( OwnerNetIndex <= GameExplosion::Owner_Net_Index_Max );
    mOwner = static_cast<u8>(OwnerNetIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static const float portalWidth = 6.f;

GamePortal::GamePortal( const matrix_t& mat ) : GameObject(GO_Portal, mat )
{
    COMPILE_TIME_ASSERT( Game::Invalid_Object_Net_Index <= GamePortal::Owner_Net_Index_Max );
    mOwner = Game::Invalid_Object_Net_Index;

	mTTL = 5.f;
	mLifeSpan = 0.f;
	mIsForward = true;
	mBrickStart = -1;
	mBrickCount = 0;
	mOffset = 0.f;
	mPortalRender = NULL;
}

GamePortal::~GamePortal()
{
}

bool GamePortal::IsInPortal( GameShip *pShip ) const
{
	int idx = pShip->GetPhysics()->GetCurrentRoadBlockIndex();

	int portalIdxStart = mBrickStart%track.getAIPointCount();
	int portalIdxEnd = portalIdxStart + mBrickCount;

	if (idx < portalIdxStart || idx > portalIdxEnd )
		return false;

	const AIPoint_t& pt = track.getAIPoint( idx );

	float lrFactor = pShip->GetPhysics()->GetLeftRightFactor();
	float shpOffset = -0.5f+(lrFactor-0.5f) * pt.mAIwidth[0] * 2.f; 

	if ( fabsf( shpOffset - mOffset ) > (portalWidth * 0.5f ) )
		return false;

	return true;
}

void GamePortal::Tick(float aTimeEllapsed)
{
	mTTL -= aTimeEllapsed;
	mLifeSpan += aTimeEllapsed;
	if ( mTTL <= 0.f )
		SetShouldBeDestroyed();
}

void GamePortal::InitForRender()
{

	
}

void GamePortal::TickForRender(float /*aTimeEllapsed*/)
{
	if ( !mPortalRender )
	{
		if ( mBrickStart > 0 )
		{
			mesh_t* portal = track.generatePortalMesh( mBrickStart, mBrickCount, mIsForward );
			mPortalRender = Renderer::addPortal();
			mPortalRender->mMesh = portal;
			mPortalRender->mOffset = mOffset;
		}
	}
	else
	{
		float density = zmin(mTTL*3.f, zmin( mLifeSpan*2.f, 1.f ) );

		
		mPortalRender->mWidth = density * portalWidth;
		mPortalRender->mMesh->color.w = density;
	}
}

void GamePortal::UninitForRender()
{
	Renderer::removePortal( mPortalRender );
}

u32 GamePortal::GetOwnerNetIndex() const
{
    return mOwner;
}
void GamePortal::SetOwnerNetIndex(u32 OwnerNetIndex)
{
    ASSERT_GAME( OwnerNetIndex <= GamePortal::Owner_Net_Index_Max );
    mOwner = static_cast<u8>(OwnerNetIndex);
}

void GamePortal::SetBricksInfos(int brickStart, int brickCount, float offset, bool bForward ) 
{ 
	const AIPoint_t& pt = track.getAIPoint(brickStart);

	mBrickStart = brickStart; 
	mBrickCount = brickCount; 
	mOffset = -0.5f+(offset-0.5f) * pt.mAIwidth[0] * 2.f; 
	mIsForward = bForward; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Game::Game():
physicDLLLibrary(0)
{
    memset(&mProps, 0, sizeof(gameProperties_t));
    mInvalidatedBonus.reserve(32);
    mValidatedBonus.reserve(32);

    mPlayerShip[0] = mPlayerShip[1] =  NULL;
    mPlayerEndRaceInactiveTime = 0.f;
    mEndResultsStringsDirty = false;
    mEndResultsStringsCount = 0;


    mGameState = GameStateWaiting;
    ResetNetIndexAllocator();

    mNbGlobalObstacles = 0;

    mStartupLocalTime = 999.f;

    mCurrentRace = NULL;

    mEnduroTotalTimeAvailable = 30.f; // 30seconds
    mEnduroSpeedThreshold = 75.f;
    mTimeBeforeElimination = -1.f;
}

Game::~Game()
{
    DestroyGameObjects();

    ResetNetIndexAllocator();

    trackBonusInstance_t::ClearBonuses();

    Channel::DestroyAllChannels();
}

void Game::DestroyGameObjects()
{
	physicWorld.ClearCollisions();
    const int gameObjCount = mGameObjects.size();

    if ( gameObjCount > 0 )
    {
        GameObject** pObjs = new GameObject* [gameObjCount];

        int cnt = 0;
        std::vector<GameObject*>::iterator iter = mGameObjects.begin();
        for (; iter != mGameObjects.end(); ++iter, cnt++)
            pObjs[cnt] = (*iter);

        for (int i =0;i<cnt;i++)
            this->Destroy(pObjs[i]);

        delete [] pObjs;
    }
#if IS_OS_WINDOWS
	if (physicDLLLibrary)
		FreeLibrary( physicDLLLibrary );
	physicDLLLibrary = NULL;
#endif
}

void Game::SetState(GameState aState)
{
    mGameState = aState;
    if (mGameState == GameStateCountDown)
    {
        for (int i = 0 ; i < 8 ; i++ )
            mEndResultsString[i] = "";

        mCountDown = 6.f;

        // pylons reset
        mStartupLocalTime = -2.f;
        for (int i=0;i<4;i++)
            mStartupPylons[i].Reset();

        //StartupPylon::ResetAll( 2.f );
    }
    if (mGameState == GameStatePlaying)
    {
        mRacingTime = 0.f;
    }
}

void Game::CreateNewGameSolo(int aType, int aNbLaps)
{
    if (GGame)
        delete GGame;
    GGame = new Game;
    GGame->mbIsAuth = true;
    GGame->mbSupportsRendering = true;
    GGame->SetType(static_cast<u8>(aType));
    GGame->SetTrack(0);
    GGame->SetNumberOfLapsToDo(static_cast<u8>(aNbLaps));
	GGame->SetSplit( false );
}

void Game::DestroyGame()
{
    if (GGame)
        delete GGame;
    GGame = NULL;
}

void Game::SetType(u8 aType)
{
    mProps.mType = aType;
    mProps.mbDirtyForNetwork |= true;
    mProps.mbDirtyForWorld |= true;
}
void Game::SetTrack(u16 aTrack)
{
    mProps.mTrackIndex = aTrack;
    mProps.mbDirtyForNetwork |= true;
    mProps.mbDirtyForWorld |= true;
}

void Game::SetNumberOfLapsToDo(u8 aNbLaps)
{
    mProps.mNumberOfLapsToDo = aNbLaps;
    mProps.mbDirtyForNetwork |= true;
    mProps.mbDirtyForWorld |= true;
}

void Game::SetSplit( bool bSplit )
{
    mProps.mSplitScreen = bSplit;
    mProps.mbDirtyForNetwork |= true;
    mProps.mbDirtyForWorld |= true;
}



void Game::InitStartupPylons(const matrix_t mats[4])
{
    for (int i=0;i<4;i++)
    {
        mStartupPylons[i].Init( mats[i] );
    }
}

GameObject* Game::Spawn( u8 objectType, const matrix_t& mat, GameObject* parent )
{
    GameObject *pObj = NULL;
    switch (objectType)
    {
    case GO_Ship:
#if IS_OS_WINDOWS
		if (!physicDLLLibrary)
			LoadPhysicExtension();
#endif
        pObj = new GameShip( mat, mGameObjectTyped[GO_Ship].size() );
        if (this->IsAuth())
        {
            const u32 NewNetIndex = AllocateNetIndex();
            ASSERT_GAME( Game::IsValidObjectNetIndex(NewNetIndex) );
            ((GameShip*)pObj)->SetOwnerNetIndex( NewNetIndex );
        }
        break;
    case GO_Missile:
        pObj = new GameMissile( mat );
        if (parent)
            ((GameMissile*)pObj)->SetOwnerNetIndex( parent->GetOwnerNetIndex() );
        break;
    case GO_Mine:
        pObj = new GameMine( mat );
        if (parent)
            ((GameMine*)pObj)->SetOwnerNetIndex( parent->GetOwnerNetIndex() );

        break;
    case GO_Explosion:
        pObj = new GameExplosion( mat );
        if (parent)
            ((GameExplosion*)pObj)->SetOwnerNetIndex( parent->GetOwnerNetIndex() );

        break;
	case GO_Portal:
		pObj = new GamePortal( mat );
        if (parent)
            ((GameExplosion*)pObj)->SetOwnerNetIndex( parent->GetOwnerNetIndex() );
		break;
    default:
        ASSERT_GAME_MSG(0, "Unsupported object type %d", objectType);
        break;
    }

    if (mbSupportsRendering)
        pObj->InitForRender();

    mGameObjects.push_back(pObj);
    mGameObjectTyped[pObj->GetType()].push_back(pObj);
    /*
    if ((!mPlayerShip) && pObj->GetType() == GO_Ship)
    {
    SetPlayerShip( (GameShip*)pObj );
    }
    */
    pObj->HandleNetworkInstance();

    return pObj;
}



GameObject* Game::SpawnShip( const matrix_t& mat, const shipIdentifier_t& shipID, u8 iNetIndex )
{
    GameShip *pObj = new GameShip( mat, mGameObjectTyped[GO_Ship].size() );
    if (this->IsAuth())
    {
        const int NewNetIndex = AllocateNetIndex();
        ASSERT_GAME( Game::IsValidObjectNetIndex(NewNetIndex) );
        //FIXME: why setting net index here and set it with iNetIndex just after??
        pObj->SetOwnerNetIndex( NewNetIndex );
    }

    pObj->SetShipIdentifier( shipID );

    if (mbSupportsRendering)
        pObj->InitForRender();

    mGameObjects.push_back(pObj);
    mGameObjectTyped[pObj->GetType()].push_back(pObj);

    if ((!mPlayerShip[0]) && pObj->GetType() == GO_Ship)
    {
        mPlayerShip[0] = pObj;
        mPlayerShip[0]->SetName(GetUserName().c_str());
    }

    pObj->SetOwnerNetIndex( iNetIndex );

    pObj->HandleNetworkInstance();

    return pObj;
}

void Game::Destroy(GameObject* pObj)
{
    if ( ( this->GetPlayerShip() == pObj ) && ( camera.GetObservedShip() ) )
        camera.SetCameraCustom();

    if (mbSupportsRendering)
        pObj->UninitForRender();

    std::vector<GameObject*>::iterator iter = mGameObjects.begin();
    for (; iter != mGameObjects.end(); )
    {
        if (  (*iter) == pObj)
        {

            iter = mGameObjects.erase(iter);
            break;
        }
        else
            ++ iter;
    }
    iter = mGameObjectTyped[pObj->GetType()].begin();
    for (; iter != mGameObjectTyped[pObj->GetType()].end(); )
    {
        if (  (*iter) == pObj)
        {
            iter = mGameObjectTyped[pObj->GetType()].erase(iter);
            break;
        }
        else
            ++ iter;
    }

	for (int i = 0;i<2;i++)
	{
		if (mPlayerShip[i] == pObj)
			mPlayerShip[i] = NULL;
	}
    pObj->HandleNetDestruction();

    delete pObj;
}

GameShip *Game::GetLastRankedOnLapsCount() const
{
    float lowestLapsDone = 99999.f;
    GameShip *lowestShip = NULL;
    std::vector<GameObject*>::const_iterator iter;
    iter = mGameObjectTyped[GO_Ship].begin();
    for (; iter != mGameObjectTyped[GO_Ship].end(); ++iter)
    {
        GameShip *ps = (GameShip*)(*iter);
        if ( ps->mProps.mNumberLapsDone <= lowestLapsDone && ps->mProps.mHealth > 0.f )
        {
            lowestLapsDone = ps->mProps.mNumberLapsDone;
            lowestShip = ps;
        }

    }
    return lowestShip;
}

unsigned int Game::GetRankBasedOnPoints( GameShip *pShip ) const
{
    unsigned int res = 1;
    std::vector<GameObject*>::const_iterator iter;
    iter = mGameObjectTyped[GO_Ship].begin();
    for (; iter != mGameObjectTyped[GO_Ship].end(); )
    {
        const GameShip *ps = (const GameShip*)(*iter);
        if (ps->mProps.mHealth > 0.f && ps->mProps.mPoints > pShip->mProps.mPoints )
        {
            res++;
        }
    }
    return res;
}

const char* Game::RankSuffixText( unsigned int aRank )
{
    if ( aRank == 1 )
        return "st";
    else if ( aRank == 2 )
        return "nd";
    else if ( aRank == 3 )
        return "rd";
    return "th";
}


unsigned int Game::GetStillAliveShipCount() const
{
    unsigned int res = 0;
    std::vector<GameObject*>::const_iterator iter;
    iter = mGameObjectTyped[GO_Ship].begin();
    for (; iter != mGameObjectTyped[GO_Ship].end(); ++iter )
    {
        const GameShip *ps = (const GameShip*)(*iter);
        if (ps->mProps.mHealth > 0.f )
        {
            res++;
        }
    }
    return res;
}


void Game::RenderFlare( const vec_t& worldReactorPosition, float reactorWidth, const vec_t& reactorInvDirection, unsigned int nbPixelsRendered )
{
    vec_t lensClipSpacePos = Camera::worldPosToClipSpace( camera.mViewProj, worldReactorPosition );
    vec_t lensClipSpacePos2 = Camera::worldPosToClipSpace( camera.mViewProj, worldReactorPosition + camera.mViewInverse.right * reactorWidth );


    float lensflareStrength = (lensClipSpacePos.z>45.f)?(10.f/(lensClipSpacePos.z-44.f)):1.f;


    float flareStrength = (reactorInvDirection.dot( normalized(camera.mViewInverse.position - worldReactorPosition) )<0.f)?1.f:0.f;
    lensflareStrength *= flareStrength;

    float flareWidth = distance(lensClipSpacePos2, lensClipSpacePos);

    if ( lensClipSpacePos.x< -1.f) lensflareStrength += (lensClipSpacePos.x+1.f)/flareWidth;
    if ( lensClipSpacePos.y< -1.f) lensflareStrength += (lensClipSpacePos.y+1.f)/flareWidth;

    if ( lensClipSpacePos.x> 1.f) lensflareStrength -= (lensClipSpacePos.x-1.f)/flareWidth;
    if ( lensClipSpacePos.y> 1.f) lensflareStrength -= (lensClipSpacePos.y-1.f)/flareWidth;

    float nbPixVisible = nbPixelsRendered/*ps->GetOcclusion()->mNbPixelsRendered*/?1.f:0.f;
    lensflareStrength *= nbPixVisible;
    flareStrength *= nbPixVisible;
    lensflareStrength = Clamp( lensflareStrength, 0.f, 1.f );
    flareStrength = Clamp( flareStrength, 0.f, 1.f );

    if ( ( lensflareStrength > FLOAT_EPSILON) && ( flareStrength > FLOAT_EPSILON ) )
        Renderer::AddLensFlare( lensClipSpacePos, flareWidth*0.66f, lensflareStrength, flareStrength );
}



void Game::Tick(float aTimeEllapsed)
{
    // check load

    // load shits

    if ( mProps.mbDirtyForWorld )
    {
		static MENU_IDS beforeLoading;
		if ( Menus::GetCurrent() != MENU_LOADING ) 
		{
			beforeLoading = Menus::GetCurrent();
			Menus::Show( MENU_LOADING );
		}
		else
		{
			if ( ggui.BlackBoxIsStable() )
			{
				Menus::Show( beforeLoading );
				bool bSetToPrerace = ( camera.GetMode() == CM_PRERACE );

				for (int i=0;i<4;i++)
					mStartupPylons[i].Clear();

				// clear camera/ships
				camera.SetCameraCustom();
				DestroyGameObjects();

				ResetNetIndexAllocator();

				physicWorld.ClearCollisions();

				track_t & toGoForTrack = Tracks[GGame->GetGameProps().mTrackIndex];
				trackBonusInstance_t::ClearBonuses();

				Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME2D );
				Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME3D );

				ClearMeshStack();

				Renderer::clearAllLights();
				Renderer::portals.clear();
				ClearFlashLights();

				track.GoWithTrack( &toGoForTrack, true );

				const gameType_t& gt = GameTypes[mProps.mType];

				Game::GenerateBonuses( toGoForTrack.mBonusGenerationParameters, toGoForTrack,
					gt.bSpeedTrackBonus,
					gt.bWeaponTrackBonus,
					gt.bColorTrackBonus );

				this->ClearFlagPropsDirtyWorld();
				if (!NetServer->Active())
					this->ClearFlagPropsDirtyNetwork();

				NetClient->informServerImReadyToPlay( );

				// reset camera track observe if needed
				if (bSetToPrerace)
					camera.SetPreRace( true );
			}
		}
    }

    //


    std::vector<GameObject*>::iterator iter;
    //aTimeEllapsed *= world.GetGameSpeed();

    if (aTimeEllapsed <= 0.001f)
        return;

    // tick things

    GameObject *tmpObjs[512];
    int tmpObjsAv = 0;

    for (iter = mGameObjects.begin(); iter != mGameObjects.end() ; iter++)
    {
        tmpObjs[tmpObjsAv++] = (*iter);
    }

    for (int ito = 0;ito<tmpObjsAv;ito++)
        tmpObjs[ito]->Tick(aTimeEllapsed);

    // game mode


    // pylons
    //StartupPylon::TickAll( aTimeEllapsed );
    if (mStartupLocalTime <5.f)
    {
        float mNextSuLT = mStartupLocalTime + aTimeEllapsed;

        for (int i = 0 ; i < 4 ; i ++)
            mStartupPylons[i].Tick( mStartupLocalTime, mNextSuLT, aTimeEllapsed );

        mStartupLocalTime = mNextSuLT;
    }


    // state

    switch (mGameState)
    {
    case GameStateWaiting: break;
    case GameStateCountDown:
        {


            float ncd = mCountDown - aTimeEllapsed;
            if ((mCountDown >3.f)&&(ncd<3.f))
            {
                if ( mbSupportsRendering )
                {
                    /*
                    static sf::Sound *sndThree = NULL;
                    if (!sndThree) sndThree = new sf::Sound( *GetSound( hashFast( "Three" ) ) );
                    sndThree->Play();
                    */
                    Audio::PlaySound( Audio::GetSound("Datas/Sounds/droid/Three.ogg"), AUDIO_GROUP_GAME2D );
                    Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/BeepStartOne.ogg"), AUDIO_GROUP_GAME2D );
                }
                // 3!
                LOG("3!!!\n");
            }
            else if ((mCountDown >2.f)&&(ncd<2.f))
            {
                if ( mbSupportsRendering )
                {
                    /*
                    static sf::Sound *sndTwo = NULL;
                    if (!sndTwo) sndTwo = new sf::Sound( *GetSound( hashFast( "Two" ) ) );
                    sndTwo->Play();
                    */
                    Audio::PlaySound( Audio::GetSound("Datas/Sounds/droid/Two.ogg"), AUDIO_GROUP_GAME2D );
                    Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/BeepStartOne.ogg"), AUDIO_GROUP_GAME2D );

                }
                LOG("2!!!\n");
            }
            else if ((mCountDown >1.f)&&(ncd<1.f))
            {
                if ( mbSupportsRendering )
                {
                    /*
                    static sf::Sound *sndOne = NULL;
                    if (!sndOne) sndOne = new sf::Sound( *GetSound( hashFast( "One" ) ) );
                    sndOne->Play();
                    */
                    Audio::PlaySound( Audio::GetSound("Datas/Sounds/droid/One.ogg"), AUDIO_GROUP_GAME2D );
                    Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/BeepStartOne.ogg"), AUDIO_GROUP_GAME2D );
                }
                LOG("1!!!\n");
            }
            else if ((mCountDown >0.f)&&(ncd<0.f))
            {
                if ( mbSupportsRendering )
                {
                    /*
                    static sf::Sound *sndGo = NULL;
                    if (!sndGo) sndGo = new sf::Sound( *GetSound( hashFast( "go" ) ) );
                    sndGo->Play();
                    */
                    Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/BeepStartTwo.ogg"), AUDIO_GROUP_GAME2D );
                }
                LOG("GO   !!!\n");
                for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
                {
                    GameShip * ps = (GameShip*)(*iter);
                    ps->mProps.ctrlEnabled = true;
                }
                SetState(GameStatePlaying);
            }
            mCountDown = ncd;
        }
        break;
    case GameStatePlaying:
        mRacingTime += aTimeEllapsed;
        if ( IsAuth() )
        {
            TickGamePlay(aTimeEllapsed);
        }
        break;
    }

    // auth ticks
    if ( IsAuth() )
    {
        // auth delete
        {

            for (int ito = 0;ito<tmpObjsAv;ito++)
            {
                if (tmpObjs[ito]->ShouldBeDestroyed())
                {
                    this->Destroy( tmpObjs[ito] );
                }
            }

        }

        // bonus
        trackBonusInstance_t::TickBonuses( aTimeEllapsed );

        // ship/bonus
        float invAIBricksCount = 1.f/static_cast<float>(track.getAIPointCount());

        for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
        {
            GameObject *object = (*iter);
            GameShip *ps = static_cast<GameShip*>(object);

            const vec_t& pcur = ps->GetPhysics()->GetCurrentPosition();
            const vec_t& pprev = ps->GetPhysics()->GetPreviousPosition();
            IPhysicShip *phys = ps->GetPhysics();

            // track done

            if ( phys->IsLapDone() )
            {
                phys->ClearLapDoneFlag();
                ps->IncrementCompletedTrackLaps();
                printf(" %5.2f laps done\n", ps->GetCompletedTrackLaps() );
                if ( ps == GetPlayerShip() )
                    mHUD.PushNewLap();

                if ( GetType() == 2 ) // destruction
                {
                    ps->mProps.mHealth += 0.5f;
                }
            }

            float nbbic = (float)phys->GetCurrentRoadBlockIndex();
            if (nbbic < 99999.f)
                ps->mProps.mNumberLapsDone = ps->GetCompletedTrackLaps() + nbbic*invAIBricksCount;

            //
            Bonus* AcquiredBonus = ps->GetAcquiredBonus();
            if ( AcquiredBonus != NULL )
            {
                AcquiredBonus->Tick(ps, aTimeEllapsed);
                if ( AcquiredBonus->CanBeDestroyed() )
                    ps->ClearBonus();

                AcquiredBonus = NULL;
            }
            trackBonusInstance_t::DetectHandleShipOnTrackBonus(pprev, (pcur-pprev), ps);

            // other objects collision
            std::vector<GameObject*>::iterator itero;
            float aDist = (pcur-pprev).length();

            vec_t shipColSphere = ps->GetPhysics()->GetPosition();
            shipColSphere.w = GameSettings.ShipRadius;
            //float distFrameShip = (pprev-pcur).length();

            vec_t invNormalizedShipDir = -normalized(phys->GetTransform().dir);
            // ship/mine

            for (itero = mGameObjectTyped[GO_Mine].begin(); itero != mGameObjectTyped[GO_Mine].end() ; ++itero)
            {
                GameObject *mine = (*itero);
                const matrix_t& mat = mine->GetMatrix();

                vec_t colSphere = mat.position;
                colSphere.w = GameSettings.mMineRadius*2.f;

                float nDistRaySphere = IntersectRaySphere(pprev, normalized(pcur-pprev), colSphere);
                float distPointPoint = (pprev-colSphere).length();//vec( (pprev-colSphere).x, 0.f, (pprev-colSphere).z).length();
                if ( ( (nDistRaySphere <= aDist) && (nDistRaySphere > 0.f) ) || (distPointPoint< colSphere.w) )
                {
                    this->Spawn( GO_Explosion, mat, ps );
                    //this->Destroy( (*itero) );
                    mine->SetShouldBeDestroyed();
								if (!ps->mProps.mShield)
			{

                    ps->mProps.mHealth -= GameSettings.MineHealthDamage;
                    ps->mProps.mBoost += invNormalizedShipDir * GameSettings.MineBoostDecrease;
					}
                }
            }

            // ship/missile
            for (itero = mGameObjectTyped[GO_Missile].begin(); itero != mGameObjectTyped[GO_Missile].end() ; ++itero)
            {
                GameObject *mis = (*itero);
                const matrix_t& mat = mis->GetMatrix();

                float misRunLength = aTimeEllapsed * GameSettings.MissileSpeed;
                float nDistRaySphere = IntersectRaySphere(mat.position, mat.dir, shipColSphere);

                vec_t missileShip = shipColSphere - mat.position;
                float distMissileShip = missileShip.length();
                missileShip *= 1.f/distMissileShip;

                float dotDestination = missileShip.dot(mat.dir);
                if (dotDestination <0.f)
                    continue;

                if ( ( (nDistRaySphere <= misRunLength) && (nDistRaySphere > 0.f) ) || (distMissileShip< shipColSphere.w) )
                {
                    this->Spawn( GO_Explosion, mat, ps );
                    //this->Destroy( (*itero) );
                    mis->SetShouldBeDestroyed();
								if (!ps->mProps.mShield)
			{

                    ps->mProps.mHealth -= GameSettings.MissileHealthDamage;
                    ps->mProps.mBoost += invNormalizedShipDir * GameSettings.MissileBoostDecrease;
					}
                }
            }
            // ship/explosion
			if (!ps->mProps.mShield)
			{
				for (itero = mGameObjectTyped[GO_Explosion].begin(); itero != mGameObjectTyped[GO_Explosion].end() ; ++itero)
				{
					GameExplosion *exp = static_cast<GameExplosion*>(*itero);
					const matrix_t& mat = exp->GetMatrix();

					float prevDist = (mat.position - pprev).length();
					float curDist = (mat.position - pcur).length();
					float aSpheresRadius = exp->GetRadius() + shipColSphere.w;
				
					if ( exp->GetType() == FIRE_BALL )
					{
						if ( ( prevDist > aSpheresRadius ) && ( curDist <= aSpheresRadius ) )
						{
							ps->mProps.mHealth -= GameSettings.InExplosionHealthDamage;
							ps->mProps.mBoost += invNormalizedShipDir * GameSettings.InExplosionBoostDecrease;
						}
					}
					else 
					{
						// wall based explosion
						if ( abs( ps->GetPhysics()->GetCurrentRoadBlockIndex() - exp->GetCurrentBrickIndex() ) < 2 )
						{
							switch ( exp->GetExplosionType() )
							{
								case FIRE_WALL:
									{
										if ( ps == mPlayerShip[0] )
											Renderer::newVelvet( vec(1.f,0.05f,0.05f), 0);
										else if ( ps == mPlayerShip[1] )
											Renderer::newVelvet( vec(1.f,0.05f,0.05f), 1);
										ps->GetPhysics()->SetZeroSpeed();
									}
									break;
								case PLASMA_WALL:
									ps->mProps.mCommandJitter = true;
									ps->mCommandJitterTime = 4.f;
									break;
								case ELECTRIC_WALL:
									ps->mProps.mGravityGrip = true;
									ps->mGravityGripTime = 4.f;
									break;
							}
						}
					}
				}
			}
        }


        // obstacles
        BuildObstacleList();

        // detect ships with <0 health
    }

	// portals, works for authoritative or not
    for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
    {
        GameObject *object = (*iter);
        GameShip *ps = static_cast<GameShip*>(object);
		std::vector<GameObject*>::iterator itero;

		for (itero = mGameObjectTyped[GO_Portal].begin(); itero != mGameObjectTyped[GO_Portal].end() ; ++itero)
        {
			GamePortal *portal = (GamePortal *)(*itero);
			if ( portal->IsInPortal( ps ) )
			{
				if ( portal->IsForward() )
				{
					ps->FastForward( 1, portal->GetOffset() );
				}
				else if (! ps->mProps.mShield )
				{
					ps->FastForward( -1, portal->GetOffset() );
				}
			}
		}
	}

    // inactivity in case of end of racing
	if (mPlayerShip[0] && mPlayerShip[1])
	{
		// splitscreen
		if ( mPlayerShip[0]->mProps.mRank && mPlayerShip[1]->mProps.mRank)
			mPlayerEndRaceInactiveTime += aTimeEllapsed;
	}
	else
	{
		if (mPlayerShip[0] && mPlayerShip[0]->mProps.mRank ) //(mPlayerShip->mProps.mNumberLapsDone >= (float)mProps.mNumberOfLapsToDo))
		{
			mPlayerEndRaceInactiveTime += aTimeEllapsed;
		}
	}
    // for every one : bullets
    TickBullets(aTimeEllapsed);

    // collisions
    const PhysCollision_t *pc = physicWorld.GetCollisions().GetFirst();
    const PhysCollision_t *pcEnd = physicWorld.GetCollisions().GetEndUsed();

    //float maxFriction = 0.f;

    for (; pc != pcEnd; pc++)
    {
        float impactValue = Clamp( (pc->mDistance * 0.0003f), 0.0f, 1.f);
        if ( pc->mObject1 )
        {
            //ASSERT_GAME( std::find( mGameObjects.begin(), mGameObjects.end(), pc->mObject1) != mGameObjects.end() );
			if ( std::find( mGameObjects.begin(), mGameObjects.end(), pc->mObject1) != mGameObjects.end() )
			{
				pc->mObject1->mProps.mHealth -= impactValue;
				pc->mObject1->ResetPID();
			}
        }
        if ( pc->mObject2 )
        {
            //ASSERT_GAME( std::find( mGameObjects.begin(), mGameObjects.end(), pc->mObject2) != mGameObjects.end() );
			if ( std::find( mGameObjects.begin(), mGameObjects.end(), pc->mObject2) != mGameObjects.end() )
			{
				pc->mObject2->mProps.mHealth -= impactValue;
				pc->mObject2->ResetPID();
			}
        }
    }
}

void Game::Interpolate( float aLerp )
{
    std::vector<GameObject*>::iterator iter;
    std::vector<GameObject*>::iterator itero;

    for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
    {
        GameShip * ps = (GameShip*)(*iter);
        ps->Interpolate( aLerp );
    }
    for (itero = mGameObjectTyped[GO_Missile].begin(); itero != mGameObjectTyped[GO_Missile].end() ; ++itero)
    {
        GameMissile *mis = (GameMissile*)(*itero);
        mis->Interpolate( aLerp );
    }
}

void Game::TickForRender(float aTimeEllapsed)
{
    if (!mbSupportsRendering)
        return;

    std::vector<GameObject*>::iterator iter;
    // lens flare
    for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
    {
        GameShip * ps = (GameShip*)(*iter);
        if ( ps->mProps.mHealth > 0.f )
            RenderFlare( ps->GetWorldReactorPosition(), 1.2f, ps->GetPhysics()->GetTransform().dir, ps->GetOcclusion()->mNbPixelsRendered);
    }
    for (iter = mGameObjectTyped[GO_Missile].begin(); iter != mGameObjectTyped[GO_Missile].end() ; ++iter)
    {
        GameMissile * ps = (GameMissile*)(*iter);
        RenderFlare( ps->GetWorldReactorPosition(), 0.6f, ps->GetMatrix().dir, ps->GetOcclusion()->mNbPixelsRendered );
    }

    /*std::vector<GameObject*>::iterator*/ iter = mGameObjects.begin();
    for (; iter != mGameObjects.end(); ++iter)
        (*iter)->TickForRender( aTimeEllapsed );

    // hud
    if (mGameState == GameStatePlaying )
        mHUD.Tick( aTimeEllapsed );

	bool hasSplit = mPlayerShip[0]&&mPlayerShip[1];
	for (int i = 0;i<2;i++)
	{
		if ( mPlayerShip[i] && ( (Menus::GetCurrent() == MENU_EMPTY_SOLO) || (Menus::GetCurrent() == MENU_EMPTY_NETWORK) || (Menus::GetCurrent() == MENU_NONE ) ) )
		{
			mHUD.Draw( this, mPlayerShip[i], hasSplit?(i+1):0 );
		}
	}

    // bullets
    GBulletSparkles->BuildFromBulletList(mBullets);
    GBulletSparkles->SetWidth( GameSettings.BulletWidth );
    GBulletSparkles->SetLength( GameSettings.BulletWidth * 20.f);
    GBulletSparkles->SetColor( vec( GameSettings.BulletRed, GameSettings.BulletGreen, GameSettings.BulletBlue, 1.f ) );
    // friction sparkles

    //GSparkles->SetWidth( 0.1f );
    //GSparkles->

    const PhysCollision_t *pc = physicWorld.GetCollisions().GetFirst();
    const PhysCollision_t *pcEnd = physicWorld.GetCollisions().GetEndUsed();

    const int collisionMaskIndexCount = E_Max_Ship_Count;

    //float maxFriction = 0.f;
    int nbColsChecked = 0;
    vec_t collisionsReportedPositions[collisionMaskIndexCount];
    u8 collisionsMask = 0;
    static float timeBeforeAnotherWallCollision[collisionMaskIndexCount] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
    for (; pc != pcEnd; pc++)
    {
        nbColsChecked ++;

        GameShip *anyShip = pc->mObject1?pc->mObject1:pc->mObject2;
        if ( anyShip->mProps.mHealth <= 0.f )
            continue;

        float difSpeed = pc->mObject1?pc->mObject1->GetInstantSpeed():0.f;
        difSpeed -= pc->mObject2?pc->mObject2->GetInstantSpeed():0.f;
        //createBoxMarker( pc->mWorldPosition, vec(1.f, 0.f, 1.f, 1.f), 1.f );

        //maxFriction = zmax(maxFriction, fabsf(difSpeed) );
        if ( fabsf(difSpeed) > 28.3f )//FLOAT_EPSILON )
        {

            //createBoxMarker( pc->mWorldPosition, vec(1.f, 0.f, 0.f, 1.f), 1.f );
			/*
			if ( (!pc->mObject1) || (!pc->mObject2) || (!pc->mObject2->GetPhysics()) || (!pc->mObject1->GetPhysics()))
				continue;
				*/
			// the 2 following checks should be removed and the bug(crash) should be found.
			if ( pc->mObject1 && (!ShipExists(pc->mObject1) ))
				continue;
			if ( pc->mObject2 && (!ShipExists(pc->mObject2) ))
				continue;

            vec_t norm = pc->mObject1?pc->mObject1->GetPhysics()->GetTransform().dir:vec( 0.f );
            norm += pc->mObject2?pc->mObject2->GetPhysics()->GetTransform().dir:vec( 0.f );

            norm.normalize();


            vec_t frictionNorm = pc->mWorldNormal - norm * Clamp( difSpeed*0.1f, 0.f, 1.f) + vec(0.f, 1.f, 0.f);
            /*
            frictionNorm = vec(0.f, 1.f, 0.f);
            frictionNorm += vec( r01()*0.2f - 0.1f, r01()*0.2f - 0.1f, r01()*0.2f - 0.1f );
            //frictionNorm += vec(0.f, 1.f, 0.f);
            frictionNorm.normalize();
            frictionNorm *= (0.3f + r01()*0.2f);
            */
            frictionNorm *= 0.195f;

            Sparkles::Spawn( pc->mWorldPosition,  frictionNorm , 0.2f, 1.0f, (fastrand()&3) );
        }
        if ( anyShip )
        {
            const int collisionMaskIndex = anyShip->GetCollisionMaskIndex();
            ASSERT_GAME( collisionMaskIndex < collisionMaskIndexCount );
            if (timeBeforeAnotherWallCollision[collisionMaskIndex]<= FLOAT_EPSILON)
            {
                collisionsMask |= 1<<collisionMaskIndex;
                collisionsReportedPositions[collisionMaskIndex] = pc->mWorldPosition;
                timeBeforeAnotherWallCollision[collisionMaskIndex] = 0.5f;
            }
        }
    }

    for (int i = 0;i<collisionMaskIndexCount;i++)
    {
        if (collisionsMask&(1<<i))
        {
            /*
            sf::Sound *psnd = FAFSound(  GetSound( hashFast( "wallCollision" ) ) );
            psnd->SetRelativeToListener(false);
            const vec_t& wallCollisionSoundPosition = collisionsReportedPositions[i];
            psnd->SetPosition( wallCollisionSoundPosition.x, wallCollisionSoundPosition.y, wallCollisionSoundPosition.z );
            psnd->Play();
            */
        }
        timeBeforeAnotherWallCollision[i] -= aTimeEllapsed;
    }
}

void PlayRicochet( const vec_t& pos, bool bOnShip )
{
    const Sound *psound = bOnShip?Audio::GetSound("Datas/Sounds/fx/BulletHitArmor.ogg"):Audio::GetSound("Datas/Sounds/fx/ricochet.ogg");

    Channel *ricochet = Channel::AllocateChannel( psound, AUDIO_GROUP_GAME3D );
    if ( ricochet != NULL )
    {
        ricochet->Play();
        ricochet->SetPosition( pos );
        ricochet->SetPitch( 0.95f + r01() * 0.1f );
        ricochet->SetVolume(0.3f);
    }
}

void Game::TickBullets(float aTimeEllapsed)
{
    if (mBullets.empty())
        return;

    PROFILER_START(BulletTicks);

    // bullet life and run

    std::vector<bullet_t>::iterator iter =  mBullets.begin();
    std::vector<bullet_t>::iterator iterEnd =  mBullets.end();
    for (; iter != iterEnd ; ++ iter)
    {
        bullet_t& pb = (*iter);
        vec_t prevPos = pb.mPos;
        float aDist = GameSettings.BulletSpeed * aTimeEllapsed;
        pb.mPos += pb.mDir * aDist;
        pb.mTTL -= aTimeEllapsed;
        pb.mbDelete |= (pb.mTTL <= 0.f);

        if (pb.mbDelete)
            continue;

        std::vector<GameObject*>::iterator itero = mGameObjects.begin();
        for (; itero != mGameObjects.end(); ++itero)
        {
            GameObject *object = (*itero);
            const u32 objectType = object->GetType();

            vec_t colSphere;
            switch (objectType)
            {
            case GO_Ship:
                {
                    GameShip *shp = static_cast<GameShip*>(object);
                    colSphere = shp->GetPhysics()->GetPosition();
                    colSphere.w = GameSettings.ShipRadius;
                }
                break;
            case GO_Missile:
                {
                    GameMissile *mis = static_cast<GameMissile*>(object);
                    colSphere = mis->GetMatrix().position;
                    colSphere.w = GameSettings.MissileRadius;
                }

                break;
            case GO_Mine:
                {
                    GameMine *min = static_cast<GameMine*>(object);
                    colSphere = min->GetMatrix().position;
                    colSphere.w = GameSettings.mMineRadius;
                }
                break;
            default:
                break;
            }
            vec_t bulletObject = colSphere-pb.mPos;
            float distBulletObject = bulletObject.length();
            bulletObject *= 1.f/distBulletObject;

            float dotDestination = bulletObject.dot(pb.mDir);
            if (dotDestination <0.f)
                continue;


            float nDistRaySphere = IntersectRaySphere(prevPos, pb.mDir, colSphere);
            float distPointPoint = (prevPos-colSphere).length();
            if ( ( (nDistRaySphere <= aDist) && (nDistRaySphere > 0.f) ) || (distPointPoint< colSphere.w) )
            {
                pb.mbDelete = true;
                // if auth
                //  for ship add damage/ slow speed
                //  for missiles/mine, spawn explosion, destroy mine/explosion
                if ( IsAuth() )
                {
                    if ( objectType == GO_Ship )
                    {
                        GameShip *shp = static_cast<GameShip*>(object);
                        //const matrix_t& shipMat = shp->GetPhysics()->GetTransform();
                        shp->mProps.mHealth -= GameSettings.BulletHealthDamage;

                        float boostLength = shp->mProps.mBoost.length();
                        if (boostLength >= GameSettings.BulletBoostDecrease)
                            shp->mProps.mBoost -= shp->mProps.mBoost * (GameSettings.BulletBoostDecrease/boostLength);
                        else
                            shp->mProps.mBoost.set( 0.f );
                    }
                    else
                    {
                        this->Spawn( GO_Explosion, object->GetMatrix() );
                        //this->Destroy( (*itero) );
                        object->SetShouldBeDestroyed();
                    }
                }

                // if render add sparkles
                if ( mbSupportsRendering )
                {
                    // draw sparkles
                    Sparkles::Spawn( pb.mPos, -pb.mDir);
                    PlayRicochet( pb.mPos, true );
                }


                //no need to go further
                break;
            }
        }



        if (pb.mbDelete)
            continue;

        // bullet/world
        vec_t hitPosition, hitNormal;
        //float hitFactor;
        //vec_t mGroundRayCast;
        if (physicWorld.rayCast(prevPos, pb.mPos, hitPosition, hitNormal))
        {
            pb.mbDelete = true;
            if ( mbSupportsRendering )
            {
                // draw sparkles
                Sparkles::Spawn( hitPosition, hitNormal);

                PlayRicochet( hitPosition, false );
            }

        }
    }

    // remove dead bullets
    iter =  mBullets.begin();
    for (; iter != mBullets.end() ; )
    {
        if ((*iter).mbDelete)
            iter = mBullets.erase(iter);
        else
            ++iter;
    }
    PROFILER_END(); // BulletTicks
}



///////////////////////////////////////////////////////////////////////////////////////////////////

void Game::Restart( int aMode )
{
	ClearValidatedAndInvalidatedBonusesList();
    mPlayerEndRaceInactiveTime = 0.f;
    mEndResultsStringsDirty = false;
    mEndResultsStringsCount = 0;

    if ( aMode >= 0 )
        mProps.mType = static_cast<u8>(aMode);

    // hud
    mHUD.Reset();

    // reset ships
    //Ribbon::ResetAll();
    // only remove particles
    GSparkles->ClearParticles();
    GBulletSparkles->ClearParticles();

    // nomore game objects, create those you want
    this->DestroyGameObjects();

    ResetNetIndexAllocator();

    physicWorld.ClearCollisions();

    Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME2D );
    Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME3D );
#if IS_OS_WINDOWS
	LoadPhysicExtension();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if IS_OS_WINDOWS
void Game::LoadPhysicExtension()
{
	// loadlibrary
#if IS_OS_WINDOWS
#ifdef _DEBUG
	physicDLLLibrary = LoadLibrary("RushPhysicsDebug.dll");
#else
	physicDLLLibrary = LoadLibrary("RushPhysics.dll");
#endif

	NewGamePhysics = (NewGamePhysics_t)(GetProcAddress(physicDLLLibrary,"?fnGamePhysics@@YAPAVIPhysicShip@@PAVGame@@PAVIPhysicWorld@@PAVITrack@@@Z"));
#endif
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string fmts(const std::string& str, int desired_length)
{
    std::string res;
    res = str;
    res += "                                          ";
    return res.substr(0, desired_length);
}
void Game::PushEndRaceStringForShip(GameShip *ps)
{
    char tmps[512];
    int racingMinutes = (int)floorf(mRacingTime/60.f);
    int racingSeconds = (int)floorf(fmodf(mRacingTime,60.f));
    int racingFrac = (int)(fmodf(mRacingTime, 1.f) * 100.f);
    switch (mProps.mType)
    {
    case 0: //classic race
        sprintf(tmps, "%s %2d\"%02d\'%02d   ", fmts(ps->GetName(), 15).c_str() , racingMinutes, racingSeconds, racingFrac);
        break;
    case 1: // eliminator
		sprintf(tmps, "%s                   ", fmts(ps->GetName(), 15).c_str() );
        break;
    }

    ASSERT_GAME( mEndResultsStringsCount < E_Max_Ship_Count );
    mEndResultsString[mEndResultsStringsCount++] = tmps;
    mEndResultsStringsDirty = true;
    //mProps.mbDirtyForEndString |= true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// race is done. we are ranked between 1st and 8th. Not called when wrecked/dead/leaver.

void Game::HandleEndOfRace( GameShip *ps )
{
    if( ps != GGame->GetPlayerShip() )
        return;

    int nbPointsForTeam = 1;
    if ( mCurrentRace )
    {
        nbPointsForTeam += Solo::TagRaceDone( mCurrentRace, ps->mProps.mRank );
    }

    if ( Solo::AddEvolutionPoints( ps->GetShipIndentifier().mTeamIndex, nbPointsForTeam ) )
    {
        // congratulations, new teamdirty evolution award!
        Audio::PlaySound( Audio::GetSound("Datas/Sounds/droid/EvolutionUnlocked.ogg"), AUDIO_GROUP_GUI );

    }
    //Solo::playerProfil.evolutions[ ps->GetShipIndentifier().mTeamIndex ] += nbPointsForTeam;
    mCurrentRace = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// only server side

void Game::TickGamePlay(float aTimeEllapsed)
{


    switch (mProps.mType)
    {
    case 0: //classic race
        {
            std::vector<GameObject*>::iterator iter;
            for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
            {
                GameShip * ps = (GameShip*)(*iter);

                //#ifndef RETAIL
                if ( keysReleased( SDLK_a ) && ps == this->GetPlayerShip() )
                    ps->mProps.mNumberLapsDone += 4.f;
                if ( keysReleased( SDLK_i ) && ps == this->GetPlayerShip(1) )
                    ps->mProps.mNumberLapsDone += 4.f;


                if ( keysReleased( SDLK_b ) && ps != this->GetPlayerShip() )
                    ps->mProps.mNumberLapsDone += 1.f;

                if ( keysReleased( SDLK_g ) && ps == this->GetPlayerShip() )
                    ps->mProps.mHealth -= 0.1f;

                //ps->mProps.mNumberLapsDone += aTimeEllapsed * r01();
                //#endif
                if (ps->mProps.mNumberLapsDone>= (float)mProps.mNumberOfLapsToDo)
                {
                    // ship finished the race
                    if (!ps->mProps.mRank)
                    {
                        ps->mProps.mAutoPilot = true;
                        PushEndRaceStringForShip(ps);
                        ps->mProps.mRank = mEndResultsStringsCount;

                        // handle end of race
                        HandleEndOfRace( ps );
                    }
                }
				/*
                // check death
                if ( ps->mProps.mHealth <= 0 )
                {
                    //                camera.SetCameraEndRace( ps );
                    ps->mProps.mRank = 0xF; // not finished
                    ps->mProps.ctrlEnabled = false;
                    ps->GetPhysics()->EnablePhysics( false );
                }
				*/
            }
        }
        break;

    case 1: // eliminator

        if ( mRacingTime>60.f)
        {
            // next elimination in 30s
            if ( mTimeBeforeElimination < 0.f )
            {
                mTimeBeforeElimination = 30.f;
            }
            if ( mTimeBeforeElimination > 0.f )
            {
                mTimeBeforeElimination -= aTimeEllapsed;
                if ( mTimeBeforeElimination <= 0.f )
                {
                    // eliminate ship
                    GameShip *ps = GetLastRankedOnLapsCount();
                    ps->mProps.mHealth = 0.f;

					PushEndRaceStringForShip(ps);
					ps->mProps.mRank = 9-mEndResultsStringsCount;

					ps->mProps.ctrlEnabled = false;
                    ps->GetPhysics()->EnablePhysics( false );
                }
            }
        }
        break;
    case 3: // cover
        {
        std::vector<GameObject*>::iterator iter;
        for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
        {
            GameShip * ps = (GameShip*)(*iter);
            int pos = GGame->GetPositionForShip( ps );
            ps->mProps.mPoints += static_cast<float>( (8-pos) + 4) * aTimeEllapsed;
        }
        }
        break;
    case 7: // collector
        {
            std::vector<GameObject*>::iterator iter;
            for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
            {
                GameShip * ps = (GameShip*)(*iter);
                ps->mProps.mPoints += ps->GetSmoothedSpeed() * 0.001f;
            }
        }
        break;
    case 8:
        {
            std::vector<GameObject*>::iterator iter;
            for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
            {
                GameShip * ps = (GameShip*)(*iter);
                if (ps->GetSmoothedSpeed() >= GetEnduroSpeedThreshold() )
                {
                    ps->mProps.mGotOverEnduroSpeedThresold = true;
                }

                if ( ps->mProps.mGotOverEnduroSpeedThresold && ps->GetSmoothedSpeed() < GetEnduroSpeedThreshold() )
                {
                    ps->mProps.mHealth -= aTimeEllapsed / GetEnduroTimeAvailable();
                }
            }
        }
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int Game::GetPositionForShip(GameShip *pShip)
{
    int pos = 0;
    std::vector<GameObject*>::iterator iter;
    for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
    {
        GameShip * ps = (GameShip*)(*iter);
        if (ps->mProps.mNumberLapsDone >= pShip->mProps.mNumberLapsDone)
        {
            pos++;
        }
    }

    return pos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool Game::EveryBodyFinishedRacing() const
{
    std::vector<GameObject*>::const_iterator iter;
    for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
    {
        GameShip * ps = (GameShip*)(*iter);
        if ( !ps->mProps.mRank )
        {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Game::AddBullet(const vec_t& pos, const vec_t& dir)
{
    mBullets.push_back(bullet_t( pos, dir ));
    // si graphique, on pete 1 son

    if (this->SupportsRendering())
    {
        Channel *gunShoot = Channel::AllocateChannel( Audio::GetSound("Datas/Sounds/fx/Shoot.ogg"), AUDIO_GROUP_GAME3D );

        if ( gunShoot != NULL )
        {
            gunShoot->SetPosition( pos );
            gunShoot->SetVolume(0.05f);
            gunShoot->Play();
        }

        PushFlashLight( pos + dir );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GameShip *Game::GetShipByNetIndex( int NetIdx ) const
{
    ASSERT_GAME( Game::IsValidObjectNetIndex(NetIdx) );

    std::vector<GameObject*>::const_iterator iter;
    for (iter = mGameObjectTyped[GO_Ship].begin(); iter != mGameObjectTyped[GO_Ship].end() ; ++iter)
    {
        GameShip * ps = (GameShip*)(*iter);
        const int ShipNetIndex = ps->GetOwnerNetIndex();

        if ( ShipNetIndex == NetIdx )
            return ps;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void HUD::Draw(Game *pGame, GameShip *pShip, int iSplitScreen)
{
    UNUSED_PARAMETER(pGame);

	int offsetX = (iSplitScreen?( (iSplitScreen-1)*32):15);

    if ( pShip->mProps.mAutoPilot && (!pShip->mProps.mRank) )
    {
        float aAutopilotRemaining = GameSettings.AutoPilotTime - pShip->GetBonusLocalTime();
        aAutopilotRemaining = (aAutopilotRemaining<0.f)?0.f:aAutopilotRemaining;
        aAutopilotRemaining *= 10.f;
        int aAutopilotRemainingInt = (int)aAutopilotRemaining;

        char tmps[512];
        sprintf(tmps, "// AUTOPILOT %d.%d\\\\", aAutopilotRemainingInt/10, aAutopilotRemainingInt%10);
        ggui.putText( offsetX+8, 18, tmps );
    }
    else
    {
        ggui.putText( offsetX+8, 18, "                   " );
    }

	// split
	{
		if (pShip->mProps.mRank && iSplitScreen)
		{
			if (iSplitScreen == 1)
				ggui.clearBitmaps();

			static const char *posEndintTexs[]= {TEXTURE_POS1,TEXTURE_POS2,TEXTURE_POS3,TEXTURE_POS4, TEXTURE_POS5, TEXTURE_POS6,TEXTURE_POS7,TEXTURE_POS8};
			ggui.addBitmap(gui::guiBitmap(vec ( (float)((iSplitScreen-1)*32 + 8), 16.f, 20.f,16.f), textures[ posEndintTexs[pShip->mProps.mRank-1] ] ) );
		}
	}
    // wrong way
	int wrongWay = pShip->GetPhysics()->IsWrongWay();

	


	if (!iSplitScreen)
	{
		ggui.putText(0, 17, "                                                                                                              ");
		ggui.putText(0, 15, "                                                                                                              ");
		ggui.putText(0, 19, "                                                                                                              ");

		if ( wrongWay >= 1 )
			ggui.putText(0, 17, "//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//");
		if ( wrongWay >= 2 )
			ggui.putText(0, 15, "ong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//");
		if ( wrongWay >= 3 )
			ggui.putText(0, 19, "Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//Wrong Way//");
	}
	else
	{
		ggui.putText(offsetX, 17,       "                                ");
		ggui.putText(offsetX, 15,       "                                ");
		ggui.putText(offsetX, 19,       "                                ");

		if ( wrongWay >= 1 )
			ggui.putText(offsetX, 17,   "//Wrong Way//Wrong Way//Wrong Way");
		if ( wrongWay >= 2 )
			ggui.putText(offsetX, 15,   "ong Way//Wrong Way//Wrong Way//Wr");
		if ( wrongWay >= 3 )
			ggui.putText(offsetX, 19,   "Way//Wrong Way//Wrong Way//Wrong ");
	}

	

    ggui.putText(offsetX+8, 34, "                                ");
    ggui.putText(offsetX+3, 33, "                                ");



    if ( GGame->GetType() == 8 && pShip->GetSmoothedSpeed() < GGame->GetEnduroSpeedThreshold() && pShip->mProps.mGotOverEnduroSpeedThresold ) // enduro
    {
        char tmps[512];
        sprintf(tmps,        "eliminated in %3.2f seconds", pShip->mProps.mHealth * GGame->GetEnduroTimeAvailable() );
		ggui.putText(offsetX+8, 34, ".speed below limit");
		ggui.putText(offsetX+3, 33, tmps );
    }
    // eliminator
    if ( GGame->GetType() == 1 )
    {
		if ( pGame->GetState() == GameStatePlaying )
		{
			if ( GGame->GetRacingTime() < 60.f )
			{
				// warming up
				char tmps[512];
				sprintf(tmps,        "%3.2f seconds before eliminations start", 60.f - GGame->GetRacingTime() );
				//ggui.putText(offsetX+8, 23, ".warming up");
				ggui.putText(offsetX+3, 34, tmps );
			}
			else
			{
				float nextEliminationTime = 30.f - fmodf( ( GGame->GetRacingTime() - 60.f ), 30.f );
				char tmps[512];
				if ( GGame->GetLastRankedOnLapsCount() == pShip )
				{
					sprintf(tmps,        "%3.2f seconds before your elimination", nextEliminationTime );
					ggui.putText(offsetX+8, 34, ".YOU ARE LAST");
					ggui.putText(offsetX+3, 33, tmps );
				}
				else
				{
					sprintf(tmps,        "%3.2f seconds before next one", nextEliminationTime );
					ggui.putText(offsetX+8, 34, ".next elimination");
					ggui.putText(offsetX+3, 33, tmps );
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// bonus : eviter les zones holein/out apres le hole
// 2 bonus speed dans le dernier holeintout avant le hole
// gestion de la fourche

void Game::GenerateBonuses(const bonusGenerationParameters_t& params, track_t & tr, bool bSpeed, bool bWeapon, bool bColors )
{
    //bonus
    trackBonusInstance_t::ClearBonuses();

    g_seed = 1234;


    //bool bInsideVirage = false;
    int virageLength = 0;
    int straightLength = 0;
    vec_t mEntreeVirageDir;


    int nbAIPoints = track.getAIPointCount();
    float distPerAIPoint = tr.trackLength / (float)(nbAIPoints);

    for (int k = 0 ; k < 2 ; k++)
    {
        bool bFourche = ( k == 1);

        float lastTimeWithoutSpeedBonus = params.bonusTimeBeforeSpeed;
        float lastTimeWithoutBonus = params.bonusTimeBeforeWeapon;
        for (int i = 14 ; i < nbAIPoints ; i++)
        {
            const AIPoint_t& aipoint = track.getAIPoint(i);
            const AIPoint_t& aipointp1 = track.getAIPoint((i+1)%nbAIPoints);
            //const AIPoint_t& aipointp2 = track.getAIPoint((i+2)%nbAIPoints);
            //const AIPoint_t& aipointp3 = track.getAIPoint((i+3)%nbAIPoints);
            const AIPoint_t& aipointp4 = track.getAIPoint((i+3)%nbAIPoints);
            //const AIPoint_t& aipointp5 = track.getAIPoint((i+5)%nbAIPoints);



            if ( (!aipoint.hasFourche) && bFourche)
                continue;

            if ( aipoint.bFourcheEnding || aipoint.bFourcheStarting || aipoint.bInsideHole || aipointp1.bInsideHole )
                continue;


            bool bSpeedBonusBeforeJump = aipointp4.bInsideHole;

            matrix_t mati, matip1;
            float matiWidth, matip1Width;

            track.getAIPoint(i).BuildMatrix( mati, bFourche );
            track.getAIPoint( (i+1)%nbAIPoints ).BuildMatrix( matip1, bFourche );
            matiWidth = track.getAIPoint(i).mAIwidth[k];
            matip1Width = track.getAIPoint((i+1)%nbAIPoints).mAIwidth[k];

            bool bStraight = false;
            //int idx1 = (i+1)%(NBSEGS*4);

            float weaponStrength = 0.f;
            float strg = matip1.dir.dot(mati.dir);
            bStraight = (strg > 0.999f);
            if (bStraight || bSpeedBonusBeforeJump)
            {
                //glColor4f(1.f, 1.f, 1.f, 1.f);
                float virageStrength = 0.f;

                // speed
                lastTimeWithoutSpeedBonus -= r01();
                if (lastTimeWithoutSpeedBonus< 0.f)
                {
                    virageStrength = r01() * params.bonusSpeedLocalDensity;
                    lastTimeWithoutSpeedBonus = params.bonusTimeBeforeSpeed;
                }
                // std bonus
                lastTimeWithoutBonus -= r01();
                if (lastTimeWithoutBonus< 0.f)
                {
                    weaponStrength = r01() * params.bonusWeaponLocalDensity;
                    lastTimeWithoutBonus = params.bonusTimeBeforeWeapon;
                }
                // sortie de virage

                if (virageLength)
                {
                    float raide = mEntreeVirageDir.dot( mati.dir );
                    virageStrength = (raide < params.bonusVirageRaide)?2.f:1.f;
                    virageStrength *= (virageLength >= params.bonusVirageLength)?2:1;
                }
                //virageStrength = 1.f;

                // full track speed before jump
                if (bSpeedBonusBeforeJump)
                {
                    virageStrength = 2.f;
                    weaponStrength = 0.f;
                }

                if ( ( virageStrength + weaponStrength)>0.f)
                {
                    // minimale curve on right or left
                    vec_t diff = matip1.position - mati.position;
                    diff.normalize();
                    float dtmc = mati.right.dot(diff);

                    float sides[2];
                    //float roadWidth = 8.f;
                    if ( dtmc > 0.f )
                    {
                        sides[0] = 1;
                        sides[1] = -1;
                    }
                    else
                    {
                        sides[0] = -1;
                        sides[1] = 1;
                    }

                    float currentDistanceOnTrack = distPerAIPoint * (float)(i);
                    float nearestHoleDist, nearestHoleLength;
                    nearestHoleDist = tr.findNearestHole( currentDistanceOnTrack, nearestHoleLength );

                    float ecartementFactor = 1.f;
                    if ( GGame && nearestHoleDist<0.f)
                        ecartementFactor = (nearestHoleDist * GameSettings.BonusPreHoleEcartementFactor + GameSettings.BonusPreHoleEcartementMinimum );// 0.01f + 0.3f);
                    ecartementFactor = Clamp( ecartementFactor, 0.f, 1.f);

                    int bonusIterate = (int)(virageStrength+weaponStrength);
                    for (int trBn = 0; trBn < bonusIterate; trBn++)
                    {

                        matrix_t bonusMat;
                        bonusMat = (trBn>1)?matip1:mati;
                        float trackWidth = (trBn>1)?matip1Width:matiWidth;

                        bonusMat.position += bonusMat.right * sides[trBn&1] * trackWidth * GameSettings.BonusPositionTrackWidthFactor * ecartementFactor;



                        if ( trBn < virageStrength)
                        {
                            if ( bSpeed )
                            {
                                /*mesh_t *bnMesh = */trackBonusInstance_t::PushTrackBonus( bonusMat, BT_Trackspeed, i );
                                /*
                                if  (bAddToProgress && bnMesh)
                                track.pushProgress( currentDistanceOnTrack, bnMesh, bnMesh->mWorldMatrix );
                                */
                            }
                        }
                        else
                        {
                            if ( bWeapon )
                            {
                                static const Bonus_Type bty[]= {	BT_Missile,	BT_Mines,	BT_Autopilot,	BT_Shield,	BT_MachineGun,	BT_SpeedBooster,	BT_3Missiles,	BT_HomingMissile,	BT_Portal,	/*BT_AntiPortal,*/	BT_Bombing,	BT_MagneticWave,	BT_PlasmaWave,	BT_FireWave, };
                                /*mesh_t *bnMesh = */trackBonusInstance_t::PushTrackBonus( bonusMat, bty[fastrand()%(sizeof(bty)/sizeof(Bonus_Type))], i );
                                /*
                                if  (bAddToProgress && bnMesh)
                                track.pushProgress( currentDistanceOnTrack, bnMesh, bnMesh->mWorldMatrix );
                                */
                            }
                            if ( bColors)
                            {
                                static const Bonus_Type bty2[] = { BT_Color1, BT_Color2 };
                                trackBonusInstance_t::PushTrackBonus( bonusMat, bty2[fastrand()%(sizeof(bty2)/sizeof(Bonus_Type))], i );
                            }
                        }
                    }


                }
                virageLength = 0;
                straightLength++;



            }
            else
            {
                lastTimeWithoutSpeedBonus = params.bonusTimeBeforeSpeed;
                lastTimeWithoutBonus = params.bonusTimeBeforeWeapon;

                //bInsideVirage = true;
                if (!virageLength)
                    mEntreeVirageDir = mati.dir;
                virageLength++;
            }
        }
        /*
        lastTimeWithoutSpeedBonus = params.bonusTimeBeforeSpeed;
        lastTimeWithoutBonus = params.bonusTimeBeforeWeapon;
        */
    }
}

int Game::sortByBrickIndex(const void *p1, const void *p2)
{
    const obstacle_t *r1 = (const obstacle_t*)p1;
    const obstacle_t *r2 = (const obstacle_t*)p2;

    if ( r2->brickindex < r1->brickindex ) return 1;
    if ( r2->brickindex > r1->brickindex ) return -1;
    return 0;
}

GameShip  *GameShip::GetClosest( float aMaxDistance )
{
    float closestDistance = 9999.f;
    GameShip *closestShip = NULL;
    float maxDistSq = aMaxDistance * aMaxDistance;
    const vec_t& shpPos = GetPhysics()->GetTransform().position;

    for (unsigned int i=0;i<GGame->GetShipCount();i++)
    {
        GameShip *shp = GGame->GetShip(i);
        if ( shp == this )
            continue;

        const vec_t& otherPos = shp->GetPhysics()->GetTransform().position;
        float dist = (otherPos - shpPos).lengthSq();
        if (dist <= closestDistance && dist < maxDistSq )
        {
            closestShip = shp;
            closestDistance = dist;
        }
    }

    return closestShip;
}

void GameShip::FastForward( int BrickCount, float portalOffset )
{
	int idx = GetPhysics()->GetCurrentRoadBlockIndex();

//	const AIPoint_t& pt1 = track.getAIPoint( idx );
	const AIPoint_t& pt2 = track.getAIPoint( idx + BrickCount );

//	vec_t dif = pt2.mAIpos[0] - pt1.mAIpos[0];
	float lrFactor = GetPhysics()->GetLeftRightFactor();
	float shpOffset = -0.5f+(lrFactor-0.5f) * pt2.mAIwidth[0] * 2.f; 


	matrix_t newMat;
	pt2.BuildMatrix( newMat, 0 );
	newMat.position += (newMat.right * (portalOffset + portalWidth*0.25f)) + (newMat.up * 2.f);
	newMat.position.w = 1.f;

	GetPhysics()->FastFowardTo( newMat );
}

void GameShip::ApplyYYAttack()
{
    if (mProps.mHealth <= 0.5f )
        return;

    // get ship around
    float amount = mProps.mHealth - 0.5f;

    GameShip *shipClose = GetClosest( 60.f );
    if ( shipClose )
    {
        shipClose->mProps.mHealth -= amount;
        mProps.mHealth = 0.5f;
    }
}

int GameShip::findBestCurveStartIndex( const vec_t& v)
{
    float glen = 999999.f;
    int bestIdx = 0;
    for (int k=0;k<2;k++)
    {
        for (int i=0;i<NBSEGS*4;i++)
        {
            vec_t df = track.minimalCurve[k][i].position - v;
            float l = df.lengthSq();
            if (l< glen)
            {
                glen = l;
                bestIdx = i;
            }
        }
    }

    return bestIdx;
}


void Game::BuildObstacleList()
{
    if (mNbGlobalObstacles > 0xFF)
        return;


    PROFILER_START( BuildObstacleList );

    mNbGlobalObstacles = 0;
    std::vector<GameObject*>::iterator itero;
    for (itero = mGameObjectTyped[GO_Mine].begin(); itero != mGameObjectTyped[GO_Mine].end() ; ++itero)
    {
        if (mNbGlobalObstacles > 0xFF)
            break;
        GameMine *mine = (GameMine*)(*itero);

        mObstacles[mNbGlobalObstacles].mat = mine->GetMatrix();
        mObstacles[mNbGlobalObstacles].radius = GameSettings.mMineRadius + GameSettings.ShipRadius;
        mObstacles[mNbGlobalObstacles].brickindex = mine->mBrickIndex;
        mObstacles[mNbGlobalObstacles].relatedObject = mine;

        mNbGlobalObstacles ++;
    }

    // have to remove relatedobject check in compute AI
    for (itero = mGameObjectTyped[GO_Ship].begin(); itero != mGameObjectTyped[GO_Ship].end() ; ++itero)
    {
        if (mNbGlobalObstacles > 0xFF)
            break;

        GameShip *ship = (GameShip*)(*itero);

        mObstacles[mNbGlobalObstacles].mat = ship->GetPhysics()->GetTransform();
        mObstacles[mNbGlobalObstacles].radius = GameSettings.ShipRadius;
        mObstacles[mNbGlobalObstacles].brickindex = ship->GetPhysics()->GetCurrentRoadBlockIndex();
        mObstacles[mNbGlobalObstacles].relatedObject = ship;

        mNbGlobalObstacles ++;
    }

    // sort by brick index

    qsort(mObstacles, mNbGlobalObstacles, sizeof(obstacle_t), sortByBrickIndex);

    PROFILER_END(); // BuildObstacleList
}

int Game::GetFirstObstacleIndexForBrickIndex( unsigned int aBrickIdx ) const
{
    if (!mNbGlobalObstacles)
        return -1;

    for (int i=0;i<mNbGlobalObstacles;i++)
    {
        if ( mObstacles[i].brickindex == aBrickIdx )
            return i;

        if ( ( mObstacles[i].brickindex <= aBrickIdx ) &&
            ( mObstacles[(i+1)%0xFF].brickindex >= aBrickIdx ) )
            return i;
    }
    return -1;
}

mesh_t* createBoxMarker(const vec_t& pos, const vec_t& col, float sz);

shipControls_t GameShip::ComputeAI( float aTimeEllapsed )
{
    shipControls_t resCtrl;

    resCtrl.reset();


    IPhysicShip *pShipPhysics = GetPhysics();



    const matrix_t& shpMat = pShipPhysics->GetTransform();
    const vec_t& shpPos = shpMat.position;
    const vec_t& shpDir = shpMat.dir;
    //const vec_t& shpRight = shpMat.dir;


    if (pShipPhysics->HasBeenReseted())
        mCurrentWayPoint = -1;

    if ( mCurrentWayPoint == -1)
    {
        mCurrentWayPoint = findBestCurveStartIndex( pShipPhysics->GetTransform().position );
        mShipPID.previouserror = 0.f;
        mShipPID.error = 0.f;
        mShipPID.I = 0.f;
    }


    resCtrl.mRun = 0xFF;

    //disabling warning C4127: conditional expression is constant
    #pragma warning(push)
    #pragma warning(disable:4127)
    while (1)
    #pragma warning(pop)
    {
        // look for attempting next way point
        matrix_t wpMat = track.minimalCurve[0][(mCurrentWayPoint)%(NBSEGS*4)];
        vec_t vp = shpPos - wpMat.position;
        vp.normalize();
        float passedBy = vp.dot(wpMat.dir);
        if (passedBy > 0.f)
        {
            mCurrentWayPoint++;
        }
        else
        {
            wpMat = track.minimalCurve[0][(mCurrentWayPoint+1)%(NBSEGS*4)];

            // attractive bonus

            static const int bonusWindowLength = 8;
            static const float bonusMaxDot = 0.1f;

            static trackBonusInstance_t* attractiveBonus = NULL;

            if (attractiveBonus)
            {
                int difBrique = attractiveBonus->trackBrickIndex - pShipPhysics->GetCurrentRoadBlockIndex();
                difBrique = (difBrique+track.getAIPointCount())%track.getAIPointCount();
                if ( (difBrique >= bonusWindowLength) || (difBrique <1))/// PARAM VALUE
                    attractiveBonus = NULL;
            }

            if (!attractiveBonus)
            {


                trackBonusInstance_t *bestAttractiveBonus = NULL;
                trackBonusInstance_t *tbi = trackBonusInstance_t::GetFirstBonusInstanceForBrickIndex(pShipPhysics->GetCurrentRoadBlockIndex());
                if (tbi)
                {
                    float bestDot = 1.f;

                    do
                    {

                        int difBrique = tbi->trackBrickIndex - pShipPhysics->GetCurrentRoadBlockIndex();
                        difBrique = (difBrique+track.getAIPointCount())%track.getAIPointCount();


                        if (tbi->IsActive())
                        {
                            vec_t bonusToShip = tbi->mWorldMatrix.position - shpPos;
                            bonusToShip.normalize();
                            float dotr = fabsf(shpMat.right.dot( bonusToShip ));

                            float dotDir = normalized(tbi->mWorldMatrix.dir).dot(shpDir);
                            //qDebug() << "ddir " << dotDir;
                            if ( (dotr <= bestDot )  && ( dotr < bonusMaxDot ) && ( difBrique > 1) &&
                                (dotDir>0.99f)
                                ) /// PARAM VALUE
                            {
                                bestAttractiveBonus = tbi;
                                bestDot = dotr;
                            }

                        }

                        if ( (difBrique >= bonusWindowLength) /*|| (difBrique <1)*/ ) /// PARAM VALUE
                            break;


                        tbi = tbi->mNextInstance;
                    }
                    //disabling warning C4127: conditional expression is constant
                    #pragma warning(push)
                    #pragma warning(disable:4127)
                    while(1);
                    #pragma warning(pop)
                }
                if (bestAttractiveBonus)
                    attractiveBonus = bestAttractiveBonus;
            }


            if (attractiveBonus)
            {
                wpMat = attractiveBonus->mWorldMatrix;

                float distToBonus = (wpMat.position - shpPos).length();
                //qDebug() << "DistTB = " << distToBonus;
                static const float distForeBonus = 80.f;
                if (distToBonus < distForeBonus)
                {
                    wpMat.position +=  normalized(wpMat.dir) * (distForeBonus - distToBonus);
                    wpMat.position.w = 1.f;
                }
            }


            // carotte

            matrix_t carotte;
            vec_t carotteDir = wpMat.position - shpPos;
            carotteDir.normalize();
            carotte.LookAt( vec(0.f), carotteDir, wpMat.up);//vec(0.f, 1.f, 0.f));
            carotte.position = shpPos+carotteDir * 30.f;
            carotte.position.w = 1.f;




            // collision carotte/obstacles


            //float lowestPlan=99999.f, highestPlan=-99999.f;
            /*
            const obstacle_t *obstaclesForShip[32];


            int nbObstacles = findObstaclesFront( pShipPhysics->maCurIdx, 16, 32, obstaclesForShip );
            */
            const Game::obstacle_t *bestObstacles = NULL;
            const int shipBrickIdx = pShipPhysics->GetCurrentRoadBlockIndex();
            int obstaclesIndex = GGame->GetFirstObstacleIndexForBrickIndex( shipBrickIdx );
            if ( 0 ) //obstaclesIndex != -1) // STUNFEST
            {
                // init plan/ vectors


                carotteDir = carotte.position - shpPos;
                carotteDir.normalize();

                vec_t carotteRight;
                carotteRight.cross( carotteDir, vec(0.f, 1.f, 0.f) );

                vec_t avoidPlan;
                avoidPlan = buildPlan(shpPos, normalized(vec(carotteRight.x, 0.f, carotteRight.z, 0.f)));
                const int nbAiPt = track.getAIPointCount();

                // find best obstacle
                for (int nbo = obstaclesIndex;nbo<obstaclesIndex+MAXNBOBSTACLES;nbo++)
                {
                    const Game::obstacle_t * obs = &GGame->GetObstacleAtIndex( nbo );
                    if (obs->relatedObject == this)
                        continue;

                    // obstacle close enough to check it? otherwise, leave
                    int difBrique = obs->brickindex - shipBrickIdx;
                    difBrique = ( difBrique + nbAiPt )%nbAiPt;
                    if ( ( difBrique > 16 ) && ( difBrique < (nbAiPt>>1) ) )
                        break;

                    if ( difBrique > (nbAiPt>>1) )
                        continue;

                    // plan check
                    float obstacleRadius = obs->radius;
                    float signedDist = avoidPlan.signedDistanceTo(obs->mat.position);

                    /*
                    vec_t obstacleShipPos = obs->mat.position-shpPos;
                    float obstacleShipPosLen = obstacleShipPos.length();
                    */
                    //obstacleShipPos *= 1.f/obstacleShipPosLen;

                    //float dotShpDirObsShip = shpDir.dot(obstacleShipPos);

                    //float distanceToShip = distance( shpPos, obs->mat.position);
                    // obstacle in range
                    if ( (fabsf(signedDist) < obstacleRadius) ) // this check : once inside obstacles, don't care.
                    {
                        /*highestPlan = zmax(highestPlan, signedDist + obstacleRadius);
                        lowestPlan = zmin(lowestPlan, signedDist - obstacleRadius);
                        */

                        bestObstacles = obs;
                        break;

                    }
                }
                // escape obstacle
                if ( bestObstacles )
                {

                    vec_t dif = bestObstacles->mat.position- shpPos;
                    dif.normalize();
                    float dtdif = shpMat.right.dot( dif );
                    if ( dtdif < 0.f )
                        carotte.position += shpMat.right*4.f;
                    else
                        carotte.position -= shpMat.right*4.f;

                }
            }


            // -- side plans collisions

            float borderAvoidSide = 0.f;
#define MAXBRICKSFORSIDEPLANS 4

            vec_t obsAvoidShip = shpPos;
            obsAvoidShip.w = 0.f;
            vec_t obsAvoidDir = shpDir;
            obsAvoidDir.w = 0.f;

            for (int i = 0 ; i<MAXBRICKSFORSIDEPLANS ; i++)
            {
                const AIPoint_t& aipt = track.getAIPoint( pShipPhysics->GetCurrentRoadBlockIndex() + i );
                vec_t plans[2] = {
                    buildPlan( aipt.mAIPos[0] + aipt.mAIRight[0] * aipt.mAIwidth[0], aipt.mAIRight[0] ),
                    buildPlan( aipt.mAIPos[0] - aipt.mAIRight[0] * aipt.mAIwidth[0], aipt.mAIRight[0] )
                };

                for (int j=0;j<2;j++)
                {

                    float lenInters = IntersectRayPlane( obsAvoidShip, obsAvoidDir, plans[j] );
                    if ( ( lenInters > 0.0f ) && ( lenInters <= 40.f) )
                    {
                        borderAvoidSide = obsAvoidDir.dot(plans[j]);
                    }
                }

            }

            // -- side plans collisions



            // -- merging results
            carotte.position.w = 1.f;

            // debug draw
            /*
            static mesh_t *aimarker = NULL;
            if ( !aimarker )
            {
            for (int lp = 0;lp<(NBSEGS*4);lp++)
            {
            const matrix_t& omop = tr.minimalCurve[0][lp];
            createBoxMarker( omop.position, vec(1.f, 0.f, 0.f, 1.f), 2.f );
            }


            aimarker = createBoxMarker( carotte.position, vec(1.f, 0.f, 0.f, 1.f), 2.f );
            }

            aimarker->mWorldMatrix.translation(carotte.position);
            aimarker->updateWorldBSphere();
            */

            // debug draw

            // turning direction
            vp = carotte.position - shpPos;

            float turnStrength = shpMat.right.dot( vp );

			mShipPID.SetIPD( 0.50f, 6.6f, 0.125f );
            float output = mShipPID.Compute( 0.f, turnStrength, aTimeEllapsed);


            turnStrength = fabsf(turnStrength);

            float difMv = output - turnStrength;
            if ( fabsf(difMv) < 1.5f )
                difMv = 0.f;


			// STUNFEST commented
            //difMv += borderAvoidSide * 10.f; // border avoidance is 10times prioritized!

            resCtrl.mLeft = ((difMv>FLOAT_EPSILON)?230:0);
            resCtrl.mRight = ((difMv<-FLOAT_EPSILON)?230:0);

            break;
        }
    }
    return resCtrl;
}

