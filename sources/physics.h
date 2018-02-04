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

#ifndef PHYSICS_H__
#define PHYSICS_H__


struct mesh_t;
#include "ZShipPhysics.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btTriangleIndexVertexArray;
class btRigidBody;
class btTransform;
class GameObject;
class GameShip;

typedef struct PhysCollision_t
{
	GameShip *mObject1, *mObject2; // user pointers
	vec_t mWorldPosition;
	vec_t mWorldNormal;
	float mDistance;
} PhysCollision_t;
#define MAXCOLLISIONSREPORT 1024



#define ROPESEGMENTS 32
class ShootItem;


static const float RopeDamping     = 0.01f;
/*
static const tvector2 GRAVITY(0, -9.81f);
float DENSITY_OFFSET                       = 100.f;
float GAS_K                                = 0.1f;
float VISC0SITY                            = 0.005f;
*/
inline void SolveVerletRope(vec_t& position, vec_t& positionOld, vec_t& velocity, vec_t acceleration, float timeStep)
{
	vec_t t;
	vec_t oldPos = position;
	//tvector2.Mult(ref acceleration, timeStep * timeStep, out acceleration);
	acceleration *= timeStep*timeStep;
	//tvector2.Sub(ref position, ref positionOld, out t);
	t = position - positionOld;
	//tvector2.Mult(ref t, 1.0f - Damping, out t);
	t *= 1.f-RopeDamping;
	//tvector2.Add(ref t, ref acceleration, out t);
	t += acceleration;
	//tvector2.Add(ref position, ref t, out position);
	position += t;
	positionOld = oldPos;

	// calculate velocity
	// Velocity = (Position - PositionOld) / dt;
	//tvector2.Sub(ref position, ref positionOld, out t);
	t = position-positionOld;
	//tvector2.Div(ref t, timeStep, out velocity);
	velocity = t*(1.f/timeStep);
}
mesh_t* createBoxMarker(const vec_t& pos, const vec_t& col, float sz);

struct Rope_t
{
public:	
	Rope_t();
    Rope_t( const Rope_t& nrope);
	~Rope_t();
	
	void Tick(float aTimeEllapsed);	

    void SetLinks( const vec_t& lnk1, mesh_t *pm, vec_t& lnk2 );

	float GetRealLength()
    {
        float len = 0.f;
        for (int i=1;i<ROPESEGMENTS;i++)
        {
            len += (mLinks[i].position-mLinks[i-1].position).length();
        }
        return len;
    }

    void SetLength( float len )
    {
        mLinkLength = len*( 1.f / (float)ROPESEGMENTS );
    }
    const matrix_t* const GetSkinMatrices() const { return skinsMat; }

	struct RopeLink_t
	{
		RopeLink_t()
		{
			position = vec( 0.f );
			positionOld = vec( 0.f );
			velocity = vec( 0.f );
			force = vec( 0.f );
		}
		RopeLink_t(const vec_t& pos)
		{
			position = pos;
			positionOld = pos;
			velocity = vec( 0.f );
		}
		vec_t position;
		vec_t positionOld;
		vec_t velocity;
		vec_t force;
	};

	RopeLink_t mLinks[ROPESEGMENTS];
	float mTime;
	float mLinkLength;

    //mesh_t *meshes[ROPESEGMENTS];
    mesh_t *cable;

    mesh_t *mLinkMesh;
    vec_t mLink1, mLink2;

    matrix_t skinsMat[32];
};


class PhysicWorld : public IPhysicWorld
{
public:
	PhysicWorld();
	virtual ~PhysicWorld() {}

	void init();
	void uninit();

	
	virtual bool rayCast(const vec_t& source, const vec_t& dest, vec_t& hit, vec_t& normal);

	void tick(float aTimeEllapsed);
	

	//ZShipPhysics* spawn(const matrix_t& mat);

	void setTrackMeshes(mesh_t *groundMesh, mesh_t *wallMesh);
	//bool sphereTest(const matrix_t& source, matrix_t& dest, float radius);

	// rigid body life
	virtual btRigidBody*	localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape);
	virtual void deleteRigidBody(btRigidBody *pBody);
    void addPhysicMesh(mesh_t *pMesh);
	// stack 
	void ClearTrack();
    btCollisionShape* BuildSimplifiedConvexHullFromMesh( mesh_t *pMesh );
    // collisions
    const FastArray<PhysCollision_t, MAXCOLLISIONSREPORT>& GetCollisions() const { return mCollisions; }
    void ClearCollisions() { mCollisions.Clear(); }

    // rope
    Rope_t *NewRope() { mRopes.push_back(Rope_t()); return &mRopes[mRopes.size()-1]; }
    std::vector<Rope_t> mRopes;

    virtual btDiscreteDynamicsWorld*	getDynamicWorld() { return dynamicsWorld; }
protected:
    FastArray<PhysCollision_t,MAXCOLLISIONSREPORT> mCollisions;
	// world
	btDefaultCollisionConfiguration*		collisionConfiguration;
	btCollisionDispatcher*					dispatcher;
	btBroadphaseInterface*					overlappingPairCache;
	btSequentialImpulseConstraintSolver*	solver;
	btDiscreteDynamicsWorld*				dynamicsWorld;
	// track
	typedef struct staticGeom_t
	{
		staticGeom_t() :
            trackShape(NULL),
            trackShapeIndexVertexArrays(NULL),
            trackBody(NULL),
            indices(NULL)
		{
		}
        ~staticGeom_t()
        {
            //must call ClearGeom() beforehands
            ASSERT_GAME(trackShape == NULL);
            ASSERT_GAME(trackShapeIndexVertexArrays == NULL);
            ASSERT_GAME(trackBody == NULL);
            ASSERT_GAME(indices == NULL);
        }

		btCollisionShape*						trackShape;
		btTriangleIndexVertexArray*				trackShapeIndexVertexArrays;
		btRigidBody*							trackBody;
		unsigned int *                          indices;
	} staticGeom_t;

	staticGeom_t mGroundGeom, mWallGeom;
	void ClearGeom(staticGeom_t& Geom);
	void addTrackMesh(mesh_t *pMesh, staticGeom_t *pGeom);

    std::vector<staticGeom_t> mPhysicBodiesList;
	
};

extern PhysicWorld physicWorld;


#endif