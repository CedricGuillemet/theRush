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

#ifndef WORLD_H__
#define WORLD_H__


struct mesh_t;


class Height2D
{
public:
	Height2D(int aWidth, int aHeight);
    ~Height2D();

	void Box(int x, int y, int width, int height, u8 value);
	void Circle(int x, int y, int radius, u8 value);
	void Noise(int strength);
	void Stars(float min, float max, unsigned int count );
	void SmoothStep(float edge0, float edge1);

    // 0 to 1
    void ContrastBrightness( float contrast, float brightness);

    // sky box composite
    static void SkyAddPlanet( u32 *pSrc, u32 *pDest );

    // blur
	void HorzBlur(int strength);

	void VertBlur(int strength);

	//factors are <<8. 256 means 1.f
	void ApplyKernel(int *factors, int kernelWidth);

	void Save(const char *szName);

	void FloeDisk( int x, int y, int radius, float height, float amp, int seed, float thresholdAdd, float thresholdFactor);
    void PerlinMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace = 0, float freq = 4.f );
    void SimplexMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace = 0, float freq = 4.f );
    
    void ComputeNormalMap( u32* nrmBits, float bumpHeightScale );

    void MaxxedBy( Height2D &hg2 );
	void SmoothCliff();
    void Erode(float smoothness);

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }
	const u8* GetBits() const { return mBits; }

protected:
	int mWidth, mHeight;
	u8 *mBits;
};

class World
{
public:
	World();
	~World() {}

	// cylindrical city // ideal : 18chars pour le texte
    void BuildCylindricalBuildingConcrete();
    void BuildCylindricalBuilding(const vec_t& position, int nbHeightElements, int capElement, const char* szText = NULL);
	void BuildCylindricalBuilding(const matrix_t& mat, int nbHeightElements, int capElement, const char* szText = NULL, const vec_t& glassColor = vec(1.f, 0.5f, 0.157f, 0.5f) );
	void BuildCylindricalConnection(const vec_t& position1, const vec_t& position2, int floorNumber, const matrix_t& basem );

    // game speed
	void SetGameSpeed(float aSpeed) { mSpeed = aSpeed; }
	float GetGameSpeed() const { return mSpeed; }

	// pylones
	mesh_t * BuildPylone(const vec_t& pos, const vec_t& posHigh, const matrix_t& matRot, int aStyle, float aBranchWidth);
    mesh_t * BuildadPylon(const matrix_t & origin, float height, float width, float depth, float anchorWidth, float anchorHeight, float pylonWidth, matrix_t& screenMatrix );

	// voxel terrain
	void InitVoxelTerrain(int sizeX, int sizeY, int sizeZ);
    void ClearVoxelDatas();

	void AddHeight2D(const vec_t& origin, const vec_t& size, const Height2D& height);
	void VoxelSphere(vec_t&sphere, bool bFill = true, const vec_t& axisScaleSquared = vec( 1.f ) );
	mesh_t *MakeVoxelMesh( float displacement = 0.f );

    bool IsVoxelDatasAvailable() const { return (voxels!=NULL); }
    bool IsThereVoxelAtWorldPos( const vec_t& worldPos );
    void DownsampleVoxel( int step );
	bool VoxelPeek( const vec_t& pos );
    void VoxelPoke( const vec_t& pos, bool bFill );
    void VoxelPoke( int ipx, int ipy, int ipz, bool bFill );

    void SetWorldToVoxelConversionParameters( const vec_t& atrMin, float atransLen ) { trMin = atrMin; transLen = atransLen; }
    vec_t WorldToVoxel( const vec_t& v ) const { vec_t hole0 = v; hole0 -= trMin; hole0 *= transLen; return hole0; }

    // ads
    void AddBuildingAd( const matrix_t& buildMat, float scale );
    void AddCentralAd( int aiPoint );
    void AddSideAd( int aiPoint, bool bLeft, int cutCount );

protected:
	float mSpeed;

    // voxel
    u8 *voxels;
    int mVoxSizeX, mVoxSizeY, mVoxSizeZ;
    

    bool voxPeek(int x, int y, int z);
    void voxSet(int x, int y, int z);
    void voxUnset(int x, int y, int z);

    // buildings
#define NBHATS 5
    mesh_t *concrete, *glass ;
    mesh_t *hats[NBHATS];
    mesh_t *antena;
    mesh_t *conn, *connMid ;
    
    // world to voxel
    vec_t trMin;
    float transLen;
};

extern World world;

#endif

