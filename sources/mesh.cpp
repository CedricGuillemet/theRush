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
#include "mesh.h"

#include "maths.h"
#include "physics.h"
#include "world.h"
#include "render.h"
#include "content.h"
#include "gui.h"
#include "fx.h"

#include "include_GL.h"

//#include "Datas/city_bin.h"

void recomputeNormals(mesh_t *pMesh);

int Delaunay(int N, u8 *vts, int vtStride, int& numTriangles, TriangleList*& triangle)
{
    int result;

    double** point = new double*[N+3];
    int i;
    for (i = 0; i < N+3; i++)
        point[i] = new double[2];
    for (i = 0; i < N; i++)
    {
		vec_t *pv = (vec_t*)vts;
        point[i][0] = pv->x;
        point[i][1] = pv->y;
		vts += vtStride;
    }

    const double DEPSILON = 0.00001;
    const int TSIZE = 75;
    const double RANGE = 10.0;

    double xmin = point[0][0], xmax = xmin;
    double ymin = point[0][1], ymax = ymin;

    for (i = 1; i < N; i++)
    {
        double value = point[i][0];
        if ( xmax < value )
            xmax = value;
        if ( xmin > value )
            xmin = value;

        value = point[i][1];
        if ( ymax < value )
            ymax = value;
        if ( ymin > value )
            ymin = value;
    }

    double xrange = xmax-xmin, yrange = ymax-ymin;
    double maxrange = xrange;
    if ( maxrange < yrange )
        maxrange = yrange;

    // need to scale the data later to do a correct triangle count
    double maxrange2 = maxrange*maxrange;

    // tweak the points by very small random numbers
    double bgs = DEPSILON*maxrange;
    srand(367);
    for (i = 0; i < N; i++)
    {
        point[i][0] += bgs*(0.5-rand()/double(RAND_MAX));
        point[i][1] += bgs*(0.5-rand()/double(RAND_MAX));
    }

    double wrk[2][3] =
    {
        { 5*RANGE, -RANGE, -RANGE },
        { -RANGE, 5*RANGE, -RANGE }
    };
    for (i = 0; i < 3; i++)
    {
        point[N+i][0] = xmin+xrange*wrk[0][i];
        point[N+i][1] = ymin+yrange*wrk[1][i];
    }

    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i11;
    int nts, ii[3];
    double xx;

    int tsz = 2*TSIZE;
    int** tmp = new int*[tsz+1];
    tmp[0] = new int[2*(tsz+1)];
    for (i0 = 1; i0 < tsz+1; i0++)
        tmp[i0] = tmp[0] + 2*i0;
    i1 = 2*(N + 2);

    int* id = new int[i1];
    for (i0 = 0; i0 < i1; i0++)
        id[i0] = i0;

    int** a3s = new int*[i1];
    a3s[0] = new int[3*i1];
    for (i0 = 1; i0 < i1; i0++)
        a3s[i0] = a3s[0] + 3*i0;
    a3s[0][0] = N;
    a3s[0][1] = N+1;
    a3s[0][2] = N+2;

    double** ccr = new double*[i1];  // circumscribed centers and radii
    ccr[0] = new double[3*i1];
    for (i0 = 1; i0 < i1; i0++)
        ccr[i0] = ccr[0] + 3*i0;
    ccr[0][0] = 0.0;
    ccr[0][1] = 0.0;
    ccr[0][2] = +9999999999.;//+NAN;// FLT_MAX;

    nts = 1;  // number of triangles
    i4 = 1;

    // compute triangulation
    for (i0 = 0; i0 < N; i0++)
    {
        i1 = i7 = -1;
        i9 = 0;
        for (i11 = 0; i11 < nts; i11++)
        {
            i1++;
            while ( a3s[i1][0] < 0 )
                i1++;
            xx = ccr[i1][2];
            for (i2 = 0; i2 < 2; i2++)
            {
                double z = point[i0][i2]-ccr[i1][i2];
                xx -= z*z;
                if ( xx < 0 )
                    goto Corner3;
            }
            i9--;
            i4--;
            id[i4] = i1;
            for (i2 = 0; i2 < 3; i2++)
            {
                ii[0] = 0;
                if (ii[0] == i2)
                    ii[0]++;
                for (i3 = 1; i3 < 2; i3++)
                {
                    ii[i3] = ii[i3-1] + 1;
                    if (ii[i3] == i2)
                        ii[i3]++;
                }
                if ( i7 > 1 )
                {
                    i8 = i7;
                    for (i3 = 0; i3 <= i8; i3++)
                    {
                        for (i5 = 0; i5 < 2; i5++)
                            if ( a3s[i1][ii[i5]] != tmp[i3][i5] )
                                goto Corner1;
                        for (i6 = 0; i6 < 2; i6++)
                            tmp[i3][i6] = tmp[i8][i6];
                        i7--;
                        goto Corner2;
                    Corner1:;
                    }
                }
                if ( ++i7 > tsz )
                {
                    // temporary storage exceeded, increase TSIZE
                    result = 0;
                    goto ExitDelaunay;
                }
                for (i3 = 0; i3 < 2; i3++)
                    tmp[i7][i3] = a3s[i1][ii[i3]];
            Corner2:;
            }
            a3s[i1][0] = -1;
        Corner3:;
        }

        for (i1 = 0; i1 <= i7; i1++)
        {
            for (i2 = 0; i2 < 2; i2++)
                for (wrk[i2][2] = 0, i3 = 0; i3 < 2; i3++)
                {
                    wrk[i2][i3] = point[tmp[i1][i2]][i3]-point[i0][i3];
                    wrk[i2][2] +=
                    0.5*wrk[i2][i3]*(point[tmp[i1][i2]][i3]+point[i0][i3]);
                }

            xx = wrk[0][0]*wrk[1][1]-wrk[1][0]*wrk[0][1];
            ccr[id[i4]][0] = (wrk[0][2]*wrk[1][1]-wrk[1][2]*wrk[0][1])/xx;
            ccr[id[i4]][1] = (wrk[0][0]*wrk[1][2]-wrk[1][0]*wrk[0][2])/xx;

            for (ccr[id[i4]][2] = 0, i2 = 0; i2 < 2; i2++)
            {
                double z = point[i0][i2]-ccr[id[i4]][i2];
                ccr[id[i4]][2] += z*z;
                a3s[id[i4]][i2] = tmp[i1][i2];
            }

            a3s[id[i4]][2] = i0;
            i4++;
            i9++;
        }
        nts += i9;
    }

    // count the number of triangles
    numTriangles = 0;
    i0 = -1;
    for (i11 = 0; i11 < nts; i11++)
    {
        i0++;
        while ( a3s[i0][0] < 0 )
            i0++;
        if ( a3s[i0][0] < N )
        {
            for (i1 = 0; i1 < 2; i1++)
                for (i2 = 0; i2 < 2; i2++)
                    wrk[i1][i2] =
                    point[a3s[i0][i1]][i2]-point[a3s[i0][2]][i2];

            xx = wrk[0][0]*wrk[1][1]-wrk[0][1]*wrk[1][0];

            if ( fabs(xx) > DEPSILON*maxrange2 )
                numTriangles++;
        }
    }

    // create the triangles
    triangle = new TriangleList[numTriangles];

    numTriangles = 0;
    i0 = -1;
    for (i11 = 0; i11 < nts; i11++)
    {
        i0++;
        while ( a3s[i0][0] < 0 )
            i0++;
        if ( a3s[i0][0] < N )
        {
            for (i1 = 0; i1 < 2; i1++)
                for (i2 = 0; i2 < 2; i2++)
                    wrk[i1][i2] =
                    point[a3s[i0][i1]][i2]-point[a3s[i0][2]][i2];

            xx = wrk[0][0]*wrk[1][1]-wrk[0][1]*wrk[1][0];

            if ( fabs(xx) > DEPSILON*maxrange2 )
            {
                int delta = xx < 0 ? 1 : 0;
                TriangleList& tri = triangle[numTriangles];
                tri[0] = a3s[i0][0];
                tri[1] = a3s[i0][1+delta];
                tri[2] = a3s[i0][2-delta];
                numTriangles++;
            }
        }
    }

    //delete [] triangle;

    result = 1;

ExitDelaunay:;
    delete[] tmp[0];
    delete[] tmp;
    delete[] id;
    delete[] a3s[0];
    delete[] a3s;
    delete[] ccr[0];
    delete[] ccr;
    for (i = 0; i < N+3; i++)
        delete[] point[i];
    delete[] point;

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define MAXANIMATIONS 1024
meshAnimation_t Animations[MAXANIMATIONS];
int AnimationCount = 0;

meshAnimation_t *allocAnimation(mesh_t *mesh)
{
    ASSERT_GAME( AnimationCount <= MAXANIMATIONS );

    meshAnimation_t *nAnim = NULL;

	if (AnimationCount < MAXANIMATIONS)
    {
        nAnim = &Animations[AnimationCount];
        AnimationCount++;

        nAnim->mLocalTime = 0.f;
        nAnim->animType = 0;
        nAnim->mbForward = true;
        nAnim->mMesh = mesh;
        nAnim->mDrunkTranslateScale = 1.f;
        nAnim->mTimeFactor = 1.f;
    }

    ASSERT_GAME( nAnim != NULL );

	return nAnim;
}

void freeAnimation(meshAnimation_t * pAnim)
{
	ASSERT_GAME( AnimationCount > 0 );

	ASSERT_GAME( pAnim != NULL );
    ASSERT_GAME( debug_IsIndexInRange( debug_GetElementIndexInArray(pAnim, Animations), 0, AnimationCount-1 ) );
	ASSERT_GAME( pAnim == pAnim->mMesh->getAnimation() );

	const meshAnimation_t& lastAnim = Animations[AnimationCount-1];
	ASSERT_GAME( lastAnim.mMesh->getAnimation() == &lastAnim );

	if (pAnim != &lastAnim)
	{
		*pAnim = lastAnim;
		pAnim->mMesh = lastAnim.mMesh;

		pAnim->mMesh->setAnimation(pAnim);
	}

	AnimationCount --;

	ASSERT_GAME( pAnim == pAnim->mMesh->getAnimation() );
}

inline vec_t AnimationInterpolate(const vec_t& start, const vec_t& end, float ratio, float threshold, bool forward, int type, bool isSinus)
{
	vec_t res;
	/*
	float edge0 = -0.1f;
	float edge1 = 1.1f;
	ratio = Clamp( ((ratio - edge0)/(edge1 - edge0)) , 0.f, 1.f);
	*/
    // Evaluate polynomial

	float x = ratio;

	float localRatio = isSinus? (x*x*x*(x*(x*6.f - 15.f) + 10.f)):ratio;// (sinf(ratio*PI-PI*0.5f)*0.5f+0.5f): ratio;
	switch (type)
	{
	case 1:
		res.lerp(start, end, localRatio);
		break;
	case 2:
		res.lerp(start, end, forward?ratio:(1.f-localRatio) );
		break;
	case 3:
		res.lerp(start, end, r01());
		break;
	case 4:
		res =  (r01()>threshold)?start:end;
		break;
    default:
        res = vec(0.f);
        break;
	}
	return res;
}

void updateAnimations(float aTimeEllapsed)
{
	PROFILER_START(Animations);
	aTimeEllapsed *= world.GetGameSpeed();

	meshAnimation_t *pa = Animations;
	for (int i=0;i<AnimationCount;i++)
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
		/*
	unsigned int mTranslationInterpolation : 1;
	unsigned int mTranslationType : 3;
	unsigned int mScalingInterpolation : 1;
	unsigned int mScalingType : 3;
	unsigned int mRotationInterpolation : 1;
	unsigned int mRotationType : 3;
	unsigned int mColorInterpolation : 1;
	unsigned int mColorType : 3;

	float mLocalTime;
	float mDuration;
	vec_t mColorStart, mColorEnd;
	vec_t mTranslationStart, mTranslationEnd;
	vec_t mScalingStart, mScalingEnd;
	vec_t mRotationStart, mRotationEnd;
	float mThreshold;
	*/
		pa->mLocalTime += aTimeEllapsed * pa->mTimeFactor;
		if (pa->mLocalTime > pa->mDuration)
		{
			pa->mLocalTime -= pa->mDuration;
			pa->mbForward = !pa->mbForward;
		}

		float ratio = pa->mLocalTime / pa->mDuration;

		matrix_t tmpMat, resMat;

        if ( pa->mEpitrochoid )
        {
            resMat.LookAt( pa->getEpiValue( pa->mLocalTime ), pa->getEpiValue( pa->mLocalTime + 0.01f*pa->mTimeFactor ), vec( 0.f, 1.f, 0.f ) );
			pa->mMesh->mLocalMatrix = resMat;
			pa->mMesh->mWorldMatrix = pa->mMesh->mBaseMatrix * resMat;
			pa->mMesh->updateWorldBSphere();
        }

        if (pa->mDrunkAnim)
        {
            //pa->mLocalTime += aTimeEllapsed*2.f;
            float drunkTime = pa->mLocalTime;
            matrix_t drunkOffset;
            vec_t decalValue =  vec( cosf( drunkTime * 0.422f ) + sinf( drunkTime * 0.666f ), cosf( drunkTime * 0.353f ) + sinf( drunkTime * 0.125f ), cosf( drunkTime * 0.23f ) + sinf( drunkTime * 0.8f ) );
            vec_t rotateValue =  vec( cosf( drunkTime * 0.422f ) + sinf( drunkTime * 0.666f ), cosf( drunkTime * 0.353f ) + sinf( drunkTime * 0.125f ), cosf( drunkTime * 0.23f ) + sinf( drunkTime * 0.8f ) );
            rotateValue *= 0.01f;
            drunkOffset.translation( decalValue * 0.05f * pa->mDrunkTranslateScale );
            matrix_t drunkRotate1, drunkRotate2;
            drunkRotate1.rotationYawPitchRoll( rotateValue.x, rotateValue.y, rotateValue.z );

            resMat = drunkRotate1 * drunkOffset;

			pa->mMesh->mLocalMatrix = resMat;
			pa->mMesh->mWorldMatrix = pa->mMesh->mBaseMatrix * resMat;
			pa->mMesh->updateWorldBSphere();
        }
		if (pa->mTranslationType)
		{
			vec_t res = AnimationInterpolate(pa->mTranslationStart, pa->mTranslationEnd, ratio, pa->mThreshold, pa->mbForward, pa->mTranslationType, pa->mTranslationInterpolation);
			tmpMat.translation(res);
			resMat = tmpMat;
		}

		if (pa->mRotationType)
		{
			vec_t res = AnimationInterpolate(pa->mRotationStart, pa->mRotationEnd, ratio, pa->mThreshold, pa->mbForward, pa->mRotationType, pa->mRotationInterpolation);
			tmpMat.rotationYawPitchRoll(res.x, res.y, res.z);
			if (pa->mTranslationType)
				resMat *= tmpMat;
			else
				resMat = tmpMat;
		}

		if (pa->mScalingType)
		{
			vec_t res = AnimationInterpolate(pa->mScalingStart, pa->mScalingEnd, ratio, pa->mThreshold, pa->mbForward, pa->mScalingType, pa->mScalingInterpolation);
			tmpMat.scale(res);
			if (pa->mTranslationType || pa->mRotationType)
				resMat *= tmpMat;
			else
				resMat = tmpMat;
		}

        if (pa->mTranslationType || pa->mScalingType || pa->mRotationType /* || pa->mDrunkAnim || pa->mEpitrochoid*/ )
		{
			pa->mMesh->mLocalMatrix = resMat;
			pa->mMesh->mWorldMatrix = resMat * pa->mMesh->mBaseMatrix;
			pa->mMesh->updateWorldBSphere();
		}
		if (pa->mColorType)
		{
			pa->mMesh->color = AnimationInterpolate(pa->mColorStart, pa->mColorEnd, ratio, pa->mThreshold, pa->mbForward, pa->mColorType, pa->mColorInterpolation);
		}
		pa ++;
	}
	PROFILER_END(); // Animations
}

void clearAnimations() { AnimationCount = 0; }

///////////////////////////////////////////////////////////////////////////////////////////////////

int GMeshStackIndex = 0;
meshesStackStrap_t GMeshes[MAX_MESH_STACK];

int GetMeshStack()
{
	return GMeshStackIndex;
}
void PushMeshStack()
{
	if ((GMeshStackIndex +1) == MAX_MESH_STACK)
		return;
	GMeshStackIndex ++;
}

void ClearMeshStack()
{
	meshesStackStrap_t & strap = GMeshes[GMeshStackIndex];
	mesh_t *td = strap.first;
	while(td)
	{
        ASSERT_GAME( td->mStackIndex == GMeshStackIndex );

		mesh_t *td2 = td;
		td = td->mNext;

        //FIXME: There are probably many objects still referencing some meshes
        //those objects must be reset accordingly
		delete td2;
		td2 = NULL;
	}
    ASSERT_GAME( strap.first == NULL );
    ASSERT_GAME( strap.last == NULL );
    strap.first = strap.last = NULL;

    ASSERT_GAME ( (!strap.mMeshCount) );
	physicWorld.ClearTrack();
}


void SetPhysicMeshOnStack(mesh_t* aGroundPhysicMesh, mesh_t* aWallPhysicMesh)
{
	meshesStackStrap_t & strap = GMeshes[GMeshStackIndex];

	ASSERT_GAME( (!strap.mGroundPhysicMesh) );
	ASSERT_GAME( (!strap.mWallPhysicMesh) );

	strap.mGroundPhysicMesh = aGroundPhysicMesh;
	strap.mWallPhysicMesh = aWallPhysicMesh;
}

void PopMeshStack()
{
	ClearMeshStack();
	GMeshStackIndex--;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

mesh_t::mesh_t()
{
    ZIndexArrayOGL* newIA = new ZIndexArrayOGL;
    ZVertexArrayOGL* newVA = new ZVertexArrayOGL;

    Init(newIA, newVA);
}

mesh_t::mesh_t(ZIndexArrayOGL *IA, ZVertexArrayOGL *VA)
{
    ASSERT_GAME( IA != NULL );
    ASSERT_GAME( VA != NULL );

    Init(IA, VA);

    IA->IncRef();
    VA->IncRef();
}

void mesh_t::Init(ZIndexArrayOGL *IA, ZVertexArrayOGL *VA)
{
    mIA = IA;
    mVA = VA;

    mPrevious = NULL;
    mNext = NULL;

    mAnim = NULL;
    mBaseMatrix.identity();
    mLocalMatrix.identity();
    mWorldMatrix.identity();

    mbTransparent = false;
    mbIsIceGround = false;
    onlyVisibleForShadows = false;
    //mbIsAdvertising = false;
    mbIsRock = false;
    mbIsSand = false;
    mRope = NULL;
    screenMesh = false;
    mbCastShadows = true;
    mbIsAurora = false;

    // linking

    addLinks( GMeshStackIndex );

    shader = vec(0.2f, 0.6f, 0.f, 0.f);
}

void mesh_t::removeLinks()
{
    ASSERT_GAME( 0 <= mStackIndex && mStackIndex < MAX_MESH_STACK );
    ASSERT_GAME( mStackIndex <= GMeshStackIndex );

	if (mNext)
		mNext->mPrevious = mPrevious;
	if (mPrevious)
		mPrevious->mNext = mNext;

	meshesStackStrap_t &strap = GMeshes[mStackIndex];
    strap.mMeshCount --;

	if (strap.first == this)
		strap.first = mNext;
	if (strap.last == this)
		strap.last = mPrevious;

    ASSERT_GAME( mPrevious == NULL || mPrevious->mNext == mNext );
    ASSERT_GAME( mNext == NULL || mNext->mPrevious == mPrevious );

    mPrevious = NULL;
    mNext = NULL;
}

void mesh_t::addLinks( int idx )
{
    ASSERT_GAME( 0 <= idx && idx < MAX_MESH_STACK );
    ASSERT_GAME( idx <= GMeshStackIndex );

    mStackIndex = idx;

    meshesStackStrap_t & strap = GMeshes[mStackIndex];

    strap.mMeshCount ++;
	mPrevious = strap.last;
	mNext = NULL;

	if (strap.last)
		strap.last->mNext = this;

	strap.last = this;

	if (!strap.first)
		strap.first = this;

    ASSERT_GAME( mPrevious == NULL || mPrevious->mNext == this );
}

void mesh_t::reattachToStackIndex( int idx )
{
    if ( mStackIndex == idx)
        return;

    removeLinks();
    addLinks( idx );
}

mesh_t * mesh_t::clone() const
{
	mesh_t *nm = new mesh_t(mIA, mVA);

    ASSERT_GAME( nm->mIA == mIA );
    ASSERT_GAME( nm->mVA == mVA );

    nm->bSphere = bSphere;
	nm->worldBSphere = worldBSphere;
	nm->AABBMin = AABBMin;
    nm->AABBMax = AABBMax;
	nm->worldAABBMin = worldAABBMin;
    nm->worldAABBMax = worldAABBMax;

    nm->triCount = triCount;

	nm->color = color;
	nm->shader = shader;// specular power, shininess factor

	nm->visible = visible;
    nm->physic = physic;
    nm->mbTransparent = mbTransparent;
    nm->mbIsIceGround = mbIsIceGround;

	nm->mBaseMatrix = mBaseMatrix;
	nm->mLocalMatrix = mLocalMatrix;
	nm->mWorldMatrix = mWorldMatrix;

    nm->mStackIndex = GMeshStackIndex;

	nm->mAnim = NULL;

	return nm;
}

mesh_t::~mesh_t()
{
    mIA->DecRef();
    mIA = NULL;
    mVA->DecRef();
    mVA = NULL;

    removeAnimation();
    mAnim = NULL;

    removeLinks();
    ASSERT_GAME( mPrevious == NULL && mNext == NULL );
}

//extern unsigned char  A8X8FONT_RAW_DATA [16384];

#define MAXTMPMESHBUFSIZE 65536
meshVertex_t mVts[MAXTMPMESHBUFSIZE];
int vtav;
unsigned short tris[MAXTMPMESHBUFSIZE];
int triav;

void pushTextVertex(const vec_t& pos1, const vec_t& pos2,
					const vec_t& pos3, const vec_t& pos4, const vec_t& norm)
{
	mVts[vtav++].set(pos1, norm);
	mVts[vtav++].set(pos2, norm);
	mVts[vtav++].set(pos3, norm);
	mVts[vtav++].set(pos4, norm);
}

void pushVertex(const vec_t& pos1, const vec_t& pos2,
					const vec_t& pos3, const vec_t& pos4,
					const vec_t& norm1, const vec_t& norm2,
					const vec_t& norm3, const vec_t& norm4 )
{
	mVts[vtav++].set(pos1, norm1);
	mVts[vtav++].set(pos2, norm2);
	mVts[vtav++].set(pos3, norm3);
	mVts[vtav++].set(pos4, norm4);
}


void pushColoredVertex(const vec_t& pos1, const vec_t& pos2,
					const vec_t& pos3, const vec_t& pos4, const vec_t& norm, uint32 color)
{
	meshColorVertex_t *pcv = (meshColorVertex_t *)mVts;
	pcv[vtav++].set(pos1, norm, color);
	pcv[vtav++].set(pos2, norm, color);
	pcv[vtav++].set(pos3, norm, color);
	pcv[vtav++].set(pos4, norm, color);
}

void pushColoredVertex(const vec_t& pos1, const vec_t& pos2,
					const vec_t& pos3, const vec_t& norm, uint32 color)
{
	meshColorVertex_t *pcv = (meshColorVertex_t *)mVts;
	pcv[vtav++].set(pos1, norm, color);
	pcv[vtav++].set(pos2, norm, color);
	pcv[vtav++].set(pos3, norm, color);
}

void pushTextQuad(int subLocal, bool bInvert)
{
	if (bInvert)
	{
        tris[triav++] = static_cast<unsigned short>(subLocal+0);
		tris[triav++] = static_cast<unsigned short>(subLocal+1);
		tris[triav++] = static_cast<unsigned short>(subLocal+2);

		tris[triav++] = static_cast<unsigned short>(subLocal+0);
		tris[triav++] = static_cast<unsigned short>(subLocal+2);
		tris[triav++] = static_cast<unsigned short>(subLocal+3);

	}
	else
	{
		tris[triav++] = static_cast<unsigned short>(subLocal+2);
		tris[triav++] = static_cast<unsigned short>(subLocal+1);
		tris[triav++] = static_cast<unsigned short>(subLocal+0);

		tris[triav++] = static_cast<unsigned short>(subLocal+3);
		tris[triav++] = static_cast<unsigned short>(subLocal+2);
		tris[triav++] = static_cast<unsigned short>(subLocal+0);
	}
}


void pushTextTri(int subLocal, bool bInvert)
{
	if (bInvert)
	{
		tris[triav++] = static_cast<unsigned short>(subLocal+0);
		tris[triav++] = static_cast<unsigned short>(subLocal+1);
		tris[triav++] = static_cast<unsigned short>(subLocal+2);
	}
	else
	{
		tris[triav++] = static_cast<unsigned short>(subLocal+2);
		tris[triav++] = static_cast<unsigned short>(subLocal+1);
		tris[triav++] = static_cast<unsigned short>(subLocal+0);
	}
}

mesh_t *pushNewMesh( int VAF )
{
	mesh_t* nMesh = new mesh_t;


	nMesh->mIA->Init( triav, VAU_STATIC );
	nMesh->mVA->Init( VAF, vtav, true, VAU_STATIC );

	nMesh->triCount = triav;
//	nMesh->vtCount = vtav;

	memcpy( nMesh->mVA->Lock(VAL_WRITE), mVts,  nMesh->mVA->GetVertexSize() * vtav);
	memcpy( nMesh->mIA->Lock(VAL_WRITE), tris, sizeof(unsigned short) * triav);
	nMesh->mIA->Unlock();
	nMesh->mVA->Unlock();
	nMesh->bSphere = vec(0.f, 0.f, 0.f, 0.5f * SQRT2);
	nMesh->worldAABBMin = nMesh->AABBMin = vec(-0.5f, -0.5f, -0.5f, 0.f);
	nMesh->worldAABBMax = nMesh->AABBMax = vec( 0.5f,  0.5f,  0.5f, 0.f);
	return nMesh;
}


mesh_t *pushNewMesh(uint32 *pInd, int nbInd, meshVertex_t *pVt, int nbVt)
{
	mesh_t* nMesh = new mesh_t;

	nMesh->mIA->Init( nbInd, VAU_STATIC, false );
	nMesh->mVA->Init( VAF_XYZ|VAF_NORMAL, nbVt, true, VAU_STATIC );

	nMesh->triCount = nbInd;
//	nMesh->vtCount = nbVt;

	memcpy( nMesh->mVA->Lock(VAL_WRITE), pVt,  nMesh->mVA->GetVertexSize() * nbVt);
	memcpy( nMesh->mIA->Lock(VAL_WRITE), pInd, nMesh->mIA->GetElementSize() * nbInd);
	nMesh->mIA->Unlock();
	nMesh->mVA->Unlock();

	return nMesh;
}

mesh_t *pushNewMesh(unsigned short *pInd, int nbInd, meshVertex_t *pVt, int nbVt)
{
	mesh_t* nMesh = new mesh_t;

	nMesh->mIA->Init( nbInd, VAU_STATIC );
	nMesh->mVA->Init( VAF_XYZ|VAF_NORMAL, nbVt, true, VAU_STATIC );

	nMesh->triCount = nbInd;
//	nMesh->vtCount = nbVt;

	memcpy( nMesh->mVA->Lock(VAL_WRITE), pVt,  nMesh->mVA->GetVertexSize() * nbVt);
	memcpy( nMesh->mIA->Lock(VAL_WRITE), pInd, nMesh->mIA->GetElementSize() * nbInd);
	nMesh->mIA->Unlock();
	nMesh->mVA->Unlock();

	return nMesh;
}

mesh_t *pushNewMesh(unsigned short *pInd, int nbInd, void *pVt, int VAF, int nbVt)
{
	mesh_t* nMesh = new mesh_t;

	nMesh->mIA->Init( nbInd, VAU_STATIC );
	nMesh->mVA->Init( VAF, nbVt, true, VAU_STATIC );

	nMesh->triCount = nbInd;
//	nMesh->vtCount = nbVt;

	memcpy( nMesh->mVA->Lock(VAL_WRITE), pVt,  nMesh->mVA->GetVertexSize() * nbVt);
	memcpy( nMesh->mIA->Lock(VAL_WRITE), pInd, nMesh->mIA->GetElementSize() * nbInd);
	nMesh->mIA->Unlock();
	nMesh->mVA->Unlock();

	return nMesh;
}

mesh_t* generateText(const char *szText)
{
	int x,y,z;

	bool* pb = generateVolumeFromText(szText, x, y, z);
	mesh_t* res = generateMeshFromVolume(pb, x, y, z);

	delete [] pb;
	pb = NULL;

	res->computeBSphere();
	return res;
}

struct batPoint_t
{
    vec_t point;
    vec_t segmentNormal;
    vec_t pointNormal;
    float length;
};

void pushBatWall( const std::vector<batPoint_t> & pts, int& sublocal, u32 col, float h1, float h2, float d1, float d2)
{
    int nbpt = pts.size();
    for (int i = 0;i<nbpt;i++)
    {
        int nextp = (i+1)%nbpt;
        const batPoint_t& b1 = pts[i];
        const batPoint_t& b2 = pts[nextp];

        vec_t pt1 = vec( b1.point.x, h1 + 10.f, b1.point.z) - b1.pointNormal * d1;
        vec_t pt2 = vec( b2.point.x, h1 + 10.f, b2.point.z) - b2.pointNormal * d1;
        vec_t pt3 = vec( b2.point.x, h2 + 10.f, b2.point.z) - b2.pointNormal * d2;
        vec_t pt4 = vec( b1.point.x, h2 + 10.f, b1.point.z) - b1.pointNormal * d2;

        vec_t nd1 = normalized( pt2 - pt1 );
        vec_t nd2 = normalized( pt4 - pt1 );

        vec_t norm;
        norm.cross( nd1, nd2 );
        norm.normalize();

        pushColoredVertex( pt1, pt2, pt3, pt4, norm, col );


        pushTextQuad( sublocal, true );
        sublocal+= 4;
    }
}

void pushBatCapping( const std::vector<batPoint_t> & pts, int& sublocal, u32 col, float h, float d)
{
    int nbpt = pts.size();
    for (int i = 0;i<nbpt-2;i++)
    {
        int nextp = (i+1)%nbpt;
        int nextp2 = (i+2)%nbpt;

        const batPoint_t& b1 = pts[0];
        const batPoint_t& b2 = pts[nextp];
        const batPoint_t& b3 = pts[nextp2];

        vec_t pt1 = vec( b1.point.x, h + 10.f, b1.point.z ) - b1.pointNormal * d;
        vec_t pt2 = vec( b2.point.x, h + 10.f, b2.point.z ) - b2.pointNormal * d;
        vec_t pt3 = vec( b3.point.x, h + 10.f, b3.point.z ) - b3.pointNormal * d;

        pushColoredVertex( pt1, pt2, pt3, vec( 0.f, 1.f, 0.f ), col );

        pushTextTri( sublocal, true );
        sublocal+= 3;
    }
}

void pushBatOrnament( const std::vector<batPoint_t> & pts, int& sublocal, u32 col, int idx, float h1, float h2, float frontDistance, float backDistance, float sideWidth)
{

    vec_t right;
    right.cross( pts[idx].pointNormal, vec(0.f, 1.f, 0.f ) );
    right.normalize();
    const vec_t& dir = pts[idx].pointNormal;

    //static const float ornamentWidth = 0.03f;
	vec_t vts[8];
	vts[0] = (right * -sideWidth) + vec(0.f, h1, 0.f) + (dir * -backDistance);
	vts[1] = (right * -sideWidth) + vec(0.f,  h1, 0.f) + (dir *  frontDistance);
	vts[2] = (right * sideWidth) + vec(0.f,  h1, 0.f) + (dir *  -backDistance);
	vts[3] = (right * sideWidth) + vec(0.f,  h1, 0.f) + (dir *  frontDistance);
	vts[4] = (right * -sideWidth) + vec(0.f,  h2, 0.f) + (dir *  -backDistance);
	vts[5] = (right * -sideWidth) + vec(0.f,  h2, 0.f) + (dir *  frontDistance);
	vts[6] = (right * sideWidth) + vec(0.f,  h2, 0.f) + (dir *  -backDistance);
	vts[7] = (right * sideWidth) + vec(0.f,  h2, 0.f) + (dir *  frontDistance);

    for ( int i = 0 ; i < 8 ; i++ )
        vts[i] += pts[idx].point + vec(0.f, 10.f, 0.f);


	pushColoredVertex(vts[0], vts[1], vts[3], vts[2], vec(0.f, -1.f, 0.f), col );
	pushColoredVertex(vts[4], vts[5], vts[7], vts[6], vec(0.f,  1.f, 0.f), col );
	pushColoredVertex(vts[0], vts[1], vts[5], vts[4], -right, col );
	pushColoredVertex(vts[2], vts[3], vts[7], vts[6], right, col );
	pushColoredVertex(vts[0], vts[2], vts[6], vts[4], -dir, col );
	pushColoredVertex(vts[1], vts[3], vts[7], vts[5], dir, col );


    pushTextQuad(sublocal, true);
	sublocal+=4;

	pushTextQuad(sublocal, false);
	sublocal+=4;

	pushTextQuad(sublocal, false);
	sublocal+=4;
	pushTextQuad(sublocal, true);
	sublocal+=4;

	pushTextQuad(sublocal, true);
	sublocal+=4;

	pushTextQuad(sublocal, false);
    sublocal+=4;

}

vec_t  getNorm(const vec_t& v1, const vec_t& v2, const vec_t& v3, const vec_t& v4)
{
    UNUSED_PARAMETER(v3);

    vec_t norm;

    vec_t d1 = normalized( v2 - v1 );
    vec_t d2 = normalized( v4 - v1 );


    norm.cross( d2, d1 );
    norm.normalize();

    return norm;
}

void pushReactor( const matrix_t& orient, float radius, int &sublocal )
{
    static const float react1[]= { 1.5f, 4.f, 4.f, 0.f };
    static const float react2[]= { 1.5f, 3.f, 3.f, 0.f };
    static const float radiuses[] = {0.f, 1.f, 2.f, 4.f };
    static const float* ptrs[5] = { react1, react1, react2, react2, react1 };

    #define CRANS 12
    static const float deltang = (2.f*PI)/static_cast<float>(CRANS*7);

    for (int j = 0;j<CRANS;j++)
    {
        float rads = static_cast<float>(j)*deltang * 7.f;
        const float ngs[5] = {rads,rads+deltang*3.f,rads+deltang*4.f,rads+deltang*7.f,rads+deltang*7.f };
        for (int q = 0;q<3;q++)
        {
            for (int i=0;i<4;i++)
            {
                float ng1 = ngs[i];
                float ng2 = ngs[i+1];
                vec_t v1 = vec(cosf(ng1), sinf(ng1) ) * (radius + radiuses[q]);
                vec_t v2 = vec(cosf(ng1), sinf(ng1) ) * (radius + radiuses[q+1]);
                vec_t v3 = vec(cosf(ng2), sinf(ng2) ) * (radius + radiuses[q+1]);
                vec_t v4 = vec(cosf(ng2), sinf(ng2) ) * (radius + radiuses[q]);
                v1.z = ptrs[i][q];
                v2.z = ptrs[i][q+1];
                v3.z = ptrs[i+1][q+1];
                v4.z = ptrs[i+1][q];
                v1.TransformPoint(orient);
                v2.TransformPoint(orient);
                v3.TransformPoint(orient);
                v4.TransformPoint(orient);

                pushColoredVertex( v1, v2, v3, v4, -getNorm(v1, v2, v3, v4), 0x80808080 );
                pushTextQuad( sublocal, true );
                sublocal+= 4;

                if (!q)
                {
                    pushColoredVertex( orient.position+orient.dir*1.5f, v1, v4, vec(0.f, 0.f, -1.f), 0xFFFF6060 );
                    pushTextTri( sublocal, true );
                    sublocal+= 3;
                }
            }
        }
    }
}

vec_t getEtronShapeNormal( vec_t* shape, int nbShape, int idx, const vec_t& wNorm = vec( 0.f, 0.f, 1.f ) )
{
    vec_t v1 = shape[idx%nbShape];
    vec_t v2 = shape[ (idx+1)%nbShape];
    vec_t v3 = shape[ (idx-1+nbShape)%nbShape];


    vec_t dif1 = normalized(v2-v1);
    vec_t dif2 = normalized(v1-v3);

    vec_t norm1, norm2, norm;
    norm1.cross( dif1, wNorm );
    norm2.cross( dif2, wNorm );

    norm = normalized( norm1 + norm2 );

    return norm;
}

void generateEtronWall( vec_t* shape, int nbShape, float z1, float z2, float bevel1, float bevel2, u32 col, int &sublocal, bool invert)
{
    // get shape normal
    vec_t d1 = normalized( shape[1] - shape[0] );
    vec_t d2 = normalized( shape[nbShape-1] - shape[0] );
    vec_t planNorm;
    planNorm.cross(d1, d2);
    planNorm.normalize();

    // iterate for each segment
    for (int i=0;i<nbShape;i++)
    {
        vec_t v1, v2, v3, v4;

        vec_t n1 =  getEtronShapeNormal( shape, nbShape, i, planNorm );
        vec_t n2 =  getEtronShapeNormal( shape, nbShape, i+1, planNorm );

        v1 = v4 = shape[i];
        v2 = v3 = shape[ (i+1)%nbShape];

        v1 += n1 * bevel1;
        v4 += n1 * bevel2;
        v2 += n2 * bevel1;
        v3 += n2 * bevel2;

        v1 += planNorm * z1;
        v2 += planNorm * z1;
        v3 += planNorm * z2;
        v4 += planNorm * z2;


        pushColoredVertex( v1, v2, v3, v4, getNorm(v1, v2, v3, v4)*(invert?-1.f:1.f), col );
        pushTextQuad( sublocal, invert );
        sublocal+= 4;
    }

}

struct auroraVertex_t
{
    float x,y,z;
    float u,v;
};

mesh_t *generateAurora( vec_t* points, int nbPoints, int nbPointsPerSegments, float height )
{
    int nbVertices = (nbPoints-1) * nbPointsPerSegments;

    vec_t *splinesPos = new vec_t [ nbVertices ];
    for ( int i = 0 ; i < nbPoints-1 ; i ++)
    {
        vec_t currentKey, nextKey, nextKeyP1, prevKey;

        currentKey = points[i];
        nextKey = points[i+1];

        if ( !i )
            prevKey = points[0] - ( points[1] - points[0] ) ;
        else
            prevKey = points[i-1];

        if ( i == (nbPoints-2) )
            nextKeyP1 = points[nbPoints-1] + ( points[nbPoints-1] - points[nbPoints-2] ) ;
        else
            nextKeyP1 = points[i+2];

        for (int j = 0 ; j < nbPointsPerSegments ; j ++)
        {
            float interp = (float)j / (float)nbPointsPerSegments;
            splinesPos[ i * nbPointsPerSegments + j ] = currentKey.interpolateHermite( nextKey, nextKeyP1, prevKey, interp );
        }
    }

    // build vertices
    auroraVertex_t * vts = new auroraVertex_t [ nbVertices << 1 ];
    for ( int i = 0; i < nbVertices ; i ++ )
    {
        // lo
        auroraVertex_t& v = vts[ ( i<<1 ) ];
        v.x = splinesPos[ i ].x;
        v.y = splinesPos[ i ].y;
        v.z = splinesPos[ i ].z;

        v.u = (float)i / (float)nbVertices;
        v.v = 0.f;
        // high
        auroraVertex_t& v2 = vts[ ( i<<1 ) + 1 ];
        v2.x = splinesPos[ i ].x;
        v2.y = splinesPos[ i ].y + height;
        v2.z = splinesPos[ i ].z;

        v2.u = (float)i / (float)nbVertices;
        v2.v = 1.f;
    }
    delete [] splinesPos;
    splinesPos = NULL;


    mesh_t* nMesh = new mesh_t;

    nMesh->triCount = (nbVertices-1) * 6;


	nMesh->mIA->Init( nMesh->triCount, VAU_STATIC, false );
	nMesh->mVA->Init( VAF_XYZ|VAF_TEX0, nbVertices * 2, true, VAU_STATIC );



	memcpy( nMesh->mVA->Lock(VAL_WRITE), vts,  nMesh->mVA->GetVertexSize() * nbVertices * 2 );
	memcpy( nMesh->mIA->Lock(VAL_WRITE), Ribbon::GetIndicesForTrails(), sizeof(unsigned int) * nMesh->triCount);
	nMesh->mIA->Unlock();
    nMesh->mVA->Unlock();
    nMesh->computeBSphere();

    delete [] vts;
    vts = NULL;

    return nMesh;
}

mesh_t * BuildAuroraBorealis( const vec_t& source, float maxAmp, float length, float height, int nbPoints )
{
    if ( nbPoints >= 32 )
        return NULL;

    vec_t aurPts[32];
    float nbpf = (float)nbPoints;

    for (int i = 0;i<nbPoints;i++)
    {
        float amp = cosf(i / nbpf * PI ) * maxAmp - ( maxAmp * 0.5f );
        aurPts[i] = vec( -(length*0.5f) + i * (length/nbpf), 0.f, r01() * amp - (amp*0.5f) ) + source;
    }

    mesh_t *aurora = generateAurora( aurPts, nbPoints, 20, height );
    aurora->visible = true;
    aurora->color = vec(0.2f,0.7f, 0.3f, 0.55f );
    aurora->mWorldMatrix.identity();
    aurora->updateWorldBSphere();
    aurora->mbIsAurora = true;
    aurora->mbCastShadows = false;
    return aurora;
}

mesh_t *generatePipe( vec_t *shape, int nbShape, float sliceHeight, float bevel, float radius, int nGons, u32 color, bool bForRope)
{
    float *localLength = new float [nbShape-1];
    float totalLength = 0.f;
    for (int i=0;i<(nbShape-1); i++)
    {
        localLength[i] = (shape[i+1] - shape[i]).length();
        totalLength += localLength[i];
    }

    int nbSlices = (int)(totalLength / sliceHeight );

    int nbVertices = 6 * nGons * nbSlices *2;
    //int nbIndices = nGons * 6 * 3;


    struct pipeVertex
    {
        float x,y,z;
        float nx, ny, nz;
        u32 col;
        float u,v;
    };

    pipeVertex *pv = new pipeVertex [ nbVertices ];
    pipeVertex *pv2 = pv;
    int sublocal = 0;
    vtav = 0;
    triav = 0;

    float lengthDone = 0.f;
    float localLengthDone = 0.f;
    int localIndex = 0;
    std::vector<vec_t> splPos;
    for (int slice=0;slice<nbSlices;slice++, lengthDone+= sliceHeight, localLengthDone += sliceHeight )
    {
        if ( localLengthDone> localLength[localIndex] )
        {
            localLengthDone -= localLength[localIndex];
            localIndex ++;
        }



	    vec_t splinePos = LERP( shape[localIndex], shape[localIndex+1], (localLengthDone*(1.f/localLength[localIndex])) );
        splinePos.w = Clamp( (31.f/totalLength)*lengthDone, 0.f, 31.f );
        splPos.push_back( splinePos );
    }
    splPos.push_back( shape[nbShape-1] );

    delete[] localLength;
    localLength = NULL;

        /*vec_t splinePos2 = LERP( shape[localIndex], shape[localIndex+1], (localLengthDone*(1./localLength[localIndex])) );
        vec_t splineDir = vec(0.f, 0.f, 1.f);
        */
    //vec_t lastDir = vec(0.f, 0.f, 1.f);
    matrix_t lastMatrix;
    lastMatrix.LookAt(shape[0], shape[1], vec(0.f, 1.f, 0.f) );

    for (int slice=0;slice<nbSlices;slice++, lengthDone+= sliceHeight, localLengthDone += sliceHeight )
    {
        matrix_t mat1, mat2;




        vec_t splinePos1 = splPos[slice];
        vec_t splinePos2 = splPos[slice+1];

        //float boneIndex[] = { splinePos1.w, splinePos1.w, splinePos1.w, splinePos2.w, splinePos2.w, splinePos2.w };
        vec_t splineDir = normalized(splinePos2-splinePos1);

        mat1 = lastMatrix;//.LookAt(splinePos1, splinePos1 + lastDir, vec(0.f, 1.f, 0.f));
        mat2.LookAt(splinePos2, splinePos2 + splineDir, vec(0.f, 1.f, 0.f));
        lastMatrix = mat2;
        //vec_t calpos[4]={splinePos1, splinePos1+splineDir*bevel, splinePos1+splineDir*(sliceHeight-bevel), splinePos1+splineDir*(sliceHeight) };
        float calpos[4]={0.f, bevel, -bevel, 0.f };

        //lastDir = splineDir;
        float localRadiuses[4]={radius-bevel, radius, radius, radius-bevel};
        static const float localNormDir[]={-bevel, 0.f, bevel };
        for (int portion = 0; portion<3;portion++)
        {

            for (int i=0;i<nGons;i++)
            {
                matrix_t *mats[4] = { &mat1, &mat1, &mat2, &mat2 };
                float ng = ((2.f*PI)/(float)nGons) * (float)(i+1) ;
                float prevNg = ((2.f*PI)/(float)nGons) * (float)(i) ;
                float prevCs = cosf(prevNg);
                float prevSn = sinf(prevNg);
                float curCs = cosf(ng);
                float curSn = sinf(ng);

                const float locR1 = localRadiuses[portion];
                const float locR2 = localRadiuses[portion+1];
                vec_t vts[4];
                vts[0] = vec( prevCs*locR1, prevSn*locR1, calpos[portion] );// + calpos[portion];
                vts[1] = vec( curCs*locR1, curSn*locR1, calpos[portion] );// + calpos[portion];
                vts[3] = vec( prevCs*locR2, prevSn*locR2, calpos[portion+1] );// + calpos[portion+1];
                vts[2] = vec( curCs*locR2, curSn*locR2, calpos[portion+1] );// + calpos[portion+1];




                vts[0].TransformPoint(*mats[portion]);
                vts[1].TransformPoint(*mats[portion]);

                vts[2].TransformPoint(*mats[portion+1]);
                vts[3].TransformPoint(*mats[portion+1]);






                vec_t nrm[4];
                nrm[0] = vec( prevCs, prevSn, localNormDir[portion] ).normalize();
                nrm[1] = vec( curCs, curSn, localNormDir[portion] ).normalize();
                nrm[3] = vec( prevCs, prevSn, localNormDir[portion] ).normalize();
                nrm[2] = vec( curCs, curSn, localNormDir[portion] ).normalize();


                nrm[0].TransformVector(*mats[portion]);
                nrm[1].TransformVector(*mats[portion]);

                nrm[2].TransformVector(*mats[portion+1]);
                nrm[3].TransformVector(*mats[portion+1]);

                for (int k = 0;k<4;k++)
                {


                    pv->x = vts[k].x;
                    pv->y = vts[k].y;
                    pv->z = vts[k].z;
                    pv->nx = nrm[k].x;
                    pv->ny = nrm[k].y;
                    pv->nz = nrm[k].z;
                    pv->col = color;
                    pv->u = (pv->z * 30.f)/totalLength;

                    if (bForRope)
                        pv->z -= (float)( int)(pv->u);
                    pv->v = 0.f;
                    pv++;
                }


                pushTextQuad(sublocal, true);
                sublocal+=4;
            }
        }
    }


    //pushReactor( orient, 5.f, sublocal );
    //mesh_t * res = pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR);
    mesh_t* nMesh = new mesh_t;


	nMesh->mIA->Init( triav, VAU_STATIC );
	nMesh->mVA->Init( VAF_XYZ|VAF_NORMAL|VAF_COLOR|VAF_TEX0, nbVertices, true, VAU_STATIC );

	nMesh->triCount = triav;
//	nMesh->vtCount = vtav;

	memcpy( nMesh->mVA->Lock(VAL_WRITE), pv2,  nMesh->mVA->GetVertexSize() * nbVertices);
	memcpy( nMesh->mIA->Lock(VAL_WRITE), tris, sizeof(unsigned short) * triav);
	nMesh->mIA->Unlock();
    nMesh->mVA->Unlock();
    nMesh->computeBSphere();

    delete [] pv2;
    pv2 = NULL;

    return nMesh;
}
mesh_t* generateReactor( const matrix_t &orient, float radius )
{
    UNUSED_PARAMETER(radius);

    int sublocal = 0;
    vtav = 0;
    triav = 0;
    pushReactor( orient, 5.f, sublocal );
    mesh_t * res = pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR);
    return res;
}


// -----------------------------------------------------------------------------------------------------------------------------------------

struct buildingText_t
{
    buildingText_t( const matrix_t &mat, const std::string& str, float scale)
    {
        mMatrix = mat;
        mStr = str;
        mScale = scale;
    }
    matrix_t mMatrix;
    std::string mStr;
    float mScale;
};


mesh_t* generateCityLowPoly( )
{
    PROFILER_START(CityGenLowPoly);

    g_seed = -448618726;
    std::list<mesh_t*> cityMeshes;

    const std::string& cityMem = getFileFromMemory( "Datas/Meshes/city.bin" );
    const char *cityMemPtr = cityMem.c_str();
    u16 nbBats = *(u16*)  cityMemPtr;
    u8 * ptr = (u8*)&cityMemPtr[2];

    int sublocal = 0;
    vtav = 0;
    triav = 0;

    // temp

    static const float cityScale = 100.f;
    for (int i = 0 ; i < nbBats ; i++ )
    {
        float height = *(float*)ptr * 100.f;
        ptr += 4;
        u8 nbPoints = *ptr++;


        std::vector<batPoint_t> batPoints;
        // build points
        for ( unsigned int j = 0;j< nbPoints;j++)
        {
            fixed816_t *fixedx = (fixed816_t*)ptr;
            float x = fixedx->toFloat() * cityScale;
            ptr += 3;

            fixed816_t *fixedy = (fixed816_t*)ptr;
            float y = fixedy->toFloat() * cityScale;
            ptr += 3;

            batPoint_t batpt;
            batpt.point = vec( x, 1.f, y );
            batPoints.push_back( batpt );

        }

        // normals seg
        for (int j = 0;j< nbPoints;j++)
        {
            vec_t pNext = batPoints[(j+1+nbPoints)%nbPoints].point;
            vec_t pPoint = batPoints[j].point;

            vec_t dif1 = normalized(pNext-pPoint);

            vec_t crs1 = normalized( cross( dif1, vec(0.f, 1.f, 0.f) ) );
            batPoints[j].segmentNormal = crs1;
            batPoints[j].length = (pNext-pPoint).length();
        }

        // normals point
        for (int j = 0;j< nbPoints;j++)
        {
            vec_t pPoint = batPoints[(j+nbPoints)%nbPoints].segmentNormal;
            vec_t pPrev = batPoints[(j-1+nbPoints)%nbPoints].segmentNormal;

            vec_t dif1 = pPoint + pPrev;
            dif1.normalize();

            batPoints[j].pointNormal = dif1;
        }



        // border
        static const float dwidth = 2.f;
        static const float dheight = 2.f;


        // push building pylon

        vec_t middle = batPoints[0].point;
        for ( unsigned int j = 1 ; j < nbPoints ; j++ )
            middle += batPoints[j].point;
        middle *= 1.f/((float)nbPoints);

        pushBatWall( batPoints, sublocal, 0x80FFFFFF, 0.f, height-dheight, 0.f, 0.f );
        pushBatCapping( batPoints, sublocal, 0x80FFFFFF, height-dheight, 0.f );


        if (vtav> 20000)
        {
            cityMeshes.push_back( pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR) );

            sublocal = 0;
            vtav = 0;
            triav = 0;
        }
    }


    if (vtav)
    {
        cityMeshes.push_back( pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR) );
    }

    mesh_t *nMesh;
    if (cityMeshes.size() > 1)
        nMesh = merge( cityMeshes );
    else
    	nMesh = *cityMeshes.begin();
    // texts
    LOG(" City building lowpoly done// %d tris %d vertices\n", nMesh->mIA->GetIndexCount(), nMesh->mVA->GetVertexCount() );
    ///textMatrices

    PROFILER_END();
    return nMesh;
}


mesh_t* generateCity( )
{
    PROFILER_START(CityGen);

    g_seed = -448618726;
    std::list<mesh_t*> cityMeshes;

    const std::string& cityMem = getFileFromMemory( "Datas/Meshes/city.bin" );
    const char *cityMemPtr = cityMem.c_str();
    u16 nbBats = *(u16*)  cityMemPtr;
    u8 * ptr = (u8*)&cityMemPtr[2];

    int sublocal = 0;
    vtav = 0;
    triav = 0;

    // temp
    std::vector<buildingText_t> buildingTexts;
    std::vector<matrix_t> AllTopMiddles;
    std::vector<matrix_t> AllAds;

    static const float cityScale = 100.f;
    for (int i = 0 ; i < nbBats ; i++ )
    {
        float height = *(float*)ptr * 100.f;
        ptr += 4;
        u8 nbPoints = *ptr++;


        std::vector<batPoint_t> batPoints;
        // build points
        for ( unsigned int j = 0;j< nbPoints;j++)
        {
            fixed816_t *fixedx = (fixed816_t*)ptr;
            float x = fixedx->toFloat() * cityScale;
            ptr += 3;

            fixed816_t *fixedy = (fixed816_t*)ptr;
            float y = fixedy->toFloat() * cityScale;
            ptr += 3;

            batPoint_t batpt;
            batpt.point = vec( x, 1.f, y );
            batPoints.push_back( batpt );

        }

        // normals seg
        for (int j = 0;j< nbPoints;j++)
        {
            vec_t pNext = batPoints[(j+1+nbPoints)%nbPoints].point;
            vec_t pPoint = batPoints[j].point;

            vec_t dif1 = normalized(pNext-pPoint);

            vec_t crs1 = normalized( cross( dif1, vec(0.f, 1.f, 0.f) ) );
            batPoints[j].segmentNormal = crs1;
            batPoints[j].length = (pNext-pPoint).length();
        }

        // normals point
        for (int j = 0;j< nbPoints;j++)
        {
            vec_t pPoint = batPoints[(j+nbPoints)%nbPoints].segmentNormal;
            vec_t pPrev = batPoints[(j-1+nbPoints)%nbPoints].segmentNormal;

            vec_t dif1 = pPoint + pPrev;
            dif1.normalize();

            batPoints[j].pointNormal = dif1;
        }

        // building color
        vec_t buildingColor, buildingOrangeColor;
        buildingColor.lerp( vec(0.85f, 0.85f, 0.85f, 0.5f), vec(1.f, 1.f, 1.f, 0.5f) , r01() );
        buildingOrangeColor = vec(0.7f + r01()* 0.3f,0.12f + r01()*0.10f, 0.12f + r01() * 0.10f, 0.5f );
        float nLerp = (height-37.5f);
        nLerp *= 0.016f;
        nLerp = Clamp(nLerp, 0.f, 1.f);
        buildingColor.lerp(buildingOrangeColor, nLerp);
        u32 buildingColoru32 = buildingColor.toUInt32();


        // border
        static const float dwidth = 2.f;
        static const float dheight = 2.f;

        if (height < 37.5f * 2.f)
        {
            //int rndTop = fastrand()&3;
            pushBatWall( batPoints, sublocal, buildingColoru32, 0.f, height-dheight, 0.f, 0.f );
        }
        else
        {
            // push building pylon

            vec_t middle = batPoints[0].point;
            for ( unsigned int j = 1 ; j < nbPoints ; j++ )
                middle += batPoints[j].point;
            middle *= 1.f/((float)nbPoints);

            // high bat
             int batTop = fastrand()&3;

             switch (batTop)
             {
             case 0:// |
                 pushBatWall( batPoints, sublocal, buildingColoru32, 0.f, height-dheight, 0.f, 0.f );
                 break;
             case 1:
                 pushBatWall( batPoints, sublocal, buildingColoru32, 0.f, 20.f, -dwidth, -dwidth );
                 pushBatWall( batPoints, sublocal, buildingColoru32, 20.f, 20.f, -dwidth, 0.f );
                 pushBatWall( batPoints, sublocal, buildingColoru32, 20.f, height-dheight, 0.f, 0.f );
                 break;
             case 2: // ]
                 {
                     int nbCrochet = (int)(height/25.f);
                     float heightDelta = (height-dheight) / (float)nbCrochet;

                     float hav = 0.f;
                     for (int ci=0;ci<nbCrochet;ci++)
                     {
                         if ( ci )
                             pushBatWall( batPoints, sublocal, buildingColoru32, hav, hav, 0.f, -dwidth );

                         pushBatWall( batPoints, sublocal, buildingColoru32, hav, hav+heightDelta-dheight, -dwidth, -dwidth );
                         pushBatWall( batPoints, sublocal, buildingColoru32, hav+heightDelta-dheight, hav+heightDelta-dheight, -dwidth, 0.f );
                         pushBatWall( batPoints, sublocal, buildingColoru32, hav+heightDelta-dheight, hav+heightDelta, 0.f, 0.f );

                         hav += heightDelta;
                     }
                 }
                 break;
             case 3: // ] en moit-moit
                 {
                     int nbCrochet = (int)(height/25.f);
                     float heightDelta = (height-dheight) / (float)nbCrochet;
                     float halfheight = heightDelta/2.f;
                     float hav = 0.f;
                     for (int ci=0;ci<nbCrochet;ci++)
                     {
                         if ( ci )
                             pushBatWall( batPoints, sublocal, buildingColoru32, hav, hav, 0.f, -dwidth );

                         pushBatWall( batPoints, sublocal, buildingColoru32, hav, hav+heightDelta-halfheight, -dwidth, -dwidth );
                         pushBatWall( batPoints, sublocal, buildingColoru32, hav+heightDelta-halfheight, hav+heightDelta-halfheight, -dwidth, 0.f );
                         pushBatWall( batPoints, sublocal, buildingColoru32, hav+heightDelta-halfheight, hav+heightDelta, 0.f, 0.f );

                         hav += heightDelta;
                     }
                 }
                 break;
             }
            //pushBatCapping( batPoints, sublocal, buildingColoru32, height, 0.f );
             // ad
             if ( height > 50.f*2.f )
             {
                 for ( unsigned int j = 0 ; j < nbPoints ; j++ )
                 {
                     if ( batPoints[j].length < 10.f )
                         continue;

                     if ( !(fastrand()&3))
                     {
                         vec_t eye = ( batPoints[j].point + batPoints[ (j+1)% nbPoints ].point )  * 0.5f;

                         eye.y += 30.f + ( height - 20.f ) * r01();
                         const vec_t& nrm = batPoints[j].segmentNormal;

                         matrix_t adMat;
                         adMat.LookAt( eye+ nrm * 10.f, eye + nrm * 20.f, vec(0.f, 1.f, 0.f ) );
                         matrix_t rotz;
                         rotz.rotationZ( PI * 0.5f );

                         matrix_t res = (batPoints[j].length<30.f)?rotz * adMat:adMat;
                         AllAds.push_back( res );
                     }
                 }
             }
            // text matrices
             for ( unsigned int j = 0 ; j < nbPoints ; j++ )
             {
                 std::string affTxt = GetRandomSentence();
                     float minLength = ((float)affTxt.length()) * 8.f * 0.5f;
                 if ( batPoints[j].length < minLength)
                     continue;
                 int randTxt = fastrand()&0x7;
                 if (randTxt>5)
                     continue;

                 float txtScale = batPoints[j].length / minLength;
                 matrix_t textMatrices;
                 randTxt = randTxt%3;
                 switch (randTxt)
                 {
                     //textMatrices.translation( batPoints[0].point + vec(0.f, height, 0.f) );//.LookAt(batPoints[0].point+vec(0.f, height, 0.f) , batPoints[0].point+vec(0.f, height, 0.f) +batPoints[0].segmentNormal , vec(0.f, 1.f, 0.f ) );
                 case 0:// 0
                     textMatrices.LookAt(batPoints[j].point+vec(0.f, height, 0.f) +batPoints[j].segmentNormal*1.0f, batPoints[j].point+vec(0.f, height, 0.f) +batPoints[j].segmentNormal *20.f, vec(0.f, 1.f, 0.f ) );
                     break;
                 case 1:// 3
                     {
                         matrix_t rotz, lkat;
                         rotz.rotationZ( -PI * 0.5f );
                         lkat.LookAt(batPoints[j].point+vec(0.f, height , 0.f) , batPoints[j].point+vec(0.f, height, 0.f) +batPoints[j].segmentNormal , vec(0.f, 1.f, 0.f ) );
                         textMatrices = rotz * lkat;
                     }
                     break;
                 case 2:// 5
                     {
                         matrix_t rotz, lkat;
                         vec_t crst;
                         crst.cross(batPoints[j].pointNormal, vec(0.f, 1.f, 0.f ) );
                         rotz.rotationZ( -PI * 0.5f );
                         //float len = (batPoints[1].point-batPoints[0].point).length() - 8.f;
                         vec_t eyePt = batPoints[j].point + vec(0.f, height , 0.f) + batPoints[j].segmentNormal;
                         lkat.LookAt( eyePt , eyePt +crst , vec(0.f, 1.f, 0.f ) );
                         textMatrices = rotz * lkat;
                     }
                     break;
                 }
                 buildingTexts.push_back( buildingText_t( textMatrices, affTxt, txtScale ) );
             }

            // ornaments

            bool everyCorners = ((fastrand()&1)!=0);
            bool aBitAbove = ((fastrand()&1)!=0);

            int nbCorners = everyCorners?nbPoints : (fastrand()&3);
            if (nbCorners > nbPoints )
                nbCorners = nbPoints;

            for (int j = 0;j<nbCorners;j++)
            {
                 pushBatOrnament( batPoints, sublocal, buildingColoru32, j, 0.f, height+(aBitAbove?1.f:-5.f), 0.5f, 2.f, 1.f );
            }

            // rotating ad
            if ( !( fastrand()&1) )
            {

                matrix_t hupSide;
                hupSide.translation( vec(middle.x, height+10.f, middle.z ) );

                hupSide.m16[15] = ( vec(middle.x, 0.f, middle.z ) - vec( batPoints[0].point.x, 0.f, batPoints[0].point.z) ).length() / (1.41421356f * 8.f);
                AllTopMiddles.push_back( hupSide );
            }

        }

        // roof

        int rndTop = fastrand()&3;
        switch (rndTop)
        {
        case 0:// _|ии

            pushBatWall( batPoints, sublocal, buildingColoru32, height-dheight, height-dheight, 0.f, dwidth );
            pushBatWall( batPoints, sublocal, buildingColoru32, height-dheight, height, dwidth, dwidth );
            // capping
            pushBatCapping( batPoints, sublocal, buildingColoru32, height, dwidth );
            break;
        case 1:// _
            //pushBatWall( batPoints, sublocal, buildingColoru32, 0.f, height, 0.f, 0.f );
            // capping
            pushBatCapping( batPoints, sublocal, buildingColoru32, height-dheight, 0.f );
            break;
        case 2:// _/
            //pushBatWall( batPoints, sublocal, buildingColoru32, 0.f, height-dheight, 0.f, 0.f );
            pushBatWall( batPoints, sublocal, buildingColoru32, height-dheight, height-dheight, 0.f, dwidth );
            pushBatWall( batPoints, sublocal, buildingColoru32, height-dheight, height, dwidth, dwidth*2.f );
            // capping
            pushBatCapping( batPoints, sublocal, buildingColoru32, height, dwidth*2.f );
            break;
        case 3:// /ии
            //pushBatWall( batPoints, sublocal, buildingColoru32, 0.f, height-dheight, 0.f, 0.f );
            pushBatWall( batPoints, sublocal, buildingColoru32, height-dheight, height, 0.f, dwidth );
            // capping
            pushBatCapping( batPoints, sublocal, buildingColoru32, height, dwidth );
            break;
        }

        if (vtav> 20000)
        {
            cityMeshes.push_back( pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR) );

            sublocal = 0;
            vtav = 0;
            triav = 0;
        }
    }


    if (vtav)
    {
        cityMeshes.push_back( pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR) );
    }

    mesh_t *nMesh;
    if (cityMeshes.size() > 1)
        nMesh = merge( cityMeshes );
    else
    	nMesh = *cityMeshes.begin();
    // texts
    LOG(" City building done// %d tris %d vertices\n", nMesh->mIA->GetIndexCount(), nMesh->mVA->GetVertexCount() );
    ///textMatrices

    // texts
    unsigned int count = buildingTexts.size();
    for (unsigned int i = 0;i< count ; i++)
    {
        const buildingText_t& buildingText = buildingTexts[i];
        matrix_t sc;
        sc.scale(0.5f * buildingText.mScale );
        mesh_t* instanceHat = generateText( buildingText.mStr.c_str() );
        instanceHat->color = vec( r01()*0.5f + 0.5f, r01()*0.5f + 0.5f, r01()*0.5f + 0.5f, 0.75f );
        instanceHat->computeBSphere();
        instanceHat->mWorldMatrix = sc * buildingText.mMatrix;
		instanceHat->visible = true;
		instanceHat->updateWorldBSphere();
        /*
        omniLight_t *pLight = Renderer::newOmni();
        pLight->mColor = instanceHat->color;
        pLight->mColor.w = 100.f;
        pLight->mPosition = instanceHat->mWorldMatrix.position;
        pLight->mPosition.w = instanceHat->bSphere.w * 4.f;
        */

    }
    // things on top
    count = AllTopMiddles.size();
    for ( unsigned int i = 0 ; i < count ; i ++)
    {
         matrix_t& hupSide = AllTopMiddles[i];

         float textRadius = hupSide.m16[15];
         hupSide.m16[15] = 1.f;

        switch (fastrand()&1)
        {
        case 0: // rotating text
            {

                std::string textRot = GetRandomSentence() + " -  ";
                mesh_t* instanceHat = generateText(textRot.c_str());
                float tLen = (float)strlen(textRot.c_str());

                matrix_t mt,sc;
                mt.translation(0.f, 0.2f, 5.f);
                sc.scale(1.f, 0.5f*(16.f / tLen), 1.f);
                transformMesh(instanceHat, mt * sc);
                instanceHat->color = vec( r01()*0.5f + 0.5f, r01()*0.5f + 0.5f, r01()*0.5f + 0.5f, 0.75f );//vec(1,0.8f,0.8f,0.7f);
                bendMesh(instanceHat, PI*2.0f, 0, 1);


                meshAnimation_t *pa = instanceHat->addAnimation();
                pa->mDuration = 5.f;

                pa->mRotationStart = vec(0.f, 0.0f, 0.f);
                pa->mRotationEnd = vec(0.f, -2.f * PI, 0.f);
                pa->mRotationType = 1;
                pa->mRotationInterpolation = 0;

                matrix_t scaleText;
                scaleText.scale( textRadius );



                instanceHat->mBaseMatrix = scaleText * hupSide;//.identity();
                instanceHat->visible = true;
                instanceHat->updateWorldBSphere();
                /*
                omniLight_t *pLight = Renderer::newOmni();
                pLight->mColor = instanceHat->color;
                pLight->mColor.w = 10.f;
                pLight->mPosition = instanceHat->mWorldMatrix.position;
                pLight->mPosition.w = instanceHat->bSphere.w * 2.f;
                */
            }
            break;
        case 1: // antena
            {
                matrix_t mlower, uncenter;
                mlower.scale(0.5f, 30.f+r01()*10.f, 0.5f);
                uncenter.translation(0.f,0.45f, 0.f);
                mesh_t *antena = generateBox();
                transformMesh(antena, uncenter * mlower );
                antena->color = vec(0.f, 0.f, 0.f, 0.5f);
                antena->visible = true;
                antena->computeBSphere();
                antena->mWorldMatrix = hupSide;
                antena->updateWorldBSphere();
            }
            break;
        }
    }

    count = AllAds.size();
    for ( unsigned int i = 0 ; i < count ; i++ )
    {
        world.AddBuildingAd( AllAds[i], 2.5f );


    }
    PROFILER_END();
    return nMesh;
}

mesh_t* generateMeshFromVolume(bool *pVol, int _x, int _y, int _z)
{
	int slicef = _x*_y;
	vtav = 0;
	triav = 0;
	int sublocal = 0;

	for (int y = 0;y<=(_y-1);y++)
	{
		for (int x = 0;x<(_x-1);x++)
		{
			for (int z = 0;z<(_z-1);z++)
			{
				vec_t vts[8];
				vts[0] = vec(x, y, z);
				vts[1] = vec(x, y, z+1);
				vts[2] = vec(x+1, y, z);
				vts[3] = vec(x+1, y, z+1);
				vts[4] = vec(x, y+1, z);
				vts[5] = vec(x, y+1, z+1);
				vts[6] = vec(x+1, y+1, z);
				vts[7] = vec(x+1, y+1, z+1);

				bool bCurrentCellFilled = pVol[slicef*z + y*_x + x];

				// X
				if ( bCurrentCellFilled ^ pVol[slicef*z + y*_x + x+1] )
				{
					pushTextVertex(vts[6], vts[7], vts[3], vts[2],
						vec( (bCurrentCellFilled?1.f:-1.f), 0.f, 0.f ));

					pushTextQuad(sublocal, bCurrentCellFilled);
					sublocal += 4;
				}

				// Y
                bool bNextForY = (y==(_y-1))?false:pVol[slicef*z + (y+1)*_x + x];
				if ( bCurrentCellFilled ^ bNextForY)
				{
					pushTextVertex(vts[4], vts[5], vts[7], vts[6],
						vec(0.f, bCurrentCellFilled?1.f:-1.f, 0.f));

					pushTextQuad(sublocal, bCurrentCellFilled);
					sublocal += 4;
				}

				// Z
				if ( bCurrentCellFilled ^ pVol[slicef*(z+1) + y*_x + x])
				{
					pushTextVertex(vts[1], vts[3], vts[7], vts[5],
						vec(0.f, 0.f, bCurrentCellFilled?1.f:-1.f));

					pushTextQuad(sublocal, bCurrentCellFilled);
					sublocal += 4;
				}
			}
		}
	}

	mesh_t* nMesh = pushNewMesh();

	return nMesh;
}

mesh_t *generateVoxelMesh(uint32 *pCols, int _x, int _y, int _z)
{
	int slicef = _x*_y;
	vtav = 0;
	triav = 0;
	int sublocal = 0;

	for (int y = -1;y<_y;y++)
	{
		for (int x = -1;x<_x;x++)
		{
			for (int z = -1;z<_z;z++)
			{
				vec_t vts[8];
				vts[0] = vec(x, y, z);
				vts[1] = vec(x, y, z+1);
				vts[2] = vec(x+1, y, z);
				vts[3] = vec(x+1, y, z+1);
				vts[4] = vec(x, y+1, z);
				vts[5] = vec(x, y+1, z+1);
				vts[6] = vec(x+1, y+1, z);
				vts[7] = vec(x+1, y+1, z+1);

				uint32 curcol = 0;
                uint32 nexcol = 0;
				if ( (x>=0) && (y>=0) && (z>=0) )
					curcol = pCols[slicef*z + y*_x + x];

				bool bCurrentCellFilled = ((curcol&0xFF000000)!=0);


				// X
				bool bNextCellFilled = false;

				if ((x+1)<_x && y>=0 && z>=0  )
                {
					nexcol = (pCols[slicef*z + y*_x + x+1]);
                    bNextCellFilled = ((nexcol&0xFF000000)!=0);
                }
				if ( bCurrentCellFilled != bNextCellFilled )
				{
					pushColoredVertex(vts[6], vts[7], vts[3], vts[2],
						vec( (bCurrentCellFilled?1.f:-1.f), 0.f, 0.f ),
						bCurrentCellFilled?curcol:nexcol);

					pushTextQuad(sublocal, bCurrentCellFilled);
					sublocal += 4;
				}

				// Y
				bNextCellFilled = false;
				if (x>=0 && (y+1)<_y && z>=0)
                {
					nexcol = (pCols[slicef*z + (y+1)*_x + x]);
                    bNextCellFilled = ((nexcol&0xFF000000)!=0);
                }
				if ( bCurrentCellFilled != bNextCellFilled)
				{
					pushColoredVertex(vts[4], vts[5], vts[7], vts[6],
						vec(0.f, bCurrentCellFilled?1.f:-1.f, 0.f),
						bCurrentCellFilled?curcol:nexcol);

					pushTextQuad(sublocal, bCurrentCellFilled);
					sublocal += 4;
				}

				// Z
				bNextCellFilled = false;
				if (x>=0 && y>=0 && (z+1)<_z )
                {
					nexcol = (pCols[slicef*(z+1) + y*_x + x]);
                    bNextCellFilled = ((nexcol&0xFF000000)!=0);
                }
				if ( bCurrentCellFilled != bNextCellFilled)
				{
					pushColoredVertex(vts[1], vts[3], vts[7], vts[5],
						vec(0.f, 0.f, bCurrentCellFilled?1.f:-1.f),
                                      bCurrentCellFilled?curcol:nexcol);

					pushTextQuad(sublocal, bCurrentCellFilled);
					sublocal += 4;
				}
			}
		}
	}

	mesh_t* nMesh = pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR );
    nMesh->computeBSphere();
	return nMesh;
}

bool*   generateVolumeFromText(const char *szText, int &x, int &y, int &z)
{
	int nchars = strlen(szText);
	int slicef = nchars*8*8;
	int nbFloats = 3*slicef;
	bool *isos = new bool [nbFloats];
	memset(isos, 0, sizeof(bool)*nbFloats);

/*    if (!m8x8FontBits)
        m8x8FontBits = getFileFromMemory("Datas/Textures/A8x8font.raw").c_str();
*/
	for (int i=0;i<nchars;i++)
	{
		unsigned int c = szText[i];
		const u8 *pbits = (const u8*)&m8x8FontBits[(c&15) * 8 + (c>>4)*8*128];

		for (int _y=7;_y>=0;_y--)
		{
			for (int _x=0;_x<8;_x++)
			{
				isos[slicef + _y*nchars*8 + i*8 +_x] = ((*pbits++)>0);
			}
			pbits += 120;
		}
	}
	x = 8*nchars;
	y = 8;
	z = 3;
	return isos;
}

void getMinMax(mesh_t *pMesh, vec_t &amax, vec_t &amin)
{
	u8 *pv = (u8*)pMesh->mVA->Lock( VAL_READONLY );

	meshVertex_t *pvt = (meshVertex_t *)pv;
	amax = vec(pvt->x, pvt->y, pvt->z);
	amin = vec(pvt->x, pvt->y, pvt->z);
	pv += pMesh->mVA->GetVertexSize();

    unsigned int count = pMesh->mVA->GetVertexCount();
    for ( unsigned int i = 1 ; i < count ; i++ )
	{
		pvt = (meshVertex_t *)pv;
		amax.isMaxOf(vec(pvt->x, pvt->y, pvt->z));
		amin.isMinOf(vec(pvt->x, pvt->y, pvt->z));
		pv += pMesh->mVA->GetVertexSize();
	}
	pMesh->mVA->Unlock();
}

void bendMesh(mesh_t *pMesh, float desiredAngle, int axeLen, int axeRot)
{
	if (fabsf(desiredAngle) <= FLOAT_EPSILON)
		return;

	vec_t amax, amin;
	getMinMax(pMesh, amax, amin);


	u8 *pv = (u8*)pMesh->mVA->Lock( VAL_READONLY );
	meshVertex_t *pvt;// = (meshVertex_t *)pv;

	float cx;
	switch (axeLen)
	{
	case 0: cx = amax.x / desiredAngle; break;
	case 1:	cx = amax.y / desiredAngle; break;
	case 2:	cx = amax.z / desiredAngle; break;
	default: cx = 0.f; ASSERT_GAME(0); break;
	}

    unsigned count = pMesh->mVA->GetVertexCount();
    for ( unsigned int i = 0 ; i < count ; i++ )
	{
		pvt = (meshVertex_t *)pv;

		vec_t vt = vec(pvt->x, pvt->y, pvt->z, 1.f);
		vec_t nm = vec(pvt->nx, pvt->ny, pvt->nz, 0.f);

		float a;
		matrix_t mt;
		switch (axeLen)
		{
			case 0: a = vt.x * desiredAngle / amax.x; vt.x = 0; break;
			case 1: a = vt.y * desiredAngle / amax.y; vt.y = 0; break;
			case 2: a = vt.z * desiredAngle / amax.z; vt.z = 0; break;
			default: a = 0.f; ASSERT_GAME(0);
		}

		switch(axeRot)
		{
		case 0:
			vt.z += cx;
			mt.rotationX(a);
			vt.transform(mt);
			vt.z -= cx;
			break;
		case 1:
			vt.y += cx;
			mt.rotationY(a);
			vt.transform(mt);
			vt.y -= cx;
			break;
		case 2:
			mt.rotationZ(a);
			switch (axeLen)
			{
			case 0:
				vt.y -= cx;
				vt.transform(mt);
				vt.y += cx;
				break;
			case 1:
				vt.x += cx;
				vt.transform(mt);
				vt.x -= cx;
				break;
			case 2:
				break;
			}
			break;
		}

		nm.normalize();
		pvt->set(vt, nm);
		pv += pMesh->mVA->GetVertexSize();
	}
	recomputeNormals( pMesh );
	pMesh->mVA->Unlock();
}

void transformMesh(mesh_t *pMesh, const matrix_t& mt)
{
	u8 *pv = (u8*)pMesh->mVA->Lock( VAL_WRITE );

    unsigned int count = pMesh->mVA->GetVertexCount();
	for ( unsigned int i = 0 ; i < count ; i++ )
	{
		meshVertex_t *pvt = (meshVertex_t *)pv;

		vec_t vt = vec(pvt->x, pvt->y, pvt->z, 1.f);
		vec_t nm = vec(pvt->nx, pvt->ny, pvt->nz, 0.f);

		vt.TransformPoint(mt);
		nm.TransformVector(mt);

		nm.normalize();
		pvt->set(vt, nm);
		pv += pMesh->mVA->GetVertexSize();
	}
	pMesh->mVA->Unlock();
}

void drawMesh(mesh_t *pMesh)
{
	pMesh->mIA->Bind();
	pMesh->mVA->Bind();
	glDrawElements(	GL_TRIANGLES, pMesh->triCount, (pMesh->mIA->GetElementSize()==2)?GL_UNSIGNED_SHORT:GL_UNSIGNED_INT, 0);
	//glDisableClientState(GL_COLOR_ARRAY);

    pMesh->mVA->Unbind();
#if 0
	/*
	glPointSize(10.f);
	glDisable(GL_CULL_FACE);
	*/
	if (pMesh->hasColors)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		int sz = sizeof(meshColorVertex_t);
		unsigned char *pvt = (unsigned char *)&pMesh->vertices[0].x;
		glVertexPointer(3, GL_FLOAT, sz,pvt);
		glNormalPointer(GL_FLOAT, sz, pvt+12);
		glColorPointer(4, GL_UNSIGNED_BYTE, sz, pvt+ 24);

		glDrawElements(	GL_TRIANGLES, pMesh->triCount, GL_UNSIGNED_INT, pMesh->indices);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
	else
	{

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glVertexPointer(3, GL_FLOAT, 24, &pMesh->vertices[0].x);
		glNormalPointer(GL_FLOAT, 24, &pMesh->vertices[0].nx);
		glDrawElements(	GL_TRIANGLES, pMesh->triCount, GL_UNSIGNED_INT, pMesh->indices);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
#endif
}

mesh_t* generateBox()
{
	vtav = 0;
	triav = 0;
	int sublocal = 0;


	vec_t vts[8];
	vts[0] = vec(-0.5f, -0.5f, -0.5f);
	vts[1] = vec(-0.5f, -0.5f, 0.5f);
	vts[2] = vec(0.5f, -0.5f, -0.5f);
	vts[3] = vec(0.5f, -0.5f, 0.5f);
	vts[4] = vec(-0.5f, 0.5f, -0.5f);
	vts[5] = vec(-0.5f, 0.5f, 0.5f);
	vts[6] = vec(0.5f, 0.5f, -0.5f);
	vts[7] = vec(0.5f, 0.5f, 0.5f);

	pushTextVertex(vts[0], vts[1], vts[3], vts[2], vec(0.f, -1.f, 0.f) );
	pushTextVertex(vts[4], vts[5], vts[7], vts[6], vec(0.f,  1.f, 0.f) );
	pushTextVertex(vts[0], vts[1], vts[5], vts[4], vec(-1.f, 0.f, 0.f) );
	pushTextVertex(vts[2], vts[3], vts[7], vts[6], vec(1.f, 0.f, 0.f) );
	pushTextVertex(vts[0], vts[2], vts[6], vts[4], vec(0.f, 0.f, -1.f) );
	pushTextVertex(vts[1], vts[3], vts[7], vts[5], vec(0.f, 0.f, 1.f) );


	pushTextQuad(sublocal, false);
	sublocal+=4;
	pushTextQuad(sublocal, true);
	sublocal+=4;

	pushTextQuad(sublocal, true);
	sublocal+=4;
	pushTextQuad(sublocal, false);
	sublocal+=4;


	pushTextQuad(sublocal, false);
	sublocal+=4;
	pushTextQuad(sublocal, true);


	mesh_t *pm = pushNewMesh();
	pm->triCount = 36;


	return pm;
}

mesh_t* generateBoxOnGround(bool hasLowerFace)
{
vtav = 0;
	triav = 0;
	int sublocal = 0;


	vec_t vts[8];
	vts[0] = vec(-0.5f, 0.f, -0.5f);
	vts[1] = vec(-0.5f, 0.f, 0.5f);
	vts[2] = vec(0.5f, 0.f, -0.5f);
	vts[3] = vec(0.5f, 0.f, 0.5f);
	vts[4] = vec(-0.5f, 1.f, -0.5f);
	vts[5] = vec(-0.5f, 1.f, 0.5f);
	vts[6] = vec(0.5f, 1.f, -0.5f);
	vts[7] = vec(0.5f, 1.f, 0.5f);

	pushTextVertex(vts[0], vts[1], vts[3], vts[2], vec(0.f, -1.f, 0.f) );
	pushTextVertex(vts[4], vts[5], vts[7], vts[6], vec(0.f,  1.f, 0.f) );
	pushTextVertex(vts[0], vts[1], vts[5], vts[4], vec(-1.f, 0.f, 0.f) );
	pushTextVertex(vts[2], vts[3], vts[7], vts[6], vec(1.f, 0.f, 0.f) );
	pushTextVertex(vts[0], vts[2], vts[6], vts[4], vec(0.f, 0.f, -1.f) );
	pushTextVertex(vts[1], vts[3], vts[7], vts[5], vec(0.f, 0.f, 1.f) );

	if (hasLowerFace)
	{
		pushTextQuad(sublocal, false);
		sublocal+=4;
	}

	pushTextQuad(sublocal, true);
	sublocal+=4;

	pushTextQuad(sublocal, true);
	sublocal+=4;
	pushTextQuad(sublocal, false);
	sublocal+=4;


	pushTextQuad(sublocal, false);
	sublocal+=4;
	pushTextQuad(sublocal, true);


	mesh_t *pm = pushNewMesh();
	pm->triCount = hasLowerFace?36:30;

	return pm;
}


mesh_t* generateBoxFrustum(bool hasLowerFace)
{
vtav = 0;
	triav = 0;
	int sublocal = 0;


	vec_t vts[8];
	vts[0] = vec(-1.f, -1.f, 0.f);
	vts[1] = vec(-1.f,  1.f, 0.f);
	vts[2] = vec( 1.f, -1.f, 0.f);
	vts[3] = vec( 1.f,  1.f, 0.f);
	vts[4] = vec(-1.f, -1.f, 1.f);
	vts[5] = vec(-1.f,  1.f, 1.f);
	vts[6] = vec( 1.f, -1.f, 1.f);
	vts[7] = vec( 1.f,  1.f, 1.f);

	pushTextVertex(vts[0], vts[1], vts[3], vts[2], vec(0.f, -1.f, 0.f) );
	pushTextVertex(vts[4], vts[5], vts[7], vts[6], vec(0.f,  1.f, 0.f) );
	pushTextVertex(vts[0], vts[1], vts[5], vts[4], vec(-1.f, 0.f, 0.f) );
	pushTextVertex(vts[2], vts[3], vts[7], vts[6], vec(1.f, 0.f, 0.f) );
	pushTextVertex(vts[0], vts[2], vts[6], vts[4], vec(0.f, 0.f, -1.f) );
	pushTextVertex(vts[1], vts[3], vts[7], vts[5], vec(0.f, 0.f, 1.f) );

	if (hasLowerFace)
	{
		pushTextQuad(sublocal, false);
		sublocal+=4;
	}

	pushTextQuad(sublocal, true);
	sublocal+=4;

	pushTextQuad(sublocal, true);
	sublocal+=4;
	pushTextQuad(sublocal, false);
	sublocal+=4;


	pushTextQuad(sublocal, false);
	sublocal+=4;
	pushTextQuad(sublocal, true);


	mesh_t *pm = pushNewMesh();
	pm->triCount = hasLowerFace?36:30;

	return pm;
}

mesh_t* generateBoxFrustum(vec_t vts[8])
{
    vtav = 0;
	triav = 0;
	int sublocal = 0;


	pushTextVertex(vts[0], vts[1], vts[3], vts[2], vec(0.f, -1.f, 0.f) );
	pushTextVertex(vts[4], vts[5], vts[7], vts[6], vec(0.f,  1.f, 0.f) );
	pushTextVertex(vts[0], vts[1], vts[5], vts[4], vec(-1.f, 0.f, 0.f) );
	pushTextVertex(vts[2], vts[3], vts[7], vts[6], vec(1.f, 0.f, 0.f) );
	pushTextVertex(vts[0], vts[2], vts[6], vts[4], vec(0.f, 0.f, -1.f) );
	pushTextVertex(vts[1], vts[3], vts[7], vts[5], vec(0.f, 0.f, 1.f) );

	pushTextQuad(sublocal, false);
	sublocal+=4;

	pushTextQuad(sublocal, true);
	sublocal+=4;

	pushTextQuad(sublocal, true);
	sublocal+=4;
	pushTextQuad(sublocal, false);
	sublocal+=4;


	pushTextQuad(sublocal, false);
	sublocal+=4;
	pushTextQuad(sublocal, true);


	mesh_t *pm = pushNewMesh();
	pm->triCount = 36;

	return pm;
}

void mesh_t::createScreenMesh( const vec_t& uvs )
{
    mIA->Init(12, VAU_STATIC, true);
    unsigned short *pi = (unsigned short*)mIA->Lock( VAL_WRITE );

    *pi++ = 0;
    *pi++ = 1;
    *pi++ = 3;
    *pi++ = 1;
    *pi++ = 2;
    *pi++ = 3;

    *pi++ = 3;
    *pi++ = 1;
    *pi++ = 0;
    *pi++ = 3;
    *pi++ = 2;
    *pi++ = 1;
    mIA->Unlock();


    struct screenVt
    {
        float x,y,z;
        float u,v;
    };

    mVA->Init( VAF_XYZ|VAF_TEX0, 4, false, VAU_STATIC );
    screenVt *pv = (screenVt*)mVA->Lock( VAL_WRITE );

    pv[0].x = -0.5f;
    pv[0].y = -0.5f;
    pv[0].z =  0.1f;
    pv[0].u = uvs.x;
    pv[0].v = uvs.y;

    pv[1].x =  0.5f;
    pv[1].y = -0.5f;
    pv[1].z =  0.1f;
    pv[1].u = uvs.z;
    pv[1].v = uvs.y;


    pv[2].x =  0.5f;
    pv[2].y =  0.5f;
    pv[2].z =  0.1f;
    pv[2].u = uvs.z;
    pv[2].v = uvs.w;


    pv[3].x = -0.5f;
    pv[3].y =  0.5f;
    pv[3].z =  0.1f;
    pv[3].u = uvs.x;
    pv[3].v = uvs.w;

    mVA->Unlock();


    triCount = 12;

    computeBSphere();
}

mesh_t * merge(mesh_t *pBaseMesh, mesh_t *pOther)
{
	mesh_t *mtom[] = { pBaseMesh, pOther };
	return merge( mtom, 2 );
}

mesh_t * merge(mesh_t **pBaseMesh, int nbMeshes)
{
	ASSERT_GAME ( (nbMeshes >= 2) );

	mesh_t *nMesh = new mesh_t;

	int nbV = 0;
	int nbI = 0;
	for (int i=0;i<nbMeshes;i++)
	{
		nbV += pBaseMesh[i]->mVA->GetVertexCount();
		nbI += pBaseMesh[i]->mIA->GetIndexCount();
        ASSERT_GAME( ( pBaseMesh[i]->mIA->GetElementSize() == pBaseMesh[0]->mIA->GetElementSize() ) );
	}

	nMesh->mIA->Init( nbI, VAU_STATIC, (nbV<65535) );
	nMesh->mVA->Init( pBaseMesh[0]->mVA->GetFormat(), nbV, true, VAU_STATIC );

    nMesh->triCount = nbI;

    if ( nbV < 65535)
    {

	    //nMesh->vtCount = nbV;
	    unsigned short *pi = (unsigned short *)nMesh->mIA->Lock( VAL_WRITE );
        u32 *pi32 = (u32*)pi;
	    u8* pvt = (u8*)nMesh->mVA->Lock( VAL_WRITE );
        ASSERT_GAME( pvt );

	    int prevVt = 0;
	    for (int i=0;i<nbMeshes;i++)
	    {
		    ASSERT_GAME( (pBaseMesh[i]->mVA->GetFormat() == pBaseMesh[0]->mVA->GetFormat()) );

		    // vertices
		    int bsz = pBaseMesh[i]->mVA->GetVertexCount() * pBaseMesh[i]->mVA->GetVertexSize();
		    memcpy(pvt, pBaseMesh[i]->mVA->Lock( VAL_READONLY ), bsz );

		    pvt += bsz;
		    pBaseMesh[i]->mVA->Unlock();

		    // indices
            if ( ( pBaseMesh[0]->mIA->GetElementSize() == 2) )
            {
		        unsigned short *pisrc = (unsigned short *)pBaseMesh[i]->mIA->Lock( VAL_READONLY );
		        for ( unsigned int j=0; j< pBaseMesh[i]->mIA->GetIndexCount(); j++)
                {
			        (*pi++) = static_cast<unsigned short>( (*pisrc++) + prevVt );
                }
            }
            else
            {
		        u32 *pisrc = (u32 *)pBaseMesh[i]->mIA->Lock( VAL_READONLY );
		        for ( unsigned int j=0; j< pBaseMesh[i]->mIA->GetIndexCount(); j++)
                {
			        (*pi32++) = static_cast<u32>( (*pisrc++) + prevVt );
                }
            }
		    pBaseMesh[i]->mIA->Unlock();
            prevVt += pBaseMesh[i]->mVA->GetVertexCount();
	    }
    }
    else
    {
        // int version
        unsigned int *pi = (unsigned int *)nMesh->mIA->Lock( VAL_WRITE );
        u32 *pi32 = (u32*)pi;
	    u8* pvt = (u8*)nMesh->mVA->Lock( VAL_WRITE );
	    int prevVt = 0;
	    for (int i=0;i<nbMeshes;i++)
	    {
		    ASSERT_GAME( (pBaseMesh[i]->mVA->GetFormat() == pBaseMesh[0]->mVA->GetFormat()) );

		    // vertices
		    int bsz = pBaseMesh[i]->mVA->GetVertexCount() * pBaseMesh[i]->mVA->GetVertexSize();
		    memcpy(pvt, pBaseMesh[i]->mVA->Lock( VAL_READONLY ), bsz );

		    pvt += bsz;
		    pBaseMesh[i]->mVA->Unlock();

		    // indices
            if ( ( pBaseMesh[0]->mIA->GetElementSize() == 2) )
            {
		        unsigned short *pisrc = (unsigned short *)pBaseMesh[i]->mIA->Lock( VAL_READONLY );
		        for ( unsigned int j=0; j< pBaseMesh[i]->mIA->GetIndexCount(); j++)
			        (*pi++) = (*pisrc++) + prevVt;
            }
            else
            {
		        u32 *pisrc = (u32 *)pBaseMesh[i]->mIA->Lock( VAL_READONLY );
		        for ( unsigned int j=0; j< pBaseMesh[i]->mIA->GetIndexCount(); j++)
			        (*pi32++) = (*pisrc++) + prevVt;
            }
		    pBaseMesh[i]->mIA->Unlock();
            prevVt += pBaseMesh[i]->mVA->GetVertexCount();
	    }
    }

	nMesh->mIA->Unlock();
	nMesh->mVA->Unlock();

	return nMesh;
}

mesh_t * merge(const std::list<mesh_t*>& meshes)
{
    const int meshCount = meshes.size();
	ASSERT_GAME ( meshCount >= 1 );

	mesh_t **mtom = new mesh_t* [meshCount];

	std::list<mesh_t*>::const_iterator iter;
	int idx = 0;
	for (iter = meshes.begin(); iter != meshes.end() ; ++iter, ++idx)
		mtom[idx] = (*iter);

    ASSERT_GAME( idx == meshCount );
	mesh_t *nMesh = merge( mtom, meshCount );

	delete [] mtom;
    mtom = NULL;

	return nMesh;
}

void deleteMeshes(std::list<mesh_t*>& meshes)
{
    const int meshCount = meshes.size();

    int idx = 0;
    while( !meshes.empty() )
    {
        mesh_t* back_mesh = meshes.back();
        meshes.pop_back();

        delete back_mesh;
        back_mesh = NULL;

        ++idx;
    }
    ASSERT_GAME( idx == meshCount );
    UNUSED_PARAMETER( meshCount );  //HACK: needed when ASSERT_GAME() is void, to prevent warning 4189 : local variable is initialized but not referenced
}

void recomputeNormals(mesh_t *pMesh)
{
	ASSERT_GAME ( ( pMesh->mVA && (pMesh->mVA->GetFormat()&VAF_NORMAL) ) );

	u8 *pv = (u8*)pMesh->mVA->Lock( VAL_WRITE );
	u8 *pv2 = pv;
	int vsize = pMesh->mVA->GetVertexSize();
    int vtCount = pMesh->mVA->GetVertexCount();
	float *nbnorm = new float [ vtCount ];
	memset(nbnorm, 0, sizeof(float)* vtCount );
	for ( int i = 0 ; i < vtCount ; i++ )
	{
		meshVertex_t *pvt = (meshVertex_t *)pv;

		pvt->nx = pvt->ny = pvt->nz = 0.f;
		pv += vsize;
	}
	//pvt = pMesh->vertices;
	unsigned short *pind = (unsigned short *)pMesh->mIA->Lock( VAL_READONLY );
	for (int i=0;i<pMesh->triCount/3;i++)
	{
		const meshVertex_t& id1 = (meshVertex_t&)pv2[ pind[0] * vsize ];
		const meshVertex_t& id2 = (meshVertex_t&)pv2[ pind[1] * vsize ];
		const meshVertex_t& id3 = (meshVertex_t&)pv2[ pind[2] * vsize ];
		vec_t vt1 = vec(id1.x, id1.y, id1.z);
		vec_t vt2 = vec(id2.x, id2.y, id2.z);
		vec_t vt3 = vec(id3.x, id3.y, id3.z);

		vec_t d1 = vt2-vt1;
		vec_t d2 = vt3-vt1;
		d1.normalize();
		d2.normalize();
		vec_t nrm;
		nrm.cross(d1, d2);
		nrm.normalize();

		for (int j=0;j<3;j++)
		{
			((meshVertex_t&)pv2[ pind[j] * vsize ]).nx += nrm.x;
			((meshVertex_t&)pv2[ pind[j] * vsize ]).ny += nrm.y;
			((meshVertex_t&)pv2[ pind[j] * vsize ]).nz += nrm.z;
			nbnorm[pind[j]] += 1.f;
		}

		pind += 3;
	}
	//pvt = pMesh->vertices;
	pv = pv2;
	for ( int i = 0 ; i < vtCount ; i++ )
	{
		meshVertex_t *pvt = (meshVertex_t *)pv;

		float inv = 1.f/nbnorm[i];
		pvt->nx *= inv;
		pvt->ny *= inv;
		pvt->nz *= inv;
		pv += vsize;
	}
	delete [] nbnorm;
    nbnorm = NULL;

	pMesh->mVA->Unlock();
	pMesh->mIA->Unlock();
}


void applyFunc(mesh_t *pMesh, const vec_t& valMax, int axe, int opType)
{
	ASSERT_GAME ( ( pMesh->mVA && (pMesh->mVA->GetFormat()&VAF_NORMAL) ) );
	u8 * pv = (u8 *)pMesh->mVA->Lock( VAL_WRITE );
	//meshVertex_t *pvt = pMesh->vertices;

	vec_t amax, amin;
	getMinMax(pMesh, amax, amin);
	vec_t adif = amax - amin;

	vec_t valCopy, val;
	if (opType)
		valCopy = vec(1.f, 1.f, 1.f);
	else
		valCopy = vec(0.f,0.f,0.f);

    unsigned count = pMesh->mVA->GetVertexCount();
    for ( unsigned int i = 0 ; i < count ; i++ )
	{
		meshVertex_t *pvt = (meshVertex_t *)pv;
		vec_t vt = vec(pvt->x, pvt->y, pvt->z, 1.f);
		vec_t nm = vec(pvt->nx, pvt->ny, pvt->nz, 0.f);

		val = valCopy;
		switch (axe)
		{
			case 0:	val.lerp(valMax, (vt.x-amin.x)/adif.x); break;
			case 1:	val.lerp(valMax, (vt.y-amin.y)/adif.y); break;
			case 2:	val.lerp(valMax, (vt.z-amin.z)/adif.z); break;
			default: ASSERT_GAME(0);
		}
		if (opType)
		{
			vt *= val;
			nm *= val;
			nm.normalize();
		}
		else
		{
			vt += val;
			nm += val;
			nm.normalize();
		}

		//pvt->setPosition(vt);
		pvt->set(vt, nm);
		//pvt ++;
		pv += pMesh->mVA->GetVertexSize();
	}
	//recomputeNormals( pMesh );

	pMesh->mVA->Unlock();
}

void taper(mesh_t *pMesh, const vec_t& decal, int axe)
{
	applyFunc(pMesh, decal, axe, 0);
}

void squiz(mesh_t *pMesh, const vec_t& ratio, int axe)
{
	applyFunc(pMesh, ratio, axe, 1);
}

mesh_t* generateCylinder(int nbSides, bool bSmooth, int bypassStart, int bypassCount, int bypassStart2, int bypassCount2)
{
	vtav = 0;
	triav = 0;
	int sublocal = 0;


	float prevNg = PI * 0.25f;
	for (int i=1;i<=nbSides;i++)
	{
		float ng = ((2*PI)/(float)nbSides) * (float)i + PI*0.25f;;
		float prevCs = cosf(prevNg)*0.5f;
		float prevSn = sinf(prevNg)*0.5f;
		float curCs = cosf(ng)*0.5f;
		float curSn = sinf(ng)*0.5f;

		vec_t vts[4];
		vts[0] = vec(prevCs, -0.5f, prevSn);
		vts[1] = vec(curCs, -0.5f, curSn);
		vts[2] = vec(prevCs, 0.5f, prevSn);
		vts[3] = vec(curCs, 0.5f, curSn);


		if (bSmooth)
		{
			vec_t nrm[4];
			nrm[0] = vec(prevCs, 0.f, prevSn);
			nrm[1] = vec(curCs, 0.f, curSn);
			nrm[2] = vec(prevCs, 0.f, prevSn);
			nrm[3] = vec(curCs, 0.f, curSn);

			nrm[0].normalize();
			nrm[1].normalize();
			nrm[2].normalize();
			nrm[3].normalize();
			pushVertex(vts[0], vts[1], vts[3], vts[2],
				nrm[0], nrm[1], nrm[3], nrm[2] );
		}
		else
		{
			vec_t nrm;
			nrm = vec(prevCs + curCs, 0.f, prevSn + curSn);
			nrm.normalize();
			pushTextVertex(vts[0], vts[1], vts[3], vts[2], nrm );
		}


		pushTextQuad(sublocal, false);
		sublocal+=4;
		prevNg = ng;
		if (i == bypassStart)
			i+= bypassCount-1;
		if (i == bypassStart2)
			i+= bypassCount2-1;

	}

	//caps 1

	int first = vtav;
	int curNbSide = 0;
	for (int i=0;i<nbSides;i++)
	{
		float ng = ((2*PI)/(float)nbSides) * (float)i + PI*0.25f;;
		mVts[vtav++].set(vec(cosf(ng)*0.5f, 0.5f, sinf(ng)*0.5f), vec(0.f, 1.f, 0.f));
		curNbSide ++;
		if (i == bypassStart)
			i+= bypassCount-1;
		if (i == bypassStart2)
			i+= bypassCount2-1;
	}

	for (int i=0;i<(curNbSide-2);i++)
	{
		tris[triav++] = static_cast<unsigned short>( first+(i)+2 );
		tris[triav++] = static_cast<unsigned short>( first+(i)+1 );
		tris[triav++] = static_cast<unsigned short>( first );
/*
		if (i == bypassStart)
			i+= bypassCount-4;
			*/
	}

	// caps 2
	first = vtav;
	curNbSide = 0;
	for (int i=0;i<nbSides;i++)
	{
		float ng = ((2*PI)/(float)nbSides) * (float)i + PI*0.25f;;
		mVts[vtav++].set(vec(cosf(ng)*0.5f, -0.5f, sinf(ng)*0.5f), vec(0.f, -1.f, 0.f));
		curNbSide ++;
		if (i == bypassStart)
			i+= bypassCount-1;
		if (i == bypassStart2)
			i+= bypassCount2-1;
	}

	for (int i=0;i<(curNbSide-2);i++)
	{
		tris[triav++] = static_cast<unsigned short>( first );
		tris[triav++] = static_cast<unsigned short>( first+(i)+1 );
		tris[triav++] = static_cast<unsigned short>( first+(i)+2 );
	}

	return pushNewMesh();
}

mesh_t* generateCone(int nbSides, int bypassStart, int bypassCount, int bypassStart2, int bypassCount2)
{
	vtav = 0;
	triav = 0;
	int sublocal = 0;


	float prevNg = PI*0.25f;
	for (int i=1;i<=nbSides;i++)
	{
		float ng = ((2*PI)/(float)nbSides) * (float)i + PI*0.25f;
		float prevCs = cosf(prevNg)*0.5f;
		float prevSn = sinf(prevNg)*0.5f;
		float curCs = cosf(ng)*0.5f;
		float curSn = sinf(ng)*0.5f;

		vec_t vts[3];
		vts[0] = vec(prevCs, -0.5f, prevSn);
		vts[1] = vec(curCs, -0.5f, curSn);
		vts[2] = vec(0.f, 0.5f, 0.f);

		vec_t nrm = vec(prevCs + curCs, 1.f, prevSn + curSn);
		nrm.normalize();

		mVts[vtav++].set(vts[0], nrm);
		mVts[vtav++].set(vts[1], nrm);
		mVts[vtav++].set(vts[2], nrm);

		tris[triav++] = static_cast<unsigned short>( sublocal );
		tris[triav++] = static_cast<unsigned short>( sublocal+1 );
		tris[triav++] = static_cast<unsigned short>( sublocal+2 );
		sublocal+=3;
		prevNg = ng;
		if (i == bypassStart)
			i+= bypassCount-1;
		if (i == bypassStart2)
			i+= bypassCount2-1;
	}

	// caps 1
	int first = vtav;
	int curNbSide = 0;
	for (int i=0;i<nbSides;i++)
	{
		float ng = ((2*PI)/(float)nbSides) * (float)i + PI*0.25f;;
		mVts[vtav++].set(vec(cosf(ng)*0.5f, -0.5f, sinf(ng)*0.5f), vec(0.f, -1.f, 0.f));
		curNbSide ++;
		if (i == bypassStart)
			i+= bypassCount-1;
		if (i == bypassStart2)
			i+= bypassCount2-1;
	}

	for (int i=0;i<(curNbSide-2);i++)
	{
		tris[triav++] = static_cast<unsigned short>( first );
		tris[triav++] = static_cast<unsigned short>( first+(i)+1 );
		tris[triav++] = static_cast<unsigned short>( first+(i)+2 );
	}
	return pushNewMesh();
}

mesh_t* generateLightCone(int nbSides)
{
	vtav = 0;
	triav = 0;
	//int sublocal = 0;


	//float prevNg = PI*0.25f;
	for (int i=1;i<=nbSides;i++)
	{
		float ng = ((2*PI)/(float)nbSides) * (float)i + PI*0.25f;
		float curCs = cosf(ng)*0.5f;
		float curSn = sinf(ng)*0.5f;

		vec_t vt = vec(curCs, curSn, 1.f);


		mVts[vtav++].set(vt, vec(0.f, 0.f, 1.f) );


	}
	mVts[vtav++].set(vec(0.f), vec(0.f, 0.f, 1.f) );

	// caps 1
	for (int i=0;i<(nbSides-2);i++)
	{
		tris[triav++] = 0;
		tris[triav++] = static_cast<unsigned short>( i+1 );
		tris[triav++] = static_cast<unsigned short>( i+2 );
	}
	// caps 2
	for (int i=0;i<nbSides;i++)
	{
		tris[triav++] = static_cast<unsigned short>( (i+2) % nbSides );
		tris[triav++] = static_cast<unsigned short>( (i+1) % nbSides );
		tris[triav++] = static_cast<unsigned short>( vtav-1 );
	}
	return pushNewMesh();
}

mesh_t* generateHexagon(float width, float height, float lateral)
{
	vtav = 0;
	triav = 0;

	// front
	vec_t nrmf = vec(0.f, 0.f, 1.f);
	mVts[vtav++].set(vec(-width*0.5f, 0.f, 0.5f), nrmf);
	mVts[vtav++].set(vec(-width*0.5f + lateral, height * 0.5f, 0.5f), nrmf);
	mVts[vtav++].set(vec( width*0.5f - lateral, height * 0.5f, 0.5f), nrmf);
	mVts[vtav++].set(vec( width*0.5f, 0.f, 0.5f), nrmf);
	mVts[vtav++].set(vec( width*0.5f - lateral, -height * 0.5f, 0.5f), nrmf);
	mVts[vtav++].set(vec(-width*0.5f + lateral, -height * 0.5f, 0.5f), nrmf);

	// back
	vec_t nrmb = vec(0.f, 0.f,  -1.f);
	mVts[vtav++].set(vec(-width*0.5f, 0.f, -0.5f), nrmb);
	mVts[vtav++].set(vec(-width*0.5f + lateral, height * 0.5f, -0.5f), nrmb);
	mVts[vtav++].set(vec( width*0.5f - lateral, height * 0.5f, -0.5f), nrmb);
	mVts[vtav++].set(vec( width*0.5f, 0.f, -0.5f), nrmb);
	mVts[vtav++].set(vec( width*0.5f - lateral, -height * 0.5f, -0.5f), nrmb);
	mVts[vtav++].set(vec(-width*0.5f + lateral, -height * 0.5f, -0.5f), nrmb);

	// side
	vec_t norms[6] = { vec(-height*0.5f, lateral, 0.f),
		vec(0.f,1.f,0.f),
		vec(height*0.5f, lateral, 0.f),
		vec(height*0.5f, -lateral, 0.f),
		vec(0.f,-1.f,0.f),
		vec(-height*0.5f, -lateral, 0.f) };

	for (int i=0;i<6;i++)
		norms[i].normalize();

	mVts[vtav++].set(vec(-width*0.5f, 0.f, 0.5f), norms[0]);
	mVts[vtav++].set(vec(-width*0.5f, 0.f, -0.5f), norms[0]);

	mVts[vtav++].set(vec(-width*0.5f + lateral, height * 0.5f, 0.5f), norms[0]);
	mVts[vtav++].set(vec(-width*0.5f + lateral, height * 0.5f, -0.5f), norms[0]);


	mVts[vtav++].set(vec(-width*0.5f + lateral, height * 0.5f, 0.5f), norms[1]);
	mVts[vtav++].set(vec(-width*0.5f + lateral, height * 0.5f, -0.5f), norms[1]);

	mVts[vtav++].set(vec( width*0.5f - lateral, height * 0.5f, 0.5f), norms[1]);
	mVts[vtav++].set(vec( width*0.5f - lateral, height * 0.5f, -0.5f), norms[1]);


	mVts[vtav++].set(vec( width*0.5f - lateral, height * 0.5f, 0.5f), norms[2]);
	mVts[vtav++].set(vec( width*0.5f - lateral, height * 0.5f, -0.5f), norms[2]);

	mVts[vtav++].set(vec( width*0.5f, 0.f, 0.5f), norms[2]);
	mVts[vtav++].set(vec( width*0.5f, 0.f, -0.5f), norms[2]);

	mVts[vtav++].set(vec( width*0.5f, 0.f, 0.5f), norms[3]);
	mVts[vtav++].set(vec( width*0.5f, 0.f, -0.5f), norms[3]);

	mVts[vtav++].set(vec( width*0.5f - lateral, -height * 0.5f, 0.5f), norms[3]);
	mVts[vtav++].set(vec( width*0.5f - lateral, -height * 0.5f, -0.5f), norms[3]);

	mVts[vtav++].set(vec( width*0.5f - lateral, -height * 0.5f, 0.5f), norms[4]);
	mVts[vtav++].set(vec( width*0.5f - lateral, -height * 0.5f, -0.5f), norms[4]);

	mVts[vtav++].set(vec(-width*0.5f + lateral, -height * 0.5f, 0.5f), norms[4]);
	mVts[vtav++].set(vec(-width*0.5f + lateral, -height * 0.5f, -0.5f), norms[4]);


	mVts[vtav++].set(vec(-width*0.5f + lateral, -height * 0.5f, 0.5f), norms[5]);
	mVts[vtav++].set(vec(-width*0.5f + lateral, -height * 0.5f, -0.5f), norms[5]);
	mVts[vtav++].set(vec(-width*0.5f, 0.f, 0.5f), norms[5]);
	mVts[vtav++].set(vec(-width*0.5f, 0.f, -0.5f), norms[5]);


	// front
	tris[triav++] = 5;
	tris[triav++] = 1;
	tris[triav++] = 0;

	tris[triav++] = 4;
	tris[triav++] = 2;
	tris[triav++] = 1;

	tris[triav++] = 5;
	tris[triav++] = 4;
	tris[triav++] = 1;

	tris[triav++] = 4;
	tris[triav++] = 3;
	tris[triav++] = 2;

	// back
	tris[triav++] = 0 + 6;
	tris[triav++] = 1 + 6;
	tris[triav++] = 5 + 6;

	tris[triav++] = 1 + 6;
	tris[triav++] = 2 + 6;
	tris[triav++] = 4 + 6;

	tris[triav++] = 1 + 6;
	tris[triav++] = 4 + 6;
	tris[triav++] = 5 + 6;

	tris[triav++] = 2 + 6;
	tris[triav++] = 3 + 6;
	tris[triav++] = 4 + 6;

	// side
	for (int i=0;i<6;i++)
	{

	    tris[triav++] = static_cast<unsigned short>( 2 + 12 + i*4 );
	    tris[triav++] = static_cast<unsigned short>( 1 + 12 + i*4 );
	    tris[triav++] = static_cast<unsigned short>( 0 + 12 + i*4 );


	    tris[triav++] = static_cast<unsigned short>( 1 + 12 + i*4 );
	    tris[triav++] = static_cast<unsigned short>( 2 + 12 + i*4 );
	    tris[triav++] = static_cast<unsigned short>( 3 + 12 + i*4 );

	}

	return pushNewMesh();
}

void geodesic(const vec_t& p1, const vec_t& p2, const vec_t& p3,
			  const vec_t& center, int curLevel, int desiredLevel, int &subLocal, bool bSmoothedNormals = false)
{
	if (curLevel == desiredLevel)
	{

        if (bSmoothedNormals)
        {
            mVts[vtav++].set( p1, normalized(p1) );
            mVts[vtav++].set( p2, normalized(p2) );
            mVts[vtav++].set( p3, normalized(p3) );
        }
        else
        {
            vec_t d1, d2, norm;
		    d1 = p2 - p1;
		    d2 = p3 - p1;
		    d1.normalize();
		    d2.normalize();
		    norm.cross(d1, d2);
		    norm.normalize();

		    mVts[vtav++].set(p1, norm);
		    mVts[vtav++].set(p2, norm);
		    mVts[vtav++].set(p3, norm);
        }



		tris[triav++] = static_cast<unsigned short>( subLocal+0 );
		tris[triav++] = static_cast<unsigned short>( subLocal+1 );
		tris[triav++] = static_cast<unsigned short>( subLocal+2 );
		subLocal += 3;
	}
	else
	{
		vec_t borders[3];
		borders[0] = (p1 + p2) * 0.5f - center;
		borders[1] = (p2 + p3) * 0.5f - center;
		borders[2] = (p3 + p1) * 0.5f - center;

		float len1 = (p1 - center).length();
		float len2 = (p2 - center).length();
		float len3 = (p3 - center).length();
		borders[0].normalize();
		borders[0] *= (len1 + len2) * 0.5f;
		borders[0] += center;
		borders[1].normalize();
		borders[1] *= (len2 + len3) * 0.5f;
		borders[1] += center;
		borders[2].normalize();
		borders[2] *= (len3 + len1) * 0.5f;
		borders[2] += center;

		geodesic(p1, borders[0], borders[2],
			  center, curLevel + 1, desiredLevel, subLocal, bSmoothedNormals);
		geodesic(borders[0], p2, borders[1],
			  center, curLevel + 1, desiredLevel, subLocal, bSmoothedNormals);

		geodesic(borders[1], p3, borders[2],
			  center, curLevel + 1, desiredLevel, subLocal, bSmoothedNormals);

		geodesic(borders[0], borders[1], borders[2],
			  center, curLevel + 1, desiredLevel, subLocal, bSmoothedNormals);


	}
}

void geodesicBorder(const vec_t& p1, const vec_t& p2, const vec_t& p3, const vec_t& p4,
			  const vec_t& center, int curLevel, int desiredLevel, int &subLocal,
			  bool bCap = false, bool bSmoothedNormals = false)
{
	if (curLevel == desiredLevel)
	{
		vec_t d1, d2, norm;
		d1 = p2 - p1;
		d2 = p4 - p1;
		d1.normalize();
		d2.normalize();
		norm.cross(d1, d2);
		norm.normalize();

		pushTextVertex(p4, p3, p2, p1, norm );
		pushTextQuad(subLocal, false);
		subLocal += 4;
		if (bCap)
		{
			norm = -d2;
			norm.normalize();
			mVts[vtav++].set(center, norm);
			mVts[vtav++].set(p2, norm);
			mVts[vtav++].set(p1, norm);
			tris[triav++] = static_cast<unsigned short>( subLocal+0 );
			tris[triav++] = static_cast<unsigned short>( subLocal+1 );
			tris[triav++] = static_cast<unsigned short>( subLocal+2 );
			subLocal += 3;
		}
	}
	else
	{

		vec_t borders[2], tt;
		tt = borders[0] = (p1 + p2) * 0.5f;
		borders[0] = (p1 + p2) * 0.5f - center;
		borders[1] = (p3 + p4) * 0.5f;

		//float bordlen = borders[0].length();

		float len1 = (p1 - center).length();
		float len2 = (p2 - center).length();
		borders[0].normalize();
		borders[0] *= (len1 + len2) * 0.5f;
		borders[0] += center;

		borders[1] += (borders[0] -tt);

		geodesicBorder(p1, borders[0], borders[1], p4,
			  center, curLevel + 1, desiredLevel, subLocal, bCap, bSmoothedNormals );

		geodesicBorder( borders[0], p2, p3, borders[1],
			  center, curLevel + 1, desiredLevel, subLocal, bCap, bSmoothedNormals );


	}
}

mesh_t* generateSuper( float bx, float by, float bz, int level,
					  bool bSimx, bool bSimy, bool bSimz, bool bSmoothedNormals )
{
	float lx,ly,lz;
	lx = ly = lz = 0.5f;
	float straightx = lx -  bx;
	float straighty = ly -  by;
	float straightz = lz -  bz;

	level = (level <=0)?1:level;
	vtav = 0;
	triav = 0;
	int sublocal = 0;

	// straight

	if (straightx > FLOAT_EPSILON)
	{
		vec_t vts[4];
		vts[0] = vec(0.f, 0.f, lz);
		vts[1] = vec(straightx, 0.f, lz);
		vts[2] = vec(straightx, straighty, lz);
		vts[3] = vec(0.f, straighty, lz);

		pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(0.f, 0.f, 1.f) );
		pushTextQuad(sublocal, true);
		sublocal+= 4;
	}

	if (straightz > FLOAT_EPSILON)
	{
		vec_t vts[4];
		vts[0] = vec(lx, 0.f, 0.f);
		vts[1] = vec(lx, 0.f, straightz);
		vts[2] = vec(lx, straighty, straightz);
		vts[3] = vec(lx, straighty, 0.f);

		pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(1.f, 0.f, 0.f) );
		pushTextQuad(sublocal, false);
		sublocal+= 4;
	}
	if (straighty > FLOAT_EPSILON)
	{
		vec_t vts[4];
		vts[0] = vec(0.f, ly, 0.f);
		vts[1] = vec(0.f, ly, straightz);
		vts[2] = vec(straightx, ly, straightz);
		vts[3] = vec(straightx, ly, 0.f);

		pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(0.f, 1.f, 0.f) );
		pushTextQuad(sublocal, true);
		sublocal+= 4;
	}


	// rounded

	if ( bx > FLOAT_EPSILON && bz>FLOAT_EPSILON)
	{
		vec_t vts[4];
		vts[3] = vec(straightx + bx, 0.f, straightz );
		vts[2] = vec(straightx  , 0.f, straightz +  bz);
		vts[1] = vec(straightx  , straighty, straightz +  bz);
		vts[0] = vec(straightx + bx, straighty, straightz );

		bool bCap = (by<=FLOAT_EPSILON);
		geodesicBorder(vts[0], vts[1], vts[2], vts[3],
			  vec(straightx, straighty, straightz), 1, level, sublocal, bCap, bSmoothedNormals);

		if (bCap)
		{
			vts[0] = vec(straightx, ly, 0.f);
			vts[1] = vec(straightx, ly, straightz);
			vts[2] = vec(lx, ly, straightz);
			vts[3] = vec(lx, ly, 0.f);

			pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(0.f, 1.f, 0.f) );
			pushTextQuad(sublocal, true);
			sublocal+= 4;


			vts[0] = vec(0.f, ly, lz);
			vts[1] = vec(0.f, ly, straightz);
			vts[2] = vec(straightx, ly, straightz);
			vts[3] = vec(straightx, ly, lz);

			pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(0.f, 1.f, 0.f) );
			pushTextQuad(sublocal, false);
			sublocal+= 4;
		}

	}

	if ( by > FLOAT_EPSILON && bz>FLOAT_EPSILON)
	{
		vec_t vts[4];
		vts[3] = vec(0.f, straighty + by, straightz );
		vts[2] = vec(0.f, straighty , straightz +  bz);
		vts[1] = vec(straightx, straighty , straightz + bz);
		vts[0] = vec(straightx, straighty + by, straightz );

		bool bCap = (bx<=FLOAT_EPSILON);

		geodesicBorder(vts[1], vts[0], vts[3], vts[2],
			  vec(straightx, straighty, straightz), 1, level, sublocal, bCap, bSmoothedNormals);

		if (bCap)
		{
			vts[0] = vec(lx, straighty, 0.f);
			vts[1] = vec(lx, straighty, straightz);
			vts[2] = vec(lx, ly, straightz);
			vts[3] = vec(lx, ly, 0.f);

			pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(1.f, 0.f, 0.f) );
			pushTextQuad(sublocal, false);
			sublocal+= 4;


			vts[0] = vec(lx, 0.f, lz);
			vts[1] = vec(lx, 0.f, straightz);
			vts[2] = vec(lx, straighty, straightz);
			vts[3] = vec(lx, straighty, lz);

			pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(1.f, 0.f, 0.f) );
			pushTextQuad(sublocal, true);
			sublocal+= 4;
		}
	}

	if ( bx > FLOAT_EPSILON && by>FLOAT_EPSILON)
	{
		vec_t vts[4];
		vts[3] = vec(straightx + bx, straighty , 0.f);
		vts[2] = vec(straightx , straighty +  by, 0.f);
		vts[1] = vec(straightx , straighty +  by, straightz);
		vts[0] = vec(straightx +bx, straighty , straightz);

		bool bCap = (bz<=FLOAT_EPSILON);

		geodesicBorder(vts[1], vts[0], vts[3], vts[2],
			  vec(straightx, straighty, straightz), 1, level, sublocal, bCap, bSmoothedNormals);


		if (bCap)
		{
			vts[0] = vec(0.f, straighty, lz);
			vts[1] = vec(straightx, straighty, lz);
			vts[2] = vec(straightx, ly, lz);
			vts[3] = vec(0.f, ly, lz);

			pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(0.f, 0.f, 1.f) );
			pushTextQuad(sublocal, true);
			sublocal+= 4;


			vts[0] = vec(straightx, 0.f, lz);
			vts[1] = vec(lx, 0.f, lz);
			vts[2] = vec(lx, straighty, lz);
			vts[3] = vec(straightx, straighty, lz);

			pushTextVertex(vts[0], vts[1], vts[2], vts[3], vec(0.f, 0.f, 1.f) );
			pushTextQuad(sublocal, true);
			sublocal+= 4;
		}
	}

	// quarter sphere
	if (bx > FLOAT_EPSILON && by>FLOAT_EPSILON && bz>FLOAT_EPSILON)
	{
		geodesic(vec(lx, straighty, straightz), vec(straightx, ly, straightz), vec(straightx, straighty, lz),
			  vec(straightx, straighty, straightz), 1, level, sublocal, bSmoothedNormals);

	}
	// symetrie
	mesh_t *nMesh = pushNewMesh();
	int nbIndicesPerSub = nMesh->triCount;
	mesh_t *liste[]={nMesh,nMesh,nMesh,nMesh,nMesh,nMesh,nMesh,nMesh};

	int nbm = (((1<<(bSimx?1:0))<<(bSimy?1:0))<<(bSimz?1:0));

	mesh_t * mergeAll = merge(liste, nbm);

	//int mask = (bSimz?4:0) + (bSimz?2:0) + (bSimz?1:0);

	meshVertex_t *pv = (meshVertex_t *)mergeAll->mVA->Lock( VAL_WRITE );//vertices;
	unsigned short *pi = (unsigned short*)mergeAll->mIA->Lock( VAL_WRITE );
	unsigned short *pi2 = pi;
	int aaidx = 0;


	for (int z = 0;z < (bSimz?2:1) ; z++)
	{
		for (int y = 0; y < (bSimy?2:1) ; y++)
		{
			for (int x = 0;x < (bSimx?2:1) ; x++)
			{
                unsigned count = mergeAll->mVA->GetVertexCount()/nbm;
                for ( unsigned int j = 0 ; j < count ; j++ )
				{
					if (x&1)
					{
						pv->x = -pv->x;
						pv->nx = -pv->nx;
					}
					if (y&1)
					{
						pv->y = -pv->y;
						pv->ny = -pv->ny;
					}
					if (z&1)
					{
						pv->z = -pv->z;
						pv->nz = -pv->nz;
					}

					pv ++;
				}


				if ( aaidx&1 )
				{
					for (int j=0;j<nbIndicesPerSub;j+=3)
					{
						unsigned short tmp = pi[j];
						pi[j] = pi[j+2];
						pi[j+2] = tmp;
					}
				}
				else
				{
					pi += mergeAll->triCount/nbm;
				}
				aaidx ++;

			}
		}
	}

	pi = pi2;


	if (nbm == 8)
	{
		// WTF??? why do I have to do that?
		int adec = 3 * nbIndicesPerSub;
		for (int j=0;j<nbIndicesPerSub;j+=3)
		{
			unsigned short tmp = pi[j + adec];
			pi[j + adec] = pi[j+2 + adec];
			pi[j+2 + adec] = tmp;
		}
		adec = 7 * nbIndicesPerSub;
		for (int j=0;j<nbIndicesPerSub;j+=3)
		{
			unsigned short tmp = pi[j + adec];
			pi[j + adec] = pi[j+2 + adec];
			pi[j+2 + adec] = tmp;
		}
	}

	mergeAll->mVA->Unlock();
	mergeAll->mIA->Unlock();

	return mergeAll;
}

void mesh_t::setBBox( const vec_t &AABBMin, const vec_t& AABBMax)
{
    vec_t mid = (AABBMin+AABBMax)*0.5f;
	float l = (AABBMax-AABBMin).length()*0.5f;
	bSphere = vec(mid.x, mid.y, mid.z, l);

	worldAABBMin = AABBMin;
	worldAABBMax = AABBMax;
}
void mesh_t::computeBSphere()
{
	u8 *pvc = (u8*)mVA->Lock( VAL_READONLY );
	AABBMin = AABBMax = *(vec_t*)pvc;
	int vtS = mVA->GetVertexSize();
	pvc += vtS;

    unsigned int count = mVA->GetVertexCount();
    for ( unsigned int i = 1 ; i < count ; i++ )
	{
		vec_t v = *(vec_t*)pvc;//vec(pv->x, pv->y, pv->z);

		AABBMin.isMinOf(v);
		AABBMax.isMaxOf(v);
		pvc += vtS;
	}
	AABBMin.w = AABBMax.w = 0.f;
    /*
	vec_t mid = (AABBMin+AABBMax)*0.5f;
	float l = (AABBMax-AABBMin).length()*0.5f;
	bSphere = vec(mid.x, mid.y, mid.z, l);

	worldAABBMin = AABBMin;
	worldAABBMax = AABBMax;
    */
    setBBox( AABBMin, AABBMax );
	mVA->Unlock();
}

void mesh_t::updateWorldBSphere()
{
	vec_t wpos = bSphere;//vec(0.f, 0.f, 0.f, 1.f);
	wpos.TransformPoint( mWorldMatrix );
	vec_t sz = vec( 1.f, 1.f, 1.f, 0.f )*bSphere.w;
	sz.TransformVector( mWorldMatrix );
	worldBSphere = vec( wpos.x, wpos.y, wpos.z, zmax( fabsf( sz.x ), zmax( fabsf( sz.y ), fabsf( sz.z ) ) )/*zmax(zmax(sz.x, sz.y), sz.z)*/);

	// world bounding box
	vec_t orientedBox[]={
		vec(AABBMin.x, AABBMin.y, AABBMin.z),
		vec(AABBMax.x, AABBMin.y, AABBMin.z),
		vec(AABBMin.x, AABBMax.y, AABBMin.z),
		vec(AABBMax.x, AABBMax.y, AABBMin.z),
		vec(AABBMin.x, AABBMin.y, AABBMax.z),
		vec(AABBMax.x, AABBMin.y, AABBMax.z),
		vec(AABBMin.x, AABBMax.y, AABBMax.z),
		vec(AABBMax.x, AABBMax.y, AABBMax.z)
	};

	vec_t trt;
	trt.TransformPoint(orientedBox[0], mWorldMatrix);
	worldAABBMin = worldAABBMax = trt;
	for (int i=1;i<8;i++)
	{
		trt.TransformPoint(orientedBox[i], mWorldMatrix);
		worldAABBMin.isMinOf(trt);
		worldAABBMax.isMaxOf(trt);
	}
}

float interf(const vec_t& v1, const vec_t& v2, const vec_t& plan)
{
	float dt1 = fabsf(plan.signedDistanceTo(v1));
	float dt2 = fabsf(plan.signedDistanceTo(v2));

	return (dt1)/(dt1+dt2);
}

void InterpolateVertex(u8 *A, u8* B, u8* res, float t, int aFormat)
{
	float *pA = (float*)A;
	float *pB = (float*)B;
	float *pR = (float*)res;
	if (aFormat&VAF_XYZ)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
	if (aFormat&VAF_XYZRHW)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
	if (aFormat&VAF_NORMAL)
	{

		pR[0] = LERP( (*pA), (*pB++), t); pA++;
		pR[1] = LERP( (*pA), (*pB++), t); pA++;
		pR[2] = LERP( (*pA), (*pB++), t); pA++;
		float l = 1.f/sqrtf(pR[0]*pR[0] + pR[1]*pR[1] + pR[2]*pR[2]);
		pR[0] *= l;
		pR[1] *= l;
		pR[2] *= l;
		pR += 3;

	}
	if (aFormat&VAF_COLOR)
	{

		u8 *pC1 = (u8*)pA;
		u8 *pC2 = (u8*)pB;
		u8 *pCR = (u8*)pR;

		*pCR++ = (u8)LERP( (*pC1), (*pC2++), t); pC1++;
		*pCR++ = (u8)LERP( (*pC1), (*pC2++), t); pC1++;
		*pCR++ = (u8)LERP( (*pC1), (*pC2++), t); pC1++;
		*pCR   = (u8)LERP( (*pC1), (*pC2++), t);
		pA++;
		pB++;
		pR++;

	}
	if (aFormat&VAF_BINORMAL)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
	if (aFormat&VAF_BITANGENT)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
	if (aFormat&VAF_TEX0)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
	if (aFormat&VAF_TEX1)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
	if (aFormat&VAF_TEX2)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
	if (aFormat&VAF_TEX3)
	{
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
		*pR++ = LERP( (*pA), (*pB++), t); pA++;
	}
}

mesh_t *mesh_t::clipPlane(const vec_t &plan)
{
	//meshVertex_t *pVertIn = vertices;
	u8 *pVertIn = (u8*)mVA->Lock( VAL_READONLY );
	int vertS = mVA->GetVertexSize();
	int vertF = mVA->GetFormat();
	unsigned short *pIndIn = (unsigned short *)mIA->Lock( VAL_READONLY );

	unsigned short *pIndDest = tris;
	u8 *pVertDest = (u8*)mVts;
	triav = vtav = 0;

	static const float threshold = 0.0001f;

	for (int i=0;i<triCount/3;i++)
	{
		u8 vts[3][128];
		float dists[3];
		int inFront = 0, inBack = 0;
		int inds[3];
		u8 vtsNew[ 8][ 128];
		int vtsNewAV = 0;
		for (int j = 0;j<3;j++)
		{
			inds[j] = pIndIn[i*3 + j];
			//vts[j] = *(vec_t*)&pVertIn[inds[j] * vertS];
			memcpy( vts[j], &pVertIn[inds[j] * vertS], vertS );
			dists[j] = plan.signedDistanceTo( *(vec_t*)&vts[j] );
			if (dists[j]>-threshold)
				inFront++;
			if (dists[j]<threshold)
				inBack ++;
		}
		//printf("%d\n", inFront);
		if (inBack == 3) continue;
		if (inFront == 3)
		{
			memcpy( pVertDest, &pVertIn[ pIndIn[i*3] * vertS ], vertS );
			pVertDest += vertS;
			memcpy( pVertDest, &pVertIn[ pIndIn[i*3 + 1] * vertS ], vertS );
			pVertDest += vertS;
			memcpy( pVertDest, &pVertIn[ pIndIn[i*3 + 2] * vertS ], vertS );
			pVertDest += vertS;

			*pIndDest++ = static_cast<unsigned short>( vtav++ );
			*pIndDest++ = static_cast<unsigned short>( vtav++ );
			*pIndDest++ = static_cast<unsigned short>( vtav++ );

			triav += 3;
			continue;
		}



		for (int j = 0;j<3;j++)
		{
			int o = (j+1)%3;
			float v1 = plan.signedDistanceTo( *(vec_t*)&vts[j] );
			float v2 = plan.signedDistanceTo( *(vec_t*)&vts[o] );

			if ((v1 >=0.f) && (v2>=0.f))
			{
				//vtsNew[vtsNewAV++] = vts[o];
				memcpy( vtsNew[vtsNewAV++], vts[o] , vertS);

			}
			else if ((v1 >=0.f) && (v2<0.f))
			{
				// interpolate
                //u8 interPos;
				float lp = interf(*(vec_t*)&vts[j], *(vec_t*)&vts[o], plan);
				/*interPos.lerp(vts[j], vts[o], lp);

                vtsNew[vtsNewAV++] = interPos;
				*/
				InterpolateVertex(vts[j], vts[o], vtsNew[vtsNewAV++], lp, vertF);

			}
			else if ((v1 <0.f) && (v2>=0.f))
			{
				// interpolate
                //meshVertex_t interPos;
				float lp = interf(*(vec_t*)&vts[j], *(vec_t*)&vts[o], plan);
				//interPos.lerp(vts[j], vts[o], lp);

				if (lp>=(1.f-FLOAT_EPSILON))
				{
					//vtsNew[vtsNewAV++] = vts[o];
					memcpy( vtsNew[vtsNewAV++], vts[o] , vertS);
					continue;
				}
				InterpolateVertex(vts[j], vts[o], vtsNew[vtsNewAV++], lp, vertF);
                //vtsNew[vtsNewAV++] = interPos;
				//vtsNew[vtsNewAV++] = vts[o];
				memcpy( vtsNew[vtsNewAV++], vts[o] , vertS);
			}
			else if (v2 >=0.f)
			{
				//vtsNew[vtsNewAV++] = vts[o];
				memcpy( vtsNew[vtsNewAV ++], vts[ o ], vertS );
			}
		}

		for (int j=0;j<vtsNewAV;j++)
		{
			//*pVertDest++ = vtsNew[j];
			memcpy( pVertDest, vtsNew[ j ], vertS );
			pVertDest += vertS;
		}

		for (int j=0;j<vtsNewAV-2;j++)
		{
			*pIndDest++ = static_cast<unsigned short>( vtav );
			*pIndDest++ = static_cast<unsigned short>( vtav+j+1 );
			*pIndDest++ = static_cast<unsigned short>( vtav+j+2 );
		}

		triav += (vtsNewAV-2)*3;
		vtav += vtsNewAV;

	}

	mIA->Unlock();
	mVA->Unlock();

	ASSERT_GAME( vtav );
	ASSERT_GAME( triav );

	return pushNewMesh( mVA->GetFormat() );
}

bool mesh_t::rayCast(vec_t& origin, vec_t&dir, float rayLength, vec_t& contactPoint, vec_t& contactNormal)
{
	unsigned short *pInd = (unsigned short*)mIA->Lock( VAL_READONLY );
	matrix_t mt;
	mt.inverse(mWorldMatrix);
	vec_t norigin, ndir;
	norigin.TransformPoint(origin, mt);
	ndir.TransformVector(dir, mt);
	ndir.normalize();
	ndir *= rayLength;
	/*
	norigin = origin;
	ndir = dir*rayLength;
	*/
	float tt = 2.0f;
	u8 *ptrv = (u8 *)mVA->Lock( VAL_READONLY );
	unsigned int vertexSize = mVA->GetVertexSize();

	for (int i=0;i<triCount; i+=3)
	{
		float u, v, t;
		float* v1 = (float*)&ptrv[(*pInd++)*vertexSize];
		float* v2 = (float*)&ptrv[(*pInd++)*vertexSize];
		float* v3 = (float*)&ptrv[(*pInd++)*vertexSize];
		if (intersect_triangle(&norigin.x, &ndir.x,
                   v1, v2, v3,
                   &t, &u, &v))
		{
			if ((t<tt)&&(t>0.f)&&(t<1.f))
			{
				vec_t *pv1 = (vec_t*)v1;
				vec_t *pv2 = (vec_t*)v2;
				vec_t *pv3 = (vec_t*)v3;

				vec_t d1 = (*pv3)-(*pv1);
				vec_t d2 = (*pv2)-(*pv1);

				vec_t crs;
				crs.cross(d2, d1);
				crs.w = 0.f;
				crs.normalize();

				contactNormal = crs;
				tt = t;
			}
		}
	}
	ndir *= tt;
	ndir.TransformVector(mWorldMatrix);
	contactPoint = origin + ndir;

	mVA->Unlock();
	mIA->Unlock();

	return (tt<2.0f);
}


int pushDelaunayIndices(u8 *vts, int nbVts, int nvStride, unsigned short *pTris, int &tav, int indexOffset, bool inverseWinding)
{
    int numTriangles = 0;
	TriangleList* triangle = NULL;
	int ret = 0;
	/*int res = */Delaunay(nbVts, vts, nvStride, numTriangles, triangle);
	//int tst = 0;
	if (numTriangles)
	{
		for (int i=0;i<numTriangles;i++)
		{
			vec_t mid = ( (*(vec_t*)&vts[triangle[i][0]*nvStride]) +
				(*(vec_t*)&vts[triangle[i][1]*nvStride]) +
				(*(vec_t*)&vts[triangle[i][2]*nvStride])) * 0.33333f;
			int cutCount = 0;
			for (int j=0;j<nbVts;j++)
			{
				vec_t res;
				if (segment2segmentXY(
					(*(vec_t*)&vts[j*nvStride]),
					(*(vec_t*)&vts[((j+1)%nbVts)*nvStride]),
					mid, mid+vec(0.f, -500.f, 0.f), res))
					cutCount ++;
			}

			//++tst &= 1;
			if ( cutCount&1 )
			{
				if ( inverseWinding )
				{
					pTris[tav++] = static_cast<unsigned short>( triangle[i][0] + indexOffset );
					pTris[tav++] = static_cast<unsigned short>( triangle[i][1] + indexOffset );
					pTris[tav++] = static_cast<unsigned short>( triangle[i][2] + indexOffset );
				}
				else
				{
					pTris[tav++] = static_cast<unsigned short>( triangle[i][2] + indexOffset );
					pTris[tav++] = static_cast<unsigned short>( triangle[i][1] + indexOffset );
					pTris[tav++] = static_cast<unsigned short>( triangle[i][0] + indexOffset );
				}
				ret ++;
			}
		}
		delete [] triangle;
        triangle = NULL;
    }
    return ret;
}

mesh_t* generateDelaunay(vec_t *vts, int nbVts)
{
	for (int i=0;i<nbVts;i++)
	{
		mVts[i].setPosition(vts[i]);
	}
	vtav = nbVts;
	triav = 0;
    if (pushDelaunayIndices((u8*)vts, nbVts, sizeof(vec_t), tris, triav))
    {
		return pushNewMesh();
	}
	return NULL;
}

void pushNewMeshInit()
{
	vtav = 0;
	triav = 0;
}
void pushTrunk(const matrix_t & m1, const matrix_t & m2, bool bEndCapped)
{

	int sublocal = vtav;
	vec_t vts[8];
	vts[0] = vec(-0.5f, -0.0f, -0.5f);
	vts[1] = vec(-0.5f, -0.0f, 0.5f);
	vts[2] = vec(0.5f, -0.0f, -0.5f);
	vts[3] = vec(0.5f, -0.0f, 0.5f);
	vts[4] = vec(-0.5f, 0.0f, -0.5f);
	vts[5] = vec(-0.5f, 0.0f, 0.5f);
	vts[6] = vec(0.5f, 0.0f, -0.5f);
	vts[7] = vec(0.5f, 0.0f, 0.5f);

	for (int i=0;i<4;i++)
		vts[i].TransformPoint(m1);
	for (int i=4;i<8;i++)
		vts[i].TransformPoint(m2);

	//pushTextVertex(vts[0], vts[1], vts[3], vts[2], MakeNormal( vts[0], vts[1], vts[3]).normalize() );
	//pushTextVertex(vts[4], vts[5], vts[7], vts[6], MakeNormal( vts[4], vts[5], vts[7]).normalize() );
	pushTextVertex(vts[0], vts[1], vts[5], vts[4], MakeNormal( vts[0], vts[1], vts[5]).normalize() );
	pushTextVertex(vts[2], vts[3], vts[7], vts[6], MakeNormal( vts[7], vts[3], vts[2]).normalize() );
	pushTextVertex(vts[0], vts[2], vts[6], vts[4], MakeNormal( vts[6], vts[2], vts[0]).normalize() );
	pushTextVertex(vts[1], vts[3], vts[7], vts[5], MakeNormal( vts[1], vts[3], vts[7]).normalize() );

    if (bEndCapped)
        pushTextVertex(vts[4], vts[5], vts[7], vts[6], MakeNormal( vts[4], vts[5], vts[7]).normalize() );

	/*
	pushTextQuad(sublocal, false);
	sublocal+=4;
	pushTextQuad(sublocal, true);
	sublocal+=4;
	*/
	pushTextQuad(sublocal, true);
	sublocal+=4;
	pushTextQuad(sublocal, false);
	sublocal+=4;


	pushTextQuad(sublocal, false);
	sublocal += 4;
	pushTextQuad(sublocal, true);

    if (bEndCapped)
    {
        sublocal += 4;
        pushTextQuad(sublocal, true);
    }

}
