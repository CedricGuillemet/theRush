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
#include "physics.h"

#include "mesh.h"
#include "ZShipPhysics.h"
#include "camera.h"
#include "render.h"

#include "include_Bullet.h"

extern ContactAddedCallback		gContactAddedCallback;
PhysicWorld physicWorld;

Rope_t::Rope_t()
{
    mTime = 0.f;
    mLinkLength = 1.f;



    vec_t shp[]={vec(0.f), vec(0.f, 0.f, 32.f)};
    cable = generatePipe(shp, 2, 1.f, 0.3f, 2.f, 8, 0xFFFFFFFF );
    cable->mRope = this;
    cable->color = vec(1.f, 0.1f, 0.1f, 0.5f );
    cable->visible = true;
    cable->mWorldMatrix.identity();
    cable->updateWorldBSphere();

    mLinkMesh = NULL;
    
    /*
    for (int i=0;i<ROPESEGMENTS;i++)
        meshes[i] = createBoxMarker( vec(0.f), vec(1.f, 0.f, 0.f, 1.f), 1.f );
        */
}

Rope_t::Rope_t( const Rope_t& nrope)
{
    mTime = nrope.mTime;
    mLinkLength = nrope.mLinkLength;
    memcpy( mLinks, nrope.mLinks, sizeof(RopeLink_t) * ROPESEGMENTS );

    cable = nrope.cable;
    cable->mRope = this;

    mLink1 = nrope.mLink1;
    mLink2 = nrope.mLink2;
    mLinkMesh = nrope.mLinkMesh;
}

Rope_t::~Rope_t()
{
}

void Rope_t::SetLinks( const vec_t& lnk1, mesh_t *pm, vec_t& lnk2 )
{
    mLink1 = lnk1;
    mLinkMesh = pm;
    mLink2 = lnk2;

    mLinks[0].position = mLink1;
    mLinks[0].position.TransformPoint( mLinkMesh->mBaseMatrix );
    mLinks[0].positionOld = mLinks[0].position;

    for (int i=1;i<ROPESEGMENTS;i++)
    {
        float t = (float)i/(float)(ROPESEGMENTS-1);
        mLinks[i].position.lerp( mLinks[0].position, lnk2, t );
        mLinks[i].positionOld = mLinks[i].position;
    }
}

void Rope_t::Tick(float aTimeEllapsed)
{
    if ( aTimeEllapsed > 0.016f)
        aTimeEllapsed = 0.016f;

    PROFILER_START(RopeTick);


    //static const float linkSz = 0.25f;
    //static const float droppingTime = 0.25f;
    //static const float unDroppingTime = 0.25f;

    //mLinkLength = 1.f;//linkSz*2.f;
    static const vec_t gForce = vec(0.f, -9.81f, 0.f);
    static const float springConstant = 100.f;

    //static float gTime = 0.f;
    //gTime += aTimeEllapsed * 0.2f;


    mTime += aTimeEllapsed;
    static const float mFreqUpdate = 1.f/800.f;
    while (mTime>mFreqUpdate)
    {

    mLinks[0].position = mLink1;
    if ( mLinkMesh )
        mLinks[0].position.TransformPoint( mLinkMesh->mWorldMatrix );

    mLinks[ROPESEGMENTS-1].position = mLink2;
        
/*        
        mLinks[0].position = vec(0.f);
        mLinks[ROPESEGMENTS-1].position = vec(30.f, -10.f, 40.f * cosf(gTime) );
*/

        for (int i=1;i<ROPESEGMENTS;i++)
        {
            SolveVerletRope(mLinks[i].position, mLinks[i].positionOld, mLinks[i].velocity, mLinks[i].force, 
                mFreqUpdate);
        }

        for (int i=1;i<ROPESEGMENTS;i++)
        {
            mLinks[i].force = gForce;
        }

        for (int i=1;i<ROPESEGMENTS;i++)
        {
            vec_t dist = mLinks[i].position-mLinks[i-1].position;
            float len = dist.lengthSq();

            if (len > 0)							// To Avoid A Division By Zero... Check If r Is Zero
            {
                len = sqrtf(len);
                // The Spring Force Is Added To The Force		
                vec_t df =-(dist * ( 1.f/ len ) ) * (len - mLinkLength) * springConstant;
                mLinks[i].force += df;
                mLinks[i-1].force -= df;
            }

        }
        mTime -= mFreqUpdate;
    }


    vec_t amin(mLinks[0].position), amax(mLinks[0].position);
    for ( int i = 1 ; i < ROPESEGMENTS ; i++ )
    {
        amin.isMinOf( mLinks[i].position );
        amax.isMaxOf( mLinks[i].position );
    }
    cable->setBBox( amin, amax );
    cable->updateWorldBSphere();

    // compute matrices
     
    for (int j = 0;j<31;j++)
    {

        skinsMat[j].LookAt( mLinks[j].position, mLinks[j+1].position, vec(0.f, 1.f, 0.f) );
    }

    //skinsMat[31].identity();
    skinsMat[31].LookAt( mLinks[31].position, mLinks[31].position + (mLinks[31].position-mLinks[30].position), vec(0.f, 1.f, 0.f) );

    PROFILER_END();
}



btRigidBody*	PhysicWorld::localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);

	if (!shape && isDynamic)
	{
		//shape = new btBoxShape( btVector3(0.5, 0.2f, 1.f));//btSphereShape(btScalar(0.5));
		shape = new btSphereShape(btScalar(0.7));
		/*
		btCompoundShape* shapeCompound = new btCompoundShape();
	btTransform localTrans;
	localTrans.setIdentity();
	localTrans.setOrigin(btVector3(0,0,0.0));
	shapeCompound->addChildShape(localTrans,new btSphereShape(btScalar(0.5)) );

	localTrans.setOrigin(btVector3(0,0,0.5));
	shapeCompound->addChildShape(localTrans,new btSphereShape(btScalar(0.2)) );

	shape = shapeCompound;
	*/
	}

	if (isDynamic)
	{
		
		shape->calculateLocalInertia(mass,localInertia);
	
		
		
		//localInertia = btVector3 (0.233333f*3.f,0.2333333f*3.f,0.16666666f*2);
		localInertia = btVector3 (6.054f,6.404f,1.188f);
	}
	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);
	cInfo.m_friction = 0;
	cInfo.m_linearDamping = 0;

	btRigidBody* body = new btRigidBody(cInfo);

	btScalar m_defaultContactProcessingThreshold = 0.f;//used when creating bodies: body->setContactProcessingThreshold(...);

	body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);
	
	if (isDynamic)
	{
		body->setActivationState(DISABLE_DEACTIVATION);
	}
	
	dynamicsWorld->addRigidBody(body);

	return body;
}

void PhysicWorld::deleteRigidBody(btRigidBody* pBody)
{
    //FIXME: are there more things to clear before removing the rigid body? (ie: constraints, actions, forces, ...)
	dynamicsWorld->removeRigidBody(pBody);
#ifdef USE_MOTIONSTATE
    btMotionState* pMotionState = pBody->getMotionState();
    pBody->setMotionState(NULL);
    delete pMotionState;
    pMotionState = NULL;
#endif
    delete pBody;

    //NOTE: btCollisionShapes are stored seperately (in GPhysicShip or staticGeom_t::trackShape)
    //this function mustn't delete the collision shape associated to this rigid body
}

static bool CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObject* colObj0,int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1)
{
    UNUSED_PARAMETER(cp);
    UNUSED_PARAMETER(colObj0);
    UNUSED_PARAMETER(partId0);
    UNUSED_PARAMETER(index0);
    UNUSED_PARAMETER(colObj1);
    UNUSED_PARAMETER(partId1);
    UNUSED_PARAMETER(index1);
    /*
    // Apply material properties
    if (colObj0->getCollisionShape()->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE)
    {
        const btCollisionShape* parent0 = colObj0->getRootCollisionShape();
        if(parent0 != 0 && parent0->getShapeType() == MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE)
        {
            btMultimaterialTriangleMeshShape* shape = (btMultimaterialTriangleMeshShape*)parent0;
            const btMaterial * props = shape->getMaterialProperties(partId0, index0);
            cp.m_combinedFriction = calculateCombinedFriction(props->m_friction, colObj1->getFriction());
            cp.m_combinedRestitution = props->m_restitution * colObj1->getRestitution();
        }
    }
    else if (colObj1->getCollisionShape()->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE)
    {
        const btCollisionShape* parent1 = colObj1->getRootCollisionShape();
        if(parent1 != 0 && parent1->getShapeType() == MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE)
        {
            btMultimaterialTriangleMeshShape* shape = (btMultimaterialTriangleMeshShape*)parent1;
            const btMaterial * props = shape->getMaterialProperties(partId1, index1);
            cp.m_combinedFriction = calculateCombinedFriction(props->m_friction, colObj0->getFriction());
            cp.m_combinedRestitution = props->m_restitution * colObj0->getRestitution();
        }
    }
*/
	/*
	ZBaseClass *pObj1 = (ZBaseClass*)colObj0->getUserPointer();
	ZBaseClass *pObj2 = (ZBaseClass*)colObj1->getUserPointer();


	// 0 distance only for ship/wall
	if ( (!pObj1) || (!pObj2))
		cp.m_distance1 = 0;
		*/
	//LOG("Custom Material\n");
    //this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
	/*
	cp.m_normalWorldOnB.m_floats[1] = 0.f;
	cp.m_distance1 *= 3.f;
	((vec_t*)cp.m_normalWorldOnB.m_floats)->normalize();*/
    return true;
}


const int physicBodyReserveCount = 200;
	
void PhysicWorld::init()
{
    PROFILER_START(Physicworld);
	LOG("Init physic World ... \n");
    
	gContactAddedCallback = CustomMaterialCombinerCallback;

	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new	btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);

	btContactSolverInfo &solverInfo = dynamicsWorld->getSolverInfo();
	solverInfo.m_friction = 0.f;

	dynamicsWorld->setGravity(btVector3(0,0,0));

    mPhysicBodiesList.reserve( physicBodyReserveCount );

	LOG("Done\n");
    PROFILER_END(); // Physicworld    
}

void  PhysicWorld::ClearGeom(staticGeom_t& Geom)
{
    if (Geom.trackBody)
    {
        deleteRigidBody( Geom.trackBody );
        Geom.trackBody = NULL;
    }

    if (Geom.trackShape)
    {
        delete Geom.trackShape;
        Geom.trackShape = NULL;
    }

    if (Geom.trackShapeIndexVertexArrays)
    {
        delete Geom.trackShapeIndexVertexArrays;
        Geom.trackShapeIndexVertexArrays = NULL;
    }

    if (Geom.indices)
    {
        delete[] Geom.indices;
        Geom.indices = NULL;
    }
}

void PhysicWorld::ClearTrack()
{
	ClearGeom(mGroundGeom);
	ClearGeom(mWallGeom);
    mRopes.clear();

    for (unsigned int i = 0;i<mPhysicBodiesList.size();i++)
    {
        ClearGeom( mPhysicBodiesList[i] );
    }
    mPhysicBodiesList.clear();
}

btCollisionShape* PhysicWorld::BuildSimplifiedConvexHullFromMesh( mesh_t *pMesh )
{
    //ASSERT ( (pMesh->mIA->GetElementSize() == 4) );
#if 0
    btTriangleIndexVertexArray *shapeArrays = new btTriangleIndexVertexArray(pMesh->triCount/3,
        (int*)pMesh->mIA->Lock( VAL_READONLY ),
		3*sizeof( u32 ),
		pMesh->vtCount, (btScalar*)(pMesh->mVA->Lock( VAL_READONLY )), pMesh->mVA->GetVertexSize() );

	pMesh->mIA->Unlock();
	pMesh->mVA->Unlock();

	bool useQuantizedAabbCompression = true;

	btBvhTriangleMeshShape *shape  = new btBvhTriangleMeshShape( shapeArrays, useQuantizedAabbCompression );
    //collisionShape
    /*
    btConcaveShape *shape = new btGImpactMeshShape(shapeArrays);
    ((btGImpactMeshShape*)trimesh)->updateBound();
    */
#endif
    const btConvexHullShape originalConvexShape( (btScalar*)(pMesh->mVA->Lock( VAL_READONLY )), pMesh->mVA->GetVertexCount(), pMesh->mVA->GetVertexSize() );
    btShapeHull hull(&originalConvexShape);
	btScalar margin = originalConvexShape.getMargin();
	hull.buildHull(margin);
	btConvexHullShape* simplifiedConvexShape = new btConvexHullShape( (const btScalar *)(hull.getVertexPointer()), hull.numVertices() );
    //delete hull;
    //delete originalConvexShape;
    return simplifiedConvexShape;//shape;
}

void PhysicWorld::addPhysicMesh(mesh_t *pMesh)
{
	if (!pMesh)
		return;

    mPhysicBodiesList.push_back(staticGeom_t());
    staticGeom_t *pGeom = &mPhysicBodiesList[mPhysicBodiesList.size()-1];

    ASSERT_GAME( pGeom->indices == NULL );
	pGeom->indices = new unsigned int [pMesh->triCount];
	unsigned short *psht = (unsigned short *)pMesh->mIA->Lock( VAL_READONLY );
	for (int i=0;i<pMesh->triCount;i++)
	{
		pGeom->indices[i] = psht[i];
	}
	pMesh->mIA->Unlock();

    ASSERT_GAME( pGeom->trackShapeIndexVertexArrays == NULL );
	pGeom->trackShapeIndexVertexArrays = new btTriangleIndexVertexArray(pMesh->triCount/3,
		(int*)(pGeom->indices),
		3*sizeof(unsigned int),
        pMesh->mVA->GetVertexCount(), (btScalar*)(pMesh->mVA->Lock( VAL_READONLY )), pMesh->mVA->GetVertexSize() );
    //FIXME: where is the matching mIA->Lock()???
	pMesh->mIA->Unlock();
	pMesh->mVA->Unlock();

	bool useQuantizedAabbCompression = true;

	//pGeom->trackShape  = new btGImpactMeshShape( pGeom->trackShapeIndexVertexArrays );//
    //((btGImpactMeshShape*)pGeom->trackShape)->updateBound();
    ASSERT_GAME( pGeom->trackShape == NULL );
    pGeom->trackShape  = new btBvhTriangleMeshShape(pGeom->trackShapeIndexVertexArrays, useQuantizedAabbCompression);
    
	float mass = 0.f;
	btTransform	startTransform;
	startTransform.setIdentity();
	
    ASSERT_GAME( pGeom->trackBody == NULL );
	pGeom->trackBody = localCreateRigidBody(mass, startTransform, pGeom->trackShape);
	pGeom->trackBody->setUserPointer(pMesh);
	pGeom->trackBody->setCollisionFlags(pGeom->trackBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);


}


void PhysicWorld::addTrackMesh(mesh_t *pMesh, staticGeom_t *pGeom)
{
	if (!pMesh)
		return;

    ASSERT_GAME( pGeom->indices == NULL );
	pGeom->indices = new unsigned int [pMesh->triCount];
	unsigned short *psht = (unsigned short *)pMesh->mIA->Lock( VAL_READONLY );
	for (int i=0;i<pMesh->triCount;i++)
	{
		pGeom->indices[i] = psht[i];
	}
	pMesh->mIA->Unlock();

    ASSERT_GAME( pGeom->trackShapeIndexVertexArrays == NULL );
	pGeom->trackShapeIndexVertexArrays = new btTriangleIndexVertexArray(pMesh->triCount/3,
		(int*)(pGeom->indices),
		3*sizeof(unsigned int),
        pMesh->mVA->GetVertexCount(), (btScalar*)(pMesh->mVA->Lock( VAL_READONLY )), pMesh->mVA->GetVertexSize() );
    //FIXME: where is the matching mIA->Lock()???
	pMesh->mIA->Unlock();
	pMesh->mVA->Unlock();

	bool useQuantizedAabbCompression = true;

	//pGeom->trackShape  = new btGImpactMeshShape( pGeom->trackShapeIndexVertexArrays );//
    //((btGImpactMeshShape*)pGeom->trackShape)->updateBound();
    ASSERT_GAME( pGeom->trackShape == NULL );
    pGeom->trackShape  = new btBvhTriangleMeshShape(pGeom->trackShapeIndexVertexArrays, useQuantizedAabbCompression);
    
	float mass = 0.f;
	btTransform	startTransform;
	startTransform.setIdentity();
	
    ASSERT_GAME( pGeom->trackBody == NULL );
	pGeom->trackBody = localCreateRigidBody(mass, startTransform, pGeom->trackShape);
	pGeom->trackBody->setUserPointer(pMesh);
	pGeom->trackBody->setCollisionFlags(pGeom->trackBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);


}

bool PhysicWorld::rayCast(const vec_t& source, const vec_t& dest, vec_t& hit, vec_t& normal)
{
	btVector3 src(source.x, source.y, source.z);
	btVector3 dst(dest.x, dest.y, dest.z);
	btCollisionWorld::ClosestRayResultCallback cb(src, dst);
    cb.m_collisionFilterMask = ~btCollisionObject::CF_STATIC_OBJECT;
	dynamicsWorld->rayTest (src, dst, cb);
	if (cb.hasHit ())
	{
		hit = vec(cb.m_hitPointWorld.getX(), cb.m_hitPointWorld.getY(), cb.m_hitPointWorld.getZ());
		normal = vec(cb.m_hitNormalWorld.getX(), cb.m_hitNormalWorld.getY(), cb.m_hitNormalWorld.getZ());
		normal.normalize ();
		return true;
	} 
	/*
	else 
	{
		hit = dest[;
		normal = btVector3(1.0, 0.0, 0.0);
	}
	*/
	return false;

}

PhysicWorld::PhysicWorld()
{

}

void PhysicWorld::uninit()
{
	LOG("Uninit Physic World\n");
	ClearTrack();

	//delete dynamics world
	delete dynamicsWorld;

	//delete solver
	delete solver;

	//delete broadphase
	delete overlappingPairCache;

	//delete dispatcher
	delete dispatcher;

	delete collisionConfiguration;
	LOG("Done\n");
}


void PhysicWorld::tick(float aTimeEllapsed)
{
	
	dynamicsWorld->stepSimulation(aTimeEllapsed);

    // get collisions
	PROFILER_START(CollisionsAcquire);
	//int &acc = mCollisionsCount[mCollisionBufferBack];
	//acc = 0;
    ClearCollisions();

	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
	
		int numContacts = contactManifold->getNumContacts();
		for (int j=0;j<numContacts;j++)
		{
			GameShip *mUserPtr1 = (GameShip *)obA->getUserPointer();
			GameShip *mUserPtr2 = (GameShip *)obB->getUserPointer();


            if ( obA->getCollisionFlags()& btCollisionObject::CF_STATIC_OBJECT )
                mUserPtr1 = NULL;
            if ( obB->getCollisionFlags()& btCollisionObject::CF_STATIC_OBJECT )
                mUserPtr2 = NULL;

			btManifoldPoint& pt = contactManifold->getContactPoint(j);
            /*
			if (GGame)
			{
				if (mUserPtr1 && ( (mUserPtr1->GetClassID() == ClassIDZShip) || (mUserPtr1->GetClassID() == ClassIDZNetShip) ) )
				{
					ZRushGame *pGame = (ZRushGame *)GGame->GetServerGame();
					if (pGame) pGame->HandleShipCollision( (ZShip*)mUserPtr1, mUserPtr2, pt.getDistance());
					pGame = (ZRushGame *)GGame->GetClientGame();
					if (pGame) pGame->HandleShipCollision( (ZShip*)mUserPtr1, mUserPtr2, pt.getDistance());
				}
				if (mUserPtr2 && ( (mUserPtr2->GetClassID() == ClassIDZShip) || (mUserPtr2->GetClassID() == ClassIDZNetShip) ) )
				{
					ZRushGame *pGame = (ZRushGame *)GGame->GetServerGame();
					if (pGame) pGame->HandleShipCollision( (ZShip*)mUserPtr2, mUserPtr1, pt.getDistance());
					pGame = (ZRushGame *)GGame->GetClientGame();
					if (pGame) pGame->HandleShipCollision( (ZShip*)mUserPtr2, mUserPtr1, pt.getDistance());
				}
			}
            */

            PhysCollision_t *pc = mCollisions.PushCapped();//&mCollisions[mCollisions.size()-1];
            if ( pc )
            {
				pc->mDistance = pt.m_distance1;
				pc->mObject1 = mUserPtr1;
				pc->mObject2 = mUserPtr2;

				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptN = pt.m_normalWorldOnB;

				pc->mWorldPosition = vec(ptA.x(), ptA.y(), ptA.z());
				pc->mWorldNormal = vec(ptN.x(), ptN.y(), ptN.z());

            }
		}

		//you can un-comment out this line, and then all points are removed
		contactManifold->clearManifold();	
	}
    /*
	++mCollisionBufferBack&=1;
	++mCollisionBufferFront&=1;
    */

    // ropes

    std::vector<Rope_t>::iterator ropeIter =  mRopes.begin();
     std::vector<Rope_t>::iterator ropeEnd = mRopes.end();
    for (; ropeIter != ropeEnd; ++ropeIter)
        (*ropeIter).Tick(aTimeEllapsed* 10.f);

	PROFILER_END(); // CollisionsAcquire
    
}

void PhysicWorld::setTrackMeshes(mesh_t *groundMesh, mesh_t *wallMesh)
{
	addTrackMesh(groundMesh, &mGroundGeom);
	addTrackMesh(wallMesh, &mWallGeom);
}


