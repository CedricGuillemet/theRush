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

#include "render.h"
#include "mesh.h"
#include "camera.h"
#include "gui.h"
#include "world.h"
#include "physics.h"
#include "fx.h"
#include "content.h"
#include "therush.h"
#include "game.h"
#include "ocean.h"
#include "track.h"
#include "toolbox.h"
#include "track.h"
#include "solo.h"
#include "gui.h"

#include "include_GL.h"

#include "fxaa3_8_minified.h"

#include "tinythread.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "physics.h"
#include "include_Bullet.h"
#include "bulletOGL/GLDebugDrawer.h"
#include "bulletOGL/GL_ShapeDrawer.h"

#include "bonus.h"

bool GUsePrePass = false;
bool ShouldRenderShadows()
{
#if RENDER_SHADOWS_ENABLE
	const config_t& config = GetEngineConfig();
	return (config.RenderShadows != 0);
#else
	return false;
#endif
}

bool ShouldRenderOcean()
{
#if RENDER_OCEAN_ENABLE
	const config_t& config = GetEngineConfig();
	return (config.RenderOcean != 0);
#else
	return false;
#endif
}

bool ShouldRenderReflection()
{
#if RENDER_REFLECTION_ENABLE
	const config_t& config = GetEngineConfig();
	return (config.RenderReflection != 0);
#else
	return false;
#endif
}

skyScatParams_t *GSkyScatParams = &Tracks[0].mSkyScatteringParameters;
extern bool AADesactivatedByCommandLine, ReflDesactivatedByCommandLine, ShadowsDesactivatedByCommandLine;

Ocean *GOcean = NULL;

bool GBDrawBeacon = true;

ArrayPool<omniLight_t, 1024> Renderer::mOmnis;
ArrayPool<spotLight_t, 1024> Renderer::mSpots;

vec_t Renderer::targetWorldPosition;
float Renderer::targetProgress = -1.f;
bool GDebugDrawPhysic = false;
renderSettings_t RenderSettings;

ArrayPool<occlusionQuery_t, 32> Renderer::mOcclusions;
std::vector<Renderer::BillBoard_t> Renderer::mBillboards;
std::list<Renderer::portal_t*> Renderer::portals;

Renderer::lensflare_t Renderer::mFlares[MAX_NB_FLARES];
int Renderer::mNbFlares = 0;

std::vector<Renderer::explosion_t> Renderer::mExplosions;


unsigned int ZIndexArrayOGL::IndexArrayInstancesCount = 0;
unsigned int ZVertexArrayOGL::VertexArrayInstancesCount = 0;

void Renderer::CheckIndexVertexArrayInstanceCount()
{
	//ASSERT( (!ZIndexArrayOGL::IndexArrayInstancesCount) );
	//ASSERT( (!ZVertexArrayOGL::VertexArrayInstancesCount) );
}

ZIndexArrayOGL::ZIndexArrayOGL() : RefCount_t()
{
	mIndexCount = 0;
	mIndexBufferSize = 0;
	mIndexBufferName = 0;

	mIndexBuffer = NULL;
	mLocked = false;
	IndexArrayInstancesCount++;
}
ZIndexArrayOGL::~ZIndexArrayOGL()
{
	if( glIsBuffer(mIndexBufferName) )
	{
		glDeleteBuffers(1, &mIndexBufferName);
	}
	if (mIndexBuffer)
		free(mIndexBuffer);
	mIndexBuffer = NULL;

	IndexArrayInstancesCount--;
}

void ZIndexArrayOGL::Init(int aQty, uint32 aFlag, bool bUseShort/* = true*/)
{
	ASSERT_GAME( aQty );

	mIndexSize = bUseShort?2:4;
	mFlag = aFlag;
	mIndexCount = aQty;
	if (mIndexBuffer)
		free(mIndexBuffer);

	mIndexBuffer = malloc(aQty * mIndexSize);
	mIndexBufferSize = aQty * mIndexSize;
}

void * ZIndexArrayOGL::Lock(IMESH_LOCK aFlag)
{
	ASSERT_GAME ( (!mLocked) );

	mLockFlag = aFlag;
	mLocked = true;

	ASSERT_GAME ( mIndexBuffer );

	return mIndexBuffer;
}

void ZIndexArrayOGL::Unlock()
{
	mLocked = false;
	if (mLockFlag == VAL_READONLY)
		return;

	if (!mIndexBufferName)
		glGenBuffers(1, &mIndexBufferName);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferName);
	switch (mFlag)
	{
	case VAU_STATIC:
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferSize, mIndexBuffer, GL_STATIC_DRAW);
		break;
	case VAU_STREAM:
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferSize, mIndexBuffer, GL_STREAM_DRAW);
		break;
	case VAU_DYNAMIC:
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferSize, mIndexBuffer, GL_DYNAMIC_DRAW);
		break;


	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void ZIndexArrayOGL::Bind()
{
	ASSERT_GAME( mIndexBufferName );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferName);
}

void ZIndexArrayOGL::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ZVertexArrayOGL::ZVertexArrayOGL() : RefCount_t()
{
	mVertexSize = 0;
	mFormat = 0;
	mVertexCount = 0;
	mRAMBuffer = NULL;
	mVBOBufferName = 0;
	mLocked = false;
	VertexArrayInstancesCount ++;
}

ZVertexArrayOGL::~ZVertexArrayOGL()
{
	if (mRAMBuffer)
		free(mRAMBuffer);

	if( glIsBuffer(mVBOBufferName) )
	{
		glDeleteBuffers(1, &mVBOBufferName);
	}
	VertexArrayInstancesCount --;
}

void ZVertexArrayOGL::Init(uint32 aFormat, int aQty, bool aKeepVBORam, uint32 aFlag)
{
	UNUSED_PARAMETER(aKeepVBORam);

	ASSERT_GAME( (aQty > 0) );
	ASSERT_GAME( aFormat );
	bool bRealloc = ((mFormat != aFormat) || (mVertexCount != static_cast<uint32>(aQty)));
	mFlag = aFlag;
	mbKeepVBORam = true;//aKeepVBORam;

	// buffer
	mVertexCount = aQty;
	mVertexSize = GetVertexSizeFromFormat(aFormat);
	mVertexBufferSize = mVertexSize * mVertexCount;
	mFormat = aFormat;
	int mStride = 0;
	if( aFormat & VAF_XYZ && aFormat & VAF_XYZRHW )
	{
		aFormat ^= VAF_XYZRHW;
	}

	if( aFormat&VAF_XYZ )
	{
		mVertexOffset = mStride;
		mStride += 3*sizeof(float);
	}

	if( aFormat&VAF_XYZRHW )
	{
		mVertexOffset = mStride;
		mStride += 4*sizeof(float);
	}

	if( aFormat&VAF_NORMAL )
	{
		mNormalOffset = mStride;
		mStride += 3*sizeof(float);
	}
	if( aFormat&VAF_COLOR )
	{
		mColorOffset = mStride;
		mStride += 4*sizeof(unsigned char);
	}
	if( aFormat&VAF_BINORMAL )
	{
		mBiNormalOffset = mStride;
		mStride += 2*sizeof(float);
	}
	if( aFormat&VAF_BITANGENT )
	{
		mBiTangentOffset = mStride;
		mStride += 2*sizeof(float);
	}
	if( aFormat&VAF_TEX0 )
	{
		mUV0Offset	= mStride;
		mStride += 2*sizeof(float);
	}
	if( aFormat&VAF_TEX1 )
	{
		mUV1Offset	= mStride;
		mStride += 2*sizeof(float);
	}
	if( aFormat&VAF_TEX2 )
	{
		mUV2Offset	= mStride;
		mStride += 2*sizeof(float);
	}
	if( aFormat&VAF_TEX3 )
	{
		mUV3Offset	= mStride;
		mStride += 2*sizeof(float);
	}

	if (mVBOBufferName)
		glDeleteBuffers(1, &mVBOBufferName);
	mVBOBufferName = 0;

	if (bRealloc)
	{
		if (mRAMBuffer)
			free(mRAMBuffer);
		mRAMBuffer = NULL;

		if (mbKeepVBORam)
			mRAMBuffer = malloc(aQty*mVertexSize);
	}
}

void * ZVertexArrayOGL::Lock(IMESH_LOCK aFlag)
{
	if ( aFlag == VAL_READONLY)
		return mRAMBuffer;

	ASSERT_GAME ( (! mLocked) );
	mLockFlag = aFlag;
	mLocked = true;

	return mRAMBuffer;
}

void ZVertexArrayOGL::Unlock()
{
	mLocked = false;

	if (mLockFlag == VAL_READONLY)
		return;

	if (!mVBOBufferName)
		glGenBuffers(1, &mVBOBufferName);

	glBindBuffer(GL_ARRAY_BUFFER, mVBOBufferName);

	switch (mFlag)
	{
	case VAU_STATIC:
		glBufferData(GL_ARRAY_BUFFER, mVertexBufferSize, mRAMBuffer, GL_STATIC_DRAW);
		break;
	case VAU_STREAM:
		glBufferData(GL_ARRAY_BUFFER, mVertexBufferSize, mRAMBuffer, GL_STREAM_DRAW);
		break;
	case VAU_DYNAMIC:
		glBufferData(GL_ARRAY_BUFFER, mVertexBufferSize, mRAMBuffer, GL_DYNAMIC_DRAW);
		break;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool ZVertexArrayOGL::Bind()
{
	ASSERT_GAME ( mVBOBufferName );

	glBindBuffer(GL_ARRAY_BUFFER, mVBOBufferName);

	if( mFormat&VAF_XYZ )
	{
		glVertexPointer(3, GL_FLOAT, mVertexSize, BUFFER_OFFSET(mVertexOffset));
		glEnableClientState(GL_VERTEX_ARRAY);
	}

	if( mFormat&VAF_XYZRHW )
	{
	}

	if( mFormat&VAF_NORMAL )
	{
		glNormalPointer(GL_FLOAT, mVertexSize, BUFFER_OFFSET(mNormalOffset));
		glEnableClientState(GL_NORMAL_ARRAY);
	}
	if( mFormat&VAF_COLOR )
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, mVertexSize, BUFFER_OFFSET(mColorOffset));
		glEnableClientState(GL_COLOR_ARRAY);
	}
	if( mFormat&VAF_BINORMAL )
	{
	}
	if( mFormat&VAF_BITANGENT )
	{
	}
	if( mFormat&VAF_TEX0 )
	{
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glActiveTextureARB	(GL_TEXTURE0_ARB);
		glTexCoordPointer(2, GL_FLOAT, mVertexSize, BUFFER_OFFSET(mUV0Offset));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if( mFormat&VAF_TEX1 )
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glActiveTextureARB	(GL_TEXTURE1_ARB);
		glTexCoordPointer(2, GL_FLOAT, mVertexSize, BUFFER_OFFSET(mUV1Offset));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if( mFormat&VAF_TEX2 )
	{
	}
	if( mFormat&VAF_TEX3 )
	{
	}

	/*
	ZDisplayDeviceOGL::mD3DDevice->SetVertexDeclaration(mD3D9Declaration);
	ZDisplayDeviceOGL::mD3DDevice->SetStreamSource(0, mD3DBuffer, 0, mVertexSize);
	*/
	return true;
}


void ZVertexArrayOGL::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if( mFormat&VAF_XYZ )
	{
		glVertexPointer(3, GL_FLOAT, 0,0);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	if( mFormat&VAF_XYZRHW )
	{
	}
	if( mFormat&VAF_NORMAL )
	{
		glNormalPointer(GL_FLOAT, 0, 0);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	if( mFormat&VAF_COLOR )
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
		glDisableClientState(GL_COLOR_ARRAY);
	}
	if( mFormat&VAF_BINORMAL )
	{
	}
	if( mFormat&VAF_BITANGENT )
	{
	}
	if( mFormat&VAF_TEX0 )
	{
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glActiveTextureARB	(GL_TEXTURE0_ARB);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if( mFormat&VAF_TEX1 )
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glActiveTextureARB	(GL_TEXTURE1_ARB);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ad screens

struct adScreen_t
{
	float localTime;
	u8 screenRenderedMovie;
	u8 screenRenderedText;

};

adScreen_t alladScreens[8];

void randomizeAdvertScreens()
{
	int movieCount = GetScreenAdsShaderCount();
	for (int i = 0;i<8;i++)
	{
		alladScreens[i].screenRenderedMovie = static_cast<u8>( i%movieCount );//(fastrand()%movieCount);
		alladScreens[i].localTime = 0.f;
		alladScreens[i].screenRenderedText = (fastrand()&7);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// shader_t

serializableField_t shader_t::mSerialisableFields[] = {SED(shader_t,mFragmentShader, "GLSL Fragment shader"),
	SED(shader_t,mVertexShader, "GLSL Vertex shader"),
	SEF(shader_t, mErrorLog, SF_ERRORLOG)
};
NBFIELDS(shader_t);


// renderSettings_t

serializableField_t renderSettings_t::mSerialisableFields[] = {SE(renderSettings_t,PSSMDistance1),
	SE(renderSettings_t,PSSMDistance2),
	SE(renderSettings_t,PSSMDistance3),
	SE(renderSettings_t,PSSMDistance4),
	SEF(renderSettings_t,mShaderTest, SF_TRANSIENT)

};
NBFIELDS(renderSettings_t);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// over GUI mesh
mesh_t * Renderer::mOverGuiMesh = NULL;
vec_t Renderer::mOverGuiMeshRectangle;


void Renderer::UpdateRenderSettings()
{

	initShaders();
}

void TexParam(GLuint MinFilter, GLuint MagFilter, GLuint WrapS, GLuint WrapT, GLuint texMode = GL_TEXTURE_2D)
{
	glTexParameteri( texMode,GL_TEXTURE_MIN_FILTER,MinFilter );
	glTexParameteri( texMode,GL_TEXTURE_MAG_FILTER,MagFilter );
	glTexParameteri( texMode, GL_TEXTURE_WRAP_S, WrapS );
	glTexParameteri( texMode, GL_TEXTURE_WRAP_T, WrapT );
}

void DrawFullScreenQuad()
{
	glBegin(GL_QUADS);

	glVertex4f(-1.f, -1.f, 1.f-FLOAT_EPSILON, 1.f);
	glVertex4f(-1.f, -1.f+ 2.f, 1.f-FLOAT_EPSILON, 1.f);
	glVertex4f(-1.f + 2.f, -1.f+ 2.f, 1.f-FLOAT_EPSILON, 1.f);
	glVertex4f(-1.f +  2.f, -1.f, 1.f-FLOAT_EPSILON, 1.f);

	glEnd();

}


void DrawHalfScreenQuad( int side )
{
	glBegin(GL_QUADS);
	float offset = (float)side * 1.f;
	glVertex4f(-1.f + offset, -1.f, 1.f-FLOAT_EPSILON, 1.f);
	glVertex4f(-1.f + offset, -1.f+ 2.f, 1.f-FLOAT_EPSILON, 1.f);
	glVertex4f(-1.f + 1.f + offset, -1.f+ 2.f, 1.f-FLOAT_EPSILON, 1.f);
	glVertex4f(-1.f + 1.f + offset, -1.f, 1.f-FLOAT_EPSILON, 1.f);

	glEnd();

}

/*
void DrawFullScreenQuad( const vec_t& ofs)
{
glBegin(GL_QUADS);
glVertex4f(-1.f, -1.f, 1.f, 1.f);
glVertex4f(-1.f, -1.f+ 2.f, 1.f, 1.f);
glVertex4f(-1.f + 2.f, -1.f+ 2.f, 1.f, 1.f);
glVertex4f(-1.f +  2.f, -1.f, 1.f, 1.f);
glEnd();

}
*/
class RenderTargetOGL
{
public:
	RenderTargetOGL()
	{
		mGLTexID = 0;   //FIXME: make sure this is the default value for invalid texture in opengl
		fbo = 0;
		depthbuffer = 0;
		mWidth = mHeight = 0;
	}

	~RenderTargetOGL() { Destroy();	}


	void InitTarget(int width, int height, bool hasZBuffer, GLuint otherDepthBuffer = 0);
	void InitFloatTarget(int width, int height );
	void InitTargetDepth(int width, int height);
	void InitGBuffer(int width, int height, bool hasZBuffer);
	void InitCube(int width, int height, bool hasZBuffer);

	void BindAsTarget()
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glViewport(0,0, mWidth, mHeight);
	}

	void Clear();

	GLuint txDepth;
	unsigned int mGLTexID;
	unsigned int mGLTexID2;
	unsigned int mGLTexID3;
	unsigned int mGLTexID4;
	int mWidth, mHeight;
	GLuint fbo;
	GLuint depthbuffer;

	void Destroy()
	{
		glDeleteTextures(1, &mGLTexID);
		glDeleteFramebuffersEXT(1, &fbo);
		glDeleteRenderbuffersEXT(1, &depthbuffer);
		mGLTexID = 0;
		fbo = depthbuffer = 0;
		mWidth = mHeight = 0;
	}

	void BindCube()
	{
		glEnable(GL_TEXTURE_CUBE_MAP_ARB );
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB , mGLTexID);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glViewport(0,0, mWidth, mHeight);
	}

	void BindCubeTarget(int aFace)
	{

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+aFace, mGLTexID, 0);

		//glPushAttrib(GL_VIEWPORT_BIT);

	}

	void CheckFBO();

};


void RenderTargetOGL::Clear()
{
	glClear( GL_COLOR_BUFFER_BIT | (depthbuffer?GL_DEPTH_BUFFER_BIT:0) );
}

void RenderTargetOGL::InitCube(int width, int height, bool hasZBuffer)
{
	/*
	if ( (width == mWidth) && (mHeight == height) && ( !( hasZBuffer ^ (depthbuffer != 0)) ))
	return;
	*/
	Destroy();

	mWidth = width;
	mHeight = height;

	LOG("New Target Cube %dx%d\n", width, height);
	/*
	if (fbo)
	glDeleteFramebuffersEXT( 1, &fbo );
	*/

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	if (hasZBuffer)
	{

		glGenTextures(1, &txDepth);
		glBindTexture(GL_TEXTURE_2D, txDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		TexParam(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, txDepth, 0);



		glGenRenderbuffersEXT(1, &depthbuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);


	}


	glGenTextures(1, &mGLTexID);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, mGLTexID);

	for (int i=0;i<6;i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);


	TexParam(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_TEXTURE_CUBE_MAP_ARB);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, mGLTexID, 0);
	CheckFBO();
}


void RenderTargetOGL::InitTarget(int width, int height, bool hasZBuffer, GLuint otherDepthBuffer)
{
	if ( (width == mWidth) && (mHeight == height) && ( !( hasZBuffer ^ (depthbuffer != 0)) ))
		return;
	Destroy();

	mWidth = width;
	mHeight = height;

	LOG("New Target %dx%d\n", width, height);

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	if (hasZBuffer)
	{
		if (otherDepthBuffer)
		{
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, otherDepthBuffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, otherDepthBuffer);
		}
		else
		{
			glGenTextures(1, &txDepth);
			glBindTexture(GL_TEXTURE_2D, txDepth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			TexParam(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, txDepth, 0);

			glGenRenderbuffersEXT(1, &depthbuffer);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
		}
	}
	glGenTextures(1, &mGLTexID);
	glBindTexture(GL_TEXTURE_2D, mGLTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	TexParam(GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mGLTexID, 0);
	CheckFBO();
}

void RenderTargetOGL::InitFloatTarget(int width, int height )
{
	if ( (width == mWidth) && (mHeight == height) )
		return;
	Destroy();

	mWidth = width;
	mHeight = height;

	LOG("New Target %dx%d\n", width, height);

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	/*
	if (hasZBuffer)
	{
	if (otherDepthBuffer)
	{
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, otherDepthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, otherDepthBuffer);
	}
	else
	{
	glGenTextures(1, &txDepth);
	glBindTexture(GL_TEXTURE_2D, txDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	TexParam(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, txDepth, 0);

	glGenRenderbuffersEXT(1, &depthbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
	}
	}
	*/
	glGenTextures(1, &mGLTexID);
	glBindTexture(GL_TEXTURE_2D, mGLTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	TexParam(GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mGLTexID, 0);
	CheckFBO();
}

void RenderTargetOGL::InitGBuffer(int width, int height, bool hasZBuffer)
{
	if ( (width == mWidth) && (mHeight == height) && ( !( hasZBuffer ^ (depthbuffer != 0)) ))
		return;
	Destroy();

	mWidth = width;
	mHeight = height;

	LOG("New Target %dx%d\n", width, height);

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);


	// diffuse
	glGenTextures(1, &mGLTexID);
	glBindTexture(GL_TEXTURE_2D, mGLTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	TexParam(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mGLTexID, 0);

	// normal
	glGenTextures(1, &mGLTexID2);
	glBindTexture(GL_TEXTURE_2D, mGLTexID2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	TexParam(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mGLTexID2, 0);

	// Z
	glGenTextures(1, &mGLTexID3);
	glBindTexture(GL_TEXTURE_2D, mGLTexID3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F,  width, height, 0, GL_LUMINANCE, GL_FLOAT, NULL);
	TexParam(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, mGLTexID3, 0);

	// specular
	glGenTextures(1, &mGLTexID4);
	glBindTexture(GL_TEXTURE_2D, mGLTexID4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	TexParam(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, mGLTexID4, 0);

	if (hasZBuffer)
	{
		glGenRenderbuffersEXT(1, &depthbuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
	}

	CheckFBO();
}

void RenderTargetOGL::CheckFBO()
{

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
#if 0
	int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{
	case GL_FRAMEBUFFER_COMPLETE_EXT:
		LOG("Framebuffer complete.\n");
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		LOG("[ERROR] Framebuffer incomplete: Attachment is NOT complete." );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
		LOG("[ERROR] Framebuffer incomplete: No image is attached to FBO." );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		LOG("[ERROR] Framebuffer incomplete: Attached images have different dimensions.");
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		LOG("[ERROR] Framebuffer incomplete: Color attached images have different internal formats." );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
		LOG("[ERROR] Framebuffer incomplete: Draw buffer.\n");
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
		LOG("[ERROR] Framebuffer incomplete: Read buffer.\n");
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
		LOG("[ERROR] Unsupported by FBO implementation.\n");
		break;

	default:
		LOG("[ERROR] Unknow error.\n");
		break;
	}
#endif
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void RenderTargetOGL::InitTargetDepth(int width, int height)
{
	if ( (width == mWidth) && (mHeight == height) )
		return;

	mWidth = width;
	mHeight = height;


	glGenTextures(1, &mGLTexID2);
	glBindTexture(GL_TEXTURE_2D, mGLTexID2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Remove artefact on the edges of the shadowmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);








	// Try to use a texture depth component
	glGenTextures(1, &mGLTexID);
	glBindTexture(GL_TEXTURE_2D, mGLTexID);

	// GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
	//TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	/*

	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	*/



	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	*/
	// Remove artefact on the edges of the shadowmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );


	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);



	// create a framebuffer object
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT ,GL_TEXTURE_2D, mGLTexID2, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, mGLTexID, 0);

	/*
	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, mGLTexID, 0);
	*/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	// check FBO status
	CheckFBO();
}




#if (!defined(RETAIL))
void printShaderInfoLog(GLuint obj, const char *debugName, const char *shaderType)
{
	int infologLength = 0;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1)
	{
		char *infoLog = (char *)malloc(infologLength);
		int charsWritten  = 0;
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);

		LOG(" %s(%s) : %s\n", debugName, shaderType, infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj, const char *debugName, const char *programType)
{
	int infologLength = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1)
	{
		char *infoLog = (char *)malloc(infologLength);
		int charsWritten  = 0;
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);

		LOG(" %s(%s) : %s\n", debugName, programType, infoLog);
		free(infoLog);
	}
}
#else
#define printShaderInfoLog(obj, debugName, shaderType)      \
	DECLARE_MACRO_BEGIN \
	(void)sizeof(obj); \
	(void)sizeof(debugName); \
	(void)sizeof(shaderType); \
	DECLARE_MACRO_END

#define printProgramInfoLog(obj, debugName, programType)    \
	DECLARE_MACRO_BEGIN \
	(void)sizeof(obj); \
	(void)sizeof(debugName); \
	(void)sizeof(programType); \
	DECLARE_MACRO_END
#endif

GLuint loadShaderProgram(const char *sourceVertex, const char*sourceFragment, const char* debugName)
{
	GLuint resVertex = sourceVertex?glCreateShader(GL_VERTEX_SHADER):0;
	GLuint resFragment = sourceFragment?glCreateShader(GL_FRAGMENT_SHADER):0;

	if (resVertex)
	{
		glShaderSource(resVertex, 1, &sourceVertex, NULL);
		glCompileShader(resVertex);
		printShaderInfoLog(resVertex, debugName, "Vertex");
	}
	if (resFragment)
	{
		glShaderSource(resFragment, 1, &sourceFragment, NULL);
		glCompileShader(resFragment);
		printShaderInfoLog(resFragment, debugName, "Fragment");
	}

	GLuint p = glCreateProgram();

	if (resVertex)
		glAttachShader(p,resVertex);

	if (resFragment)
		glAttachShader(p,resFragment);

	glLinkProgram(p);

	printProgramInfoLog(p, debugName, "Program");
	return p;
}

FastArray<shader_t*, 100> shader_t::DirtyShaders;

void shader_t::Bind() 
{ 
	glUseProgram( mShaderProgram );
}

void shader_t::SetVec4( GLuint uniformIndex, const vec_t& value )
{
	glUniform4fv( uniformIndex, 1, &value.x );
}
void shader_t::SetVec2( GLuint uniformIndex, const vec_t& value )
{
	glUniform2f( uniformIndex, value.x, value.y );
}
void shader_t::PrintInfoLog( GLuint obj, const char *programType, bool bShader )
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	if ( bShader )
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
	else
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1)
	{
		infoLog = (char *)malloc(infologLength);
		if ( bShader )
			glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		else
			glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);

		char buf[2048];
		sprintf( buf, " %s : %s\n", programType, infoLog );
		mErrorLog += buf;

		free(infoLog);
	}
}

GLuint shader_t::LoadShaderProgram( const char *sourceVertex, const char*sourceFragment )
{
	mErrorLog = "";
	GLuint resVertex = sourceVertex?glCreateShader(GL_VERTEX_SHADER):0;
	GLuint resFragment = sourceFragment?glCreateShader(GL_FRAGMENT_SHADER):0;

	if (resVertex)
	{
		glShaderSource(resVertex, 1, &sourceVertex, NULL);
		glCompileShader(resVertex);
		PrintInfoLog( resVertex, "Vertex", true );
	}
	if (resFragment)
	{
		glShaderSource(resFragment, 1, &sourceFragment, NULL);
		glCompileShader(resFragment);
		PrintInfoLog( resFragment, "Fragment", true );
	}

	GLuint p = glCreateProgram();

	if (resVertex)
		glAttachShader(p,resVertex);

	if (resFragment)
		glAttachShader(p,resFragment);

	glLinkProgram(p);

	PrintInfoLog(p, "Program", false);
	mbIsDirty = false;
	return p;
}


void shader_t::FieldsHasBeenModified()
{
	mbIsDirty = true;
	*DirtyShaders.Push() = this;
}

void shader_t::LoadShader()
{
	if (mShaderProgram)
		glDeleteProgram(mShaderProgram);

	mShaderProgram = LoadShaderProgram( mVertexShader.c_str(), mFragmentShader.c_str() );
}

void shader_t::LoadDirtyShaders()
{
	while (!DirtyShaders.Empty())
	{
		shader_t *shd = *DirtyShaders.Pop();
		shd->LoadShader();
	}
}

GLuint shader_t::GetUniform(const char *szName)
{
	return glGetUniformLocation( mShaderProgram, szName );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// black velvet
float GBlackVelvetTime[2] = {-1.f, -1.f};
vec_t GVelvetColor[2];

int mCurrentFrameBuffer = 0;


float DOFFocusDistance = 20.f;
float DOFBlur = 0.f;
bool showFocalCursor = false;
void enableShowFocalCursor(bool bEnable) { showFocalCursor = bEnable; }
float DOFPlanMarkerValue = 0.f;

void setDOFFocus(float aFocus) 
{ 
	if ( (DOFBlur > FLOAT_EPSILON) && (DOFFocusDistance != aFocus))
		DOFPlanMarkerValue = 1.f;

	DOFFocusDistance = aFocus;

}

void setDOFBlur(float aBlur) 
{ 
	float oldBlur = DOFBlur;
	DOFBlur = aBlur * 0.004f;
	if ( (DOFBlur > FLOAT_EPSILON) && ( fabsf(oldBlur - DOFBlur)>FLOAT_EPSILON) )
		DOFPlanMarkerValue = 1.f;
}

void Renderer::Tick( float aTimeEllapsed )
{
	DOFPlanMarkerValue -= aTimeEllapsed*0.33f;
}

int iMakeScreenShot = 0;
bool mbCaptureGUI = true;
std::string makeScreenShot(int timeSuffix, bool bCaptureGUI) 
{ 
	iMakeScreenShot = timeSuffix;
	mbCaptureGUI = bCaptureGUI;
	return Renderer::GetScreenCaptureName();
}

int GRenderStackIndex = 0;
void Renderer::setRenderMeshStack(int aStack)
{
	GRenderStackIndex = aStack;
}

void Renderer::newBlackVelvet( int playerId )
{
	GBlackVelvetTime[playerId] = 1.f;
	GVelvetColor[playerId] = vec( 0.f );
}
void Renderer::newVelvet( const vec_t& col, int playerId )
{
	GVelvetColor[playerId] = col;
	GBlackVelvetTime[playerId] = 1.f;
}


GLuint velvetShaderProgram = 0;
GLuint velvetAlpha;
GLuint velvetColor;

GLuint skyProgram = 0;
GLuint skyDproj;
GLuint skyColor;
GLuint groundColor;
GLuint skyNormalViewMatrix;
GLuint skyModelViewProjection;

GLuint debugShaderProgram = 0;
GLuint debugModelViewProjectionMatrix;

GLuint overGuiShaderProgram;
GLuint overGuiModelViewProjectionMatrix;


GLuint solidShaderProgram = 0;
GLuint solidModelViewProjectionMatrix;
GLuint solidColor;
GLuint solidNormalViewMatrix;
GLuint solidShader;
GLuint solidShadowMap;
GLuint solidNearPlanes;
GLuint solidFarPlanes;
GLuint solidShadowMatrices;
GLuint solidRotRandom;

GLuint solidICEShaderProgram = 0;
GLuint solidICEModelViewProjectionMatrix;
GLuint solidICEColor;
GLuint solidICENormalViewMatrix;
GLuint solidICEShader;
GLuint solidICEShadowMap;
GLuint solidICENearPlanes;
GLuint solidICEFarPlanes;
GLuint solidICEShadowMatrices;
GLuint solidICERotRandom;



GLuint solidSandShaderProgram = 0;
GLuint solidSandModelViewProjectionMatrix;
GLuint solidSandColor;
GLuint solidSandNormalViewMatrix;
GLuint solidSandShader;
GLuint solidSandShadowMap;
GLuint solidSandNearPlanes;
GLuint solidSandFarPlanes;
GLuint solidSandShadowMatrSands;
GLuint solidSandRotRandom;

GLuint solidRockShaderProgram = 0;
GLuint solidRockModelViewProjectionMatrix;
GLuint solidRockNormalViewMatrix;
GLuint solidRockShader;
GLuint solidRockShadowMap;
GLuint solidRockNearPlanes;
GLuint solidRockFarPlanes;
GLuint solidRockShadowMatrRocks;
GLuint solidRockNormal;
GLuint solidRockDiffuse;
GLuint solidRockRotRandom;

RenderTargetOGL mGBuffer;

RenderTargetOGL mShadowMap;
GLuint mShadowMapMini;
GLuint shadowShaderProgram = 0;
GLuint shadowModelViewProjectionMatrix;

RenderTargetOGL mBloomRT;
RenderTargetOGL mRTBlur;
RenderTargetOGL mFrameBuffer[2];
RenderTargetOGL mFrameBufferScratch[2];
RenderTargetOGL scatCubeTexture;
RenderTargetOGL scatCubeTextureSpace;
RenderTargetOGL scatDownCubeTexture[4];
RenderTargetOGL mReflectionMap;
RenderTargetOGL mReflectionMapMini;
RenderTargetOGL mAdsRt[8];

GLuint shadowedShaderProgram = 0;
GLuint shadowedModelViewProjectionMatrix;
GLuint shadowedWorldMatrix;
GLuint shadowedClipPlane;
GLuint shadowedShadowMatrix;
GLuint shadowedPlanes;
GLuint shadowedNormalMatrix;
GLuint shadowedColor;
GLuint shadowedShader;
GLuint shadowedSunDirection;
GLuint shadowedSunColor;
GLuint shadowedSunAmbientColor;
GLuint shadowedSSAO;
GLuint shadowedShadowMap;
GLuint shadowedCubeAmbient;
GLuint shadowedNearPlanes;
GLuint shadowedFarPlanes;
GLuint shadowedCameraPosition;

GLuint sketchShaderProgram = 0;
GLuint sketchPixelSize, sketchZBuffer, sketchNormalBuffer;
GLuint sketchFogColor, sketchFogDensity;


GLuint sketchFrameBuffer, sketchBloom;

GLuint hblurShaderProgram = 0;
GLuint hblurPixelSize;
GLuint vblurShaderProgram;
GLuint vblurPixelSize;

GLuint bloomShaderProgram;
//GLuint checkerTex;
//GLuint noiseTex;
GLuint rotRandomTex;
GLuint spotTex;
//GLuint transitionTex;

GLuint DOFShaderProgram = 0;
GLuint DOFFocus;
GLuint DOFDepthTexture;
GLuint DOFFramebuffer;
GLuint DOFBias;
GLuint DOFAspectRatio;

GLuint DOFPlanShaderProgram = 0;
GLuint DOFPlanDepthTexture;
GLuint DOFPlanFocus;
GLuint DOFPlanMarker;


GLuint trailShaderProgram = 0;
GLuint trailColor;
GLuint trailEye;
GLuint trailModelViewProjectionMatrix;
GLuint trailWorldMatrix;
GLuint trailTexture;
GLuint trailVDecal;
GLuint trailFogDensity, trailFogColor;
GLuint trailWidth;

GLuint fxaa3ShaderProgram = 0;
GLuint fxaaRcpFrame;
GLuint fxaaRcpFrameOpt;
GLuint fxaaTexture;
GLuint fxaaScreenSize;



GLuint skyScatShaderProgram = 0;
GLuint skyScatViewport;
GLuint skyScatInvProj;
GLuint skyScatInvViewRot;
GLuint skyScatLightDir;
GLuint skyScatKr;
GLuint skyScatRayleighBrightness;
GLuint skyScatMieBrightness;
GLuint skyScatSpotBrightness;
GLuint skyScatScatterStrength;
GLuint skyScatRayleighStrength;
GLuint skyScatMieStrength;
GLuint skyScatRayleighCollectionPower;
GLuint skyScatMieCollectionPower;
GLuint skyScatMieDistribution;

GLuint skyCubeShaderProgram = 0;
GLuint skyCubeViewport;
GLuint skyCubeInvProj;
GLuint skyCubeInvViewRot;
GLuint skyCubeTexSource;


GLuint cubeDownSamplerProgram = 0;
GLuint cubeDownSamplerViewport;
GLuint cubeDownSamplerInvProj;
GLuint cubeDownSamplerInvView;
GLuint cubeDownSamplerTexture;


GLuint oceanShaderProgram = 0;
GLuint oceanShaderViewProj;
GLuint oceanShaderHoleDisks;
GLuint oceanShaderView;
GLuint oceanShaderviewPosition;
GLuint oceanShaderSunShininess;
GLuint oceanShaderSunVec;
GLuint oceanShaderSunColor;
GLuint oceanShaderReflrefrOffset;
GLuint oceanShaderSkyCubeTexture;
GLuint oceanShaderRelfectionTexture;
GLuint oceanShadowMap;
GLuint oceanShadowMatrix;
GLuint oceanNearPlanes;
GLuint oceanFarPlanes;
GLuint oceanViewMat;

GLuint deferedSunProgram = 0;
GLuint deferedSunDeproj;
GLuint deferedSunDir;
GLuint deferedNormalTexture;
GLuint deferedSunSkyCubeTexture;
GLuint deferedSunInvViewMatrix;
GLuint deferedSunColor;
GLuint deferedSunDiffuseTexture;
GLuint deferedSunSSAO;
GLuint deferedSpecularTexture;
GLuint deferedSunViewMatrix;
//GLuint deferedDepthTexture;

GLuint omniShaderProgram = 0;
GLuint omniWorldViewProj;
GLuint omniDeprojection;
GLuint omniPosition;
GLuint omniColor;
GLuint omniAlbedoTexture;
GLuint omniDepthTexture;
GLuint omniNormalTexture;
GLuint omniSpecularTexture;
GLuint omniViewMatrix;

GLuint spotShaderProgram = 0;
GLuint spotWorldViewProj;
GLuint spotDeprojection;
GLuint spotPosition;
GLuint spotColor;
GLuint spotAlbedoTexture;
GLuint spotDepthTexture;
GLuint spotNormalTexture;
GLuint spotAttenBegin;
GLuint spotAttenEnd;
GLuint spotDirection;
GLuint spotCosAngleAtten;
GLuint spotViewMatrix;


GLuint badTVShaderProgram = 0;
GLuint badTVTexture;
GLuint badTVStrength;
GLuint badTVTime;

GLuint simpleBlitProgram = 0;
GLuint blitProgram = 0;
GLuint blitProgramColor = 0;

GLuint blit2DProgram = 0;
GLuint blit2DProgramColor = 0;


GLuint sdsmDepthRedux = 0;

GLuint sdsmFrustraProgram = 0;
GLuint sdsmFrustraDProj;
GLuint sdsmFrustraLightMat;
GLuint sdsmFrustraShadowZPlanes;


GLuint adProgram = 0;
GLuint adMVP;
GLuint adTexScreen;

GLuint finishLineProgram = 0;
GLuint finishLineMVP;
GLuint finishLineTexScreen;



GLuint adsTextShaderProgram = 0;
GLuint adsTextTexture;
GLuint adsTextMVP;

GLuint beaconProgram = 0;
GLuint beaconMVP;
GLuint beaconTex;
GLuint beaconParam1;
GLuint beaconParam2;
GLuint beaconBonusXY;


GLuint beaconTextProgram = 0;
GLuint beaconTextMVP;
GLuint beaconTextTex;
GLuint beaconTextWeaponTex;
GLuint beaconTextBonusXY;

GLuint ropeProgram = 0;
GLuint ropeViewProjectionMatrix;
GLuint ropeSkin;
GLuint ropeShadowMat;
GLuint ropeColor;
GLuint ropeShadowMap;
GLuint ropeSolidShader;
GLuint sopeShadowNearPlanes;
GLuint ropeShadowFarPlanes;
GLuint ropeViewMatrix;


GLuint ropeShadowProgram = 0;
GLuint ropeShadowSkin;
GLuint ropeShadowViewProjectionMatrix;

GLuint explosionProgram = 0;
GLuint explosionTime;
GLuint explosionWeight;
GLuint explosionObjectMatrix;
GLuint explosionModelViewMatrix;
GLuint explosionProjectionMatrix;
GLuint explosionAlpha;

GLuint shockwaveProgram = 0;
GLuint shockwaveAlpha;
GLuint shockwaveMVP;

GLuint auroraProgram = 0;
GLuint auroraMVP;
GLuint auroraTime;
GLuint auroraWorldEyePos;

GLuint portalProgram = 0;
GLuint portalMVP;
GLuint portalWidth;
GLuint portalTime;
GLuint portalOffset;
GLuint portalColor;

shader_t textRenderingShader;

void DrawDofCursor()
{
	// DOF Cursor --------------------------------------------------------------------------------------
	if ((DOFBlur>FLOAT_EPSILON) && showFocalCursor)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(DOFPlanShaderProgram);

		glUniform1iARB(DOFPlanDepthTexture,0);
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,mGBuffer.mGLTexID3);


		glUniform1f(DOFPlanFocus, DOFFocusDistance);
		glUniform1f(DOFPlanMarker, DOFPlanMarkerValue);

		glDisable(GL_CULL_FACE);
		glDepthMask(0);
		DrawFullScreenQuad();

		glDisable(GL_BLEND);
	}
}

inline void Tex0(GLuint parameter, GLuint texID)
{
	glUniform1iARB(parameter,0);
	glActiveTextureARB( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, texID );
}

inline void Tex1(GLuint parameter, GLuint texID)
{
	glUniform1iARB( parameter, 1 );
	glActiveTextureARB( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, texID );
}

inline void Tex2(GLuint parameter, GLuint texID)
{
	glUniform1iARB(parameter, 2);
	glActiveTextureARB( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, texID );
}

inline void Tex3(GLuint parameter, GLuint texID)
{
	glUniform1iARB( parameter, 3 );
	glActiveTextureARB( GL_TEXTURE3 );
	glBindTexture( GL_TEXTURE_2D, texID );
}

void Renderer::DrawSky( const matrix_t& skyRotate, Camera *pCamera, bool halfScreen, bool bIsSpace )
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(1);

	glUseProgram(skyCubeShaderProgram);
	glActiveTextureARB(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, bIsSpace?scatCubeTextureSpace.mGLTexID:scatCubeTexture.mGLTexID);
	glUniform2f(skyCubeViewport, halfScreen?(WIDTH/2):WIDTH, HEIGHT);
	glUniformMatrix4fv(skyCubeInvProj, 1, 0, pCamera->mProjectionInverse.m16);

	/*
	if (bIsSpace)
	{
	static float rx = 0.f, ry = 0.f;
	rx += aTimeEllapsed * 0.1f;
	ry += aTimeEllapsed * 0.01f;
	matrix_t rotatingSpace;
	rotatingSpace.rotationYawPitchRoll( rx, ry, 0.f );

	matrix_t vps = pCamera->mViewInverse * rotatingSpace;

	float vip[9] = {vps.m16[0], vps.m16[1], vps.m16[2],
	vps.m16[4], vps.m16[5], vps.m16[6],
	vps.m16[8], vps.m16[9], vps.m16[10] };

	glUniformMatrix3fv(skyCubeInvViewRot, 1, 0, vip);
	}
	else
	{
	float vip[9] = {pCamera->mViewInverse.m16[0],pCamera->mViewInverse.m16[1],pCamera->mViewInverse.m16[2],
	pCamera->mViewInverse.m16[4],pCamera->mViewInverse.m16[5],pCamera->mViewInverse.m16[6],
	pCamera->mViewInverse.m16[8],pCamera->mViewInverse.m16[9],pCamera->mViewInverse.m16[10] };

	glUniformMatrix3fv(skyCubeInvViewRot, 1, 0, vip);
	}
	*/

	matrix_t vps;
	if ( bIsSpace )
		vps = pCamera->mViewInverse * skyRotate;
	else
		vps = pCamera->mViewInverse;

	float vip[9] = {vps.m16[0], vps.m16[1], vps.m16[2],
		vps.m16[4], vps.m16[5], vps.m16[6],
		vps.m16[8], vps.m16[9], vps.m16[10] };

	glUniformMatrix3fv(skyCubeInvViewRot, 1, 0, vip);

	DrawFullScreenQuad();

	glDisable(GL_TEXTURE_CUBE_MAP_ARB);
}


void Renderer::DrawBillboard(const matrix_t& viewProj, unsigned int texID, const vec_t& worldPos, float size, int splitScreenMode)
{
	// Target ------------------------------------------------------------------------------------------

	static const float targetClipSpaceWidth = 0.5f;//float sizex = 0.5f;
	float targetClipSpaceHeight = (targetClipSpaceWidth / HEIGHT) * WIDTH;
	vec_t targetPos = worldPos;
	if ( Camera::IsBoxOnScreen(viewProj, targetPos, targetClipSpaceWidth*0.5f, targetClipSpaceHeight) )
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glUseProgram(blit2DProgram);


		glActiveTextureARB(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, texID);
		TexParam( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT );

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_CULL_FACE);
		glDepthMask(0);
		glDisable( GL_DEPTH_TEST );

		static const float targetWidth = 0.25f;
		static const vec_t qpos[]={ vec(0.f-targetWidth*(splitScreenMode?2:0), 0.f, 0.f),
			vec(0.f-targetWidth*(splitScreenMode?2:0), 0.f+targetWidth*2.f, 0.f),
			vec(0.f+targetWidth*(splitScreenMode?2:0), 0.f+targetWidth*2.f, 0.f),
			vec(0.f+targetWidth*(splitScreenMode?2:0), 0.f, 0.f) };

		vec_t tmp;

		matrix_t rot, tra, sca, res;
		//rot.rotationZ(targetProgress * (2*PI/3.f));
		tra.translation(targetPos.x, targetPos.y, 0.f);

		glUniform4f( blitProgramColor, 1.f, 1.f, 1.f, 0.95f );




		static const float asc[]= {0.5f, 0.7f, 1.f};
		glBegin(GL_QUADS);
		sca.scale(targetClipSpaceWidth*size, targetClipSpaceHeight*size, 1.f);
		res =sca * tra ;


		tmp.TransformPoint(qpos[0], res);
		glTexCoord2f(0.f, 0.f);
		tmp.z = 0.0f;
		tmp.w = 1.0f;
		glVertex4fv(&tmp.x);

		tmp.TransformPoint(qpos[1], res);
		glTexCoord2f(0.f, 1.f);
		tmp.z = 0.0f;
		tmp.w = 0.0f;
		glVertex4fv(&tmp.x);

		tmp.TransformPoint(qpos[2], res);
		glTexCoord2f(1.f, 1.f);
		tmp.z = 1.0f;
		tmp.w = 0.0f;
		glVertex4fv(&tmp.x);

		tmp.TransformPoint(qpos[3], res);
		glTexCoord2f(1.f, 0.f);
		tmp.z = 1.0f;
		tmp.w = 1.0f;
		glVertex4fv(&tmp.x);

		glEnd();
		glDisable(GL_BLEND);
		glColor4f(1,1,1,1);
	}
}

void Renderer::DrawMovingTarget( const matrix_t& viewProj, int splitScreenMode )
{
	// Target ------------------------------------------------------------------------------------------

	static const float targetClipSpaceWidth = 0.5f;//float sizex = 0.5f;
	float targetClipSpaceHeight = (targetClipSpaceWidth / HEIGHT) * WIDTH;
	vec_t targetPos = targetWorldPosition;
	if ( (targetProgress >= 0.f) &&
		Camera::IsBoxOnScreen(viewProj, targetPos, targetClipSpaceWidth*0.5f, targetClipSpaceHeight) )
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glUseProgram(blit2DProgram);


		glActiveTextureARB(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, GTargetTexture);
		TexParam( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT );

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_CULL_FACE);
		glDepthMask(0);
		glDisable( GL_DEPTH_TEST );

		static const float targetWidth = 0.25f;
		static const vec_t qpos[]={ vec(0.f-targetWidth, -targetWidth, 0.f),
			vec(0.f-targetWidth, 0.f+targetWidth, 0.f),
			vec(0.f+targetWidth, 0.f+targetWidth, 0.f),
			vec(0.f+targetWidth, 0.f-targetWidth, 0.f) };

		

		vec_t tmp;

		matrix_t rot, tra, sca, res;
		//rot.rotationZ(targetProgress * (2*PI/3.f));
		tra.translation(targetPos.x, targetPos.y, 0.f);

		if (targetProgress >= 1.f)
			glUniform4f( blitProgramColor, 0.f, 1.f, 0.f, 0.9f );
		else
			glUniform4f( blitProgramColor, 1.f, 0.f, 0.f, 0.7f );




		static const float asc[]= {0.5f, 0.7f, 1.f};
		glBegin(GL_QUADS);
		for (int i=0;i<3;i++)
		{
			rot.rotationZ((1.f-targetProgress) * (3-i)*1.2f + (i*2.f*PI/3.f));
			sca.scale(targetClipSpaceWidth*asc[i] * (splitScreenMode?2.f:1.f), targetClipSpaceHeight*asc[i], 1.f);
			res =rot * sca * tra ;


			tmp.TransformPoint(qpos[0], res);
			glTexCoord2f(0.f, 0.f);
			tmp.z = 0.0f;
			tmp.w = 0.0f;
			glVertex4fv(&tmp.x);

			tmp.TransformPoint(qpos[1], res);
			glTexCoord2f(0.f, 1.f);
			tmp.z = 0.0f;
			tmp.w = 1.0f;
			glVertex4fv(&tmp.x);

			tmp.TransformPoint(qpos[2], res);
			glTexCoord2f(1.f, 1.f);
			tmp.z = 1.0f;
			tmp.w = 1.0f;
			glVertex4fv(&tmp.x);

			tmp.TransformPoint(qpos[3], res);
			glTexCoord2f(1.f, 0.f);
			tmp.z = 1.0f;
			tmp.w = 0.0f;
			glVertex4fv(&tmp.x);
		}
		glEnd();
		glDisable(GL_BLEND);
		glColor4f(1,1,1,1);
	}
}
void Blur(bool bHorizontal, RenderTargetOGL &source, RenderTargetOGL &dest, unsigned int sourceWidth, unsigned int sourceHeight )
{
	dest.BindAsTarget();

	glUseProgram(bHorizontal?hblurShaderProgram:vblurShaderProgram);
	glUniform2f(bHorizontal?hblurPixelSize:vblurPixelSize, (1.f/(float)sourceWidth), (1.f/(float)sourceHeight));

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,source.mGLTexID);

	glDisable(GL_CULL_FACE);
	glDepthMask(0);
	DrawFullScreenQuad();

}
void Blur(bool bHorizontal, unsigned int source, RenderTargetOGL &dest, unsigned int sourceWidth, unsigned int sourceHeight )
{
	dest.BindAsTarget();

	glUseProgram(bHorizontal?hblurShaderProgram:vblurShaderProgram);
	glUniform2f(bHorizontal?hblurPixelSize:vblurPixelSize, (1.f/(float)sourceWidth), (1.f/(float)sourceHeight));

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,source);

	DrawFullScreenQuad();
}



void RenderSkyScattering(RenderTargetOGL *pCubeTexture, skyScatParams_t *pParam, bool bIsSpace )
{
	if ( !pParam->mbIsDirty )
		return;

	pParam->mbIsDirty = false;

	static const vec_t skyRotsDir[] = { vec(1.f, 0.f, 0.f),
		vec(-1.f, 0.f, 0.f),
		vec(0.f, -1.f, 0.f),
		vec(0.f, 1.f, 0.f),
		vec(0.f, 0.f, 1.f),
		vec(0.f, 0.f, -1.f) };

	static const vec_t skyRotsUp[] = { vec(0.f, -1.f, 0.f),
		vec(0.f, -1.f, 0.f),
		vec(0.f, 0.f, -1.f),
		vec(0.f, 0.f, -1.f),
		vec(0.f, -1.f, 0.f),
		vec(0.f, -1.f, 0.f) };

	matrix_t skyProj;
	skyProj.glhPerspectivef2(90.0f, 1.f, 0.1f, 1000.0f);
	skyProj.inverse();
	matrix_t skyRot;

	if ( !bIsSpace )
	{
		glUseProgram(skyScatShaderProgram);

		glUniformMatrix4fv(skyScatInvProj, 1, 0, skyProj.m16);

		glUniform3f(skyScatLightDir, -GSkyScatParams->mSunDirection.x, GSkyScatParams->mSunDirection.y, -GSkyScatParams->mSunDirection.z);
		glUniform3fv(skyScatKr, 1, &pParam->mKr.x);
		glUniform2f(skyScatViewport, (float)pCubeTexture->mWidth, (float)pCubeTexture->mHeight);
		glUniform1f(skyScatRayleighBrightness, pParam->mRayleighBrightness);
		glUniform1f(skyScatMieBrightness, pParam->mMieBrightness);
		glUniform1f(skyScatSpotBrightness, pParam->mSpotBrightness);
		glUniform1f(skyScatScatterStrength, pParam->mScatterStrength);
		glUniform1f(skyScatRayleighStrength, pParam->mRayleighStrength);
		glUniform1f(skyScatMieStrength, pParam->mMieStrength);
		glUniform1f(skyScatRayleighCollectionPower, pParam->mRayleighCollectionPower);
		glUniform1f(skyScatMieCollectionPower, pParam->mMieCollectionPower);
		glUniform1f(skyScatMieDistribution, pParam->mMieDistribution);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(0);
		/*
		glEnable(GL_TEXTURE_CUBE_MAP_ARB );
		pCubeTexture->BindCubeTarget(1); // not setting that once here cause the target 0 to not be rendererd // BUG
		*/
		pCubeTexture->InitCube(256, 256, false);

		pCubeTexture->BindCube();

		for (int i = 0;i<6;i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		}
		
		pCubeTexture->BindCube();

		for (int i=0;i<6;i++)
		{
			pCubeTexture->BindCubeTarget(i);
			glViewport(0,0,256,256);
			skyRot.LookAt(vec(0.f), skyRotsDir[i], skyRotsUp[i]);

			float vip[9] = {skyRot.m16[0],skyRot.m16[1],skyRot.m16[2],
				skyRot.m16[4],skyRot.m16[5],skyRot.m16[6],
				skyRot.m16[8],skyRot.m16[9],skyRot.m16[10] };

			glUniformMatrix3fv(skyScatInvViewRot, 1, 0, vip);



			DrawFullScreenQuad();
		}
		
	}
	else
	{
		if (!pCubeTexture->mGLTexID)
		{
			pCubeTexture->InitCube(1024, 1024, false);

			pCubeTexture->BindCube();

			extern u32 *GSpaceSkyboxMem[6];
			for (int i = 0;i<6;i++)
			{
				ASSERT_GAME( GSpaceSkyboxMem[i] != NULL );

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, GSpaceSkyboxMem[i] );
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}


	// go for downsamples
	glUseProgram(cubeDownSamplerProgram);
	for (int j=0;j<4;j++)
	{
		glUniform2f(cubeDownSamplerViewport, (float)(128>>j),  (float)(128>>j));
		if (j==0)
			pCubeTexture->BindCube();
		else
			scatDownCubeTexture[j-1].BindCube();
		glViewport( 0, 0, (128>>j), (128>>j) );
		glUniformMatrix4fv(cubeDownSamplerInvProj, 1, 0, skyProj.m16);

		for (int i=0;i<6;i++)
		{
			scatDownCubeTexture[j].BindCubeTarget(i);

			skyRot.LookAt(vec(0.f), skyRotsDir[i], skyRotsUp[i]);

			float vip[9] = {skyRot.m16[0],skyRot.m16[1],skyRot.m16[2],
				skyRot.m16[4],skyRot.m16[5],skyRot.m16[6],
				skyRot.m16[8],skyRot.m16[9],skyRot.m16[10] };

			glUniformMatrix3fv(cubeDownSamplerInvView, 1, 0, vip);

			DrawFullScreenQuad();
		}
	}

	glDisable(GL_TEXTURE_CUBE_MAP_ARB );
	glDisable(GL_BLEND);
	glDepthMask(1);
}

void Renderer::AddLensFlare( const vec_t& clipSpacePosition, float flareWidth, float flareStrength, float lensFlareStrength )
{
	if ( mNbFlares >= MAX_NB_FLARES )
		return;
	mFlares[ mNbFlares ].mClipSpacePosition = clipSpacePosition;
	mFlares[ mNbFlares ].mFlareWidth = flareWidth;
	mFlares[ mNbFlares ].mFlareStrength = flareStrength;
	mFlares[ mNbFlares ].mLensFlareStrength = lensFlareStrength;

	mNbFlares ++;
}

void Renderer::initShaders()
{
	// velvet
	if (velvetShaderProgram) glDeleteProgram(velvetShaderProgram);
	velvetShaderProgram = loadShaderProgram( VS(curtain), FS(curtain), "Curtain");
	velvetAlpha = glGetUniformLocation(velvetShaderProgram,"velvetAlpha");
	velvetColor = glGetUniformLocation(velvetShaderProgram,"velvetColor");

	// debug
	if (debugShaderProgram) glDeleteProgram(debugShaderProgram);
	debugShaderProgram = loadShaderProgram(VS(debug), FS(debug), "Debug");
	debugModelViewProjectionMatrix = glGetUniformLocation(debugShaderProgram,"modelViewProjectionMatrix");

	overGuiShaderProgram = loadShaderProgram(VS(overGui), FS(overGui), "overGUI");
	overGuiModelViewProjectionMatrix = glGetUniformLocation(overGuiShaderProgram,"modelViewProjectionMatrix");


	// solid
	if (solidShaderProgram) glDeleteProgram(solidShaderProgram);
#if IS_OS_MACOSX
	solidShaderProgram = loadShaderProgram(VS(solid), FS(solid.mac), "Solid");    
#else
	solidShaderProgram = loadShaderProgram(VS(solid), FS(solid), "Solid");
#endif
	solidModelViewProjectionMatrix = glGetUniformLocation(solidShaderProgram,"modelViewProjectionMatrix");
	solidColor = glGetUniformLocation(solidShaderProgram,"color");
	solidNormalViewMatrix = glGetUniformLocation(solidShaderProgram,"normalViewMatrix");
	solidShader = glGetUniformLocation(solidShaderProgram,"solidShader");
	solidShadowMap = glGetUniformLocation(solidShaderProgram,"ShadowMap");
	solidNearPlanes = glGetUniformLocation(solidShaderProgram,"shadowNearPlanes");
	solidFarPlanes = glGetUniformLocation(solidShaderProgram,"shadowFarPlanes");
	solidShadowMatrices = glGetUniformLocation(solidShaderProgram,"shadowMat");
	solidRotRandom = glGetUniformLocation(solidShaderProgram,"rotRandomMap");

	// solid ice
	if (solidICEShaderProgram) glDeleteProgram(solidICEShaderProgram);
	solidICEShaderProgram = loadShaderProgram(VS(solidICE), FS(solidICE), "SolidICE");
	solidICEModelViewProjectionMatrix = glGetUniformLocation(solidICEShaderProgram,"modelViewProjectionMatrix");
	solidICEColor = glGetUniformLocation(solidICEShaderProgram,"color");
	solidICENormalViewMatrix = glGetUniformLocation(solidICEShaderProgram,"normalViewMatrix");
	solidICEShader = glGetUniformLocation(solidICEShaderProgram,"solidShader");
	solidICEShadowMap = glGetUniformLocation(solidICEShaderProgram,"ShadowMap");
	solidICENearPlanes = glGetUniformLocation(solidICEShaderProgram,"shadowNearPlanes");
	solidICEFarPlanes = glGetUniformLocation(solidICEShaderProgram,"shadowFarPlanes");
	solidICEShadowMatrices = glGetUniformLocation(solidICEShaderProgram,"shadowMat");
	solidICERotRandom = glGetUniformLocation(solidICEShaderProgram,"rotRandomMap");

	// sand
	if (solidSandShaderProgram) glDeleteProgram(solidSandShaderProgram);
	solidSandShaderProgram = loadShaderProgram(VS(solidSand), FS(solidSand), "SolidSand");
	solidSandModelViewProjectionMatrix = glGetUniformLocation(solidSandShaderProgram,"modelViewProjectionMatrix");
	solidSandColor = glGetUniformLocation(solidSandShaderProgram,"color");
	solidSandNormalViewMatrix = glGetUniformLocation(solidSandShaderProgram,"normalViewMatrix");
	solidSandShader = glGetUniformLocation(solidSandShaderProgram,"solidShader");
	solidSandShadowMap = glGetUniformLocation(solidSandShaderProgram,"ShadowMap");
	solidSandNearPlanes = glGetUniformLocation(solidSandShaderProgram,"shadowNearPlanes");
	solidSandFarPlanes = glGetUniformLocation(solidSandShaderProgram,"shadowFarPlanes");
	solidSandShadowMatrSands = glGetUniformLocation(solidSandShaderProgram,"shadowMat");
	solidSandRotRandom = glGetUniformLocation(solidSandShaderProgram,"rotRandomMap");

	// rock
	if (solidRockShaderProgram) glDeleteProgram(solidRockShaderProgram);
	solidRockShaderProgram = loadShaderProgram(VS(solidRock), FS(solidRock), "SolidRock");
	solidRockModelViewProjectionMatrix = glGetUniformLocation(solidRockShaderProgram,"modelViewProjectionMatrix");
	solidRockNormalViewMatrix = glGetUniformLocation(solidRockShaderProgram,"normalViewMatrix");
	solidRockShader = glGetUniformLocation(solidRockShaderProgram,"solidShader");
	solidRockShadowMap = glGetUniformLocation(solidRockShaderProgram,"ShadowMap");
	solidRockNearPlanes = glGetUniformLocation(solidRockShaderProgram,"shadowNearPlanes");
	solidRockFarPlanes = glGetUniformLocation(solidRockShaderProgram,"shadowFarPlanes");
	solidRockShadowMatrRocks = glGetUniformLocation(solidRockShaderProgram,"shadowMat");
	solidRockNormal = glGetUniformLocation(solidRockShaderProgram,"NormalMap");
	solidRockDiffuse = glGetUniformLocation(solidRockShaderProgram,"DiffuseMap");
	solidRockRotRandom = glGetUniformLocation(solidRockShaderProgram,"rotRandomMap");

	// shadow
	if (shadowShaderProgram) glDeleteProgram(shadowShaderProgram);
	shadowShaderProgram = loadShaderProgram(VS(shadowCaster), FS(shadowCaster), "Shadow");
	shadowModelViewProjectionMatrix = glGetUniformLocation(shadowShaderProgram,"modelViewProjectionMatrix");

	// shadowed
	shadowedShaderProgram = loadShaderProgram(VS(shadowed), FS(shadowed), "Shadowed");
	shadowedModelViewProjectionMatrix = glGetUniformLocation(shadowedShaderProgram,"modelViewProjectionMatrix");
	shadowedWorldMatrix = glGetUniformLocation(shadowedShaderProgram,"worldMatrix");
	shadowedClipPlane = glGetUniformLocation(shadowedShaderProgram,"clipPlane");
	shadowedShadowMatrix = glGetUniformLocation(shadowedShaderProgram,"shadowMat");
	shadowedPlanes = glGetUniformLocation(shadowedShaderProgram,"planes");
	shadowedNormalMatrix = glGetUniformLocation(shadowedShaderProgram,"normalMatrix");
	shadowedColor = glGetUniformLocation(shadowedShaderProgram,"color");
	shadowedSunDirection = glGetUniformLocation(shadowedShaderProgram,"sunDirection");
	shadowedSunColor = glGetUniformLocation(shadowedShaderProgram,"sunColor");
	shadowedSSAO = glGetUniformLocation(shadowedShaderProgram,"ssao");
	shadowedShadowMap = glGetUniformLocation(shadowedShaderProgram,"ShadowMap");
	shadowedCubeAmbient =  glGetUniformLocation(shadowedShaderProgram,"textureAmbient");
	shadowedNearPlanes = glGetUniformLocation(shadowedShaderProgram,"shadowNearPlanes");
	shadowedFarPlanes = glGetUniformLocation(shadowedShaderProgram,"shadowFarPlanes");
	shadowedCameraPosition = glGetUniformLocation(shadowedShaderProgram,"CameraPosition");
	shadowedShader = glGetUniformLocation(shadowedShaderProgram,"shader");

	// sketch
	if (sketchShaderProgram) glDeleteProgram(sketchShaderProgram);
	sketchShaderProgram = loadShaderProgram(VS(fullscreen), FS(bloomFog), "BloomFog");
	sketchPixelSize = glGetUniformLocation(sketchShaderProgram,"PixelSize");
	sketchZBuffer = glGetUniformLocation(sketchShaderProgram,"zBuffer");
	sketchNormalBuffer = glGetUniformLocation(sketchShaderProgram,"normalBuffer");
	sketchFrameBuffer = glGetUniformLocation(sketchShaderProgram,"frameBuffer");
	sketchBloom = glGetUniformLocation(sketchShaderProgram,"bloomBuffer");
	sketchFogDensity = glGetUniformLocation(sketchShaderProgram,"fogDensity");
	sketchFogColor = glGetUniformLocation(sketchShaderProgram,"fogColor");

	// blur
	if (hblurShaderProgram) glDeleteProgram(hblurShaderProgram);
	hblurShaderProgram = loadShaderProgram(VS(fullscreen), FS(hblur), "hblur");
	hblurPixelSize = glGetUniformLocation(hblurShaderProgram,"PixelSize");
	vblurShaderProgram = loadShaderProgram(VS(fullscreen), FS(vblur), "vblur");
	vblurPixelSize = glGetUniformLocation(vblurShaderProgram,"PixelSize");

	// bloom
	if (bloomShaderProgram) glDeleteProgram(velvetShaderProgram);
	bloomShaderProgram = loadShaderProgram(VS(fullscreen), FS(bloom), "Bloom");

	/*
	// Transition
	if (transitionShaderProgram) glDeleteProgram(transitionShaderProgram);
	transitionShaderProgram = loadShaderProgram(proj2DShaderVertex, transitionShaderFragment, "Transition");
	transitionCosSin = glGetUniformLocation(transitionShaderProgram,"cossinng");
	transitionRatio = glGetUniformLocation(transitionShaderProgram,"ratio");
	transitionTexSpot = glGetUniformLocation(transitionShaderProgram,"texSpot");
	transitionTexTransition = glGetUniformLocation(transitionShaderProgram,"texTransition");
	transitionParam = glGetUniformLocation(transitionShaderProgram,"TPARAM1");
	*/
	// DOF
	if (DOFShaderProgram) glDeleteProgram(DOFShaderProgram);
	DOFShaderProgram = loadShaderProgram(VS(fullscreen), FS(DOF), "DOF");
	DOFFocus = glGetUniformLocation(DOFShaderProgram, "focus");
	DOFBias = glGetUniformLocation(DOFShaderProgram, "bias");
	DOFDepthTexture = glGetUniformLocation(DOFShaderProgram, "bgl_DepthTexture");
	DOFFramebuffer = glGetUniformLocation(DOFShaderProgram, "bgl_RenderedTexture");
	DOFAspectRatio = glGetUniformLocation(DOFShaderProgram, "aspectratio");

	// DOF plan
	if (DOFPlanShaderProgram) glDeleteProgram(DOFPlanShaderProgram);
	DOFPlanShaderProgram = loadShaderProgram(VS(fullscreen), FS(DOFFocalPlan), "DOFPlan");
	DOFPlanDepthTexture = glGetUniformLocation(DOFPlanShaderProgram, "bgl_DepthTexture");
	DOFPlanFocus = glGetUniformLocation(DOFPlanShaderProgram, "focus");
	DOFPlanMarker = glGetUniformLocation(DOFPlanShaderProgram, "marker");

	// Trail
	if (trailShaderProgram) glDeleteProgram(trailShaderProgram);
	trailShaderProgram = loadShaderProgram(VS(trail), FS(trail), "Trail");
	trailColor = glGetUniformLocation(trailShaderProgram, "trailColor");
	trailEye = glGetUniformLocation(trailShaderProgram, "vEye");
	trailModelViewProjectionMatrix = glGetUniformLocation(trailShaderProgram, "modelViewProjectionMatrix");
	trailWorldMatrix = glGetUniformLocation(trailShaderProgram, "worldMatrix");
	trailTexture = glGetUniformLocation(trailShaderProgram, "texTrail");
	trailVDecal = glGetUniformLocation(trailShaderProgram, "vDecal");
	trailFogDensity = glGetUniformLocation(trailShaderProgram, "fogDensity");
	trailFogColor = glGetUniformLocation(trailShaderProgram, "fogColor");
	trailWidth = glGetUniformLocation(trailShaderProgram, "Width");

	// FXAA
	// build string
	if (fxaa3ShaderProgram) glDeleteProgram(fxaa3ShaderProgram);
	const std::string& fxaa38 = Fxaa3_8_h;//getFileFromMemory("Datas/Shaders/Fxaa3_8.h");
	const char *fxaaCont = getFileFromMemory("Datas/Shaders/fxaa.fs").c_str();
	char *fxaa3finalShaderString = new char [ fxaa38.length() * 2 ];
	sprintf(fxaa3finalShaderString, fxaaCont, fxaa38.c_str() );
	fxaa3ShaderProgram = loadShaderProgram(VS(fxaa), fxaa3finalShaderString, "FXAA");
	fxaaRcpFrame = glGetUniformLocation(fxaa3ShaderProgram, "ArcpFrame");
	fxaaRcpFrameOpt = glGetUniformLocation(fxaa3ShaderProgram, "ArcpFrameOpt");
	fxaaTexture = glGetUniformLocation(fxaa3ShaderProgram, "colorSourceTexture");
	fxaaScreenSize = glGetUniformLocation(fxaa3ShaderProgram, "screenSize");

	delete [] fxaa3finalShaderString;
	fxaa3finalShaderString = NULL;

	// sky scattering
	if (skyScatShaderProgram) glDeleteProgram(skyScatShaderProgram);
	skyScatShaderProgram = loadShaderProgram(VS(fullscreen), FS(skyScattering), "SkyScat");
	skyScatInvProj = glGetUniformLocation(skyScatShaderProgram, "inv_proj");
	skyScatInvViewRot = glGetUniformLocation(skyScatShaderProgram, "inv_view_rot");
	skyScatLightDir = glGetUniformLocation(skyScatShaderProgram, "lightdir");
	skyScatViewport = glGetUniformLocation(skyScatShaderProgram, "viewport");
	skyScatKr = glGetUniformLocation(skyScatShaderProgram, "Kr");
	skyScatRayleighBrightness = glGetUniformLocation(skyScatShaderProgram, "rayleigh_brightness");
	skyScatMieBrightness = glGetUniformLocation(skyScatShaderProgram, "mie_brightness");
	skyScatSpotBrightness = glGetUniformLocation(skyScatShaderProgram, "spot_brightness");
	skyScatScatterStrength = glGetUniformLocation(skyScatShaderProgram, "scatter_strength");
	skyScatRayleighStrength = glGetUniformLocation(skyScatShaderProgram, "rayleigh_strength");
	skyScatMieStrength = glGetUniformLocation(skyScatShaderProgram, "mie_strength");
	skyScatRayleighCollectionPower = glGetUniformLocation(skyScatShaderProgram, "rayleigh_collection_power");
	skyScatMieCollectionPower = glGetUniformLocation(skyScatShaderProgram, "mie_collection_power");
	skyScatMieDistribution = glGetUniformLocation(skyScatShaderProgram, "mie_distribution");


	// skycube
	if (skyCubeShaderProgram) glDeleteProgram(skyCubeShaderProgram);
	skyCubeShaderProgram = loadShaderProgram(VS(fullscreen), FS(skyCube), "SkyCube");
	skyCubeViewport = glGetUniformLocation(skyCubeShaderProgram, "viewport");
	skyCubeInvProj = glGetUniformLocation(skyCubeShaderProgram, "inv_proj");
	skyCubeInvViewRot = glGetUniformLocation(skyCubeShaderProgram, "inv_view_rot");
	skyCubeTexSource = glGetUniformLocation(skyCubeShaderProgram, "source");

	// cube downsampler
	if (cubeDownSamplerProgram) glDeleteProgram(cubeDownSamplerProgram);
	cubeDownSamplerProgram = loadShaderProgram(VS(fullscreen), FS(cubeDownSampler), "cubeDownSampler");
	cubeDownSamplerViewport = glGetUniformLocation(cubeDownSamplerProgram, "viewport");
	cubeDownSamplerInvProj = glGetUniformLocation(cubeDownSamplerProgram, "inv_proj");
	cubeDownSamplerInvView = glGetUniformLocation(cubeDownSamplerProgram, "inv_view_rot");
	cubeDownSamplerTexture = glGetUniformLocation(cubeDownSamplerProgram, "source");


	// ocean shader
	oceanShaderProgram = loadShaderProgram(VS(ocean), FS(ocean), "OceanShader");
	oceanShaderViewProj = glGetUniformLocation(oceanShaderProgram, "mViewProj");
	oceanShaderviewPosition = glGetUniformLocation(oceanShaderProgram, "view_position");
	oceanShaderSunShininess = glGetUniformLocation(oceanShaderProgram, "sun_shininess");
	oceanShaderSunVec = glGetUniformLocation(oceanShaderProgram, "sun_vec");
	oceanShaderSunColor = glGetUniformLocation(oceanShaderProgram, "sun_color");
	oceanShaderReflrefrOffset = glGetUniformLocation(oceanShaderProgram, "reflrefr_offset");
	oceanShaderHoleDisks = glGetUniformLocation(oceanShaderProgram, "holeDisks");
	oceanShaderSkyCubeTexture = glGetUniformLocation(oceanShaderProgram, "sky");
	oceanShaderRelfectionTexture = glGetUniformLocation(oceanShaderProgram, "reflmap");
	oceanShadowMap = glGetUniformLocation(oceanShaderProgram, "ShadowMap");
	oceanShadowMatrix = glGetUniformLocation(oceanShaderProgram, "shadowMat");
	oceanNearPlanes = glGetUniformLocation(oceanShaderProgram, "shadowNearPlanes");
	oceanFarPlanes = glGetUniformLocation(oceanShaderProgram, "shadowFarPlanes");
	oceanViewMat = glGetUniformLocation(oceanShaderProgram, "mViewMat");

	// defered directionnal
	if (deferedSunProgram) glDeleteProgram(deferedSunProgram);
	deferedSunProgram = loadShaderProgram(VS(fullscreen), FS(deferedSun), "DeferedSun");
	deferedSunDeproj = glGetUniformLocation(deferedSunProgram, "dproj");
	deferedSunDir = glGetUniformLocation(deferedSunProgram, "sunDir");
	deferedNormalTexture = glGetUniformLocation(deferedSunProgram, "textureNormals");
	deferedSunSkyCubeTexture = glGetUniformLocation(deferedSunProgram, "textureAmbient");
	deferedSunInvViewMatrix = glGetUniformLocation(deferedSunProgram, "invViewMatrix");
	deferedSunColor =  glGetUniformLocation(deferedSunProgram, "sunColor");
	deferedSunDiffuseTexture = glGetUniformLocation(deferedSunProgram, "textureDiffuse");
	deferedSpecularTexture = glGetUniformLocation(deferedSunProgram, "textureSpecular");
	deferedSunSSAO = glGetUniformLocation(deferedSunProgram, "textureSSAO");
	deferedSunViewMatrix = glGetUniformLocation(deferedSunProgram, "viewMatrix");
	//deferedDepthTexture = glGetUniformLocation(deferedSunProgram, "depthSampler");

	// defered omni
	if (omniShaderProgram) glDeleteProgram(omniShaderProgram);
	omniShaderProgram = loadShaderProgram(VS(deferedLight), FS(deferedOmni), "DeferedOmni");
	omniWorldViewProj = glGetUniformLocation(omniShaderProgram, "worldViewProj");
	omniDeprojection = glGetUniformLocation(omniShaderProgram, "deprojection");
	omniPosition = glGetUniformLocation(omniShaderProgram, "lightViewPos");
	omniColor = glGetUniformLocation(omniShaderProgram, "lightViewColor");
	omniAlbedoTexture = glGetUniformLocation(omniShaderProgram, "albedoSampler");
	omniDepthTexture = glGetUniformLocation(omniShaderProgram, "depthSampler");
	omniNormalTexture = glGetUniformLocation(omniShaderProgram, "normalSampler");
	omniSpecularTexture = glGetUniformLocation(omniShaderProgram, "textureSpecular");
	omniViewMatrix = glGetUniformLocation(omniShaderProgram, "invView");

	// defered spot
	if (spotShaderProgram) glDeleteProgram(spotShaderProgram);
	spotShaderProgram = loadShaderProgram(VS(deferedLight), FS(deferedSpot), "DeferedSpot");
	spotWorldViewProj = glGetUniformLocation(spotShaderProgram, "worldViewProj");
	spotDeprojection = glGetUniformLocation(spotShaderProgram, "deprojection");
	spotPosition = glGetUniformLocation(spotShaderProgram, "lightViewPos");
	spotColor = glGetUniformLocation(spotShaderProgram, "lightViewColor");
	spotAlbedoTexture = glGetUniformLocation(spotShaderProgram, "albedoSampler");
	spotDepthTexture = glGetUniformLocation(spotShaderProgram, "depthSampler");
	spotNormalTexture = glGetUniformLocation(spotShaderProgram, "normalSampler");
	spotAttenBegin = glGetUniformLocation(spotShaderProgram, "light_atten_begin");
	spotAttenEnd = glGetUniformLocation(spotShaderProgram, "light_atten_end");
	spotDirection = glGetUniformLocation(spotShaderProgram, "light_direction");
	spotCosAngleAtten = glGetUniformLocation(spotShaderProgram, "cos_light_angle_atten");
	spotViewMatrix = glGetUniformLocation(spotShaderProgram, "viewMatrix");

	// bad tv
	if (badTVShaderProgram) glDeleteProgram(badTVShaderProgram);
	badTVShaderProgram = loadShaderProgram(VS(fullscreen), FS(interference), "badTV");
	badTVTime = glGetUniformLocation(badTVShaderProgram, "time");
	badTVTexture = glGetUniformLocation(badTVShaderProgram, "tex0");
	badTVStrength = glGetUniformLocation(badTVShaderProgram, "strength");
	/*
	if (simpleBlitProgram) glDeleteProgram(simpleBlitProgram);
	simpleBlitProgram = loadShaderProgram(VS(fullscreen), FS(coloredBlit), "simpleBlit");
	*/

	if (blitProgram) glDeleteProgram(blitProgram);
	blitProgram = loadShaderProgram(VS(fullscreen), FS(coloredBlit), "Blit");
	blitProgramColor = glGetUniformLocation( blitProgram, "col");

	if (blit2DProgram) glDeleteProgram(blit2DProgram);
	blit2DProgram = loadShaderProgram(VS(coloredBlit), FS(coloredBlit), "Blit2D");
	blit2DProgramColor = glGetUniformLocation( blit2DProgram, "col");


	if (sdsmDepthRedux) glDeleteProgram(sdsmDepthRedux);
	sdsmDepthRedux = loadShaderProgram(VS(sdsmDepth), FS(sdsmDepth), "sdsmRedux");


	/*
	if (sdsmFrustraProgram) glDeleteProgram(sdsmFrustraProgram);
	sdsmFrustraProgram = loadShaderProgram(sdsmFrustraShaderVertex, sdsmFrustraShaderFragment, "sdsmFrustra");
	sdsmFrustraDProj = glGetUniformLocation(sdsmFrustraProgram, "dproj");
	sdsmFrustraLightMat = glGetUniformLocation(sdsmFrustraProgram, "lightMat");
	sdsmFrustraShadowZPlanes = glGetUniformLocation(sdsmFrustraProgram, "zPlanes");
	*/

	// text rendering


	// ad shader
	if (adProgram) glDeleteProgram(adProgram);
	adProgram = loadShaderProgram(VS(ad), FS(ad), "ad");
	adMVP = glGetUniformLocation(adProgram, "modelViewProjectionMatrix");
	adTexScreen = glGetUniformLocation(adProgram, "texScreen");

	// finish line
	if (finishLineProgram) glDeleteProgram(finishLineProgram);
	finishLineProgram = loadShaderProgram(VS(ad), FS(finishLine), "finishLine");
	finishLineMVP = glGetUniformLocation(finishLineProgram, "modelViewProjectionMatrix");
	finishLineTexScreen = glGetUniformLocation(finishLineProgram, "texScreen");

	if (adsTextShaderProgram) glDeleteProgram(adsTextShaderProgram);
	adsTextShaderProgram = loadShaderProgram(VS(adText), FS(adText), "adText");
	adsTextTexture = glGetUniformLocation(adsTextShaderProgram, "adTextTexture");
	adsTextMVP = glGetUniformLocation(adsTextShaderProgram, "mvp");

	// Beacon
	if (beaconProgram) glDeleteProgram(beaconProgram);
	beaconProgram = loadShaderProgram(VS(beacon), FS(beacon), "beacon");
	beaconMVP = glGetUniformLocation(beaconProgram, "modelViewProjectionMatrix");
	beaconTex = glGetUniformLocation(beaconProgram, "TextureSampler");
	beaconParam1 = glGetUniformLocation(beaconProgram, "shininess");
	beaconParam2 = glGetUniformLocation(beaconProgram, "shininess2");
	beaconBonusXY = glGetUniformLocation(beaconProgram, "bonusXY");

	if (beaconTextProgram) glDeleteProgram(beaconTextProgram);
	beaconTextProgram = loadShaderProgram(VS(beaconText), FS(beaconText), "beaconText");
	beaconTextMVP = glGetUniformLocation(beaconTextProgram, "modelViewProjectionMatrix");
	beaconTextTex = glGetUniformLocation(beaconTextProgram, "TextureSampler");
	beaconTextWeaponTex = glGetUniformLocation(beaconTextProgram, "TextureWeapon");
	beaconTextBonusXY = glGetUniformLocation(beaconTextProgram, "bonusXY");


	if (ropeProgram) glDeleteProgram(ropeProgram);
	ropeProgram = loadShaderProgram(VS(rope), FS(rope), "rope");
	ropeViewProjectionMatrix = glGetUniformLocation(ropeProgram, "viewProjectionMatrix");
	ropeSkin = glGetUniformLocation(ropeProgram, "skin");
	ropeShadowMat = glGetUniformLocation(ropeProgram, "shadowMat");
	ropeColor = glGetUniformLocation(ropeProgram, "color");
	ropeShadowMap = glGetUniformLocation(ropeProgram, "ShadowMap");
	ropeSolidShader = glGetUniformLocation(ropeProgram, "solidShader");
	sopeShadowNearPlanes = glGetUniformLocation(ropeProgram, "shadowNearPlanes");
	ropeShadowFarPlanes = glGetUniformLocation(ropeProgram, "shadowFarPlanes");
	ropeViewMatrix = glGetUniformLocation(ropeProgram, "viewMatrix");


	if (ropeShadowProgram) glDeleteProgram(ropeShadowProgram);
	ropeShadowProgram = loadShaderProgram(VS(ropeCaster), FS(ropeCaster), "ropeCaster");
	ropeShadowViewProjectionMatrix = glGetUniformLocation(ropeShadowProgram, "viewProjectionMatrix");
	ropeShadowSkin = glGetUniformLocation(ropeShadowProgram, "skin");

	// explosion
	if (explosionProgram) glDeleteProgram(explosionProgram);
	explosionProgram = loadShaderProgram(VS(explosion), FS(explosion), "explosion");
	explosionTime = glGetUniformLocation(explosionProgram, "time");
	explosionWeight = glGetUniformLocation(explosionProgram, "weight");
	explosionObjectMatrix = glGetUniformLocation(explosionProgram, "objectMatrix");
	explosionModelViewMatrix = glGetUniformLocation(explosionProgram, "modelViewMatrix");
	explosionProjectionMatrix = glGetUniformLocation(explosionProgram, "projectionMatrix");
	explosionAlpha = glGetUniformLocation(explosionProgram, "alpha");

	// shockwave
	if (shockwaveProgram) glDeleteProgram(shockwaveProgram);
	shockwaveProgram = loadShaderProgram(VS(shockwave), FS(shockwave), "shockwave");
	shockwaveAlpha = glGetUniformLocation(shockwaveProgram, "alpha");
	shockwaveMVP = glGetUniformLocation(shockwaveProgram, "mvp");

	// aurora
	if (auroraProgram) glDeleteProgram(auroraProgram);
	auroraProgram = loadShaderProgram(VS(aurora), FS(aurora), "aurora");
	auroraMVP = glGetUniformLocation(auroraProgram, "mvp");
	auroraTime = glGetUniformLocation(auroraProgram, "t");
	auroraWorldEyePos = glGetUniformLocation(auroraProgram, "worldEyePos");

	// portal
	if (portalProgram) glDeleteProgram(portalProgram);
	portalProgram = loadShaderProgram(VS(portal), FS(portal), "portal");
	portalMVP = glGetUniformLocation(portalProgram, "MVP");
	portalWidth = glGetUniformLocation(portalProgram, "Width");
	portalTime = glGetUniformLocation(portalProgram, "Time");
	portalOffset = glGetUniformLocation(portalProgram, "Offset");
	portalColor = glGetUniformLocation(portalProgram, "Color");

	/*
	GLuint  = 0;
	GLuint ropeViewProjectionMatrix;
	GLuint ropeSkin;
	GLuint ropeShadowMat;

	GLuint sdsmFrustraProgram = 0;
	GLuint sdsmFrustraDProj;
	GLuint sdsmLightMat;



	GLuint sdsmDepthRedux = 0;const char *sdsmDepthShaderFragment={"uniform sampler2D depthTex; varying vec2 uv;void main() { "
	"vec4 minmax = vec4(1000.0, 0.0);"
	"for (int y=0;y<8;y++){"
	"for (int x=0;x<8;x++){"
	"float depth = texture2D(depthTex,uv+vec2(x * 0.00078125, y*0.001388889).r;"
	"if (depth>0.0){"
	"minmax.x = min(minmax.x, depth);"
	"minmax.y = min(minmax.y, depth);"
	"} } }"
	"gl_FragColor = minmax;}\0"};
	const char *sdsmDepthShaderVertex={"varying vec2 uv; void main(){gl_Position = gl_Vertex; uv = (vec2( gl_Position.xy ) + vec2( 1.0 ) ) * 0.5; }\0"};


	const char *badTVFragmentShader = {"uniform float time;"
	"uniform sampler2D tex0;"
	"uniform float strength;"
	"varying vec2 uv;"
	"void main(void) {"
	*/
}

void initGeneratedTextures()
{
	// noise
	/*
	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NOISE_RAW_DATA);
	*/
	// spot

	glGenTextures(1, &spotTex);
	glBindTexture(GL_TEXTURE_2D, spotTex);
	TexParam( GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT );

	u8 *spotBits = new u8 [128*128];
	for (int y = 0;y<128;y++)
	{
		for (int x= 0;x<128;x++)
		{
			spotBits[y*128+x] = 1 + (u8)( (sinf((float)x / 128.f * PI) * 0.5f + 0.5f) *
				(sinf((float)y / 128.f * PI) * 0.5f + 0.5f) * 254.f );
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 128, 128, 0, GL_ALPHA, GL_UNSIGNED_BYTE, spotBits);//SPOT_RAW_DATA);
	delete [] spotBits;
	spotBits = NULL;

	glGenTextures(1, &rotRandomTex);
	glBindTexture(GL_TEXTURE_2D, rotRandomTex);
	TexParam( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );

	u8 *rotRandomBits = new u8 [64*64*3];
	for (int y = 0;y<64;y++)
	{
		for (int x= 0;x<64;x++)
		{
			rotRandomBits[ (y*64+x) * 3] = (u8)(cosf(r01()*2.f * PI) * 255.f + 128.f);
			rotRandomBits[ (y*64+x) * 3+1] = (u8)(sinf(r01()*2.f * PI) * 255.f + 128.f);
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, rotRandomBits);//ROTRANDOM_RAW_DATA);
	delete [] rotRandomBits;
	rotRandomBits = NULL;
	//glGenTextures(1, &transitionTex);

	//checkerTex = generateDebugTexture();
}

void initScreenSizeRTs(int Width, int Height)
{
	mGBuffer.InitGBuffer(Width, Height, true);
	mFrameBuffer[0].InitTarget(Width, Height, true, mGBuffer.depthbuffer);
	mFrameBuffer[1].InitTarget(Width, Height, true, mGBuffer.depthbuffer);
	mFrameBufferScratch[0].InitTarget(Width, Height, true, mGBuffer.depthbuffer);
	mFrameBufferScratch[1].InitTarget(Width, Height, true, mGBuffer.depthbuffer);

	mRTBlur.InitTarget(Width>>1, Height>>1, false);
	mBloomRT.InitTarget(Width>>1, Height>>1, false);
}

void initFixedSizeRTs()
{
	mShadowMap.InitTargetDepth(4096, 1024);
	
	glGenTextures(1, &mShadowMapMini);
	glBindTexture(GL_TEXTURE_2D,mShadowMapMini);
	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	float blankShadowMap[]={ 10000.f, 10000.f, 10000.f, 10000.f };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, blankShadowMap);



	//mShadowMap.InitTarget(4096, 1024, true);

	mReflectionMap.InitTarget(512, 512, true);
	mReflectionMapMini.InitTarget(16, 16, true);
	for (int i=0;i<8;i++)
		mAdsRt[i].InitTarget(512, 220, false ); // 21/9 aspect ratio

	// sky

	scatCubeTexture.InitCube(256, 256, false);
	scatDownCubeTexture[0].InitCube(128, 128, false);
	scatDownCubeTexture[1].InitCube(64, 64, false);
	scatDownCubeTexture[2].InitCube(32, 32, false);
	scatDownCubeTexture[3].InitCube(16, 16, false);
	/*
	mShadowMapMini.BindAsTarget();
	glDepthMask(1);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1,1,1,1);
	glClearDepth(0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,0);
	glClearDepth(1.f);
	*/
}

void Renderer::Init()
{
	PROFILER_START(Renderer::init);
	// occlusions init
	for ( ArrayPool<occlusionQuery_t, 32>::poolElt *iter = mOcclusions.GetFirstFree(); iter; iter=iter->GetNext() )
		glGenQueries( 1, &iter->mGLOccQuery);

	// --

	GLeeInit();

	UpdateRenderSettings();

	initGeneratedTextures();

	// render targets
	extern float WIDTH, HEIGHT;
	const int Width = static_cast<int>(WIDTH);
	const int Height = static_cast<int>(HEIGHT);
	initScreenSizeRTs(Width, Height);
	initFixedSizeRTs();

	//ocean
	GOcean = new Ocean(128,256, &oceanCamera);

	// preallocate explosions
	mExplosions.reserve( 16 );

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do

	glViewport(0, 0, Width, Height );
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	PROFILER_END();
}

void Renderer::Reset()
{
	extern float WIDTH, HEIGHT;
	const int Width = static_cast<int>(WIDTH);
	const int Height = static_cast<int>(HEIGHT);
	initScreenSizeRTs(Width, Height);

	glViewport(0, 0, Width, Height );
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

typedef struct meshentry_t
{
	mesh_t *pMesh;
	float Z;
}meshentry_t;

#define MAX_MESH_ENTRY 16384

typedef struct meshbatch_t
{
	meshentry_t mList[MAX_MESH_ENTRY];
	int mAV;
	void addMesh(mesh_t *pm)
	{
		meshentry_t* pme = &mList[mAV++];
		pme->pMesh = pm;
	}

	void addMeshTransparent(mesh_t *pm, Camera *pCamera )
	{
		meshentry_t* pme = &mList[mAV++];
		pme->pMesh = pm;
		vec_t trp;
		trp.TransformPoint(pm->mWorldMatrix.position, pCamera->mView);
		pme->Z = trp.z;
	}

	static int sortTranslucent(const void *p1, const void *p2)
	{
		const meshentry_t *r1 = (const meshentry_t*)p1;
		const meshentry_t *r2 = (const meshentry_t*)p2;

		if (r2->Z < r1->Z) return 1;
		if (r2->Z > r1->Z) return -1;
		return 0;
	}
	void ZSort()
	{
		qsort(mList, mAV, sizeof(meshentry_t), sortTranslucent);

	}

} meshbatch_t;

meshbatch_t mSolidBatch;
meshbatch_t mTransparentBatch;

meshbatch_t mShadowBatch[PSSM_SPLIT_COUNT];

meshbatch_t mReflectionBatch;

meshbatch_t mIceTerrain;
meshbatch_t mRockTerrain;
meshbatch_t mSandTerrain;
meshbatch_t mScreensBatch;
meshbatch_t mPipes;
meshbatch_t mAuroraBatch;

void DebugDraw();
mesh_t* createBoxMarker(const vec_t& pos, const vec_t& col, float sz);

int sortOceanHoles(const void *p1, const void *p2)
{
	const vec_t *r1 = (const vec_t*)p1;
	const vec_t *r2 = (const vec_t*)p2;

	float lenSq1 = r1->x * r1->x + r1->y*r1->y + r1->z*r1->z;
	float lenSq2 = r2->x * r2->x + r2->y*r2->y + r2->z*r2->z;
	if ( lenSq1 < lenSq2 ) return -1;
	if ( lenSq1 > lenSq2 ) return 1;
	return 0;
}

void drawRope( Rope_t* pr, mesh_t *pMesh, const matrix_t& vp, Camera *pCamera )
{

	glUniformMatrix4fv(ropeViewProjectionMatrix, 1, 0, vp.m16);
	glUniformMatrix4fv(ropeViewMatrix, 1, 0, pCamera->mView.m16);

	glUniform4fv(ropeSolidShader, 1, &pMesh->shader.x);

	matrix_t tmpwsm[4];
	for ( int smi = 0 ; smi < 4 ; smi++)
		tmpwsm[smi] = pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];

	glUniformMatrix4fv(ropeShadowMat, 4, 0, (GLfloat*)tmpwsm);
	glUniformMatrix4fv(ropeSkin, 32, false, pr->GetSkinMatrices()->m16 );

	glUniform4fv(ropeColor, 1, &pMesh->color.x );
	drawMesh(pMesh);
}

void drawRopeCaster( Rope_t* pr, mesh_t *pMesh, const matrix_t& vp )
{
	glUniformMatrix4fv(ropeShadowSkin, 32, false, pr->GetSkinMatrices()->m16 );
	glUniformMatrix4fv(ropeShadowViewProjectionMatrix, 1, false, vp.m16 );

	drawMesh(pMesh);
}


int previousSplitMode = 0;
void Renderer::Render( float aTimeEllapsed, Camera *pCamera, int splitScreenMode )
{
	PROFILER_START(Render);

	if ( (previousSplitMode?1:0) != (splitScreenMode?1:0) )
	{
		previousSplitMode = splitScreenMode;
		if (splitScreenMode)
			initScreenSizeRTs((int)(WIDTH/2), (int)(HEIGHT) );
		else
			initScreenSizeRTs( (int)(WIDTH), (int)(HEIGHT) );
	}
	// is space
	bool bIsSpace = ( track.GetCurrentTrack() && (track.GetCurrentTrack()->mEnvironmentIndex == 3) );
	matrix_t skyRotateMatrix;
	if (bIsSpace)
	{
		static float rx = 0.f;
		
		rx += aTimeEllapsed * 0.1f * world.GetGameSpeed();
		matrix_t planetGlobal;
		planetGlobal.LookAt(vec(-1,1,-1), vec(0,0,0), vec(0,1,0));

		matrix_t planetLocal;
		planetLocal.rotationX( rx);

		skyRotateMatrix = planetGlobal * planetLocal;// * ;
	}
	else
	{
		skyRotateMatrix.identity();
	}
	pCamera->SetRotatingSky( skyRotateMatrix );

	// rotatin sky done
	shader_t::LoadDirtyShaders();

	pCamera->project( pCamera->mFov, splitScreenMode?(8.f/9.f):(16.f/9.f), 0.1f, GSkyScatParams->mFarPlane );

	// pass 1:
	//			Trier tous les mesh dans la bonne structure (solid/transparent)
	//			trier les transparents suivant leur Z
	//			les dessiner
	// pass 2:
	//			Shader pour rendu grace a leur couleur, matrice world
	// pass 3:
	//			MRT diffuse/normal/Z & affichage des resultats
	// pass 4:
	//			shader post process de mixage
	//			attention: les mesh transparent ont 1 forward renderer
	// pass 5:
	//
	matrix_t scalem1;
	scalem1.scale(1.f, -1.f, 1.f);

	bool adsVisible[8] = { false, false, false, false, false, false, false, false };

	PROFILER_START(Ocean);

	bool bDrawOcean = false;
	if ( ShouldRenderOcean() )
	{
		bDrawOcean = GSkyScatParams->mbDrawOcean;

		oceanCamera.update(pCamera->mView, pCamera->mProjection, pCamera->mViewInverse, pCamera->mProjectionInverse);
		if (bDrawOcean)
		{
			bool bIsOceanVisible = GOcean->PrepareVisibily(&oceanCamera);
			if (pCamera->mViewInverse.position.y <= 0.f)
				bIsOceanVisible = false;

			if (bIsOceanVisible)
				GOcean->ComputeMesh( aTimeEllapsed * world.GetGameSpeed() );

			bDrawOcean = bIsOceanVisible;
		}
	}
	PROFILER_END(); // Ocean

	PROFILER_START(MeshSorting);

	glDisable(GL_TEXTURE_2D);
	matrix_t viewProj = pCamera->mView * pCamera->mProjection ;
	pCamera->mViewProj = viewProj;//mView * mProjection;
	// sorting
	int av[PSSM_SPLIT_COUNT+1];
	memset(av, 0, sizeof(int)*(PSSM_SPLIT_COUNT+1));//= 0;
	mSolidBatch.mAV = 0;
	mTransparentBatch.mAV = 0;

	for (int i = 0;i<PSSM_SPLIT_COUNT;i++)
		mShadowBatch[i].mAV = 0;

	mReflectionBatch.mAV = 0;

	mIceTerrain.mAV = 0;
	mSandTerrain.mAV = 0;
	mRockTerrain.mAV = 0;
	mScreensBatch.mAV = 0;
	mPipes.mAV = 0;
	mAuroraBatch.mAV = 0;

	const float PSSMDistance0 = RenderSettings.PSSMDistance0;
	const float PSSMDistance1 = RenderSettings.PSSMDistance1;
	const float PSSMDistance2 = RenderSettings.PSSMDistance2;
	const float PSSMDistance3 = RenderSettings.PSSMDistance3;
	const float PSSMDistance4 = RenderSettings.PSSMDistance4;

	mesh_t*pm = GMeshes[GRenderStackIndex].first;
	while(pm)
	{
		//mesh_t *pm = (*iter);
		if (pm->visible)
		{
			if (pCamera->mFrustum.SphereInFrustumVis(pm->worldBSphere) )
			{
				if (!pm->onlyVisibleForShadows)
				{
					if ( pm->screenMesh)
					{
						adsVisible[ pm->screenMesh - 1 ] |= true;
						mScreensBatch.addMesh( pm );
					}
					else if ( (pm->color.w<0.5f) || (pm->mbTransparent) )
					{
						mTransparentBatch.addMeshTransparent( pm, pCamera ); // transparent
					}
					else
					{
						if (pm->mbIsIceGround )
							mIceTerrain.addMesh( pm ); // solid

						else if (pm->mbIsRock )
							mRockTerrain.addMesh( pm );
						else if (pm->mbIsSand )
							mSandTerrain.addMesh( pm );
						else if (pm->mRope)
							mPipes.addMesh( pm ); // pipe
						else if (pm->mbIsAurora)
							mAuroraBatch.addMesh( pm );
						else
							mSolidBatch.addMesh( pm ); // solid
					}
					av[0]++;
				}
			}

			if ( ShouldRenderShadows() && pm->mbCastShadows && (!splitScreenMode) )
			{
				// shadows
				for (int i=0;i<PSSM_SPLIT_COUNT;i++)
				{
					if (pCamera->mPSSMFrustum[i].SphereInFrustumVis(pm->worldBSphere))
					{
						mShadowBatch[i].addMesh(pm);
						av[i+1]++;
					}
				}
			}
		}
		pm = pm->mNext;
	}

#if 0    

	int zwidth = WIDTH;
	int zheight = HEIGHT;
	//glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, mGBuffer.fbo);
	//    glReadBuffer(GL_COLOR_ATTACHMENT3_EXT);
	//glReadBuffer(GL_DEPTH_ATTACHMENT_EXT);


	//glReadPixels(0, 0, screenWidth, screenHeight, GL_BGR, GL_UNSIGNED_BYTE, 0);
	//glReadPixels(0, 0, zwidth, zheight, GL_R32F, GL_FLOAT, 0);


	float *tmpTexture = new float [zwidth * zheight * 4 ];
	glBindTexture( GL_TEXTURE_2D, mGBuffer.mGLTexID3 );
	glGetTexImage(	GL_TEXTURE_2D,
		0,
		GL_LUMINANCE,
		GL_FLOAT,
		tmpTexture);


	//GLubyte* src = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
	//OR

	float minz = 1000.f;
	float maxz = 0.f;
	//float *cvimg = (float*) glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
	float *cvimg = tmpTexture;
	float *cvimg2 = cvimg;
	cvimg += zwidth * 50;
	if(cvimg)
	{
		for (int y=50;y<zheight-100;y++)
		{
			for (int x=0;x<zwidth;x++)
			{
				float v = *cvimg++;
				if ( v> FLT_EPSILON  && v<(1000.f-FLT_EPSILON))
				{
					minz = zmin( v, minz );
					maxz = zmax( v, maxz );                    
				}
			}
		}


		static mesh_t* raycastMesh = NULL;
		if (!raycastMesh)
		{
			raycastMesh = createBoxMarker( vec(0.f), vec(1.f, 0.f, 0.f, 1.f), 5.f );
		}

		float depth = cvimg2[(zheight>>1)*zwidth + (zwidth>>1)];
		//printf("Z : %5.4f %5.4f - %5.4f\n", minz, maxz, depth );
		/*vec_t worldPos = pCamera->mViewInverse.position + pCamera->mViewInverse.dir * depth;
		raycastMesh->mWorldMatrix.translation(worldPos);
		raycastMesh->updateWorldBSphere();

		*/

		delete [] tmpTexture;

		//Process src OR cvim->imageData
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);     // release pointer to the mapped buffer
	}
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);


#endif
#if ( defined(TEXTUREREAD) && TEXTUREREAD )
	// float textures
	//http://www.opengl.org/wiki/Floating_point_and_mipmapping_and_filtering

	//2D RGBA16 float
	uint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//In this case, the driver will convert your 32 bit float to 16 bit float
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_BGRA, GL_FLOAT, ptexels);
	//If you have 16 bit float and you want to submit directly to your GPU, you need GL_ARB_half_float_pixel
	//This extension is also core in GL 3.0
	//Example :
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_BGRA, GL_HALF_FLOAT, ptexels_half_float);    
#endif


	// reflection
	if ( ShouldRenderOcean() && ShouldRenderReflection() && bDrawOcean && (!splitScreenMode) && (!ReflDesactivatedByCommandLine) )
	{
		matrix_t reflViewProj, reflectionView, reflectionProj;
		reflectionView = scalem1 * pCamera->mView;
		reflectionProj = pCamera->mProjection *scalem1;
		reflViewProj = reflectionView * reflectionProj;
		ZFrustum reflectionFrustum;
		reflectionFrustum.Update( reflectionView, reflectionProj );

		pm = GMeshes[GRenderStackIndex].first;
		while(pm)
		{
			if (pm->visible && reflectionFrustum.SphereInFrustumVis(pm->worldBSphere) && pm->color.w>=0.5f && (!pm->mbTransparent) && (!pm->mbIsAurora) )
				mReflectionBatch.addMesh(pm);

			pm = pm->mNext;
		}
	}

	// sort translucent
	mTransparentBatch.ZSort();
	mScreensBatch.ZSort();
	PROFILER_END(); // MeshSorting

	// Shadows ---------------------------------------------------------------------------------------------------------

	PROFILER_START(ShadowMaps);

	if ( ShouldRenderShadows() && (!splitScreenMode)&&(!ShadowsDesactivatedByCommandLine) )
	{
		mShadowMap.BindAsTarget();
		glDepthMask(1);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);


		for (int j=0;j<PSSM_SPLIT_COUNT;j++)
		{
			matrix_t shadowViewProj;
			shadowViewProj = pCamera->mPSSMView * pCamera->mPSSMProjection[j];
			glViewport( 1024 * j, 0, 1024, 1024 );

			glUseProgram(shadowShaderProgram);

			for (int i=0;i<mShadowBatch[j].mAV;i++)
			{
				meshentry_t* pme = &mShadowBatch[j].mList[i];
				if ( !pme->pMesh->mRope )
				{
					matrix_t tmp = pme->pMesh->mWorldMatrix * shadowViewProj;
					glUniformMatrix4fv(shadowModelViewProjectionMatrix, 1, 0, tmp.m16);

					drawMesh(pme->pMesh);
				}
			}

			glUseProgram(ropeShadowProgram);
			for (int i=0;i<mShadowBatch[j].mAV;i++)
			{
				meshentry_t* pme = &mShadowBatch[j].mList[i];
				if ( pme->pMesh->mRope )
				{
					drawRopeCaster( pme->pMesh->mRope, pme->pMesh, shadowViewProj );
				}

			}
		}
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
	}

	PROFILER_END(); // ShadowMaps

	// Advertising ------------------------------------------------------------------------------------------------------
	// screens rendering

#if !IS_OS_MACOSX

	matrix_t adViewMat, adProjMat;
	adViewMat.lookAtRH( vec(0.f, 125.f, 0.f), vec(0.f), vec(0.f, 0.f, -1.f ) );
	adProjMat.glhPerspectivef2(45.f, WIDTH/HEIGHT, 0.1f, 2000.f);
	matrix_t adViewProj = adViewMat * adProjMat;

	static float adGlobalTime = 0.f;


	for (int i = 0 ; i < 8 ; i++ )
	{
		adScreen_t& screen = alladScreens[ i ];
		if ( adsVisible[ i ] )
		{
			glDepthMask( 0 );
			glDisable( GL_DEPTH_TEST );
			glEnable( GL_CULL_FACE );

			screen.localTime -= aTimeEllapsed * world.GetGameSpeed();
			if ( screen.localTime <= 0.f )
			{
				screen.localTime = 1.f/25.f; // new update maxed at 40ms

				// render to Texture
				mAdsRt[ i ].BindAsTarget();

				//
				BindScreenAdsShader( screen.screenRenderedMovie );

				glCullFace( GL_FRONT );
				DrawFullScreenQuad();

				// check ads text

				glCullFace( GL_BACK );
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


				glUseProgram(adsTextShaderProgram);
				glDepthMask(1);
				glEnable(GL_CULL_FACE);
				glEnable( GL_DEPTH_TEST );
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc ( GL_GREATER, 0.5 );
				Tex0( adsTextTexture, GAdTextTexture );
				TexParam( GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
				matrix_t tmp2;

				int adTextIndex = screen.screenRenderedText;
				{
					mesh_t* adsmesh = GetAdvertisingMesh( adTextIndex );

					tmp2 = GetAdvertisingAnimationMatrix( adTextIndex, adGlobalTime+screen.localTime+i*10.f ) * adViewProj;

					glUniformMatrix4fv(adsTextMVP, 1, 0, (GLfloat*)tmp2);
					drawMesh(adsmesh);
				}

				glDisable(GL_BLEND);
				glDisable(GL_ALPHA_TEST);
			}
		}
	}    

	adGlobalTime += aTimeEllapsed * 8.f * world.GetGameSpeed();
#endif  // !IS_OS_MACOSX



	glDepthMask( 1 );
	glEnable( GL_DEPTH_TEST );

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


	// draw Sky Scattering cube map ---------------------------------------------------------------------------------------


	RenderSkyScattering(bIsSpace?&scatCubeTextureSpace:&scatCubeTexture, GSkyScatParams, bIsSpace );

	// reflections ------------------------------------------------------------------------------------------------------

	if ( ShouldRenderOcean() && ShouldRenderReflection() && bDrawOcean && mReflectionBatch.mAV && (!splitScreenMode) )
	{
		mReflectionMap.BindAsTarget();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shadowedShaderProgram);

		if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine) )
			Tex0( shadowedShadowMap, mShadowMap.mGLTexID );
		else
			Tex0( shadowedShadowMap, mShadowMapMini );
		TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );


		glUniform1iARB(shadowedCubeAmbient,1);
		glActiveTextureARB(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, scatDownCubeTexture[2].mGLTexID);


		glUniform4f(shadowedNearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
		glUniform4f(shadowedFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);
		glUniform4fv(shadowedSunDirection, 1, &GSkyScatParams->mSunDirection.x);
		glUniform4fv(shadowedSunColor, 1, &GSkyScatParams->mSunColor.x);
		glUniform3fv(shadowedCameraPosition, 1, &pCamera->mViewInverse.position.x );

		matrix_t reflViewProj;
		reflViewProj = scalem1 * pCamera->mView  * pCamera->mProjection * scalem1;
		//reflViewProj = pCamera->mView * pCamera->mProjection;

		glUniform4f(shadowedClipPlane, 0.f, 1.f, 0.f, -5.f );

		for (int i=0;i<mReflectionBatch.mAV;i++)
		{
			meshentry_t* pme = &mReflectionBatch.mList[i];
			matrix_t tmp = pme->pMesh->mWorldMatrix * reflViewProj;
			glUniformMatrix4fv(shadowedModelViewProjectionMatrix, 1, 0, tmp.m16);
			glUniformMatrix4fv(shadowedWorldMatrix, 1, 0, pme->pMesh->mWorldMatrix.m16);
			matrix_t tmp2[4];

			for ( int smi = 0 ; smi < 4 ; smi++)
				tmp2[smi] = pme->pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];//lightView * mTexScale;


			glUniformMatrix4fv(shadowedShadowMatrix, 4, 0, (GLfloat*)tmp2);
			glUniformMatrix4fv(shadowedNormalMatrix, 1, 0, pme->pMesh->mWorldMatrix.m16);
			glUniform4fv(shadowedColor, 1, &pme->pMesh->color.x);
			glUniform4fv(shadowedShader, 1, &pme->pMesh->shader.x);
			drawMesh(pme->pMesh);
		}

		glActiveTextureARB(GL_TEXTURE1);
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);

	}
	else
	{
		mReflectionMapMini.BindAsTarget();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	// advertising 
	/*
	mAdsRt.BindAsTarget();
	glClearColor(1,1,1,1);
	glClear( GL_COLOR_BUFFER_BIT );
	*/
	// GBuffer ---------------------------------------------------------------------------------------------------------

	PROFILER_START(GBuffer);
	mGBuffer.BindAsTarget();
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
	glDrawBuffers(4, buffers);


	glDepthMask(1);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// prepass
	
	if (GUsePrePass)
	{
		glUseProgram(debugShaderProgram);
		glColorMask( 0, 0, 0, 0 );
		glEnable( GL_DEPTH_TEST );
		glDepthMask( true );
		// solid
		for (int i=0;i<mSolidBatch.mAV;i++)
		{
			meshentry_t* pme = &mSolidBatch.mList[i];
			matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
			glUniformMatrix4fv(debugModelViewProjectionMatrix, 1, 0, tmp.m16);
			drawMesh(pme->pMesh);
		}
		for (int i=0;i<mIceTerrain.mAV;i++)
		{
			meshentry_t* pme = &mIceTerrain.mList[i];
			matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
			glUniformMatrix4fv(debugModelViewProjectionMatrix, 1, 0, tmp.m16);
			drawMesh(pme->pMesh);
		}
		for (int i=0;i<mRockTerrain.mAV;i++)
		{
			meshentry_t* pme = &mRockTerrain.mList[i];
			matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
			glUniformMatrix4fv(debugModelViewProjectionMatrix, 1, 0, tmp.m16);
			drawMesh(pme->pMesh);
		}
		for (int i=0;i<mSandTerrain.mAV;i++)
		{
			meshentry_t* pme = &mSandTerrain.mList[i];
			matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
			glUniformMatrix4fv(debugModelViewProjectionMatrix, 1, 0, tmp.m16);
			drawMesh(pme->pMesh);
		}
	}
	glColorMask( 1,1,1,1 );
	glDepthFunc(GL_LEQUAL);

	glUseProgram(solidShaderProgram);

	if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine)  )
		Tex0( solidShadowMap, mShadowMap.mGLTexID);
	else
		Tex0( solidShadowMap, mShadowMapMini);

	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

	Tex1( solidRotRandom, rotRandomTex);
	TexParam( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );


	vec_t sunDirViewSpace;
	if (bIsSpace)
	{
		/*
		sunDirViewSpace.TransformVector( vec(1.f, 0.f, 0.f), skyRotateMatrix );
		sunDirViewSpace.z = -sunDirViewSpace.z;
		sunDirViewSpace.y = -sunDirViewSpace.y;
		*/
		sunDirViewSpace = vec(1,1,1);
	}
	else
		//sunDirViewSpace.TransformVector( GSkyScatParams->mSunDirection, skyRotateMatrix );
		sunDirViewSpace = GSkyScatParams->mSunDirection;

	//sunDirViewSpace.TransformVector( pCamera->mView );
	sunDirViewSpace.normalize();



	glUniform4f(solidNearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
	glUniform4f(solidFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (int i=0;i<mSolidBatch.mAV;i++)
	{
		meshentry_t* pme = &mSolidBatch.mList[i];
		matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
		glUniformMatrix4fv(solidModelViewProjectionMatrix, 1, 0, tmp.m16);
		glUniform4fv(solidColor, 1, &pme->pMesh->color.x);
		matrix_t tmp2;
		tmp2 = pme->pMesh->mWorldMatrix;// * pCamera->mView;
		glUniformMatrix4fv(solidNormalViewMatrix, 1, 0, tmp2.m16);
		glUniform4fv(solidShader, 1, &pme->pMesh->shader.x);

		matrix_t tmpwsm[4];
		for ( int smi = 0 ; smi < 4 ; smi++)
			tmpwsm[smi] = pme->pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];

		glUniformMatrix4fv(solidShadowMatrices, 4, 0, (GLfloat*)tmpwsm);



		drawMesh(pme->pMesh);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// ICE -------------------------------------------------------------------------------------------------
	glUseProgram(solidICEShaderProgram);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.f);
	if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine)  )
		Tex0( solidICEShadowMap, mShadowMap.mGLTexID);
	else
		Tex0( solidICEShadowMap, mShadowMapMini);
	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

	Tex1( solidICERotRandom, rotRandomTex);
	TexParam( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );


	glUniform4f(solidICENearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
	glUniform4f(solidICEFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);


	for (int i=0;i<mIceTerrain.mAV;i++)
	{
		meshentry_t* pme = &mIceTerrain.mList[i];
		matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
		glUniformMatrix4fv(solidICEModelViewProjectionMatrix, 1, 0, tmp.m16);
		//glUniform4fv(solidColor, 1, &pme->pMesh->color.x);
		matrix_t tmp2;
		tmp2 = pme->pMesh->mWorldMatrix;// * pCamera->mView;
		glUniformMatrix4fv(solidICENormalViewMatrix, 1, 0, tmp2.m16);
		glUniform4fv(solidICEShader, 1, &pme->pMesh->shader.x);

		matrix_t tmpwsm[4];
		for ( int smi = 0 ; smi < 4 ; smi++)
			tmpwsm[smi] = pme->pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];

		glUniformMatrix4fv(solidICEShadowMatrices, 4, 0, (GLfloat*)tmpwsm);

		drawMesh(pme->pMesh);
	}
	// rock ---------------------------------------------------------------------------------------------------
	glUseProgram(solidRockShaderProgram);

	if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine)  )
		Tex0( solidRockShadowMap, mShadowMap.mGLTexID);
	else
		Tex0( solidRockShadowMap, mShadowMapMini);

	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

	Tex1( solidRockRotRandom, rotRandomTex);
	TexParam( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );

	Tex2( solidRockNormal, GRockNormalTexture);
	TexParam( GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT );

	int rockTexIndex = 0;
	if ( track.GetCurrentTrack() )
		rockTexIndex = track.GetCurrentTrack()->mRockGradiant;

	Tex3( solidRockDiffuse, GRockDiffuseTexture[rockTexIndex&7]);
	TexParam( GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT );



	glUniform4f(solidRockNearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
	glUniform4f(solidRockFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);

	for (int i=0;i<mRockTerrain.mAV;i++)
	{
		meshentry_t* pme = &mRockTerrain.mList[i];
		matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
		glUniformMatrix4fv(solidRockModelViewProjectionMatrix, 1, 0, tmp.m16);
		matrix_t tmp2;
		tmp2 = pme->pMesh->mWorldMatrix;// * pCamera->mView;
		glUniformMatrix4fv(solidRockNormalViewMatrix, 1, 0, tmp2.m16);
		glUniform4fv(solidRockShader, 1, &pme->pMesh->shader.x);

		matrix_t tmpwsm[4];
		for ( int smi = 0 ; smi < 4 ; smi++)
			tmpwsm[smi] = pme->pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];

		glUniformMatrix4fv(solidRockShadowMatrRocks, 4, 0, (GLfloat*)tmpwsm);

		drawMesh(pme->pMesh);
	}

	// sand ---------------------------------------------------------------------------------------------------
	glUseProgram(solidSandShaderProgram);

	glLineWidth(3.f);
	if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine)  )
		Tex0( solidSandShadowMap, mShadowMap.mGLTexID);
	else
		Tex0( solidSandShadowMap, mShadowMapMini);

	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

	Tex1( solidSandRotRandom, rotRandomTex);
	TexParam( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );


	glUniform4f(solidSandNearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
	glUniform4f(solidSandFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);


	for (int i=0;i<mSandTerrain.mAV;i++)
	{
		meshentry_t* pme = &mSandTerrain.mList[i];
		matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
		glUniformMatrix4fv(solidSandModelViewProjectionMatrix, 1, 0, tmp.m16);
		matrix_t tmp2;
		tmp2 = pme->pMesh->mWorldMatrix;// * pCamera->mView;
		glUniformMatrix4fv(solidSandNormalViewMatrix, 1, 0, tmp2.m16);
		glUniform4fv(solidSandShader, 1, &pme->pMesh->shader.x);

		matrix_t tmpwsm[4];
		for ( int smi = 0 ; smi < 4 ; smi++)
			tmpwsm[smi] = pme->pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];

		glUniformMatrix4fv(solidSandShadowMatrSands, 4, 0, (GLfloat*)tmpwsm);

		drawMesh(pme->pMesh);
	}


	// pipes ---------------------------------------------------------------------------------------------------

	glUseProgram(ropeProgram);

	if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine)  )
		Tex0( ropeShadowMap, mShadowMap.mGLTexID);
	else
		Tex0( ropeShadowMap, mShadowMapMini);

	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );


	glUniform4f(sopeShadowNearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
	glUniform4f(ropeShadowFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);


	for (int i=0;i<mPipes.mAV;i++)
	{
		meshentry_t* pme = &mPipes.mList[i];
		Rope_t *pr = pme->pMesh->mRope;
		drawRope( pr, pme->pMesh, viewProj, pCamera );
	}

	// ad Screens ----------------------------------------------------------------------------------------------

	if ( mScreensBatch.mAV )
	{
#if !IS_OS_MACOSX
		glUseProgram(adProgram);

		glDisable(GL_CULL_FACE);


		glUniform1iARB(adTexScreen,0);
		glActiveTextureARB(GL_TEXTURE1);
		TexParam( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );

		for (int i=0;i<mScreensBatch.mAV;i++)
		{
			meshentry_t* pme = &mScreensBatch.mList[i];

			Tex1( adTexScreen, mAdsRt[ pme->pMesh->screenMesh-1 ].mGLTexID );
			TexParam( GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

			matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
			glUniformMatrix4fv(adMVP, 1, 0, tmp.m16);

			drawMesh(pme->pMesh);
		}
#endif  // !IS_OS_MACOSX
	}






	// Occlusion ------------------------------------------------------------------------------------------------------
	// occlusion queries ---------------------------------------------------------------------------------------------
	//for (int i=0;i<mNbOcclusions;i++)
	//#if !IS_OS_MACOSX
	for ( ArrayPool<occlusionQuery_t, 32>::poolElt *iter = mOcclusions.GetFirst(); iter; iter=iter->GetNext() )
	{
		//int queryParam = 0;
		GLint currentPixRendered = 0;
		if ( iter->mbReady )
			glGetQueryObjectiv( iter->mGLOccQuery,  GL_QUERY_RESULT,  &currentPixRendered);

		iter->mNbPixelsRendered >>= 2;
		iter->mNbPixelsRendered += currentPixRendered;
	}
	if ( mOcclusions.GetFirst() )
	{
		glUseProgram(debugShaderProgram);
		glColorMask( 0, 0, 0, 0 );
		glEnable( GL_DEPTH_TEST );
		glDepthMask( 0 );
		//for (int i=0;i<mNbOcclusions;i++)
		for ( ArrayPool<occlusionQuery_t, 32>::poolElt *iter = mOcclusions.GetFirst(); iter; iter=iter->GetNext() )
		{
			if (!iter->mMesh)
				continue;
			//const occlusionQuery_t& pOcc = mOcclusions[i];

			glBeginQuery( GL_SAMPLES_PASSED, iter->mGLOccQuery );

			matrix_t tmp;
			tmp = iter->mWorldMatrix * viewProj;
			glUniformMatrix4fv(debugModelViewProjectionMatrix, 1, 0, tmp.m16);

			drawMesh( iter->mMesh );

			glEndQuery( GL_SAMPLES_PASSED );
			iter->mbReady = true;
		}
		glColorMask( 1, 1, 1, 1 );
		glDepthMask( 1 );
	}
	//#endif    // !IS_OS_MACOSX


	// Ocean ------------------------------------------------------------------------------------------------------

	if ( ShouldRenderOcean() && bDrawOcean )
	{
		PROFILER_START(RenderOcean);

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(1);
		glUseProgram(oceanShaderProgram);

		glUniformMatrix4fv( oceanShaderViewProj, 1, 0, viewProj.m16 );
		matrix_t oceanCamView =  pCamera->mView;
		/*
		oceanCamView.position = vec(0.f, 0.f, 0.f, 1.f);
		oceanCamView.orthoNormalize();
		//oceanCamView.identity();
		//oceanCamView.transpose();
		*/
		glUniformMatrix4fv( oceanViewMat, 1, 0, oceanCamView.m16 );
		glUniform4fv(oceanShaderviewPosition, 1, &pCamera->mViewInverse.position.x);
		glUniform3fv(oceanShaderSunVec, 1, &GSkyScatParams->mSunDirection.x);
		glUniform1f(oceanShaderSunShininess, 10.f);
		glUniform4fv(oceanShaderSunColor, 1, &GSkyScatParams->mSunColor.x);

		glUniform1iARB(oceanShaderSkyCubeTexture,0);
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, bIsSpace?scatCubeTextureSpace.mGLTexID:scatCubeTexture.mGLTexID);


		

		if ( ShouldRenderOcean() && ShouldRenderReflection() && bDrawOcean && mReflectionBatch.mAV && (!splitScreenMode) )
			Tex1( oceanShaderRelfectionTexture, mReflectionMap.mGLTexID);
		else
			Tex1( oceanShaderRelfectionTexture, mReflectionMapMini.mGLTexID);
			


		if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine)  )
			Tex2( oceanShadowMap, mShadowMap.mGLTexID);
		else
			Tex2( oceanShadowMap, mShadowMapMini);

		TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

		glUniformMatrix4fv(oceanShadowMatrix, 4, 0, (GLfloat*)pCamera->mPSSMShadowMatAtlas);

		glUniform4f(oceanNearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
		glUniform4f(oceanFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);


		// ocean holes

		vec_t tmpHoles[MAX_TRACK_OCEAN_HOLES];
		vec_t camPos = vec(pCamera->mViewInverse.position.x, pCamera->mViewInverse.position.y, pCamera->mViewInverse.position.z);
		for ( int i = 0 ;  i < track.mTrackHolesAv ; i++ )
			tmpHoles[i] = track.mTrackHoles[i] - camPos;
		for ( int i = track.mTrackHolesAv ;  i < 4 ; i++ )
			tmpHoles[i] = vec(0.f);


		if (track.mTrackHolesAv > 4)
			qsort(tmpHoles, track.mTrackHolesAv, sizeof(vec_t), sortOceanHoles);
		for ( int i = 0 ;  i < 4 ; i++ )
			tmpHoles[i] += camPos;

		glUniform4fv( oceanShaderHoleDisks,  4, &tmpHoles[0].x);

		GOcean->Draw();

		PROFILER_END(); // RenderOcean
	}

	/*
	for (int i=0;i<mTransparentBatch.mAV;i++)
	{
	meshentry_t* pme = &mTransparentBatch.mList[i];
	matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
	glUniformMatrix4fv(solidModelViewProjectionMatrix, 1, 0, tmp.m16);
	glUniform4fv(solidColor, 1, &pme->pMesh->color.x);
	matrix_t tmp2;
	tmp2 = pme->pMesh->mWorldMatrix * pCamera->mView;
	glUniformMatrix4fv(solidNormalViewMatrix, 1, 0, tmp2.m16);


	matrix_t tmpwsm[4];
	for ( int smi = 0 ; smi < 4 ; smi++)
	tmpwsm[smi] = pme->pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];

	glUniformMatrix4fv(solidShadowMatrices, 4, 0, (GLfloat*)tmpwsm);

	drawMesh(pme->pMesh);
	}
	*/

	PROFILER_END(); // GBuffer

	// WHITE
	/*
	mGBuffer.BindAsTarget();
	GLenum buffers2[] = { GL_COLOR_ATTACHMENT0_EXT};
	glDrawBuffers(1, buffers2);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glColorMask(1,1,1,0);

	glUseProgram( velvetShaderProgram );
	glUniform1f( velvetAlpha, 1.f );
	//glUniform3f( velvetColor, 0.5f,0.5f,0.5f );
	glUniform3f( velvetColor, 1.f, 1.f, 1.f );
	DrawFullScreenQuad( );

	glUseProgram(0);


	glColorMask(1,1,1,1);
	*/

	// defered sun ------------------------------------------------------------------------------------------------------
	PROFILER_START(Shadowed);

	mFrameBufferScratch[0].BindAsTarget();
	glDepthMask(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(deferedSunProgram);

	Tex0( deferedNormalTexture, mGBuffer.mGLTexID2 );

	glUniform1iARB(deferedSunSkyCubeTexture,1);
	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, scatDownCubeTexture[2].mGLTexID);

	Tex2( deferedSunDiffuseTexture, mGBuffer.mGLTexID );
	Tex3( deferedSpecularTexture, mGBuffer.mGLTexID4 );

	glUniform3fv(deferedSunDir, 1, &sunDirViewSpace.x);
	glUniform3fv(deferedSunColor, 1, &GSkyScatParams->mSunColor.x);
	glUniformMatrix4fv(deferedSunInvViewMatrix, 1, 0, pCamera->mViewInverse.m16);
	glUniform4f(deferedSunDeproj, -1.f/pCamera->mProjection.m16[0], -1.f/pCamera->mProjection.m16[5], 0.f, 0.f);
	glUniformMatrix4fv(deferedSunViewMatrix, 1, 0, pCamera->mViewInverse.m16);

	DrawFullScreenQuad();

	PROFILER_END(); // Shadowed


	// Omnis and spots ----------------------------------------------------------------------------------------------

	glEnable(GL_DEPTH_TEST);

	if (!mOmnis.empty())
	{

		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_BLEND);

		glUseProgram(omniShaderProgram);

		Tex0( omniNormalTexture, mGBuffer.mGLTexID2 );
		Tex1( omniDepthTexture, mGBuffer.mGLTexID3 );
		Tex2( omniAlbedoTexture, mGBuffer.mGLTexID );
		Tex3( omniSpecularTexture, mGBuffer.mGLTexID4 );

		glUniform2f(omniDeprojection, 1.f/pCamera->mProjection.m16[0], 1.f/pCamera->mProjection.m16[5]);

		float *pcvm = pCamera->mView.m16;
		float invView[9]={pcvm[0],pcvm[1],pcvm[2],
			pcvm[4],pcvm[5],pcvm[6],
			pcvm[8],pcvm[9],pcvm[10] };
		glUniformMatrix3fv(omniViewMatrix, 1, 0,  invView);

		matrix_t omniSphereMatrix;
		omniSphereMatrix.identity();
		mesh_t *omniMeshPrefab = GetGeoSpherePrefab( 4 );

		//glEnableClientState(GL_VERTEX_ARRAY);
		//glVertexPointer(3, GL_FLOAT, 24, &omniMeshPrefab->vertices[0].x);
		omniMeshPrefab->mVA->Bind();
		omniMeshPrefab->mIA->Bind();
		const vec_t& viewerPos = pCamera->mViewInverse.position;
		//for (; iteromnis != mOmnis.end() ; ++iteromnis)
		for ( ArrayPool<omniLight_t, 1024>::poolElt *iter = mOmnis.GetFirst(); iter; iter=iter->GetNext() )
		{
			omniLight_t *pomni = iter;

			if ( pomni->mColor.w < FLOAT_EPSILON )
				continue;
			if (!pCamera->mFrustum.SphereInFrustum(pomni->mPosition))
				continue;

			vec_t camToLight = (viewerPos - pomni->mPosition);
			camToLight.w = 0.f;
			bool bInversed = (camToLight.lengthSq() <= (pomni->mPosition.w * pomni->mPosition.w));
			if (bInversed)
			{
				glDisable(GL_CULL_FACE);
				glDisable(GL_DEPTH_TEST);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);
			}


			vec_t omniViewPos;

			omniViewPos.TransformPoint(vec(pomni->mPosition.x, pomni->mPosition.y, pomni->mPosition.z, 1.f), pCamera->mView );
			omniViewPos.z = -omniViewPos.z;
			omniViewPos.w = 1.f/(pomni->mPosition.w*pomni->mPosition.w);

			glUniform4fv( omniPosition, 1, &omniViewPos.x );
			glUniform4fv( omniColor, 1, &pomni->mColor.x );

			omniSphereMatrix.m16[10] = omniSphereMatrix.m16[5] = omniSphereMatrix.m16[0] = pomni->mPosition.w * 2.f;

			omniSphereMatrix.position = vec( pomni->mPosition.x, pomni->mPosition.y, pomni->mPosition.z, 1.f );

			matrix_t omniwvp = omniSphereMatrix * viewProj;
			glUniformMatrix4fv( omniWorldViewProj, 1, 0, omniwvp.m16 );

			glDrawElements(	GL_TRIANGLES, omniMeshPrefab->triCount, GL_UNSIGNED_SHORT, 0 );

		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
	}
#if 0
	if (0)
	{
		//spots

		static mesh_t *cone = NULL;
		if (!cone)
		{
			cone = generateLightCone(16);

			cone->visible = false;
			/*
			cone->color = vec(1.f, 1.f, 0.f, 0.5f);
			cone->computeBSphere();
			matrix_t coneSize, coneMat;
			coneSize.scale(5.f, 5.f, 10.f);
			coneMat.LookAt(vec(0.f, 5.f, 0.f), vec(5.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));

			cone->mWorldMatrix = coneSize * coneMat;
			cone->updateWorldBSphere();
			*/
		}

		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_BLEND);

		glUseProgram(spotShaderProgram);

		vec_t spotSource = vec(0.f, 5.f, 0.f, 1.f);
		vec_t spotDest = vec(5.f, 0.f, 0.f, 1.f);
		vec_t spotDir = normalized( spotDest-spotSource );
		glUniform3fv(spotDirection, 1, &spotDir.x);

		/*
		spotDeprojection = glGetUniformLocation(spotShaderProgram, "deprojection");
		spotPosition = glGetUniformLocation(spotShaderProgram, "lightViewPos");
		spotColor = glGetUniformLocation(spotShaderProgram, "lightViewColor");
		//spotAlbedoTexture = glGetUniformLocation(spotShaderProgram, "albedoSampler");
		//spotDepthTexture = glGetUniformLocation(spotShaderProgram, "depthSampler");
		//spotNormalTexture = glGetUniformLocation(spotShaderProgram, "normalSampler");
		spotAttenBegin = glGetUniformLocation(spotShaderProgram, "light_atten_begin");
		spotAttenEnd = glGetUniformLocation(spotShaderProgram, "light_atten_end");
		//spotDirection = glGetUniformLocation(spotShaderProgram, "light_direction");
		spotCosAngleAtten = glGetUniformLocation(spotShaderProgram, "cos_light_angle_atten");
		*/



		Tex0( omniNormalTexture, mGBuffer.mGLTexID2 );
		Tex1( omniDepthTexture, mGBuffer.mGLTexID3 );
		Tex2( omniAlbedoTexture, mGBuffer.mGLTexID );

		glUniform2f(spotDeprojection, 1.f/pCamera->mProjection.m16[0], 1.f/pCamera->mProjection.m16[5]);

		matrix_t omniSphereMatrix;
		omniSphereMatrix.identity();
		mesh_t *spotMeshPrefab = mesh_t::GetGeoSpherePrefab(3);
		//glEnableClientState(GL_VERTEX_ARRAY);
		//glVertexPointer(3, GL_FLOAT, 24, &cone->vertices[0].x);
		spotMeshPrefab->mVA->Bind();
		spotMeshPrefab->mIA->Bind();
		//const vec_t& viewerPos = pCamera->mViewInverse.position;
		//for (; iterspot != mSpots.end() ; ++iterspot)
		{
			//spotLight_t *pspot = (*iterspot);
			/*
			if (!pCamera->mFrustum.SphereInFrustum(pspot->mPosition))
			continue;

			vec_t camToLight = (viewerPos - pomni->mPosition);
			camToLight.w = 0.f;
			bool bInversed = (camToLight.lengthSq() <= (pomni->mPosition.w * pomni->mPosition.w));
			if (bInversed)
			{
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			}
			else
			{
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			}

			*/
			vec_t spotViewPos;

			spotViewPos.TransformPoint(vec(spotSource.x, spotSource.y, spotSource.z, 1.f), pCamera->mView );
			spotViewPos.z = -spotViewPos.z;
			spotViewPos.w = 1.f/(spotSource.w*spotSource.w);

			glUniform4fv( spotPosition, 1, &spotViewPos.x );
			//glUniform4fv( spotColor, 1, &pspot->mColor.x );
			glUniform4f( spotColor, 1.f, 0.f, 1.f, 1.f);//

			/*
			omniSphereMatrix.m16[10] = omniSphereMatrix.m16[5] = omniSphereMatrix.m16[0] = pomni->mPosition.w * 2.f;

			omniSphereMatrix.position = vec( pomni->mPosition.x, pomni->mPosition.y, pomni->mPosition.z, 1.f );

			matrix_t spotwvp = spotSphereMatrix * viewProj;
			glUniformMatrix4fv( omniWorldViewProj, 1, 0, omniwvp.m16 );
			*/
			matrix_t spotMatrix;
			spotMatrix.LookAt(spotSource, spotDest, vec(0.f, 1.f, 0.f));
			matrix_t spotwvp = spotMatrix * viewProj;
			glUniformMatrix4fv( spotWorldViewProj, 1, 0, spotwvp.m16 );
			glDrawElements(	GL_TRIANGLES, cone->triCount, GL_UNSIGNED_SHORT, 0);

		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

	}
#endif


	// Sky ------------------------------------------------------------------------------------------------------

	DrawSky( skyRotateMatrix, pCamera, (splitScreenMode!=0), bIsSpace );

	// Aurora Borealis ----------------------------------------------------------------------------

	if ( mAuroraBatch.mAV )
	{
		PROFILER_START(Aurora);
		glUseProgram(auroraProgram);

		glDepthMask(0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_CULL_FACE);
		
		static float auroraGlobalTime = 0.f;
		auroraGlobalTime += aTimeEllapsed;
		glUniform1f( auroraTime, auroraGlobalTime );
		glUniform3fv( auroraWorldEyePos, 1, &pCamera->mViewInverse.position.x );

		for (int i=0;i<mAuroraBatch.mAV;i++)
		{
			meshentry_t* pme = &mAuroraBatch.mList[i];
			matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
			glUniformMatrix4fv(auroraMVP, 1, 0, tmp.m16);

			drawMesh(pme->pMesh);
		}
		PROFILER_END(); // Aurora
	}

	// transparent ---------------------------------------------------------------------------------------------------------
	PROFILER_START(Transparent);
	glUseProgram(shadowedShaderProgram);

	glDepthMask(0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	glUniform4f(shadowedNearPlanes, PSSMDistance0, PSSMDistance1, PSSMDistance2, PSSMDistance3);
	glUniform4f(shadowedFarPlanes, PSSMDistance1, PSSMDistance2, PSSMDistance3, PSSMDistance4);
	glUniform4fv(shadowedSunDirection, 1, &GSkyScatParams->mSunDirection.x);
	glUniform4fv(shadowedSunColor, 1, &GSkyScatParams->mSunColor.x);
	glUniform3fv(shadowedCameraPosition, 1, &pCamera->mViewInverse.position.x );

	glUniform4f(shadowedClipPlane, 0.f, 0.f, 0.f, 0.f);

	if ( ShouldRenderShadows() && (!splitScreenMode) && (!ShadowsDesactivatedByCommandLine)  )
		Tex0( shadowedShadowMap, mShadowMap.mGLTexID );
	else
		Tex0( shadowedShadowMap, mShadowMapMini );

	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

	glUniform1iARB(shadowedCubeAmbient,1);
	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, scatDownCubeTexture[2].mGLTexID);

	for (int i=0;i<mTransparentBatch.mAV;i++)
	{
		meshentry_t* pme = &mTransparentBatch.mList[i];
		matrix_t tmp = pme->pMesh->mWorldMatrix * viewProj;
		glUniformMatrix4fv(shadowedModelViewProjectionMatrix, 1, 0, tmp.m16);
		glUniformMatrix4fv(shadowedWorldMatrix, 1, 0, pme->pMesh->mWorldMatrix.m16);

		matrix_t tmp2[4];

		for ( int smi = 0 ; smi < 4 ; smi++)
			tmp2[smi] = pme->pMesh->mWorldMatrix * pCamera->mPSSMShadowMatAtlas[smi];//lightView * mTexScale;

		glUniformMatrix4fv(shadowedShadowMatrix, 4, 0, (GLfloat*)tmp2);
		glUniformMatrix4fv(shadowedNormalMatrix, 1, 0, pme->pMesh->mWorldMatrix.m16);

		glUniform4fv(shadowedColor, 1, &pme->pMesh->color.x);
		glUniform4fv(shadowedShader, 1, &pme->pMesh->shader.x);

		drawMesh(pme->pMesh);
	}

	glDisable(GL_BLEND);
	PROFILER_END(); // Transparent


	// Explosions -----------------------------------------------------------------------------------------------------


	if ( !mExplosions.empty() )
	{
#if !IS_OS_MACOSX
		glDepthMask(0);
		glEnable(GL_CULL_FACE);
		mesh_t *exploMeshPrefab = GetGeoSmoothedSpherePrefab( 6 );

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		// fireball
		glUseProgram( explosionProgram );

		glActiveTextureARB(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);

		glActiveTextureARB(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);


		const int explosionCount = mExplosions.size();

		for (int i = 0; i < explosionCount; ++i)
		{
			const Renderer::explosion_t& explosion = mExplosions[i];

			glUniform1f( explosionAlpha, explosion.mAlpha );

			//glUniform1f(explosionTime, GetApplicationTime()*0.2f );
			glUniform1f(explosionWeight, 0.0002f );

			mesh_t *expMesh;
			switch ( explosion.mType )
			{
			case FIRE_BALL:
				glUniform1f(explosionTime, GetApplicationTime()*0.2f );
				glBindTexture( GL_TEXTURE_2D, GExplosionTexture );
				expMesh = exploMeshPrefab;
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				break;
			case FIRE_WALL:
				glUniform1f(explosionTime, GetApplicationTime()*2.f );
				glBindTexture( GL_TEXTURE_2D, GExplosionTexture );
				expMesh = exploMeshPrefab;
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				break;
			case PLASMA_WALL:
				glUniform1f(explosionTime, GetApplicationTime()*1.3f );
				glBindTexture( GL_TEXTURE_2D, GPlasmaTexture );
				expMesh = exploMeshPrefab;//GetWall();
				//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glBlendFunc( GL_ONE, GL_ONE );
				break;
			case ELECTRIC_WALL:
				glUniform1f(explosionTime, GetApplicationTime()*0.3f );
				glBindTexture( GL_TEXTURE_2D, GElectricWallTexture );
				expMesh = exploMeshPrefab;//GetWall();
				glBlendFunc( GL_ONE, GL_ONE );
				break;
			}
			TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );


			matrix_t exploModelViewMat = explosion.mFireballMat * pCamera->mView;

			glUniformMatrix4fv(explosionObjectMatrix, 1, 0, explosion.mFireballMat.m16 );
			glUniformMatrix4fv(explosionModelViewMatrix, 1, 0, exploModelViewMat.m16 );
			glUniformMatrix4fv(explosionProjectionMatrix, 1, 0, pCamera->mProjection.m16 );

			drawMesh( expMesh );
			//drawMesh(  );
		}
		// shockwave
		glUseProgram( shockwaveProgram );
		glDisable(GL_CULL_FACE);

		ASSERT_GAME( explosionCount == static_cast<int>( mExplosions.size() ) );

		for (int i = 0; i < explosionCount; ++i)
		{
			const Renderer::explosion_t& explosion = mExplosions[i];

			if ( explosion.mType != FIRE_BALL )
				continue;

			glUniform1f(shockwaveAlpha, 0.5f * explosion.mAlpha );
			matrix_t mvp = explosion.mWaveMat * viewProj;

			glUniformMatrix4fv(shockwaveMVP, 1, 0, mvp.m16 );

			glBegin(GL_QUADS);
			glVertex3f(-1.f, 0.f, -1.f);
			glVertex3f(-1.f, 0.f, 1.f);
			glVertex3f(-1.f + 2.f, 0.f, 1.f);
			glVertex3f(-1.f +  2.f, 0.f, -1.f);
			glEnd();
		}
		glDisable( GL_BLEND );
#endif  // !IS_OS_MACOSX

		mExplosions.clear();
		glDepthMask(1);
	}

	// Explosions -----------------------------------------------------------------------------------------------------


	if ( !portals.empty() )
	{
		glDepthMask(0);
		glEnable(GL_CULL_FACE);

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		// fireball
		glUseProgram( portalProgram );

		glActiveTextureARB(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);

		glActiveTextureARB(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glBindTexture( GL_TEXTURE_2D, textures[TEXTURE_PORTAL] );
		TexParam( GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT );

		//const int portalCount = portals.size();
		glUniformMatrix4fv(portalMVP, 1, 0, camera.mViewProj.m16 );
		static float portalTimeValue = 0.f;
		portalTimeValue += aTimeEllapsed;
		glUniform1f( portalTime, portalTimeValue*3.f );
		//for (int i = 0; i < portalCount; ++i)
		std::list<portal_t*>::const_iterator itp = portals.begin();
		for (; itp != portals.end(); ++itp)
		{
			const Renderer::portal_t& portal = *(*itp);//portals[i];

			glUniform1f( portalWidth, portal.mWidth );
			glUniform1f( portalOffset, portal.mOffset );
			glUniform4fv( portalColor, 1, &portal.mMesh->color.x );

			drawMesh( portal.mMesh );
		}
		glDisable( GL_BLEND );
	}

	int beaconPlayerId = splitScreenMode?(splitScreenMode-1):0;

	// Finish Line -----------------------------------------------------------------------------------------------------
	if ( GGame && GGame->GetPlayerShip(beaconPlayerId) && ( pCamera->GetMode() == CM_BEHINDSHIP ) )
	{
		static GLuint finishLineTexture = 0;

		static bool previouslyVisible = false;

		unsigned int nbBlocks = track.getAIPointCount();
		int blockRunned = GGame->GetPlayerShip(beaconPlayerId)->GetPhysics()->GetCurrentRoadBlockIndexRaceSpace();
		float nbLapsDone = static_cast<float>(blockRunned)/ static_cast<float>(nbBlocks);//  GGame->GetPlayerShip()->GetCompletedTrackLaps();
		float nbLapsDoneInTrack = static_cast<float>( GGame->GetPlayerShip(beaconPlayerId)->GetPhysics()->GetCurrentRoadBlockIndex())/ static_cast<float>(nbBlocks);//  ;
		nbLapsDone-=1.f;
		nbLapsDone = zmax(nbLapsDone, 0.f);
		float nbLapsTodo = static_cast<float>(GGame->GetGameProps().mNumberOfLapsToDo);

		bool bNowVisible = ( fmodf(nbLapsDoneInTrack,1.f) > 0.5f);

		/*
			char tmps[512];
			sprintf(tmps, "laps done : %f", nbLapsDone);
			ggui.putText(0,0,tmps);
			*/




		if ( GGame->GetPlayerShip(beaconPlayerId)->GetPhysics()->HalfTrackDone() && 
			bNowVisible &&
			(!previouslyVisible) )

		{
			char newFinishLineText[32];


			if (nbLapsDone>(nbLapsTodo-2.5f))
			{
				// final lap
				if (nbLapsDone>(nbLapsTodo-1.5f))
				{
					strcpy( newFinishLineText, "FINISH LINE//" );
				}
				else
					strcpy( newFinishLineText, "FINAL Lap//" );
			}
			else 
			{
				sprintf( newFinishLineText, "Lap %d Starts here//", static_cast<int>(nbLapsDone+0.5f)+2 );
			}

			ASSERT_GAME( (strlen(newFinishLineText)<21) );

			static const int imgw (20*8 + 2);
			static const int imgh(10);
			static u32 img[imgw * imgh ];
			memset( img, 0, imgw * imgh*sizeof(u32) );

			unsigned int textLen = strlen( newFinishLineText );
			unsigned texWidth = textLen * 8 +2;

			RenderText( newFinishLineText, textLen, texWidth, img );
			BlackBorder( img, texWidth, imgh, 0, texWidth, 0, imgh );

			if (!finishLineTexture)
				glGenTextures( 1, &finishLineTexture );

			glBindTexture( GL_TEXTURE_2D, finishLineTexture );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texWidth, imgh, 0, GL_RGBA, GL_UNSIGNED_BYTE, img );

		}
		previouslyVisible = bNowVisible;
		glUseProgram(finishLineProgram);

		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);        

		if ( bNowVisible )
		{
			Tex1( finishLineTexScreen, finishLineTexture );
			TexParam( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );

			const AIPoint_t& aipt = track.getAIPoint(0);
			matrix_t lineScale, lineLocal;

			lineScale.scale( aipt.mAIwidth[0]+1.f );

			aipt.BuildMatrix( lineLocal, 0 );
			matrix_t lineMat = lineScale * lineLocal;

			for ( int i = 0 ; i < 3 ; i ++ )
			{
				static const float timeFactors[3]={ 0.1f, -0.13f, 0.16f };
				// u decal
				float uDecal = GetApplicationTime() * timeFactors[i];

				// u/v ratio
				//FIXME: 'uvRatio' is never used, should it be removed
				//float uvRatio = aipt.mAIwidth[0]; // <- depends on text length, not only 

				float textHeight = 0.05f;

				lineMat.position += lineMat.up * textHeight * 2.f;
				lineMat.position.w = 1.f;

				matrix_t tmp = lineMat * viewProj;
				glUniformMatrix4fv(finishLineMVP, 1, 0, tmp.m16);


				float uScale = 2.f;

				glBegin(GL_QUADS);
				glTexCoord2f(uDecal, 1.f);
				glVertex3f(1.f, -textHeight, 0.f );//0.9999f, 1.f);
				glTexCoord2f(uDecal, 0.f);
				glVertex3f(1.f, textHeight, 0.f );//0.9999f, 1.f);
				glTexCoord2f(uDecal+uScale, 0.f);
				glVertex3f(-1.f, textHeight, 0.f );//0.9999f, 1.f);
				glTexCoord2f(uDecal+uScale, 1.f);
				glVertex3f(-1.f, -textHeight, 0.f );//0.9999f, 1.f);
				glEnd();



			}
			glDisable(GL_BLEND);
		}

	}
	// Beacon -----------------------------------------------------------------------------------------------------
	
	if ( GGame && GGame->GetPlayerShip(beaconPlayerId) && ( pCamera->GetMode() == CM_BEHINDSHIP ) )
	{
#if !IS_OS_MACOSX

		GameShip*pShip = GGame->GetPlayerShip(beaconPlayerId);

		glUseProgram(beaconProgram);

		glDepthMask(0);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc ( GL_GREATER, 0.5 );
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);

		glUniform1f(beaconParam1, Clamp(pShip->GetSmoothedSpeed() * 0.005f, 0.f, 1.f) );

		glUniform1f(beaconParam2, pShip->GetHealthForRendering() );


		vec_t beaconIcons[BT_BonusCount]={ 
			vec(2,3),//        { 3, 2, false, BT_Trackspeed,
			vec(2,3),//        { 3, 2, false, BT_Color1,
			vec(2,3),//        { 3, 2, false, BT_Color2,
			vec(3,3),//        { 3, 2, false, BT_Missile,
			vec(1,1),//        { 3, 2, false, BT_Mines,
			vec(2,0),//        { 3, 2, false, BT_Autopilot,
			vec(0,1),//        { 3, 2, false, BT_Shield,
			vec(3,0),//        { 3, 2, false, BT_MachineGun,
			vec(2,1),//        { 3, 2, false, BT_SpeedBooster,
			vec(0,0),//        { 3, 2, false, BT_3Missiles,
			vec(1,0),//        { 3, 2, false, BT_HomingMissile,
			vec(0,2),//        { 3, 2, false, BT_Portal,
			//vec(1,3),//        { 3, 2, false, BT_AntiPortal,
			vec(0,3),//        { 3, 2, false, BT_Bombing,
			vec(1,2),//        { 3, 2, false, BT_MagneticWave,
			vec(2,2),//        { 3, 2, false, BT_PlasmaWave,
			vec(3,2),//        { 3, 2, false, BT_FireWave,
			vec(2,3)//        { 3, 2, false, BT_None,
		};

		const vec_t& beaconIconUV = beaconIcons[(pShip->mProps.mBonusType==GameShip::shipProps_t::Invalid_Bonus_type)?0:pShip->mProps.mBonusType];
		glUniform2f(beaconBonusXY, beaconIconUV.x*0.25f, beaconIconUV.y*0.25f );




		matrix_t beaconLocal;
		//matrix_t beaconProj = pCamera->mProjection;
		matrix_t beaconFullMat;
		if ( !Solo::playerProfil.cameraMode[beaconPlayerId] )
		{
			if (splitScreenMode)
				beaconLocal.scale( 0.025f*1.66f, 0.025f, -0.025f );
			else
				beaconLocal.scale( 0.025f, 0.025f, -0.025f );

			beaconLocal.position = vec( 0.f, -1.3f, 2.5f, 1.f );
			glDisable( GL_DEPTH_TEST);

			matrix_t beaconProj;
			beaconProj.glhPerspectivef2( 90.f, WIDTH/HEIGHT, 0.1f, 1000.0f );

			beaconFullMat = beaconLocal * pShip->GetPhysics()->GetTransform() * pCamera->mView * beaconProj;
			glUniformMatrix4fv(beaconMVP, 1, 0, (GLfloat*)beaconFullMat);

		}
		else
		{
			beaconLocal.scale( -0.014f, 0.014f, -0.014f );
			beaconLocal.position = vec( 0.f, -0.266f, -1.0f, 1.f );
			//beaconLocal.position = vec( 0.f, 0.3f, -2.0f, 1.f );

			// compile everything
			beaconFullMat = beaconLocal * pShip->GetMatrixForMissilesAndGun() * pCamera->mView * pCamera->mProjection;
			glUniformMatrix4fv(beaconMVP, 1, 0, (GLfloat*)beaconFullMat);

		}       
		/*
		matrix_t viewProjBeacon = pCamera->mView * beaconProj ;
		const matrix_t & shptrMat = pShip->GetPhysics()->GetTransform();
		// from ground to smoother ground with position
		matrix_t newGrdMatrix = pShip->GetPhysics()->GetGroundOrientationSmoothed();

		float distanceOverGround = (newGrdMatrix.position-pShip->GetPhysics()->GetGroundMatrix().position).dot(newGrdMatrix.up);
		newGrdMatrix.position = shptrMat.position - shptrMat.up * distanceOverGround;
		newGrdMatrix.position.w = 1.f;

		// get projected angle and compute a local Y rotation

		float ngDotDir = shptrMat.dir.dot( newGrdMatrix.dir );
		float ngDotRight = shptrMat.dir.dot( newGrdMatrix.right );

		float Yng = -acosf( ngDotDir );
		if ( ngDotRight>0.f) Yng = -Yng;

		matrix_t tmpbeaconLocalY;
		tmpbeaconLocalY.rotationY( Yng );

		static matrix_t beaconLocalY = matrix_t::Identity;
		beaconLocalY.lerp( beaconLocalY, tmpbeaconLocalY, 0.5f );

		*/


		if (GBDrawBeacon )
			drawMesh( getSprite(9) );

		// text
		static char _localHUDText[2][32] = {0,0};
		char *localHUDText = _localHUDText[beaconPlayerId];
		char newHUDText[32];

		static char _localHUDText2[2][32] = {0,0};
		char *localHUDText2 = _localHUDText2[beaconPlayerId];
		char newHUDText2[32];

		static GLuint hudInfoText[2] = {0,0};
		if (!hudInfoText[0])
			glGenTextures( 2, hudInfoText );

		int currentLap = (int)(floorf(pShip->mProps.mNumberLapsDone));

		bool mbBeaconUseTwoLines = ( ( GGame->GetType() == 3 ) || ( GGame->GetType() == 7) );

		sprintf(newHUDText, "L%02d/%02d  P%d/%d", currentLap+1, GGame->GetGameProps().mNumberOfLapsToDo,
			GGame->GetPositionForShip(GGame->GetPlayerShip(beaconPlayerId)), GGame->GetStillAliveShipCount() );

		int szLen = strlen(newHUDText);

		if (!mbBeaconUseTwoLines)
		{
			if ( memcmp( localHUDText, newHUDText, szLen ) )
			{
				memcpy( localHUDText, newHUDText, szLen );

				static const int imgw = (6*8 + 2);
				static const int imgh = (16+2+5);

				static u32 img[imgw * imgh]; // L00/99  P8/8
				memset( img, 0, imgw * imgh*sizeof(u32) );


				char tmps[512];
				sprintf(tmps, "L%02d/%02d", currentLap+1, GGame->GetGameProps().mNumberOfLapsToDo );
				//RenderText( localHUDText, szLen, imgw, img );
				RenderText( tmps, strlen(tmps), imgw, img+8*imgw );
				sprintf(tmps, "P%d/%d", GGame->GetPositionForShip(GGame->GetPlayerShip(beaconPlayerId)), GGame->GetStillAliveShipCount() );
				RenderText( tmps, strlen(tmps), imgw, img  );

				BlackBorder( img, imgw, imgh, 0, imgw, 0, imgh );


				glBindTexture( GL_TEXTURE_2D, hudInfoText[beaconPlayerId] );
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);


				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgw, imgh, 0, GL_RGBA, GL_UNSIGNED_BYTE, img );
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
		else
		{
			// 2 lines with points
			unsigned int pointsShipRank = GGame->GetRankBasedOnPoints( pShip );
			sprintf(newHUDText2, "%6.2f P %d%s", pShip->GetPointsForRendering(), pointsShipRank, Game::RankSuffixText( pointsShipRank ) );
			int szLen2 = strlen(newHUDText2);

			if ( memcmp( localHUDText, newHUDText, szLen ) || memcmp( localHUDText2, newHUDText2, szLen2 ) )
			{
				memcpy( localHUDText, newHUDText, szLen );
				memcpy( localHUDText2, newHUDText2, szLen2 );

				static const int imgw = (12*8 + 2);
				static const int imgh = (16+2);

				static u32 img[imgw * (imgh+5)]; // L00/99  P8/8
				memset( img, 0, imgw * imgh*sizeof(u32) );


				RenderText( localHUDText, szLen, imgw, img+8*imgw );
				RenderText( localHUDText2, szLen2, imgw, img  );

				BlackBorder( img, imgw, imgh, 0, imgw, 0, imgh );


				glBindTexture( GL_TEXTURE_2D, hudInfoText[beaconPlayerId] );
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);


				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgw, imgh+5, 0, GL_RGBA, GL_UNSIGNED_BYTE, img );
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		glUseProgram(beaconTextProgram);

		Tex0( beaconTextTex, hudInfoText[beaconPlayerId] );
		TexParam( GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

		Tex1( beaconTextWeaponTex, textures[TEXTURE_BEACONWEAPON] );
		TexParam( GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

		
		glUniform2f(beaconTextBonusXY, beaconIconUV.x*0.25f, beaconIconUV.y*0.25f );
		matrix_t abitabove;
		
		if ( !Solo::playerProfil.cameraMode[beaconPlayerId] )
		{
			if (splitScreenMode)
				abitabove.scale( 1.66f, 2.f, 1.f );
			else
				abitabove.scale( 1.f, 2.f, 1.f );
			abitabove.position = vec( 0.f, 5.05f, 0.f, 1.f );
		}
		else
		{
			abitabove.scale( 1.85f, 2.4f, 1.f );
			abitabove.position = vec( 0.f, 30.87f, 0.f, 1.f );
		}

		glUniformMatrix4fv(beaconTextMVP, 1, 0, (GLfloat*)(abitabove*beaconFullMat).m16);
		if (GBDrawBeacon )
			drawMesh( getSprite(8) );


		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glEnable( GL_DEPTH_TEST);
#endif  // !IS_OS_MACOSX
	}

	// FX ---------------------------------------------------------------------------------------------------------
	if (Ribbon::HasRibbonsToDraw())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glUseProgram(trailShaderProgram);
		glUniform4fv(trailEye, 1, &pCamera->mViewInverse.position.x);

		glUniform1f(trailFogDensity, 0.001f);//GFogDensity);
		glUniform3f(trailFogColor, 1.f, 1.f, 1.f);//GFogColor.x, GFogColor.y, GFogColor.z);

		glDepthMask(0);

		TexParam( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP, GL_REPEAT );


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);

		Ribbon*const* iter = Ribbon::GetRibbons().GetFirst();
		Ribbon*const* iterEnd = Ribbon::GetRibbons().GetEndUsed();
		for (; iter != iterEnd ; ++iter )            
		{
			glActiveTextureARB(GL_TEXTURE0);
			(*iter)->BindTexture();
			glEnable(GL_TEXTURE_2D);


			glUniform1f(trailVDecal, (*iter)->GetVShift());
			glUniform1f(trailWidth, -(*iter)->GetWidth());
			glUniform4fv(trailColor, 1, &(*iter)->GetColor().x);

			matrix_t trailWorldViewProj;
			trailWorldViewProj = (*iter)->GetWorldMatrix() * viewProj;

			glUniformMatrix4fv(trailModelViewProjectionMatrix, 1, 0, viewProj.m16);
			glUniformMatrix4fv(trailWorldMatrix, 1, 0, (*iter)->GetWorldMatrix().m16);

			(*iter)->Draw();
		}
	}


	// post process part

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(0);
	glDisable(GL_BLEND);

#if 0
	// SDSM test. total crap!

	/*
	GLuint pbo;
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, mGBuffer.fbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, 1280 * 720 * sizeof(GLfloat), NULL, GL_STREAM_READ_ARB);
	*/
	/*
	float *tmpTexture = new float [1280 * 720 * 4 ];
	glBindTexture( GL_TEXTURE_2D, mGBuffer.mGLTexID3 );
	glGetTexImage(	GL_TEXTURE_2D,
	0,
	GL_RGB,
	GL_FLOAT,
	tmpTexture);

	float *imgDatas = tmpTexture;
	float amin(10000.f), amax(0.f);
	for (int i=0;i<1280*720*3;i+=3)
	{
	float v = *imgDatas;
	if (v>0.f)
	amin= zmin(amin, v);
	amax = zmax(amax, v);
	imgDatas +=3;
	}
	char tmps[512]; 
	sprintf(tmps," FullZ %5.4f, %5.4f", amin, amax);
	ggui.putText(3, 4, tmps );
	delete [] tmpTexture;
	*/
	//#if !IS_OS_MACOSX
	static bool bFloatTexSet = false;
	static RenderTargetOGL floatTex;
	if (! bFloatTexSet )
	{
		bFloatTexSet = true;
		//floatTex.InitFloatTarget( 80, 45 );
		floatTex.InitFloatTarget( 80, 45 );
	}

	floatTex.BindAsTarget();
	glUseProgram(sdsmDepthRedux);
	TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP );

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_2D, mGBuffer.mGLTexID3 );

	DrawFullScreenQuad();
	//mBloomRT.BindAsTarget();


	float *tmpTexture2 = new float [ (80 * 45 * 4) ];
	glBindTexture( GL_TEXTURE_2D, floatTex.mGLTexID );
	glGetTexImage(	GL_TEXTURE_2D,
		0,
		GL_RGB,
		GL_FLOAT,
		tmpTexture2);

	float *imgDatas2 = tmpTexture2;
	float amin2(10000.f), amax2(0.f);
	for (int i=0;i<80*45*3;i+=3)
	{
		float v1 = *imgDatas2++;
		float v2 = *imgDatas2++;
		//        if (v1>0.f)
		amin2= zmin(amin2, v1);
		//        if (v2<1000.f)
		amax2 = zmax(amax2, v2);
		imgDatas2 ++;
	}
	// compute splits

	float Zmin = amin2;
	float Zmax = amax2;

	float depth = tmpTexture2[80*22 + 40];
	//    printf("Z : %5.4f %5.4f - %5.4f\n", Zmin, Zmax, depth );

	static mesh_t *depthMarker = NULL;
	if (!depthMarker)
	{
		depthMarker = createBoxMarker( vec(0.f), vec(1.f), 10.f );
		depthMarker->computeBSphere();
	}
	depthMarker->visible = true;
	depthMarker->mWorldMatrix.translation( pCamera->mViewInverse.position - pCamera->mViewInverse.dir* depth );
	depthMarker->updateWorldBSphere();



#if 0
	// Compute split distances
	Zmax = zmax(Zmax, 350.f);
	float range = Zmax - Zmin;
	float ratio = Zmax / Zmin;
	float mPSSMDistances[5];
	//mPSSMDistances.reserve( 5 );
	static const float lambda = 0.8f;
	//mPSSMDistances.resize(0);
	for (int i = 0; i < 4; ++i) 
	{
		float p = i / (4.f);
		float log = Zmin * powf(ratio, p);
		float uniform = Zmin + range * p;
		float d = lambda * (log - uniform) + uniform;
		//mPSSMDistances.push_back(d);
		mPSSMDistances[i] = d;
	}
	mPSSMDistances[4] = Zmax;//mPSSMDistances.push_back(amax2);


	RenderSettings.PSSMDistance0 = mPSSMDistances[0];
	RenderSettings.PSSMDistance1 = mPSSMDistances[1];
	RenderSettings.PSSMDistance2 = mPSSMDistances[2];
	RenderSettings.PSSMDistance3 = mPSSMDistances[3];
	RenderSettings.PSSMDistance4 = mPSSMDistances[4];
#endif
#if 0

	// light matrix
	matrix_t lightMat;
	lightMat.lookAtLH(GSkyScatParams->mSunDirection, vec(0.f), vec(0.f, 1.f, 0.f));

	// frustra part 1 --------
	//sdsmFrustraProgram = loadShaderProgram(sdsmFrustraShaderVertex, sdsmFrustraShaderFragment, "sdsmFrustra");
	glUseProgram(sdsmFrustraProgram);
	glUniform2f( sdsmFrustraProgram, pCamera->mProjection.m16[4], pCamera->mProjection.m16[5] );//sdsmFrustraDProj = glGetUniformLocation(sdsmFrustraProgram, "dproj");

	glUniformMatrix4fv( sdsmFrustraLightMat, 1, 0, lightMat.m16 );
	for (int j = 0;j<4;j++)
	{
		glUniform2f( sdsmFrustraShadowZPlanes, mPSSMDistances[j], mPSSMDistances[j+1] );

		/*, mPSSMDistances[2], mPSSMDistances[3] );
		glUniform4f( sdsmFrustraShadowFarPlanes, mPSSMDistances[1], mPSSMDistances[2], mPSSMDistances[3], mPSSMDistances[4] );
		*/
		TexParam( GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

		//glBindTexture( GL_TEXTURE_2D, mGBuffer.mGLTexID3 );

		DrawFullScreenQuad();



		glBindTexture( GL_TEXTURE_2D, floatTex.mGLTexID );
		glGetTexImage(	GL_TEXTURE_2D,
			0,
			GL_RGBA,
			GL_FLOAT,
			tmpTexture2);




		//float *imgDatas2 = tmpTexture2;
		float Xmin = 10000.f;
		float Xmax = -10000.f;
		float Ymin = 10000.f;
		float Ymax = -10000.f;


		float *imgDatas3 = tmpTexture2;
		for (int i=0;i<80*45;i++)
		{
			float v1 = *imgDatas3++;
			float v2 = *imgDatas3++;
			float v3 = *imgDatas3++;
			float v4 = *imgDatas3++;
			/*
			if (v1>0.f)
			Xmin = zmin(Xmin, v1);
			if (v3>0.f)
			Ymin = zmin(Ymin, v3);
			*/
			Xmin = zmin(Xmin, v1);
			Xmax = zmax(Xmax, v2);

			Ymin = zmin(Ymin, v3);
			Ymax = zmax(Ymax, v4);
		}

		pCamera->ComputeShadowMatrix( j, lightMat, vec(Xmin, Ymin, -300.f), vec( Xmax, Ymax, 300.f ) );
	}
#endif
	/*
	char tmps2[512];
	sprintf(tmps2,"ReduxZ %5.4f, %5.4f// %5.2f %5.2f %5.2f %5.2f                          ", amin2, amax2, Xmin, Xmax, Ymin, Ymax );
	ggui.putText(3, 5, tmps2 );
	*/
	delete [] tmpTexture2;
#endif

	//#endif // SDSM

#if 0

	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, mGBuffer.fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT2_EXT);
	//glReadBuffer(GL_DEPTH_ATTACHMENT_EXT);

	glReadPixels(0, 0, 1280, 720, GL_R32F, GL_FLOAT, 0);

	//GLubyte* src = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
	//OR



	float *imgDatas = (float*) glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
	if(imgDatas)
	{
		float amin(10000.f), amax(0.f);
		for (int i=0;i<1280*720;i++)
		{
			float v = *imgDatas++;
			amin= zmin(amin, v);
			amax = zmax(amax, v);

		}
		//Process src OR cvim->imageData
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);     // release pointer to the mapped buffer
	}
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

#endif


	// -- bloom ---------------------------------------------------------------------------------------------------------

	mBloomRT.BindAsTarget();
	glUseProgram(bloomShaderProgram);

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mFrameBufferScratch[0].mGLTexID);


	DrawFullScreenQuad();

	const unsigned int width = static_cast<unsigned int>(WIDTH * (splitScreenMode?0.5f:1.f));
	const unsigned int height = static_cast<unsigned int>(HEIGHT);

	Blur(false, mBloomRT, mRTBlur, width, height );
	Blur(true, mRTBlur, mBloomRT, width, height );


	// Sketch + bloom + FOG ------------------------------------------------------------------------------------------

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, NULL);

	bool hasDOF = (DOFBlur>FLOAT_EPSILON);
#if IS_OS_MACOSX
	hasDOF = false;
#endif

	const config_t& config = GetEngineConfig();
	const bool hasAA = (config.AAactive != 0)&&(!AADesactivatedByCommandLine);
	int nbPPToDo = (hasDOF?1:0) + (hasAA?1:0);
	int currentFBScratchIndex = 0;

	if (nbPPToDo>0)
		mFrameBufferScratch[(currentFBScratchIndex+1)&1].BindAsTarget();
	else
		mFrameBuffer[mCurrentFrameBuffer].BindAsTarget();



	//
	glUseProgram(sketchShaderProgram);

	Tex0( sketchZBuffer, mGBuffer.mGLTexID3 );
	Tex1( sketchNormalBuffer, mGBuffer.mGLTexID2 );
	Tex2( sketchFrameBuffer, mFrameBufferScratch[(currentFBScratchIndex)&1].mGLTexID );
	Tex3( sketchBloom, mBloomRT.mGLTexID);

	glUniform2f(sketchPixelSize, (1.f/(float)mGBuffer.mWidth), (1.f/(float)mGBuffer.mHeight));
	glUniform1f(sketchFogDensity, GSkyScatParams->mFogDensity*0.001f);
	glUniform4fv(sketchFogColor, 1, &GSkyScatParams->mFogColor.x);

	DrawFullScreenQuad();
	currentFBScratchIndex ++;




	// DOF ------------------------------------------------------------------------------------------------
	if (hasDOF)
	{
		--nbPPToDo;
		if (nbPPToDo>0)
			mFrameBufferScratch[(currentFBScratchIndex+1)&1].BindAsTarget();
		else
			mFrameBuffer[mCurrentFrameBuffer].BindAsTarget();

		glUseProgram(DOFShaderProgram);

		Tex0( DOFDepthTexture, mGBuffer.mGLTexID3);
		Tex1( DOFFramebuffer, mFrameBufferScratch[(currentFBScratchIndex)&1].mGLTexID );

		glUniform1f(DOFFocus, DOFFocusDistance);
		glUniform1f(DOFBias, DOFBlur);
		glUniform1f(DOFAspectRatio, WIDTH/HEIGHT);

		DrawFullScreenQuad();
		currentFBScratchIndex++;
	}

	// AA -------------------------------------------------------------------------------------------------
	if (hasAA)
	{
		--nbPPToDo;
		if (nbPPToDo>0)
			mFrameBufferScratch[(currentFBScratchIndex+1)&1].BindAsTarget();
		else
			mFrameBuffer[mCurrentFrameBuffer].BindAsTarget();

		glUseProgram( fxaa3ShaderProgram );

		Tex0( fxaaTexture, mFrameBufferScratch[(currentFBScratchIndex)&1].mGLTexID );
		TexParam( GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP );

		glUniform4f( fxaaRcpFrameOpt, (2.f/WIDTH), (2.f/HEIGHT), (0.5f/WIDTH), (0.5f/HEIGHT) );
		glUniform2f( fxaaRcpFrame, (1.f/WIDTH), (1.f/HEIGHT) );
		glUniform2f( fxaaScreenSize, WIDTH, HEIGHT );

		DrawFullScreenQuad();
		currentFBScratchIndex ++;
	}

	// Lens flare --------------------------------------------------------------------------------------
#if 0
	if (mNbFlares)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(blit2DProgram);

		glActiveTextureARB(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, GLensTexture);
		TexParam( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );


		glEnable(GL_BLEND);
		glBlendFunc( GL_ONE, GL_ONE );
		glDisable( GL_CULL_FACE);
		glDepthMask(0);


		for (int i=0;i<mNbFlares;i++)
		{
			if (mFlares[i].mClipSpacePosition.z < 0.f)
				continue;

			const lensflare_t & lf = mFlares[i];

			float lensposx = lf.mClipSpacePosition.x;
			float lensposy = lf.mClipSpacePosition.y;

			// lens
			glUniform4f( blit2DProgramColor, lf.mFlareStrength*0.35f, lf.mFlareStrength*0.35f, lf.mFlareStrength*0.65f, 1.f );

			glBegin( GL_QUADS );
			glVertex4f( -2.f+lensposx, -0.1f+lensposy, 0.33f, 0.f);
			glVertex4f( 2.f+lensposx, -0.1f+lensposy, 1.f, 0.f);
			glVertex4f( 2.f+lensposx, 0.0f+lensposy, 1.f, 1.f);
			glVertex4f( -2.f+lensposx, 0.0f+lensposy, 0.33f, 1.f);

			glVertex4f( -2.f+lensposx, -0.f+lensposy, 0.33f, 1.f);
			glVertex4f( 2.f+lensposx, -0.f+lensposy, 1.f, 1.f);
			glVertex4f( 2.f+lensposx, 0.1f+lensposy, 1.f, 0.f);
			glVertex4f( -2.f+lensposx, 0.1f+lensposy, 0.33f, 0.f);

			glEnd();

			// flare
			glUniform4f( blit2DProgramColor, lf.mLensFlareStrength, lf.mLensFlareStrength, lf.mLensFlareStrength, 1.f );

			glBegin(GL_QUADS);
			glVertex4f( -lf.mFlareWidth+lensposx, -lf.mFlareWidth*pCamera->mRatio+lensposy, 0.f, 0.f);
			glVertex4f( lf.mFlareWidth+lensposx, -lf.mFlareWidth*pCamera->mRatio+lensposy, 0.33f, 0.f);
			glVertex4f( lf.mFlareWidth+lensposx, 0.f*pCamera->mRatio+lensposy, 0.33f, 1.f);
			glVertex4f( -lf.mFlareWidth+lensposx, 0.f*pCamera->mRatio+lensposy, 0.f, 1.f);

			glVertex4f( -lf.mFlareWidth+lensposx, 0.f*pCamera->mRatio+lensposy, 0.f, 1.f);
			glVertex4f( lf.mFlareWidth+lensposx, 0.f*pCamera->mRatio+lensposy, 0.33f, 1.f);
			glVertex4f( lf.mFlareWidth+lensposx, lf.mFlareWidth*pCamera->mRatio+lensposy, 0.33f, 0.f);
			glVertex4f( -lf.mFlareWidth+lensposx, lf.mFlareWidth*pCamera->mRatio+lensposy, 0.f, 0.f);
			glEnd();

		}
		glDisable(GL_BLEND);
		mNbFlares = 0;

	}
#endif

	// Capture------------------------------------------------------------------------------------------
	if (iMakeScreenShot && (!mbCaptureGUI))
	{
		PROFILER_START(Capture);
		SaveCapture();
		PROFILER_END(); // Capture
	}



	// dof cursor
	DrawDofCursor();

	// Target
	DrawMovingTarget( viewProj, splitScreenMode );

	// billboards
	for (unsigned int i =0;i<mBillboards.size();i++)
	{
		DrawBillboard(viewProj, mBillboards[i].mTexId, mBillboards[i].mWorldPos, mBillboards[i].mSize, splitScreenMode );
	}
	mBillboards.clear();

	// BadTV/copy ----------------------------------------------------------------------------------------
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	switch (splitScreenMode)
	{
		case 0:
			glViewport(0, ((int)SCREENHEIGHT-(int)HEIGHT)>>1, (int)WIDTH, (int)HEIGHT);
			break;
		case 1:
			glViewport(0, ((int)SCREENHEIGHT-(int)HEIGHT)>>1, (int)WIDTH/2, (int)HEIGHT);
			break;
		case 2:
			glViewport((int)WIDTH/2, ((int)SCREENHEIGHT-(int)HEIGHT)>>1, (int)WIDTH/2, (int)HEIGHT);
			break;
	}
	

	glActiveTextureARB( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, mFrameBuffer[mCurrentFrameBuffer].mGLTexID );
	TexParam( GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT );

	glUseProgram( blitProgram );
	glUniform4f( blitProgramColor, 1.f, 1.f, 1.f, 1.f );
	DrawFullScreenQuad();

	glActiveTextureARB(GL_TEXTURE0);
	glUseProgram(0);

	GBlackVelvetTime[0] -= aTimeEllapsed * 1.0f;
	GBlackVelvetTime[1] -= aTimeEllapsed * 1.0f;

	glViewport(0, ((int)SCREENHEIGHT-(int)HEIGHT)>>1, (int)WIDTH, (int)HEIGHT);
	//  ---------------------------------------------------------------------------------------------------------

	PROFILER_END(); // Render
	//DebugDraw();
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::renderVelvet( bool hasSplitScreen )
{
	int cnt = hasSplitScreen?2:1;
	for (int i=0;i<cnt;i++)
		{
		if (GBlackVelvetTime[i] > 0.f)
		{
			glUseProgram( velvetShaderProgram );
			glUniform1f( velvetAlpha, GBlackVelvetTime[i] );
			glUniform3f( velvetColor, GVelvetColor[i].x, GVelvetColor[i].y, GVelvetColor[i].z );
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			if (hasSplitScreen)
				DrawHalfScreenQuad( i );
			else
				DrawFullScreenQuad( );

			glDisable( GL_BLEND );
			glUseProgram(0);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
std::string Renderer::GetScreenCaptureName()
{
	char tmps[512];
	sprintf(tmps,"%s/therush_Capture_%d.png", GetPictureDirectoy().c_str(), iMakeScreenShot);
	return tmps;
}

void Renderer::DoSaveScreenShot( const char *szName)
{
	extern float WIDTH, HEIGHT;
	int Width = (int)WIDTH;
	int Height = (int)HEIGHT;

	int ind = 0;
	unsigned char *imgBits = new unsigned char [ Width * Height * 3 ];
	int i=Height-1;
	for (;i>=0;i--)
	{
		glReadPixels(0, (int)((SCREENHEIGHT-HEIGHT)*0.5f) + i, Width, 1, GL_RGB, GL_UNSIGNED_BYTE, &imgBits[ind]);
		ind += Width*3;
	}


	stbi_write_png( szName, Width, Height, 3, imgBits, Width*3 );
	delete [] imgBits;
}

void Renderer::SaveCapture()
{
	// save png ---------------------------------------------------------------------------------------------------------
	if (iMakeScreenShot)
	{
		DoSaveScreenShot( GetScreenCaptureName().c_str() );
		iMakeScreenShot = 0;
	}
}

void Renderer::endOfRendering( float aTimeEllapsed, Camera *pCamera, bool hasSplitScreen )
{
	SaveCapture();
	Renderer::renderVelvet( hasSplitScreen );

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	extern GLuint mGuiTexture;

	ggui.draw( aTimeEllapsed );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	textRenderingShader.Bind();
	glBindTexture( GL_TEXTURE_2D, mGuiTexture );
	DrawFullScreenQuad();

	// over gui mesh
	if (mOverGuiMesh)
	{
		glViewport( int((mOverGuiMeshRectangle.x/64.f)*WIDTH),
			int((mOverGuiMeshRectangle.y/36.f)*HEIGHT),
			int((mOverGuiMeshRectangle.z/64.f)*WIDTH),
			int((mOverGuiMeshRectangle.w/36.f)*HEIGHT) );

		glClear( GL_DEPTH_BUFFER_BIT);
		glDepthMask( 1 );
		glEnable( GL_DEPTH_TEST );
		glUseProgram( overGuiShaderProgram );
		glDisable( GL_BLEND );

		static float rotObject = 0.f;
		rotObject += aTimeEllapsed;

		matrix_t objProj;
		objProj.glhPerspectivef2(90.f, 1.f, 0.05f, 2000.f);


		matrix_t overGuiMat, overGuiView, objRot;
		objRot.rotationY( rotObject );

		overGuiView.lookAtRH( mOverGuiMesh->bSphere + vec(0.13f) * mOverGuiMesh->bSphere.w*2.0f + vec(0.f,-mOverGuiMesh->bSphere.w*0.06f,0.f) , mOverGuiMesh->bSphere, vec(0.f, 1.f, 0.f) );
		overGuiMat = objRot * overGuiView * objProj;

		glUniformMatrix4fv(overGuiModelViewProjectionMatrix, 1, 0, overGuiMat.m16);


		drawMesh( mOverGuiMesh );
	}

	//
	glUseProgram( 0 );
	glDisable(GL_BLEND);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	ggui.drawBitmaps();


	if (physicWorld.getDynamicWorld()&&GDebugDrawPhysic)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glLoadIdentity();									// Reset The Projection Matrix
		glMultMatrixf( pCamera->mProjection.m16 );

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glLoadIdentity();		
		glMultMatrixf( pCamera->mView.m16 );

		static GLDebugDrawer sDebugDraw;
		physicWorld.getDynamicWorld()->setDebugDrawer( &sDebugDraw );
		physicWorld.getDynamicWorld()->debugDrawWorld();

		btDiscreteDynamicsWorld* m_dynamicsWorld = physicWorld.getDynamicWorld();

		static GL_ShapeDrawer*	m_shapeDrawer = NULL;
		if (!m_shapeDrawer)
			m_shapeDrawer = new GL_ShapeDrawer ();

		m_shapeDrawer->enableTexture(true);


		btScalar	m[16];
		btMatrix3x3	rot;rot.setIdentity();
		const int	numObjects=m_dynamicsWorld->getNumCollisionObjects();
		btVector3 wireColor(1,0,0);
		for(int i=0;i<numObjects;i++)
		{
			btCollisionObject*	colObj=m_dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody*		body=btRigidBody::upcast(colObj);
			if(body&&body->getMotionState())
			{
				btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
				myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
				rot=myMotionState->m_graphicsWorldTrans.getBasis();
			}
			else
			{
				colObj->getWorldTransform().getOpenGLMatrix(m);
				rot=colObj->getWorldTransform().getBasis();
			}
			btVector3 wireColor(1.f,1.0f,0.5f); //wants deactivation
			if(i&1) wireColor=btVector3(0.f,0.0f,1.f);
			///color differently for active, sleeping, wantsdeactivation states
			if (colObj->getActivationState() == 1) //active
			{
				if (i & 1)
				{
					wireColor += btVector3 (1.f,0.f,0.f);
				}
				else
				{			
					wireColor += btVector3 (.5f,0.f,0.f);
				}
			}
			if(colObj->getActivationState()==2) //ISLAND_SLEEPING
			{
				if(i&1)
				{
					wireColor += btVector3 (0.f,1.f, 0.f);
				}
				else
				{
					wireColor += btVector3 (0.f,0.5f,0.f);
				}
			}

			btVector3 aabbMin,aabbMax;
			m_dynamicsWorld->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);

			aabbMin-=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
			aabbMax+=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);

			m_shapeDrawer->drawOpenGL(m,colObj->getCollisionShape(),wireColor,0,aabbMin,aabbMax);

		}
	}
}
