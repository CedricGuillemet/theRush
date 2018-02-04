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


#ifndef CAMERA_H__
#define CAMERA_H__


#define PSSM_SPLIT_COUNT 4
struct mesh_t;
class GameShip;

///////////////////////////////////////////////////////////////////////////////////////////////////

enum CAMERA_MODE
{
	CM_FPS, // default
	CM_CUSTOM, // editor
	CM_PRERACE,
	CM_TRACKOBSERVE,
	CM_ENDRACE,
	CM_BEHINDSHIP,
	CM_SELONESHIP
};

typedef struct photoModeParams_t
{
	matrix_t mShipMatrix;
	float mAngle;
	float mDistance;
	float mRoll;
	float mHeight;
	float mLateral;
	float mDOF;
	float mFocal;
} photoModeParams_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

class Camera
{
public:
	Camera() { mLockOnMatrix = NULL; mMode= CM_FPS; mCurrentCreditMesh = NULL; miCameraForcedUpdate = 0; mObservedShip = NULL; mSensibility = 1.f; }
	~Camera() {}
	
	void project(float angle, float ratio, float zmin = 0.1f, float zmax = 1000.f);
	void view(const vec_t& eye, const vec_t& target, const vec_t& up);
	void view(photoModeParams_t &params);

	void tick(float aTimeEllapsed);
    void updateFPSMoves();
	void computeMatricesAndFrustums();
	
	// FPS mode sensibility
	float mSensibility;

	// cam props
	float mNear, mFar, mFov, mRatio;
	// matrices
	matrix_t mProjection, mView, mViewInverse, mProjectionInverse, mViewProj;
	ZFrustum mFrustum;

	matrix_t mPSSMProjection[PSSM_SPLIT_COUNT], mPSSMView;
	ZFrustum mPSSMFrustum[PSSM_SPLIT_COUNT];
	matrix_t mPSSMShadowMatAtlas[PSSM_SPLIT_COUNT];

    // high level stuff
    void lockOnMatrix( const matrix_t *pMat ) { mLockOnMatrix = pMat; }
    
    void BuildRay(int x, int y, int frameX, int frameY, vec_t &rayOrigin, vec_t &rayDir);
	// camera mode

	void SetCameraCustom();
	void SetCameraTrackObserve();
	void SetPreRace( bool bForced = false );
    GameShip *GetObservedShip() const { return mObservedShip; }
	void SetCameraEndRace(GameShip *pShp) { mObservedShip = pShp; mMode = CM_ENDRACE; mLockOnMatrix = NULL; }
	void SetSelectOneShip(GameShip *pShp) { mObservedShip = pShp; mMode = CM_SELONESHIP; mLockOnMatrix = NULL; }

	
	void SetCameraBehindShip(GameShip *pShp);
    void SetModeFPS() { mMode = CM_FPS; }
    CAMERA_MODE GetMode() const { return mMode; }


    // Targets
    // halfWidth/halfHeight are in clipspace (-1,1)
	// pos is transformed in clip space
    static bool IsBoxOnScreen(const matrix_t & viewproj, vec_t& pos, float halfWidth, float halfHeight)
	{
		vec_t targetPos = worldPosToClipSpace( viewproj, pos );

		float sizex = halfWidth;
		float sizey = halfHeight;
			
			
		if ( (targetPos.z < 0.f) || (targetPos.x<(-1.f-sizex*0.5f) ) || (targetPos.x> (1.f+sizex*0.5f)) ||
			(targetPos.y<(-1.f-sizey*0.5f) ) || (targetPos.y> (1.f+sizey*0.5f)) )
		{
			return false;
		}
		pos = targetPos;
		return true;
	}

    static vec_t worldPosToClipSpace(const matrix_t & viewproj, const vec_t& pos)
	{
		vec_t targetPos;
		targetPos.TransformPoint( pos, viewproj );

		targetPos.x /= targetPos.w;
		targetPos.y /= targetPos.w;

        return targetPos;
	}

    void SetRotatingSky( const matrix_t& rt) { rotatingSky = rt; }
protected:
	CAMERA_MODE mMode;

	GameShip *mObservedShip;
    
    // lock on matrix
    const matrix_t *mLockOnMatrix;

    // values camera behind ship
    bool mbLerpToShipUp;
    vec_t mCameraToShipUp;
    
    // local times
    float preRaceLocTime;
    
    // rotating view
    matrix_t rotatingSky;

	int mCreditMeshIndex;
	mesh_t *mCurrentCreditMesh;
	int IndexAIPoint;
    
    unsigned int miCameraForcedUpdate;
    
    // observing
    vec_t observeDir, observeSrc, observeDst, observeUp;
    float halfLocTime;
    
    // tool functions
    matrix_t buildCreditsCameraAlignedMatrix(const matrix_t &roadMat);

    // FPS mode
    
	void postTranslate(const vec_t & vt);
	void computeFPSView(float nXDiff, float nYDiff);

    // split computation
    void ComputeShadowMatrix( int iSplit, const matrix_t& psmView, const vec_t& frustMin, const vec_t& frustMax );
    void computeCascadedShadowInfos();
};

extern Camera camera;
extern Camera camera2;
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
