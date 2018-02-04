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

#ifndef RENDER_H__
#define RENDER_H__

class Camera;
#ifndef APIENTRY
	#define APIENTRY
#endif

#ifndef APIENTRYP
	#define APIENTRYP APIENTRY *
#endif
#if IS_OS_WINDOWS
typedef ptrdiff_t GLsizeiptr;
#endif
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

#define MAX_NB_FLARES 32

//HACK: to avoid including include_GL.h
#if !defined(GLuint)
typedef unsigned int GLuint;
#endif


enum ExplosionType
{
	FIRE_BALL,
	FIRE_WALL,
	PLASMA_WALL,
	ELECTRIC_WALL
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct shader_t : public serialisableObject_t
{
    SERIALIZABLE(shader_t,"Shader")

    shader_t()
    {
        mFragmentShader="void main() { gl_FragColor = vec4(1,0,1,1);}";
        mVertexShader = "uniform mat4 modelViewProjectionMatrix;void main(){gl_Position = modelViewProjectionMatrix * gl_Vertex;}";
        mErrorLog = "";
        mShaderProgram = 0;
        mbIsDirty = false;
    }
    virtual ~shader_t() {}

    virtual void FieldsHasBeenModified();
    GLuint GetUniform( const char *szName );

	void LoadShader( const char *sourceVertex, const char*sourceFragment )
	{
		mFragmentShader = sourceFragment;
		mVertexShader = sourceVertex;
		LoadShader();
	}
    void LoadShader();
    static void LoadDirtyShaders();
    virtual bool ObjectIsDirty() const { return mbIsDirty; }
	bool IsLoaded() const { return ( mShaderProgram != 0 ); }
	void Bind();
	void SetVec4( GLuint uniformIndex, const vec_t& value );
	void SetVec2( GLuint uniformIndex, const vec_t& value );
private:
    std::string mFragmentShader;
    std::string mVertexShader;
    std::string mErrorLog;
    GLuint mShaderProgram;

    bool mbIsDirty;

    GLuint LoadShaderProgram( const char *sourceVertex, const char*sourceFragment );
    void PrintInfoLog( GLuint obj, const char *programType, bool bShader );
    static FastArray<shader_t*, 100> DirtyShaders;
} shader_t;


typedef struct renderSettings_t : public serialisableObject_t
{
    SERIALIZABLE(renderSettings_t,"Rendering settings and shaders.")

    renderSettings_t() : PSSMDistance1(50), PSSMDistance2(100), PSSMDistance3(200), PSSMDistance4(350), mbOcclusionQueries(true)
    { PSSMDistance0 = 0.05f; }

    virtual ~renderSettings_t() {}

    float PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4;
    bool mbOcclusionQueries;

    // shaders
    shader_t mShaderTest;

} renderSettings_t;

bool ShouldRenderShadows();
bool ShouldRenderOcean();
bool ShouldRenderReflection();

extern renderSettings_t RenderSettings;
struct mesh_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////

enum IMESH_VAF
{
    VAF_XYZ = 1,
    VAF_XYZRHW = 1<<1,
    VAF_NORMAL = 1<<2,
    VAF_COLOR = 1<<3,
    VAF_BINORMAL = 1<<4,
    VAF_BITANGENT = 1<<5,
    VAF_TEX0 = 1<<6,
    VAF_TEX1 = 1<<7,
    VAF_TEX2 = 1<<8,
    VAF_TEX3 = 1<<9,
	VAF_HWI_MATRIX = 1<<10
};

//VAU for flag in vertexarray::init
enum IMESH_VAU
{
	VAU_STATIC,
	VAU_STREAM,
	VAU_DYNAMIC
};

enum IMESH_CREATION_FLAG
{
    VACF_NORMAL,
    VACF_DYNAMIC
};

enum IMESH_LOCK
{
    VAL_READONLY = 1,
    VAL_WRITE = 2
};

inline unsigned int GetVertexSizeFromFormat(uint32 aFormat)
{
	uint mVertexSize = 0;
	if (aFormat&VAF_XYZ) mVertexSize+=12;
	if (aFormat&VAF_XYZRHW) mVertexSize+=16;
	if (aFormat&VAF_NORMAL) mVertexSize+=12;
	if (aFormat&VAF_COLOR) mVertexSize+=4;
	if (aFormat&VAF_BINORMAL) mVertexSize+=12;
	if (aFormat&VAF_BITANGENT) mVertexSize+=12;
	if (aFormat&VAF_TEX0) mVertexSize+=8;
	if (aFormat&VAF_TEX1) mVertexSize+=8;
	if (aFormat&VAF_TEX2) mVertexSize+=8;
	if (aFormat&VAF_TEX3) mVertexSize+=8;
	if (aFormat&VAF_HWI_MATRIX) mVertexSize+=48;
	return mVertexSize;
}

class RefCount_t
{
public:
    RefCount_t() { mRefCount = 1; }
    virtual ~RefCount_t() {}

    void IncRef()
    {
        ASSERT_GAME( mRefCount > 0 );
        ++mRefCount;
    }
    void DecRef()
    {
        ASSERT_GAME( mRefCount > 0 );
        --mRefCount;

        if ( mRefCount <= 0)
        {
            delete this;
        }
    }
private:
    int mRefCount;
};

class ZIndexArrayOGL : public RefCount_t
{
public:
    ZIndexArrayOGL();
    virtual ~ZIndexArrayOGL();

    void Init(int aQty, uint32 aFlag, bool bUseShort = true);

    uint32 GetIndexCount() const { return mIndexCount; }

    void * Lock(IMESH_LOCK aFlag);
    void Unlock();

    void Bind();
	void Unbind();

	size_t GetMemoryUsed() const { return (sizeof(ZIndexArrayOGL) + mIndexBufferSize); }
	int GetElementSize() const { return mIndexSize; }

protected:
	int mIndexSize;
    uint32 mIndexCount;
	uint32 mFlag;
	uint32 mIndexBufferSize;
	GLuint	mIndexBufferName;
	void *mIndexBuffer;
	IMESH_LOCK mLockFlag;
	bool mLocked;

    static unsigned int IndexArrayInstancesCount;
    //IDirect3DIndexBuffer9 * mD3DBuffer;
    friend class Render;
};


class ZVertexArrayOGL : public RefCount_t
{
public:
    ZVertexArrayOGL();
    virtual ~ZVertexArrayOGL();

    void Init(uint32 aFormat, int aQty, bool aKeepVBORam, uint32 aFlag);

    uint32 GetFormat() const { return mFormat; }
    uint32 GetVertexSize() const { return mVertexSize; }
    uint32 GetVertexCount() const { return mVertexCount; }

    void * Lock(IMESH_LOCK aFlag);
    void Unlock();

    bool Bind();
	void Unbind();

    virtual size_t GetMemoryUsed() const { return (sizeof(ZVertexArrayOGL) + mVertexSize*mVertexCount); }

protected:
    uint32 mVertexSize;
    uint32 mFormat;
    uint32 mVertexCount;
	uint32 mFlag;
	bool mbKeepVBORam;

	unsigned long	mVertexBufferSize;
	int				mVertexOffset;
	int				mNormalOffset;
	int				mColorOffset;
	int				mBiNormalOffset;
	int				mBiTangentOffset;
	int				mUV0Offset;
	int				mUV1Offset;
	int				mUV2Offset;
	int				mUV3Offset;
	void			*mRAMBuffer;
	GLuint			mVBOBufferName;
	bool			mLocked;
	IMESH_LOCK		mLockFlag;
    static unsigned int VertexArrayInstancesCount;
    friend class Render;
};
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct light_t
{
	vec_t mColor;
	vec_t mPosition;
}light_t;

typedef struct omniLight_t : public light_t
{
}omniLight_t;

typedef struct spotLight_t : public light_t
{
	vec_t mLightDirection;
	float cos_light_angle_atten, cos_light_angle_atten2;
	float light_atten_begin, light_atten_end;

}spotLight_t;

#define MAX_OCCLUSIONS 16
typedef struct occlusionQuery_t
{
    occlusionQuery_t() { mNbPixelsRendered = 0; }
    int mNbPixelsRendered;
    mesh_t *mMesh;
    matrix_t mWorldMatrix;
    unsigned int mGLOccQuery;
    bool mbReady;
} occlusionQuery_t;

struct skyScatParams_t;
extern skyScatParams_t *GSkyScatParams;

class Renderer
{
public:
	static void Init();
    static void Reset();

    static void UpdateRenderSettings();

	static void setRenderMeshStack(int aStack);
	static void Render( float aTimeEllapsed, Camera *pCam, int splitScreenMode = 0 );
	static void endOfRendering( float aTimeEllapsed, Camera *pCamera, bool hasSplitScreen );
	static void SaveCapture();
    static void DoSaveScreenShot( const char *szName);
	static std::string GetScreenCaptureName();
	static void Tick( float aTimeEllapsed);

	// target
	static void DrawTarget(const vec_t& worldPos, float aProgress) { targetWorldPosition = worldPos; targetProgress = aProgress; } // aProgress == -1.f means remove target

	// black velvet
	static void newBlackVelvet( int playerId );
	static void newVelvet( const vec_t& col, int playerId );
	static void renderVelvet( bool hasSplitScreen );

	// lights
    static void clearAllLights() { mSpots.ForceClear(); mOmnis.ForceClear(); }
	static void destroyOmni(omniLight_t* pOmni)	{ if ( pOmni != NULL ) mOmnis.Delete( pOmni ); }
    static void destroySpot(spotLight_t* pSpot) { if ( pSpot != NULL ) mSpots.Delete( pSpot ); }
    static spotLight_t *newSpot() { return mSpots.New(); }
    static omniLight_t *newOmni() { return mOmnis.New(); }

	// over gui
	static void SetOverGUIMesh( mesh_t * pMesh ) { mOverGuiMesh = pMesh; }
	static void SetOverGUIMeshRectangle( const vec_t& rect ) { mOverGuiMeshRectangle = rect; }
	// lens
	static void AddLensFlare( const vec_t& clipSpacePosition, float flareWidth, float flareStrength, float lensFlareStrength );

    // Occlusion Query
    static occlusionQuery_t *NewOcclusionQuery() { occlusionQuery_t *nocc = mOcclusions.New(); nocc->mNbPixelsRendered = 0; nocc->mbReady = false; return nocc; }
    static void DeleteOcclusionQuery( occlusionQuery_t * pOcc ) { mOcclusions.Delete( pOcc ); }
    static void CheckIndexVertexArrayInstanceCount();

    // explosions
    struct explosion_t
    {
        explosion_t( const matrix_t& fireball, const matrix_t& wave, float alpha )
        {
            mFireballMat = fireball;
            mWaveMat = wave;
            mAlpha = alpha;
			mType = FIRE_BALL;
        }
		explosion_t( const matrix_t& matrix, float alpha, ExplosionType type, bool )
		{
			mFireballMat = matrix;
			mAlpha = alpha;
			mType = type;
		}
        matrix_t mFireballMat;
        matrix_t mWaveMat;
        float mAlpha;
		ExplosionType mType;
    };
    static std::vector<explosion_t> mExplosions;
	static void AddBillBoard( const vec_t& worldPos, float size, int texId) { mBillboards.push_back(BillBoard_t(worldPos, size, texId)); }

	struct portal_t
	{
		float mWidth;
		float mOffset;
		mesh_t *mMesh;
	};
	static std::list<portal_t*> portals;
	static portal_t * addPortal() 
	{ 
		portals.push_back( new portal_t());
		return portals.back(); 
	}
	static void removePortal( portal_t* portal ) 
	{ 
		portals.remove( portal ); 
		delete portal;
	}

protected:
    static ArrayPool<occlusionQuery_t, 32> mOcclusions;
    static ArrayPool<omniLight_t, 1024> mOmnis;
    static ArrayPool<spotLight_t, 1024> mSpots;


	static vec_t targetWorldPosition;
	static float targetProgress;


    static void initShaders();
	static void DrawMovingTarget( const matrix_t& viewProj, int splitScreenMode );
	static void DrawSky( const matrix_t& skyRotate, Camera *pCamera, bool halfScreen, bool bIsSpace );

	// over GUI
	static mesh_t * mOverGuiMesh;
	static vec_t mOverGuiMeshRectangle;

	// lensflare
	typedef struct lensflare_t
	{
		vec_t mClipSpacePosition;
		float mFlareWidth;
		float mFlareStrength;
		float mLensFlareStrength;
	} lensflare_t;

	static lensflare_t mFlares[MAX_NB_FLARES];
	static int mNbFlares;

	struct BillBoard_t
	{
		BillBoard_t(const vec_t& worldPos, float size, int texId)
			:mWorldPos(worldPos), mSize(size), mTexId(texId)
		{
		}
		vec_t mWorldPos;
		int mTexId;
		float mSize;
	};
	static std::vector<BillBoard_t> mBillboards;
	static void DrawBillboard(const matrix_t& viewProj, unsigned int texID, const vec_t& worldPos, float size, int splitScreenMode );
};

void setDOFFocus(float aFocus);
void setDOFBlur(float aBlur);
void enableShowFocalCursor(bool bEnable);

std::string makeScreenShot(int timeSuffix, bool bCaptureGUI = true);
void randomizeAdvertScreens();


#define VS(x) getFileFromMemory("Datas/Shaders/"QUOTE(x)".vs").c_str()
#define FS(x) getFileFromMemory("Datas/Shaders/"QUOTE(x)".fs").c_str()

void initScreenSizeRTs(int Width, int Height);

#endif  //  RENDER_H__
