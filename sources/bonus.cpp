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
#include "bonus.h"
#include "game.h"
#include "mesh.h"
#include "ZShipPhysics.h"
#include "content.h"
#include "camera.h"
#include "audio.h"
#include "track.h"

#include <float.h>  //  required for FLT_MAX on MacOSX at least

///////////////////////////////////////////////////////////////////////////////////////////////////

static const float MinimalMissileDistance = 4.f;

typedef struct trackBonusType_t
{
    unsigned int activeStateSprite;
    unsigned int inactiveStateSprite;
    bool autoUse; // true pour bonus speed & ying yang, false to acquier it
    Bonus_Type mType;  // <- useless/ use index
    //const char *szSoundName;
    //NOTE: removing mSoundHash 'const' attribute to get rid of:
    // warning C4510: 'trackBonusType_t' : default constructor could not be generated
    // warning C4512: 'trackBonusType_t' : assignment operator could not be generated
    // the other solution would be to implement both
    uint32 mSoundHash;
}trackBonusType_t;

enum { TrackBonusTypeCount = 18 };

const trackBonusType_t TrackBonusTypes[TrackBonusTypeCount]= {
    { 1, 0, true, BT_Trackspeed, NULL },
    { 3, 2, true, BT_Color1, NULL },
    { 4, 2, true, BT_Color2, NULL },
    { 3, 2, false, BT_Missile, hashFast("Datas/Sounds/droid/Missile.ogg") },
    { 3, 2, false, BT_Mines, hashFast("Datas/Sounds/droid/Mines.ogg") },
    { 3, 2, false, BT_Autopilot, hashFast("Datas/Sounds/droid/autopilot.ogg") },
    { 3, 2, false, BT_Shield, hashFast("Datas/Sounds/droid/Shield.ogg") },
    { 3, 2, false, BT_MachineGun, hashFast("Datas/Sounds/droid/MachineGun.ogg") },
	{ 3, 2, false, BT_SpeedBooster, hashFast("Datas/Sounds/droid/SpeedBooster.ogg") },
	{ 3, 2, false, BT_3Missiles, hashFast("Datas/Sounds/droid/3Missiles.ogg") },
    { 3, 2, false, BT_HomingMissile, hashFast("Datas/Sounds/droid/HomingMissile.ogg") },
	{ 3, 2, false, BT_Portal, hashFast("Datas/Sounds/droid/Portal.ogg") },
	//{ 3, 2, false, BT_AntiPortal, hashFast("Datas/Sounds/droid/AntiPortal.ogg") },
	{ 3, 2, false, BT_MagneticWave, hashFast("Datas/Sounds/droid/MagneticWave.ogg") },
	{ 3, 2, false, BT_PlasmaWave, hashFast("Datas/Sounds/droid/PlasmaWave.ogg") },
	{ 3, 2, false, BT_Bombing, hashFast("Datas/Sounds/droid/Shelling.ogg") },
	{ 3, 2, false, BT_FireWave, hashFast("Datas/Sounds/droid/FireWave.ogg") }
	
};

///////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<trackBonusInstance_t*> trackBonusInstance_t::mFirstTrackBonusForBrickIndex;
int trackBonusInstance_t::lastBrickIndex = 0;
std::vector<trackBonusInstance_t> trackBonusInstance_t::GTrackBonusInstances;
trackBonusInstance_t *trackBonusInstance_t::mLastInstance = NULL;
bool trackBonusInstance_t::bAllBonusActive = true;

///////////////////////////////////////////////////////////////////////////////////////////////////

trackBonusInstance_t::trackBonusInstance_t()
{
    Clear();
}

mesh_t* trackBonusInstance_t::Init( const matrix_t& mat, Bonus_Type aType, int brickIndex )
{
    ASSERT_GAME( 0 <= aType && aType < TrackBonusTypeCount );

    bonusSphere = mat.position;
    bonusSphere.w = GameSettings.BonusRadius * 2.f;//.3.f;
    mType = aType;
    inactivityTime = 0.f;

    if (lastBrickIndex != brickIndex)
    {
        mFirstTrackBonusForBrickIndex.resize( brickIndex+1 );

        for (int i = lastBrickIndex ; i <= brickIndex ; i++)
            mFirstTrackBonusForBrickIndex[i] = this;
        lastBrickIndex = brickIndex;
    }
    trackBrickIndex = brickIndex;

    ASSERT_GAME( mLastInstance == NULL || ( mFirstTrackBonusForBrickIndex.size() > 0 && mLastInstance->mNextInstance == mFirstTrackBonusForBrickIndex[0] ) );

    if ( mLastInstance == NULL )
        mLastInstance = this;

    mLastInstance->mNextInstance = this;
    mNextInstance = mFirstTrackBonusForBrickIndex[0];
    mLastInstance = this;

    static const float TrackBonusScale = 0.1f;
    matrix_t asc;
    asc.scale(TrackBonusScale, TrackBonusScale, TrackBonusScale);

    mWorldMatrix = asc * mat ;

    if (GGame && GGame->SupportsRendering())
    {
        // create new mesh
        activeStateMesh = getSprite( TrackBonusTypes[(int)aType].activeStateSprite )->clone();
        inactiveStateMesh = getSprite( TrackBonusTypes[(int)aType].inactiveStateSprite )->clone();

        activeStateMesh->mWorldMatrix = inactiveStateMesh->mWorldMatrix = mWorldMatrix;

        activeStateMesh->updateWorldBSphere();
        inactiveStateMesh->updateWorldBSphere();
        activeStateMesh->visible = true;
        inactiveStateMesh->visible = false;

        activeStateMesh->color = vec(1.f, 1.f, 1.f, 1.f);
        activeStateMesh->physic = false;
        inactiveStateMesh->color = vec(1.f, 1.f, 1.f, 1.f);
        inactiveStateMesh->physic = false;
    }

    return activeStateMesh;
}

trackBonusInstance_t::~trackBonusInstance_t()
{
    ASSERT_GAME( activeStateMesh == NULL );
    ASSERT_GAME( inactiveStateMesh == NULL );
    ASSERT_GAME( mNextInstance == NULL );
}

void trackBonusInstance_t::Clear()
{
    activeStateMesh = NULL;
    inactiveStateMesh = NULL;

    bonusSphere = vec_t::zero;
    inactivityTime = 0.f;
    mType = BT_None;
    trackBrickIndex = 0;
    mWorldMatrix.identity();

    mNextInstance = NULL;
}

void trackBonusInstance_t::GetActive()
{
    if (activeStateMesh && inactiveStateMesh)
    {
        activeStateMesh->visible = true;
        inactiveStateMesh->visible = false;
    }

}

void trackBonusInstance_t::GetInactive()
{
    inactivityTime = GameSettings.TrackBonusInactivityTime;
    if (activeStateMesh && inactiveStateMesh)
    {
        activeStateMesh->visible = false;
        inactiveStateMesh->visible = true;
    }
}



void trackBonusInstance_t::ClearBonuses()
{
    mLastInstance = NULL;
    lastBrickIndex = 0;
    mFirstTrackBonusForBrickIndex.clear();

    std::vector<trackBonusInstance_t>::iterator iter = GTrackBonusInstances.begin();
    for (; iter != GTrackBonusInstances.end(); ++ iter)
    {
        trackBonusInstance_t & tbi = (*iter);
        tbi.Clear();
    }

    GTrackBonusInstances.clear();
    GTrackBonusInstances.reserve( 1000 );
}

mesh_t* trackBonusInstance_t::PushTrackBonus(const matrix_t& mat, Bonus_Type aType, int brickIndex)
{
    mesh_t* bonusMesh = NULL;

    const u32 bonusCount = GTrackBonusInstances.size();
    const u32 bonusCountMax = GTrackBonusInstances.capacity();

    //HACK: preventing any vector memory reallocation, so that objects holding pointers to bonus memory don't have to be updated
    if ( bonusCount < bonusCountMax )
    {
        trackBonusInstance_t emptyBonus;
        GTrackBonusInstances.push_back(emptyBonus);
        ASSERT_GAME( GTrackBonusInstances.size() == bonusCount + 1 );

        const u32 bonusIdx = bonusCount;
        trackBonusInstance_t& tbi = GTrackBonusInstances[bonusIdx];
        bonusMesh = tbi.Init( mat, aType, brickIndex );
    }

    ASSERT_GAME( bonusMesh != NULL );

    return bonusMesh;
}

void trackBonusInstance_t::TickBonuses(float aTimeEllapsed)
{		
    int idx = 0;
    std::vector<trackBonusInstance_t>::iterator iter = GTrackBonusInstances.begin();
    for (; iter != GTrackBonusInstances.end(); ++ iter, idx++)
    {
        trackBonusInstance_t & tbi = (*iter);
        float newInacTime = tbi.inactivityTime - aTimeEllapsed;

        if (newInacTime <= 0.f && tbi.inactivityTime >FLOAT_EPSILON)
        {
            tbi.inactivityTime = newInacTime;

            tbi.GetActive();
            GGame->AddValidatedBonus(idx);
        }
        else
            tbi.inactivityTime = newInacTime;
    }
}

void trackBonusInstance_t::DetectHandleShipOnTrackBonus(const vec_t& shipOrigin, const vec_t& shipDir, GameShip *pShip)
{
    if (!bAllBonusActive)
        return;

    vec_t normDir = shipDir;
    float aDist = shipDir.length();
    if (aDist >= FLOAT_EPSILON)
        normDir *= (1.f / aDist);
    else
        return;

    // auto acquire bonus
    /*
   if (!pShip->GetAcquiredBonus())
   {
	   static float timeBeforeNewBonus = 0.f;
	   timeBeforeNewBonus -= 0.01f;
	   if (timeBeforeNewBonus <= 0.f)
	   {
		   timeBeforeNewBonus = 1.f;
		   Bonus *bn = Bonus::Instanciate( BT_Missile );
		   pShip->SetAcquiredBonus( bn, BT_Missile );
	   }
   }
  */ 
  
    // get every possible track bonus
    std::vector<trackBonusInstance_t>::iterator iter = GTrackBonusInstances.begin();
    std::vector<trackBonusInstance_t>::iterator iterEnd = GTrackBonusInstances.end();
    int idx = 0;
    for (; iter != iterEnd; ++ iter, idx ++)
    {
        trackBonusInstance_t& bonus = (*iter);
        if (bonus.inactivityTime <= 0.f)
        {
            float nDistRaySphere = IntersectRaySphere(shipOrigin, normDir, bonus.bonusSphere);
            float distPointPoint = (shipOrigin-bonus.bonusSphere).length();
            if ( ( (nDistRaySphere <= aDist) && (nDistRaySphere > 0.f) ) || (distPointPoint< bonus.bonusSphere.w) )
            {
                bool bCanPlaySounds = ( GGame->SupportsRendering() && ( GGame->GetPlayerShip() == pShip ) && ( camera.GetMode() == CM_BEHINDSHIP ) );

                ASSERT_GAME( 0 <= bonus.mType && bonus.mType < TrackBonusTypeCount );
                if (TrackBonusTypes[bonus.mType].autoUse)
                {
                    
                    switch (bonus.mType)
                    {
                    case BT_Trackspeed:			
                        {
                            vec_t bonusDirection = normalized(bonus.mWorldMatrix.dir) * GameSettings.TrackBonusSpeedBoost;
                            pShip->mProps.mBoost = bonusDirection;

                            // collector mode
                            if ( GGame->IsAuth() && GGame->GetType() == 7 )
                            {
                                pShip->mProps.mPoints += 20.f;
                            }
                            bonus.GetInactive();
                            GGame->AddInvalidatedBonus(idx);

                            // boost sound
                            if ( bCanPlaySounds )
                                Audio::PlaySound( Audio::GetSound( "Datas/Sounds/fx/trackSpeed.ogg" ), AUDIO_GROUP_GAME2D ) ;
                        }
                        break;
                    case BT_Color1:				
                        if (pShip->mProps.mCurrentColor == 1)
                        {
                            pShip->mProps.mHealth += GameSettings.ColorBonusAmount;
                            bonus.GetInactive();
                            GGame->AddInvalidatedBonus(idx);

                            if ( bCanPlaySounds )
                                Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/getcolor.ogg"), AUDIO_GROUP_GAME2D );
                        }
                        break;
                    case BT_Color2:	
                        if (pShip->mProps.mCurrentColor == 0)
                        {
                            pShip->mProps.mHealth += GameSettings.ColorBonusAmount;
                            bonus.GetInactive();
                            GGame->AddInvalidatedBonus(idx);

                            if ( bCanPlaySounds )
                                Audio::PlaySound( Audio::GetSound("Datas/Sounds/fx/getcolor.ogg"), AUDIO_GROUP_GAME2D );
                        }
                        break;
                    default:
                        break;
                    }
                    //Bonus *bn = Bonus::Instanciate(bonus.mType);
                    //bn->Apply(pShip);
                    //delete bn;
                }
                else
                {
                    // std bn
                    if (!pShip->GetAcquiredBonus())
                    {
                        bonus.GetInactive();
                        GGame->AddInvalidatedBonus(idx);
                        Bonus *bn = Bonus::Instanciate( (Bonus::BonusForced()==BT_BonusCount)?bonus.mType:Bonus::BonusForced() );
                        pShip->SetAcquiredBonus(bn, bonus.mType);
                        if ( bCanPlaySounds )
                        {
                            ASSERT_GAME( 0 <= bonus.mType && bonus.mType < TrackBonusTypeCount );

                            if ( TrackBonusTypes[bonus.mType].mSoundHash )
                            {
                                Audio::PlaySound( Audio::GetSound( TrackBonusTypes[bonus.mType].mSoundHash ), AUDIO_GROUP_GAME2D ) ;
                            }
                            /*
                            sf::Sound *psnd = TrackBonusTypes[bonus.mType].acquiredSound;
                            psnd->SetRelativeToListener(true);
                            psnd->SetPosition(0.f, 0.f, 0.f);
                            psnd->Play();
                             */
                        }
                    }
                }
            }
        }
    }
    // tag them as used
    // apply auto use bonus
    // assign the first to the ship

}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ForceBonusToShip(GameShip *pShip, int bonusNb)
{
    if ( bonusNb < 0 )
        return;
    Bonus_Type bnIndex = (Bonus_Type)( bonusNb+(int)BT_Missile);
    Bonus *bn = Bonus::Instanciate( bnIndex );
    pShip->SetAcquiredBonus( bn, bnIndex );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Bonus_Type Bonus::bonusForcedToBe = BT_BonusCount;
GameShip * Bonus::GetAnyShipTargetableBy( GameShip *pShip, float aMinimalDistance, float aMaximalDistance, float aMaxDot )
{
    const vec_t& shpPos = pShip->GetPhysics()->GetTransform().position;
    const vec_t& shpDir = pShip->GetPhysics()->GetTransform().dir;
    int nbShips = GGame->GetShipCount();
    for (int i=0;i<nbShips;i++)
    {
        GameShip * other = GGame->GetShip( i );
        if ( other == pShip )
            continue;

        vec_t dif = other->GetPhysics()->GetTransform().position - shpPos;
        float difLen = dif.length();

        if ( ( difLen < aMinimalDistance  ) || ( difLen > aMaximalDistance ) )
            continue;

        dif *= 1.f/difLen;

        if ( dif.dot( shpDir ) > aMaxDot ) //GameSettings.MissileDotForAI )
            return other;

    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusMissile : public Bonus
{
public:
    BonusMissile()
    {
        mAICurrentTime = 0.f;
        mbMissileFired = false;
    }
    virtual ~BonusMissile()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_Missile; }

    virtual void Apply(GameShip *pShip)
    {
        if (mbMissileFired)
            return; // safety
        
        matrix_t misMat = pShip->GetPhysics()->GetTransform();
        const matrix_t& shpGroundMat = pShip->GetMatrixForMissilesAndGun();
        misMat.position = shpGroundMat.position + shpGroundMat.dir * MinimalMissileDistance;

        GameMissile *pMis = (GameMissile *)GGame->Spawn(GO_Missile, misMat, pShip);
        pMis->SetBrickIndex( pShip->GetPhysics()->GetCurrentRoadBlockIndex() );
        pMis->SetDirection(pShip->GetPhysics()->GetTransform().dir);
        mbMissileFired = true;
    }
    virtual bool CanBeDestroyed() const { return mbMissileFired; }
    bool mbMissileFired;



    // AI
    float mAICurrentTime;
    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        mAICurrentTime += aTimeEllapsed;
        if (mAICurrentTime >= GameSettings.MissileTimeBeforeFiredAnyway )
            return true;

        return ( GetAnyShipTargetableBy( pShip, GameSettings.MissileMinimalTargetDistance, GameSettings.MissileDotForAI ) != NULL);
    }


};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusMines : public BonusTimedDrop
{
public:
    BonusMines() : BonusTimedDrop( GameSettings.NumberOfMinesToDrop, GameSettings.TimeBetwen2MinesDrop) {}
    virtual ~BonusMines()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_Mines; }

    virtual void DoDrop(GameShip *pShip)
    {
        ASSERT_GAME( mShip != NULL );

        const vec_t& shpDir = mShip->GetPhysics()->GetTransform().dir;//
        matrix_t mineMat = mShip->GetPhysics()->GetGroundMatrix();
        //mineMat.position -= mineMat.up * 9.f;
        mineMat.position -= shpDir * (GameSettings.ShipRadius + GameSettings.mMineRadius * 3.f );
        mineMat.position.w = 1.f;
        mineMat.orthoNormalize();
        GameMine *mine = (GameMine*)GGame->Spawn(GO_Mine, mineMat, pShip );
        //FIXME: the owner net index should have already been set in Spawn()?
        //mine->mOwner = pShip->mProps.mNetIndex;
        mine->mBrickIndex = pShip->GetPhysics()->GetCurrentRoadBlockIndex();

    }
    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
        // timed properties-> direct apply
        return true;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusTimedProperty : public Bonus
{
public:  
    BonusTimedProperty(float aTime)
    {
        mCurrentTime = 0.f;
        mShip = NULL;
        mbApplied = false;
        mPropTimeEffect = aTime;
    }

    virtual void SetProp(bool bEnable) { UNUSED_PARAMETER(bEnable); }
    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);

        if (mbApplied)
        {
            mCurrentTime += aTimeEllapsed;
            if (mCurrentTime >= mPropTimeEffect)
            {
                mbApplied = false;
                this->SetProp( false );
            }
        }
    }
    virtual void Apply(GameShip *pShip)
    {
        if (!mbApplied)
        {
            mCurrentTime = 0.f;
            mbApplied = true;
            mShip = pShip;
            this->SetProp( true );
        }
    }
    virtual bool CanBeDestroyed() const {
        return (mCurrentTime >= mPropTimeEffect);
    }

    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
        // timed properties-> direct apply
        return true;
    }

    float mCurrentTime;
    bool mbApplied;
    float mPropTimeEffect;
    GameShip *mShip;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusAutopilot : public BonusTimedProperty
{
public:
    BonusAutopilot() : BonusTimedProperty(GameSettings.AutoPilotTime) { }
    virtual ~BonusAutopilot()
    {
        if ( mShip )
            SetProp( false );
    }

    virtual Bonus_Type GetBonusType() const { return BT_Autopilot; }

    virtual void SetProp(bool bEnable) { mShip->mProps.mAutoPilot = bEnable; }
    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
        // AI discard it
        mCurrentTime = 9999.f;
        return false;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusShield : public BonusTimedProperty
{
public:
    BonusShield() : BonusTimedProperty(GameSettings.ShieldTime) { }
    virtual ~BonusShield()
    {
        if ( mShip ) SetProp( false );
    }

    virtual Bonus_Type GetBonusType() const { return BT_Shield; }

    virtual void SetProp(bool bEnable) { mShip->mProps.mShield = bEnable; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusSpeedBooster : public BonusTimedProperty
{
public:
    BonusSpeedBooster() : BonusTimedProperty(GameSettings.SpeedBoosterTime) { }
    virtual ~BonusSpeedBooster()
    {
        if ( mShip )
            SetProp( false );
    }

    virtual Bonus_Type GetBonusType() const { return BT_SpeedBooster; }

    virtual void SetProp(bool bEnable)
    {
        UNUSED_PARAMETER(bEnable);
    }
    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        BonusTimedProperty::Tick(pShip, aTimeEllapsed);

        if (mbApplied)
            mShip->mProps.mBoost += normalized(pShip->GetPhysics()->GetTransform().dir) * GameSettings.SpeedBoosterIncPerSecond*aTimeEllapsed;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

void BonusTimedDrop::Tick(GameShip *pShip, float aTimeEllapsed) 
{
    ASSERT_GAME( mShip != NULL || mbDropping == false );

    if ( (mbDropping) && (mDropCount >0) )
    {
        mCurrentTime += aTimeEllapsed;
        const vec_t& shipPos = mShip->GetPhysics()->GetTransform().position;
        while ( (mCurrentTime >= mDropTime) && mDropCount &&
                ((shipPos - prevDropPos).length() >= mMinimalDistance ) )
        {
            prevDropPos = shipPos;
            this->DoDrop( pShip );
            mCurrentTime -= mDropTime;
            mDropCount--;
        }
    }
}

void BonusTimedDrop::Apply(GameShip *pShip) 
{
    ASSERT_GAME( pShip != NULL );

    mbDropping = true;
    mShip = pShip;
    prevDropPos = pShip->GetPhysics()->GetTransform().position;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

MachineGun::MachineGun() : BonusTimedDrop( GameSettings.MachineGunBullets, GameSettings.MachineGunTimeBetween2Bullets)
{
}

MachineGun::~MachineGun()
{
}

void MachineGun::Apply(GameShip *pShip)
{
    BonusTimedDrop::Apply(pShip);
    ASSERT_GAME( pShip == mShip );

    mShip->mProps.mShooting = true;
}

void MachineGun::DoDrop(GameShip *pShip) 
{
    UNUSED_PARAMETER(pShip);

	ASSERT_GAME( mShip != NULL );

    const matrix_t& mt = mShip->GetPhysics()->GetTransform();
    vec_t pos = mt.position + mt.right * ((mDropCount&1)?-0.2f:0.2f);

	GameShip * otherShip = GetAnyShipTargetableBy( pShip, 1.f, 300.f, 0.8f );//GameSettings.MissileDotForAI );

	vec_t bulletDir( mt.dir );

	if (otherShip)
		bulletDir = normalized(otherShip->GetPhysics()->GetTransform().position - mt.position );
	
    GGame->AddBullet(pos, bulletDir );
}  

void MachineGun::Tick(GameShip *pShip, float aTimeEllapsed) 
{
    BonusTimedDrop::Tick( pShip, aTimeEllapsed);

    ASSERT_GAME( mShip != NULL || mbDropping == false );

    //FIXME: should it be 'if ( mbDropping )' ?
    if ( mShip != NULL )
    {
        mbDropping = false; // need Use key down to apply and shoot
        mShip->mProps.mShooting = false;
    }
}

bool MachineGun::AIShouldUseIt( GameShip *pShip, float aTimeEllapsed ) 
{
    UNUSED_PARAMETER(aTimeEllapsed);

    // timed properties-> direct apply
    return ( GetAnyShipTargetableBy( pShip, 1.f, 9999.f, 0.85f ) != NULL);
}

void MachineGun::SetShellCount( unsigned int count )
{
    mDropCount = count; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
class BonusRepair : public Bonus
{
public:
    BonusRepair() { mbApplied = false; }
    virtual ~BonusRepair()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_Repair; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
    }
    virtual void Apply(GameShip *pShip)
    {
        if (!mbApplied)
        {
            pShip->mProps.mHealth += GameSettings.BonusRepairInc;
            pShip->mProps.mHealth = Clamp(pShip->mProps.mHealth, 0.f, 1.f);
        }
        mbApplied = true;
    }
    virtual bool CanBeDestroyed() const
    {
        return mbApplied;
    }
protected:
    bool mbApplied;
};
*/
///////////////////////////////////////////////////////////////////////////////////////////////////

class Bonus3Missiles : public Bonus
{
public:
    Bonus3Missiles() { mAICurrentTime = 0.f; mbMissilesFired = false; }
    virtual ~Bonus3Missiles()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_3Missiles; }

    virtual void Apply(GameShip *pShip)
    {
        if (mbMissilesFired)
            return; // safety
        matrix_t rotMis1, rotMis2, matmis;
        rotMis1.rotationY(GameSettings.BonusMissileSpread);
        rotMis2.rotationY(-GameSettings.BonusMissileSpread);
        const matrix_t& mt = pShip->GetMatrixForMissilesAndGun();
        
        matmis = mt;
        matmis.position += matmis.dir * MinimalMissileDistance;
        GameMissile *pMis = (GameMissile*)GGame->Spawn( GO_Missile, matmis, pShip );
        pMis->SetBrickIndex( pShip->GetPhysics()->GetCurrentRoadBlockIndex() );

        matmis = rotMis1 * mt;
        matmis.orthoNormalize();
        matmis.position += matmis.dir * MinimalMissileDistance;
        pMis = (GameMissile*)GGame->Spawn( GO_Missile, matmis, pShip );
        pMis->SetBrickIndex( pShip->GetPhysics()->GetCurrentRoadBlockIndex() );

        matmis = rotMis2 * mt;
        matmis.orthoNormalize();
        matmis.position += matmis.dir * MinimalMissileDistance;
        pMis = (GameMissile*)GGame->Spawn( GO_Missile, matmis, pShip );
        pMis->SetBrickIndex( pShip->GetPhysics()->GetCurrentRoadBlockIndex() );
        /*
        pMisMid->SetDirection(mt.dir);
        pMisRight->SetDirection(mt.dir + right);
        pMisLeft->SetDirection(mt.dir - right);
        */
        mbMissilesFired = true;
    }
    virtual bool CanBeDestroyed() const { return mbMissilesFired; }
    bool mbMissilesFired;

    // AI
    float mAICurrentTime;
    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        mAICurrentTime += aTimeEllapsed;
        if (mAICurrentTime >= GameSettings.MissileTimeBeforeFiredAnyway )
            return true;

        return ( GetAnyShipTargetableBy( pShip, GameSettings.MissileMinimalTargetDistance ) != NULL);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusHomingMissile : public Bonus
{
public:
    BonusHomingMissile() : Bonus(), mbMissileLaunched(false)
    {
        mLockingProgress = GameSettings.BonusHomingTimeBeforeArmed;
        mPreviouslyLocking = NULL;
        mAICurrentTime = 0.f;
    }
    virtual ~BonusHomingMissile()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_HomingMissile; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        if (mbMissileLaunched)
            return;

        matrix_t proj;
		extern float WIDTH, HEIGHT;
        proj.glhPerspectivef2(90.f, WIDTH/HEIGHT, 0.05f, 1000.f);

        const matrix_t& shipWorlTransform = pShip->GetPhysics()->GetTransform();
        matrix_t shipViewMatrix, shipViewProj;
        shipViewMatrix.lookAtRH( shipWorlTransform.position, shipWorlTransform.position + shipWorlTransform.dir, shipWorlTransform.up);

        shipViewProj = shipViewMatrix * proj;
        GameShip *lockingShip = NULL;

        float nearestZ = FLT_MAX;
        for (unsigned int i = 0 ; i < GGame->GetShipCount() ; i++)
        {
            GameShip * shp = GGame->GetShip(i);
            if ( shp == pShip)
                continue;

            vec_t shpPosClipSpace = shp->GetPhysics()->GetTransform().position;
            if ( Camera::IsBoxOnScreen( shipViewProj, shpPosClipSpace, 0.5f, 0.5f*1280.f/720.f ) && ( shpPosClipSpace.z < nearestZ) )
            {
                lockingShip = shp;
                nearestZ = shpPosClipSpace.z;

            }
        }

        if ((mPreviouslyLocking != lockingShip) || (!lockingShip))
        {
            mLockingProgress = GameSettings.BonusHomingTimeBeforeArmed;
        }
        mLockingProgress -= aTimeEllapsed;
        mLockingProgress = Clamp(mLockingProgress, 0.f, 10.f);
        pShip->mProps.lockingProgress = 0xFF - (u8)( (mLockingProgress/GameSettings.BonusHomingTimeBeforeArmed) * 255.f);

        if ( (mPreviouslyLocking != lockingShip) && (mPreviouslyLocking) )
        {
            mPreviouslyLocking->ClearLockedByNetIndex();
        }


        mPreviouslyLocking = lockingShip;

        if (lockingShip)
        {
            lockingShip->SetLockedByNetIndex( pShip->GetOwnerNetIndex() );
            pShip->SetLockingNetIndex( lockingShip->GetOwnerNetIndex() );
        }
        else
        {
            pShip->ClearLockingNetIndex();
        }
    }

    virtual void Apply(GameShip *pShip)
    {
        matrix_t misMat = pShip->GetMatrixForMissilesAndGun();
        misMat.position += misMat.dir * MinimalMissileDistance;

        GameMissile *pMis = (GameMissile *)GGame->Spawn(GO_Missile, misMat, pShip);
        pMis->SetDirection(pShip->GetPhysics()->GetTransform().dir);
        pMis->SetBrickIndex( pShip->GetPhysics()->GetCurrentRoadBlockIndex() );
        pShip->ClearLockingNetIndex();

        if (mPreviouslyLocking)
        {
            mPreviouslyLocking->ClearLockedByNetIndex();
            if (mLockingProgress <= FLOAT_EPSILON)
                pMis->SetTargeting( mPreviouslyLocking->GetOwnerNetIndex() );
        }

        mbMissileLaunched = true;
    }

    virtual bool CanBeDestroyed() const { return mbMissileLaunched;}
    // AI
    float mAICurrentTime;
    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        UNUSED_PARAMETER(pShip);

        mAICurrentTime += aTimeEllapsed;
        if ( mAICurrentTime >= GameSettings.MissileHomingTimeBeforeFiredAnyway )
            return true;

        if ( mLockingProgress <= FLOAT_EPSILON)
            return true;

        return false;

    }
protected:
    bool mbMissileLaunched;
    //u8 mLockedShipNetIndex;
    float mLockingProgress;
    GameShip *mPreviouslyLocking;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
class BonusTeslaHook : public Bonus
{
public:
    BonusTeslaHook() : Bonus(), mbHookUsed(false)
    {
        mHookRemaining = GameSettings.TeslaHookDuration;
        mAICurrentTime = 0.f;
    }
    virtual ~BonusTeslaHook()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_TeslaHook; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        pShip->mProps.mTeslaHooking = 0;

        if (mbHookUsed && ( mHookRemaining > 0.f) )
        {
            const vec_t& shpPos = pShip->GetPhysics()->GetTransform().position;

            const float maxTeslaDist = GameSettings.TeslaHookMaximalDistance;
            pShip->mProps.mTeslaHooking = 1;

            for (unsigned int i=0;i<GGame->GetShipCount();i++)
            {
                GameShip *shp = GGame->GetShip(i);
                if ( shp == pShip)
                    continue;

                const vec_t& otherPos = shp->GetPhysics()->GetTransform().position;
                float dist = (otherPos - shpPos).length();
                if (dist <= maxTeslaDist)
                {
                    pShip->mProps.mTeslaHooking = 2;
                }
            }
            mHookRemaining -= aTimeEllapsed;
            if ( mHookRemaining <= 0.f )
                pShip->mProps.mTeslaHooking = 0;
        }

    }
    virtual void Apply(GameShip *pShip)
    {
        UNUSED_PARAMETER(pShip);
        mbHookUsed = true;
    }
    virtual bool CanBeDestroyed() const { return ( mbHookUsed && (mHookRemaining <= 0.f) ); }

    float mAICurrentTime;
    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        ASSERT_GAME ( ( mAICurrentTime >= 0.f ) );

        mAICurrentTime += aTimeEllapsed;
        if (mAICurrentTime >= GameSettings.TeslaHookTimeBeforeUsedAnyway )
            return true;
        // use it if at least 1 other ship few meters in front of the ship
        return ( GetAnyShipTargetableBy( pShip, GameSettings.TeslaHookMinimalDistance, GameSettings.TeslaHookMaximalDistance ) != NULL);
    }

protected:
    float mHookRemaining;
    bool mbHookUsed;
};
*/


///////////////////////////////////////////////////////////////////////////////////////////////////
#define PORTAL_BRICK_OFFSET 10
#define PORTAL_BRICK_LENGTH 30

class BonusPortal : public Bonus
{
public:
    BonusPortal() { mbApplied = false; }
    virtual ~BonusPortal()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_Portal; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
    }
    virtual void Apply(GameShip *pShip)
    {
        if (!mbApplied)
        {
			GamePortal *portal= (GamePortal *)GGame->Spawn(GO_Portal, matrix_t::Identity );
			portal->SetBricksInfos( pShip->GetPhysics()->GetCurrentRoadBlockIndex() + PORTAL_BRICK_OFFSET, PORTAL_BRICK_LENGTH, pShip->GetPhysics()->GetLeftRightFactor(), true );
        }
        mbApplied = true;
    }
    virtual bool CanBeDestroyed() const
    {
        return mbApplied;
    }
	bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed ) 
	{
		return true;
	}

protected:
    bool mbApplied;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
class BonusAntiPortal : public Bonus
{
public:
    BonusAntiPortal() { mbApplied = false; }
    virtual ~BonusAntiPortal()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_AntiPortal; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
    }
    virtual void Apply(GameShip *pShip)
    {
        if (!mbApplied)
        {
			GamePortal *portal= (GamePortal *)GGame->Spawn(GO_Portal, matrix_t::Identity );
			portal->SetBricksInfos( pShip->GetPhysics()->GetCurrentRoadBlockIndex() - PORTAL_BRICK_OFFSET - PORTAL_BRICK_LENGTH, PORTAL_BRICK_LENGTH, pShip->GetPhysics()->GetLeftRightFactor(), false );

        }
        mbApplied = true;
    }
    virtual bool CanBeDestroyed() const
    {
        return mbApplied;
    }
	bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed ) 
	{
		return true;
	}

protected:
    bool mbApplied;
};
*/
///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusMagneticWave : public Bonus
{
public:
    BonusMagneticWave() { mbApplied = false; }
    virtual ~BonusMagneticWave()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_MagneticWave; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
    }
    virtual void Apply(GameShip *pShip)
    {
        if (!mbApplied)
        {
			//const matrix_t& shpGroundMat = pShip->GetMatrixForMissilesAndGun();
			
			GameExplosion *explo= (GameExplosion *)GGame->Spawn( GO_Explosion, matrix_t::Identity );
			explo->SetExplosionType( ELECTRIC_WALL );
			explo->SetBrick( pShip->GetPhysics()->GetCurrentRoadBlockIndex()+2 );
			/*
            pShip->mProps.mHealth += GameSettings.BonusRepairInc;
            pShip->mProps.mHealth = Clamp(pShip->mProps.mHealth, 0.f, 1.f);
			*/
        }
        mbApplied = true;
    }
    virtual bool CanBeDestroyed() const
    {
        return mbApplied;
    }
	bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed ) 
	{
		return true;
	}

protected:
    bool mbApplied;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusPlasmaWave : public Bonus
{
public:
    BonusPlasmaWave() { mbApplied = false; }
    virtual ~BonusPlasmaWave()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_PlasmaWave; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
    }
    virtual void Apply(GameShip *pShip)
    {
        if (!mbApplied)
        {
			GameExplosion *explo= (GameExplosion *)GGame->Spawn( GO_Explosion, matrix_t::Identity );
			explo->SetExplosionType( PLASMA_WALL );
			explo->SetBrick( pShip->GetPhysics()->GetCurrentRoadBlockIndex()+2 );
        }
        mbApplied = true;
    }
    virtual bool CanBeDestroyed() const
    {
        return mbApplied;
    }
protected:
    bool mbApplied;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusFireWave : public Bonus
{
public:
    BonusFireWave() { mbApplied = false; }
    virtual ~BonusFireWave()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_FireWave; }

    virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
    }
    virtual void Apply(GameShip *pShip)
    {
        if (!mbApplied)
        {
			GameExplosion *explo= (GameExplosion *)GGame->Spawn( GO_Explosion, matrix_t::Identity );
			explo->SetExplosionType( FIRE_WALL );
			explo->SetBrick( pShip->GetPhysics()->GetCurrentRoadBlockIndex()+2 );
        }
        mbApplied = true;
    }
    virtual bool CanBeDestroyed() const
    {
        return mbApplied;
    }
	bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed ) 
	{
		return true;
	}

protected:
    bool mbApplied;
};

///////////////////////////////////////////////////////////////////////////////////////////////////


class BonusBombing : public BonusTimedDrop
{
public:
    BonusBombing() : BonusTimedDrop( 8/*GameSettings.NumberOfMinesToDrop*/, 0.2f/*GameSettings.TimeBetwen2MinesDrop*/) , mCount(0) {}
    virtual ~BonusBombing()
    {
    }

    virtual Bonus_Type GetBonusType() const { return BT_Bombing; }

    virtual void DoDrop(GameShip *pShip)
    {
        ASSERT_GAME( mShip != NULL );

        //const vec_t& shpDir = mShip->GetPhysics()->GetTransform().dir;//
		//const vec_t& shpUp = mShip->GetPhysics()->GetTransform().up;//
        //matrix_t misMat = mShip->GetPhysics()->GetGroundMatrix();
		const matrix_t& shpGroundMat = pShip->GetMatrixForMissilesAndGun();
		matrix_t misMat;
		static const float heightloc = 30.f;
		misMat.LookAt( shpGroundMat.position + shpGroundMat.up*heightloc, shpGroundMat.position + shpGroundMat.up*heightloc + shpGroundMat.dir, shpGroundMat.up );
        //mineMat.position -= mineMat.up * 9.f;
        /*misMat.position = (shpDir +shpUp) (GameSettings.ShipRadius + GameSettings.mMineRadius * 3.f );
        misMat.position.w = 1.f;
        mineMat.orthoNormalize();
		*/
        GameMissile *mis = (GameMissile*)GGame->Spawn(GO_Missile, misMat, pShip );
        //FIXME: the owner net index should have already been set in Spawn()?
        //mine->mOwner = pShip->mProps.mNetIndex;
        //mine->mBrickIndex = pShip->GetPhysics()->GetCurrentRoadBlockIndex();
		

		mis->SetRoadTargeting( pShip->GetPhysics()->GetCurrentRoadBlockIndex() + 30 + mCount );
		mCount ++;

    }
    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
        // timed properties-> direct apply
        return true;
    }
	int mCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

Bonus * Bonus::Instanciate(Bonus_Type bt)
{
    Bonus *res = NULL;
    switch( bt)
    {
    case BT_Trackspeed:
    case BT_Color1:
    case BT_Color2:
        ASSERT_GAME(0);
        break;
    case BT_Missile:
        res = new BonusMissile;
        break;
    case BT_Mines:
        res = new BonusMines;
        break;
    case BT_Autopilot:
        res = new BonusAutopilot;
        break;
    case BT_Shield:
        res = new BonusShield;
        break;
    case BT_MachineGun:
        res = new MachineGun;
        break;
    case BT_SpeedBooster:
        res = new BonusSpeedBooster;
        break;
    case BT_3Missiles:
        res = new Bonus3Missiles;
        break;
    case BT_HomingMissile:
        res = new BonusHomingMissile;
        break;

	case BT_Portal:
		 res = new BonusPortal;
        break;
		/*
	case BT_AntiPortal:
		res = new BonusAntiPortal;
        break;
		*/
	case BT_MagneticWave:
		 res = new BonusMagneticWave;
        break;
	case BT_PlasmaWave:
		 res = new BonusPlasmaWave;
        break;
	case BT_FireWave:
		 res = new BonusFireWave;
        break;

	case BT_Bombing:
		 res = new BonusBombing;
        break;

    default:
        res = NULL;
        break;
    }
    ASSERT_GAME( (res == NULL) || (res->GetBonusType() == bt) );

    return res;
}

