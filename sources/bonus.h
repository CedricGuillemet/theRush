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

#ifndef BONUS_H__
#define BONUS_H__

///////////////////////////////////////////////////////////////////////////////////////////////////

struct mesh_t;
class GameShip;

void ForceBonusToShip(GameShip *pShip, int bonusNb);

enum Bonus_Type
{
	BT_Trackspeed,
	BT_Color1,
	BT_Color2,
	BT_Missile,
	BT_Mines,
	BT_Autopilot,
	BT_Shield,
	BT_MachineGun,
	BT_SpeedBooster,
	BT_3Missiles,
	BT_HomingMissile,
	BT_Portal,
	//BT_AntiPortal,
	BT_Bombing,
	BT_MagneticWave,
	BT_PlasmaWave,
	BT_FireWave,
    BT_None,
    BT_BonusCount
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Bonus
{
protected:
	Bonus() {}
	
public:
	virtual ~Bonus() {}

    virtual Bonus_Type GetBonusType() const = 0;

	virtual void Tick(GameShip *pShip, float aTimeEllapsed)
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
    }

	virtual void Apply(GameShip *pShip)
    {
        UNUSED_PARAMETER(pShip);
    }

	virtual bool CanBeDestroyed() const = 0;

    virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed )
    {
        UNUSED_PARAMETER(pShip);
        UNUSED_PARAMETER(aTimeEllapsed);
        return false;
    }

	static Bonus * Instanciate(Bonus_Type bt);
    // parameter BT_BonusCount to reset random
    static void ForceWeaponToAlwaysBe( Bonus_Type bt ) { bonusForcedToBe = bt; }

    static Bonus_Type BonusForced() { return bonusForcedToBe; }

protected:

    GameShip * GetAnyShipTargetableBy( GameShip *pShip, float aMinimalDistance, float aMaximalDistance = 99999.f, float aMaxDot = 1.f );

    static Bonus_Type bonusForcedToBe;
};

typedef struct trackBonusInstance_t
{
        trackBonusInstance_t();

        mesh_t* Init(const matrix_t& mat, Bonus_Type aType, int brickIndex);

        ~trackBonusInstance_t();

        void Clear();

        void GetActive();
        void GetInactive();
        bool IsActive() const { return (inactivityTime <= FLOAT_EPSILON); }

        mesh_t *activeStateMesh;
        mesh_t *inactiveStateMesh;
        vec_t bonusSphere;
        float inactivityTime; // <=0 means active. >0 time before getting back active
        Bonus_Type mType;
        int trackBrickIndex;
        matrix_t mWorldMatrix;

        // link
        trackBonusInstance_t *mNextInstance;

        // bonus activity

        static bool bAllBonusActive;

        // static
        static trackBonusInstance_t* GetFirstBonusInstanceForBrickIndex( unsigned int iBrickIndex )
        {
            if (iBrickIndex < mFirstTrackBonusForBrickIndex.size())
                return mFirstTrackBonusForBrickIndex[iBrickIndex];
            return NULL;
        }

        static void ClearBonuses();
        static mesh_t* PushTrackBonus(const matrix_t& mat, Bonus_Type aType, int brickIndex);
        static void TickBonuses(float aTimeEllapsed);


        static void DetectHandleShipOnTrackBonus(const vec_t& shipOrigin, const vec_t& shipDir, GameShip *pShip);

        static std::vector<trackBonusInstance_t>& GetTrackBonuses() { return GTrackBonusInstances; }
protected:
        static int lastBrickIndex;
        static std::vector<trackBonusInstance_t*> mFirstTrackBonusForBrickIndex;
        static trackBonusInstance_t *mLastInstance;
        static std::vector<trackBonusInstance_t> GTrackBonusInstances;

        
} trackBonusInstance_t;


///////////////////////////////////////////////////////////////////////////////////////////////////

class BonusTimedDrop : public Bonus
{
public:
    BonusTimedDrop(int dropCount, float dropTime, float minDist = 0.f) 
    { 
        mDropCount = dropCount; 
        mDropTime = dropTime;
        mCurrentTime = 0.f;
        mMinimalDistance = minDist;
        mShip = NULL;
		mbDropping = false;
    }

	virtual ~BonusTimedDrop()
	{
		mDropCount = -1;
		mDropTime = -1.f;
		mCurrentTime = -1.f;
		mMinimalDistance = -1.f;
		mShip = NULL;
		mbDropping = false;
	}

    virtual void DoDrop(GameShip *pShip) = 0;
	virtual void Tick(GameShip *pShip, float aTimeEllapsed);
    
	virtual void Apply(GameShip *pShip);
    
	virtual bool CanBeDestroyed() const { return (!mDropCount); }
    
protected:
    int mDropCount;
    bool mbDropping;
    GameShip *mShip;
    float mCurrentTime;
    float mDropTime;
    float mMinimalDistance;
    vec_t prevDropPos;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class MachineGun : public BonusTimedDrop
{
public:
    MachineGun();
    virtual ~MachineGun();

    virtual Bonus_Type GetBonusType() const { return BT_MachineGun; }
    
	virtual void Apply(GameShip *pShip);
    
    virtual void DoDrop(GameShip *pShip);
    virtual void Tick(GameShip *pShip, float aTimeEllapsed);

	virtual bool AIShouldUseIt( GameShip *pShip, float aTimeEllapsed );

    void SetShellCount( unsigned int count );
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
