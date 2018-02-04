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
#include "camera.h"
#include "world.h"
#include "physics.h"
#include "track.h"
#include "render.h"
#include "content.h"
#include "game.h"
#include "solo.h"
#include "gui.h"
#include "therush.h"
#include "audio.h"

Camera camera;
Camera camera2;


void Camera::project(float angle, float ratio, float zmin, float zmax)
{
	mProjection.glhPerspectivef2(angle, ratio, zmin, zmax);
	mNear = zmin;
	mFar = zmax;
	mFov = angle;
	mRatio = ratio;
	mProjectionInverse.inverse(mProjection);

	
}

void Camera::view(const vec_t& eye, const vec_t& target, const vec_t& up)
{
	mView.lookAtRH(eye, target, up);
	//mViewInverse.inverse(mView);
	//mViewInverse.LookAt(eye, target, up);
	mViewInverse.inverse(mView);
}

/*
typedef struct photoModeParams_t
{
	vec_t mPos;
	float mAngle;
	float mDistance;
	float mRoll;
	float mHeight;
} photoModeParams_t;
*/
void Camera::view(photoModeParams_t &params)
{
	vec_t eye = params.mShipMatrix.position;
	vec_t height = params.mShipMatrix.up * params.mHeight;

	eye += (params.mShipMatrix.right * cosf(params.mAngle) + params.mShipMatrix.dir * sinf(params.mAngle) ) * params.mDistance ;
	
	vec_t target = params.mShipMatrix.position;



	matrix_t transform;
	transform.LookAt(eye, target, vec(0.f, 1.f, 0.f));


	eye += height + transform.right * params.mLateral;
	target += height + transform.right * params.mLateral;


	vec_t locUp = vec(sinf(params.mRoll), cosf(params.mRoll), 0.f);
	locUp.TransformVector(transform);
	locUp.normalize();

	view(eye, target, locUp);
}

void Camera::computeCascadedShadowInfos()
{
    ASSERT_GAME( ShouldRenderShadows() );

    bool bIsSpace = ( track.GetCurrentTrack() && (track.GetCurrentTrack()->mEnvironmentIndex == 3) );
	vec_t rotatedSun =  bIsSpace?vec(1,1,1):GSkyScatParams->mSunDirection;
	mPSSMView.lookAtLH( rotatedSun/*GSkyScatParams->mSunDirection*/, vec(0.f), vec(0.f, 1.f, 0.f));
	
	float splitDists[] = { RenderSettings.PSSMDistance0,
                RenderSettings.PSSMDistance1,
                RenderSettings.PSSMDistance2,
                RenderSettings.PSSMDistance3,
                RenderSettings.PSSMDistance4
                             };

	matrix_t vp = mView * mProjection;
	matrix_t vpi;
	vpi.inverse(vp);

	vec_t localCorners[8] = { vec( -1.f, -1.f, 0.f ), 
		vec( -1.f,  1.f, 0.f ),
		vec(  1.f, -1.f, 0.f ),
		vec(  1.f,  1.f, 0.f ),
		vec( -1.f, -1.f, 1.f ), 
		vec( -1.f,  1.f, 1.f ),
		vec(  1.f, -1.f, 1.f ),
		vec(  1.f,  1.f, 1.f )};

	vec_t worldCorners[8], frustumVectors[4];
	for (int i=0;i<8;i++)
	{
		worldCorners[i].TransformPoint(localCorners[i], vpi);
		worldCorners[i] *= 1.f/worldCorners[i].w;
	}

	for (int i=0;i<4;i++)
		frustumVectors[i] = worldCorners[i+4] - worldCorners[i];



	vec_t splitCorners[8];
	float frustumZFactor = 1.f/(mFar - mNear);
    vec_t frustMin[4], frustMax[4];
	for (int iSplit = 0;iSplit<4;iSplit++)
	{
		float aLen = (splitDists[iSplit+1] - splitDists[iSplit]) * frustumZFactor;
		float aTr = (splitDists[iSplit] * frustumZFactor);

		for (int i=0;i<4;i++)
		{
			splitCorners[i] = worldCorners[i] + frustumVectors[i] * aTr;
			splitCorners[i+4] = splitCorners[i] + frustumVectors[i] * aLen;
		}

		for (int i=0;i<8;i++)
			splitCorners[i].TransformPoint(mPSSMView);

		
		frustMin[iSplit] = frustMax[iSplit] = splitCorners[0];

		for (int i = 1 ; i < 8 ; i ++)
		{
			frustMin[iSplit].isMinOf(splitCorners[i]);
			frustMax[iSplit].isMaxOf(splitCorners[i]);
		}
#if 0
		vec_t vDiagonal = splitCorners[0] - splitCorners[7];
		vDiagonal = vec(vDiagonal.length());
            
        // The bound is the length of the diagonal of the frustum interval.
        //float fCascadeBound = vDiagonal.x;
		/*
		const vec_t BorderOffset = ( vDiagonal - ( frustMax - frustMin ) ) * 0.5f;
		frustMax += BorderOffset;
		frustMin -= BorderOffset;
		*/

		//float fWorldUnitsPerTexel = fCascadeBound / (float)1024.f;//m_iBufferSize;
        //vec_t vWorldUnitsPerTexel = vec( fWorldUnitsPerTexel, fWorldUnitsPerTexel, 1.0f, 1.0f ); 

		frustMin.x *= 1.f/fWorldUnitsPerTexel;
		frustMin.y *= 1.f/fWorldUnitsPerTexel;
		frustMin.x = floorf( frustMin.x );
		frustMin.y = floorf( frustMin.y );
		frustMin.x *= fWorldUnitsPerTexel;
		frustMin.y *= fWorldUnitsPerTexel;


		frustMax.x *= 1.f/fWorldUnitsPerTexel;
		frustMax.y *= 1.f/fWorldUnitsPerTexel;
		frustMax.x = floorf( frustMax.x );
		frustMax.y = floorf( frustMax.y );
		frustMax.x *= fWorldUnitsPerTexel;
		frustMax.y *= fWorldUnitsPerTexel;
#endif  // 0
	
	}

    //FIXME: 'minZ' and 'maxZ' aren't used anymore, is this sthg that needs to be fixed?
    //float minZ = zmin(zmin(zmin(frustMin[0].z, frustMin[1].z), frustMin[2].z), frustMin[3].z );
    //float maxZ = zmax(zmax(zmax(frustMax[0].z, frustMax[1].z), frustMax[2].z), frustMax[3].z );

    for (int iSplit = 0;iSplit<4;iSplit++)
    {
        //frustMin[iSplit].z = minZ;
        //frustMax[iSplit].z = maxZ;
        
        ComputeShadowMatrix( iSplit, mPSSMView, frustMin[iSplit], frustMax[iSplit]  );
    }
}

void Camera::ComputeShadowMatrix( int iSplit, const matrix_t& psmView, const vec_t& frustMin, const vec_t& frustMax )
{
    ASSERT_GAME( ShouldRenderShadows() );

    mPSSMProjection[iSplit].OrthoOffCenterLH(
        frustMin.x , 
        frustMax.x , 
        frustMin.y , 
        frustMax.y , 
        frustMin.z-700.f, frustMax.z +100.f);
	
	mPSSMFrustum[iSplit].Update(psmView, mPSSMProjection[iSplit]);

	// Calculate a matrix to transform points to shadowmap texture coordinates
	// (this should be exactly like in your standard shadowmap implementation)
	//
	float fTexOffset = 0.5f;

	matrix_t mTexScale(  vec(0.5f,               0.0f,      0.0f,   0.0f),
		vec(0.0f,              0.5f,      0.0f,   0.0f),
		vec(0.0f,               0.0f,      1.0f,   0.0f),
		vec(fTexOffset,    fTexOffset,     0.00000f,   1.0f) );

	matrix_t PSSMShadowMat = mPSSMView * mPSSMProjection[iSplit] * mTexScale;

		
	matrix_t resc(  vec(0.25f,   0.0f,      0.0f,   0.0f),
		vec(0.0f,              1.f,      0.0f,   0.0f),
		vec(0.0f,               0.0f,      1.f,   0.0f),
		vec(0.25f*(float)iSplit,    0.f,     0.f,   1.0f) );
		
	mPSSMShadowMatAtlas[iSplit] = PSSMShadowMat * resc;
}

void Camera::computeMatricesAndFrustums()
{
    if (mLockOnMatrix)
    {
        vec_t behind = vec(0.f, 0.f, -4.f);
        behind.TransformPoint(*mLockOnMatrix);
        vec_t front = vec(0.f, 0.f, 30.f);
        front.TransformPoint(*mLockOnMatrix);
        
        view(behind+vec(0.f, 2.f, 0.f), front, vec(0.f, 1.f, 0.f));
    }

	mViewInverse.inverse(mView);

	mFrustum.Update(mView, mProjection);

    if ( ShouldRenderShadows() )
    {
        computeCascadedShadowInfos();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Camera::postTranslate(const vec_t & trans)
{
	matrix_t mt;
	mt.translation(trans);
	matrix_t mNewView = mView;
	mNewView *= mt;
	
	mView = mNewView;
}

void Camera::computeFPSView(float nXDiff, float nYDiff)
{
    if ((nXDiff != 0.0f)||(nYDiff != 0.0f))
    {
		nXDiff *= mSensibility;
		nYDiff *= mSensibility;
        vec_t	g_vEye;
        const matrix_t oldm = mViewInverse;//mView;//GetTransform()->GetWorldMatrix();

		g_vEye = oldm.position;

		vec_t	g_vLook = oldm.dir;
		vec_t	g_vUp = oldm.up;
		vec_t	g_vRight = oldm.right;


        // make rotation matrices
        matrix_t matRotation;


        matRotation.rotationAxis(g_vRight, nYDiff*0.05f);
		g_vLook.TransformVector(matRotation);
        g_vUp.TransformVector(matRotation);


        matRotation.rotationAxis(vec(0.f, 1.f, 0.f), nXDiff*0.05f);
        g_vLook.TransformVector(matRotation);
        g_vUp.TransformVector(matRotation);

        // make view matrix

        g_vLook.normalize();
        g_vRight.cross(g_vUp, g_vLook);
        g_vRight.normalize();
        g_vUp.cross(g_vLook, g_vRight);
        g_vUp.normalize();

		view(g_vEye, (g_vEye-g_vLook), g_vUp);
    }
}

void Camera::updateFPSMoves()
{
	float interv = mSensibility * 1.f;

	if (KeyShiftDown())
		interv *= 4.f;


	if (KeyLeftDown())
	{
		postTranslate(vec( interv, 0.f, 0.f ));
	}
	if (KeyRightDown())
	{
		postTranslate(vec( -interv, 0.f, 0.f ));
	}

	if (KeyUpDown())
	{
		postTranslate(vec( 0.f, 0.f, interv ));
	}
	if (KeyDownDown())
	{
		postTranslate(vec( 0.f, 0.f, -interv ));
	}
}

void Camera::BuildRay(int x, int y, int frameX, int frameY, vec_t &rayOrigin, vec_t &rayDir)
{

    vec_t screen_space;

    // device space to normalized screen space
    screen_space.x = ( ( (2.f * (float)x) / (float)frameX ) - 1 ) / mProjection.m[0][0];//.right.x;
    screen_space.y = -( ( (2.f * (float)y) / (float)frameY ) - 1 ) / mProjection.m[1][1];
    screen_space.z = -1.f;

    // screen space to world space

    rayOrigin = mViewInverse.position;
    rayDir.TransformVector(screen_space, mViewInverse);
    rayDir.normalize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Camera::tick(float aTimeEllapsed)
{
    if ( SequenceIsBeingPlayed() )
        return;

    if ( mObservedShip && mObservedShip->GetShipMesh() )
        mObservedShip->GetShipMesh()->onlyVisibleForShadows = false;

	switch (mMode)
	{
	case CM_SELONESHIP:
		{
            if (mCurrentCreditMesh)
                delete mCurrentCreditMesh;
            
            mCurrentCreditMesh = NULL;

			static float ngAround = 0.f;
			ngAround += aTimeEllapsed * 0.3f;

            const matrix_t& misMat = mObservedShip->GetMatrixForMissilesAndGun();
			const matrix_t &mt = mObservedShip->GetPhysics()->GetTransform();
			vec_t curEye = vec(cosf(ngAround), sinf(ngAround), 0.f) * 5.f + vec(0.f, 0.f, 1.0f);
            
            
            matrix_t plan;
            plan.LookAt(mt.position, mt.position + misMat.up, vec(0.f, 0.f, 1.f));
            
            curEye.TransformPoint(plan);
                        
            view(curEye, misMat.position-misMat.up*2.0f, misMat.up );

			matrix_t trLeft;
			trLeft.translation(2.5f, 0.f, 0.f);


			mView *= trLeft;
			mViewInverse.inverse(mView);

			//view( misMat.position + curEye, misMat.position, vec(0.f, 1.f, 0.f) );

			float len = (mt.position-curEye).length();
			setDOFFocus( len );
			setDOFBlur( 0.08f );
	}
			break;
	case CM_ENDRACE:
		{
            if (mCurrentCreditMesh)
                delete mCurrentCreditMesh;
            
            mCurrentCreditMesh = NULL;

			static float ngAround = 0.f;
			ngAround += aTimeEllapsed * 0.3f;

            const matrix_t& misMat = mObservedShip->GetMatrixForMissilesAndGun();
			const matrix_t &mt = mObservedShip->GetPhysics()->GetTransform();
			vec_t curEye = vec(cosf(ngAround), sinf(ngAround), 0.f) * 5.f + vec(0.f, 0.f, 1.5f);
            
            
            matrix_t plan;
            plan.LookAt(mt.position, mt.position + misMat.up, vec(0.f, 0.f, 1.f));
            
            curEye.TransformPoint(plan);
                        
			if (mObservedShip->mProps.mHealth <= 0.f )
				view(curEye+misMat.up*2.f, misMat.position-misMat.up*0.5f, misMat.up );
			else
				view(curEye, misMat.position-misMat.up*2.5f, misMat.up );

			float len = (mt.position-curEye).length();
			setDOFFocus( len );
			setDOFBlur( 0.08f );
		}
		break;
	case CM_BEHINDSHIP:
		{
			 int playerId = 0;
			 if (this == &camera2)
				 playerId = 1;

			 matrix_t drunkMat = mObservedShip->GetDrunkMatrix();
			 drunkMat.position *= 0.5f;
			 drunkMat.position.w = 1.f;
            matrix_t shipMat = drunkMat * mObservedShip->GetPhysics()->GetTransform();
            if (Solo::playerProfil.cameraMode[playerId] == 0)
            {
                // cockpit view
                view( shipMat.position, shipMat.position + shipMat.dir, shipMat.up );
                mObservedShip->GetShipMesh()->onlyVisibleForShadows = true;
                break;
            }
            
            
            vec_t currentEye = mViewInverse.position;

            const gameSettings_t& settings = GameSettings;
            /*
            float cameraBehindDistance = GameSettings;
            float cameraTensionFactor = 2.f;
            float cameraTargetDistance = 30.f;
            float cameraUpGroundShipFactor = 0.5f;
            float cameraLocalUp = 2.f;
            */

            // eye interpolation


            const CameraSetup_t& camSetup = settings.cameraSetups[ Solo::playerProfil.cameraMode[playerId] - 1 ];
            vec_t desiredEye = vec( 0.f, 0.f, -camSetup.CameraBehindDistance );
            desiredEye.TransformPoint( shipMat );

            vec_t localUp = vec( 0.f, camSetup.CameraLocalUp, 0.f );
            localUp.TransformVector( mObservedShip->GetPhysics()->GetGroundMatrix() );

            desiredEye += localUp;

            vec_t front = vec( 0.f, 0.f, camSetup.CameraTargetDistance );
            front.TransformPoint( shipMat );

            if ( miCameraForcedUpdate )
            {
                currentEye = desiredEye;
                miCameraForcedUpdate--;
            }


            currentEye = LERP( currentEye, desiredEye, aTimeEllapsed * camSetup.CameraTensionFactor );
            //currentEye = desiredEye;

            // planar clipping FRONT/BACK ---------------------------------------------------------

            vec_t normalBehindShip = - shipMat.dir;
            vec_t difBehind = currentEye - shipMat.position;
            float difLen = difBehind.dot(normalBehindShip);

            
            // recul
            if (difLen > camSetup.CameraBehindDistanceMax)
                currentEye -= normalBehindShip*(difLen-camSetup.CameraBehindDistanceMax);
            
            // front
            if (difLen < 0.f)
                currentEye -= normalBehindShip * difLen;
             
            // up
            vec_t currentUp = vec(0.f, 1.f, 0.f);
            currentUp.lerp( shipMat.up, camSetup.CameraUpGroundShipFactor );
            currentUp.normalize();
            
            // support for roll cage
            static int aCamYpType = 0;

            float dotRoll = currentUp.dot( shipMat.up );
            if ( dotRoll < camSetup.cameraRollingSwitch )
            {
                aCamYpType = 0;
            }
            else if ( dotRoll > 0.999f )
            {
                aCamYpType = 1;
            }

            
            if (aCamYpType == 0)
            {
                if (! mbLerpToShipUp )
                {
                    mCameraToShipUp = currentUp;
                }
                mbLerpToShipUp = true;

                mCameraToShipUp.lerp( shipMat.up, camSetup.cameraRollingInterpolationFactor ); 
                currentUp = mCameraToShipUp;
            }
            else if (aCamYpType == 1)
            {

                if ( mbLerpToShipUp )
                {
                    float canGetBackToCameraUp = currentUp.dot( mCameraToShipUp );
                    if ( canGetBackToCameraUp> 0.999f )
                    {
                        mbLerpToShipUp = false;
                    }
                    mCameraToShipUp.lerp( currentUp, camSetup.cameraRollingInterpolationFactor ); 
                    currentUp = mCameraToShipUp;
                }
                
            }
            
            // planar clipping UP/DOWN ------------------------------------------------------------

            vec_t normalOverShip = shipMat.up;
            vec_t difAbove = currentEye -  shipMat.position;
            float difHeight = difAbove.dot( normalOverShip );
            
            // above
            if (difHeight > camSetup.CameraAboveDistanceMax)
                currentEye -= normalOverShip*(difHeight-camSetup.CameraAboveDistanceMax);

            // under
            if (difHeight < camSetup.CameraAboveDistanceMin)
                currentEye += normalOverShip*(camSetup.CameraAboveDistanceMin - difHeight);
                
            // planar clipping LEFT/RIGHT ---------------------------------------------------------

            vec_t normalSideShip = shipMat.right;
            vec_t difSide = currentEye -  shipMat.position;
            float difRight = difSide.dot( normalSideShip );
            
            // right
            if ( difRight > camSetup.CameraSideDistance )
                currentEye -= normalSideShip*(difRight-camSetup.CameraSideDistance);

            if ( difRight < -camSetup.CameraSideDistance )
                currentEye -= normalSideShip*(difRight+camSetup.CameraSideDistance);
                
			// build matrices ---------------------------------------------------------------------

			view( currentEye, front, currentUp );

		}
		break;
	case CM_FPS:
		extern int MouseDeltaX, MouseDeltaY;
		camera.computeFPSView(-(float)MouseDeltaX, -(float)MouseDeltaY);
        updateFPSMoves();
		break;
	case CM_PRERACE:
		{
            preRaceLocTime -= aTimeEllapsed;
			if (preRaceLocTime <= 0.f)
			{
				preRaceLocTime = r01() * 1.2f + 1.0f;
                preRaceLocTime *= 4.f;

				halfLocTime = preRaceLocTime * 0.5f;
				
				IndexAIPoint += 40;
				const AIPoint_t& aipt = track.getAIPoint(IndexAIPoint);
				float ngx = r01() * 2.f * PI;
				float ngy = r01() + 0.3f ;
				float rad = 20.f;
				

				vec_t eyeEloi = (aipt.mAIRight[0] * cosf(ngx) + aipt.mAIDir[0] * sinf(ngx) * cosf(ngy)) + (aipt.mAIUp[0] * sinf(ngy));
				eyeEloi.normalize();
				observeSrc = aipt.mAIPos[0] + eyeEloi * rad;
				
				observeDst = track.getAIPoint(IndexAIPoint+2).mAIPos[0];
				
				observeUp = aipt.mAIUp[0];
				observeDir.cross(eyeEloi, observeUp);
				observeDir *= r01() - 0.5f;
				observeDir += observeUp * (r01() - 0.5f);
				observeDir.normalize();



			}
			vec_t curEye = observeSrc + (observeDir * (preRaceLocTime - halfLocTime) ) *0.3f;
			view(curEye, observeDst, observeUp);
			float len = (observeDst-curEye).length();
			setDOFFocus( len );
			setDOFBlur( 0.04f );
		}
		break;
	case CM_TRACKOBSERVE:
		{
			bool bSetMesh = false;
			matrix_t roadMat;

			static float locTime = -1.0f;
			locTime -= aTimeEllapsed;
			
            if (!mCurrentCreditMesh)
                bSetMesh = true;

			if ( bSetMesh || (locTime <= 0.f) )
			{
				locTime = r01() * 6.f + 2.f;
				halfLocTime = locTime * 0.5f;
				
				
				int idx = fastrand();
				const AIPoint_t& aipt = track.getAIPoint(idx);
				float ngx = r01() * 2.f * PI;
				float ngy = r01() + 0.3f ;
				float rad = 20.f;

				
				aipt.BuildMatrix( roadMat, false);

				vec_t eyeEloi = (aipt.mAIRight[0] * cosf(ngx) + aipt.mAIDir[0] * sinf(ngx) * cosf(ngy)) + (aipt.mAIUp[0] * sinf(ngy));
				eyeEloi.normalize();
				observeSrc = aipt.mAIPos[0] + eyeEloi * rad;
				
				observeDst = aipt.mAIPos[0];
				
				observeUp = aipt.mAIUp[0];
				observeDir.cross(eyeEloi, observeUp);
				observeDir *= r01() - 0.5f;
				observeDir += observeUp * (r01() - 0.5f);
				observeDir.normalize();

				bSetMesh = true;

				

			}
			vec_t curEye = observeSrc + (observeDir * (locTime - halfLocTime) ) *0.3f;
			view(curEye, observeDst, observeUp);
			vec_t hit, normal;

			if ( physicWorld.rayCast(curEye, observeDst, hit, normal) )
			{
				float len = (hit-curEye).length();
				setDOFFocus( len );
				setDOFBlur( 0.05f );
				//enableShowFocalCursor(true);
			}
			if (bSetMesh)
			{
				matrix_t meshMat = buildCreditsCameraAlignedMatrix(roadMat);

				// mesh params
                if (mCurrentCreditMesh)
                    delete mCurrentCreditMesh;
		
		
                mCurrentCreditMesh = getCreditsMesh(mCreditMeshIndex)->clone();
                //mCurrentCreditMesh->reattachToStackIndex( GetMeshStack() );
				mCreditMeshIndex  = getNextCreditsMeshIndex(mCreditMeshIndex);

                roadMat.position += roadMat.up * 3.f;
				mCurrentCreditMesh->mWorldMatrix = meshMat * roadMat;
				mCurrentCreditMesh->updateWorldBSphere();
                mCurrentCreditMesh->color = vec(1.f, 1.f, 1.f, 0.55f);
				mCurrentCreditMesh->visible = true;
			}
		}
		break;
	case CM_CUSTOM:
        if ( mCurrentCreditMesh )
            delete mCurrentCreditMesh;//
        mCurrentCreditMesh = NULL;//mCurrentCreditMesh->visible = false;
		break;
	}
}

void Camera::SetCameraCustom()
{ 
    Audio::InvalidatePosition();
	mLockOnMatrix = NULL; 
	mMode = CM_CUSTOM; 
	if (mCurrentCreditMesh)
		delete mCurrentCreditMesh;
	mCurrentCreditMesh = NULL;
    if (mObservedShip && mObservedShip->GetShipMesh() )
        mObservedShip->GetShipMesh()->onlyVisibleForShadows = false;
    mObservedShip = NULL;
}

void Camera::SetCameraBehindShip(GameShip *pShp) 
{ 
    Audio::InvalidatePosition();
    miCameraForcedUpdate = 2; 
    mObservedShip = pShp; 
    mMode = CM_BEHINDSHIP; 
    mLockOnMatrix = NULL; 
    mbLerpToShipUp = false; 

	int playerId = 0;
	if (this == &camera2)
		playerId = 1;

    if ( !Solo::playerProfil.cameraMode[playerId] )
        mObservedShip->GetShipMesh()->onlyVisibleForShadows = true;

}

void Camera::SetCameraTrackObserve() 
{ 
    Audio::InvalidatePosition();
	if (mMode == CM_TRACKOBSERVE)
		return;
	mLockOnMatrix = NULL; 
	mMode = CM_TRACKOBSERVE; 
	mCreditMeshIndex = 0;
    mObservedShip = NULL;
    if (mCurrentCreditMesh)
        delete mCurrentCreditMesh;
    mCurrentCreditMesh = NULL;
}

void Camera::SetPreRace( bool bForced )
{
    if ( (!bForced ) && ( mMode == CM_PRERACE ) )
        return;

    Audio::InvalidatePosition();

    preRaceLocTime = -1.0f;
    IndexAIPoint = 0;
    mMode = CM_PRERACE;
    mLockOnMatrix = NULL; 
    mCreditMeshIndex = 0;
    mObservedShip = NULL;
    if (mCurrentCreditMesh)
        delete mCurrentCreditMesh;
    mCurrentCreditMesh = NULL;
}

matrix_t Camera::buildCreditsCameraAlignedMatrix(const matrix_t &roadMat)
{
    // roadmat : mat at track point
	vec_t posText = roadMat.position;
	vec_t dirText = posText-camera.mViewInverse.position;

	matrix_t invRoadMat;
	invRoadMat.inverse( roadMat );
	dirText.TransformVector( invRoadMat );
	dirText.normalize();

	bool upView = (fabsf(dirText.y) >= 0.66f);
	dirText.y = 0.f;
	dirText.normalize();

	vec_t aMaxedDir;

	if ( fabsf(dirText.x) > fabsf(dirText.z) )
		aMaxedDir = vec( (dirText.x<0.f)?-1.f:1.f, 0.f, 0.f );
	else
		aMaxedDir = vec( 0.f, 0.f, (dirText.z<0.f)?-1.f:1.f );

	matrix_t lookat;

	if (!upView)
	{
		lookat.LookAt( vec(0.f), -aMaxedDir, vec(0.f, 1.f, 0.f));
	}
	else
	{
		lookat.LookAt( vec(0.f), vec(0.f, 1.f, 0.f), aMaxedDir );
	}
	return lookat;
}
