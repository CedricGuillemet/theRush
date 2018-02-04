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

#ifndef MESH_H__
#define MESH_H__

///////////////////////////////////////////////////////////////////////////////////////////////////

#define MAXTMPMESHBUFSIZE 65536

struct mesh_t;
class ZIndexArrayOGL;
class ZVertexArrayOGL;
struct Rope_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct meshVertex_t
{
	float x,y,z;
	float nx, ny, nz;
	void set(const vec_t& pos,const vec_t norm)
	{
		x = pos.x; y = pos.y; z = pos.z;
		nx = norm.x; ny = norm.y; nz = norm.z;
	}

	void setPosition(const vec_t& pos)
	{
		x = pos.x; y = pos.y; z = pos.z;
	}
	void setNormal(const vec_t& n)
	{
		nx = n.x; ny = n.y; nz = n.z;
	}

	vec_t getPosition() const
	{
		return vec(x, y, z);
	}

	void lerp(const meshVertex_t&v1, const meshVertex_t&v2, float t)
	{
		x = LERP(v1.x, v2.x, t);
		y = LERP(v1.y, v2.y, t);
		z = LERP(v1.z, v2.z, t);

		nx = LERP(v1.nx, v2.nx, t);
		ny = LERP(v1.ny, v2.ny, t);
		nz = LERP(v1.nz, v2.nz, t);

		float len = sqrtf(nx*nx +ny *ny + nz*nz);
		if (len> FLOAT_EPSILON)
		{
			len = 1.f/len;
			nx *= len;
			ny *= len;
			nz *= len;
		}
	}
	void TransformPoint(const vec_t& v, const matrix_t& mt)
	{
		((vec_t*)&x )->TransformPoint(v, mt);
	}

	void transform(const matrix_t& mt)
	{
		vec_t v = vec(x, y, z);
		vec_t n = vec(nx, ny, nz);
		v.TransformPoint(mt);
		n.TransformVector(mt);
		n.normalize();
		x = v.x;
		y = v.y;
		z = v.z;

		nx = n.x;
		ny = n.y;
		nz = n.z;
	}
} meshVertex_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct meshColorVertex_t : public meshVertex_t
{
public:
	uint32 color;
	void set(const vec_t& pos,const vec_t norm, uint32 acolor)
	{
		meshVertex_t::set(pos, norm);
		color = acolor;
	}

} meshColorVertex_t;


typedef struct meshColorUVVertex_t : public meshColorVertex_t
{
public:
	float u,v;
	void set(const vec_t& pos,const vec_t norm, uint32 acolor, vec_t uv)
	{
		meshColorVertex_t::set( pos, norm, acolor );
		u = uv.x;
		v = uv.y;
	}

} meshColorUVVertex_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct meshAnimation_t
{
	/*
	type:
		0 = none
		1 = repeat
		2 = ping pong
		3 = rand 01
		4 = rand threshold
	Interpolation:
		0 : linear
		1 : sinus
	*/
	union
	{
		unsigned int animType;
		struct
		{
            unsigned int mDrunkAnim : 1;
            unsigned int mEpitrochoid : 1;
			unsigned int mTranslationInterpolation : 1;
			unsigned int mTranslationType : 3;
			unsigned int mScalingInterpolation : 1;
			unsigned int mScalingType : 3;
			unsigned int mRotationInterpolation : 1;
			unsigned int mRotationType : 3;
			unsigned int mColorInterpolation : 1;
			unsigned int mColorType : 3;
		};
	};

    void setEpitrochoid( float r, float R, float d, const vec_t& center )
    {
        mDuration = 9999999.f;
        mEpitrochoid = true;
        mr = r;
        mR = R;
        md = d;
        mTranslationStart = center;
    }
    vec_t getEpiValue(float t)
    {
        vec_t res = mTranslationStart;
        float val = ( ( mR + mr ) / mr ) * t;
        res.x += ( mR + mr ) * cosf(t) - md * cosf( val );
        res.z += ( mR + mr ) * sinf(t) - md * sinf( val );
        return res;
    }

	float mLocalTime;
	float mDuration;
	vec_t mColorStart, mColorEnd;
	vec_t mTranslationStart, mTranslationEnd;
	vec_t mScalingStart, mScalingEnd;
	vec_t mRotationStart, mRotationEnd;
	float mThreshold;
    float mDrunkTranslateScale;
    float mr, mR, md;
    float mTimeFactor;
	bool mbForward;
	mesh_t *mMesh;

    void operator = (const meshAnimation_t& other)
    {
        animType = other.animType;
        mLocalTime = other.mLocalTime;
        mDuration = other.mDuration;
        mColorStart = other.mColorStart;
        mColorEnd = other.mColorEnd;
        mTranslationStart = other.mTranslationStart;
        mTranslationEnd = other.mTranslationEnd;
        mScalingStart = other.mScalingStart;
        mScalingEnd = other.mScalingEnd;
        mRotationStart = other.mRotationStart;
        mRotationEnd = other.mRotationEnd;
        mThreshold = other.mThreshold;
        mDrunkTranslateScale = other.mDrunkTranslateScale;
        mr = other.mr;
        mR = other.mR;
        md = other.md;
        mTimeFactor = other.mTimeFactor;
        mbForward = other.mbForward;
        //mMesh = other.mMesh;
    }

} meshAnimation_t;

meshAnimation_t *allocAnimation(mesh_t *mesh);
void freeAnimation(meshAnimation_t * pAnim);
void updateAnimations(float aTimeEllapsed);

///////////////////////////////////////////////////////////////////////////////////////////////////

void clearAnimations();
typedef struct mesh_t
{
private:
    void Init(ZIndexArrayOGL *IA, ZVertexArrayOGL *VA);

public:
	mesh_t();
    mesh_t(ZIndexArrayOGL *IA, ZVertexArrayOGL *VA);
	~mesh_t();

	ZIndexArrayOGL *mIA;
	ZVertexArrayOGL *mVA;

	// instancing
    mesh_t * clone() const;

	// linking
	mesh_t *mPrevious;
	mesh_t *mNext;
    void reattachToStackIndex( int idx );

    //
	int triCount;
	int mStackIndex;

	// props
	vec_t bSphere;
	vec_t worldBSphere;
	vec_t AABBMin, AABBMax;
	vec_t worldAABBMin, worldAABBMax;



	vec_t color;
	vec_t shader;// specular power, shininess factor


	matrix_t mBaseMatrix;
	matrix_t mLocalMatrix;
	matrix_t mWorldMatrix;

	// tools/methods
	void computeBSphere();
	mesh_t *clipPlane(const vec_t &plan);
    void updateWorldBSphere();


	// collision
	bool rayCast(vec_t& origin, vec_t&dir, float rayLength, vec_t& contactPoint, vec_t& contactNormal);

	// animation
	void removeAnimation()
	{
		ASSERT_GAME( mAnim == NULL || mAnim->mMesh == this );

		if (mAnim)
			freeAnimation(mAnim);

		mAnim = NULL;
	}
	meshAnimation_t * addAnimation()
	{
		removeAnimation();

		mAnim = allocAnimation(this);
		ASSERT_GAME( mAnim == NULL || mAnim->mMesh == this );

		return mAnim;
	}

	const meshAnimation_t* getAnimation() const { return mAnim; }
	void setAnimation(meshAnimation_t* _anim) { mAnim = _anim; }

    // props
    Rope_t *mRope;
    u8 screenMesh;// 0 none.

    u8 visible:1;
    u8 physic:1;
    u8 mbTransparent:1;
    u8 mbIsIceGround:1;
    u8 onlyVisibleForShadows:1;
    u8 mbIsRock:1;
    u8 mbIsSand:1;
    u8 mbCastShadows:1;
    u8 mbIsAurora:1;

    void createScreenMesh( const vec_t& uvs );
    void setBBox( const vec_t &AABBMin, const vec_t& AABBMax);
protected:
    // animation
    meshAnimation_t *mAnim;

    void removeLinks();
    void addLinks( int idx );


} mesh_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

mesh_t* generateText(const char *szText);
mesh_t* generateMeshFromVolume(bool *pVol, int x, int y, int z);
bool*   generateVolumeFromText(const char *szText, int &x, int &y, int &z);
mesh_t *generateVoxelMesh(uint32 *pCols, int x, int y, int z);

mesh_t* generateBox();
mesh_t* generateBoxOnGround(bool hasLowerFace = true);
mesh_t* generateBoxFrustum(bool hasLowerFace = true);
mesh_t* generateBoxFrustum(vec_t vts[8]);

mesh_t* generateCylinder(int nbSides, bool bSmooth = true, int bypassStart = 9999, int bypassCount = 0, int bypassStart2 = 9999, int bypassCount2 = 0);
mesh_t* generateCone(int nbSides, int bypassStart = 9999, int bypassCount = 0, int bypassStart2 = 9999, int bypassCount2 = 0);
mesh_t* generateLightCone(int nbSides);
mesh_t* generateSuper( float bx, float by, float bz,
					  int level,
					  bool bSimx = true, bool bSimy = true, bool bSimz = true, bool bSmoothedNormals = false );
mesh_t* generateHexagon(float width, float height, float lateral);

mesh_t* generateReactor( const matrix_t &orient, float radius );
void drawMesh(mesh_t *pMesh);

mesh_t *pushNewMesh( int VAF = 5/*VAF_XYZ|VAF_NORMAL*/ );


void bendMesh(mesh_t *pMesh, float desiredAngle, int axeLen, int axeRot);
void transformMesh(mesh_t *pMesh, const matrix_t& mt);
mesh_t * merge(mesh_t *pBaseMesh, mesh_t *pOther);
mesh_t * merge(mesh_t **pBaseMesh, int nbMeshes);
mesh_t * merge(const std::list<mesh_t*>& meshes);
void deleteMeshes(std::list<mesh_t*>& meshes);
void taper(mesh_t *pMesh, const vec_t& decal, int axe);
void squiz(mesh_t *pMesh, const vec_t& ratio, int axe);

void pushNewMeshInit();
void pushTrunk( const matrix_t & m1, const matrix_t & m2, bool bEndCapped = false );

mesh_t* generateDelaunay(vec_t *vts, int nbVts);
int pushDelaunayIndices(u8 *vts, int nbVts, int nvStride, unsigned short *pTris, int &tav, int indexOffset = 0, bool inverseWinding = false);
mesh_t *pushNewMesh(unsigned short *pInd, int nbInd, meshVertex_t *pVt, int nbVt);
mesh_t *pushNewMesh(unsigned short *pInd, int nbInd, void *pVt, int VAF, int nbVt);
mesh_t *pushNewMesh(uint32 *pInd, int nbInd, meshVertex_t *pVt, int nbVt);
void createScreenMesh();
mesh_t* generateCity(  );
mesh_t* generateCityLowPoly();
mesh_t *generatePipe( vec_t *shape, int nbShape, float sliceHeight, float bevel, float radius, int nGons, u32 color, bool bForRope = false );

mesh_t *generateAurora( vec_t* points, int nbPoints, int nbPointsPerSegments, float height );
mesh_t *BuildAuroraBorealis( const vec_t& source, float maxAmp, float length, float height, int nbPoints );

///////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_MESH_STACK 10

typedef struct meshesStackStrap_t
{
	meshesStackStrap_t()
	{
		first = last = NULL;
		mGroundPhysicMesh = mWallPhysicMesh = NULL;
        mMeshCount = 0;
	}
	mesh_t* first;
	mesh_t* last;

	mesh_t* mGroundPhysicMesh;
	mesh_t* mWallPhysicMesh;

    unsigned int mMeshCount;
} meshesStackStrap_t;

extern meshesStackStrap_t GMeshes[MAX_MESH_STACK];

void PushMeshStack();
void PopMeshStack();
int GetMeshStack();
void ClearMeshStack();
void SetPhysicMeshOnStack(mesh_t* aGroundPhysicMesh, mesh_t* aWallPhysicMesh);


#endif
