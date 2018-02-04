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
#include "world.h"
#include "mesh.h"
#include "track.h"
#include "render.h"

#include "stb_image_write.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

World world;

World::World() 
{ 
    voxels = NULL;
	mSpeed = 1.f;

    concrete = NULL;
    glass = NULL;
    conn = NULL;
    connMid = NULL;
}


void World::BuildCylindricalBuildingConcrete()
{
    matrix_t mlower;
    mlower.scale(128,20,128);
    matrix_t uncenter;
    uncenter.translation(0,10,0);

    // cyl1
    mesh_t* cyl1 = generateCylinder(32);
    transformMesh(cyl1, mlower * uncenter);

    // cyl2
    mesh_t* cyl2 = generateCylinder(32);
    matrix_t trup;
    trup.translation(0,32,0);
    transformMesh(cyl2, mlower * uncenter * trup);


    // merge 1&2
    mesh_t *res = merge(cyl1, cyl2);
    delete cyl1;
    cyl1 = NULL;
    delete cyl2;
    cyl2 = NULL;

    matrix_t rescale;
    rescale.scale(0.1f, 0.1f, 0.1f);
    transformMesh(res, rescale);

    //set it
    res->color = vec(0.5f,0.5f,0.5f,0.5f);
    res->computeBSphere();
    res->visible = false;
    concrete = res;

    // cyl3
    matrix_t glassmat, mlower2;
    glassmat.translation(0, 26, 0);
    mlower2.scale(122, 12, 122);
    mesh_t* cyl3 = generateCylinder(32);
    transformMesh(cyl3, mlower2 * glassmat * rescale);

    cyl3->color = vec(1.f, 0.5f, 0.157f, 0.5f);
    cyl3->computeBSphere();
    cyl3->visible = false;
    glass = cyl3;

    // chapeau 1

    mlower.scale(128,30,128);
    uncenter.translation(0,15,0);
    hats[0] = generateCylinder(32, true);
    transformMesh(hats[0], mlower * uncenter);
    squiz(hats[0], vec(0.8f,1.f,0.8f), 1);
    transformMesh(hats[0], rescale);

    // chapeau 2
    float cubesz = floorf(r01() * 32.f) + 32.f;

    mlower.scale(cubesz,cubesz,cubesz);
    uncenter.translation(8.f,cubesz*0.5f,-13.f);
    hats[1] = generateBox();
    transformMesh(hats[1], mlower * uncenter * rescale);

    // chapeau 3
    mlower.scale(100,50,100);
    uncenter.translation(0,25,0);
    hats[2] = generateCylinder(32, true);
    transformMesh(hats[2], mlower * uncenter);
    transformMesh(hats[2], rescale);

    // chapeau 4 // hemisphere
    mlower.scale(128,128,128);
    uncenter.translation(0,-2,0);
    hats[3] = generateSuper(0.5f, 0.5f, 0.5f,
        4,
        true, false, true);

    transformMesh(hats[3], mlower * uncenter );
    transformMesh(hats[3], rescale);

    // chapeau 5 // squiz + sphere
    mlower.scale(128,18,128);
    uncenter.translation(0,9,0);
    hats[4] = generateCylinder(32, true);
    transformMesh(hats[4], mlower * uncenter);
    squiz(hats[4], vec(0.8f,1.f,0.8f), 1);


    mlower.scale(128 * 0.8f, 128* 0.8f, 128 * 0.8f);
    uncenter.translation(0,18,0);
    mesh_t *sph = generateSuper(0.5f, 0.5f, 0.5f,
        4,
        true, false, true);

    transformMesh(sph, mlower * uncenter);
    mesh_t *h = merge(sph, hats[4]);
    delete sph;
    delete hats[4];
    transformMesh(h, rescale);
    hats[4] = h;
    //
    for (int i = 0;i<NBHATS;i++)
    {
        hats[i]->mLocalMatrix.identity();
        hats[i]->color = vec( 0.4f, 0.4f, 0.4f, 0.5f);
        hats[i]->computeBSphere();
        hats[i]->visible = false;
    }


    // antenna
    mlower.scale(1.f, 200.f, 1.f);
    uncenter.translation(0.f,50.f, 0.f);
    antena = generateBox();
    transformMesh(antena, mlower * uncenter * rescale);
    antena->color = vec(0.f, 0.f, 0.f, 0.5f);
    antena->computeBSphere();


    // connection
    {
	matrix_t recal0, rot0, rot1, rot12, decalUp;

	recal0.translation(0,0.5f, 0.f);
	rot0.rotationX(PI*0.5f);
	rot1.rotationY( (PI/16.f)*8.5f );
	rot12.rotationY( ((PI/16.f)*8.5f) + PI );
	decalUp.translation(0.f, 0.06f, 0.f);

	// cyl1
    ASSERT_GAME( cyl1 == NULL );
	cyl1 = generateCylinder(32, true, 3, 19);
	transformMesh(cyl1, rot1 * recal0 * rot0);	

	// cyl2
    ASSERT_GAME( cyl2 == NULL );
	cyl2 = generateCylinder(32, true, 3, 19);
	transformMesh(cyl2, rot12 * recal0 * rot0 * decalUp);	

	// merge
	conn = merge(cyl1, cyl2);
    //FIXME: should cyl1 and cyl2 be deleted?

	// middle
	connMid = generateBox();
	transformMesh(connMid, recal0 * rot0);

	// set std props

	conn->computeBSphere();
	connMid->computeBSphere();
	connMid->visible = false;
	connMid->mLocalMatrix.identity();
	conn->color = vec(1,1,1,0.5f);
	connMid->color = vec(1.f, 0.5f, 0.157f, 0.5f);
	conn->visible = false;
    }
}

void World::BuildCylindricalBuilding(const vec_t& position, int nbHeightElements, int capElement, const char* szText)
{
    matrix_t tr;
    tr.translation( position );
    BuildCylindricalBuilding( tr, nbHeightElements, capElement, szText );
}

void World::BuildCylindricalBuilding(const matrix_t& mat, int nbHeightElements, int capElement, const char* szText, const vec_t& glassColor )
{

	matrix_t hupSide;
	for (int i = 0;i<nbHeightElements;i++)
	{
		
		hupSide.translation(0.f, 0.f + 5.3f*(float)(i), 0.f);

		mesh_t *instanceConcrete = concrete->clone();
		instanceConcrete->mWorldMatrix = hupSide * mat;//.identity();
		instanceConcrete->visible = true;
		instanceConcrete->updateWorldBSphere();

        //FIXME: who holds a reference to 'instanceConcrete'?
        //Its memory could have to be freed later on

		mesh_t *instanceGlass = glass->clone();
		instanceGlass->mWorldMatrix = hupSide * mat;//.identity();
		instanceGlass->visible = true;
        instanceGlass->color = glassColor;
		instanceGlass->updateWorldBSphere();

        //FIXME: who holds a reference to 'instanceGlass'?
        //Its memory could have to be freed later on
	}
	hupSide.translation( 0.f, 0.f + 5.3f*(float)(nbHeightElements), 0.f );
	//mesh_t *instanceHat = hats[1]->clone();

	// text anim
	if ( ( (capElement%(NBHATS+1)) == NBHATS ) && szText)
	{

		mesh_t* instanceHat = generateText(szText);
		float tLen = (float)strlen(szText);

        //FIXME: who holds a reference to 'instanceHat'?
        //Its memory could have to be freed later on

		matrix_t mt,sc;
		mt.translation(0.f, 0.2f, 5.f);
		sc.scale(1.f, 0.5f*(16.f / tLen), 1.f);
		transformMesh(instanceHat, mt * sc);
		instanceHat->color = vec(1,0.8f,0.8f,0.7f);
		bendMesh(instanceHat, PI*2.0f, 0, 1);
	

		meshAnimation_t *pa = instanceHat->addAnimation();
		pa->mDuration = 5.f;
	
		pa->mRotationStart = vec(0.f, 0.0f, 0.f);
		pa->mRotationEnd = vec(0.f, -2.f * PI, 0.f);
		pa->mRotationType = 1;
		pa->mRotationInterpolation = 0;




		instanceHat->mBaseMatrix = hupSide * mat;//.identity();
		instanceHat->visible = true;
		instanceHat->updateWorldBSphere();


		instanceHat = hats[2]->clone();
		instanceHat->mWorldMatrix = hupSide * mat;//.identity();
		instanceHat->visible = true;
		instanceHat->updateWorldBSphere();

        //FIXME: who holds a reference to 'instanceHat'?
        //Its memory could have to be freed later on
	}
	else
	{
		if (capElement > -1)
		{
			mesh_t *instanceHat = hats[capElement%NBHATS]->clone();
			instanceHat->mWorldMatrix = hupSide * mat;//.identity();
			instanceHat->visible = true;
			instanceHat->updateWorldBSphere();

            //FIXME: who holds a reference to 'instanceHat'?
            //Its memory could have to be freed later on
		}
	}
	// antena
	if (capElement > -1)
	{
		int nbAntenas = (fastrand()&3);
		for (int i=0;i<nbAntenas;i++)
		{
			matrix_t antenaTrans, antenaScale;
			static const float antenaDropRadius = 8.f;
			antenaTrans.translation(r01() * antenaDropRadius - antenaDropRadius*0.5f,0.f, r01() * antenaDropRadius - antenaDropRadius*0.5f);
			antenaScale.scale(1.f, r01() *0.5f + 0.5f, 1.f);
			mesh_t *instanceHat = antena->clone();
			instanceHat->mWorldMatrix = antenaScale * hupSide * antenaTrans * mat;//.identity();
			instanceHat->visible = true;
			instanceHat->updateWorldBSphere();

            //FIXME: who holds a reference to 'instanceHat'?
            //Its memory could have to be freed later on
		}
	}	
}

void World::BuildCylindricalConnection(const vec_t& position1, const vec_t& position2, int floorNumber, const matrix_t& basem)
{
	float len = (position2-position1).length();

	matrix_t instanceMat1, instanceMat2, lkat;
	vec_t hgt = vec(0.f, 2.45f + 5.3f * (float)floorNumber, 0.f);
	lkat.LookAt(position1 + hgt, position2 + hgt,  vec(0.f, 1.f, 0.f) );
	instanceMat1.scale(5.2f, 5.2f, len);
	instanceMat2.scale(4.2f, 1.8f, len);
	instanceMat1 = instanceMat1 * lkat;
	instanceMat2 = instanceMat2 * lkat;
	
	mesh_t *connCyl = conn->clone();
	connCyl->mWorldMatrix = basem * connCyl->mLocalMatrix * instanceMat1;
	connCyl->updateWorldBSphere();
	connCyl->visible = true;
    connCyl->color = vec(0.5f,0.5f,0.5f,0.5f);

    //FIXME: who holds a reference to 'connCyl'?
    //Its memory could have to be freed later on

	mesh_t *connGlass = connMid->clone();
	connGlass->mWorldMatrix = basem * connGlass->mLocalMatrix * instanceMat2;
	connGlass->updateWorldBSphere();
	connGlass->visible = true;
    connGlass->color = vec( 0.5f + r01()*0.5f, 0.5f + r01()*0.5f, 0.5f + r01()*0.5f, 0.7f );

    //FIXME: who holds a reference to 'connGLass'?
    //Its memory could have to be freed later on
}

mesh_t * World::BuildPylone(const vec_t& pos, const vec_t& posHigh, const matrix_t& matRot, int aStyle, float aBranchWidth )
{
	float totalHeight = (posHigh-pos).y;//length();
	if ( totalHeight < 1.f)
		return NULL;
		
		// type 1
	if ( (aStyle == 0) || (aStyle == 2) )
	{

		float socle1size = 5.f;
		float socle2size = 3.5f;
		float socle3size = 2.f;

		float socle1height = 4.f;
		float branchDeal = aBranchWidth;
		float branchBackFor = (aStyle == 2)?aBranchWidth:0.f;
		float branchHeight = 6.f;

		//float trunkHeight = (posHigh-pos).length() - branchHeight;

		matrix_t matSocle1Scale, matSocle2Scale, matSocle3Scale;
		matSocle1Scale.scale(socle1size, 1.f, socle1size);
		matSocle2Scale.scale(socle2size, 1.f, socle2size);
		matSocle3Scale.scale(socle3size, 1.f, socle2size);


		pushNewMeshInit();

		matrix_t mt1, mt2;

		// socle
		bool bDrawTrunk;
		float localSocleHeight = 0.f, localTrunkHeight = 0.f;
		bool bDrawBranch;
		if (totalHeight<socle1height)
		{
			localSocleHeight = totalHeight;
			bDrawTrunk = false;
			bDrawBranch = false;
		}
		else
		{
			localSocleHeight = socle1height;
			bDrawTrunk = true;
			bDrawBranch = (totalHeight >= (socle1height + branchHeight));
			localTrunkHeight = totalHeight;
			if (bDrawBranch)
			{
				localTrunkHeight -= branchHeight;
			}
		}

		mt1.translation(pos);
		mt2.translation(pos+vec(0.f, localSocleHeight, 0.f));
		pushTrunk( matRot * matSocle1Scale* mt1 , matRot  * matSocle1Scale* mt2 );

		if (bDrawTrunk)
		{
			pushTrunk(  matRot *matSocle1Scale *  mt2, matRot * matSocle2Scale *  mt2);

			// trunk
            vec_t pylDir = normalized(posHigh-pos);
			mt1.translation(pos + pylDir * localTrunkHeight );//vec(0.f, localTrunkHeight, 0.f) );
			pushTrunk(  matRot * matSocle2Scale * mt2, matRot * matSocle2Scale * mt1);
		}

		// anchors
		if (bDrawBranch)
		{
			vec_t locrl;
			locrl = vec(-branchDeal, 0.f, -branchBackFor);
			locrl.TransformVector(matRot);
			mt2.translation(posHigh + locrl);
			pushTrunk(  matSocle2Scale * matRot * mt1, matSocle3Scale * matRot * mt2);

			locrl = vec( branchDeal, 0.f, branchBackFor);
			locrl.TransformVector(matRot);
			mt2.translation(posHigh + locrl );//
			pushTrunk( matSocle2Scale * matRot * mt1, matSocle3Scale * matRot * mt2);

			if (aStyle == 2)
			{
				locrl = vec(-branchDeal, 0.f, branchBackFor);
				locrl.TransformVector(matRot);
			mt2.translation(posHigh + locrl );
			pushTrunk(  matSocle2Scale * matRot * mt1, matSocle3Scale * matRot * mt2);

				locrl = vec( branchDeal, 0.f, -branchBackFor);
				locrl.TransformVector(matRot);
			mt2.translation(posHigh + locrl);
			pushTrunk( matSocle2Scale * matRot * mt1, matSocle3Scale * matRot * mt2);
			}
		}
	}
	else if (aStyle == 1)
	{
		
		float headHeight = 5.f;

		float trunkHeight;
		bool bHasHead = (totalHeight >= headHeight*3.f);

		if (bHasHead)
			trunkHeight = totalHeight - headHeight;
		else
			trunkHeight = totalHeight;

		matrix_t matSocle1Scale, matSocle2Scale, matSocle3Scale;
		matrix_t mt1, mt2, mt3;
		matSocle1Scale.scale(8.f, 1.f, 5.f);
		matSocle2Scale.scale(4.f, 1.f, 1.5f);
		matSocle3Scale.scale(12.f, 1.f, 4.f);


		mt1.translation(pos);
		mt2.translation(pos+vec(0.f, trunkHeight, 0.f));
		mt3.translation(pos+vec(0.f, totalHeight, 0.f));
		pushTrunk( matSocle1Scale *  matRot * mt1, matSocle2Scale *matRot *  mt2 );
		if (bHasHead)
			pushTrunk( matSocle2Scale *matRot *  mt2, matSocle3Scale *matRot *  mt3 );

	}


	mesh_t *mesh = pushNewMesh();
	mesh->visible = true;
	mesh->computeBSphere();
	mesh->mWorldMatrix.identity();
	mesh->updateWorldBSphere();
	mesh->color = vec(1.f, 1.f, 1.f, 0.5f);

	return mesh;
}

mesh_t * World::BuildadPylon(const matrix_t & origin, float height, float width, float depth, float anchorWidth, float anchorHeight, float pylonWidth, matrix_t& screenMatrix )
{
    pushNewMeshInit();

    matrix_t pylonWidthScale;
    pylonWidthScale.scale( pylonWidth );

    bool oneBranch = ( fabsf( width ) < FLOAT_EPSILON );
    float halfPylonWidth = 0.5f * pylonWidth;

    // partie verticale
    matrix_t anc1;
    anc1.translation( vec( 0.f, height, 0.f ) );

    pushTrunk( pylonWidthScale * origin, pylonWidthScale * anc1 * origin, oneBranch );

    // partie horizontale

    if ( !oneBranch )
    {
        matrix_t anc2,anc3;

        anc2.set( vec(0.f, 0.f, -1.f, 0.f), vec(1.f, 0.f, 0.f, 0.f ), vec(0.f, -1.f, 0.f, 0.f), vec( -halfPylonWidth, height-halfPylonWidth, 0.f, 1.f ) );
        anc3.set( vec(0.f, 0.f, -1.f, 0.f), vec(1.f, 0.f, 0.f, 0.f ), vec(0.f, -1.f, 0.f, 0.f), vec( width, height-halfPylonWidth, 0.f, 1.f ) );

        pushTrunk( pylonWidthScale * anc2 * origin, pylonWidthScale * anc3 * origin, true );
    }

    // ancres
    matrix_t anc4, anc5, anc6, anc7, anc8;

    float decalRight = oneBranch?0.f:halfPylonWidth;

    anc4.set( vec(1.f, 0.f, 0.f, 0.f), vec(0.f, 0.f, -1.f, 0.f ), vec(0.f, -1.f, 0.f, 0.f), vec( width-decalRight, height-halfPylonWidth, 0.5f, 1.f ) );
    anc5.set( vec(1.f, 0.f, 0.f, 0.f), vec(0.f, 0.f, -1.f, 0.f ), vec(0.f, -1.f, 0.f, 0.f), vec( width-decalRight-anchorWidth*0.5f, height-0.5f-anchorHeight*0.5f, depth, 1.f ) );
    anc6.set( vec(1.f, 0.f, 0.f, 0.f), vec(0.f, 0.f, -1.f, 0.f ), vec(0.f, -1.f, 0.f, 0.f), vec( width-decalRight-anchorWidth*0.5f, height-0.5f+anchorHeight*0.5f, depth, 1.f ) );
    anc7.set( vec(1.f, 0.f, 0.f, 0.f), vec(0.f, 0.f, -1.f, 0.f ), vec(0.f, -1.f, 0.f, 0.f), vec( width-decalRight+anchorWidth*0.5f, height-0.5f-anchorHeight*0.5f, depth, 1.f ) );
    anc8.set( vec(1.f, 0.f, 0.f, 0.f), vec(0.f, 0.f, -1.f, 0.f ), vec(0.f, -1.f, 0.f, 0.f), vec( width-decalRight+anchorWidth*0.5f, height-0.5f+anchorHeight*0.5f, depth, 1.f ) );
    

    pushTrunk( pylonWidthScale * anc4* origin, pylonWidthScale * anc5 * origin, true );
    pushTrunk( pylonWidthScale * anc4* origin, pylonWidthScale * anc6 * origin, true );
    pushTrunk( pylonWidthScale * anc4* origin, pylonWidthScale * anc7 * origin, true );
    pushTrunk( pylonWidthScale * anc4* origin, pylonWidthScale * anc8 * origin, true );
    
    matrix_t screenAnc;
    screenAnc.set( vec(1.f, 0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f, 0.f ), vec(0.f, 0.f, 1.f, 0.f), vec( width-decalRight, height-halfPylonWidth, depth, 1.f ) );
    screenMatrix = screenAnc * origin;

    // build mesh

	mesh_t *mesh = pushNewMesh();
	mesh->visible = true;
	mesh->computeBSphere();
	mesh->mWorldMatrix.identity();
	mesh->updateWorldBSphere();
	mesh->color = vec(1.f, 1.f, 1.f, 0.5f);

	return mesh;
}



void World::InitVoxelTerrain(int sizeX, int sizeY, int sizeZ)
{
	ClearVoxelDatas();
    ASSERT_GAME( voxels == NULL );

	int voxSize = (sizeZ * sizeY * sizeX)>>3;
	voxels = new u8 [voxSize];
	memset(voxels, 0, sizeof(u8) * voxSize);

	mVoxSizeX = sizeX;
	mVoxSizeY = sizeY;
	mVoxSizeZ = sizeZ;
}

void World::ClearVoxelDatas()
{
    if (voxels)
    {
        delete [] voxels;
        voxels = NULL;
    }
}

bool World::voxPeek(int x, int y, int z)
{
	if (x<0 || x >=mVoxSizeX || y <0 || y>mVoxSizeY || z<0 || z >=mVoxSizeZ)
		return false;
	int slice = (mVoxSizeY*(mVoxSizeX>>3));

	int bit = x&7;
	int btMask = 1<<bit;
	return ( (voxels[z * slice + y * (mVoxSizeX>>3) + (x>>3) ] & btMask) != 0);
}

void World::voxSet(int x, int y, int z)
{
	if (x<0 || x >=mVoxSizeX || y <0 || y>mVoxSizeY || z<0 || z >=mVoxSizeZ)
		return;

	int slice = (mVoxSizeY*(mVoxSizeX>>3));

	int bit = x&7;
	int btMask = 1<<bit;
	
	int putit = z * slice + y * (mVoxSizeX>>3) + (x>>3) ;

	voxels[putit] |= btMask;
}

void World::voxUnset(int x, int y, int z)
{
	if (x<0 || x >=mVoxSizeX || y <0 || y>mVoxSizeY || z<0 || z >=mVoxSizeZ)
		return;

	int slice = (mVoxSizeY*(mVoxSizeX>>3));

	int bit = x&7;
	int btMask = 1<<bit;
	
	int putit = z * slice + y * (mVoxSizeX>>3) + (x>>3);

	voxels[ putit ] &= ~btMask;
}

bool World::VoxelPeek(const vec_t& pos)
{
	return voxPeek( (int)pos.x, (int)pos.y, (int)pos.z );
}

bool World::IsThereVoxelAtWorldPos( const vec_t& worldPos )
{
    if (!voxels)
        return false;

    vec_t pos = WorldToVoxel( worldPos );

    if ( pos.x < 0.f || pos.y < 0.f || pos.z < 0.f ||
        pos.x >= mVoxSizeX || pos.y >= mVoxSizeY || pos.z >= mVoxSizeZ )
        return false;

    return ( VoxelPeek(pos) != 0 );
}

void World::VoxelPoke( const vec_t& pos, bool bFill )
{
    int ipx = static_cast<int>(pos.x);
    int ipy = static_cast<int>(pos.y);
    int ipz = static_cast<int>(pos.z);

    if ( bFill )
        voxSet( ipx, ipy, ipz );
    else
        voxUnset( ipx, ipy, ipz );
       
}

void World::VoxelPoke( int ipx, int ipy, int ipz, bool bFill )
{
    if ( bFill )
        voxSet( ipx, ipy, ipz );
    else
        voxUnset( ipx, ipy, ipz );
}

void World::VoxelSphere(vec_t&sphere, bool bFill, const vec_t& axisScaleSquared)
{
	int stz = (int)Clamp(sphere.z-sphere.w, 0, mVoxSizeZ);
	int ndz = (int)Clamp(sphere.z+sphere.w, 0, mVoxSizeZ);
	int sty = (int)Clamp(sphere.y-sphere.w, 0, mVoxSizeY);
	int ndy = (int)Clamp(sphere.y+sphere.w, 0, mVoxSizeY);
	int stx = (int)Clamp(sphere.x-sphere.w, 0, mVoxSizeX);
	int ndx = (int)Clamp(sphere.x+sphere.w, 0, mVoxSizeX);
	float radiussq = sphere.w*sphere.w;
	if (bFill)
	{
		for (int az = stz; az < ndz; az++)
		{
			for (int ay = sty; ay < ndy; ay++)
			{
				for (int ax = stx; ax < ndx; ax++)
				{
					float dist = (ax-sphere.x)*(ax-sphere.x)*axisScaleSquared.x + (ay-sphere.y)*(ay-sphere.y)*axisScaleSquared.y + (az-sphere.z)*(az-sphere.z)*axisScaleSquared.z;
					if ( dist <= radiussq )
					{
						voxSet(ax, ay, az);
					}
				}
			}
		}
	}
	else
	{
		for (int az = stz; az < ndz; az++)
		{
			for (int ay = sty; ay < ndy; ay++)
			{
				for (int ax = stx; ax < ndx; ax++)
				{
					float dist = (ax-sphere.x)*(ax-sphere.x)*axisScaleSquared.x + (ay-sphere.y)*(ay-sphere.y)*axisScaleSquared.y + (az-sphere.z)*(az-sphere.z)*axisScaleSquared.z;
					if ( dist <= radiussq )
					{
						voxUnset(ax, ay, az);
					}
				}
			}
		}
	}
}

typedef struct voxVt_t
{
	
	voxVt_t() { nbFriends = 0; }
		voxVt_t(const vec_t & apos )
	{
		pos = apos;
		nbFriends = 0;
	}

	voxVt_t(const vec_t & apos, const vec_t & anorm)
	{
		pos = apos;
		norm = anorm;
		nbFriends = 0;
	}
	void addFriends(unsigned int friend1, unsigned int friend2)
	{
		friends[nbFriends++] = friend1;
		friends[nbFriends++] = friend2;
	}
	void addFriend(unsigned int afriend)
	{
		friends[nbFriends++] = afriend;
	}

	vec_t pos;
	vec_t norm;

	unsigned int nbFriends;
	unsigned int friends[6];
} voxVt_t;




void smoothVertices( const std::vector<voxVt_t>& src, std::vector<voxVt_t>& dest)
{
	static const float othersFactor = 0.04f;
    unsigned int voxSize = src.size();
	for (unsigned int i = 0;i< voxSize ; i++)
	{
		const vec_t& vtOrigin = src[i].pos;
		vec_t normOrigin = src[i].norm;
		vec_t newPos = vtOrigin;
		for (unsigned int j = 0;j<src[i].nbFriends;j++)
		{
            const voxVt_t& readVt = src[src[i].friends[j]];
			newPos += (readVt.pos - vtOrigin) * othersFactor;
			normOrigin += readVt.norm;
			//afact += othersFactor;
		}
		//vt *= (1.f / afact);
		dest[i].pos = newPos;

        float alen = (normOrigin.length()+0.001f);
            
		dest[i].norm = normOrigin * ( 1.f / alen );
	}
}

void pushIndicesFace( std::vector<uint32> &voxIdx, int ind0, int ind1, int ind2, int ind3, bool bInverse )
{
	if (bInverse)
	{
		voxIdx.push_back(ind2);
		voxIdx.push_back(ind1);
		voxIdx.push_back(ind0);
					
		voxIdx.push_back(ind3);
		voxIdx.push_back(ind2);
		voxIdx.push_back(ind0);
		
		
	}
	else
	{
		voxIdx.push_back(ind0);
		voxIdx.push_back(ind1);
		voxIdx.push_back(ind2);
					
		voxIdx.push_back(ind0);
		voxIdx.push_back(ind2);
		voxIdx.push_back(ind3);
		
	}
}

mesh_t *World::MakeVoxelMesh( float displacement )
{
	if (!voxels)
		return NULL;

    int currentVoxVt = 0;

    std::vector<voxVt_t> voxVts;
    std::vector<voxVt_t> voxVts2;
    std::vector<uint32> voxIdx;


    int *backLayerIndices[2];
	backLayerIndices[0] = new int [mVoxSizeX * mVoxSizeZ];
	backLayerIndices[1] = new int [mVoxSizeX * mVoxSizeZ];


	memset(backLayerIndices[0], -1, sizeof(int) * (mVoxSizeX * mVoxSizeZ));
	memset(backLayerIndices[1], -1, sizeof(int) * (mVoxSizeX * mVoxSizeZ));


	//int slice = (mVoxSizeY*(mVoxSizeX>>3));

	int indBack = 0;
	int indFront = 1;
	for (int ay = 0 ; ay< (mVoxSizeY-1) ; ay ++ )
	{
		memset(backLayerIndices[indFront], -1, sizeof(int) * (mVoxSizeX * mVoxSizeZ) );
		
		for (int az = 0 ; az < (mVoxSizeZ-1); az ++ )
		{
			for (int ax = (mVoxSizeX-2) ; ax >=0 ; ax -- )
			{
				bool b	= voxPeek(ax,   ay,   az);
				bool bx = voxPeek(ax+1, ay,   az);
				bool by = voxPeek(ax  , ay+1, az);
				bool bz = voxPeek(ax  , ay,   az+1);
                int stitch = ((ay&1)^(az&1)^(ax&1))?1:0; // pour alterner le edge du milieu

				int *reuse[8];
				
				int reuseIdx[] = {
					az * mVoxSizeX + ax,
					az * mVoxSizeX + ax+1,
					( az + 1 ) * mVoxSizeX + ax + 1,
					( az + 1 ) * mVoxSizeX + ax};

				for (int k = 0;k<4;k++)
					reuse[k] = &backLayerIndices[indBack][reuseIdx[k]];
				//for (int k = 0;k<4;k++)
					reuse[4] = &backLayerIndices[indFront][reuseIdx[0]];
                    reuse[4+1] = &backLayerIndices[indFront][reuseIdx[1]];
                    reuse[4+2] = &backLayerIndices[indFront][reuseIdx[2]];
                    reuse[4+3] = &backLayerIndices[indFront][reuseIdx[3]];


				vec_t usePoints[] = { vec(ax, ay, az),
					vec(ax+1, ay, az),
					vec(ax+1, ay, az+1),
					vec(ax, ay, az+1),

					vec(ax, ay+1, az),
					vec(ax+1, ay+1, az),
					vec(ax+1, ay+1, az+1),
					vec(ax, ay+1, az+1)};

				float mulNormFactor = b?1.f:-1.f;
				bool bBuildFace[] = { b!=bx, b!=by, b!=bz, b!=by };
				vec_t normalFace[] = { vec(1.f, 0.f, 0.f), vec(0.f, 1.f, 0.f), vec(0.f, 0.f, 1.f) };
				int faceUseIndex[3][4] = { {1,2,6,5},{4, 5, 6, 7},{2,3,7,6}};
                
                if ( stitch )
                {
                    int faceUseIndex2[3][4] = { {2,6,5,1},{5, 6, 7,4},{3,7,6,2}};
                    memcpy(faceUseIndex,faceUseIndex2, 12*sizeof(int));
                }
                
				for (int face = 0;face<3;face++)
				{
					if (!bBuildFace[face])
						continue;

					int *wantuse = faceUseIndex[face];
					bool bNewOnes[8] = {false, false, false, false, false, false, false, false};
					vec_t curNorm = normalFace[face] * mulNormFactor;
					for (int i=0;i<4;i++)
					{
						if ((*reuse[wantuse[i]]) == -1)
						{
							bNewOnes[wantuse[i]] = true;
							voxVts.push_back( voxVt_t( usePoints[wantuse[i]], curNorm ) );
							(*reuse[wantuse[i]]) = currentVoxVt++;
						}
					}

					// friendify
					
					for (int i=0;i<8;i++)
					{
						if (bNewOnes[i]) //wantuse[i]])
						{
                            static int neigbours[8][3] = { { 1,3,4}, {2,0,5}, {3,1, 6}, {0,2,7}, {0,5,7}, {1, 4,6},{5,7,2},{6,4,3} };
                            for (int nei = 0;nei<3;nei++)
                            {
                                int other = neigbours[i][nei];
                                int ind1 = (*reuse[ other ]);
                                if ( ind1 !=-1 )
                                {
                                    int ind0 = (*reuse[i]);
                                    voxVts[ind0].addFriend( ind1 );
                                    if (!bNewOnes[other])
                                        voxVts[ind1].addFriend( ind0 );

                                }

                            }
                            /*
							int ind1 = (*reuse[wantuse[(i+1)&3]]);
							int ind2 = (*reuse[wantuse[(i-1)&3]]);

							voxVts[ind0].addFriends(ind1, ind2);
							if (!bNewOnes[ (i+1)&3 ])
								voxVts[ind1].addFriend(ind0);
							if (!bNewOnes[ (i-1)&3 ])
								voxVts[ind2].addFriend(ind0);
                                */
						}
					}

                    if ( face != 3)
					    pushIndicesFace( voxIdx, (*reuse[wantuse[0]]), (*reuse[wantuse[1]]), (*reuse[wantuse[2]]), (*reuse[wantuse[3]]), b );
				}
			}
		}
		++indBack &= 1;
		++indFront &= 1;
	}

	voxVts2.resize( voxVts.size() );
	voxVts2 = voxVts;

	for (int i=0;i<12;i++)
	{
		smoothVertices( voxVts, voxVts2 );
		smoothVertices( voxVts2, voxVts );
	}

    


    // --
    if ( displacement > FLOAT_EPSILON )
    {
        Perlin *pinpin = new Perlin( 4, 0.15f, 1.f, 55418 );
        for (unsigned int i=0;i<voxVts2.size();i++)
        {
            vec_t& v = voxVts[i].pos;
            float displace = pinpin->Get( v.x, v.y, v.z ) * displacement;
            v += voxVts[i].norm * displace;
        }

        delete pinpin;
        pinpin = NULL;
#if 0
        // -- resmooth normals
        for (unsigned int i=0;i<voxVts.size();i++)
	    {
            voxVt_t& vox = voxVts[i];
            const vec_t & posv = vox.pos;
        
            int nbf = vox.nbFriends;
            if ( nbf )
            {
                vec_t nrm = vec( 0.f );
                for (int j=0;j<nbf;j++)
                {
                    vec_t dif1 = voxVts[vox.friends[j]].pos - posv;
                    vec_t dif2 = voxVts[vox.friends[(j+1)%nbf]].pos - posv;
                    dif1.normalize();
                    dif2.normalize();
                    vec_t locn;
                    locn.cross( dif2, dif1 );
                    locn.normalize();
                    vec_t locNorm = normalized(locn);
                    /*
                    if ( vox.norm.dot( locn ) < 0.f )
                        nrm -= locn;
                    else
                    */
                        nrm += locn;
                }
                if ( vox.norm.dot( nrm ) > 0.f)
                    vox.norm = normalized( nrm );
                else
                    vox.norm = -normalized( nrm );

            }
		
	    }
#endif
        for (unsigned int i=0;i<voxVts.size();i++)
	    {
            voxVts[i].norm = vec(0.f);
        }

        const int voxIdxMax = voxIdx.size();
        ASSERT_GAME( voxIdxMax%3 == 0 );

        for (int i = 0; i < voxIdxMax; i+=3)
        {
            int i1 = voxIdx[i];
            int i2 = voxIdx[i+1];
            int i3 = voxIdx[i+2];

            const vec_t& posv = voxVts[i1].pos;
            vec_t dif1 = voxVts[i2].pos - posv;
            vec_t dif2 = voxVts[i3].pos - posv;
            dif1.normalize();
            dif2.normalize();
            vec_t locn;
            locn.cross( dif2, dif1 );
            locn.normalize();
            vec_t locNorm = normalized(locn);

            voxVts[i1].norm -= locNorm;
            voxVts[i2].norm -= locNorm;
            voxVts[i3].norm -= locNorm;

        }
    }
    // --

	meshVertex_t* pVt = new meshVertex_t[voxVts.size()];
	for (unsigned int i=0;i<voxVts2.size();i++)
		pVt[i].set(voxVts[i].pos, normalized(voxVts[i].norm));

    // --
	mesh_t * nMesh = pushNewMesh(&voxIdx[0], voxIdx.size(), pVt, voxVts.size());

	voxVts2.clear();
	voxVts.clear();
    voxIdx.clear();

	delete [] backLayerIndices[0];
	delete [] backLayerIndices[1];

	return nMesh;

}

void World::DownsampleVoxel( int step )
{
    u8 *prev = new u8[ (mVoxSizeX/step) * (mVoxSizeY/step) * (mVoxSizeZ/step) ];
    memset(prev, 0 , (mVoxSizeX/step) * (mVoxSizeY/step) * (mVoxSizeZ/step) );
    u8* prev2 = prev;

    for (int ay = 0 ; ay< mVoxSizeY ; ay += step )
	{
		for (int az = 0 ; az < mVoxSizeZ ; az += step )
		{
			for (int ax = 0 ; ax < mVoxSizeX ; ax +=step )
            {
                *prev2 ++ = voxPeek(ax, ay, az);
            }
        }
    }
    
    InitVoxelTerrain( (mVoxSizeX/step), (mVoxSizeY/step), (mVoxSizeZ/step) );

    prev2 = prev;
    for (int ay = 0 ; ay< mVoxSizeY ; ay ++ )
	{
		for (int az = 0 ; az < mVoxSizeZ ; az ++ )
		{
			for (int ax = 0 ; ax < mVoxSizeX ; ax ++ )
            {
                if (*prev2 ++ )
                    voxSet(ax, ay, az);
            }
        }
    }
    delete [] prev;
}

void World::AddHeight2D(const vec_t& origin, const vec_t& size, const Height2D & height)
{
	int stz = (int)Clamp(origin.z, 0, mVoxSizeZ);
	int ndz = (int)Clamp(origin.z+size.z, 0, mVoxSizeZ);
	int sty = (int)Clamp(origin.y, 0, mVoxSizeY);
	int ndy = (int)Clamp(origin.y+size.y, 0, mVoxSizeY);
	int stx = (int)Clamp(origin.x, 0, mVoxSizeX);
	int ndx = (int)Clamp(origin.x+size.x, 0, mVoxSizeX);

	float scx = height.GetWidth()/size.x;
	float scz = height.GetHeight()/size.z;
	float scy = 256/size.y;
	const u8* bits = height.GetBits();

	
	float pz = 0;
	
	for (int az = stz; az < ndz; az++)	
	{
		float px = 0;
		for (int ax = stx; ax < ndx; ax++)	
		{
			float py = 0;
			for (int ay = sty; ay < ndy; ay++)
			{
				
				if (bits[ ((int)pz) * height.GetWidth() + (int)px] > py)
					voxSet(ax, ay, az);
				py += scy;
			}
			px += scx;
		}
		pz += scz;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void World::AddBuildingAd( const matrix_t& adp, float scale )
{

    matrix_t pylonMat;
    pylonMat = adp;
    pylonMat.position -= adp.dir * 10.f;

    matrix_t screenMat;
    world.BuildadPylon( pylonMat, 0.f, 0.f, 2.5f* scale,8.4f* scale,3.6f* scale,1,screenMat);
    
    matrix_t screenScale;
    screenScale.scale( 21.f*1.5f * scale, 9.f*1.5f * scale, 1.f );
    mesh_t*screenMesh = new mesh_t;
    screenMesh->createScreenMesh( vec( 0.f,0.f, 1.f, 1.f ) );
    
    
    screenMesh->mWorldMatrix = screenScale * screenMat;
    screenMesh->updateWorldBSphere();
    screenMesh->visible = true;
    
    
    screenMesh->color = vec( 1.f, 1.f, 1.f, 0.6f );
    screenMesh->screenMesh = fastrand()&3;
    /*
    omniLight_t *pLight = Renderer::newOmni();
    pLight->mColor = vec(1.f);
    pLight->mColor.w = 100.f;
    pLight->mPosition = screenMesh->mWorldMatrix.position;
    pLight->mPosition.w = screenMesh->bSphere.w * 2.f;
    */
}

void World::AddCentralAd( int aiPoint )
{
    matrix_t adp;
    
    const AIPoint_t& aipt = track.getAIPoint(aiPoint);
    aipt.BuildMatrix(adp, false);
    
    
    matrix_t convAd1;
    convAd1.rotationY( PI );
    convAd1.position.x = + aipt.mAIwidth[0];
    matrix_t screenMat;
    world.BuildadPylon( convAd1*adp, 15, 20, 3,4.2f,1.8f,1,screenMat);
    
    matrix_t screenScale;
    screenScale.scale( 21.f*1.5f, 9.f*1.5f, 1.f );
    mesh_t*screenMesh = new mesh_t;
    screenMesh->createScreenMesh( vec( 0.f,0.f, 1.f, 1.f ) );
    
    
    screenMesh->mWorldMatrix = screenScale * screenMat;
    screenMesh->updateWorldBSphere();
    screenMesh->visible = true;
    
    
    screenMesh->color = vec( 1.f, 1.f, 1.f, 0.5f );
    screenMesh->screenMesh = fastrand()&3;;    
}

void World::AddSideAd( int aiPoint, bool bLeft, int cutCount )
{
    //int scMesh = fastrand()&3;
    //float onev = 1.f/(float)(cutCount);
    for (int i=0;i<cutCount;i++)
    {
        
        const AIPoint_t& aipt = track.getAIPoint( (aiPoint + i * 2)%track.getAIPointCount());
        
        
        float ratio = (float)(i) / (float)(cutCount);
        
        matrix_t adp;
        
        aipt.BuildMatrix(adp, false);
        
        
        matrix_t convAd1;
        convAd1.rotationY( bLeft?(-PI*0.5f):(PI*0.5f) );
        convAd1.position.x = aipt.mAIwidth[0] * (bLeft?1.f:-1.f);
        
        matrix_t screenMat;
        world.BuildadPylon( convAd1*adp, 10.f, 0.f, 2, 3.2f, 1.37f, 0.8f, screenMat);
        
        matrix_t screenScale;
        screenScale.scale( 21.f/((float)cutCount) , 9.f, 1.f );


        mesh_t*screenMesh = new mesh_t;
        vec_t uvs;
        uvs = vec( ratio,0.f, ratio + 1.f/((float)cutCount), 1.f );
        
        screenMesh->createScreenMesh( uvs );
        
        
        screenMesh->mWorldMatrix = screenScale * screenMat;
        screenMesh->updateWorldBSphere();
        screenMesh->visible = true;
        
        
        screenMesh->color = vec( 1.f, 1.f, 1.f, 0.5f );
        screenMesh->screenMesh = 2;
    }
   
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Height2D::Height2D(int aWidth, int aHeight)
{
	mWidth = aWidth;
	mHeight = aHeight;
	mBits = new u8 [mWidth*mHeight];
	memset(mBits, 0, sizeof(u8) * mWidth*mHeight );
}

Height2D::~Height2D()
{
    delete[] mBits;
    mBits = NULL;
}

// x + : z,y
// x - : -z, y
// y + : x,z
// y - : -x, z
// z + : x,y
// z - : -x,y
void Height2D::PerlinMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace, float freq )
{
	Perlin *pinpin = new Perlin( 4, freq, amp, seed ); // snow height
	

    float invx = 1.f/(float)mWidth;
    float invy = 1.f/(float)mHeight;
    


    int sty = y;
    int ndy = Clamp(y+height, 0, mHeight);
    int stx = x;
    int ndx = Clamp(x+width, 0, mWidth);


    for ( int iy = sty ; iy < ndy; iy ++)
    {
        for (int ix = stx; ix < ndx; ix++)
        {
            float ppv;
            switch (cubeFace)
            {
            case 0:
                ppv = pinpin->Get( 0.5f, iy*invy - 0.5f, ix*invx - 0.5f );
                break;
            case 1:
                ppv = pinpin->Get( -0.5f, iy*invy - 0.5f, -ix*invx + 0.5f );
                break;
            case 2:
                ppv = pinpin->Get( ix*invx - 0.5f, -0.5f, -iy*invy + 0.5f );
                break;
            case 3:
                ppv = pinpin->Get( ix*invx - 0.5f, 0.5f, iy*invy - 0.5f );
                break;
            case 4:
                ppv = pinpin->Get( ix*invx - 0.5f, iy*invy - 0.5f, -0.5f );
                break;
            default:
            case 5:
                ppv = pinpin->Get( -ix*invx + 0.5f, iy*invy - 0.5f, 0.5f );
                break;
            }
            
            mBits[iy * mWidth + ix] = (u8)(ppv + base);
        }
    }

    delete pinpin;
		
}

void Height2D::SimplexMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace, float freq)
{
    UNUSED_PARAMETER(freq);
    UNUSED_PARAMETER(cubeFace);
    UNUSED_PARAMETER(seed);

    float invx = 1.f/(float)mWidth;
    float invy = 1.f/(float)mHeight;
    
    
    
    int sty = y;
    int ndy = Clamp(y+height, 0, mHeight);
    int stx = x;
    int ndx = Clamp(x+width, 0, mWidth);
    
    float chgRep = static_cast<float>((amp*0.5)/(1.f + 0.5f + 0.25f + 0.125f));
    float chgRepAdd = amp*0.5f + base;
    for ( int iy = sty ; iy < ndy; iy ++)
    {
        for (int ix = stx; ix < ndx; ix++)
        {
            float u = (float)ix * invx * 2.f*PI;
            float v = (float)iy * invy * 2.f*PI;;
            //            float r = noise(u, v) + noise(u*2.f, v*2.f)*0.5f +noise(u*4.f, v*4.f)*0.25f +noise(u*8.f, v*8.f) * 0.125f;
            float r = noise( cos(u), sin(u), cos(v), sin(v) );
            r += static_cast<float>(noise( cos(u)*2.f, sin(u)*2.f, cos(v)*2.f, sin(v)*2.f ) * 0.5);
            r += static_cast<float>(noise( cos(u)*4.f, sin(u)*4.f, cos(v)*4.f, sin(v)*4.f ) * 0.25);
            r += static_cast<float>(noise( cos(u)*8.f, sin(u)*8.f, cos(v)*8.f, sin(v)*8.f ) * 0.125);
            
            
            u8 p = (u8)(r * chgRep + chgRepAdd );
            mBits[iy * mWidth + ix] = p;//(u8)(r + base);

        }
    }

}

void Height2D::ComputeNormalMap( u32* nrmBits, float bumpHeightScale )
{
    UNUSED_PARAMETER(bumpHeightScale);

    const u8 *pBits = GetBits();

    for (int y = 0;y<mHeight;y++)
    {
        for (int x = 0;x<mWidth;x++)
        {
            u8 v = pBits[ (y*mHeight) + x ];
            u8 vN = pBits[ ( (((y-1)+mHeight)&(mHeight-1)) *mHeight) + x ];
            u8 vS = pBits[ (((y+1)&(mHeight-1)) *mHeight) + x ];
            u8 vW = pBits[ (y*mHeight) + ( ( x + mWidth-1)&(mWidth-1)) ];
            u8 vE = pBits[ (y*mHeight) + ( ( x + 1)&(mWidth-1)) ];

            float me = static_cast<float>( v ) * (1.f/255.f);
            float n = static_cast<float>( vN ) * (1.f/255.f);
            float s = static_cast<float>( vS ) * (1.f/255.f);
            float w = static_cast<float>( vW ) * (1.f/255.f);
            float e = static_cast<float>( vE ) * (1.f/255.f);

            vec_t norm = vec( 0.f, 0.f, 1.f );

            //find perpendicular vector to norm:        
            vec_t temp = norm; //a temporary vector that is not parallel to norm
            /*
            if(norm.x==1)
            temp.y+=0.5f;
            else
            /*/
            temp.x+=0.5f;

            float bumpHeightScale = 4.0f;
            //form a basis with norm being one of the axes:
            vec_t perp1 = normalized(cross(norm,temp));
            vec_t perp2 = normalized(cross(norm,perp1));

            //use the basis to move the normal in its own space by the offset        
            vec_t normalOffset = (perp1 * (-bumpHeightScale*(((n-me)-(s-me)))) + (perp2*((e-me)-(w-me))));
            norm += normalOffset;
            norm = normalized(norm);


            norm *= 0.5f;
            norm += vec(0.5f);
            norm.w = 1.f;

            nrmBits[ (y*mHeight) + x ] = norm.toUInt32();
        }
    }
}

void Height2D::FloeDisk( int x, int y, int radius, float height, float amp, int seed, float thresholdAdd, float thresholdFactor)
{
	Perlin *pinpin = new Perlin( 4, 4, amp, seed ); // snow height
	Perlin *pinpin2  = new Perlin( 4, 4, 1, seed*2 ); // floe borders

	float invx = 1.f/(float)mWidth;
	float invy = 1.f/(float)mHeight;
	float invRadius = 1.f/(float)radius;

	float radsq = (float)(radius*radius);

        int sty = Clamp(y-radius, 0, mHeight);
        int ndy = Clamp(y+radius, 0, mHeight);
        int stx = Clamp(x-radius, 0, mWidth);
        int ndx = Clamp(x+radius, 0, mWidth);

        //for (int iy = y-radius; iy < y+radius; iy++)
        for ( int iy = sty ; iy < ndy; iy ++)
	{
                //for (int ix = x-radius; ix < x+radius; ix++)
                for (int ix = stx; ix < ndx; ix++)
		{
			float distsq = (float)((ix-x)*(ix-x) + (iy-y)*(iy-y));
			if ( distsq <= radsq )
			{
				float ppv = pinpin->Get( ix*invx, iy*invy );

				float cutting = pinpin2->Get( ix*invx, iy*invy ) + thresholdAdd;
				cutting -= thresholdFactor* ( sqrtf(distsq) * invRadius ); ///(radsq * 2.f);
				if (cutting>0.f)
					mBits[iy * mWidth + ix] = (u8)(ppv + height);

				//mBits[iy * mWidth + ix] = 255;
			}
		}
	}

		delete pinpin;
		delete pinpin2;
}

void Height2D::SmoothCliff()
{
	for (int y = 1;y<mHeight-1;y++)
	{
		for (int x=1;x<mWidth-1;x++)
		{
			u8 value = mBits[y * mWidth + x];
			if (!value)
			{
				u8 amax = 0;
				for (int iy = -1;iy<2;iy++)
				{
					for (int ix = -1;ix<2;ix++)
					{
						amax=zmax( amax, mBits[(y+iy) * mWidth + (x+ix)] );
					}
				}
				u8 rnd = (fastrand()&0x3f)-0x20 + 0x80;
				int newvalue =  ( amax * rnd )>>8;
				//newvalue = (newvalue>0xF0)?0xF0:newvalue;

				mBits[y * mWidth + x] = (u8)newvalue;//( (float)(amax)*(0.5f + r01()*0.5f) ); //*(64+(fastrand()&0x3F))
			}
		}
	}
}

void Height2D::Erode(float smoothness)
{
    for (int i = 1; i < mHeight - 1; i++)
    {
        for (int j = 1; j < mWidth - 1; j++)
        {
            float d_max = 0.0f;
            int match[] = { 0, 0 };

            for (int u = -1; u <= 1; u++)
            {
                for (int v = -1; v <= 1; v++)
                {
                    if(abs(u) + abs(v) > 0)
                    {
                        float d_i = static_cast<float>(mBits[i*mWidth + j] - mBits[ (i + u  ) * mWidth + ( j + v ) ]);
                        if (d_i > d_max)
                        {
                            d_max = d_i;
                            match[0] = u; match[1] = v;
                        }
                    }
                }
            }

            if(0 < d_max && d_max <= (smoothness ))
            {
                const u8 d_h = static_cast<u8>(0.5f * d_max);
                mBits[i*mWidth + j] -= d_h;
                mBits[ (i + match[0])*mWidth + ( j + match[1] ) ] += d_h;
            }
        }
    }
}





void Height2D::Box(int x, int y, int width, int height, u8 value)
{
	for (int iy = y; iy < y+height; iy++)
		for (int ix = x; ix < x+width; ix++)
			mBits[iy * mWidth + ix] = value;
}

void Height2D::Circle(int x, int y, int radius, u8 value)
{
	for (int iy = y-radius; iy < y+radius; iy++)
		for (int ix = x-radius; ix < x+radius; ix++)
			if ( ((ix-x)*(ix-x) + (iy-y)*(iy-y)) <= (radius*radius) )
				mBits[iy * mWidth + ix] = value;
}

void Height2D::Noise(int strength)
{
	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			int v = mBits[y * mWidth + x];
			v += ((fastrand()&255)*strength - (strength<<7))>>8;
			v = Clamp(v, 0, 255);
			mBits[y * mWidth + x] = static_cast<u8>( v );
		}
	}
}

void Height2D::Stars(float min, float max, unsigned int count )
{
    UNUSED_PARAMETER(min);
    UNUSED_PARAMETER(max);

	for ( unsigned int i = 0 ; i < count ; i++ )
	{
        float rx = r01() * (mWidth-1);
        float ry = r01() * (mHeight-1);
        //float v = r01() * (float)(max-min) + (float)min;

        if ( fastrand()&1)
            mBits[ ((int)ry) * mWidth + (int)rx] = 0xFF;
        else
        {
            if ((rx>1)&&(rx<(mWidth-1))&&(ry>1)&&(ry<(mHeight-1)))
            {
                u8 val = fastrand()&63 + 192;

                mBits[ ((int)ry) * mWidth + (int)rx] = val;
                mBits[ ((int)ry) * mWidth + (int)rx-1] = val;
                mBits[ ((int)ry) * mWidth + (int)rx+1] = val;
                mBits[ ((int)ry+1) * mWidth + (int)rx] = val;
                mBits[ ((int)ry-1) * mWidth + (int)rx] = val;
            }
        }
	}
}

void Height2D::SmoothStep(float edge0, float edge1)
{
	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			float v = mBits[y * mWidth + x] * 0.003921f;
			v = smootherstep(edge0, edge1, v) * 255.f;
			mBits[y * mWidth + x] = (u8)v;
		}
	}
}

// 0 to 1
void Height2D::ContrastBrightness( float contrast, float brightness)
{
    float brightness255 = brightness * 255.f;
	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			float v = static_cast<float>(mBits[y * mWidth + x]) * contrast + brightness255;
			mBits[y * mWidth + x] = static_cast<u8>( Clamp( v , 0.f, 255.f) );
		}
	}
}

// sky box composite
void Height2D::SkyAddPlanet( u32 *pSrc, u32 *pDest )
{
    for (int _y = 0;_y<1024; _y++)
	{
		for (int _x = 0;_x<1024; _x++)
		{
			int x = _x * 2;
			int y = _y * 2;

            vec_t col;
			col.fromUInt32( pSrc[_y * 1024 + _x] );
			float v01 = col.x;
			/*
            float v01 = static_cast<float>( nebulus1.mBits[(y>>1) * nebulus1.mWidth + (x>>1)] ) * (1.f/255.f);
			
            float v2 = static_cast<float>( nebulus2.mBits[(y>>2) * nebulus2.mWidth + (x>>2)] ) * (1.f/255.f);
            float vstars = static_cast<float>( stars.mBits[y * 2048 + x] ) * (1.f/255.f);

            col = (col1 * v01) + (col2 * v2 ) + vec( vstars );
			*/
            //if ( bAddPlanet )
            {
                // sun
                float d  = sqrtf( static_cast<float>( (x-1024)*(x-1024) + (y -1536)*(y-1536) ) );
                float d2 = sqrtf( static_cast<float>( (x-1024)*(x-1024) + (y -1024)*(y-1024) ) );
                //float v = static_cast<float>(nebulus1.mBits[ (y>>1) * nebulus1.mWidth + (x>>1)] );
                float distSolid = d - 500.f;
                float distAtmos = d - 600.f;
                
                // sun light + hallow
                float sunmul = LERP(1.f, 0.f, Clamp( d2/780.f, 0.f, 1.f) );
                float solidEarth = Clamp(distSolid, 0.f, 1.f); //solid earth
                col *= solidEarth;
                float sun = 300255.f/powf(d2, 2.5f);
                float sunSum = sun * 5.f + smootherstep(0.05f, 0.99f,sunmul)*128.f;

                // atmosphere
                float diffHeight = 1.f - Clamp(d-496.f, 0.f, 150.f)/150.f;
                float diffSmooth = smootherstep(0.3f, 0.96f, diffHeight * diffHeight * diffHeight );// diffHeight);
                float atmos = LERP(1.f, 0, Clamp(distAtmos, 0.f, 1.f)) * sunmul * diffSmooth * Clamp(v01*1.1f + solidEarth , 0.f, 1.f);

                // clamp
                col += vec(255.f, 239.f, 102.f ) * atmos * (1.f/255.f);
                col += vec( sunSum * (1.f/255.f));
            }
            col.x = Clamp( col.x, 0.f, 1.f );
            col.y = Clamp( col.y, 0.f, 1.f );
            col.z = Clamp( col.z, 0.f, 1.f );
            col.w = 1.f;//Clamp( col.w, 0.f, 1.f );
            pDest[_y * 1024 + _x] = col.toUInt32();
        }
    }
}

// blur
void Height2D::HorzBlur(int strength)
{
    if (!strength)
        return;
    
    int nbp = (strength<<1) +1 ;    
    
	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			int val = 0;

			for (int ax = x-strength; ax<=x+strength; ax++)
			{
				if ((ax<0)||(ax>=mWidth))
					continue;
				val += mBits[y * mWidth + ax];
			}
			val /= nbp;
			mBits[y * mWidth + x] = static_cast<u8>( val );
		}
	}
}

void Height2D::VertBlur(int strength)
{
    if (!strength)
        return;
    
    int nbp = (strength<<1) +1 ;      
    
	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			int val = 0;
			for (int ay = y-strength; ay<=y+strength; ay++)
			{
				if ((ay<0)||(ay>=mHeight))
					continue;
				val += mBits[ay * mWidth + x];
			}
			val /= nbp;
			mBits[y * mWidth + x] = static_cast<u8>( val );
		}
	}
}

//factors are <<8. 256 means 1.f
void Height2D::ApplyKernel(int *factors, int kernelWidth)
{
	int ratio = factors[0];
	for (int i=1;i<kernelWidth*kernelWidth;i++)
		ratio += factors[i];

	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			int kernidx = 0;
			int val = 0;
			for (int ky = (y-(kernelWidth>>1)); ky <= (y+(kernelWidth>>1)) ; ky++)
			{
				for (int kx = (x-(kernelWidth>>1)); kx <= (x+(kernelWidth>>1)) ; kx++, kernidx++)
				{
					if ( (ky<0) || (ky>255))
						continue;
					if ( (kx<0) || (kx>255))
						continue;

					int factor = factors[kernidx];
					val += mBits[ky * mWidth + kx] * factor;
				}
			}
			val /= ratio;
			mBits[y * mWidth + x] = static_cast<u8>( val );
		}
	}
}

void Height2D::Save(const char *szName)
{
	stbi_write_png(szName, mWidth, mHeight, 1, mBits, mWidth);
}

void Height2D::MaxxedBy( Height2D &hg2 )
{
    for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
            u8 &basep = mBits[ y * mWidth + x ];
            u8 &basep2 = hg2.mBits[ y * mWidth + x ];
            basep = (basep>basep2)?basep:basep2;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////