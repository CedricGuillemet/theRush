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

#include "content.h"

#include "mesh.h"
#include "world.h"
#include "camera.h"
#include "gui.h"
#include "track.h"

#include "therush.h"
#include "render.h"
#include "game.h"
#include "physics.h"
#include "audio.h"

#include "tinythread.h"
#include "ImageManipulator.h"
#include "3DSLoader.h"

#ifdef _DEBUG
#define DEBUG_WRITE_ROCK_NORMAL_TEXTURE_TO_PNG  1
#else
#define DEBUG_WRITE_ROCK_NORMAL_TEXTURE_TO_PNG  0
#endif

#if DEBUG_WRITE_ROCK_NORMAL_TEXTURE_TO_PNG
#include "stb_image_write.h"
#endif

#include "include_GL.h"
#include "include_SDL.h"

#ifdef LINUX
#define _stricmp strcmp
#endif

std::map<std::string, mesh_t*> prefabs;
std::map<std::string, unsigned int> textures;
mesh_t *Convert3dsMesh( Mesh3DS *mesh );

static const float poisson[]={
    -0.7156625f, -0.5676929f,
    -0.4479515f, -0.4064881f,
    -0.5602767f, -0.6753573f,
    -0.1859253f, -0.5129925f,
    -0.0626616f, -0.2489686f,
    -0.9082983f, -0.2934761f,
    -0.7727796f, -0.1372935f,
    -0.9566305f, -0.01781697f,
    -0.7525854f, 0.1281011f,
    -0.3530746f, -0.1634489f,
    -0.07578549f, -0.666626f,
    -0.3290774f, 0.09741066f,
    -0.6190775f, -0.2589825f,
    -0.2878782f, 0.4254526f,
    -0.5322883f, 0.07044882f,
    -0.2506151f, 0.6103317f,
    -0.1248656f, 0.253704f,
    -0.07963683f, -0.0219193f,
    0.02445977f, -0.824051f,
    0.2097377f, -0.06578364f,
    0.06330776f, -0.497751f,
    -0.2754177f, -0.7771276f,
    0.0213044f, 0.4779276f,
    0.1997149f, 0.2166541f,
    0.3121204f, -0.5361038f,
    0.5222889f, 0.3613991f,
    -0.1326703f, -0.9607863f,
    0.510476f, 0.06971583f,
    -0.5262984f, 0.4977616f,
    -0.5061155f, 0.7925663f,
    0.4031245f, -0.179749f,
    0.183956f, 0.7144879f,
    0.1882124f, -0.3329107f,
    0.3242353f, 0.4260685f,
    0.2076432f, -0.7560259f,
    -0.02167154f, 0.8987929f,
    0.6143446f, -0.1587977f,
    0.7385247f, -0.02769649f,
    0.7179573f, -0.3144497f,
    0.4880966f, -0.4485202f,
    0.9306735f, -0.1776443f,
    0.7046303f, 0.5568131f,
    0.9731737f, 0.09411023f,
    -0.003404821f, 0.7136531f,
    0.8436828f, 0.3864793f,
    0.6791437f, 0.1964355f
};

const char *screenVertex={"varying vec2 uv; void main(){gl_Position = gl_Vertex; uv = vec2(gl_Vertex.x,gl_Vertex.y);}\0"};


float GetApplicationTime();
GLuint loadShaderProgram(const char *sourceVertex, const char*sourceFragment, const char* debugName);

struct ScreenAdvertising_t
{
    GLuint program;
    GLuint time;

    void Bind()
    {
        glUseProgram( program );
        glUniform1f( time, GetWorldTime() );
    }
};

static const char *AdvertShaders[]={ "Monjori.fs", "Nautilus.fs", "Apple.fs", "Flower.fs" };
   
int GetScreenAdsShaderCount() { return sizeof(AdvertShaders)/sizeof(const char*); }
ScreenAdvertising_t screenAdsShaderPrograms[sizeof(AdvertShaders)/sizeof(const char*)];
void BindScreenAdsShader( int idx )
{
    screenAdsShaderPrograms[idx].Bind();
}

void InitScreenAdsShaderPrograms()
{
    for (int i=0;i<GetScreenAdsShaderCount();i++)
    {
        ScreenAdvertising_t& sa = screenAdsShaderPrograms[i];
        std::string shaderFileName = "Datas/Shaders/Adverts/";
        shaderFileName += AdvertShaders[i];
        sa.program = loadShaderProgram( screenVertex, getFileFromMemory( shaderFileName.c_str() ).c_str(), AdvertShaders[i] );
        sa.time = glGetUniformLocation( sa.program, "time" );
    }
}

unsigned int GLensTexture;
unsigned int GSparkleTexture;
unsigned int GExplosionTexture;
unsigned int GPlasmaTexture;
unsigned int GElectricWallTexture;


///////////////////////////////////////////////////////////////////////////////////////////////////
const int GeoPrefabsCount = 6;
mesh_t *GeoSpherePrefabs[GeoPrefabsCount] = {NULL};
mesh_t *GeoSmoothedSpherePrefabs[GeoPrefabsCount] = {NULL};

void InitGeoSpherePrefabs()
{
    for (int  i = 0 ; i < GeoPrefabsCount; i ++)
    {
        GeoSpherePrefabs[i] = generateSuper( 0.5f, 0.5f, 0.5f, i+1);
        GeoSmoothedSpherePrefabs[i] = generateSuper( 0.5f, 0.5f, 0.5f, i+1, true, true, true, true );
    }
}
mesh_t *GetGeoSpherePrefab( int aLevel )
{
    ASSERT_GAME( 1 <= aLevel && aLevel <= GeoPrefabsCount );
    return GeoSpherePrefabs[ aLevel-1 ];
}

mesh_t *GetGeoSmoothedSpherePrefab( int aLevel )
{
    ASSERT_GAME( 1 <= aLevel && aLevel <= GeoPrefabsCount );
    return GeoSmoothedSpherePrefabs[ aLevel-1 ];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Assets/slices.cpp"

shapePattern_t shapePatterns[]=
{
        //////////////////////////////////
	// 0 - startup begin
	{
	&Shapes[Startup2],
	&Shapes[Startup1],
	0,
	0x80AAAAAA,
	0xD0000080,
	},
	// 1 - startup 
	{
	&Shapes[Startup1],
	&Shapes[Startup2],
	0,
	0x80AAAAAA, // AABBGGRR
	0xD0000080,
	},
	// 2- startup end
	{
	&Shapes[Startup2],
	&Shapes[Startup2],
	0,
	0x80AAAAAA, // AABBGGRR
	0xD0000080,
	},
	// 3 - default
	{
	&Shapes[Startup3],
	&Shapes[Startup3],
	0,
	0x80AAAAAA, // AABBGGRR
	0xD0000080,
	},

        //////////////////////////////////
	// 4 - tremplin begin
	{
	&Shapes[tremplin3],
	&Shapes[tremplin1],
	0,
	0x80000088, // AABBGGRR
	0xD0000080,
	},
	// 5 - tremplin end
	{
	&Shapes[tremplin1],
	&Shapes[tremplin2],
	0,
	0x80000088, // AABBGGRR
	0xD0000080,
	},
        // 6 - tremplin loop rouge
	{
	&Shapes[tremplin2],
	&Shapes[tremplin2],
	0,
	0x80000088, // AABBGGRR
	0xD0000080,
	},

        // 7 - tremplin loop jaune
        {
        &Shapes[tremplin2],
        &Shapes[tremplin2],
        0,
        0x8020B0D0, // AABBGGRR
        0xD0000080,
        },
        // 8 - tremplin end
        {
        &Shapes[tremplin2],
        &Shapes[tremplin1],
        0,
        0x80000088, // AABBGGRR
        0xD0000080,
        },
        // 9 - tremplin end2
        {
        &Shapes[tremplin1],
        &Shapes[tremplin3],
        0,
        0x80000088, // AABBGGRR
        0xD0000080,
        },



        //////////////////////////////////
        // 10 - tunnel begin
	{
	&Shapes[tunnel2],
	&Shapes[tunnel3],
	0,
	0x80AAAAAA, // AABBGGRR
	0xD0000080,
	},
        // 11 - tunnel part 1
	{
	&Shapes[tunnel3],
	&Shapes[tunnel3],
	0,
	0x80AAAAAA, // AABBGGRR
	0xFFFFFFFF,
	},
        // 12 - tunnel part 2
	{
	&Shapes[tunnel1],
	&Shapes[tunnel1],
	0,
	0x80AAAAAA, // AABBGGRR
	0xFFFFFFFF,
	},
        // 13 - tunnel end
	{
	&Shapes[tunnel3],
	&Shapes[tunnel2],
	0,
	0x80AAAAAA, // AABBGGRR
	0xD0000080,
	},
	


        //////////////////////////////////
        // 14 - trans p1
	{
	&Shapes[trans1],
	&Shapes[trans2],
	0,
	0x80808080, // AABBGGRR
	0x900000A0,
	},
        // 15 - trans p1
	{
	&Shapes[trans2],
	&Shapes[trans3],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},
        // 16 - trans p1
	{
	&Shapes[trans3],
	&Shapes[trans3],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},
        // 17 - trans p1
	{
	&Shapes[trans4],
	&Shapes[trans5],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},
        // 18 - trans middle
	{
	&Shapes[trans5],
	&Shapes[trans5],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},

        // 19 - trans p1
        {
        &Shapes[trans5],
        &Shapes[trans4],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },
        // 20 - trans p1
        {
        &Shapes[trans3],
        &Shapes[trans3],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },
        // 21 - trans p1
        {
        &Shapes[trans3],
        &Shapes[trans2],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },
        // 22 - trans p1
        {
        &Shapes[trans2],
        &Shapes[trans1],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },

        //////////////////////////////////////////////////////////////////////////////////////////
        // transparent with borders
        // 23 - trans p1
	{
	&Shapes[trans1],
	&Shapes[trans2],
	0,
	0x80808080, // AABBGGRR
	0x900000A0,
	},
        // 24 - trans p1
	{
	&Shapes[trans2],
	&Shapes[trans6],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},
        // 25 - trans p1
	{
	&Shapes[trans6],
	&Shapes[trans6],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},
        // 26 - trans p1
	{
	&Shapes[trans7],
	&Shapes[trans8],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},
        // 27- trans middle
	{
	&Shapes[trans8],
	&Shapes[trans8],
	0,
	0x80808080, // AABBGGRR
	0x90000080,
	},

        // 28 - trans p1
        {
        &Shapes[trans8],
        &Shapes[trans7],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },
        // 29- trans p1
        {
        &Shapes[trans6],
        &Shapes[trans6],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },
        // 30 - trans p1
        {
        &Shapes[trans6],
        &Shapes[trans2],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },
        // 31 - trans p1
        {
        &Shapes[trans2],
        &Shapes[trans1],
        0,
        0x80808080, // AABBGGRR
        0x90000080,
        },

};





shapeSequence_t shapeSequences[]=
{
    // tunnel
    {4, false,  1000, { {&shapePatterns[10], false},{&shapePatterns[11], true}, {&shapePatterns[12], true},{&shapePatterns[13],false}}},
    // startup
    {123, false, 1000, { {&shapePatterns[2], false}, {&shapePatterns[0], false},{&shapePatterns[1], false}, {&shapePatterns[2], false},
                  {&shapePatterns[2], false}, {&shapePatterns[0], false},{&shapePatterns[1], false}, {&shapePatterns[2], false},
                  {&shapePatterns[2], false}, {&shapePatterns[0], false},{&shapePatterns[1], false}, {&shapePatterns[2], false},
                  {&shapePatterns[2], false}, {&shapePatterns[0], false},{&shapePatterns[1], false}, {&shapePatterns[2], false} 
                    } },
    // transparent straight
    {9, true,   30, { {&shapePatterns[14], false},{&shapePatterns[15], false}, {&shapePatterns[16], true}, {&shapePatterns[17], false},
                  {&shapePatterns[18], true },
                  {&shapePatterns[19], false},{&shapePatterns[20], true}, {&shapePatterns[21], false}, {&shapePatterns[22], false} } },
    // hole
    {6, false,  1000, { {&shapePatterns[4], false},{&shapePatterns[5], false},
                  {&shapePatterns[6], true },{&shapePatterns[7], true },
                  {&shapePatterns[8], false},{&shapePatterns[9], false} } },

    // transparent straight with borders
    {9, true,   30, { {&shapePatterns[23], false},{&shapePatterns[24], false}, {&shapePatterns[25], true}, {&shapePatterns[26], false},
                  {&shapePatterns[27], true },
                  {&shapePatterns[28], false},{&shapePatterns[29], true}, {&shapePatterns[30], false}, {&shapePatterns[31], false} } },
    };




int shapePattern_t::GenerateMesh(u8 *vtDest, int fmt, u16* indDest,
		int &nbIndices,
		const matrix_t& mtNear, const matrix_t& mtFar,
		int triangulation, int shapeSegmentArray, /* 0= solid, 1=transparent,...*/
        float aWidthRatioNear, float aWidthRatioFar, 
        u32 lightColor, u32 darkColor, 
        int leftDarkStart, int leftDarkCount,
        int rightDarkStart, int rightDarkCount, 
                                 u32 wallColor
        ) const 
{
	int nbSegsToUse = shapeNear->mNbSegs[shapeSegmentArray];
	if (!nbSegsToUse)
		return 0;

	int vtSize = GetVertexSizeFromFormat( fmt );
	triangulation |= mTriangulation;
	float *pfDest = (float*)vtDest;
	unsigned short *pind = (u16*)indDest;

	float* toTransform[2][2] = { {NULL, NULL}, {NULL, NULL} };

	
	shapeSegment_t* sst[] = {shapeNear->mSegs[shapeSegmentArray], shapeFar->mSegs[shapeSegmentArray]};



	int nbVts1 = nbSegsToUse*2 + nbSegsToUse*2; // 2 VT/seg * 2 sides
	int nbVts2 = nbSegsToUse*2 + nbSegsToUse*2; // 2 VT/seg * 2 sides
	int idxVt1 = 0;
	int idxVt2 = nbVts1;

	
	const matrix_t* ssm[] = { &mtNear, &mtFar };

	u32 usedColor;


	for (int l=0;l<2;l++) // front and back
	{
		toTransform[l][0] = pfDest;

		for (int k=0;k<2;k++) // sides
		{
			shapeSegment_t *pSegs = sst[l];//shapeNear->mSegs;
			for (int i = 0 ; i < nbSegsToUse ; i++)
			{


	            if (shapeSegmentArray) 
		            usedColor = colorTransparentMesh;
	            else 
                {
		            //usedColor = 0xFFFFFFFF;//colorSolidMesh;
                    if (!k)
                    {
                        // right
                        if ( (i>=rightDarkStart) && (i<(rightDarkStart+rightDarkCount)) )
                            usedColor = darkColor;
                        else
                            usedColor = lightColor;
                    }
                    else
                    {
                        // left
                        if ( (i>=leftDarkStart) && (i<(leftDarkStart+leftDarkCount)) )
                            usedColor = darkColor;
                        else
                            usedColor = lightColor;
                    }
                }



				// position
				vec_t pos1 = vec((k==1)?-pSegs->x1:pSegs->x2, (k==1)?pSegs->y1:pSegs->y2, 0.f);
				vec_t pos2 = vec((k==1)?-pSegs->x2:pSegs->x1, (k==1)?pSegs->y2:pSegs->y1, 0.f);

                if ( ( fabsf(pos1.x)>= (12.f-FLOAT_EPSILON) ) &&
                    ( fabsf(pos2.x)>= (12.f-FLOAT_EPSILON) ) )
                {
                    int v1 = (usedColor&0xFF) + (wallColor&0xFF);
                    v1 = ( v1 > 0xFF )?0xFF:v1;
                    usedColor &= 0xFFFFFF00;
                    usedColor += v1;
                }
                
				u32 alpha = pSegs->color<<24;
				if (alpha > 0x80)
				{
					usedColor &= 0xFFFFFF;
					usedColor += alpha;
				}
				
				vec_t norm, dif = pos2-pos1;
				norm.cross( vec(0.f, 0.f, 1.f), dif );
				if (norm.lengthSq() < FLOAT_EPSILON)
				{
					if (pSegs->y1>pSegs->y2)
						norm = vec( -1.f, 0.f, 0.f );
					else
						norm = vec( 1.f, 0.f, 0.f );
				}

				*pfDest++ = pos2.x; *pfDest++ = pos2.y; *pfDest++ = pos2.z;
				if (fmt&VAF_NORMAL)
				{	*pfDest++ = norm.x; *pfDest++ = norm.y; *pfDest++ = norm.z; }
				if (fmt&VAF_COLOR)
					*pfDest++ = *(float*)&usedColor;

				*pfDest++ = pos1.x; *pfDest++ = pos1.y; *pfDest++ = pos1.z;
				if (fmt&VAF_NORMAL)
				{ *pfDest++ = norm.x; *pfDest++ = norm.y; *pfDest++ = norm.z; }
				if (fmt&VAF_COLOR)
					*pfDest++ = *(float*)&usedColor;
		
				pSegs++;
			}
		}
	}

	// capping
	
	int nbVertsRet = nbVts1 + nbVts2;
	for (int i = 0;i<2 ; i++)
	{
		float adirz = i?-1.f:1;
		if (triangulation & (1<<i) )
		{
			toTransform[i][1] = pfDest;

			float *svgf = pfDest;
			float *srcv = (float*)(vtDest+(vtSize * nbVts1 * i));

			//memcpy( pfDest, srcv, vtSize * nbVts1);
			// recreer les vertices, les pusher

			
			int decColor = (fmt&VAF_NORMAL)?6:3;
			int vtSizeDecal = (vtSize>>2);

			for (int ix=0;ix<nbVts1/4;ix++)
			{
				pfDest[0] = srcv[0]; pfDest[1] = srcv[1]; pfDest[2] = srcv[2];
				if (fmt&VAF_NORMAL)
					pfDest[3] = 0.f; pfDest[4] = 0.f; pfDest[5] = adirz;

				
				if (fmt&VAF_COLOR)
					pfDest[decColor] = srcv[decColor];

				pfDest += vtSizeDecal;
				srcv += vtSizeDecal*2;
			}
			
			srcv += (vtSize>>2)*((nbVts1/2)-2);
			for (int ix=0;ix<nbVts1/4;ix++)
			{
				pfDest[0] = srcv[0]; pfDest[1] = srcv[1]; pfDest[2] = srcv[2];
				if (fmt&VAF_NORMAL)
					pfDest[3] = 0.f; pfDest[4] = 0.f; pfDest[5] = adirz;
				
				if (fmt&VAF_COLOR)
					pfDest[decColor] = srcv[decColor];

				pfDest += (vtSize>>2);
				srcv -= (vtSize>>2)*2;
			}
			
		
			pind += (pushDelaunayIndices((u8*)svgf, (nbVts1/2), vtSize, pind, nbIndices, nbVertsRet, (i!=0) ) * 3);

			nbVertsRet += (nbVts1/2);
		}
	}
	
	// indices
	for (int i = 0  ; i < nbVts1 ; i+=2)
	{
		*pind ++ = static_cast<unsigned short>( idxVt1+i );
		*pind ++ = static_cast<unsigned short>( idxVt1+((i+1)%nbVts1) );
		*pind ++ = static_cast<unsigned short>( idxVt2+i );
		
		*pind ++ = static_cast<unsigned short>( idxVt2+((i+1)%nbVts1) );
		*pind ++ = static_cast<unsigned short>( idxVt2+i );
		*pind ++ = static_cast<unsigned short>( idxVt1+((i+1)%nbVts1) );

		nbIndices += 6;
	}

		
	// transform position and normals
	for (int i=0;i<2;i++) // near & far
	{
        float aWidthRatio = i?aWidthRatioFar:aWidthRatioNear;

		for (int j = 0;j<2;j++) // normal & capping
		{
			float *pf = toTransform[i][j];
			if (!pf)
				continue;

			for (int k=0;k<nbVts1;k++)
			{
				vec_t pos = *(vec_t*)pf;
                
                if ( fabsf(pos.x)>=12.f-FLOAT_EPSILON)
                {
                    if (pos.x<0.f)
                    {
                        pos.x -= 12.f*(aWidthRatio-1.f);
                    }
                    else
                    {
                        pos.x += 12.f*(aWidthRatio-1.f);
                    }
                }
                else
                {
                    pos.x *= aWidthRatio;
                }
				pos.TransformPoint( *ssm[i] );
				*pf++ = pos.x; *pf++ = pos.y; *pf++= pos.z;
				if (fmt&VAF_NORMAL)
				{
					vec_t norm = *(vec_t*)pf;
					norm.TransformVector( *ssm[i] );
					norm.normalize();
					*pf++ = norm.x; *pf++ = norm.y; *pf++= norm.z;
				}
				if (fmt&VAF_COLOR)
					pf++;
			}
		}
	}
	
	return nbVertsRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ad texts
GLuint GAdTextTexture = 0;

void generateAdTexture()
{
    static const char *szAdTexts[]={ "the  FUTURE  of  RACING  is  HAPPENING  NOW",
"what  happens  when  SCIENCE  SPEEDS  up",
"better  than  SEX  for  the  LUXURY  ELITES",
"the  ART  of  MONEY  is  eternal  vigilance",
"OUT  of  this  WORLD  and  NO  return",
"DON'T  be  scared  to  BE  yourself",
"unlocking  the  CELL,  releasing  MANKIND",
"BEHIND  media-obesity,  write  DESTINY",
"oceanic  POTENTIAL  HIGH  depth",
"aligning  YOURSELF  to  HIGH  VELOCITY",
"meeting  TOMORROW  challenges  today",
"straight  TALK  on  strategic  issues",
"DARE  to  BE  ground  BREAKING",
"rethinking  HUMAN  CAPITAL",
"a  FUNDAMENTAL  reinvention  of  CASH",
"BUSINESS  battles  the  mighty  GEEK" };

    u32 *img = new u32 [512 * 256];
    memset( img, 0, 512*256*sizeof(u32) );
    //extern unsigned char  A8X8FONT_RAW_DATA [16384];
    int strAv = 0;
    for ( int y = 0;y< 16*16;y+=16,strAv++)
    {
        int strCharAv = 0;
        const char* szTxt = szAdTexts[strAv];
        unsigned count =  (strlen( szTxt )*8);
        for ( unsigned int x = 1 ; x < count ; x+=8, strCharAv++ )
        {
            unsigned char c = szTxt[strCharAv];
            if (c && c != ' ')
            {
                u32 *pgpp = img + ( x ) + ( ( y + 3) * 512 );
                    
                for ( int cy = 0 ; cy < 8 ; cy ++ )
                {
                    const u8 *pfs = (const u8*)m8x8FontBits;
                    pfs += (c&0xF)*8 + ( ( c >> 4 ) )*8*128 + ( cy )*128;
                        
                    for (int cx = 0; cx < 8 ; cx ++ )
                    {
                        *pgpp++ = ( *pfs++ )?0xFFFFFFFF:0;
                    }
                        
                    pgpp+= (512-8);
                }
            }
        }
    }

    for ( int y = 0 ; y < 256 ; y++ )
    {
        for ( int x = 0 ; x < 512 ; x++ )
        {

            if ( img[ (y<<9)+x] == 0xFFFFFFFF )
                continue;

            int sty = ((y-1)>=0)?(y-1):0;
            int stx = ((x-1)>=0)?(x-1):0;
            int ndy = ((y+2)<=256)?(y+2):256;
            int ndx = ((x+2)<=512)?(x+2):512;
            
            for (int my = sty; my <ndy; my++)
            {
                for (int mx = stx; mx < ndx; mx ++)
                {
                    if ( img[ (my<<9) +mx] == 0xFFFFFFFF ) 
                    {
                        img[ (y<<9)+x] = 0xFF000000; 
                        goto hasBlack;
                    }
                }
            }
            
hasBlack:; 
        }
    }

    glGenTextures( 1, &GAdTextTexture );
	glBindTexture( GL_TEXTURE_2D, GAdTextTexture );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, img );
    glGenerateMipmap(GL_TEXTURE_2D);


    delete [] img;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
std::vector<mesh_t*> GReactors[4];
std::vector<mesh_t*> GShips[4];
*/
const int TeamCount = 4;
const int ShipCount = 8;

mesh_t* GReactors[TeamCount][ShipCount] = {{0}};
mesh_t* GShips[TeamCount][ShipCount] = {{0}};

std::vector<btCollisionShape*> GPhysicShip[TeamCount];

/*
std::vector<mesh_t*> ghipNoseMeshes;
std::vector<mesh_t*> ghipBackMeshes;
std::vector<mesh_t*> ghipSideMeshes;
std::vector<mesh_t*> ghipMiddleMeshes;
std::vector<mesh_t*> ghipReactor;
*/
std::vector<mesh_t*> ghipGameObjects;
#if 0
typedef struct compoungShips_t
{
    compoungShips_t( u32 shipNb ) //mesh_t * pRenderingMesh, btCollisionShape* pPhysicMesh )
    {
        /*mRenderingMesh = pRenderingMesh;
        mPhysicMesh = pPhysicMesh;
        */
        mesh_t *compound[4];
        
        int part1 = (shipNb>>24)&0xFF;
        int part2 = (shipNb>>16)&0xFF;
	    int part3 = (shipNb>>8)&0xFF;
	    int part4 = (shipNb)&0xFF;
	    compound[0] = ghipNoseMeshes[ part1 ];
	    compound[1] = ghipSideMeshes[ part2 ];
	    compound[3] = ghipMiddleMeshes[ part3 ];
	    compound[2] = ghipBackMeshes[ part4 ];


	    // rendering

	    mRenderingMesh = merge(compound, 4 );
	    mRenderingMesh->computeBSphere();
	    mRenderingMesh->visible = false;
        mRenderingMesh->reattachToStackIndex( 0 );

        // physic
        mPhysicMesh = physicWorld.BuildSimplifiedConvexHullFromMesh( mRenderingMesh );
        //mPhysicMesh->reattachToStackIndex( 0 );
    }

    compoungShips_t( const compoungShips_t& other)
    {
        mRenderingMesh = other.mRenderingMesh;
        mPhysicMesh = other.mPhysicMesh;
    }

    mesh_t *mRenderingMesh;
    btCollisionShape* mPhysicMesh;
} compoungShips_t;

std::map<u32, compoungShips_t> gShipsCompound;


int GetMiddleCount() { return ghipMiddleMeshes.size(); }
int GetSideCount() { return ghipSideMeshes.size(); }
int GetBackCount() { return ghipBackMeshes.size(); }
int GetNoseCount() { return ghipNoseMeshes.size(); }

#endif
u32 getRandomShipCompound()
{
    shipIdentifier_t res;
    res.mTeamIndex = (fastrand()&3);
    res.mEvolutionIndex = 0;
//    res.mShipColor = (fastrand()%MAX_NB_COLORS);

    return res.id;
}

mesh_t *getReactorMesh( u32 shipId )
{
    shipIdentifier_t res;
    res.id = shipId;
    
    const u32 teamNb = res.mTeamIndex;
    const u32 shipNb = res.mEvolutionIndex;//(shipId>>8)&0xFF;
    ASSERT_GAME_MSG( teamNb < TeamCount, "Trying to access team %d, but there are only %d teams.", teamNb+1, TeamCount );
    ASSERT_GAME_MSG( shipNb < ShipCount, "Trying to access ship %d, but there are only %d ships.", shipNb+1, ShipCount );

    return GReactors[teamNb][shipNb];
}


const mesh_t* getRenderingCompoundShip( u32 shipId, vec_t& reactorPosition )
{
    shipIdentifier_t res;
    res.id = shipId;
    /*
	// find it
    int part2 = (shipNb>>16)&0xFF;
    reactorPosition = ghipReactor[ part2 ]->bSphere;

	std::map<u32, compoungShips_t>::const_iterator iter = gShipsCompound.find( shipNb );
	if (iter != gShipsCompound.end())
        return (*iter).second.mRenderingMesh;

	// build it
    compoungShips_t compound( shipNb );
	gShipsCompound.insert( std::make_pair( shipNb, compound ) );

    return compound.mRenderingMesh;
    */
    const u32 teamNb = res.mTeamIndex;
    const u32 shipNb = res.mEvolutionIndex;//(shipId>>8)&0xFF;

    ASSERT_GAME_MSG( teamNb < TeamCount, "Trying to access team %d, but there are only %d teams.", teamNb+1, TeamCount );
    ASSERT_GAME_MSG( shipNb < ShipCount, "Trying to access ship %d, but there are only %d ships.", shipNb+1, ShipCount );

    reactorPosition = GReactors[teamNb][shipNb]->bSphere;

    return GShips[teamNb][shipNb];
}

btCollisionShape* GetCompoundPhysicMesh( u32 shipId )
{
    shipIdentifier_t res;
    res.id = shipId;
    
    /*
	std::map<u32, compoungShips_t>::const_iterator iter = gShipsCompound.find( shipNb );
	if (iter != gShipsCompound.end())
        return (*iter).second.mPhysicMesh;

	// build it
    compoungShips_t compound( shipNb );
	gShipsCompound.insert( std::make_pair( shipNb, compound ) );

    return compound.mPhysicMesh;
    */
    const u32 teamNb = res.mTeamIndex;//(shipId>>16)&0xFF;
    const u32 shipNb = res.mEvolutionIndex;//(shipId>>8)&0xFF;
    ASSERT_GAME_MSG( teamNb < TeamCount, "Trying to access team %d, but there are only %d teams.", teamNb+1, TeamCount );
    ASSERT_GAME_MSG( shipNb < ShipCount, "Trying to access ship %d, but there are only %d ships.", shipNb+1, ShipCount );

    return GPhysicShip[teamNb][shipNb];
}

#pragma pack(push)
#pragma pack(1)
struct smallMeshVertex_t
{
    float x,y,z;
    short nx, ny, nz;
    u32 col;
};
#pragma pack(pop)

struct expandedMeshVertex_t
{
    float x,y,z;
    float nx, ny, nz;
    uint32 col;

    void BuildFrom( const smallMeshVertex_t &sm, bool bRemoveSelfIllum )
    {
        x = sm.x;
        y = sm.y;
        z = sm.z;
        nx = static_cast<float>(sm.nx) * (1.f/32767.f);
        ny = static_cast<float>(sm.ny) * (1.f/32767.f);
        nz = static_cast<float>(sm.nz) * (1.f/32767.f);
        vec_t norm = vec( nx, ny, nz , 0.f ).normalize();
        nx = norm.x;
        ny = norm.y;
        nz = norm.z;
        col = sm.col;

        UNUSED_PARAMETER(bRemoveSelfIllum);
        /*
        vec_t fcol;
        fcol.fromUInt5551( sm.col );
        if (bRemoveSelfIllum)
            fcol.w = 0.5f;
        col = fcol.toUInt32();
        */
    }
};

struct smallAdVertex_t
{
    float x,y,z;
    short u,v;
};
struct expandedAdVertex_t
{
    float x,y,z;
    float u,v;

    void BuildFrom( const smallAdVertex_t &sm )
    {
        x = sm.x;
        y = sm.y;
        z = sm.z;
        u = static_cast<float>(sm.u) * (1.f/32767.f);
        v = 1.f-static_cast<float>(sm.v) * (1.f/32767.f);
    }
};



void readMeshIndices( mesh_t* pm, const u8 * & ptr, u32 vtCount )
{
    // indices

	pm->triCount = *((u32*)ptr);
	ptr += 4;

    bool bHasU8 = (vtCount<256);
    bool bHasU16 = (vtCount<65536);
	pm->mIA->Init( pm->triCount, VAU_STATIC, (bHasU8||bHasU16) );
    int idxStreamSize;// = ( (bHasU8?sizeof(u8):sizeof(unsigned short)) * pm->triCount);
    if ( bHasU8 )
        idxStreamSize = sizeof(u8) * pm->triCount;
    else if ( bHasU16)
        idxStreamSize = sizeof(u16) * pm->triCount;
    else
        idxStreamSize = sizeof(u32) * pm->triCount;
    if (!bHasU8)
    {
        if ( vtCount < 65536 )
	        memcpy( pm->mIA->Lock(VAL_WRITE), ptr, idxStreamSize );
        else
            memcpy( pm->mIA->Lock(VAL_WRITE), ptr, idxStreamSize );
    }
    else
    {
        u8 *pis = (u8*)ptr;
        u16 *pid = (u16*)pm->mIA->Lock(VAL_WRITE);
        for (int i = 0;i< pm->triCount;i++)
        {
            pid[i] = pis[i];//
        }
    }
    ptr += idxStreamSize;

	pm->mIA->Unlock();
}

std::vector<mesh_t*> GAdvertisings;
matrix_t GAdvertisingsAnims[20][41];
mesh_t* GetAdvertisingMesh( unsigned int idx )
{
    ASSERT_GAME( idx < GAdvertisings.size() );
    return GAdvertisings[idx];
}
matrix_t GetAdvertisingAnimationMatrix( unsigned int idx, float frame ) 
{ 
    matrix_t res;
    int idxm = static_cast<int>(frame)%40;
    int idxmp1 = idxm+1;
    float frac = fmodf(frame, 1.f);
    res.lerp( GAdvertisingsAnims[idx][idxm], GAdvertisingsAnims[idx][idxmp1], frac );
    res.orthoNormalize();
    return res;
}

unsigned int GetAdvertisingMeshCount( ) { return GAdvertisings.size(); }

const u8* CreateAdvertisingMeshAsset( mesh_t* pm, const u8 *ptr, int meshIndex )
{

	u32 vtCount = *((u32*)ptr);
	ptr += 4;

    vec_t localTranslation = vec(0.f);

    // vertices
	pm->mVA->Init( VAF_XYZ|VAF_TEX0, vtCount, true, VAU_STATIC );

    unsigned char *ptrDest = (unsigned char *)pm->mVA->Lock( VAL_WRITE);

    for (unsigned int i=0;i<vtCount;i++, ptr += 16, ptrDest += 20)
    {
        smallAdVertex_t *smv = (smallAdVertex_t*)ptr;
        expandedAdVertex_t *epv = (expandedAdVertex_t*)ptrDest;
        epv->BuildFrom( *smv );
    }  
    pm->mVA->Unlock();
    // indices
    readMeshIndices( pm, ptr, vtCount);
	

    // anims
    vec_t animsPos[41];
    animsPos[0] = vec( 0.f, 0.f, 0.f, 1.f );
    unsigned int lastAnimPos = 0;

    vec_t animsRot[41];
    animsRot[0] = vec( 0.f, 0.f, 0.f, 0.f );
    unsigned int lastAnimRot = 0;    

    // anim pos
    u8 nbAnimPos = *ptr++;
    if ( !nbAnimPos )
    {
        localTranslation.x = *(float*)ptr;
        ptr += sizeof(float);
        localTranslation.y = *(float*)ptr;
        ptr += sizeof(float);
        localTranslation.z = *(float*)ptr;
        ptr += sizeof(float);
    }
    for (int i = 0;i<nbAnimPos;i++)
    {
        u8 frame =  *ptr++;
        float px = *(float*) ptr;
        ptr += sizeof(float);
        float py = *(float*) ptr;
        ptr += sizeof(float);
        float pz = *(float*) ptr;
        ptr += sizeof(float);

        vec_t curPos = vec( px, py, pz, 1.f );

        animsPos[frame] = curPos;
        float lerpCount = 1.f/static_cast<float>(frame-lastAnimPos);
        float lerpAv = lerpCount;
        for (int j = lastAnimPos+1;j<frame;j++, lerpAv+=lerpCount)
        {
            animsPos[j].lerp(animsPos[lastAnimPos], curPos, lerpAv);
        }
        lastAnimPos = frame;
    }

    // anim rot
    u8 nbAnimRot = *ptr++;
    for (int i = 0;i<nbAnimRot;i++)
    {
        u8 frame =  *ptr++;
        float ax = *(float*) ptr;
        ptr += sizeof(float);
        float ay = *(float*) ptr;
        ptr += sizeof(float);
        float az = *(float*) ptr;
        ptr += sizeof(float);
        float ng = *(float*) ptr;
        ptr += sizeof(float);

        //vec_t curRot = 

        animsRot[frame] = vec( ax, ay, az, ng );//curRot;
        //float lerpCount = 1.f/static_cast<float>(frame-lastAnimRot);
        //float lerpAv = lerpCount;
        for (int j = lastAnimRot+1;j<frame;j++/*, lerpAv+=lerpCount*/)
        {
            animsRot[j] = vec(0.f);//animsRot[lastAnimRot];
        }
        lastAnimRot = frame;
    }
    // padding
    if (lastAnimPos < 41)
        for (int i=lastAnimPos+1;i<41;i++) 
            animsPos[i] = animsPos[lastAnimPos];

    if (lastAnimRot < 41)
        for (int i=lastAnimRot+1;i<41;i++) 
            animsRot[i] = vec( 0.f );

    if ( lastAnimRot || lastAnimPos )
    {
        matrix_t prevMat;
        prevMat.identity();
        for (int i=0;i<41;i++)
        {
            matrix_t curMat;
            curMat.rotationAxis( -animsRot[i], animsRot[i].w);
            
            prevMat = GAdvertisingsAnims[meshIndex][i] = prevMat * curMat;
            GAdvertisingsAnims[meshIndex][i].position = animsPos[i] - localTranslation;// + vec(79.3859f,	81.3748f,	-4.0476f, 0.f );
        }
    }

    // finish him!
	//pm->computeBSphere();
	pm->visible = false;

	return ptr;
}

const u8* CreateMeshAsset( mesh_t* pm, const u8 *ptr, bool bRemoveSelfIllum = false )
{
    ptr += strlen( (const char*)ptr) +1;

	u32 vtCount = *((u32*)ptr);
	ptr += 4;

    if ( vtCount < 65536 )
	    pm->mVA->Init( VAF_XYZ|VAF_NORMAL|VAF_COLOR, vtCount, true, VAU_STATIC );
    else
        pm->mVA->Init( VAF_XYZ|VAF_NORMAL|VAF_COLOR, vtCount, false, VAU_STATIC );

    unsigned char *ptrDest = (unsigned char *)pm->mVA->Lock( VAL_WRITE);

    for (unsigned int i=0;i<vtCount;i++, ptr += 22, ptrDest += 28)
    {
        smallMeshVertex_t *smv = (smallMeshVertex_t*)ptr;
        expandedMeshVertex_t *epv = (expandedMeshVertex_t*)ptrDest;
        epv->BuildFrom( *smv, bRemoveSelfIllum );
    }  
    pm->mVA->Unlock();
    
    readMeshIndices( pm, ptr, vtCount);
	
    ptr+=2;// animations bypass

    // finish him!
	pm->computeBSphere();
	pm->visible = false;

	return ptr;
}

mesh_t * tankerMesh = NULL;
mesh_t *GetTankerMesh() { return tankerMesh; }

std::vector<matrix_t> tankerValves;

mesh_t* env4Anchor = NULL, *env4 = NULL;
mesh_t *GetEnv4Anchor() { return env4Anchor; }
mesh_t *GetEnv4() { return env4; }


#if 0   //UNUSED CODE

mesh_t *GetRacingShip(int teamIdx, int evoIdx)
{
    ASSERT_GAME_MSG( 0 <= teamIdx && teamIdx < TeamCount, "Trying to access team %d, but there are only %d teams.", teamIdx+1, TeamCount );
    ASSERT_GAME_MSG( 0 <= evoIdx && evoIdx < ShipCount, "Trying to access ship %d, but there are only %d ships.", evoIdx+1, ShipCount );
    return GShips[teamIdx][evoIdx];
}

mesh_t *GetRacingReactor(int teamIdx, int evoIdx)
{
    ASSERT_GAME_MSG( 0 <= teamIdx && teamIdx < TeamCount, "Trying to access team %d, but there are only %d teams.", teamIdx+1, TeamCount );
    ASSERT_GAME_MSG( 0 <= evoIdx && evoIdx < ShipCount, "Trying to access ship %d, but there are only %d ships.", evoIdx+1, ShipCount );
    return GReactors[teamIdx][evoIdx];
}
#endif

const char *szEnvNames[TeamCount] = {
    " Arctic Land  ", 
    "City of Lestac", 
    "KohKed Desert ", 
    "Meaban Orbital" 
};

const char *GetEnvName( unsigned int envIdx )
{
    ASSERT_GAME( envIdx < TeamCount );
    return szEnvNames[ envIdx ];
    
}

const char * szTeamNames[TeamCount] = { "Frostworld Inc.", "Aniplex Motors", "Skylane Express", "Teramind Electronics" };
const char * GetTeamName( unsigned int idx )
{
    ASSERT_GAME( idx < TeamCount );
    return szTeamNames[ idx ];
}


u32 *nrmBits = NULL;

unsigned int GRockNormalTexture = 0;

tthread::thread *rockNormalMapThread = NULL;
tthread::thread *genthreads[6] = {NULL};

u32 *GSpaceSkyboxMem[6] = {NULL};

u32 *rockDiffuseBits = NULL;
unsigned int GRockDiffuseTexture[8] = {0};

mesh_t *GLowPolyCity = NULL;

mesh_t *GetLowPolyCity()
{
    return GLowPolyCity;
}

#ifdef _DEBUG
static const int RockNormalTextureDimension = 512;
#else
static const int RockNormalTextureDimension = 512;
#endif

void GenerateRockNormalTexture( void *arg )
{
    UNUSED_PARAMETER(arg);

    PROFILER_START(RockNormalTexture);
    PROFILER_START(RockPerlin);
    

    Height2D hg(512, 512);
    
    //hg.PerlinMap( 0, 0, RockNormalTextureDimension, RockNormalTextureDimension, 127.f, 128.f, 887878, 0, 80.f );
    hg.SimplexMap(0, 0, 512, 512, 255, 0, 321 );

    PROFILER_END(); // RockPerlin

    PROFILER_START(RockNormal);
    nrmBits = new u32 [ RockNormalTextureDimension * RockNormalTextureDimension ];
    hg.ComputeNormalMap( nrmBits, 4.f );

    /*
    hg.Save( "d:/temp/height.png" );
    stbi_write_png( "d:/temp/nmap.png", 4096,4096, 4, nrmBits, 4096*4 );
    delete [] nrmBits;
    */

    PROFILER_END(); // RockNormal
    PROFILER_END(); // RockNormalTexture


    //
    rockDiffuseBits = new u32[256*256 * 8];

    for ( int i = 0 ; i < 8 ; i++ )
    {
        PROFILER_START(RockDiffuseTexture);

        Height2D hgd( 256, 256 );
        
        hgd.PerlinMap( 0,0, 256, 256, 16.f, 16.f, 665+i );
        
        char tmps[512];
        sprintf( tmps,"Datas/Textures/rockGradiant%02d.raw", i+1 );
        u32* gradiant = (u32*)getFileFromMemory(tmps).c_str();

        

        for ( int y = 0 ; y < 256 ; y ++ )
        {
            for ( int x = 0 ; x < 256 ; x ++ )
            {
                u8 perlinValue = hgd.GetBits()[ (y<<8)+x ];
                perlinValue += static_cast<u8>( static_cast<float>(x) * ((256.f-32.f) / 255.f) );
                rockDiffuseBits[y*256 + x + (i*256*256)] = gradiant[ perlinValue ] | 0xFF000000;
            }
        }

        //

        //delete [] img;

        PROFILER_END(); // RockDiffuseTexture
    }

#if DEBUG_WRITE_ROCK_NORMAL_TEXTURE_TO_PNG
    //sprintf(tmps,"d:/temp/gradiants.png", i);
    stbi_write_png( "d:/temp/gradiants.png", 256,256*8, 4, rockDiffuseBits, 256*4 );
    //MessageBox(NULL,"a","a",MB_OK);
#endif
}

void ComputeSpaceSkybox( void *arg )
{
    //PROFILER_START( ComputeSpaceSkybox );
    int i = *(int*)&arg;
	/*
    PROFILER_START(perlin1);
    Height2D hg( 1024,1024 );
    hg.PerlinMap( 0,0, 1024, 1024, 127.f, 128.f, 89746, i );
    hg.ContrastBrightness( 0.3f, 0.0f );
    PROFILER_END(); // perlin 1

    PROFILER_START(perlin2);
    Height2D hg2( 512, 512 );
    hg2.PerlinMap( 0,0, 512, 512, 127.f, 128.f, 785, i );
    hg2.SmoothStep(0.55f, 1.f);
    PROFILER_END(); // perlin 2

    PROFILER_START(stars);
    Height2D hg3( 2048, 2048 );
    hg3.Stars(200.f, 255.f, 5000 );
    hg3.HorzBlur(1);
    hg3.VertBlur(1);
    PROFILER_END(); // stars
	*/
    //PROFILER_START(mix); 

	 //Height2D hg3;//( 2048, 2048 );
	 //hg3.lo
    GSpaceSkyboxMem[i] = new u32 [ 1024 * 1024];
	static const char *spaceBoxPngNames[]={"Datas/Images/skybox-env4/env4_right1.png",
		"Datas/Images/skybox-env4/env4_left2.png",
		"Datas/Images/skybox-env4/env4_top3.png",
		"Datas/Images/skybox-env4/env4_bottom4.png",
		"Datas/Images/skybox-env4/env4_front5.png",
		"Datas/Images/skybox-env4/env4_back6.png" };
	ImageManipulator spacePng(spaceBoxPngNames[i]);
	memcpy(GSpaceSkyboxMem[i], spacePng.GetBits(), 1024*1024*sizeof(u32) );
	/*
	 if (!i)
	 {
		 //Height2D::SkyAddPlanet( GSpaceSkyboxMem[i], GSpaceSkyboxMem[i] );
		 Height2D::SkyAddPlanet( spacePng.GetBits(), spacePng.GetBits() );
		 spacePng.Save("d:/fullPicture.png");

	 }
	 */
    //Height2D::SkyComposite( hg, hg2, vec(173.f/255.f, 14.f/255.f, 2.f/255.f, 1.f)*2.f, vec(6.f/255.f, 18.f/255.f, 100.f/255.f)*1.3f, hg3, GSpaceSkyboxMem[i], (i==0) );
    
    //PROFILER_END(); // mix

    //PROFILER_END(); // ComputeSpaceSkybox
}


void LoadSounds();
void initCreditMeshes();
void StartThreadedInits()
{
    rockNormalMapThread = new  tthread::thread( GenerateRockNormalTexture, 0 );
    // compute skybox
    for (int i=0;i<6;i++)
    {
        genthreads[i] = new  tthread::thread( ComputeSpaceSkybox, (void*)i );
    }
}

void WaitForThreadedInits()
{
	extern volatile float ThingsInited;

    // wait for rock thread to end
    rockNormalMapThread->join();
    delete rockNormalMapThread;
    rockNormalMapThread = NULL;
	ThingsInited++;

    // upload rock normal texture

    glGenTextures( 1, &GRockNormalTexture );
	glBindTexture( GL_TEXTURE_2D, GRockNormalTexture );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RockNormalTextureDimension, RockNormalTextureDimension, 0, GL_RGBA, GL_UNSIGNED_BYTE, nrmBits );

    delete [] nrmBits;
    nrmBits = NULL;
	ThingsInited++;
    // --
    for (int i = 0;i<8;i++)
    {
        glGenTextures( 1, &GRockDiffuseTexture[i] );
	    glBindTexture( GL_TEXTURE_2D, GRockDiffuseTexture[i] );
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rockDiffuseBits[i * 256*256] );
		ThingsInited++;
        //[i];
    }
    delete [] rockDiffuseBits;
    rockDiffuseBits = NULL;

    // wait for space skybox to finish
    for (int i=0;i<6;i++)
    {
        //
        genthreads[i]->join();
        delete genthreads[i];
        genthreads[i] = NULL;
		ThingsInited++;
    }
}

//NOTE: Disabled as it probably needs fixing
#define APPLY_SCALE_TO_SHIPS    1

void InitMeshes()
{
    LoadSounds();

    InitGeoSpherePrefabs();

	PROFILER_START(ContentMeshes);
	LOG("Init content meshes ... ");
    
	// texture
	glGenTextures( 1, &GLensTexture );
	glBindTexture( GL_TEXTURE_2D, GLensTexture );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);


    const u8* lensRaw = (const u8*)getFileFromMemory("Datas/Textures/lens.raw").c_str();
	u8 *conv = new u8 [ 192 * 32 * 4 ];
	for (int i = 0;i<192 * 32;i++)
    {
		conv[i<<2] = conv[(i<<2) + 1] = conv[(i<<2) + 2] = conv[(i<<2) + 3] = lensRaw[i];
    }

    
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 192, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, conv);
	delete [] conv;
    conv = NULL;
    glGenerateMipmap(GL_TEXTURE_2D);
    
    
	// explosion texture

	GExplosionTexture = ImageManipulator("Datas/Textures/explosion.png").UploadToGL();
	GPlasmaTexture = ImageManipulator("Datas/Textures/plasma.png").UploadToGL();
	GElectricWallTexture = ImageManipulator("Datas/Textures/electricWall.png").UploadToGL();
	/*
	glGenTextures( 1, &GExplosionTexture );
	glBindTexture( GL_TEXTURE_2D, GExplosionTexture );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
    const u8* explosionRaw = (const u8*)getFileFromMemory("Datas/Textures/explosion.raw").c_str();
    
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, explosionRaw );
    */
    // screen shaders
    InitScreenAdsShaderPrograms();

	// ships mesh
    const std::string& shipMem = getFileFromMemory("Datas/Meshes/ships.bin");
	const u8 *ptr = (const u8*)shipMem.c_str();
    const u8 *ptrEnd = (const u8*)(shipMem.c_str()+shipMem.size()-5);
	while (ptr < ptrEnd )
	{
		mesh_t *pm = new mesh_t;

        if (strstr( (const char*)ptr, (const char*)"reactor"))
        {
            int teamIndex, shipIndex;
            sscanf( (const char*)ptr,"reactor%dship%d",&teamIndex, &shipIndex );
            ASSERT_GAME_MSG( 0 < teamIndex && teamIndex <= TeamCount, "Trying to access team %d, but there are only %d teams.", teamIndex, TeamCount );
            ASSERT_GAME_MSG( 0 < shipIndex && shipIndex <= ShipCount, "Trying to access ship %d, but there are only %d ships.", shipIndex, ShipCount );
            teamIndex--;
            shipIndex--;
			
#if APPLY_SCALE_TO_SHIPS
            matrix_t ska;
            ska.scale(0.25f);
            transformMesh( pm, ska );
#endif

            GReactors[teamIndex][shipIndex] = pm ;
        }
        else
        {
            int teamIndex, shipIndex;
            sscanf( (const char*)ptr,"team%dship%d",&teamIndex, &shipIndex );
            ASSERT_GAME_MSG( 0 < teamIndex && teamIndex <= TeamCount, "Trying to access team %d, but there are only %d teams.", teamIndex, TeamCount );
            ASSERT_GAME_MSG( 0 < shipIndex && shipIndex <= ShipCount, "Trying to access ship %d, but there are only %d ships.", shipIndex, ShipCount );
            teamIndex--;
            shipIndex--;



            GShips[teamIndex][shipIndex] = pm ;
        }
        /*
		if (strstr( (const char*)ptr, (const char*)"nose"))
        {
            ghipNoseMeshes.push_back(pm);
        }
		else if (strstr( (const char*)ptr, (const char*)"middle"))
			ghipMiddleMeshes.push_back(pm);
		else if (strstr( (const char*)ptr, (const char*)"side"))
        {
			ghipSideMeshes.push_back(pm);		
        }
		else if (strstr( (const char*)ptr, (const char*)"back"))
			ghipBackMeshes.push_back(pm);
		else if (strstr( (const char*)ptr, (const char*)"reactor"))
			ghipReactor.push_back(pm);
            */

		LOG(" ship Mesh %s\n", (const char*)ptr );
		ptr = CreateMeshAsset ( pm, ptr );
	}
    // build physic shape
    const int teamIdxMax = TeamCount;
    const int shipIdxMax = ShipCount;
    for (int teamIdx = 0; teamIdx < teamIdxMax; ++teamIdx)
    {
        for (int shipIdx = 0; shipIdx < shipIdxMax; ++shipIdx)
        {
            GPhysicShip[teamIdx].push_back( physicWorld.BuildSimplifiedConvexHullFromMesh( GShips[teamIdx][shipIdx] ) );
        }
    }


	// game objects
    static const char* meshNames[] = { "speedoff", "speedon", "weaponoff", "weaponon1", "weaponon2", 
        "mine", "missile", "cityGround", "BeaconText", "Beacon","StartupPylon","finishLine" };
    static const unsigned int maxNames = sizeof(meshNames)/sizeof(const char*);
	ghipGameObjects.resize( maxNames );

    const std::string& objectsMem = getFileFromMemory("Datas/Meshes/objects3D.bin");
    ptr = (const u8*)objectsMem.c_str();
    ptrEnd = (const u8*)(objectsMem.c_str()+objectsMem.length()-5);
	while (ptr < ptrEnd )
	{
		mesh_t *pm = new mesh_t;


        for (unsigned int i = 0;i<maxNames;i++)
        {
            if (strstr( (const char*)ptr, meshNames[i]))
            {
			    ghipGameObjects[ i ] = pm;
                ptr = CreateMeshAsset ( pm, ptr );
                break;
            }
        }
	}
    // advertising
    const std::string& adsMem = getFileFromMemory("Datas/Meshes/adTexts.bin");
    ptr = (const u8*)adsMem.c_str();//OBJECTS3D_BIN_DATA;
    ptrEnd = (const u8*)(adsMem.c_str()+adsMem.length()-5);
    int meshIndex = 0;
	while (ptr < ptrEnd )
	{
		mesh_t *pm = new mesh_t;
        GAdvertisings.push_back( pm );
		ptr = CreateAdvertisingMeshAsset ( pm, ptr, meshIndex++ );
	}

    // tanker
    
    const std::string& tkMem = getFileFromMemory("Datas/Meshes/tanker.bin");
    ptr = (const u8*)tkMem.c_str();
	{
		tankerMesh = new mesh_t;
		ptr = CreateMeshAsset ( tankerMesh, ptr, true );
	}
    // place holders and pipes
    {
        std::vector<mesh_t*> reactors;
        reactors.reserve( 100 );
        u8 nbValves, nbReactors;//, nbPipes;
        nbReactors = *ptr++;

        for (unsigned int i=0;i<nbReactors;i++)
        {
            vec_t position, direction;
            float radius;
            float *pv = (float*)ptr;
            position = vec(pv[0], pv[1], pv[2]);
            direction = vec(pv[3], pv[4], pv[5]);
            radius = pv[6];

            matrix_t orient;
            orient.LookAt( position, position-direction, vec(0.f, 1.f, 0.f) );

            mesh_t* react = generateReactor( orient, radius );
            reactors.push_back( react );

            ptr += 7 * sizeof(float);

        }

        if ( nbReactors)
        {
            reactors.push_back( tankerMesh );
            tankerMesh = merge( &reactors[0], reactors.size() );

            //FIXME: should reactors[] elements be deleted explicitly?
        }

        // valves
        nbValves = *ptr++;
        for (unsigned int i=0;i<nbValves;i++)
        {
            vec_t position, direction;
            //float radius;
            float *pv = (float*)ptr;
            position = vec(pv[0], pv[1], pv[2]);
            direction = vec(pv[3], pv[4], pv[5]);
            //radius = pv[6];
            
            matrix_t orient;
            orient.LookAt( position, position+direction, vec(0.f, 1.f, 0.f) );

            tankerValves.push_back( orient );
            /*
            mesh_t* react = generateReactor( orient, radius );
            reactors.push_back( react );
            */
            ptr += 7 * sizeof(float);

        }
        // pipes
        /*nbPipes = * */ptr++;
        for (unsigned int i=0;i<nbValves;i++)
        {
            u8 nbPoints = *ptr++;
            for (unsigned int j=0;j<nbPoints;j++)
            {
                //float *pv = (float*)ptr;
                //vec_t position = vec(pv[0], pv[1], pv[2]);
                ptr += 3 * sizeof(float);
            }
        }


    }

    // ENV 4

    const std::string& env4Mem = getFileFromMemory("Datas/Meshes/env4.bin");
    ptr = (const u8*)env4Mem.c_str();
    ptrEnd = (const u8*)(env4Mem.c_str()+env4Mem.length()-5);
    while (ptr < ptrEnd )
    {
        mesh_t *pm = new mesh_t;

        if (strstr( (const char*)ptr, "env4anch"))
            env4Anchor = pm;
        else if (strstr( (const char*)ptr, "env4"))
            env4 = pm;
        else
            ASSERT_GAME(false);

        ptr = CreateMeshAsset ( pm, ptr );
    }

    // ad texture
    generateAdTexture();

    //cyl building
    world.BuildCylindricalBuildingConcrete();
    
    // credit meshes
    initCreditMeshes();
    
    // lowpoly city
    GLowPolyCity = generateCityLowPoly();

    // scale ships
#if APPLY_SCALE_TO_SHIPS
    for (int teamIdx = 0; teamIdx < teamIdxMax; ++teamIdx)
    {
        for (int shipIdx = 0; shipIdx < shipIdxMax; ++shipIdx)
        {
            matrix_t ska;
            ska.scale(0.35f);
            transformMesh( GShips[teamIdx][shipIdx], ska );
            transformMesh( GReactors[teamIdx][shipIdx], ska );
			GReactors[teamIdx][shipIdx]->computeBSphere();
			GShips[teamIdx][shipIdx]->color = vec(1.2f);
        }
    }
#endif

	LoadPrefabs();


	// textures
	textures.insert( std::make_pair(TEXTURE_LOGO, ImageManipulator(TEXTURE_LOGO).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS1, ImageManipulator(TEXTURE_POS1).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS2, ImageManipulator(TEXTURE_POS2).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS3, ImageManipulator(TEXTURE_POS3).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS4, ImageManipulator(TEXTURE_POS4).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS5, ImageManipulator(TEXTURE_POS5).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS6, ImageManipulator(TEXTURE_POS6).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS7, ImageManipulator(TEXTURE_POS7).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_POS8, ImageManipulator(TEXTURE_POS8).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_PLAYER1, ImageManipulator(TEXTURE_PLAYER1).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_PLAYER2, ImageManipulator(TEXTURE_PLAYER2).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_BEACONWEAPON, ImageManipulator(TEXTURE_BEACONWEAPON).UploadToGL() ) );
	

	textures.insert( std::make_pair(TEXTURE_HOWTO_KBD0, ImageManipulator(TEXTURE_HOWTO_KBD0).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_HOWTO_KBD1, ImageManipulator(TEXTURE_HOWTO_KBD1).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_HOWTO_KBD2, ImageManipulator(TEXTURE_HOWTO_KBD2).UploadToGL() ) );
	textures.insert( std::make_pair(TEXTURE_HOWTO_PAD, ImageManipulator(TEXTURE_HOWTO_PAD).UploadToGL() ) );

	textures.insert( std::make_pair(TEXTURE_PORTAL, ImageManipulator(TEXTURE_PORTAL).UploadToGL() ) );

	
    // done!
	LOG("Done\n");
	PROFILER_END(); // content meshes    
}


mesh_t *Convert3dsMesh( Mesh3DS *mesh )
{
	mesh_t* pm = new mesh_t;
	pm->reattachToStackIndex(0);


	pm->mIA->Init( mesh->mNumTriangles*3, VAU_STATIC );
	unsigned short *pi = ( unsigned short*)pm->mIA->Lock(VAL_WRITE);
	for (unsigned int j = 0;j<mesh->mNumTriangles;j++)
	{
		*pi++ = (unsigned short)mesh->mTriangles[j].v0;
		*pi++ = (unsigned short)mesh->mTriangles[j].v1;
		*pi++ = (unsigned short)mesh->mTriangles[j].v2;
	}
	pm->mIA->Unlock();

	pm->mVA->Init( VAF_XYZ|VAF_NORMAL|VAF_COLOR, mesh->mNumVertices, false, VAU_STATIC );
	meshColorVertex_t *ptrDest = (meshColorVertex_t *)pm->mVA->Lock( VAL_WRITE);
	Vertex3DS *ptrSrc = mesh->mVertices;
	for (unsigned int j = 0;j<mesh->mNumVertices;j++,ptrSrc++,ptrDest++)
	{
		ptrDest->set( vec(ptrSrc->x, ptrSrc->y, ptrSrc->z ), vec( ptrSrc->nx, ptrSrc->ny, ptrSrc->nz ), vec(ptrSrc->b, ptrSrc->g, ptrSrc->r, 0.5f).toUInt32() );
	}

	pm->mVA->Unlock();

	// finish him!
	pm->triCount = mesh->mNumTriangles*3;
	pm->computeBSphere();
	pm->visible = false;

	return pm;
}

void LoadPrefabs()
{
	// clean them
	std::map<std::string, mesh_t*>::iterator iter = prefabs.begin();
	for ( ; iter != prefabs.end(); ++iter)
	{
		delete (*iter).second;
	}
	prefabs.clear();

	// prefabs

	const char *szPrefabExts[] = {".3ds",".3DS"};
	std::vector<std::string> PrefabsList;
    for (int i = 0;i<sizeof(szPrefabExts)/sizeof(const char*);i++)
    {
        GetFilesList( PrefabsList, "Datas/Meshes/prefabs/", szPrefabExts[i], false, false, true );
    }
	for (unsigned int i = 0;i<PrefabsList.size();i++)
	{
		bool loadingStatus = false;
		//prefabs.insert( std::make_pair("c:/prefabs/Cube.3ds", generateBox() ) );
		Loader3DS loader3DS;
		loadingStatus = loader3DS.loadFile(PrefabsList[i]);
		
		if (!loadingStatus)
			continue;

		for (unsigned int m=0; m<loader3DS.numMeshes; m++) 
		{

			Mesh3DS* mesh = &loader3DS.meshes[m];
			
			mesh_t* pm = Convert3dsMesh( mesh );

			if (loader3DS.numMeshes>1)
			{
				char tmps[512];
				sprintf(tmps,"%s-%d", PrefabsList[i].c_str(), m );
				prefabs.insert( std::make_pair(tmps, pm ) );
			}
			else
			{
				prefabs.insert( std::make_pair(PrefabsList[i].c_str(), pm ) );
			}
		}
	}
}

mesh_t *getSprite(unsigned int idx)
{
    ASSERT_GAME( idx < ghipGameObjects.size() );

	return ghipGameObjects[idx];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// memory files

// map of hash name and data
std::map< uint32, std::string > memoryFiles;

void buildMemoryFilesFromDirectory( const char *szDirectory, bool addJSON )
{
#if 0 //IS_OS_MACOSX
    chdir("/Users/cedricguillemet/projects/svnrepository/R2/Bin");
#endif
    memoryFiles.clear();
    
    std::vector<std::string> aList;
    const char *szExts[] = {".ogg",".lua",".raw",".bin",".h",".vs",".fs"};

    for (int i = 0;i<sizeof(szExts)/sizeof(const char*);i++)
    {
        GetFilesList( aList, szDirectory, szExts[i], true, false, true );
    }
    if ( addJSON )
        GetFilesList( aList, szDirectory, ".json", true, false, true );
    
    for ( unsigned int i = 0 ; i < aList.size() ; i++ )
    {
        uint32 hash = superFastHash( aList[i].c_str(), aList[i].length() );
        memoryFiles[hash] = FileToString( aList[i].c_str() );       
        LOG("%s (%d)\n", aList[i].c_str(), hash );
    }
}

/*
void dumpToHexa( FILE *fp, u8* ptr, int count)
{
    for (int i = 0;i<count;i++)
    {
        fprintf( fp, "0x%x,", *ptr++);
        if ( !(i&15))
            fprintf( fp, "\n");
    }
}


void serializeMemoryFilesToBin( const char *szMetaMemoryFile )
{
    FILE *fp = fopen(szMetaMemoryFile, "wb");
    if (!fp)
        return;
    
    fprintf(fp, "const unsigned char MemoryFileBinary[]={\n" );

    u32 nbFiles = memoryFiles.size();
    //fwrite(&nbFiles, sizeof(u32), 1, fp );
    dumpToHexa( fp, (u8*)&nbFiles, sizeof(u32) );
    
    std::map< uint32, std::string >::iterator iter = memoryFiles.begin();
    for( ; iter != memoryFiles.end(); ++iter)
    {
        //fwrite(&(*iter).first, sizeof(uint32), 1, fp );
        dumpToHexa( fp, (u8*)&(*iter).first, sizeof(uint32) );
        u32 fsize = (*iter).second.length();
        //fwrite(&fsize, sizeof(u32), 1, fp);
        dumpToHexa( fp, (u8*)&fsize, sizeof(u32) );
        //fwrite((*iter).second.c_str(), (*iter).second.length(), 1, fp );
        dumpToHexa( fp, (u8*)(*iter).second.c_str(), (*iter).second.length() );
    }
    fprintf( fp, "};\nconst unsigned char *MemoryFileBinaryPtr = &MemoryFileBinary[0];\n");
    fclose( fp );
}
*/

void buildMemoryFilesFromMemory( const unsigned char *pd )
{
    u32 nbFiles = *(u32*)pd;
    pd += sizeof(u32);
    for ( unsigned int i = 0 ; i < nbFiles; i ++ )
    {
        u32 hash = *(u32*)pd;
        pd+=sizeof(u32);
        u32 fsize = *(u32*)pd;
        pd += sizeof(u32);
        std::string str( (char*)pd, fsize );
        pd += fsize;
        str+= '\0';
        memoryFiles[hash] = str;
    }
}

const std::string &getFileFromMemory( const char *szFileName )
{
    uint32 filenameHash = superFastHash(szFileName, strlen( szFileName ) );
    return getFileFromMemory( filenameHash );
}

const std::string& getFileFromMemory( uint32 hash )
{
    std::map< uint32, std::string >::const_iterator iter = memoryFiles.find( hash );
    if ( iter != memoryFiles.end() )
        return (*iter).second;
    static std::string memoryFileNotFound("");
    return memoryFileNotFound;    
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Credits


typedef struct creditTexts_t
{
	int nbLines;
	const char *texts[5];
}creditTexts_t;

creditTexts_t creditTexts[] = { 
	{ 3, {"A game by", "Cedric", "Guillemet" } },
    { 3, {"Gameplay by", "Cedric Guillemet","Laurent Brossard" } },
    { 4, {"Additional", "code","Maxime","Houlier" } },
	{ 2, {"Sebastien", "Hillaire"} },
	{ 3, {"Ships by", "Jean-Edouard","Fages" } },
	{ 4, {"using", "Raknet", "BulletPhysics", "SDL OpenAL" } } ,
	{ 4, {"also using", "GLee", "JSON_parser", "stb_image_write" } } ,
	{ 3, {"FlGlet by", "Glenn Chappell", "and Ian Chai" } },
    { 4, {"Additional","shaders by", "Inigo Quilez","martins upitis" } },
    { 4, {"crashrpt","mongoose","LUA","Luna" } },
	{ 4, {"thanks to", "Sean Barrett", "JSON.org", "Ben Woodhouse" } },
	{ 3, {"Timothy Lottes", "Laurent Gomilla", "Paul Hsieh" } },
    { 4, {"Ken Perlin","John Ratcliff","Erwin Coumans","Mike Acton" } },
	{ 1, {"Alex Peterson" } },
	{ 4, {"And","Original","WipEout", "team"} },
	{ 4, {"Greetings to", "nofrag.com","blogs.wefrag.com", "tigsource.com"} },
    { 3, {"3HitCombo","WestIndie","Collective" } },
	{ 4, {"indiegames.com","Darkstryder","Divide","Michael" } },
    { 4, {"MyKiwi","Aristo","Lork","ecaheti"} },
    { 4, {"elemiah","darkalex", "SauCiSSoN","polioman"} },
    { 4, {"Xotik","Mangeurdenfants","jiminy-billy-bob","JiHeM"} },
    { 4, {"Nomys_Tempar","Nerro","Caroline","KominAaa"} },
    { 4, {"Coww", "faelnor", "Krev", "moSk"} },
    { 4, {"sgt_canardo", "yutini", "Gama", "KaB"} },
    { 3, {"Gojax", "Burgerlolz", "Holi" } },
    { 4, {"SkyVector","Valryon","Stunfest","drloser"} },
    { 4, {"LeGreg","ZeTo","Redeyed","Vahron"} },
	{ 3, {"Julien Guertault", "Golaem S.A.", "Christophe Giraud"} },
    { 4, {"xandar","existenz","Piotrek Gruszka","Madeon"} },
	{ 4, {"Cedric \03","Virginie","Arthur","Zia" } },
	{ 3, {"Thanks", "for playing",".the rush//" } },
	{ 4, {"Copyright","(C)2007-2013", "Cedric", "Guillemet" } },
};

std::vector<mesh_t*> CreditMeshes;

void initCreditMeshes()
{
	PROFILER_START(CreditTexts);
    LOG("Init Credit Meshes ...");
    
	matrix_t trans, sc;
	float ass;
	for (int j = 0;j<sizeof(creditTexts)/sizeof(creditTexts_t); j++)
	{
		const int nbl = creditTexts[j].nbLines;
        ASSERT_GAME( nbl >= 1 );

		float avy = 0.f;
		mesh_t **tocomp = new mesh_t * [nbl];
		int aMaxLen = 0;
		for (int i=0;i<nbl;i++)
		{
			int nLen = strlen(creditTexts[j].texts[i]);
			if (nLen>aMaxLen)
				aMaxLen = nLen;
		}


		for (int i=nbl-1;i>=0;i--)
		{

			mesh_t *pm3 = generateText(creditTexts[j].texts[i]);
			ass = (float)(aMaxLen) / ((float)(strlen(creditTexts[j].texts[i]))); // nbre caracteres de la string / nb caracteres souhaité
            ass *= 0.8f;
			trans.translation(0, avy, 0);
			sc.scale(ass, ass, 1.f);
			transformMesh(pm3, sc * trans);
			tocomp[i] = pm3;
			avy += 8.f * ass;
		}


		mesh_t *creditsMesh;
		if ( nbl>1 )
		{
			creditsMesh = merge(tocomp, nbl);
			for (int i=0;i<nbl;i++)
			{
				delete tocomp[i];
				tocomp[i] = NULL;
			}

			delete [] tocomp;
			tocomp = NULL;
		}
		else
			creditsMesh = tocomp[0];

		CreditMeshes.push_back( creditsMesh );


		// rendering props
		float asc = 16.f/(7.f* 9.f);//1.f/((12.f*7.f)/9.f);
		sc.scale(asc, asc, asc);
		trans.translation(-8.f, 0.f, 0.f);
		transformMesh(creditsMesh, sc * trans);	

		creditsMesh->computeBSphere();

		creditsMesh->mLocalMatrix.identity();
		creditsMesh->color = vec(1,1,1,0.5f);
		creditsMesh->visible = false;
        
      
	}
	
	LOG("Done\n");    
	PROFILER_END(); // CreditTexts
}

mesh_t * getCreditsMesh(unsigned int idx)
{
    ASSERT_GAME( idx < CreditMeshes.size());
	return CreditMeshes[idx];
}

unsigned int getNextCreditsMeshIndex(unsigned int idx)
{
    const unsigned int nextIdx = (idx+1) % CreditMeshes.size();
    return nextIdx;
}

vec_t  StartupPylon::FireColors[] = {
	vec(0.35f, 0.35f, 0.35f, 0.5f), // all down
	vec(1.f, 0.f, 0.f, 1.f), // red
	vec(1.f, 0.5f, 0.25f, 1.f), // orange
	vec(1.f, 1.f, 0.f, 1.f), // jaune
	vec(0.f, 1.f, 0.f, 1.f) // vert
};

StartupPylon::StartupPylon()
{
    Clear();
}

StartupPylon::~StartupPylon()
{
    Clear();
}

void StartupPylon::Init(const matrix_t & mat)
{
	mesh_t *startPylonMeshInstance = getSprite(10)->clone();
	startPylonMeshInstance->visible = true;

	startPylonMeshInstance->mWorldMatrix.scale(0.02f);//.identity();
	startPylonMeshInstance->mWorldMatrix *= mat;
	startPylonMeshInstance->updateWorldBSphere();
	startPylonMeshInstance->color = vec(1.f, 1.f, 1.f, 0.5f);


	matrix_t startupBoxesScale;
	startupBoxesScale.scale(45.f, 35.f, 15.f);

	for (int i=0;i<4;i++)
	{
        //ASSERT_GAME( startupBoxes[i] == NULL );
        startupBoxes[i] = generateBox();
		startupBoxes[i]->visible = true;
		matrix_t stbtr;
		stbtr.translation(267.f + (float)i*60.f, 228.f, 15.f);
		startupBoxes[i]->mWorldMatrix = startupBoxesScale * stbtr * startPylonMeshInstance->mWorldMatrix;
		
		startupBoxes[i]->updateWorldBSphere();
	}
	Reset();
}

void StartupPylon::Clear()
{
    for ( int i=0; i<4; ++i )
    {
        startupBoxes[i] = NULL;
    }
}

void StartupPylon::Reset()
{
	for (int i=0;i<4;i++)
		startupBoxes[i]->color = FireColors[0];
}

void StartupPylon::Tick( float mStartupLocalTime, float mNextSuLT, float aTimeEllapsed )
{
	for (int j=0;j<5;j++)
	{
		if ((mStartupLocalTime< (float)(j)) && (mNextSuLT>(float)(j)))
		{
			for (int i=0;i<j;i++)
				startupBoxes[i]->color = FireColors[j];
		}
	}
	for (int i=0;i<4;i++)
	{
		if (startupBoxes[i]->color.w > 0.6f)
			startupBoxes[i]->color.w = LERP(startupBoxes[i]->color.w, 0.6f, aTimeEllapsed * 2.f);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if EDITOR_ENABLE
extern bool GBEditorInited;
extern bool GBEditorTestingMap;
#endif

void digTunnel( track_t *pTrack, const vec_t& trMin, float transLen, float radius = 1.f, float heightScale = 1.f )
{
	for (int i=0;i<NBSEGS;i++)
	{
		for (float j = 0;j<1.f; j+= 0.1666f)
		{
			for (int k = 0;k<2;k++)
			{
				vec_t pos = pTrack->getPointOnTrack( i, j, (k==1) );
				float width = pTrack->getWidthOnTrack( i, j, (k==1) );
                vec_t hole0;
				hole0 = pos + vec( r01()-0.5f, r01()-0.5f, r01()-0.5f );
				hole0 -= trMin;
				hole0.w = ( width * 1.3f ) * LERP( 0.92f, 1.3f, r01() ) * radius;

				hole0 *= transLen;

                hole0.y *= heightScale ;
				world.VoxelSphere( hole0, false, vec(1.f, 1.f/(heightScale*heightScale), 1.f) );
					
			}
		}
	}
}


void AddTunnelLightsAndAdScreens( track_t *pTrack, TrackTopology& topo, matrix_t *mats1, matrix_t *mats2 )
{
    int brickCountBeforeAd = 0;
    int brickCountBeforeAdLeftRight = 0;
    for (int k=0;k<1;k++)
    {
        int enteringTunnel = 0;
        //bool bFourche = (k==1);
        std::vector<TrackTopology::trackTopology_t>& vv = topo.mCurrentToopology[k];
        
        for (unsigned int i=0;i<vv.size();i++)
        {
            
            // advertising
            
            if ( (!(vv[i].trackSliceProperties&TrackTopology::TOPO_HOLE)) &&
                (!(vv[i].trackSliceProperties&TrackTopology::TOPO_INGROUND)) )
            {
                brickCountBeforeAd++;
                brickCountBeforeAdLeftRight ++;
            }
            // advert
            
            bool bCanPutAd = (!( (vv[i].trackSliceProperties&TrackTopology::TOPO_HOLE)||(vv[i].trackSliceProperties&TrackTopology::TOPO_INGROUND)||
                                (vv[i].trackSliceProperties&TrackTopology::TOPO_HIGHSLOPE)||(vv[i].trackSliceProperties&TrackTopology::TOPO_TRANSPARENT) ) );
            if ( bCanPutAd )
            {
                if ( brickCountBeforeAd > 93 )
                {
                    //world.AddCentralAd( i );
                    brickCountBeforeAd = 0;//fastrand()&7;
                }
                
                if (( vv[i].trackSliceProperties&TrackTopology::TOPO_TURNING_LEFT )&&(brickCountBeforeAdLeftRight>63))
                {
                    //world.AddSideAd( i, false, 3 );
                    brickCountBeforeAdLeftRight = 0;
                }

                if ( ( vv[i].trackSliceProperties&TrackTopology::TOPO_TURNING_RIGHT ) && (brickCountBeforeAdLeftRight>63) )
                {
                    //world.AddSideAd( i, true, 3 );
                    brickCountBeforeAdLeftRight = 0;
                }
            }
            
            
            /*
             if (bFourche&&(! (vv[i].trackSliceProperties&TrackTopology::TOPO_FOURCHE)))
             continue;
             */
            if ( world.IsVoxelDatasAvailable())
            {            
                vec_t hole0 = world.WorldToVoxel( (k==0)?mats1[i].position:mats2[i].position ) + vec(0.f, 3.f, 0.f);
                
                if ( world.VoxelPeek( hole0 ) )
                {




                    enteringTunnel ++;
                    vv[i].trackSliceProperties |= TrackTopology::TOPO_INGROUND;
                    // tunnel light
                    
                    if ( (enteringTunnel > 15) && ( (i&7) == 7)&&
                        ( !(vv[i].trackSliceProperties&(TrackTopology::TOPO_HOLEINOUT|TrackTopology::TOPO_HOLE) ) ))
                    {


                        matrix_t trkMat;
                        const AIPoint_t& aipt = track.getAIPoint(i);
                        
                        for (int l = 0;l<2;l++)
                        {
                            aipt.BuildMatrix( trkMat, (k==1) );

                            trkMat.position += trkMat.right * ((aipt.mAIwidth[k]+1.6f) * ((l==0)?1.f:-1.f));
                            trkMat.position += trkMat.up * 2.5f;
                            trkMat.position.w = 1.f;

                            matrix_t lightBoxMat;
                            lightBoxMat.scale(2.f, 2.f, 6.5f);
                            lightBoxMat.position = vec(0.f, 0.f, 3.25f, 1.f);
                            lightBoxMat *= trkMat;

                            // light box
                            static const vec_t lightColor = vec(1.1f, 0.4f, 0.20f,60.f);
                            if (pTrack->mEnvironmentIndex != 3 )
                            {
                                mesh_t *pm = generateBox();
                                pm->mWorldMatrix = lightBoxMat;
                                pm->updateWorldBSphere();
                                pm->color = lightColor;
                                pm->visible = true;
                                pm->physic = false;
                                pm->mbCastShadows = false;
                            }

                            // light
                            vec_t lgtpos = lightBoxMat.position;
                            lgtpos.w = 32.f;

                            omniLight_t *pl = Renderer::newOmni();
                            pl->mPosition = lgtpos;// + vec(0.f, 2.f, 0.f, 30.f);
                            pl->mColor = lightColor;
                        }
                    }
                }
                else
                    enteringTunnel = 0;
            }
        }
    }
    
    LOG(" ad and tunnel lights done.\n");
}

void TrackSetupWorld( track_t *pTrack, TrackTopology& topo, matrix_t *mats1, matrix_t *mats2, PerSliceInfo_t *psi1, PerSliceInfo_t *psi2 )
{
    UNUSED_PARAMETER(psi1);
    UNUSED_PARAMETER(psi2);

#if EDITOR_ENABLE
    if (GBEditorInited && (!GBEditorTestingMap) )
        return;
#endif

    PROFILER_START(TrackSetupWorld);
    g_seed = 2063437480;

    
    // ----------------------------------------------------------------------------------------------------
    // arctic
    if ( pTrack->mEnvironmentIndex == 0 )
    {
        BuildAuroraBorealis( vec(0.f, 150.f, 0.f ) , 200.f, 1600, 600.f, 8 );
        BuildAuroraBorealis( vec(0.f, 160.f, 500.f ) , 120.f, 1000, 400.f, 6 );
        BuildAuroraBorealis( vec(0.f, 160.f, -500.f ) , 120.f, 1000, 400.f, 6 );
    }

    // city
    if ( pTrack->mEnvironmentIndex == 1 )
    {
        world.ClearVoxelDatas();
        AddTunnelLightsAndAdScreens( pTrack, topo, mats1, mats2 );
    }
    
    // space
    if (pTrack->mEnvironmentIndex == 3 )
    {
        vec_t trMin = vec(-1000.f,-800.f, -1000.f);
        vec_t trMax =  vec(1000.f,300.f, 1000.f);


        float asterSize = 32.f;
        vec_t asteroPos[4], asteroSize[4];
        for (int i = 0;i<4;i++)
        {
            //asteroPos[i] = LERP(vec( asterSize ), vec( 256.f - asterSize, 128.f-asterSize, 256.f-asterSize ), r01() );
            asteroPos[i] = vec( asterSize + ( 256.f - asterSize*2.f) * r01(),
                asterSize + ( 128.f - asterSize*2.f) * r01(),
                asterSize + ( 256.f - asterSize*2.f) * r01() );

            asteroSize[i] = vec ( (1.f/(asterSize + r01() * asterSize ) ),
                 (1.f/(asterSize + r01() * asterSize ) ),
                 (1.f/(asterSize + r01() * asterSize ) )
                );
        }
        /*
        asteroPos[0] = vec(asterSize,asterSize,asterSize);
        asteroPos[1] = asteroPos[0] * 2.f;
         asteroSize[0] =  asteroSize[1] = (vec(1.f/asterSize));
         */
        vec_t lmin = trMin;

        vec_t lmax = trMax;
        vec_t dif = lmax-lmin;

        dif = lmax-lmin;

        float transLen = 256.f / zmax(dif.x, dif.z);


        // terrain
        world.SetWorldToVoxelConversionParameters( trMin, transLen );
        world.InitVoxelTerrain(256, 128, 256);

        int trackIndex = pTrack - GetTrack0();

        Perlin *pinpin = new Perlin( 4, 4, 1.f, 5541 + trackIndex );
        float ax = 0.f;
        for (int x = 0 ; x < 256 ; x++, ax+=(1.f/256.f) )
        {
            float fact = 1.f;
            for (int y = 0 ; y < 128 ; y++, fact-=(1.f/127.f) )
            {
                float az = 0.f;
                for (int z = 0 ; z < 256 ; z++, az+=(1.f/256.f) )
                {
                    float v = (pinpin->Get( ax, fact, az )*0.5f)+0.5f;

                    float cumul = 0.f;
                    for ( int i = 0 ; i < 4 ; i++ )
                    {
                        vec_t pos = vec( x, y, z ) - asteroPos[i];
                        pos *= asteroSize[i];

                        cumul +=  Clamp( v * 2.f - pos.length(), 0.f, 1.f );
                    }
                    if ( cumul > 0.5f )
                    {
                        world.VoxelPoke(  x, y, z, true );
                    }
                }
            }
        }
        delete pinpin;
        pinpin = NULL;

        AddTunnelLightsAndAdScreens( pTrack, topo, mats1, mats2 );

	    digTunnel( pTrack, trMin, transLen, 1.f );
        // set 

        // tunnel sub


        LOG("pre voxel mesh\n");

        {
            mesh_t *terr = world.MakeVoxelMesh( 1.8f );
            terr->computeBSphere();

            matrix_t scTerr, trTerr;
            scTerr.scale(1.f/transLen);
            trTerr.translation(trMin);

            terr->mWorldMatrix = scTerr * trTerr; 
            terr->visible = true;
            terr->color = vec(130.0f/255.0f,100.0f/255.0f,14.0f/255.0f, 0.5f);
            terr->shader = vec(0.2f, 0.4f, 0.f, 0.f );

            terr->mbIsRock = true;
            terr->updateWorldBSphere();

        }

        LOG("voxel mesh done \n");
    }
    // ----------------------------------------------------------------------------------------------------
    // sand land
    if (pTrack->mEnvironmentIndex == 2)
    {

        vec_t trMin = vec(-600.f,-0.f, -600.f);
        vec_t trMax =  vec(600.f,800.f, 600.f);

        vec_t lmin = trMin;

        vec_t lmax = trMax;
        vec_t dif = lmax-lmin;

        dif = lmax-lmin;

        float transLen = 256.f / zmax(dif.x, dif.z);

        // sand
        Height2D hg( 256, 256 );
        
        hg.PerlinMap( 0,0, 256, 256, 16.f, 32.f, 89746 );
        hg.HorzBlur(4);
        hg.Noise(6);
        hg.HorzBlur(4);
        hg.Noise(3);
        hg.HorzBlur(4);

        {
            // sand
            
            vec_t trMin2 = trMin;
            float transLen2 = transLen;
            
            trMin2.x *= 2.f;
            //trMin.y *= 2.45f;//2.35f;
            trMin2.z *= 2.f;
            transLen2 *= 0.25f;
            
            world.InitVoxelTerrain(128, 32, 128);
            world.AddHeight2D( vec(0.f), vec(128.f, 32.f, 128.f), hg );
            digTunnel( pTrack, trMin2, transLen2, 2.f );
            
            mesh_t *terr = world.MakeVoxelMesh( 1.5f );
            terr->computeBSphere();
            
            matrix_t scTerr, trTerr;
            scTerr.scale(1.f/transLen2);
            trTerr.translation(trMin2);
            
            terr->mWorldMatrix = scTerr * trTerr;
            terr->visible = true;
            terr->mbCastShadows = false;
            terr->color = vec(214.f/255.0f,140.f/255.0f,69.0f/255.0f, 0.5f);
            
            terr->mbIsSand = true;
            terr->updateWorldBSphere();
            
        }

        // terrain
        world.SetWorldToVoxelConversionParameters( trMin, transLen );
        world.InitVoxelTerrain(256, 64, 256);

        int trackIndex = pTrack - GetTrack0();

        Perlin *pinpin = new Perlin( 4, 4, 1.f, 5541 + trackIndex );
        float ax = 0.f;
        for (int x = 0 ; x < 256 ; x++, ax+=(1.f/256.f) )
        {
            float fact = 1.f;
            for (int y = 0 ; y < 64 ; y++, fact-=(1.f/63.f) )
            {
                float az = 0.f;
                float facts = fact;
                for (int z = 0 ; z < 256 ; z++, az+=(1.f/256.f) )
                {
                    float v = (pinpin->Get( ax, fact, az )*0.5f)+0.5f;
                    v *= facts;

                    if ( v > 0.35f )
                    {
                        world.VoxelPoke(  x, y, z, true );
                    }
                }
            }
        }
        delete pinpin;
        pinpin = NULL;


        AddTunnelLightsAndAdScreens( pTrack, topo, mats1, mats2 );

	    digTunnel( pTrack, trMin, transLen, 1.f, 0.5f );
        // set 

        // tunnel sub


        LOG("pre voxel mesh\n");


        
        {
            mesh_t *terr = world.MakeVoxelMesh( 2.0f );
            terr->computeBSphere();

            matrix_t scTerr, trTerr;
            scTerr.scale(1.f/transLen);
            scTerr.m16[5] *= 2.f;
            trTerr.translation(trMin);

            terr->mWorldMatrix = scTerr * trTerr ; 
            terr->visible = true;
            terr->color = vec(130.0f/255.0f,100.0f/255.0f,14.0f/255.0f, 0.5f);
            terr->shader = vec(0.2f, 0.4f, 0.f, 0.f );

            terr->mbIsRock = true;
            terr->updateWorldBSphere();

        }



        LOG("voxel mesh done \n");
    }
    // ----------------------------------------------------------------------------------------------------

    if (pTrack->mEnvironmentIndex == 0)
    {

vec_t trMin = track.boundMin;
	vec_t trMax = track.boundMax;
	
	trMin.y -= 30.f;
	trMax.y += 30.f;

	vec_t lmin = trMin;

	vec_t lmax = trMax;//pTrack->boundMax;
	vec_t dif = lmax-lmin;
	
	/*
	lmin.y -= 100.f;
	lmax.y += 80.f;
	*/
	lmin -= dif*0.1f;
	lmax += dif*0.1f;


	dif = lmax-lmin;
	//dif.y = 0.f;



	float transLen = 256.f / zmax(dif.x, dif.z);
	vec_t hole0;



	LOG("VoxelEdit\n");
    world.SetWorldToVoxelConversionParameters( trMin, transLen );
	world.InitVoxelTerrain(256, 64, 256);//1024,256,1024);
	Height2D hg( 256, 256 );


    // base circle
        float minHeight = (255.f/(trMax.y - trMin.y)) * fabsf(trMin.y);
    //u8 minHeight8 = ((u8)minHeight)+1;

    hg.FloeDisk( 128, 128, 128, minHeight + r01() * 16.f, 45, fastrand(), 1.5f, 0.0f );


    // tunnels add
	for (int i=0;i<NBSEGS;i++)
	{
		for (float j = 0;j<1.f; j+= 0.1f)
		{
			for (int k = 0;k<2;k++)
			{
				vec_t pos = pTrack->getPointOnTrack(i, j, (k==1));
				if (pos.y<5.f)
				{
					hole0 = pos;
					hole0 -= trMin;
					hole0.w = 30.f + (-pos.y* 8.0f);


					//hole0.w = 30.f;//sqrtf(hole0.w);
					hole0 *= transLen;


					//world.VoxelSphere( hole0, true );
					hg.FloeDisk( static_cast<int>(hole0.x), static_cast<int>(hole0.z), static_cast<int>(hole0.w*0.8f), 120, 45, fastrand(), 0.5f, 0.9f );
				}
			}
		}
	}
    

    
    for (int i = 0;i<10;i++)
    {
		hg.FloeDisk( fastrand()&0xFF, fastrand()&0xFF, static_cast<int>( r01() * 16.f + 16.f ), 140, 45, fastrand(), 0.5f, 0.9f );
    }
	LOG(" FloeDisk done\n");
	
    

    //hg.Box(0,0,256, 256, minHeight );

    

	// banquise
#if 0
	for (int i=0;i<1;i++)
	{
		//hg.FloeDisk( 100/*+r01()*600*/, 100/*+r01()*600*/, 100, 150, 30, 23412, 0.5f, 1.f  );
	}
	//hg.FloeDisk( 512, 512, 32 );

	// banquise plus basse 
	hg.FloeDisk( 180, 70, 70, 75, 15, 98, 0.5f, 0.9f );

	// 2 pics dedans
	hg.FloeDisk( 160, 90, 30, 140, 20, 198, 0.5f, 0.9f );
	hg.FloeDisk( 220, 150, 28, 150, 30, 1298, 0.5f, 0.9f );
	
	// gros morceau à gauche
	hg.FloeDisk( 80, 80, 75, 130, 30, 7791, 0.55f, 0.9f );

	// gros bloc à droite
	hg.FloeDisk( 200, 128, 48, 115, 40, 11618, 0.5f, 0.9f );
#endif
	LOG(" map floe disk done\n");

	//hg.Save( "d:\\banquise.png" );


	hg.SmoothCliff();
	hg.SmoothCliff();
	hg.SmoothCliff();
	LOG(" smooth cliff done\n");
    
    hg.HorzBlur(1);
    hg.VertBlur(1);

	world.AddHeight2D( vec(0.f), vec(255.f, 64.f, 255.f), hg );
	// hg2 solo
	/*
	Height2D hg2( 128, 128 );
	hg2.FloeDisk( 64, 64, 60, 40, 10, 23412, 0.5f, 0.9f );

	hg2.FloeDisk( 64, 40, 27, 140, 20, 8412, 0.5f, 0.9f );
	hg2.FloeDisk( 40, 80, 25, 145, 23, 9412, 0.55f, 0.92f );
	hg2.FloeDisk( 80, 80, 29, 135, 19, 7412, 0.48f, 0.89f );
	hg2.SmoothCliff();
	hg2.SmoothCliff();
	hg2.SmoothCliff();

	hg2.Save( "d:\\banquise2.png" );
	exit(0);
	*/
	LOG(" heightmap 2 voxel done\n");

    AddTunnelLightsAndAdScreens( pTrack, topo, mats1, mats2 );
	// set

	// tunnel sub
    digTunnel( pTrack, trMin, transLen, 1.f );

	LOG("pre voxel mesh\n");

    //world.DownsampleVoxel( 4 );


	mesh_t *terr = world.MakeVoxelMesh( );
	terr->computeBSphere();
	
	matrix_t scTerr, trTerr;
	scTerr.scale( (1.f/transLen) );
	trTerr.translation(trMin);

	terr->mWorldMatrix = scTerr * trTerr; 
	terr->visible = true;
	terr->color = vec(50.f/255.f, 175.f/255.f, 183.f/255.f, 0.5f);

	terr->mbIsIceGround = true;
	terr->updateWorldBSphere();

    //physicWorld.setWorldMeshes( terr );

    LOG("voxel mesh done -> %d indices - %d vertices\n", terr->mIA->GetIndexCount(), terr->mVA->GetVertexCount() );
    }

    // finish lines

    {
        // matrix
        matrix_t lineScale, lineLocal, lineRot;
        lineRot.rotationY( PI );

        lineScale.scale( 0.06f );

        const AIPoint_t& aipt = track.getAIPoint(0);
        aipt.BuildMatrix( lineLocal, 0 );

        lineLocal.position += lineLocal.right * ( aipt.mAIwidth[0] - 1.5f );
        lineLocal.position.w = 1.f;
     
        // left
         mesh_t *finishLine = getSprite(11)->clone();
        finishLine->computeBSphere();
        finishLine->mWorldMatrix = lineRot * lineScale * lineLocal;//.scale(100.f, 200.f, 100.f);//
        finishLine->updateWorldBSphere();
        finishLine->visible = true;
        finishLine->mbCastShadows = true;
        finishLine->color = vec(1.f, 1.f, 1.f, 1.f );

        // right

        lineLocal.position -= lineLocal.right * ( aipt.mAIwidth[0] - 1.5f ) * 2.f;
        lineLocal.position.w = 1.f;

        finishLine = getSprite(11)->clone();
        finishLine->computeBSphere();
        finishLine->mWorldMatrix = lineScale * lineLocal;//.scale(100.f, 200.f, 100.f);//
        finishLine->updateWorldBSphere();
        finishLine->visible = true;
        finishLine->mbCastShadows = true;
        finishLine->color = vec(1.f, 1.f, 1.f, 1.f );
    }

    PROFILER_END();//TrackSetupWorld
}

void CreateTankerOverDesert( const vec_t& mapPos )
{
    mesh_t *groundcity = GetTankerMesh( )->clone();
    groundcity->computeBSphere();

    matrix_t roty,sca;
    roty.rotationY( 2.f * PI * r01() );
    sca.scale( 1.f );
    groundcity->mBaseMatrix = sca * roty;
    groundcity->mBaseMatrix.position = vec( mapPos.x , 405.f + r01() * 60.f, mapPos.y, 1.f );

    groundcity->visible = true;
    groundcity->color = vec(1.f, 1.f, 1.f, 1.f );


    meshAnimation_t *pa = groundcity->addAnimation();
    pa->mDuration = 9999999.f;
    pa->mDrunkAnim = true;
    pa->mDrunkTranslateScale = 18.f + r01() * 14.f;
    pa->mLocalTime = 10.f * r01();


    for (unsigned int i = 0;i<tankerValves.size();i++)
    {
        Rope_t *rope = physicWorld.NewRope();
        vec_t lotbout = vec( 0.f, -groundcity->mBaseMatrix.position.y, 0.f );


        lotbout.TransformPoint( groundcity->mBaseMatrix );

        vec_t valveDir = tankerValves[i].dir;
        valveDir.TransformVector( groundcity->mBaseMatrix );
        lotbout += vec( valveDir.x, 0.f, valveDir.z ) * (180.f + r01() * 120.f );


        rope->SetLength( (lotbout-groundcity->mBaseMatrix.position).length() * 0.9f);
        rope->SetLinks( tankerValves[i].position, groundcity, lotbout );
    }
}

struct env4building
{
    env4building( const matrix_t& mt, int hg)
    {
        mat = mt;
        height = hg;
    }
    matrix_t mat;
    int height;
};

void TrackSetupGuenin( track_t *pTrack, TrackTopology& topo, matrix_t *mats1, matrix_t *mats2, PerSliceInfo_t *psi1, PerSliceInfo_t *psi2 )
{
    UNUSED_PARAMETER(mats2);
    UNUSED_PARAMETER(psi2);

    if (pTrack->mEnvironmentIndex == 1)
    {
        mesh_t* city = generateCity( );
        //


        city->computeBSphere();
        city->visible = true;
        city->color = vec(1.f, 1.f, 1.f, 0.5f );
        city->updateWorldBSphere();

        mesh_t *groundcity = getSprite(7)->clone();
        groundcity->computeBSphere();
        //groundcity->mWorldMatrix.identity();
        groundcity->mWorldMatrix.scale(100.f, 200.f, 100.f);//
        //groundcity->mWorldMatrix.position = vec(0.f, -20.f, 0.f, 1.f);
        groundcity->updateWorldBSphere();
        groundcity->visible = true;
        groundcity->color = vec(1.f, 1.f, 1.f, 0.5f );



        float r1 = ( vec( -1375.3945f, 457.57065f, -532.00653f )-vec( -1172.4023f, 329.32339f, -1094.5302f ) ).length();
        float r2 = ( vec( 537.26624f, 371.22031f,-556.27673f )-vec( -673.10663f, 259.91925f, -325.68979f ) ).length();
        float r3 = ( vec( -65.337372f, 525.60107f, 1144.1542f )-vec( -1098.3905f, 536.12238f, 203.71213f ) ).length();

        CreateTankerOverCity( vec( -1375.3945f, 457.57065f, -532.00653f ), r1 * 0.5f, r1, r1 * 0.3f, 0.01f );
        CreateTankerOverCity( vec( 537.26624f, 371.22031f,-556.27673f ), r2 * 0.4f, r2, r2 * 0.25f, -0.02f );
        CreateTankerOverCity( vec( -65.337372f, 525.60107f, 1144.1542f ), r3 * 0.45f, r3, r3 * 0.45f, 0.015f );

        // physic mesh

        mesh_t* physicCityMesh = GetLowPolyCity();
        physicCityMesh->visible = false;
        physicWorld.addPhysicMesh( physicCityMesh );
        physicWorld.addPhysicMesh( getSprite(7) );
    }
    else if (pTrack->mEnvironmentIndex == 2)
    {
        g_seed = 2063437480;

        CreateTankerOverCity( vec( 0.f, 580.60107f, 0.f ), 300.f, 400, 200, 0.03f );

        
        CreateTankerOverDesert( vec(poisson[10] * 800.f,poisson[11] * 800.f ) );
        
        CreateTankerOverDesert( vec(poisson[20] * 400.f,poisson[21] * 400.f ) );
        
        CreateTankerOverDesert( -vec(poisson[10] * 1000.f,poisson[11] * 100.f ) );
        CreateTankerOverDesert( -vec(poisson[4] * 800.f,poisson[5] * 800.f ) );
        
        CreateTankerOverDesert( -vec(poisson[22] * 80.f, -poisson[23] * 80.f ) );

    }
    else if (pTrack->mEnvironmentIndex == 3)
    {
        g_seed = 2063437480;

         /*mesh_t *anchor;
         
        {
            anchor = GetEnv4Anchor()->clone();
            anchor->mWorldMatrix.identity();//.position = vec(0.f, 50.f, 0.f,1.f);
            anchor->updateWorldBSphere();
            anchor->visible = true;
            anchor->color = vec(1.f, 1.f, 1.f, 1.0f );
        }
        */
        {
            mesh_t *groundcity = GetEnv4()->clone();
            groundcity->mWorldMatrix.identity();//.position = vec(0.f, 50.f, 0.f,1.f);
            groundcity->updateWorldBSphere();
            groundcity->visible = true;
            groundcity->color = vec(1.f, 1.f, 1.f, 1.0f );
        }


        /*
        std::vector<env4building> buildingSpots;

        matrix_t matRot;
        matRot.rotationX(PI*0.5f);
        matrix_t scaleIt;
        scaleIt.scale(3.f);
        for (int i = 0;i<sizeof(poisson)/(sizeof(float)*2);i++)
        {
            vec_t contact, contactNormal;
            vec_t ray = vec( poisson[i*2] * 530.f, 500.f, poisson[i*2+1] * 530.f );//vec(421.f,500.f, 0.f);

            vec_t yDown = vec( 0.f, -1.f, 0.f );
            anchor->rayCast( ray, yDown, 500.f, contact, contactNormal ); 

            matrix_t mat1;
            mat1.LookAt( contact, contact + contactNormal, vec(0.f, 1.f, 0.f) );

            matrix_t resm = scaleIt * matRot * mat1 ;
            int batHeight = ( 3 + (fastrand()&7) );
            buildingSpots.push_back( env4building( resm, batHeight ) );

            world.BuildCylindricalBuilding( resm, batHeight, fastrand(), GetRandomSentence().c_str(), vec( 0.5f + r01()*0.5f, 0.5f + r01()*0.5f, 0.5f + r01()*0.5f, 0.7f ) );
        }
        // add links
        matrix_t scaleCon;
        scaleCon.scale(3.f, 3.f, 1.f);
        for (unsigned int i=0;i<buildingSpots.size();i++)
        {
            for (unsigned int j=i+1;j<buildingSpots.size();j++)
            {
                float dist = distance( buildingSpots[i].mat.position, buildingSpots[j].mat.position );
                if ( dist < 180.f && (fastrand()&1) )
                {
                    int maxh = (buildingSpots[i].height<buildingSpots[j].height)?buildingSpots[i].height:buildingSpots[j].height;
                    maxh = (fastrand()%maxh);
                    vec_t v1 = buildingSpots[i].mat.position + buildingSpots[i].mat.up * (5.3f * maxh );
                    vec_t v2 = buildingSpots[j].mat.position + buildingSpots[j].mat.up * (5.3f * maxh );
                    world.BuildCylindricalConnection( v1, v2, 0, scaleCon );
                    
                }

            }
        }
        */
    }
    // pub

	LOG("TrackSetupGuenin\n");
	
    	
	// fill the slices

	LOG("FillTopologyWithSequence\n");
	// startup
	topo.FillTopologyWithSequence( false, &shapeSequences[1], 0, 16 );
    topo.FillTopologyWithSequence( true, &shapeSequences[1], 0, 16 );
    if (pTrack->mEnvironmentIndex == 3)
    {
        topo.FillTopologyWithSequence( false, &shapeSequences[0], 16, topo.mCurrentToopology[0].size()-16 );
        topo.FillTopologyWithSequence( true, &shapeSequences[0], 16, topo.mCurrentToopology[0].size()-16 );
    }

    if (!pTrack->mbOnlyDefaultBrick)
	for (int k=0;k<2;k++)
	{
		bool bFourche = (k==1);
		std::vector<TrackTopology::trackTopology_t>& vv = topo.mCurrentToopology[k];

		// tunnels
		unsigned int slstart = 0, slcount;
		do
		{
			topo.GetBiggestContinuousCount(TrackTopology::TOPO_INGROUND, true, bFourche, slstart, slcount);
			if (slcount)
				topo.FillTopologyWithSequence( bFourche, &shapeSequences[0], slstart, slcount);
			slstart += slcount;
		}
		while (slcount);

		// hole
		slstart = 0;
		do
		{
			topo.GetBiggestContinuousCount(TrackTopology::TOPO_HOLEINOUT, false, bFourche, slstart, slcount);
			if (slcount)
				topo.FillTopologyWithSequence( bFourche, &shapeSequences[3], slstart, slcount);
			slstart += slcount;
		}
		while (slcount);

        // transparent/ border forced
        if (pTrack->mEnvironmentIndex != 3)
        {
		    slstart = 0;//, slcount;
		    do
		    {
			    topo.GetBiggestContinuousCount( (TrackTopology::TOPO_STRAIGHT|TrackTopology::TOPO_BORDERFORCED), true, bFourche, slstart, slcount);
			    if (slcount > shapeSequences[2].maxLength)
				    slcount = shapeSequences[2].maxLength;
            
			    if (slcount && (slcount > 10 ) )
                {
                    //printf("Trans %d %d\n", slstart, slcount );
				    topo.FillTopologyWithSequence( bFourche, &shapeSequences[4], slstart, slcount);
                }
			    slstart += slcount;
		    }
		    while (slcount);
        }

        


		// transparent
        if (pTrack->mEnvironmentIndex != 3)
        {
		    slstart = 0;
		    do
		    {
			    topo.GetBiggestContinuousCount(TrackTopology::TOPO_STRAIGHT, true, bFourche, slstart, slcount);
			    if (slcount > shapeSequences[2].maxLength)
				    slcount = shapeSequences[2].maxLength;
            
			    if (slcount && (slcount > 10 ) )
                {
                    //printf("Trans %d %d\n", slstart, slcount );
				    topo.FillTopologyWithSequence( bFourche, &shapeSequences[2], slstart, slcount);
                }
			    slstart += slcount;
		    }
		    while (slcount);
        }
		
	
	    // transparent pylones
		if (bFourche)
			continue;
		
		//float topoPieceLen = pTrack->trackLength / (float)(topo.mCurrentToopology[0].size());
        unsigned int topoSize = topo.mCurrentToopology[0].size();
       
		for (unsigned int i = 0 ; i < topoSize ; )

		{
			if ( (vv[i].trackSliceProperties&TrackTopology::TOPO_HOLE) ||
				(vv[i].trackSliceProperties&TrackTopology::TOPO_INGROUND) )
			{
				i++;
				continue;
			}

            //
			int idx = i;
			matrix_t mt = mats1[idx];
            
            // desert
            /*
            if ( pTrack->mEnvironmentIndex == 2 )
            {
                i++;
                continue;
            }
            */
            // space
            if ( pTrack->mEnvironmentIndex == 3 )// && ( mats1[idx].position.length()> 530.f) )
            {
                i++;
                continue;
            }
            // space
            if ( mats1[idx].up.y<0.95f)
            {
                i++;
                continue;
            }
			mt.position = vec( 0.f, 0.f, 0.f, 1.f );
			mt.orthoNormalize();

			int pylonType = 0;
			vec_t decalPylonUp = vec(0.f, -0.8f, 0.f);

			if (vv[i].trackSliceProperties&TrackTopology::TOPO_HOLEINOUT)
				decalPylonUp = vec(0.f, -2.f, 0.f);

			if (vv[i].trackSliceProperties&TrackTopology::TOPO_TRANSPARENT)
			{
				pylonType = 2;
				i+= 6;
			}
			else
			{
				i+=5;
			}
            // is there a building pylon around
            bool bShouldDisappear = false;

			vec_t pylonBase = vec(mats1[idx].position.x, -1.f, mats1[idx].position.z);
            vec_t pylonHigh = mats1[idx].position + decalPylonUp;

            //float maxHeight = 

            if ( pylonHigh.y > 150.f )
            {
                bShouldDisappear = true;
                //goto pylonEnd;
            }

            // go ahead

            // check intersection
            
            //const trackSeg_t *pSegs = pTrack->segs;
            if ( !bShouldDisappear )
            {
                for ( unsigned int topoIdx = 0 ; topoIdx < topoSize ; topoIdx++ )
                {
                    const matrix_t& cmt1 = mats1[ topoIdx ];
                    const matrix_t& cmt2 = mats1[ (topoIdx+1)%topoSize ];
                    vec_t resCol;
                    vec_t seg1 = cmt1.position;
                    vec_t seg2 = cmt2.position;
                    seg1.y = seg2.y = -1.f;
                    if ( CollisionClosestPointOnSegment( pylonBase, seg1, seg2, resCol  ) )
                    {
                        float lensq = (resCol-pylonBase).lengthSq();
                        if ( ( lensq > 1.f) &&  ( lensq < (12.f*12.f) ) )
                        {
                            float minSegHeight = zmin( cmt1.position.y, cmt2.position.y );
                            if ( minSegHeight < pylonHigh.y )
                            {
                                i++;
                                //continue;
                                bShouldDisappear = true;
                                goto pylonEnd;
                            }
                        }
                    }
                }
            }

            //
pylonEnd:
            if ( !bShouldDisappear )
            {
			    mesh_t *pylon = world.BuildPylone( pylonBase, 
				    pylonHigh, mt, pylonType, vv[i%vv.size()].width );
                //track.pushProgress( (float)(i) * topoPieceLen, pylon, pylonBase );

                if (bShouldDisappear)
                {
                    pylon->color = vec(1.f, 0.f, 0.f, 1.f);
                }
            }
		}
	}
    LOG("Done\n");
	// startup pylons

	if ( GGame )
	{
		matrix_t sc, tr, rt;
		rt.rotationY( PI );
		sc.scale( vec( 1.75f, 1.5f, 1.5f ) );
		
		
		matrix_t stpm[4];
		int stpscp = 4;
        
        int indexStart = 3;
        tr.translation( vec( psi1[2].width+.5f, 0.f, 0.f ) );
		stpm[0] = sc*rt*tr*mats1[ indexStart ];
        
        int idxpp = indexStart + stpscp;
        tr.translation( vec( psi1[idxpp].width+.5f, 0.f, 0.f ) );
		stpm[1] = sc*rt*tr*mats1[ idxpp  ];
        
        idxpp = indexStart + stpscp *2;
        tr.translation( vec( psi1[idxpp].width+.5f, 0.f, 0.f ) );
		stpm[2] = sc*rt*tr*mats1[ idxpp ];

        idxpp = indexStart + stpscp *3;

        tr.translation( vec( psi1[idxpp].width+.5f, 0.f, 0.f ) );
		stpm[3] = sc*rt*tr*mats1[ idxpp ];
		GGame->InitStartupPylons( stpm );
	}
	LOG("Pylons done\n");

    randomizeAdvertScreens();
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void CreateTankerOverCity( const vec_t& center, float r, float R, float d, float timeFactor )
{
    //mesh_t *groundcity = getSprite(7)->clone();

    mesh_t *groundcity = GetTankerMesh( )->clone();
    groundcity->computeBSphere();
    groundcity->mWorldMatrix.scale(2.f);//
    groundcity->mWorldMatrix.position = vec(0.f, 50.f, 0.f,1.f);
    groundcity->updateWorldBSphere();
    groundcity->visible = true;
    groundcity->color = vec(1.f, 1.f, 1.f, 1.0f );


    meshAnimation_t *pa = groundcity->addAnimation();
    pa->mDuration = 9999999.f;
    /*
    pa->mDrunkAnim = true;
    pa->mDrunkTranslateScale = 10.f;
    */
    pa->setEpitrochoid( r, R, d, center );
    pa->mLocalTime = r01()*5.f;
    pa->mTimeFactor = timeFactor;

    Renderer::setRenderMeshStack(GetMeshStack());

    float scale = 4.f;
    matrix_t screenScale;
    screenScale.scale( 21.f*1.5f * scale, 9.f*1.5f * scale, 1.f );
    
    for (int i = 0;i<2;i++)
    {
        mesh_t*screenMesh = new mesh_t;
        screenMesh->createScreenMesh( vec( 0.f,0.f, 1.f, 1.f ) );

        //matrix_t screenMat
        matrix_t screenlook;
        if (i)
            screenlook.LookAt( vec(47.f,-25.f, 5.f), vec(48.f, -25.50f, 5.f), vec(0.f,1.f, 0.f ) );
        else
            screenlook.LookAt( vec(-26.f,-25.f, 50.f), vec(-27.f, -25.50f, 50.f), vec(0.f,1.f, 0.f ) );

        screenMesh->mBaseMatrix = screenScale * screenlook;// * screenMat;
        screenMesh->mWorldMatrix = screenMesh->mBaseMatrix;
        screenMesh->updateWorldBSphere();
        screenMesh->visible = true;


        screenMesh->color = vec( 1.f, 1.f, 1.f, 0.5f );
        screenMesh->screenMesh = fastrand()&3;


        meshAnimation_t *pa2 = screenMesh->addAnimation();
        *pa2 = *pa;
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// cut scenes
/*
typedef struct cutScenePlan_t
{
    vec_t eyeStart, dirStart;
    vec_t eyeEnd, dirEnd;
    float duration; // seconds
    float smoothEdge1, smoothEdge2;
    vec_t velvetColor; // alpha == 0 -> no velvet
    float DOFStrength, DOFFocalDistance;
    float trackProgressStart;
    u8 cameraMode;
    u8 indexOnEscKey;
    //d on key(X)/goto_index, camera mode(0 = no change, 1=menu track observe), trackprogress value (-1 = not set)
}cutScenePlan_t;

typedef struct cutScene_t
{
    int nbPlans;
    cutScenePlan_t *plans;
} cutScene_t;
*/

cutScenePlan_t introArctic[]={
    // arctic
    // avance tete en bas
    {vec(926.7801f, 15.0245f, -993.3340f), vec(-0.0104f, 0.9999f, 0.0030f),  vec(613.9367f, 15.0245f, -696.9808f), vec(0.5959f, 0.7569f, -0.2684f), 5.f, 0.0f, 0.0f, vec(0.f, 0.f, 0.f, 1.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    // leve la tete
    {vec(613.9367f, 15.0245f, -696.9808f), vec(0.5959f, 0.7569f, -0.2684f), vec(495.8740f, 15.0245f, -597.6476f), vec(0.6614f, -0.2294f, -0.7140f), 1.0f, 0.0f, 0.0f, vec(0.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    { vec(495.8740f, 15.0245f, -597.6476f), vec(0.6614f, -0.2294f, -0.7140f), vec(495.8740f, 15.0245f, -597.6476f), vec(0.6614f, -0.2294f, -0.7140f), 6.f, 0.25f, 0.75f, vec(1.f), 0.f, 0.f, -1.f, 0, 0xFF, MENU_MAIN}
};

cutScenePlan_t introCity[]={
    /* city*/
    {vec(-1294.9888f, 18.491186f, -2250.1328f), vec(-0.0104f, 0.9999f, 0.0030f),  vec(-1300.f, 50.f, -1800.f), vec(0.5959f, 0.7569f, -0.2684f), 5.f, 0.0f, 0.0f, vec(0.f, 0.f, 0.f, 1.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    // leve la tete
    {vec(-1300.f, 50.f, -1800.f), vec(0.5959f, 0.7569f, -0.2684f), vec(-1307.5820f, 79.372604f, -1468.9442f), vec(0.6614f, -0.2294f, -0.7140f), 1.0f, 0.0f, 0.0f, vec(0.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    { vec(-1307.5820f, 79.372604f, -1468.9442f), vec(0.6614f, -0.2294f, -0.7140f), vec(-1307.5820f, 79.372604f, -1468.9442f), vec(0.6614f, -0.2294f, -0.7140f), 6.f, 0.25f, 0.75f, vec(1.f), 0.f, 0.f, -1.f, 0, 0xFF, MENU_MAIN}
};

cutScenePlan_t introDesert[]={
    // desert
    {vec(-447.8617f, 336.6251f, 431.4624f), vec(-0.3112f, 0.8751f, 0.3706f),  vec(-271.2024f, 353.0278f, 237.2300f), vec(-0.2722f, 0.9376f, 0.2165f), 5.f, 0.0f, 0.0f, vec(0.f, 0.f, 0.f, 1.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    // leve la tete
    {vec(-271.2024f, 353.0278f, 237.2300f), vec(-0.2722f, 0.9376f, 0.2165f), vec(-258.8556f, 326.9317f, 235.0837f), vec(-0.8940f, -0.2806f, 0.3492f), 1.0f, 0.0f, 0.0f, vec(0.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    {vec(-258.8556f, 326.9317f, 235.0837f), vec(-0.8940f, -0.2806f, 0.3492f),vec(-258.8556f, 326.9317f, 235.0837f), vec(-0.8940f, -0.2806f, 0.3492f), 6.f, 0.25f, 0.75f, vec(1.f), 0.f, 0.f, -1.f, 0, 0xFF, MENU_MAIN}
};

cutScenePlan_t introSpace[]={
    //space
    {vec(926.7801f, 15.0245f, -993.3340f), vec(-0.0104f, 0.9999f, 0.0030f),  vec(613.9367f, 15.0245f, -696.9808f), vec(0.5959f, 0.7569f, -0.2684f), 5.f, 0.0f, 0.0f, vec(0.f, 0.f, 0.f, 1.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    // leve la tete
    {vec(613.9367f, 15.0245f, -696.9808f), vec(0.5959f, 0.7569f, -0.2684f), vec(495.8740f, 15.0245f, -597.6476f), vec(0.6614f, -0.2294f, -0.7140f), 1.0f, 0.0f, 0.0f, vec(0.f), 0.f, 0.f, -1.f, 0, 2, MENU_NONE},
    { vec(495.8740f, 15.0245f, -597.6476f), vec(0.6614f, -0.2294f, -0.7140f), vec(495.8740f, 15.0245f, -597.6476f), vec(0.6614f, -0.2294f, -0.7140f), 6.f, 0.25f, 0.75f, vec(1.f), 0.f, 0.f, -1.f, 0, 0xFF, MENU_MAIN}
};

cutScene_t introSceneArctic = { sizeof(introArctic)/sizeof(cutScenePlan_t), introArctic };
cutScene_t introSceneCity = { sizeof(introCity)/sizeof(cutScenePlan_t), introCity };
cutScene_t introSceneDesert = { sizeof(introDesert)/sizeof(cutScenePlan_t), introDesert };
cutScene_t introSceneSpace = { sizeof(introSpace)/sizeof(cutScenePlan_t), introSpace };

static float GLocalPlaybackTime = 0.f;
static const cutScene_t* GCurrentCutScene = NULL;
static unsigned int GSequencePlanIndex = 0;

void PlaySequence( const cutScene_t& seq )
{

    GLocalPlaybackTime = 0.f;
    GSequencePlanIndex = 0;
    GCurrentCutScene = &seq;
    camera.SetCameraCustom();
}

bool SequenceIsBeingPlayed()
{
    return ( GCurrentCutScene != NULL );
}

void StopSequencePlayback()
{
    GCurrentCutScene = NULL;
}

void TickSequencePlayback( float aTimeEllapsed )
{
    if ( GCurrentCutScene )
    {
        bool bApplyPlanSettings = false;

        // 1st plan settings
        if ( GLocalPlaybackTime < FLOAT_EPSILON && GSequencePlanIndex == 0 )
            bApplyPlanSettings = true;

        GLocalPlaybackTime += aTimeEllapsed;

        
        u8 planOnEscKey = GCurrentCutScene->plans[GSequencePlanIndex].indexOnEscKey;
        if ( keysReleased( SDLK_ESCAPE) && planOnEscKey != 0xFF )
        {
            GSequencePlanIndex = planOnEscKey;
            GLocalPlaybackTime = 0.f;
            bApplyPlanSettings = true;
        }
        const float planDuration = GCurrentCutScene->plans[GSequencePlanIndex].duration;
        if ( GLocalPlaybackTime >= planDuration )
        {
            // next and qui playback
            GSequencePlanIndex ++;
            GLocalPlaybackTime -= planDuration;
            if ( GSequencePlanIndex >= GCurrentCutScene->nbPlans )
            {
                GCurrentCutScene = NULL;
                return;
            }
            bApplyPlanSettings = true;
        }

        if ( bApplyPlanSettings )
        {
            // velvet
            const cutScenePlan_t& plan = GCurrentCutScene->plans[GSequencePlanIndex];
            if ( plan.velvetColor.w > FLOAT_EPSILON)
            {
                Renderer::newVelvet( plan.velvetColor, 0 );
				//
				ggui.putText(46, 8,"Cedric Guillemet");
				ggui.putText(46, 7,"presents");
            }
            // menu
            if (plan.menuToSet != MENU_NONE)
            {
				ggui.clearText();
                Audio::PlaySound( Audio::GetSound("Datas/Sounds/droid/WelcomeToTheRush.ogg"), AUDIO_GROUP_GUI );
                Menus::Show( MENU_MAIN );
            }
        }
        const cutScenePlan_t& plan = GCurrentCutScene->plans[GSequencePlanIndex];
        // interp

        float interp = ( GLocalPlaybackTime/plan.duration );
        if ( plan.smoothEdge1> FLOAT_EPSILON || plan.smoothEdge2 > FLOAT_EPSILON) 
            interp = smootherstep( plan.smoothEdge1, plan.smoothEdge2, interp );

        vec_t currentEye, currentDir;
        currentEye.lerp( plan.eyeStart, plan.eyeEnd, interp );
        currentDir.lerp( plan.dirStart, plan.dirEnd, interp );

        camera.view( currentEye, currentEye - currentDir, vec( 0.f, 1.f, 0.f ) );
        /*
        setDOFFocus( LERP( float aFocus);
        setDOFBlur(float aBlur);
        float DOFStrength, DOFFocalDistance;
        Renderer::setd
        */
        
    }
}



static const char* adjectifs[]={ "nano", "epic", "e", "1337", "super", "uber", "no", "beautifull", "fast", "smart", "I\03", "hi", "high", "my_","win","speciAl","AAA","awesome","great","amazing","unbelievable","terrific","indie"};

static const char* noms[] = { "l0ve", ".the rush//", "Cedric Guillemet", "life", "space", "design", "impact", "race", "respect", "Xperience", "rush", "Sensation",
"Rednalhgih", "Skaven", "Ship", "Money", "Celebrity", "girl",
"Song", "game" };

std::string GetRandomSentence()
{
	std::string res = adjectifs[fastrand()%(sizeof(adjectifs)/sizeof(const char*))];
	res += " ";
	res += noms[fastrand()%(sizeof(noms)/sizeof(const char*))];

    if (res.empty())
    {
        ASSERT_GAME(0);
    }

	return res;
}
static const char *botNames[]={
"August Oannes","Lasseya Arlos","Corana Mrrgwharr","Seit Sunfell","Roy Zatoq",
"Shivas Beelen","Damien Aban","Vaj Bahol","Rangu Keel",
"Athala Pavish","Lisa Reed","Lesher Reed","Logra Talonspyre",
"Rob Hardin","Jens Droma","Jope Bela-Trepada","Minno Dol",
"Thoja Crow","Tulan Silero","Khorde Chi","Irizon Bain",
"Abav Parlayy","Noval Zarbo","J.P. Donos","Jorg Vandorack",
"Mena Styles","Lana Charr","Henri Oligard","Indiglo Hooge",
"Julia Tonor","Keth Irimore","Doja Dyer","Darin Greenback",
"Joshua Jerikko","Loretta  Tachi","Herub Leuf","Joti Damar",
"Kalira Lars","Mikal Renning","Mal Varm","Nomar Trite",
"Ait Vapasi","John Filtayn","Vyrim Gregory","Zeno Thrall",
"Palaras Amadis","Sei Xen","Montross Sa-Vin","Hak Hocha",
"Huff Brie","Benny Arcturus","Neo Paul","Ben Tucker",
"Miwa Toarinar","Gramor Formoe","Rath Terrik","Rizann Walker",
"Raihane Richardson","Megara Davout","Bic  Orailus","Wain Cass",
"Mando Panne","Gadon Vox","Kraton Zamba","Jalek Flynn",
"Kennex H'darr","Markus Greeta","Vifi Kiyanu","Mikal Vel Aath",
"Caralho Riv","Geopin Rhane","Telon Mishkoll","Aklee Cahob",
"Loman Bintaghr","Ral Mackali","Stella Kobylarz","Thes Logan",
"Groms Magnus","Xoquon Fodao","Calaveylon  Kasra","Hersh Voddher",
"Nicole Joyriak","Wor Verbenti","Rellius Tierney","Arash  Gunstar",
"Illusian Odai","Relain Arcaina","Lando Thashin","Crevil  Jien",
"Zaphone Saleem","Amseth Starfall","Erik Asto","Auros Tallon",
"Oret Tanoor","Tarrex Shodon","Thaneo Paesante","Kista Turnell",
"Yun Crane","Gaen Claermoore","Pimmelolymp Alucard"};

const char* GetRandomBotName()
{
	return botNames[rand() % (sizeof(botNames) / sizeof(const char *) ) ];
}


// Sounds

void LoadSounds()
{
    PROFILER_START(LoadSounds);
    Audio::ClearSounds();    
    // preload .ogg
    const char *oggsMenu[] = {
        // GUI
        "ButtonAsk","ButtonError","ButtonNextPrev","ButtonBack","ButtonLocked","ButtonValid"
    };
    // droid
    const char *oggsDroid[] = {
    "Congratulations", "DestructionIn", "DestructionShield", "Disengagement", "DisengagementInTwoSeconds", 
    "FinalLap", "GravityGrip", "HomingMissile", "Loser", "MachineGun", "Mines", "Missile", "MoreEnergy", 
    "One", "ProtectionShield", "TeslaHook", "Three", "ThreeMissiles", "Two", "WarningIncomingMissile", 
    "WarningLowEnergy", "WarningMines", "YourShipHasBeenEliminatedThankYou", "autopilot", "booster", 
    "eight", "fifteen", "five", "four", "go", "nine", "seconds", "seven", "six", "ten", "thirty", "twenty", 
        "twentyFive", "zero","EvolutionUnlocked", "SpeedThresholdReached", "WelcomeToTheRush" };
    // fx
    const char *oggsFX[] = {
    "Electric", "engines", "GravityGripFX", "explode", "MissileAlarm", "flamme_bruleur", "MissileLaunch", "forge_outil",
    "ShieldLoop", "gotColor", "ShieldOff", "grincement", "ShieldOn", "repair", "Shoot", "ricochet", "WallCollisionDefault", "trackSpeed",
    "dropmine", "tracking", "BeepStartOne", "BeepStartTwo", "releasecolors", "switchcolor", "getcolor", "BulletHitArmor" };
 
    for (int i = 0 ; i < sizeof(oggsMenu)/sizeof(const char*);i++)
        Audio::GetSound( (std::string("Datas/Sounds/menu/") + oggsMenu[i] + ".ogg").c_str() );

    for (int i = 0 ; i < sizeof(oggsDroid)/sizeof(const char*);i++)
        Audio::GetSound( (std::string("Datas/Sounds/droid/") + oggsDroid[i] + ".ogg").c_str() );

    for (int i = 0 ; i < sizeof(oggsFX)/sizeof(const char*);i++)
        Audio::GetSound( (std::string("Datas/Sounds/fx/") + oggsFX[i] + ".ogg").c_str() );
    
    PROFILER_END(); // LoadSounds
}
