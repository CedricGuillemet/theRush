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
#include "fx.h"

#include "game.h"
#include "world.h"
#include "render.h"

#include "include_GL.h"

//#ifndef MACOSX
//extern void ( APIENTRYP glGenerateMipmap)(GLenum target);
//#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

GLuint GShipTrailTexture = 0;
GLuint GLightningTexture = 0;
GLuint GTargetTexture = 0;

Sparkles *GSparkles = NULL;
Sparkles *GBulletSparkles = NULL;

static FastArray<omniLight_t*, 20> GSparklesGeneratedLights;

///////////////////////////////////////////////////////////////////////////////////////////////////

int *Ribbon::mIndicesForSparkles = NULL;
int *Ribbon::mIndicesForTrails = NULL;

FastArray<Ribbon*, Ribbon::Ribbon_Count_Max> Ribbon::mGRibbons;

void Ribbon::Init()
{
    const int MaxIndiceCount = 2048 * 6;
    COMPILE_TIME_ASSERT( Sparkle_Point_Count_Max <= MaxIndiceCount );
    COMPILE_TIME_ASSERT( Trail_Indice_Count_Max <= MaxIndiceCount );
    COMPILE_TIME_ASSERT( Lightning_Indice_Count_Max <= MaxIndiceCount );

    ASSERT_GAME( mIndicesForSparkles == NULL );

    mIndicesForSparkles = new int [MaxIndiceCount];
    int av = 0;
    for ( int i =0;i<MaxIndiceCount;i += 6)
    {
        mIndicesForSparkles[i + 0] = 0 + av;
        mIndicesForSparkles[i + 1] = 1 + av;
        mIndicesForSparkles[i + 2] = 3 + av;

        mIndicesForSparkles[i + 3] = 0 + av;
        mIndicesForSparkles[i + 4] = 3 + av;
        mIndicesForSparkles[i + 5] = 2 + av;
        av += 4;
    }

    ASSERT_GAME( mIndicesForTrails == NULL );

    mIndicesForTrails = new int [MaxIndiceCount];
    av = 0;

    for (int i = 0;i<MaxIndiceCount;i+=6)
    {
        mIndicesForTrails[i + 0] = 0 + av;
        mIndicesForTrails[i + 1] = 1 + av;
        mIndicesForTrails[i + 2] = 3 + av;

        mIndicesForTrails[i + 3] = 0 + av;
        mIndicesForTrails[i + 4] = 3 + av;
        mIndicesForTrails[i + 5] = 2 + av;
        av += 2;
    }

}

#if 0
void Ribbon::ClearAll()
{
    Ribbon*const* iter = mGRibbons.GetFirst();
    Ribbon*const* iterEnd = mGRibbons.GetEndUsed();
    for (; iter != iterEnd ; ++iter )
    {
        ASSERT_GAME( iterEnd == mGRibbons.GetEndUsed() );
        delete (*iter);
    }

    mGRibbons.Clear();
}

void Ribbon::ResetAll()
{
    Ribbon*const* iter = mGRibbons.GetFirst();
    Ribbon*const* iterEnd = mGRibbons.GetEndUsed();
    for (; iter != iterEnd ; )
    {
        ASSERT_GAME( iterEnd == mGRibbons.GetEndUsed() );

        if ( (*iter)->Reset())
        {
            delete (*iter);
        }
        //FIXME: does it really make sense to increment the iterator for only a few and not all?
        else
            ++iter;
    }
}
#endif  //  0

void Ribbon::TickAll( float aTimeEllapsed )
{
    if ( aTimeEllapsed < FLOAT_EPSILON )
        return;

    PROFILER_START(FXTickAll);

    Ribbon*const* iter = mGRibbons.GetFirst();
    Ribbon*const* iterEnd = mGRibbons.GetEndUsed();
    for (; iter != iterEnd ; ++iter )
    {
        (*iter)->Tick( aTimeEllapsed );
    }

    // generated sparkle lights
    omniLight_t* *plc = GSparklesGeneratedLights.GetFirstToModify();
    for (; plc != GSparklesGeneratedLights.GetEndUsed(); )
    {
        (*plc)->mColor.lerp( vec(0.f), aTimeEllapsed * 10.f );
        if ( (*plc)->mColor.w <= 0.1f )
        {
            Renderer::destroyOmni( (*plc) );
            GSparklesGeneratedLights.Erase( plc );
        }
        else
            plc ++;
    }

    PROFILER_END(); // FXTickAll
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Ribbon::Ribbon( unsigned int verticeCountMax )
{
    mVShift = mVShitAVPerSecond = 0.f;
    mColor = vec(1.f);

    mbUseTrailIndices = true;
    mbVisible = true;

    mVertices = new ribbonVertex_t[verticeCountMax];
    mVerticeCountMax = verticeCountMax;
    mNbIndicesToDraw = 0;

    mWidth = 1.f;
    mWorldMatrix.identity();

    *mGRibbons.Push() = this;
}

Ribbon::~Ribbon()
{
//    debug_printf("destroying ribbons at address: %p\n", ((void*)this) );

    ASSERT_GAME( mGRibbons.Contains(this) == true );
    mGRibbons.Erase(this);
    ASSERT_GAME( mGRibbons.Contains(this) == false );
    //mGRibbons.remove(this);

    delete [] mVertices;
    mVertices = NULL;
}

void Ribbon::BindTexture()
{
    glBindTexture(GL_TEXTURE_2D, mGLTex);
}

void Ribbon::Draw()
{

	ASSERT_GAME( mVertices != NULL );

    if ( !mNbIndicesToDraw || !mbVisible )
        return;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(ribbonVertex_t),mVertices);
    glTexCoordPointer(2, GL_FLOAT, sizeof(ribbonVertex_t),&mVertices->u);
    glNormalPointer(GL_FLOAT, sizeof(ribbonVertex_t),&mVertices->nx);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ribbonVertex_t), &mVertices->color);

    glDrawElements(GL_TRIANGLES, mNbIndicesToDraw, GL_UNSIGNED_INT, mbUseTrailIndices?mIndicesForTrails:mIndicesForSparkles);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Sparkles::Spawn(const vec_t& sparklesPoint, const vec_t& generationNormal, float aSpread, float aStrength, int aCount)
{
    matrix_t plan;
    plan.LookAt(sparklesPoint, sparklesPoint + generationNormal, vec(0.f, 1.f, 0.f));

    for (int i=0;i<aCount;i++)
    {
        vec_t vel;
        vel = generationNormal + plan.right * ((r01()-0.5f) * aSpread) + plan.up * ((r01()-0.5f) * aSpread);
        vel.normalize();
        vel *= aStrength * 0.85f;
        GSparkles->Spawn( sparklesPoint, vel, 0xFFFFFFFF, r01()*0.10f + 0.08f);
    }

    PushFlashLight( sparklesPoint + generationNormal );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Sparkles::Sparkles() : Ribbon(Ribbon::Sparkle_Vertice_Count_Max)
{
    mbUseTrailIndices = false;
    mGLTex = GSparkleTexture;

    ASSERT_GAME( mVertices != NULL );
    ASSERT_GAME( mVerticeCountMax == Ribbon::Sparkle_Vertice_Count_Max );

    COMPILE_TIME_ASSERT( sizeof(sparks) == sizeof(sparklePoint_t) * Ribbon::Sparkle_Point_Count_Max );
    memset (&sparks[0], 0, sizeof(sparks) );

    mNbSparkles = 0;

    mGravity = vec(0.f, -100.f, 0.f);
    mbTickDeported = false;
    mLength = 1.f;
}

Sparkles::~Sparkles()
{
//    debug_printf("destroying sparkles at address: %p\n", ((void*)this) );
}

void Sparkles::Spawn(const vec_t& pos, const vec_t& velocity, uint32 color, float TTL)
{
    ASSERT_GAME( 0 <= mNbSparkles && mNbSparkles <= Ribbon::Sparkle_Point_Count_Max );

    if ( mNbSparkles < Ribbon::Sparkle_Point_Count_Max )
    {
        sparklePoint_t& sp = sparks[mNbSparkles];
        sp.position = pos;
        sp.positionOld = pos - velocity;
        sp.mTTL = TTL;
        sp.color = color;

        mNbSparkles ++;
    }
}

bool Sparkles::Reset()
{
    return true;
}

void Sparkles::Tick(float aTimeEllapsed)
{
    ASSERT_GAME( 0 <= mNbSparkles && mNbSparkles <= Ribbon::Sparkle_Point_Count_Max );

    if (mbTickDeported)
        return;

    sparklePoint_t* sp = &sparks[0];
    for (int i=0;i<mNbSparkles;i++)
    {

        sp->mTTL -= aTimeEllapsed;
        sp->SolveVerlet(mGravity, aTimeEllapsed);
        sp++;
    }
    sp = &sparks[0];
    for (int i=0;i<mNbSparkles;)
    {
        if (sp->mTTL <= 0.f)
        {
            (*sp) = sparks[mNbSparkles-1];
            mNbSparkles--;
        }
        else
        {
            ++i;
            ++sp;
        }
    }
    // build VS
    for (int i=0;i<mNbSparkles;i++)
    {
        ASSERT_GAME( (i*4 + 3) < mVerticeCountMax );
        pushRibbonSparkle(mVertices+i*4, sparks[i].position, normalized(sparks[i].velocity), mLength, sparks[i].color, sparks[i].mTTL);
    }

    mNbIndicesToDraw = mNbSparkles*6;
    ASSERT_GAME( mNbIndicesToDraw <= Ribbon::Sparkle_Indice_Count_Max );
}

void Sparkles::BuildFromBulletList( const std::vector<bullet_t>& mBullets )
{
    std::vector<bullet_t>::const_iterator iter = mBullets.begin();
    const std::vector<bullet_t>::const_iterator iterEnd = mBullets.end();
    unsigned int i = 0;
    for ( ; iter != iterEnd ; i++, ++iter )
    {
        if ( i >= Ribbon::Sparkle_Point_Count_Max )
            break;

        const bullet_t & b = (*iter);
        const vec_t& pos = b.mPos;
        const vec_t& dir = b.mDir;
        pushRibbonSparkle( mVertices + ( i << 2 ), pos, normalized(dir), 2.5f, 0xFFFFFFFF, 1.f );
    }
    ASSERT_GAME( i == mBullets.size() || i == Ribbon::Sparkle_Point_Count_Max );

    mNbIndicesToDraw = i * 6 ;
    ASSERT_GAME( mNbIndicesToDraw <= Ribbon::Sparkle_Indice_Count_Max );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Trail::Trail() : Ribbon(Ribbon::Trail_Vertice_Count_Max)
{
	mLastPoint = vec(-99999.f, -99999.f, -99999.f);
	mIntensityLossPerSecond = 1.f;
	mGLTex = GShipTrailTexture;

    ASSERT_GAME( mVertices != NULL );
    ASSERT_GAME( mVerticeCountMax == Ribbon::Trail_Vertice_Count_Max );

	mNbIndicesToDraw = Ribbon::Trail_Indice_Count_Max;

	Reset();
    ASSERT_GAME( mIdx == 0 );

	mMinimalDistance = 2.f;
}

Trail::~Trail()
{
//    debug_printf("destroying trail at address: %p\n", ((void*)this) );
}

bool Trail::Reset()
{
    COMPILE_TIME_ASSERT( sizeof(mPts) == sizeof(trailPt_t) * Ribbon::Trail_Point_Count_Max );
    memset (&mPts[0], 0, sizeof(mPts) );

    //FIXME: even though it's not necessary, I thought 'mIdx' could be set to 0 to be also called in Trail() constructor
    mIdx = 0;

    return false;
}

void Trail::Tick(float aTimeEllapsed)
{
    const int TrailMaxPts = Ribbon::Trail_Point_Count_Max;

    trailPt_t *p = &mPts[0];
    for (int i = 0 ; i < TrailMaxPts ; i++)
    {
        p->intensity -= mIntensityLossPerSecond * aTimeEllapsed;
        ++p;
    }

    // build VB
    vec_t vdir, oldvdir;

    for (int i=0;i<=TrailMaxPts-1;i++)
    {
        int idx1 = (mIdx-i+TrailMaxPts)&(TrailMaxPts-1);
        ASSERT_GAME( idx1 < TrailMaxPts );
        int idx2 = (mIdx-i-1+TrailMaxPts)&(TrailMaxPts-1);
        ASSERT_GAME( idx2 < TrailMaxPts );
        vdir = mPts[idx2].position - mPts[idx1].position;
        if ( !i )
        {
            oldvdir = vdir;
        }

        ASSERT_GAME( (i*2 + 1) < mVerticeCountMax);
        pushRibbonPoints(mVertices+(i*2), mPts[idx1].position, ( oldvdir + vdir ) * 0.5f, 0.f, mPts[idx1].intensity);
        oldvdir = vdir;
    }
	/*
    int i = TrailMaxPts-1;
    int idx1 = (i+mIdx)&(TrailMaxPts-1);
    ASSERT_GAME( idx1 < TrailMaxPts );
    int idx3 = (i+mIdx-1+TrailMaxPts)&(TrailMaxPts-1);
    ASSERT_GAME( idx3 < TrailMaxPts );
    vdir = mPts[idx1].position - mPts[idx3].position;

    ASSERT_GAME( (i*2 + 1) < mVerticeCountMax);
    pushRibbonPoints(mVertices+(i*2), mPts[idx1].position, vdir, 0.f, mPts[idx1].intensity);
	*/
}

void Trail::SetCurrentPoint(const vec_t & pos, float intensity /*= 1.f*/, bool forced )
{
    const int TrailMaxPts = Ribbon::Trail_Point_Count_Max;
    ASSERT_GAME( mIdx < TrailMaxPts );

    if ( ( (pos - mLastPoint).lengthSq() > mMinimalDistance * mMinimalDistance) || forced )
    {
		mLastPoint = pos;
		++mIdx &= (TrailMaxPts-1);
        mPts[mIdx] = trailPt_t(pos, intensity);
        
        ASSERT_GAME( mIdx < TrailMaxPts );
    }
    else
        mPts[mIdx] = trailPt_t(pos, intensity);
}
void Trail::TeleportTo(const vec_t & pos)
{
    const int TrailMaxPts = Ribbon::Trail_Point_Count_Max;
    ASSERT_GAME( mIdx < TrailMaxPts );

    mPts[mIdx] = trailPt_t(mPts[mIdx].position, 0.f);
    ++mIdx &= (TrailMaxPts-1);
    ASSERT_GAME( mIdx < TrailMaxPts );

    mPts[mIdx] = trailPt_t(pos, 0.f);
    ++mIdx &= (TrailMaxPts-1);
    ASSERT_GAME( mIdx < TrailMaxPts );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Lightning::Lightning(int nbSteps) : Ribbon( (nbSteps-1) * 4 )
{
    ASSERT_GAME( 3 <= nbSteps && nbSteps <= Ribbon::Lightning_Point_Count_Max );

    mGLTex = GLightningTexture;
    mbUseTrailIndices = false;

    const int nbStepCountMax = Ribbon::Lightning_Point_Count_Max;
    mNbSteps = (nbSteps < nbStepCountMax)? nbSteps : nbStepCountMax;

    ASSERT_GAME( mVertices != NULL );
    ASSERT_GAME( mVerticeCountMax == (mNbSteps-1) * 4 );

    mLocalCounter = 0.f;
}

Lightning::~Lightning()
{
//    debug_printf("destroying lightning at address: %p\n", ((void*)this) );
}

bool Lightning::Reset()
{
    return true;
}

void Lightning::Tick(float aTimeEllapsed)
{
    int idx = 0;

    mLocalCounter += aTimeEllapsed;
    while (mLocalCounter >= 0.f)
    {
        idx = 0;

        vec_t right, front;
        vec_t nextPos = mPoint1;

        float len;
        float aStepLength = (mPoint1-mPoint2).length() * (1.f/(float)mNbSteps);

        ASSERT_GAME( idx < mNbSteps-2 );
        mPts[idx++] = mPoint1;
        nextPos += mStartupDirection;
        //mPts[idx++] = nextPos;

        do
        {
            vec_t dir = mPoint2 - nextPos;
            dir.normalize();

            if (dir.dot(vec(0.f, 1.f, 0.f)) > 0.9f)
            {
                right.cross(dir, vec(1.f, 0.f, 0.f));
                right.normalize();
                front.cross(dir, right);
                front.normalize();
            }
            else
            {
                right.cross(dir, vec(0.f, 1.f, 0.f));
                right.normalize();
                front.cross(dir, right);
                front.normalize();
            }

            vec_t newDir = dir + (right*(r01()-0.5f))*1.3f + (front*(r01()-0.5f))*1.3f;
            nextPos += newDir*aStepLength;
            //glVertex3fv(&nextPos.x);

            ASSERT_GAME( idx < mNbSteps-1 );
            mPts[idx++] = nextPos;

            if (idx == (mNbSteps-1))
                break;
            len = (nextPos- mPoint2).length();
        }
        while (len>aStepLength);

        ASSERT_GAME( idx < mNbSteps );
        mPts[idx++] = mPoint2;

        mLocalCounter -= (1.f/60.f);
    }

    // build vertices
    if (idx)
    {
        ASSERT_GAME( idx <= mNbSteps );

        for (int i = 0; i<(idx-1); ++i)
        {
            float v = (float)(rand()&0xE0) * (1.f/512.f);
            float v2 = v + (31.f* (1.f/512.f));

            vec_t nnext = (i == 0)? normalized(mPts[1]-mPts[0]) : normalized(mPts[i+1]-mPts[i-1]);
            vec_t np1next = ( (i+1) == (idx-1))? normalized(mPts[idx-1]-mPts[idx-2]) : normalized(mPts[i+2]-mPts[i]);
            ASSERT_GAME( (i*4 + 3) < mVerticeCountMax );
            pushRibbonPoints(mVertices+(i*4), mPts[i], nnext, v);
            pushRibbonPoints(mVertices+(i*4)+2, mPts[i+1], np1next, v2);
        }
        mNbIndicesToDraw = (idx-1)*6;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void InitFX()
{
    PROFILER_START(FX);
	LOG("Init FX ... ");
    
	{
		u32 *v;
		// Trail texture
	
		v = new u32 [64];
	
		for (int i=0;i<64;i++)
		{
			v[i] = 0xFFFFFF + (((u8)(sinf( ((float(i) / 63.f) * PI)) * 255.f))<<24);
		}
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &GShipTrailTexture);	
    
		glBindTexture(GL_TEXTURE_2D, GShipTrailTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, v);
		glGenerateMipmap(GL_TEXTURE_2D);

		delete [] v;
        v = NULL;

		// sparkles
		v = new u32 [64*64];
	
		for (int i=0;i<64;i++)
		{
			for (int j=0;j<64;j++)
			{
				float xv = sinf( ((float(j) / 63.f) * PI));
				float yv = sinf( ((float(i) / 63.f) * PI));
				v[i*64+j] = 0xFFFFFF + ((u8)(xv * yv * xv * yv * 200.f)<<24);
			}
		}

        delete [] v;
        v = NULL;

        //FIXME: should it generate some textures?
	}
    
	// lightning
	glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &GLightningTexture);	
    
    glBindTexture(GL_TEXTURE_2D, GLightningTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
    u32 * plraw32 = new u32 [ 32 * 512 ];
    const char *lraw = getFileFromMemory("Datas/Textures/lightning.raw").c_str();

    for (int i = 0;i<32*512;i++)
    {
        u8 v = *lraw++;
        plraw32[i] = (v<<24)+(v<<16)+(v<<8)+(v);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, plraw32 );
    glGenerateMipmap(GL_TEXTURE_2D);
    
    delete [] plraw32;
    plraw32 = NULL;

    // sparkle
    glEnable(GL_TEXTURE_2D);
    glGenTextures( 1, &GSparkleTexture );
    glBindTexture( GL_TEXTURE_2D, GSparkleTexture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, getFileFromMemory("Datas/Textures/sparkle.raw").c_str() );

	// ribbon/sparkles
	Ribbon::Init();

	GSparkles = new Sparkles;
	GSparkles->SetWidth( 0.125f * 0.5f );
    GSparkles->SetLength( 1.0f * 0.5f );
    GSparkles->SetColor( vec(1.f, 0.5f, 0.5f, 10.f) );//vec(1.f, 0.9f, 0.3f, 1.f) );
    

	GBulletSparkles = new Sparkles;
	GBulletSparkles->SetWidth( 2.0f );
	GBulletSparkles->SetTickDeported(true);
    GBulletSparkles->SetLength( 16.f );

	// target textures
	
    {
        u8 *v;
        // Trail texture

        int texSize = 512;
        v = new u8 [texSize * texSize];
        memset(v, 0, texSize * texSize);
        vec_t repere = vec(1.f/(float)texSize, 1.f/(float)texSize, 0.f, 0.f);

        bool hasmat[32];
        for (int i=0;i<32;)
        {
            bool crant = (rand()&1)!=0;
            int crantcount = (rand()&1)+1;
            int j=0;
            for (j=0;j<crantcount;j++)
            {
                if (i+j<32)
                    hasmat[i+j] = crant;
            }
            i+=j;
        }

        for (int i=0;i<texSize;i++)
        {
            for (int j=0;j<texSize;j++)
            {
                vec_t texpos = vec( (float)i, (float)j, 0.f, 0.f);
                texpos *= repere;

                vec_t topt = texpos - vec(0.5f, 0.5f, 0.f, 0.f);
                float toptL = topt.length();


                float distToCenter = toptL*2.f;
                if ( (distToCenter >0.6f) && (distToCenter<0.8f))
                {
                    float dt = topt.x/toptL;
                    ASSERT_GAME( (-1.f < dt && dt < 1.f) || IsNearlyEqual(dt, -1.f) || IsNearlyEqual(dt, 1.f) ) ;
                    dt = Clamp(dt, -1.f, 1.f);

                    float ng = acosf(dt);
                    if (topt.y<0.f)
                        ng = -ng + 2.f*PI;

                    if ( (ng> (45.f /360.f)*2*PI) && (ng< (75.f /360.f)*2*PI))
                    {}
                    else if ( (ng> (165.f /360.f)*2*PI) && (ng< (195.f /360.f)*2*PI))
                    {}
                    else if ( (ng> (285.f /360.f)*2*PI) && (ng< (315.f /360.f)*2*PI))
                    {}
                    else
                    {
                        v[i*texSize+j] = 0xFF;
                        if (distToCenter >0.65f) 
                        {
                            dt = topt.x/toptL;
                            ASSERT_GAME( (-1.f < dt && dt < 1.f) || IsNearlyEqual(dt, -1.f) || IsNearlyEqual(dt, 1.f) ) ;
                            dt = Clamp(dt, -1.f, 1.f);

                            ng = acosf(dt);
                            if (topt.y<0.f)
                                ng = -ng + 2.f*PI;
                            ng *= 36.f;
                            ng /= 2.f*PI;

                            v[i*texSize+j] = hasmat[((int)ng)&0x1F]?0xFF : 0;
                        }
                    }
                }

            }
        }




        int texSizeHalf = texSize>>1;
        u32 *v2 = new u32 [texSizeHalf * texSizeHalf];

        for (int i=0;i<texSizeHalf;i++)
        {
            for (int j=0;j<texSizeHalf;j++)
            {
                int baseidx = (i*texSize+j)*2;

                int val = v[baseidx];
                val += v[baseidx+1];
                val += v[baseidx+texSize];
                val += v[baseidx+texSize+1];
                int rval = (val>>2);
                v2[i*texSizeHalf+j] = (rval<<24) + (rval<<16) + (rval<<8) + rval;
            }
        }
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &GTargetTexture);	

        glBindTexture(GL_TEXTURE_2D, GTargetTexture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texSizeHalf, texSizeHalf, 0, GL_RGBA, GL_UNSIGNED_BYTE, v2);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete [] v;
        v = NULL;
        delete [] v2;
        v2 = NULL;
    }


	LOG("Done\n");
    PROFILER_END(); // FX
            
}

void UninitFX()
{
    //FIXME: should it free memory allocated in Ribbon::Init() ?
	LOG("UninitFX\n");
    delete GSparkles;
    GSparkles = NULL;
	delete GBulletSparkles;
    GBulletSparkles = NULL;
	LOG("Done\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

void PushFlashLight( const vec_t& position )
{
    omniLight_t **pnl= GSparklesGeneratedLights.PushCapped();
    if ( pnl )
    {
        omniLight_t *pLight = Renderer::newOmni();
        pLight->mPosition = position;
        pLight->mPosition.w = 10.f;
        pLight->mColor = vec(1.f, 0.6f, 0.2f, 0.3f);
        *pnl = pLight;
    }
}

void ClearFlashLights()
{
    GSparklesGeneratedLights.Clear();
}