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

#ifndef TRACK_H__
#define TRACK_H__

#define NBSEGS 32
#define NBSEGS_MINIMALCURVE (NBSEGS*4)
#define UNASSIGNED_BRICK_INDEX 9999999

#include "mesh.h"
#include "game.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct skyScatParams_t : public serialisableObject_t
{
    SERIALIZABLE(skyScatParams_t,"Sky cube parameters")

	skyScatParams_t()
	{
		mKr = vec( (float)(0x30)/255.f, (float)(0x7f)/255.f, (float)(0xa9)/255.f );
		mRayleighBrightness = 3.3f; // /10
		mMieBrightness = 0.1f; // /1000
		mSpotBrightness = 1000.f;
		mScatterStrength = 0.028f; // /1000
		mRayleighStrength = 0.139f; // /1000
		mMieStrength = 0.0264f; // /10000
		mRayleighCollectionPower = 0.81f; // /100
		mMieCollectionPower = 0.39f; // /100
		mMieDistribution = 0.63f; // /100
	    mSunDirection = normalized( vec(0.f, 0.8f, 1.0f, 0.f) ); 
	    mSunColor = vec(1.f, 1.f, 1.f, 1.f); 

        mFogDensity = 0.001f;
        mFogColor = vec(0.6f, 0.9f, 0.6f);

        mFarPlane = 1000.f;

        mbIsDirty = true;
        mbDrawOcean = true;
	}
    virtual void FieldsHasBeenModified() { mSunDirection.normalize(); mbIsDirty = true; } // flag your object as dirty/need update
    virtual bool ObjectIsDirty() const { return mbIsDirty; } // object is still dirty/needs update
/*
        shading_mix: 5,
        specularity: 45,
        reflectivity: 20,
        scatter_strength: 28,
        mie_distribution: 63,
        rayleigh: 33,
        mie: 100,
        spot: 1000,
        rayleigh_strength: 139,
        mie_strength: 264,
        rayleigh_collected: 81,
        mie_collected: 39,

mars: ({"shading_mix":63,"specularity":63,"reflectivity":25,"color":"#a98232","scatter_strength":0.054,"mie_distribution":0.74,"rayleigh":19,"mie":44,"spot":373,"rayleigh_strength":359,"mie_strength":308,"rayleigh_collected":81,"mie_collected":39}),
    venus: ({"shading_mix":63,"specularity":63,"reflectivity":25,"color":"#aa924b","scatter_strength":140,"mie_distribution":81,"rayleigh":25,"mie":124,"spot":0,"rayleigh_strength":397,"mie_strength":298,"rayleigh_collected":34,"mie_collected":76}),
    uranus: ({"shading_mix":63,"specularity":63,"reflectivity":25,"color":"#4586e3","scatter_strength":18,"mie_distribution":56,"rayleigh":80,"mie":67,"spot":0,"rayleigh_strength":136,"mie_strength":68,"rayleigh_collected":71,"mie_collected":0}),
    alien: ({"shading_mix":5,"specularity":45,"reflectivity":20,"color":"#3f885a","scatter_strength":26,"mie_distribution":86,"rayleigh":44,"mie":60,"spot":0,"rayleigh_strength":169,"mie_strength":139,"rayleigh_collected":71,"mie_collected":46}),
*/

	vec_t mKr;
	float mRayleighBrightness;
	float mMieBrightness;
	float mSpotBrightness;
	float mScatterStrength;
	float mRayleighStrength;
	float mMieStrength;
	float mRayleighCollectionPower;
	float mMieCollectionPower;
	float mMieDistribution;
    bool mbIsDirty;

	vec_t mSunDirection;
	vec_t mSunColor;

    float mFogDensity;
    vec_t mFogColor;

    float mFarPlane;

    bool mbDrawOcean;
}skyScatParams_t;

struct shapePattern_t;
struct shapeSequence_t;

struct PerSliceInfo_t
{
    float width;
    float borderForced;//0 = not forced >=0.5 = forced
};



typedef struct AIPoint_t
{
    AIPoint_t(const vec_t& aIPos, const vec_t& aIRight, const vec_t& aIUp, const vec_t& aIDir, float width );
    
    AIPoint_t(const vec_t& aIPos, const vec_t& aIRight, const vec_t& aIUp, const vec_t& aIDir, const vec_t& railLeft, const vec_t& railRight, float width,
              const vec_t& aIPos2, const vec_t& aIRight2, const vec_t& aIUp2, const vec_t& aIDir2, const vec_t& railLeft2, const vec_t& railRight2, float width2);

    void BuildMatrix(matrix_t& dest, bool bFourche) const
    {
        int idx = bFourche?1:0;
        dest.set(mAIRight[idx], mAIUp[idx], mAIDir[idx], mAIPos[idx]);
        dest.position.w = 1.f;
    }
    vec_t mAIPos[2], mAIRight[2], mAIUp[2], mAIDir[2], mRailLeft[2], mRailRight[2];

    float mAIwidth[2];

    bool hasFourche;
    bool bFourcheStarting, bFourcheEnding;
    bool bInsideHole;
    unsigned int topology[2];
} AIPoint_t;




typedef struct trackSeg_t : public serialisableObject_t
{
    SERIALIZABLE( trackSeg_t, "" )

        trackSeg_t()
    {
        border[0] = border[1] = false;
		groundLock[0] = groundLock[1] = true;
		positionLock[0] = positionLock[1] = false;
    }
    vec_t pt;
    vec_t force;
    vec_t pressure;
    vec_t pressure2;

    // fourches
    bool mbHasFourche;
    vec_t force2;
    vec_t point2;


    vec_t up[2];
    vec_t right[2];

    float width[2];

    bool border[2];
	bool groundLock[2];
	bool positionLock[2];
    // physic tris
    /*
 unsigned short *physicIndices;
 vec_t *		physicVertices;
 int				nbPhysicTris;
 */
    vec_t minCurvePoint[2];
    vec_t minCurveRight[2];

    matrix_t GetMat( bool bFourche )
    {
        matrix_t res;
        int idx = bFourche?1:0;

        res.dir.cross( up[idx], right[idx] );
        res.dir.normalize();

        res.up = up[idx];
        res.right = right[idx];
        res.position = bFourche?point2:pt;
        res.position.w = 1.f;

        return res;
    }
} trackSeg_t;

class TrackTopology;

#define MAX_NB_TRACKS 32
#define NBTracks MAX_NB_TRACKS

typedef struct hole_t : public serialisableObject_t
{
    SERIALIZABLE(hole_t,"An hole.")

    float holeStartDistance;
    float holeLength;
} hole_t;

struct prefabInstance_t : public serialisableObject_t
{
	SERIALIZABLE(prefabInstance_t,"Prefab instance.")

	prefabInstance_t() {}
	prefabInstance_t( mesh_t *mesh, const std::string &prefabFile, const matrix_t &mat ) :
	mMesh( mesh ), mPrefabFile( prefabFile ), mPrefabMatrix( mat ) 
	{
	}
	virtual ~prefabInstance_t() {}
	mesh_t *mMesh;
	std::string mPrefabFile;
	matrix_t mPrefabMatrix;
};

typedef struct track_t : public serialisableObject_t
{
    SERIALIZABLE(track_t,"Track information.")


    track_t();
    virtual ~track_t() {}

    virtual void FieldsHasBeenModified() { mbIsDirty = true; } // flag your object as dirty/need update
    virtual bool ObjectIsDirty() const { return mbIsDirty; } // object is still dirty/needs update

    virtual const char *GetObjectName() const { return mName.c_str(); }
    std::string mName;


    // Bonus
    bonusGenerationParameters_t mBonusGenerationParameters;
    // sky 
    skyScatParams_t mSkyScatteringParameters;
    // color
    u32 color1, color2, color3, color4, color5;
    // envirronment
    int mEnvironmentIndex;
    // bricks
    bool mbOnlyDefaultBrick;
    // rock gradiant
    int mRockGradiant;

    typedef struct fourche_t
    {
        int start;
        int end;
    } fourche_t;


    
    void (*TrackSetupFunction)(TrackTopology&, matrix_t*, matrix_t* );
    shapePattern_t *mDefaultShapePattern;

    bool mbIsDirty;

    mesh_t *mMiniMesh;

    unsigned int seed;
    float trackLength;
    float YTranslation;

    int nbHoles;
    hole_t holes[8];

    int nbFourches;
    fourche_t fourches[5];


	std::vector<prefabInstance_t> mPrefabs;


    // track control points
    trackSeg_t segs[NBSEGS];

    // manage segments
    void BuildT0(float length);
    void ApplyPressures(int segSize, int decal, float strength);
    void ApplyPressureModifier();
    void ApplyMaxCurveModifier();
    void ApplyMaxSlopeModifier(float aSlope);
    void ApplyDistModifier(float dist);
    void ApplyForces();
    void ApplyAntiCover(float trackLength, float segDist, float minimalDist);
    void SmoothInclinaison(const vec_t &vm1, const vec_t& v, vec_t& vp1, float threshold, float redresseRatio);
    void ApplyFourcheModifiers();
    void limiteEcart(int idx, vec_t symetricalPlan, vec_t segRight);
    void ComputeUpForSpeed();
    // segments init
    void ComputeSegments();
    void UpdateSegments( float decalUp = 0.f );

    // handle track
    vec_t getPointOnTrack( int segTrackNum, float interp, bool bFourche );
    float getWidthOnTrack( int segTrackNum, float interp, bool bFourche );
    float getBorderForcedOnTrack( int segTrackNum, float interp, bool bFourche );
	float getGroundLockOnTrack( int segTrackNum, float interp, bool bFourche );

    // hole
    int isInHole(float aPos, float aSegLen);
    float findNearestHole(float currentDistance, float &holeLength);
    float shortestDistanceToHoleForthAndBack(float currentDistance);
    bool holeOnTheWay( float aPos, float aCheckLength);

    // bias from 0 (flat) to N. 1 = 45degrees
    float computeDiveHeight( float distToHole, float holeLength, float bias)
    {
        float dist = Clamp( (holeLength - distToHole*0.99f), 0.f, holeLength);
        float r =smootherstep( 0.20f, 1.1f, dist/holeLength);
        return  r * holeLength * bias;
    }
    void DumpMatrices( matrix_t *pMats, bool bFourche, float distance, int nbMatsMax, PerSliceInfo_t *psi );


    void ComputeBounds( vec_t & boundMin, vec_t & boundMax );

} track_t;

extern track_t *Tracks;


///////////////////////////////////////////////////////////////////////////////////////////////////
// topology



class TrackTopology
{
public:
    TrackTopology() {}
    ~TrackTopology() {}

    void GetBiggestContinuousCount( u32 desiredTopology, bool bStrict, bool bFourche, unsigned int &start, unsigned int &count )
    {
        count = 0;
        const std::vector<trackTopology_t>& vv = mCurrentToopology[bFourche?1:0];

        if (start >= vv.size())
            return;

        for (unsigned int i=start;i<vv.size();i++)
        {
            bool stricted = ( vv[i].trackSliceProperties == desiredTopology);
            bool nonStricted = ( (vv[i].trackSliceProperties&desiredTopology )!=0);
            bool bChecked = bStrict?stricted:nonStricted;
            if ( bChecked )
            {
                unsigned int tmpCount = 0;
                unsigned int tmpStart = i;
                for (;i<vv.size();i++)
                {
                    tmpCount ++;
                    if ( ( vv[i].trackSliceProperties&desiredTopology ) != desiredTopology )
                        break;
                }
                if ( tmpCount > count)
                {
                    count = tmpCount;
                    start = tmpStart;
                }
            }
        }
    }

    void BuildTopology( bool bFourche, const matrix_t *pMats, int nbMats, PerSliceInfo_t *psi, track_t& tr);

    void InitWithDefault(shapePattern_t* defaultPattern, bool bFourche, int count, PerSliceInfo_t *psi )
    {
        std::vector<trackTopology_t>& vv = mCurrentToopology[bFourche?1:0];

        vv.resize( count );
        for (unsigned int i=0;i<vv.size();i++)
        {
            vv[i].pattern = defaultPattern;
            vv[i].trackSliceProperties = TOPO_USED;
            vv[i].colorLight = vv[i].colorDark = vec( 1.f, 1.f, 1.f, 0.5f );
            vv[i].leftRight = 0.f;
            vv[i].upDown = 0.f;
            vv[i].width = psi[i].width;
            vv[i].borderForced = psi[i].borderForced;
        }
    }

    void FillTopologyWithSequence( bool bFourche, shapeSequence_t *pSeq, int start, int count );

    const shapePattern_t* pattern (bool bFourche, int idx) const { return mCurrentToopology[bFourche?1:0][idx].pattern; }
    const vec_t& colorLight(bool bFourche, int idx) const { return mCurrentToopology[bFourche?1:0][idx].colorLight; }
    const vec_t& colorDark(bool bFourche, int idx) const { return mCurrentToopology[bFourche?1:0][idx].colorDark; }
    bool turningRight(bool bFourche, int idx) const { return (mCurrentToopology[bFourche?1:0][idx].trackSliceProperties&TOPO_TURNING_RIGHT) != 0 ; }
    bool turningLeft(bool bFourche, int idx) const { return (mCurrentToopology[bFourche?1:0][idx].trackSliceProperties&TOPO_TURNING_RIGHT) != 0 ; }
    float width(bool bFourche, int idx) const { return mCurrentToopology[bFourche?1:0][idx].width; }
    float wallColorRedAlpha(bool bFourche, int idx) const { return mCurrentToopology[bFourche?1:0][idx].wallColorRedAlpha; }
    
    
    enum TRACKTOPOLOGIES
    {
        TOPO_USED = 0,
        TOPO_INGROUND = 1 << 0,
        TOPO_HOLEINOUT = 1 << 1,
        TOPO_HOLE = 1 << 2,
        TOPO_HIGHSLOPE = 1 << 3,
        TOPO_FOURCHE = 1 << 4,
        TOPO_STRAIGHT = 1 << 5,
        TOPO_STARTUP = 1 << 6,
        TOPO_TRANSPARENT = 1 << 7,
        TOPO_COLORSET = 1 << 8,
        TOPO_TURNING_LEFT = 1<< 9,
        TOPO_TURNING_RIGHT = 1<<10,
        TOPO_GETTING_UP = 1<<11,
        TOPO_GETTING_DOWN = 1<<12,
        TOPO_BORDERFORCED = 1<<13
    };

    
    typedef struct trackTopology_t
    {
        u32 trackSliceProperties;
        shapePattern_t *pattern;
        vec_t colorLight, colorDark;
        float leftRight, upDown;
        float width;
        float borderForced;
        float wallColorRedAlpha;
    } trackTopology_t;

    std::vector<trackTopology_t> mCurrentToopology[2];
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TRACK

class Track : public ITrack
{
public:	
    Track() {}
    virtual ~Track() {}

    void GoWithTrack( track_t *pTrack, bool bForceComputation = false );
    mesh_t* GenerateTrackForMenus(track_t *pTrack, matrix_t &mt);
    
    virtual bool getClosestSampledPoint(const vec_t& mobilePoint,
                                matrix_t& trackMatrix,
                                vec_t& trackMiddle,
                                int &aaCurIdx,
                                float &rightLeftFactor,
                                const vec_t& lastKnownGravity,
                                float &upDownFactor,
                                float& distanceToBorder,
                                int &aaCurFourche,
								bool bHoleDetected);

    virtual const AIPoint_t& getAIPoint(int idx) const { return mAIPoints[(idx+mAIPoints.size())%mAIPoints.size()]; }
    virtual unsigned int getAIPointCount() const { return mAIPoints.size(); }
    const matrix_t & GetSpawnMatrix(unsigned int idx) const { return spawnPoints[idx&7]; }
    virtual track_t *GetCurrentTrack() const { return mCurrentTrack; }

	mesh_t* generatePortalMesh( int brickStart, int brickEnd, bool forward  );
protected:
    track_t *mCurrentTrack;
    std::vector<AIPoint_t> mAIPoints;
    matrix_t spawnPoints[8];
    

public:	


    void deformPieces( track_t *pTrack, float trackLength, float segDist, const TrackTopology &topo, matrix_t *deformMatrices[2], bool bMiniTrack = false );


    mesh_t* clipMesh(mesh_t *pm, const vec_t& pt1, const vec_t& pt2, int slice, const vec_t&branchPoint);
    void setDeformedMeshProperties( mesh_t *pm, float distance );

	virtual bool holeOnTheWay( float aPos, float aCheckLength) { return GetCurrentTrack()->holeOnTheWay( aPos, aCheckLength); }
#define MAX_TRACK_OCEAN_HOLES 32
    vec_t mTrackHoles[MAX_TRACK_OCEAN_HOLES];
    int mTrackHolesAv;

    // build me up struct and method
#if 0
    typedef struct buildProgress_t
    {
        buildProgress_t() {}
        buildProgress_t(float _distance, mesh_t *_mesh, const vec_t& _pivot)
        {
            distance = _distance;
            pMesh = _mesh;
            pivot = _pivot;
            bUseWorldMat = false;
        }
        buildProgress_t(float _distance, mesh_t *_mesh, const matrix_t& _worldMat)
        {
            distance = _distance;
            pMesh = _mesh;
            mWorldMat = _worldMat;
            bUseWorldMat = true;
        }
        float distance;
        mesh_t *pMesh;
        vec_t pivot;
        matrix_t mWorldMat;
        bool bUseWorldMat;

    } buildProgress_t;
    std::vector<buildProgress_t> mBuildProgress;
    void pushProgress(float _distance, mesh_t *_mesh, const vec_t& _pivot) { mBuildProgress.push_back( buildProgress_t( _distance, _mesh, _pivot ) ); }
    void pushProgress(float _distance, mesh_t *_mesh, const matrix_t &worldMat) { mBuildProgress.push_back( buildProgress_t( _distance, _mesh, worldMat ) ); }
    void progressMesh( float distance, float length);
#endif
    // minimal curve
    matrix_t curve[2][NBSEGS * 4];
    matrix_t minimalCurve[2][NBSEGS * 4];
    float curveWidth[2][NBSEGS * 4];

    vec_t boundMin, boundMax;

    // AI
    void ComputeMinimalCurve( track_t& tr );
};

extern Track track;


void initTracks();
void initTracksMiniMesh();

#endif
