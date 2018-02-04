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

#ifndef ZSHIPPHYSICS_H__
#define ZSHIPPHYSICS_H__

//class ZRushGame;
//struct ShipSync_t;
#include <stdint.h>
#include <math.h>
#include "define_platform.h"
#include "define_macros.h"
#include "debug_common.h"
#include "debug_breakpoint.h"

#include "define_types.h"

#include "debug_assert.h"

#include <string>
#include <vector>
#include <list>
#include "maths.h"
#include "toolbox.h"


typedef vec_t tquaternion;
class btCollisionShape;
class GameShip;
#include "IPhysicShip.h"


///////////////////////////////////////////////////////////////////////////////////////////////////

struct fhConstructionInfo: public serialisableObject_t
{
    SERIALIZABLE(shipParameters_t,"Ship spring Parameters.")
    fhConstructionInfo();

    float m_fh_distance;
    float m_fh_damping;
    float m_radius;
    float m_fh_spring;

    float m_gravityForce;

    bool m_fh_normal;
    bool m_do_fh;
    bool m_do_anisotropic;
    bool m_do_rot_fh;
};

typedef struct shipParameters_t : public serialisableObject_t
{
    SERIALIZABLE(shipParameters_t,"Ship Physic Parameters.")

    shipParameters_t()
    {
        mLinearDamping = 0.2f;
        mAngularDamping = 0.99f;

        mCompensateNoseForce = 320.0f;
        mCompensateRightLeftForce = 10.0f;
        mTorqueZ = 20.f; // axe Z
        mTorqueY = 40.2f; // axe Y

        mRunForceFactor = 47.500f;
        mGravityForce = 5.8f;
        mGroundForce = 5.600f * 0.25f;


        mBrakeForce = 1.000f;
        mBrakeLeftRightForce = 200.f;
        mBrakeLeftRightForceBackward = 750.f;

        mEquilibriumHeight = 2.f; // cruise height
        mEquilibriumDeadzone = 0.1f;
        mMinimalHeightOnTrack = 0.5f;

        mTimeToLockShipOnTrack = 0.016f * 10.f;

        mGripFactor = 3.f;
    }


    float mLinearDamping, mAngularDamping;
	float mCompensateNoseForce;
	float mCompensateRightLeftForce;
	float mTorqueZ;
	float mTorqueY;
	float mRunForceFactor;
	float mGravityForce;
	float mGroundForce;
	float mBrakeForce;
	float mBrakeLeftRightForce;
	float mBrakeLeftRightForceBackward;

    float mEquilibriumHeight;
    float mEquilibriumDeadzone;
    float mMinimalHeightOnTrack;

    float mTimeToLockShipOnTrack;

    float mGripFactor;

    fhConstructionInfo mSpring;
} shipParameters_t;



extern shipParameters_t ShipParameters;

///////////////////////////////////////////////////////////////////////////////////////////////////

class ShipbtRigidBody;

class ZShipPhysics : public IPhysicShip
{

public:
	ZShipPhysics( Game *_GGame, IPhysicWorld *_physicWorld, ITrack *_track );
	//ZShipPhysics(ZRushGame *pGame);
	virtual ~ZShipPhysics();

    void Tick( float aTimeEllapsed, bool commandJittering, bool roadGrip );
    void Interpolate(float aLerp);

    void LoadShip( const matrix_t &initialMatrix, btCollisionShape *pCollisionShape, GameShip * pShip );


	// piloting

    void TurnLeft(float strength = 1.0f);
    void TurnRight(float strength = 1.0f);
    void NoTurn();
    void Run(float aFactor = 1.f, const vec_t& boost = vec(0.f));
    void NoRun();
	void Brake(float strength = 1.0f);
	void BrakeRight(float strength = 1.0f);
	void BrakeLeft(float strength = 1.0f);
	void NoBrake();

	// end piloting

    bool m_bOnFloor;



    int mWrongWay;
   
    const matrix_t& GetTransform() const { return mTransform; }
	virtual int  IsWrongWay() const { return mWrongWay; }
    bool mbHalfTrackDone;
	bool mbTrackDone;



        const vec_t& GetPosition() { return mTransform.position; }

	virtual float GetSpeed() { return m_Speed; }

	virtual const matrix_t& GetTime0RestartMatrix() const { return mTime0Restarmatrix_t; }

	virtual const matrix_t& GetGroundMatrix() const { return mGroundMatrix; }
    const matrix_t& GetGroundOrientationSmoothed() const { return mGroundOrientationSmoothed; }
	void  SetGroundOrientationSmoothed( const matrix_t& mat ) { mGroundOrientationSmoothed = mat; }
    // get track block index modulo aipoints count
	int GetCurrentRoadBlockIndex() const { return maCurIdx; }
    // get the block index in race space (not modulo aipoints count)
	int GetCurrentRoadBlockIndexRaceSpace() const { return maTrackCurIdx; }
	virtual float GetLeftRightFactor() const { return mLeftrightfactor; }
    virtual bool HasBeenReseted() const { return mbHasBeenReseted; }

	void Reset();
	void Reset(const matrix_t &aMat);
	void EnablePhysics(bool bEnable) { mbPhysEnable = bEnable; }
    bool PhysicEnabled() const { return mbPhysEnable; }
	virtual vec_t GetRoadDirectedVector()
	{
		const vec_t& shipdir = GetTransform().dir;
		const matrix_t& mat1 = GetGroundMatrix();
		vec_t avel = mat1.right*mat1.right.dot(shipdir);
		avel += mat1.dir*GetGroundMatrix().dir.dot(shipdir);
		avel.normalize();
		return avel;
	}

	int GetNbBricksRunned() { return mNbBricksRunned; }

	virtual bool HalfTrackDone() const { return mbHalfTrackDone; }
#ifdef SYNC
	void SetSync(const ShipSync_t& ss);
	void GetSync(ShipSync_t* ss) const;
#endif

  
	void SetServerResync(const ServerShipSync_t& serverSync);

	bool IsLapDone() const { return mbTrackDone; }
	void ClearLapDoneFlag() { mbTrackDone = false; }
    
    void SetSkill( float aSkill ) { mSkill = aSkill; }
    
	virtual void SetZeroSpeed();
	virtual void FastFowardTo( const matrix_t& mat );
public:
    
    float mSkill;
	int mNbBricksRunned;




    vec_t GravityDirection;
    vec_t mCurrentPlan;


	float mTurnStrength;
	float m_Speed;

    vec_t mBoost;

	matrix_t mGroundMatrix;
    matrix_t mGroundOrientationSmoothed;
	
	float mLeftrightfactor;
public:
	// controls
	float mBrakeLeftStrength;
	float mBrakeRightStrength;
	float mRunStrength;
	float mBrakeFullStrength;

	matrix_t mTime0Restarmatrix_t;
	float mWallZoneWithCollision;

	// left track
	vec_t mLastKnownGoodPointOnTrack;
	unsigned int mLastKnownGoodBrickIndexOnTrack;
	void HandleBulletCollisions(float aTimeEllapsed);
	bool HasGroundRaycast() const { return mbHasGroundRaycasted; }
	// ground raycast
	bool mbHasGroundRaycasted;
	void HandleGroundRayCast();

	// track response
	void HandleTrackResponse( float aTimeEllapsed, bool &bHoleDetected );


    
	// new version
	

    // not interpolated values
    const vec_t& GetCurrentPosition() const { return mCurrentPosition; }
    const vec_t& GetPreviousPosition() const { return mPreviousPosition; }
    const vec_t& GetCurrentOrientation() const { return mCurrentQuaternion; }
    // asked by the server to build values that are streamed to distant clients
    ServerShipSync_t BuildResyncValues() const  { ServerShipSync_t res(mCurrentPosition, mCurrentQuaternion );  return res; }
protected:
    int maCurIdx, maCurFourche, maTrackCurIdx;
    matrix_t mTransform;

    vec_t m_Cnt; // a supprimer
    float leftrightfactor;

    int mTurning;
    ShipbtRigidBody* mShipBody;

    bool mbPhysEnable; // usefullness?

    // road lock
    bool mbLockedOnRoad;
    float mfTimeBeforeRoadLock;


    // reseted
    bool mbHasBeenReseted;

    // Interpolation
    tquaternion mPreviousQuaternion, mCurrentQuaternion;
    vec_t mPreviousPosition, mCurrentPosition;
    vec_t mCurrentOrientation, mPreviousOrientation;
    //float mCurrentFrameInterpolation;
    ServerShipSync_t mServerSync;
    bool mbHasServerSync;
    void HandleServerResync(float totalElapsed);

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
