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

#ifndef FX_H__
#define FX_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define Damping     0.01f

//HACK: to avoid including gl.h
#if !defined(GLuint)
typedef unsigned int GLuint;
#endif

extern GLuint GTargetTexture;
extern GLuint GShipTrailTexture;
//extern GLuint GSparklesTexture;
extern GLuint GSparkleTexture;

struct bullet_t;

typedef struct ribbonVertex_t
{
    float x,y,z;
    float nx, ny, nz;
    float u, v;
    uint32 color;


} ribbonVertex_t;

inline void pushRibbonPoints(ribbonVertex_t *p, const vec_t& pos, const vec_t& dir, float v, float alpha = 1.f)
{
    alpha = (alpha<0.f)?0.f:alpha;
    u32 locCol = 0xFFFFFF + ((u8)(alpha*255.f)<<24);    
    p->x = pos.x; p->y = pos.y; p->z = pos.z;
    p->nx = dir.x; p->ny = dir.y; p->nz = dir.z;
    p->color = locCol;
    p->u = -0.5f;
    p->v = v;
    p++;
    p->x = pos.x; p->y = pos.y; p->z = pos.z;
    p->nx = dir.x; p->ny = dir.y; p->nz = dir.z;
    p->color = locCol;
    p->u = 0.5f;
    p->v = v;

}

inline void pushRibbonSparkle(ribbonVertex_t *p, const vec_t& pos, const vec_t& dir, float aSize, u32 col, float alpha = 1.f)
{
    alpha = (alpha<0.f)?0.f:alpha;
    u32 acol =  (col&0xFFFFFF) + ((u8)(alpha*255.f)<<24);
    vec_t halfdir = dir*aSize*0.5f;

    p->x = pos.x-halfdir.x; p->y = pos.y-halfdir.y; p->z = pos.z-halfdir.z;
    p->nx = dir.x; p->ny = dir.y; p->nz = dir.z;
    p->color = acol;
    p->u = -0.5f; p->v = 0.f;
    p++;
    p->x = pos.x-halfdir.x; p->y = pos.y-halfdir.y; p->z = pos.z-halfdir.z;
    p->nx = dir.x; p->ny = dir.y; p->nz = dir.z;
    p->color = 0xFFFFFF + ((u8)(alpha*255.f)<<24);
    p->u = 0.5f; p->v = 0.f;
    p++;
    p->x = pos.x+halfdir.x; p->y = pos.y+halfdir.y; p->z = pos.z+halfdir.z;
    p->nx = dir.x; p->ny = dir.y; p->nz = dir.z;
    p->color = acol;
    p->u = -0.5f; p->v = 1.f;
    p++;
    p->x = pos.x+halfdir.x; p->y = pos.y+halfdir.y; p->z = pos.z+halfdir.z;
    p->nx = dir.x; p->ny = dir.y; p->nz = dir.z;
    p->color = 0xFFFFFF + ((u8)(alpha*255.f)<<24);
    p->u = 0.5f; p->v = 1.f;

}

typedef struct sparklePoint_t
{
    vec_t position;
    vec_t positionOld;
    vec_t velocity;

    inline void SolveVerlet(vec_t acceleration, float timeStep)
    {
        vec_t t;
        vec_t oldPos = position;
        //tvector2.Mult(ref acceleration, timeStep * timeStep, out acceleration);
        acceleration *= timeStep*timeStep;
        //tvector2.Sub(ref position, ref positionOld, out t);
        t = position - positionOld;
        //tvector2.Mult(ref t, 1.0f - Damping, out t);
        t *= 1.f-Damping;
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
        velocity = t* (1.f/timeStep);
    }
    float mTTL;
    uint32 color;
}sparklePoint_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Ribbon
{
protected:
    enum{ Sparkle_Point_Count_Max = 2048 };
    enum{ Sparkle_Vertice_Count_Max = Sparkle_Point_Count_Max * 4 };
    enum{ Sparkle_Indice_Count_Max = Sparkle_Point_Count_Max * 6 };

    enum{ Trail_Point_Count_Max = 128 };
    enum{ Trail_Vertice_Count_Max = Trail_Point_Count_Max * 2 };
    enum{ Trail_Indice_Count_Max = (Trail_Point_Count_Max - 1) * 6 };

    enum{ Lightning_Point_Count_Max = 64 };
    enum{ Lightning_Vertice_Count_Max = Lightning_Point_Count_Max * 2 };
    enum{ Lightning_Indice_Count_Max = (Lightning_Point_Count_Max - 1) * 6 };

private:
    enum{ Ribbon_Count_Max = 1024 };

    static FastArray<Ribbon*, Ribbon_Count_Max> mGRibbons;

    static int *mIndicesForSparkles;
    static int *mIndicesForTrails;

    Ribbon();

public:
    Ribbon( unsigned int verticeCountMax );
    virtual ~Ribbon();

    // reset : return true if must be deleted. make reset otherwise
    virtual bool Reset() { return true; }
    virtual void Tick(float aTimeEllapsed)
    {
        mVShift += mVShitAVPerSecond*aTimeEllapsed;
    }

    void BindTexture();

    void Draw();

    // props
    void SetColor(const vec_t& color) { mColor = color; }
    const vec_t& GetColor() const { return mColor; }
    void SetWidth(float width) { mWidth = width; }
    float GetWidth() const { return mWidth; }

    void SetVShift(float aShiftPerSecond) { mVShitAVPerSecond = aShiftPerSecond; }
    float GetVShift() const { return mVShift; }

    const matrix_t& GetWorldMatrix() const { return mWorldMatrix; }
    void SetWorldMatrix(const matrix_t &mat) { mWorldMatrix = mat; }

    void SetVisible( bool bVis) { mbVisible = bVis; }
    bool IsVisible() const { return mbVisible; }

    // init
    static void Init();
#if 0
    static void ClearAll();
    static void ResetAll();
#endif
    static void TickAll( float aTimeEllapsed );

    static bool HasRibbonsToDraw() { return (!mGRibbons.Empty()); }

    static const FastArray<Ribbon*, Ribbon_Count_Max>& GetRibbons() { return mGRibbons; }

    static const int *GetIndicesForTrails() { return mIndicesForTrails; }

protected:

    float mVShift, mVShitAVPerSecond;
    vec_t mColor;

    bool mbUseTrailIndices;
    bool mbVisible;

    ribbonVertex_t *mVertices;
//FIXME: mNbVertices wasn't used, should it be removed?
//    int mNbVertices;
    int mVerticeCountMax;
    int mNbIndicesToDraw;
    float mWidth;
    unsigned int mGLTex;

    matrix_t mWorldMatrix;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sparkles : public Ribbon
{
public:
    Sparkles();
    virtual ~Sparkles();

    // reset : return true if must be deleted. make reset otherwise
    virtual bool Reset();
    virtual void Tick(float aTimeEllapsed);

    void Spawn(const vec_t& pos, const vec_t& velocity, uint32 color, float TTL);

    void BuildFromBulletList(const std::vector<bullet_t>& mBullets);

    void SetTickDeported(bool bDeported) { mbTickDeported = bDeported; }

    void SetLength( float aLength ) { mLength = aLength; }
    float GetLength( ) const { return mLength; }

    void ClearParticles() { mNbSparkles = 0; }

    static void Spawn(const vec_t& sparklesPoint, const vec_t& generationNormal, float aSpread = 0.5f, float aStrength = 1.f, int aCount = 5);

protected:
    sparklePoint_t sparks[Ribbon::Sparkle_Point_Count_Max];
    int mNbSparkles;
    vec_t mGravity;
    bool mbTickDeported;
    float mLength;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Trail : public Ribbon
{
public:
    Trail();
    virtual ~Trail();

    // reset : return true if must be deleted. make reset otherwise
    virtual bool Reset();
    virtual void Tick(float aTimeEllapsed);

    void SetCurrentPoint(const vec_t & pos, float intensity = 1.f, bool forced = false );
    void TeleportTo(const vec_t & pos);

    void SetLossPerSecond(float aVal) { mIntensityLossPerSecond = aVal; }
    void SetMinimalDistance(float aDist) { mMinimalDistance = aDist; }

protected:
    vec_t mLastPoint;
    int mIdx;
    float mIntensityLossPerSecond;
    float mMinimalDistance;

    typedef struct trailPt_t
    {
        trailPt_t() {}
        trailPt_t(const vec_t &p, float it)
        {
            position = p;
            intensity = it;
        }
        vec_t position;
        float intensity;
    } trailPt_t;

    trailPt_t mPts[Ribbon::Trail_Point_Count_Max];

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Lightning : public Ribbon
{
public:
    Lightning(int nbSteps);
    virtual ~Lightning();

    // reset : return true if must be deleted. make reset otherwise
    virtual bool Reset();
    virtual void Tick(float aTimeEllapsed);

    void SetPoints(const vec_t& p1, const vec_t& p2 )
    {
        mPoint1 = p1;
        mPoint2 = p2;
        mStartupDirection = vec( 0.f );
    }
    void SetPoints(const vec_t& p1, const vec_t& p2, const vec_t& startupDirection )
    {
        mPoint1 = p1;
        mPoint2 = p2;
        mStartupDirection = startupDirection;
    }

protected:
    vec_t mPoint1, mPoint2;
    vec_t mPts[Ribbon::Lightning_Point_Count_Max];
    int mNbSteps;
    float mLocalCounter;
    vec_t mStartupDirection;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InitFX();
void UninitFX();

extern Sparkles *GSparkles;
extern Sparkles *GBulletSparkles;
void PushFlashLight( const vec_t& position );
void ClearFlashLights();

#endif



