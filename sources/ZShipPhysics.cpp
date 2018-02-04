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

#include "ZShipPhysics.h"
#include "physics.h"
#include "track.h"
#include "render.h"
#include "gui.h"
#include "camera.h"
#include "world.h"

#include "include_Bullet.h"

bool KeyCtrlDown();

//#define PHYSICDEBUG

mesh_t* createBoxMarker(const vec_t& pos, const vec_t& col, float sz);

///////////////////////////////////////////////////////////////////////////////////////////////////

class ShipbtRigidBody
{
private:
    btRigidBody* mBtRigidBody;

public:
    ShipbtRigidBody() : mBtRigidBody(NULL)
    {
    }

    ~ShipbtRigidBody()
    {
        ASSERT_GAME_MSG( mBtRigidBody == NULL, "%s", "Bullet Rigid body must be released before destroying ShipbtRigidBody" );
    }

    void SetBtRigidBody( btRigidBody* shipRigidBody )
    {
        ASSERT_GAME( mBtRigidBody == NULL );
        ASSERT_GAME( shipRigidBody != NULL );

        mBtRigidBody = shipRigidBody;
    }

    btRigidBody* ReleaseBtRigidBody()
    {
        btRigidBody* shipRigidBody = mBtRigidBody;
        mBtRigidBody = NULL;

        return shipRigidBody;
    }
    
    btRigidBody* getBtRigidBody()
    {
        return mBtRigidBody;
    }

    bool IsInitialized() const
    {
        return (mBtRigidBody != NULL);
    }

    void GetMatrix(matrix_t* pMat) const
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->getWorldTransform().getOpenGLMatrix(pMat->m16);
    }
    void SetMatrix(const matrix_t& mat)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->getWorldTransform().setFromOpenGLMatrix(mat.m16);
    }
    void SetGravity(const vec_t& aGrav)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setGravity(btVector3(aGrav.x, aGrav.y, aGrav.z));
    }
    void ApplyForce(const vec_t& aForce)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->activate(true);
        mBtRigidBody->applyCentralImpulse(btVector3(aForce.x, aForce.y, aForce.z));
    }
    void ApplyTorque(const vec_t&aForce)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->activate(true);
        mBtRigidBody->applyTorque(btVector3(aForce.x, aForce.y, aForce.z));
    }

    tquaternion GetOrientationQuaternion() const
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        btQuaternion quat = mBtRigidBody->getWorldTransform().getRotation();
        return vec(quat.x(), quat.y(), quat.z(), quat.w());
    }
    void SetPosition(const vec_t &pos)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        //btQuaternion btquat(quat.x, quat.y, quat.z, quat.w);
        return mBtRigidBody->getWorldTransform().setOrigin( btVector3( pos.x, pos.y, pos.z ) );//.setRotation(btquat);
    }

    void SetOrientationQuaternion(const tquaternion &quat)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        btQuaternion btquat(quat.x, quat.y, quat.z, quat.w);
        return mBtRigidBody->getWorldTransform().setRotation(btquat);
    }

    vec_t GetLinearVelocity() const
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        const btVector3& vel = mBtRigidBody->getLinearVelocity();
        return vec(vel.getX(), vel.getY(), vel.getZ());
    }

    void SetLinearVelocity(const vec_t &aVelocity)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setLinearVelocity(btVector3(aVelocity.x, aVelocity.y, aVelocity.z));
    }

    void SetAngularVelocity(const vec_t &aAngularVelocity)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setAngularVelocity(btVector3(aAngularVelocity.x, aAngularVelocity.y, aAngularVelocity.z));
    }

    vec_t GetAngularVelocity() const
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        const btVector3& vel = mBtRigidBody->getAngularVelocity();
        return vec(vel.getX(), vel.getY(), vel.getZ());
    }

    void ResetForceAndTorque()
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setAngularVelocity(btVector3(0,0,0));
        mBtRigidBody->setLinearVelocity(btVector3(0,0,0));
    }
    void SetDamping(float aLinear, float aAngular)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setDamping(aLinear, aAngular);
    }

    float GetMass() const
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        return (1.f/mBtRigidBody->getInvMass());
    }
    float GetInvMass() const
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        return mBtRigidBody->getInvMass();
    }

    void setCcdSweptSphereRadius(float aRadius)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setCcdSweptSphereRadius(aRadius);
    }

    void setCcdMotionThreshold(float ccdMotionThreshold)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setCcdMotionThreshold(ccdMotionThreshold);
    }

    void setUserPointer(void* userPointer)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        mBtRigidBody->setUserPointer(userPointer);
    }

    void applyCentralImpulse(const vec3 impulse)
    {
        ASSERT_GAME( mBtRigidBody != NULL );
        btVector3 btImpulse(impulse.x, impulse.y, impulse.z);
        mBtRigidBody->applyCentralImpulse(btImpulse);
    }
};


///////////////////////////////////////////////////////////////////////////////////////////////////

fhConstructionInfo::fhConstructionInfo()
{
    m_fh_distance = 2.0f;
    m_fh_damping = 0.25f;
    m_radius = 1.0f;
    m_fh_spring = 0.65f;

    m_gravityForce = 10.f;
    m_fh_normal = true;
    m_do_fh = true;
    m_do_rot_fh = true;
    m_do_anisotropic = true;
}

serializableField_t fhConstructionInfo::mSerialisableFields[] = {
    SED(fhConstructionInfo,m_fh_distance,"Spring distance over ground"),
    SED(fhConstructionInfo,m_fh_damping,"Spring damping"),
    SED(fhConstructionInfo,m_radius,"Spring influence radius"),
    SED(fhConstructionInfo,m_fh_spring,"Spring strength"),

    SED(fhConstructionInfo,m_gravityForce,"gravity force"),

    SED(fhConstructionInfo,m_fh_normal,"ground normal spring"),
    SED(fhConstructionInfo,m_do_fh,"linear spring"),
    SED(fhConstructionInfo,m_do_rot_fh,"rotation spring"),
    SED(fhConstructionInfo,m_do_anisotropic,"Compute anisotropic")

    };
NBFIELDS(fhConstructionInfo);

///////////////////////////////////////////////////////////////////////////////////////////////////

serializableField_t shipParameters_t::mSerialisableFields[] = {
    SED(shipParameters_t,mLinearDamping,"The higher, the more friction"),
    SED(shipParameters_t,mAngularDamping,"The higher, the more friction but for angles."),
	SED(shipParameters_t,mCompensateNoseForce,"Speed factor to get ship nose parallel to the road"),
	SED(shipParameters_t,mCompensateRightLeftForce,"Speed factor to get ship lateraly parallel to the road"),
	SED(shipParameters_t,mTorqueZ,"Force for turning"),
	SED(shipParameters_t,mTorqueY,"For of rolling (when ship is turning)"),
	SED(shipParameters_t,mRunForceFactor,"Force applied to the ship when accelerating"),
	SED(shipParameters_t,mGravityForce,"Gravity force factor. The higher, the faster it'll get down."),
	SED(shipParameters_t,mGroundForce,"Road counter gravity force. Only applied when ship is below mEquilibriumHeight"),

	SED(shipParameters_t,mBrakeForce,"Not used ATM"),
	SED(shipParameters_t,mBrakeLeftRightForce,"Not used ATM"),
	SED(shipParameters_t,mBrakeLeftRightForceBackward,"Not used ATM"),

    SED(shipParameters_t,mEquilibriumHeight,"Height at wich the ship is levitating."),
    SED(shipParameters_t,mEquilibriumDeadzone,"Dead zone. levitation is between Height-deadzone and height+deadzone"),
    SED(shipParameters_t,mMinimalHeightOnTrack,"Ship can't get below that height. never."),

    SED(shipParameters_t,mTimeToLockShipOnTrack,"Time To lock ship on track when it's in Equilibrium"),

    SED(shipParameters_t,mGripFactor,"Road grip. the higher, the more grip. the lower, the more savonette."),

    SED(shipParameters_t,mSpring,"Spring")
};
NBFIELDS(shipParameters_t);

///////////////////////////////////////////////////////////////////////////////////////////////////

shipParameters_t ShipParameters;

ZShipPhysics::ZShipPhysics( Game *_GGame, IPhysicWorld *_physicWorld, ITrack *_track ) : IPhysicShip( _GGame, _physicWorld, _track ) //ZRushGame *pGame)
{
    mbHasBeenReseted = false;
    mShipBody = new ShipbtRigidBody();
    mbHasGroundRaycasted = false;
    mbPhysEnable = true;
    m_bOnFloor=false;
    m_Speed = 0;
    maCurFourche = 0;
    mTurnStrength = 0.f;

    GravityDirection = vec(0.f, -1.f, 0.f);
    mCurrentPlan = vec(0.f, 1.f, 0.f, 0.f);

    mWrongWay = 0;
    maTrackCurIdx = maCurIdx = UNASSIGNED_BRICK_INDEX;

    mbHalfTrackDone = false;
    mbTrackDone = false;
    mTurning = 0;

    mBrakeLeftStrength = 0;
    mBrakeRightStrength = 0;
    mRunStrength = 0;
    mBrakeFullStrength = 0;
	mLeftrightfactor = 0.f;
    mNbBricksRunned = 0;//GTrack->GetSpawnMatrices().size() - pGame->GetClientCount();// >mShips.size();


    mLastKnownGoodBrickIndexOnTrack = UNASSIGNED_BRICK_INDEX;

    mSkill = 1.f;

    mWallZoneWithCollision = 1.f;


    // valeurs a exposer dans le system de serialisation:
    /*
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
    */


	/*
	Check en RT
	*/
	ShipParameters.mTorqueZ = 2.f * 12.f; // axe Z
	ShipParameters.mTorqueY = 2.67f * 12.f; // axe Y

        ShipParameters.mLinearDamping = 0.4f;
        ShipParameters.mAngularDamping = 0.99f;


	// --

    mbHasServerSync = false;

    mGroundMatrix.identity();
    mGroundOrientationSmoothed.identity();
}

ZShipPhysics::~ZShipPhysics()
{
    btRigidBody* shipRigidBody = mShipBody->ReleaseBtRigidBody();
    if ( shipRigidBody != NULL )
    {
        physicWorld->deleteRigidBody(shipRigidBody);
        shipRigidBody = NULL;
    }

    delete mShipBody;
    mShipBody = NULL;

}
//btRigidBody*	localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape);

void ZShipPhysics::LoadShip( const matrix_t &initialMatrix, btCollisionShape *pCollisionShape, GameShip * pShip  )
{
    mTransform = initialMatrix;
    mTime0Restarmatrix_t = initialMatrix;

    //btCollisionShape* bcs = new btBoxShape(btVector3(1.2f, 0.2f, 2.f) * 0.6f);
    btTransform tr;
    tr.setOrigin( btVector3( initialMatrix.position.x, initialMatrix.position.y, initialMatrix.position.z ) );
    tr.setRotation( btQuaternion( 0.f, 0.f, 0.f,1.f ) );

    ASSERT_GAME( mShipBody != NULL );
    btRigidBody* shipRigidBody = mShipBody->ReleaseBtRigidBody();

    if ( shipRigidBody != NULL )
    {
        physicWorld->deleteRigidBody(shipRigidBody);
        shipRigidBody = NULL;
    }

    shipRigidBody = physicWorld->localCreateRigidBody( 1.f, tr, NULL); //pCollisionShape );
    ASSERT_GAME( shipRigidBody != NULL );
    mShipBody->SetBtRigidBody( shipRigidBody );

    mShipBody->setCcdMotionThreshold(1.f);
    mShipBody->setCcdSweptSphereRadius(0.2f);

    mShipBody->SetGravity(vec(0.f, 0.f, 0.f));
    mShipBody->SetDamping( ShipParameters.mLinearDamping, ShipParameters.mAngularDamping );
    mShipBody->setUserPointer( pShip );

    Reset(initialMatrix);
}

void ZShipPhysics::Reset(const matrix_t &aMat)
{
    ASSERT_GAME( mShipBody != NULL && mShipBody->IsInitialized() );

    Reset();
    /*
    matrix_t mt2, mt3;
    mt2.translation(0.f, 1.f, 0.f);// drop height
    mt3 = aMat*mt2;
    */
    mTransform = aMat;

    m_Cnt = mTransform.position;
    mLastKnownGoodPointOnTrack = m_Cnt;
    mPreviousPosition = mCurrentPosition = m_Cnt;

    //SetDirection(aMat.dir, aMat.up);

    GravityDirection = -aMat.up;
    mCurrentPlan = vec(aMat.up.x, aMat.up.y, aMat.up.z, aMat.position.dot(vec_t(aMat.up)));//vec_t(0, 1, 0), 0);

    mShipBody->SetMatrix(aMat);
	btTransform trouv;
	trouv.setFromOpenGLMatrix( aMat.m16 );
	mShipBody->getBtRigidBody()->getMotionState()->setWorldTransform(trouv);
    mShipBody->ResetForceAndTorque();

    mTurning = 0;
    mRunStrength = 0.f;

    mGroundMatrix.identity();
    mGroundOrientationSmoothed.identity();
}

void ZShipPhysics::HandleBulletCollisions(float aTimeEllapsed)
{
    UNUSED_PARAMETER(aTimeEllapsed);

    ASSERT_GAME( mShipBody != NULL && mShipBody->IsInitialized() );

#ifdef CUSTOMCOLS
    matrix_t trackMat = mGroundMatrix;
    int nbCols;
    PhysCollision_t *pCols = mPhysicWorld->GetCollisionList(nbCols);
    vec_t aright = trackMat.right;

    if (nbCols)
    {
        /*
        static int frictionCount = 0;
        frictionCount++;
        LOG("Friction %d\n", frictionCount);
        */

        tvector4 colplan;
        colplan = vector4(trackMat.position, trackMat.right);

        float closerPlaneDist = 999999.f;
        int closerPlaneDistIndex = -1;


        float biggestWallPenetration=0;
        for (int co = 0;co<nbCols;co++)
        {
            // ship/ship Collision
            ZBaseClass *pObj1 = pCols[co].mObject1;
            ZBaseClass *pObj2 = pCols[co].mObject2;
            // ship/wall
            if  ( ( (pObj1 == this) && (!pObj2) ) ||
                ( (pObj2 == this) && (!pObj1) ) )
            {


                if (pCols[co].mDistance < biggestWallPenetration)
                    biggestWallPenetration = pCols[co].mDistance;

            }
            // ship/ship
            else if  ( ((pObj2 && (pObj1 == this) && ( pObj2->GetClassID() == ClassIDZShipPhysics ) )) ||
                ((pObj1 && (pObj2 == this) && ( pObj1->GetClassID() == ClassIDZShipPhysics ) )) )
            {
                if (fabsf(pCols[co].mDistance)>0.f)
                {
                    ZShipPhysics *pPhysicOther = (ZShipPhysics*)((pObj1==this)?pObj2:pObj1);
                    vec_t dif = pCols[co].mWorldPosition - GetPosition();
                    dif.normalize();
                    dif -= dif.dot(mGroundMatrix.up);

                    // position
                    vec_t decalage = dif * pCols[co].mDistance*0.5f;
                    m_Cnt += decalage;
                    pPhysicOther->m_Cnt -= decalage;

                    // force
                    /*
                    m_Force *= 0.6f;
                    pPhysicOther->m_Force *= 0.6f;
                    vec_t force1 = m_Force;
                    vec_t force2 = pPhysicOther->m_Force;
                    m_Force += force2*0.6f;
                    pPhysicOther->m_Force += force1*0.6f;
                    */

                    pCols[co].mDistance = 0.f;
                }
            }
        }
        mWallZoneWithCollision = 1.f - biggestWallPenetration +0.8f;
    }
    else
    {
        mWallZoneWithCollision = 1;
    }
#endif
}


vec_t   processFhSprings( btDynamicsWorld* m_dynamicsWorld, btRigidBody* body, const vec_t& gravDir )
{
    
    //const fhConstructionInfo& hitObjShapeProps = ShipParameters.mSpring;
	fhConstructionInfo hitObjShapeProps;
	
	hitObjShapeProps.m_fh_distance = 1.0f;
    hitObjShapeProps.m_fh_damping = 0.88f;
    hitObjShapeProps.m_radius = 0.5f;
    hitObjShapeProps.m_fh_spring = 0.99f;

    hitObjShapeProps.m_gravityForce = 5.f;
    hitObjShapeProps.m_fh_normal = true;
    hitObjShapeProps.m_do_fh = true;
    hitObjShapeProps.m_do_rot_fh = true;
    hitObjShapeProps.m_do_anisotropic = true;


    //vec_t grav = gravDir * ( hitObjShapeProps.m_gravityForce );// / body->getInvMass() );

    btRigidBody* cl_object = body;

    float rayLen = hitObjShapeProps.m_fh_distance + hitObjShapeProps.m_radius;
    btVector3 rayDirLocal(gravDir.x*rayLen,gravDir.y*rayLen,gravDir.z*rayLen);

    //m_dynamicsWorld
    //ctrl->GetRigidBody();
    btVector3 rayFromWorld = body->getCenterOfMassPosition();
    //btVector3   rayToWorld = rayFromWorld + body->getCenterOfMassTransform().getBasis() * rayDirLocal;
    //ray always points down the z axis in world space...
    btVector3   rayToWorld = rayFromWorld + rayDirLocal;


    btCollisionWorld::ClosestRayResultCallback resultCallback(rayFromWorld,rayToWorld);

    m_dynamicsWorld->rayTest (rayFromWorld,rayToWorld, resultCallback);
    if (resultCallback.hasHit())
    {
         btRigidBody* hit_object = body;

		 
        float distance = resultCallback.m_closestHitFraction;//*rayDirLocal.length();//-hitObjShapeProps.m_radius;
		/*
		
        if (distance >= daH)//(hitObjShapeProps.m_fh_distance + hitObjShapeProps.m_radius))
        {
            // apply gravity
            return grav * powf(distance - daH, 3.f);
        }
		*/
		//eturn -grav * powf(daH - distance, 0.2f);
		/*vec_t newVel = 
		cl_object->setLinearVelocity(-gravDir));
		*/
	
	/*
	btTransform trf;
	trf = body->getWorldTransform(  );
	//body->setLinearVelocity(btVector3(0,-0.1f,0));
	//body->getMotionState()->setWorldTransform( btTransform(trf.getRotation(), trf.getOrigin() + btVector3(0,-0.1f,0)));
	body->setCenterOfMassTransform( btTransform(trf.getRotation(), trf.getOrigin() + btVector3(0,-0.1f,0)));
	return vec(0.f) ;
#if 0
	*/
        btVector3 ray_dir = rayDirLocal.normalized();
        btVector3 normal = resultCallback.m_hitNormalWorld;
        normal.normalize();

        if (hitObjShapeProps.m_do_fh) 
        {
            btVector3 lspot = cl_object->getCenterOfMassPosition()
                + rayDirLocal * resultCallback.m_closestHitFraction;




            lspot -= hit_object->getCenterOfMassPosition();
            btVector3 rel_vel = cl_object->getLinearVelocity() - hit_object->getVelocityInLocalPoint(lspot);
            btScalar rel_vel_ray = ray_dir.dot(rel_vel);
            btScalar spring_extent = 1.0f - distance / hitObjShapeProps.m_fh_distance; 

            btScalar i_spring = spring_extent * hitObjShapeProps.m_fh_spring;
            btScalar i_damp =   rel_vel_ray * hitObjShapeProps.m_fh_damping;

            cl_object->setLinearVelocity(cl_object->getLinearVelocity() + (-(i_spring + i_damp) * ray_dir)); 
            if (hitObjShapeProps.m_fh_normal) 
            {
                cl_object->setLinearVelocity(cl_object->getLinearVelocity()+(i_spring + i_damp) *(normal - normal.dot(ray_dir) * ray_dir));
            }

            btVector3 lateral = rel_vel - rel_vel_ray * ray_dir;


            if (hitObjShapeProps.m_do_anisotropic)
            { 
                //Bullet basis contains no scaling/shear etc.
                const btMatrix3x3& lcs = cl_object->getCenterOfMassTransform().getBasis();
                btVector3 loc_lateral = lateral * lcs;
                const btVector3& friction_scaling = cl_object->getAnisotropicFriction();
                loc_lateral *= friction_scaling;
                lateral = lcs * loc_lateral;
            }

            btScalar rel_vel_lateral = lateral.length();

            if (rel_vel_lateral > SIMD_EPSILON) {
                btScalar friction_factor = hit_object->getFriction();

                btScalar max_friction = friction_factor * btMax(btScalar(0.0), i_spring);

                btScalar rel_mom_lateral = rel_vel_lateral / cl_object->getInvMass();

                btVector3 friction = (rel_mom_lateral > max_friction) ?
                    -lateral * (max_friction / rel_vel_lateral) :
                -lateral;

                cl_object->applyCentralImpulse(friction);
            }
        }


        if (hitObjShapeProps.m_do_rot_fh) 
        {
            btVector3 up2 = cl_object->getWorldTransform().getBasis().getColumn(1);

            btVector3 t_spring = up2.cross(normal) * hitObjShapeProps.m_fh_spring;
            btVector3 ang_vel = cl_object->getAngularVelocity();

            // only rotations that tilt relative to the normal are damped
            ang_vel -= ang_vel.dot(normal) * normal;

            btVector3 t_damp = ang_vel * hitObjShapeProps.m_fh_damping;  

            cl_object->setAngularVelocity(cl_object->getAngularVelocity() + (t_spring - t_damp));
        }
          
    }

	return vec(0.f);
}




void ZShipPhysics::HandleGroundRayCast()
{
    vec_t hitPosition, hitNormal;
    //float hitFactor;
    //vec_t mGroundRayCast;
    mbHasGroundRaycasted = false;
    //	if (mPhysicWorld->SyncRayCast(m_Cnt, m_Cnt+GravityDirection*50, &hitPosition, &hitNormal, &hitFactor, 0xFFFFFFFE))
    if (physicWorld->rayCast(m_Cnt, m_Cnt+GravityDirection*10.f, hitPosition, hitNormal))
    {
        
        if ( (hitPosition-m_Cnt).length() < 10.f )
            mbHasGroundRaycasted = true;
            
    }

    // leaving track

    vec_t trackHeightDif = mLastKnownGoodPointOnTrack-m_Cnt;
    if (trackHeightDif.dot(GravityDirection) < -20.f)
    {
        matrix_t restartMat;
        if (mLastKnownGoodBrickIndexOnTrack == UNASSIGNED_BRICK_INDEX)
        {
            restartMat = mTime0Restarmatrix_t;
        }
        else
        {
            track->getAIPoint(/*mLastKnownGoodBrickIndexOnTrack-12*/mLastKnownGoodBrickIndexOnTrack).BuildMatrix(restartMat, false);
            restartMat.position += restartMat.up * 3.f;
            restartMat.position.w = 1.f;

            //restartMat =//GetBrickMatrix(mLastKnownGoodBrickIndexOnTrack);
            //restartMat = mTime0Restarmatrix_t;
        }

        Reset(restartMat);
        mShipBody->applyCentralImpulse(vec3(restartMat.dir.x, restartMat.dir.y, restartMat.dir.z)*50.f);
        maTrackCurIdx = maCurIdx = mLastKnownGoodBrickIndexOnTrack;

        mbHasBeenReseted = true;
    }

}

void ZShipPhysics::HandleTrackResponse( float aTimeEllapsed, bool &bHoleDetected )
{
    UNUSED_PARAMETER(aTimeEllapsed);

//    vec_t closestPoint;
//    tquaternion closestQuat;
    vec_t trackMiddle;
    matrix_t trackMat;
    

    float distanceToBorder = 0;

    int prevmacuridx = maCurIdx;




    float mUpDownFactor; // unused for now
	
    if (!mbHasGroundRaycasted)
    {
        vec_t fakePtCol = m_Cnt;
        fakePtCol += GravityDirection*50;

        mCurrentPlan = buildPlan(fakePtCol , -GravityDirection);
        trackMat.translation(fakePtCol);
        //trackMat.position = vector4(fakePtCol, 1);
        // fix for lemniscate
        trackMat = mGroundMatrix;
        trackMat.position = vec( fakePtCol.x, fakePtCol.y, fakePtCol.z, 1.f );

        // fix for lemniscate

        mGroundMatrix = trackMat;

        // unlock
        mbLockedOnRoad = false;
        mfTimeBeforeRoadLock = ShipParameters.mTimeToLockShipOnTrack;

    }
    else if (track->getClosestSampledPoint(m_Cnt, trackMat, trackMiddle, maCurIdx, leftrightfactor,-GravityDirection, mUpDownFactor, distanceToBorder, maCurFourche, bHoleDetected))
    {
		mLeftrightfactor = leftrightfactor;
        mGroundOrientationSmoothed.lerp( mGroundOrientationSmoothed, trackMat, 0.1f );
        mGroundOrientationSmoothed.orthoNormalize();
        mGroundOrientationSmoothed.position.lerp( mGroundOrientationSmoothed.position, trackMat.position, 0.5f );
        //mGroundOrientationSmoothed.position = vec( 0.f, 0.f, 0.f, 1.f );
        /*
        matrix_t grdMat1, grdMat2;
        track->getAIPoint(maCurIdx).BuildMatrix( grdMat1, false );
        track->getAIPoint(maCurIdx+1).BuildMatrix( grdMat2, false );

        mGroundOrientationSmoothed.lerp( grdMat1, grdMat2, Clamp( mUpDownFactor*2.f, 0.f, 1.f) );
        */
        // 

        int trackPointCount = track->getAIPointCount();
        
        //printf("%d / %d / %d %s\n", maCurIdx, (int)(trackPointCount*0.4f), (int)(trackPointCount*0.6f), mbHalfTrackDone?"(HTD)":"" );
        if ( ( maCurIdx > (trackPointCount*0.4f)) && ( maCurIdx < (trackPointCount*0.6f)) )
        {
            mbHalfTrackDone = true;
            //printf("Half track done\n");
        }
        if ( ( maCurIdx < 8) && (mbHalfTrackDone))
        {
            mbHalfTrackDone = false;
            mbTrackDone = true;
            //printf("Track done\n");
        }

        mLastKnownGoodPointOnTrack = trackMat.position;
        //mLastKnownGoodBrickIndexOnTrack = maCurIdx;

        GravityDirection = -trackMat.up;


        mCurrentPlan = buildPlan(trackMat.position, trackMat.up);
        mGroundMatrix = trackMat;



		// wrong way computation
		float wrongWayDot = mTransform.dir.dot(mGroundMatrix.dir);
		mWrongWay = 0;
		if (wrongWayDot < 0)
		{
			if (wrongWayDot > -0.5f)
				mWrongWay = 1;
			else if (wrongWayDot > -0.75f)
				mWrongWay = 2;
			else 
				mWrongWay = 3;
		}

        /*
        if (abs(int(maCurIdx-prevmacuridx))<10000)
        mNbBricksRunned += (maCurIdx-prevmacuridx);
        */
#ifdef BRICKSRUNNED
        int nbIdx = GTrack->GetNbSamples();
        if ( (maCurIdx-prevmacuridx) < -(nbIdx>>1)) // new 5 prev 10000
            mNbBricksRunned += ( ( maCurIdx + nbIdx ) - prevmacuridx);
        else if ( (prevmacuridx-maCurIdx) < -(nbIdx>>1) ) // new 10000 prev 5
            mNbBricksRunned += ( maCurIdx - ( nbIdx + prevmacuridx ) );
        else
            mNbBricksRunned += ( maCurIdx - prevmacuridx );
#endif
    }
    // reset
    if ( maCurIdx != UNASSIGNED_BRICK_INDEX )
    {
        if ( mLastKnownGoodBrickIndexOnTrack == UNASSIGNED_BRICK_INDEX )
            mLastKnownGoodBrickIndexOnTrack = maCurIdx;

        if ( prevmacuridx == UNASSIGNED_BRICK_INDEX )
            prevmacuridx = maCurIdx;

        if ( maTrackCurIdx == UNASSIGNED_BRICK_INDEX )
            maTrackCurIdx = maCurIdx;

    }    


    int difBrique = maCurIdx-prevmacuridx;
    difBrique = (difBrique+track->getAIPointCount())%track->getAIPointCount();
    if ( difBrique > (int)(track->getAIPointCount()>>1) )
        difBrique = difBrique-track->getAIPointCount();

    maTrackCurIdx += difBrique;


    bHoleDetected = false;
    float trackLenFactor = track->GetCurrentTrack()->trackLength / (float)track->getAIPointCount();
    float minDistanceBeforeHole = (20.f * trackLenFactor);
    for ( int i = mLastKnownGoodBrickIndexOnTrack ; i < maTrackCurIdx ; i++ )
    {
        if (!track->holeOnTheWay( (float)(i%track->getAIPointCount())*trackLenFactor, minDistanceBeforeHole ) )
            //if ( ! track->GetCurrentTrack()->holeNearBrickIndex( i, 12 ) )
        {
            mLastKnownGoodBrickIndexOnTrack = i;
        }
        /*
        else
        {
            bHoleDetected = true;
        }
        */
        if (track->holeOnTheWay( (float)(i%track->getAIPointCount())*trackLenFactor, 30.f ) )
        {
            bHoleDetected = true;
        }
    }

    // debug print
    #ifdef PHYSICDEBUG
#ifndef RETAIL
    {
    char tmps[512];
    sprintf (tmps, "Track %d     ", maTrackCurIdx );
    ggui.putText( 0, 30, tmps );

    sprintf (tmps, "Last Good %d // %s            ", mLastKnownGoodBrickIndexOnTrack, bHoleDetected?"Hole detected":"" );
    ggui.putText( 0, 29, tmps );

    sprintf (tmps, "Local %d     ", maCurIdx );
    ggui.putText( 0, 28, tmps );
    }
#endif
#endif
}


void ZShipPhysics::SetZeroSpeed()
{
	m_Speed = 0.f;
	mShipBody->SetLinearVelocity( vec( 0.f ) );
}

void ZShipPhysics::FastFowardTo( const matrix_t& aMat )
{
	GravityDirection = -aMat.up;
    mCurrentPlan = aMat.up;

	mTransform = aMat;

    m_Cnt = mTransform.position;
    mLastKnownGoodPointOnTrack = m_Cnt;
    mPreviousPosition = mCurrentPosition = m_Cnt;


	mShipBody->SetMatrix(aMat);
	btTransform trouv;
	trouv.setFromOpenGLMatrix( aMat.m16 );
	mShipBody->getBtRigidBody()->getMotionState()->setWorldTransform(trouv);
    //mShipBody->ResetForceAndTorque();

	bool bHoleDetected = false;
	HandleTrackResponse( 0.001f, bHoleDetected );
}

void ZShipPhysics::Tick( float totalElapsed, bool commandJittering, bool roadGrip )
{
    ASSERT_GAME( mShipBody != NULL && mShipBody->IsInitialized() );

    if (totalElapsed>1.f) 
        return;

	mShipBody->SetDamping( ShipParameters.mLinearDamping, ShipParameters.mAngularDamping );

    mbHasBeenReseted = false;
    // new transform matrix
    matrix_t mNewTransformMatrix;

    // get bullet collisions and handle them
    HandleBulletCollisions(totalElapsed);

    // do a raycast to know if we are still on track
    HandleGroundRayCast();

    // physic-fake ship floating matrix


    //mTransform->Update();


    // server sync
    if ( GGame && GGame->GetPlayerShip() && GGame->GetPlayerShip()->GetPhysics() != this)
        HandleServerResync(totalElapsed);

    // ship transform

    mShipBody->GetMatrix(&mNewTransformMatrix);//ComputeShipTransform();

    // road locking system
/*   
    
    static float mfTimeBeforeRoadLock = TIME_TO_LOCK_ON_ROAD;
    */

    // apply new matrix
    //mTransform->SetLocalMatrix(mNewTransformMatrix);
    mTransform = mNewTransformMatrix;


    matrix_t currentShipMatrix = mTransform;//->GetLocalMatrix();

    // - lock easier/faster
    vec_t cPlanlock = buildPlan(mGroundMatrix.position, mGroundMatrix.up );
    float equiMin = ( ShipParameters.mEquilibriumHeight - ShipParameters.mEquilibriumDeadzone );
    if ( (cPlanlock.signedDistanceTo(m_Cnt) < equiMin ) && ( cPlanlock.signedDistanceTo(currentShipMatrix.position) > equiMin ) )
        mbLockedOnRoad = true;
    // -- lock easier

    m_Cnt = currentShipMatrix.position;

    bool bHoleDetected;
    HandleTrackResponse( totalElapsed, bHoleDetected );



     // here starts everything

    matrix_t GNewCalcsGround = mGroundMatrix;//mTime0Restarmatrix_t;



    vec_t shipDirAxe = currentShipMatrix.dir;
    vec_t shipRightAxe = currentShipMatrix.right;
    vec_t shipUpAxe = GNewCalcsGround.up;



    // pique du nez

    matrix_t curGround = GNewCalcsGround;
    curGround.inverse();
    vec_t invSRA;
    invSRA.TransformVector(shipDirAxe, curGround);


    vec_t invSRightA;
    invSRightA.TransformVector(shipRightAxe, curGround);

    vec_t newTorque;
    newTorque = vec_t::zero;

    // physic values



    newTorque += shipRightAxe * ShipParameters.mCompensateNoseForce * invSRA.y;

    // left/right compensate

    float leftrighttang = invSRightA.y;

    newTorque -= shipDirAxe * ShipParameters.mCompensateRightLeftForce * invSRightA.y;


    matrix_t mtCompensateTang;
    mtCompensateTang.identity();

    float ngZ = asinf(invSRightA.y);

    if (fabsf(ngZ)>0.5f)
    {
        float ngZToApply = (ngZ<0.f)?ngZ+0.5f:ngZ-0.5f;
        mtCompensateTang.rotationZ(-ngZToApply);
    }


    matrix_t mtCompensateNose;
    mtCompensateNose.identity();
	
    float ngX = asinf(invSRA.y);
    float mNGXMAxDelta = 0.2f;
    //LOG("ngX %5.4f\n",ngX);
    if (fabsf(ngX)>mNGXMAxDelta)
    {
        float ngXToApply = (ngX<0.f)?ngX+mNGXMAxDelta:ngX-mNGXMAxDelta;
        mtCompensateNose.rotationX(ngXToApply);
    }
	
    // turning --------------------------------------------------------

    if (mTurning == 1)
    {
        newTorque += (shipUpAxe * (- ShipParameters.mTorqueY ) * mTurnStrength);

        if (leftrighttang<0.5f)
        {
            newTorque += (shipDirAxe * ( ShipParameters.mTorqueZ ) * mTurnStrength);
        }

    }
    else if (mTurning == 2)
    {
        newTorque += (shipUpAxe * ( ShipParameters.mTorqueY ) * mTurnStrength);
        if (leftrighttang>-0.5f)
        {
            newTorque += (shipDirAxe * ( -ShipParameters.mTorqueZ ) * mTurnStrength);
        }
    }


	if ( commandJittering )
	{
		static float jitterTime = 0.f;
		jitterTime += totalElapsed;
		static float rand1, rand2, rand3;
		if (jitterTime > 0.04f)
		{
			rand1 = r01();
			rand2 = r01();
			rand3 = r01();
			jitterTime = -0.04f;
		}
		else if (jitterTime < 0.f)
		{
			newTorque+= shipUpAxe * (rand1 * 0.5f -0.25f) * 500.f;
			newTorque+= shipDirAxe * (rand2 * 0.5f -0.25f) * 500.f;
			newTorque+= shipRightAxe * (rand3 * 0.5f -0.25f) * 500.f;
		}
	}
	if (roadGrip)
		newTorque *= 0.5f;

    mShipBody->ApplyTorque( newTorque /**totalElapsed*/);


    // forces ----------------------------------

    vec_t ForceToApply(vec_t::zero);

    // running
    vec_t forceVector = currentShipMatrix.dir;
    forceVector = mGroundMatrix.dir*currentShipMatrix.dir.dot(mGroundMatrix.dir) +
        mGroundMatrix.right*currentShipMatrix.dir.dot(mGroundMatrix.right);
    forceVector.normalize();

   
    float redempForce = Clamp( 1.f -  ( m_Speed/128.f ), 0.f, 1.f);
    redempForce = Clamp( powf(redempForce, 0.5f ) , 0.f, 1.f );

    ForceToApply += forceVector * ShipParameters.mRunForceFactor * mRunStrength * redempForce + mBoost;//275.f;

	if (roadGrip)
		ForceToApply *= 0.5f;

    // braking 
     /* BRAKING - UNUSED
    if (mBrakeFullStrength>0.f)
    {
        ForceToApply += currentShipMatrix.dir * aSpeed * (-mBrakeForce) * mBrakeFullStrength;
    }
    else if (mBrakeLeftStrength>0.f)
    {
        ForceToApply += currentShipMatrix.right * aSpeed * (-mBrakeLeftRightForce) * mBrakeLeftStrength;
        ForceToApply += currentShipMatrix.dir * aSpeed * (-mBrakeLeftRightForceBackward) * mBrakeLeftStrength;
    }
    else if (mBrakeRightStrength>0.f)
    {
        ForceToApply -= currentShipMatrix.right * aSpeed * (-mBrakeLeftRightForce) * mBrakeRightStrength;
        ForceToApply += currentShipMatrix.dir * aSpeed * (-mBrakeLeftRightForceBackward) * mBrakeRightStrength;
    }
    */

    // gravity -------------------------------------------

#ifdef PHYSICDEBUG
#ifndef RETAIL
    char tmps[512];

#endif
#endif

    vec_t cPlan = buildPlan(GNewCalcsGround.position, GNewCalcsGround.up );
    float signedDist = cPlan.signedDistanceTo(currentShipMatrix.position );
    float ftStrength = ( fabsf( signedDist - ShipParameters.mEquilibriumHeight ) + 1.f + m_Speed);

    float recalageFactor = 0.f;
    
    if (bHoleDetected)
    {
        mbLockedOnRoad = false;
        mfTimeBeforeRoadLock = ShipParameters.mTimeToLockShipOnTrack;
    }
    
    

    bool bNeedForcedHeightRecal = false;

    if (mbPhysEnable)
    {
        if (!mbLockedOnRoad)
        {
            if ( signedDist < ShipParameters.mMinimalHeightOnTrack )
            {
                bNeedForcedHeightRecal = true;
            }

            if ( signedDist > ( ShipParameters.mEquilibriumHeight + ShipParameters.mEquilibriumDeadzone ) )
            {
                ForceToApply += cPlan * ( -ShipParameters.mGravityForce )  *ftStrength * 0.5f; // 0.5 -> lower gravity force 
                mfTimeBeforeRoadLock = ShipParameters.mTimeToLockShipOnTrack;
            }
            else if ( signedDist < ( ShipParameters.mEquilibriumHeight - ShipParameters.mEquilibriumDeadzone ) )
            {
                ForceToApply += cPlan * (  ShipParameters.mGravityForce /*ShipParameters.mGroundForce*/ ) * ftStrength;
                mfTimeBeforeRoadLock = ShipParameters.mTimeToLockShipOnTrack;
            }
            else
            {
                mfTimeBeforeRoadLock -= totalElapsed;
                if ( mfTimeBeforeRoadLock <= 0.f )
                {
                    mbLockedOnRoad = true;
                }
            }
        }

        else
        {

            if ( signedDist < ShipParameters.mMinimalHeightOnTrack )
            {
                bNeedForcedHeightRecal = true;
            }
			
            else
            if ( signedDist > ( ShipParameters.mEquilibriumHeight + ShipParameters.mEquilibriumDeadzone ) )
            {
                recalageFactor = ( ShipParameters.mEquilibriumHeight + ShipParameters.mEquilibriumDeadzone - signedDist); 
                m_Cnt += GNewCalcsGround.up * recalageFactor;       
            }
            else if ( signedDist < ( ShipParameters.mEquilibriumHeight - ShipParameters.mEquilibriumDeadzone ) )
            {
                recalageFactor = ( ShipParameters.mEquilibriumHeight - ShipParameters.mEquilibriumDeadzone - signedDist);   
                m_Cnt += GNewCalcsGround.up * recalageFactor;                  
            }
			
        }
    }
    else 
    {
        // only gravity
        ForceToApply = cPlan * ( -ShipParameters.mGravityForce )  *ftStrength;        
    }

    // DEBUG
#ifdef PHYSICDEBUG
#ifndef RETAIL
    sprintf( tmps, " H %5.2f %s - %s - %s (%5.3f) spd : %5.2f        ", signedDist, mbLockedOnRoad?"LOCKED":"      ", mbHasGroundRaycasted?"RAY":"   ",bHoleDetected?"HOLE":"    ", recalageFactor, m_Speed );
    ggui.putText(3, 6, tmps );
    static float timeForSpeedLog = 0.f;
    //LOG("%5.4f;%5.4f\n", timeForSpeedLog, m_Speed);
    timeForSpeedLog += totalElapsed;
#endif
#endif
    // END of Tick


    // LOCKER ON ROAD
    /*
    static mesh_t* roadProjection = NULL;
    if ( ! roadProjection)
    {
        roadProjection = createBoxMarker( vec(0.f), vec(1.f, 0.f, 0.f, 1.f), 30.f );
        roadProjection->visible = true;
    }
    if ( this == GGame->GetPlayerShip()->GetPhysics())
        roadProjection->mWorldMatrix = mGroundMatrix;

    roadProjection->updateWorldBSphere();
    */
    

    // -- / anti wall over \
	
     // collisions
#if 0
    const PhysCollision_t *pc = physicWorld.GetCollisions().GetFirst();
    const PhysCollision_t *pcEnd = physicWorld.GetCollisions().GetEndUsed();

    //float maxFriction = 0.f;
    bool helpGetItOnTrack = false;
    for (; pc != pcEnd; pc++)
    {
        //float impactValue = Clamp( (pc->mDistance * 0.01f), 0.0001f, 1.f);
        if ( pc->mObject1 && pc->mObject1->GetPhysics() == this )
            helpGetItOnTrack = true;
        if ( pc->mObject2 && pc->mObject2->GetPhysics() == this )
            helpGetItOnTrack = true;
        
        // no ship/ship collision will help you get on track
        if (pc->mObject2 && pc->mObject1)
            helpGetItOnTrack = false;
    }

    vec_t decalPosLateral = vec( 0.f );
    vec_t decalPosDirectional = vec( 0.f );

    if ( mbPhysEnable && helpGetItOnTrack )
    {
        
        vec_t dirShip = currentShipMatrix.dir;
        float dotGroundRight = mGroundMatrix.right.dot(dirShip);
        float dotGroundDir = mGroundMatrix.dir.dot(dirShip);


        float aAngle = 1.f;
        float aVector = 1.f;
        if ( dotGroundRight > 0.f )
        {
            aAngle = -aAngle;
            
        }
        if (leftrightfactor<0.5f)
        {
            aVector = -aVector;
        }

        if ( dotGroundDir < 0.f)
        {
            aAngle = -aAngle;
            
        }
        /*
#ifdef PHYSICDEBUG
#ifndef RETAIL
    char tmps[512];
    sprintf( tmps, " dif %5.2f %5.2f %5.2f / %5.4f    ", grdRight.x, grdRight.y, grdRight.z, leftrightfactor );
    ggui.putText(3, 7, tmps );
#endif
#endif
    */
        decalPosLateral =  mGroundMatrix.right * aVector;
        decalPosDirectional = mGroundMatrix.dir * aAngle;
    }
#endif

#ifdef PHYSICDEBUG
#ifndef RETAIL
    else
    {
        sprintf( tmps, "                                    ");
        ggui.putText(3, 7, tmps );
    }
#endif
#endif

    if ( bNeedForcedHeightRecal && mbPhysEnable)
    {
        //vec_t linearVelocity = mShipBody->GetLinearVelocity();
        //linearVelocity += mGroundMatrix.up * ShipParameters.mMinimalHeightOnTrack * 100.f;
        //mShipBody->SetLinearVelocity( linearVelocity );//
        //m_Cnt = ;
        m_Cnt = mGroundMatrix.position + mGroundMatrix.up * ShipParameters.mMinimalHeightOnTrack;
    }


    mNewTransformMatrix.position = vec(m_Cnt.x, m_Cnt.y, m_Cnt.z, 1.f);
    matrix_t newShipBodyMatrix = mtCompensateTang*mtCompensateNose*mNewTransformMatrix;

    // -- /

    // road grip
    vec_t shipSpeedVector = mShipBody->GetLinearVelocity();
    vec_t gravitySpeed = mGroundMatrix.up * shipSpeedVector.dot(mGroundMatrix.up);

    float shipProperSpeed = (shipSpeedVector - gravitySpeed).length();
    
    if (mbPhysEnable)
    {
        ForceToApply -= ( shipSpeedVector - gravitySpeed ) * ShipParameters.mGripFactor;//5.f;
        ForceToApply += ( newShipBodyMatrix.dir * shipProperSpeed ) * ShipParameters.mGripFactor;//5.f;
    }

    mShipBody->ApplyForce(ForceToApply*totalElapsed );


    // recompute 

    if ( mbLockedOnRoad && mbPhysEnable)
    {        
        vec_t linearVelocity = mShipBody->GetLinearVelocity();
        
        float linearVelocityLength = linearVelocity.length();
        
        if (linearVelocityLength > 0.01f)
        {

            vec_t linearVelocityNormalized = linearVelocity * (1.f/linearVelocityLength);
        

            linearVelocityNormalized -= mGroundMatrix.up * linearVelocityNormalized.dot( mGroundMatrix.up );

            mShipBody->SetLinearVelocity( linearVelocityNormalized * linearVelocityLength );//
        }
    }


#if 0
    if ( helpGetItOnTrack )
    {        
        vec_t linearVelocity = mShipBody->GetLinearVelocity();
        float linearVelocityLength = linearVelocity.length();
        
        vec_t linearVelocityNormalized = linearVelocity * (1.f/linearVelocityLength);
        if (linearVelocityLength > 0.01f)
        {

            vec_t newVelocity;
            newVelocity.lerp(decalPosLateral, linearVelocityNormalized, 0.90f);
            newVelocity.normalize();



            mShipBody->SetLinearVelocity( newVelocity * linearVelocityLength );//
        }
    }

#endif
        

    


	if ( mbPhysEnable )
		mShipBody->SetMatrix(newShipBodyMatrix);


    mPreviousQuaternion = mCurrentQuaternion;
    mPreviousPosition = mCurrentPosition;
    mCurrentPosition = newShipBodyMatrix.position;
    mCurrentQuaternion = mShipBody->GetOrientationQuaternion();

}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::TurnRight(float strength)
{
    mTurning = 2;
    mTurnStrength = Clamp(strength,0.f,1.f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::TurnLeft(float strength)
{
    mTurning = 1;
    mTurnStrength = Clamp(strength,0.f,1.f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::NoTurn()
{
    mTurning = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::Run(float aForce, const vec_t& boost )
{
    mRunStrength = aForce * mSkill;
    mBoost = boost;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::NoRun()
{
    mRunStrength = 0;
    mBoost = vec(0.f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::Brake(float strength )
{
    mBrakeFullStrength = strength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::BrakeRight(float strength )
{
    mBrakeRightStrength = strength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::BrakeLeft(float strength )
{
    mBrakeLeftStrength = strength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::NoBrake()
{
    mBrakeRightStrength = 0;
    mBrakeLeftStrength = 0;
    mBrakeFullStrength = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::Reset()
{
    m_bOnFloor=false;
    m_Speed=0;

    GravityDirection = vec(0.f, -1.f, 0.f);
    mCurrentPlan = vec(0.f, 1.f, 0.f, 0.f);

    mWrongWay = 0;
    maCurIdx = UNASSIGNED_BRICK_INDEX;

    //mbHalfTrackDone = false;
    mTurning = 0;
    mbHasBeenReseted = true;
    mbLockedOnRoad = false;
    mfTimeBeforeRoadLock = ShipParameters.mTimeToLockShipOnTrack;

    mbHasGroundRaycasted = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SYNC
void ZShipPhysics::SetSync(const ShipSync_t& ss)
{
    ASSERT_GAME( mShipBody != NULL && mShipBody->IsInitialized() );
    mShipBody->Sematrix_t(ss.mMatrix);
    mShipBody->SetLinearVelocity(ss.mVelocity);
    mShipBody->SetAngularVelocity(ss.mAngularVelocity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::GetSync(ShipSync_t* ss) const
{
    ASSERT_GAME( mShipBody != NULL && mShipBody->IsInitialized() );
    mShipBody->Gematrix_t(&ss->mMatrix);
    ss->mVelocity = mShipBody->GetLinearVelocity();
    ss->mAngularVelocity = mShipBody->GetAngularVelocity();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#endif
void ZShipPhysics::Interpolate(float aLerp)
{
    UNUSED_PARAMETER(aLerp);

    /*
    //mCurrentFrameInterpolation = aLerp;
    tquaternion newQuat;
    newQuat.Slerp(mPreviousQuaternion, mCurrentQuaternion, aLerp);
    vec_t newPosition;
    newPosition.Lerp(mPreviousPosition, mCurrentPosition, aLerp);
    matrix_t newMat;
    newMat.RotationQuaternion(newQuat);
    newMat.position = vector4(newPosition.x, newPosition.y, newPosition.z, 1);

    //mShipBody->Sematrix_t(newMat);
    //mTransform->SetLocalMatrix(newMat);
    //mTransform->Update();
    mTransform = newMat;
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::SetServerResync(const ServerShipSync_t& serverSync)
{
    mServerSync = serverSync;
    mbHasServerSync = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ZShipPhysics::HandleServerResync(float totalElapsed)
{
    if (!mbHasServerSync)
        return;

    ASSERT_GAME( mShipBody != NULL && mShipBody->IsInitialized() );

    UNUSED_PARAMETER(totalElapsed);

    //float tightness = Clamp(totalElapsed, 0.f, 1.f);
    

    float distDif = (mCurrentPosition-mServerSync.mPos).length();


    if (distDif> 5.f)
    {
        // snap
        matrix_t newMat;
        mPreviousQuaternion = mCurrentQuaternion = mServerSync.mOrientation;
        mPreviousPosition = mCurrentPosition = mServerSync.mPos;
        newMat.rotationQuaternion( mServerSync.mOrientation );
        newMat.position = vec( mServerSync.mPos.x, mServerSync.mPos.y, mServerSync.mPos.z, 1.f );
        mShipBody->SetMatrix( newMat );
    }
    else 
    {
        tquaternion newActiveOrientation = mShipBody->GetOrientationQuaternion();
#if 0
        if (distDif >0.5f)
        {
            // position/force
            vec_t diffForce = ( mServerSync.mPos - mCurrentPosition );

            /*
            diffForce *= mShipBody->GetMass() * totalElapsed * 20.f;
            mShipBody->ApplyForce( diffForce );
            // rotation
            /*
            newActiveOrientation.slerp(mShipBody->GetOrientationQuaternion(), mServerSync.mOrientation, tightness);
                */
            
        }

        vec_t linearVel = mShipBody->GetLinearVelocity();
        float linearVelLength = linearVel.length();
        if ( linearVelLength > FLOAT_EPSILON )
        {
            linearVel *= (1.f / linearVelLength );


            vec_t serverLinearVel = ( mServerSync.mPos - mCurrentPosition );
            float serverLinearVelLength = serverLinearVel.length();

            if ( serverLinearVelLength > FLOAT_EPSILON)
            {
                serverLinearVel *= (1.f/serverLinearVelLength);
                linearVel.lerp( serverLinearVel, 0.5f );
                linearVel.normalize();

                linearVel *= linearVelLength;

                //LOG("vel %5.4f %5.4f %5.4f \n", linearVel.x, linearVel.y, linearVel.z );
                mShipBody->SetLinearVelocity( linearVel );
            }
        }
#endif
        vec_t currentShipOrientation = mShipBody->GetOrientationQuaternion();
        slerp( &newActiveOrientation, &currentShipOrientation, &mServerSync.mOrientation, 0.5f );

        mbHasServerSync = false;
        mShipBody->SetOrientationQuaternion( newActiveOrientation );

        // pos
         vec_t nPos;
         nPos.lerp( mCurrentPosition, mServerSync.mPos, 0.5f );
        mShipBody->SetPosition( nPos );

    }

    //
}

///////////////////////////////////////////////////////////////////////////////////////////////////
