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

#pragma once

#include "maths.h"

class btRigidBody;
class Game;
class PhysicWorld;
class Track;
struct AIPoint_t;
struct track_t;
class btDiscreteDynamicsWorld;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ServerShipSync_t
{
	ServerShipSync_t()
	{
	}
	ServerShipSync_t(const vec_t &pos, const tquaternion& orient)
	{
		mPos = pos;
		mOrientation = orient;
	}

	vec_t mPos;
	tquaternion mOrientation;
} ServerShipSync_t;


class IPhysicWorld
{
public:
	virtual void deleteRigidBody( btRigidBody *) = 0;
	virtual btRigidBody * localCreateRigidBody(float,class btTransform const &,class btCollisionShape *) = 0;
	virtual bool rayCast(struct vec_t const &,struct vec_t const &,struct vec_t &,struct vec_t &) = 0;
	virtual btDiscreteDynamicsWorld*	getDynamicWorld() = 0;
};

class ITrack
{
public:

	virtual bool getClosestSampledPoint(struct vec_t const &,struct matrix_t &,struct vec_t &,int &,float &,struct vec_t const &,float &,float &,int &, bool) = 0;
	virtual bool holeOnTheWay(float,float) = 0;
	virtual const AIPoint_t& getAIPoint(int idx) const = 0;
	virtual unsigned int getAIPointCount() const = 0;
	virtual track_t *GetCurrentTrack() const = 0;
};

class IPhysicShip
{
public:
	IPhysicShip( Game *_GGame, IPhysicWorld *_physicWorld, ITrack *_track):
	  	GGame(_GGame),physicWorld(_physicWorld),track(_track)
	{
	}
	virtual ~IPhysicShip() {}

	virtual void Tick(float /*aTimeEllapsed*/, bool, bool ) {}
	virtual void Interpolate(float /*aLerp*/){}

	virtual void LoadShip( const matrix_t & /*initialMatrix*/, btCollisionShape * /*pCollisionShape*/, GameShip * /*pShip*/ ) {}


	// piloting

	virtual void TurnLeft(float /*strength*/ = 1.0f) {}
	virtual void TurnRight(float /*strength*/ = 1.0f) {}
	virtual void NoTurn() {}
	virtual void Run(float /*aFactor*/ = 1.f, const vec_t& /*boost*/ = vec(0.f)) {}
	virtual void NoRun() {}
	virtual void Brake(float /*strength*/ = 1.0f) {}
	virtual void BrakeRight(float /*strength*/ = 1.0f) {}
	virtual void BrakeLeft(float /*strength*/ = 1.0f) {}
	virtual void NoBrake() {}


	virtual void SetServerResync(const ServerShipSync_t& /*serverSync*/) {}

	virtual bool IsLapDone() const { return false; }
	virtual void ClearLapDoneFlag() {  }
	virtual int  IsWrongWay() const { return 0; }
	virtual const matrix_t& GetTime0RestartMatrix() const { return unusedMatrix; }
	virtual bool HasBeenReseted() const { return false; }
	virtual bool HalfTrackDone() const { return false; }

	virtual void SetSkill( float /*aSkill*/ ) {}

	virtual void EnablePhysics(bool /*bEnable*/) {  }
    virtual bool PhysicEnabled() const { return false; }

	virtual const matrix_t& GetGroundMatrix() const { return unusedMatrix; }
	virtual const matrix_t& GetGroundOrientationSmoothed() const { return unusedMatrix; }
	virtual void  SetGroundOrientationSmoothed( const matrix_t& ) {}
	virtual const matrix_t& GetTransform() const { return unusedMatrix; }
	virtual const vec_t& GetPosition() { return unusedVec; }
	virtual const vec_t& GetCurrentPosition() const { return unusedVec; }
	virtual const vec_t& GetCurrentOrientation() const { return unusedVec; }
	virtual const vec_t& GetPreviousPosition() const { return unusedVec; }
	virtual float GetSpeed() { return 0.f; }

	virtual int GetCurrentRoadBlockIndex() const { return 0; }
	virtual int GetCurrentRoadBlockIndexRaceSpace() const { return 0; }
	virtual ServerShipSync_t BuildResyncValues() const  { ServerShipSync_t res(vec(0.f), vec(0.f)  );  return res; }
	virtual void Reset() {}
	virtual void Reset(const matrix_t &/*aMat*/) { }
	virtual bool HasGroundRaycast() const { return true; }

	virtual float GetLeftRightFactor() const { return 0.f; }

	virtual void SetZeroSpeed() {}
	virtual void FastFowardTo( const matrix_t& ) {}
protected:
	matrix_t unusedMatrix;
	vec_t unusedVec;

	Game *GGame;
	IPhysicWorld *physicWorld;
	ITrack *track;
};
