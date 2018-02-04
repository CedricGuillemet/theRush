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
#include "world.h"
#include "camera.h"
#include "track.h"
#include "physics.h"
#include "menus.h"
#include "game.h"
#include "net.h"
#include "fx.h"
#include "content.h"
#include "bonus.h"
#include "render.h"
#include "gui.h"

Trail *trail = NULL;

void testUnit1()
{


    //importShipsASE("D:/Projects/R2/Assets/ships2.ASE", "D:/Projects/R2/Assets/ships2.bin");

#if 0
	for (int i=0;i<256;i++)
	{
		mesh_t *pm = generateBox();
		//pm->matrix.		
		matrix_t tr;
		float tall = 10.f;
				tr.translation(0,tall-0.5f,0);
	tr.translation( (float)(i&0xF)*2.f - 15.f, 0.f, (float)(i>>4)*2.f -15.f);

		pm->mBaseMatrix.scale( 1.f, tall*2.f, 1.f);
	
		pm->mWorldMatrix = tr;

		pm->updateWorldBSphere();                                                                                              

		float alpha = r01()*0.8f + 0.2f;//((float)i* (1.f/255.f));
		pm->color = vec(r01(), r01(), r01(), alpha);
		pm->visible = true;
		pm->physic = true;

	}
	


	mesh_t *pm = generateBox();
	matrix_t tr;
	tr.translation(0.f, -0.5f, 0.f);
	pm->mWorldMatrix.scale( 1200.f, 1.f, 1200.f);
	pm->mWorldMatrix *= tr;
	pm->updateWorldBSphere();
	pm->color = vec(0.8f, 1.f, 0.8f, 0.5f);
	pm->visible = false;
	pm->physic = true;
#endif	

	extern float WIDTH , HEIGHT;

	camera.project(90.0f, WIDTH/HEIGHT, 0.1f, 2000.0f);
	camera.view(vec(4,1,4), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));


     




	// mini sphere test
	/*
	vec_t vts[4];
	vts[0] = vec(-1.f, 0.f, -1.f);
	vts[1] = vec( 1.f, 0.f, -1.f);
	vts[2] = vec(-0.5f, 1.f, 1.f);
	vts[3] = vec( 0.5f, 1.f, 1.f);

	sphere_t sph = sphere_t::miniBall((unsigned char*)vts, sizeof(vec_t), 4);
*/
#if 0
	mesh_t* hexa = generateHexagon(3.f, 2.f, 1.f);

	tr.translation(0.f, 3.5f, 0.f);
	hexa->mBaseMatrix = tr;
	hexa->updateWorldBSphere();
	hexa->color = vec(1.f, 1.f, 1.f, 0.5f);
	hexa->visible = true;
	hexa->physic = false;

	// animation
	meshAnimation_t *pa = hexa->addAnimation();
	pa->mDuration = 3.f;
	pa->mThreshold = 0.1f;

	pa->mColorStart = vec(0.5f, 0.5f, 0.5f, 0.5f);
	pa->mColorEnd = vec(1.f, 1.f, 1.f, 1.f);
	//pa->mColorType = 4;
	
	pa->mTranslationStart = vec(0.f, 0.5f, 0.f);
	pa->mTranslationEnd = vec(0.f, 2.5f, 0.f);
	pa->mTranslationType = 2;
	pa->mTranslationInterpolation = 1;


	pa->mScalingStart = vec(1.f, 1.f, 1.f);
	pa->mScalingEnd = vec(2.f, 2.f, 2.f);
	pa->mScalingType = 2;
	pa->mScalingInterpolation = 1;

	// hexa2
	hexa = generateHexagon(3.f, 2.f, 1.f);

	tr.translation(5.f, 3.5f, 0.f);
	hexa->mBaseMatrix = tr;
	hexa->updateWorldBSphere();
	hexa->color = vec(1.f, 1.f, 1.f, 0.5f);
	hexa->visible = true;
	hexa->physic = false;

	// animation
	pa = hexa->addAnimation();
	pa->mDuration = 3.f;
	pa->mThreshold = 0.1f;

	pa->mColorStart = vec(0.5f, 0.5f, 0.5f, 0.5f);
	pa->mColorEnd = vec(1.f, 1.f, 1.f, 1.f);
	//pa->mColorType = 4;
	
	pa->mTranslationStart = vec(0.f, 0.5f, 0.f);
	pa->mTranslationEnd = vec(0.f, 2.5f, 0.f);
	pa->mTranslationType = 2;
	pa->mTranslationInterpolation = 1;




	// text
	mesh_t* txtm = generateText("I\3.the rush//");
	
//	bendMesh(txtm);
	matrix_t mt;
	mt.rotationZ(PI*0.5f);
	mt.translation(0, 5,0);
	mt.scale(0.3f, 0.3f, 0.3f);
	txtm->mWorldMatrix = mt;
	//transformMesh(txtm, mt);
	//bendMesh(txtm, -PI*0.5f, 0, 2);
	//transformMesh(txtm, mt);
	txtm->color = vec(1.f, 0.4f, 0.6f, 0.6f);
	txtm->visible = true;
	txtm->physic = false;
	txtm->updateWorldBSphere();

	// static mesh for physics
//	physicWorld.physicMeshesToBulletPhysic();
	// delaunay
	/*
	vec_t vtsd[4] = {vec(-1.f, -1.f, 0.f),
		vec(-1.f, 1.f, 0.f),
		vec(1.f, 1.f, 0.f),
		vec(1.f, -1.f, 0.f)};
		*/
	vec_t vtsd[12];
	for (int i=0;i<12;i++)
		vtsd[i] = vec(cosf(i/12.f * 2.f * PI),
		0.f, sinf(i/12.f * 2.f * PI));
	mesh_t * delau = generateDelaunay(vtsd, 12);
	delau->computeBSphere();
	delau->mWorldMatrix.translation(0.f, 5.f, -5.f);
	delau->color = vec(0.4f, 1.0f, 0.6f, 0.6f);
	delau->visible = true;
	delau->physic = false;
	delau->updateWorldBSphere();
	

	vec_t vtsd2[] = {
		vec(0.f, 0.f, 0.f ),
		vec(0.f, 0.f, 2.f ),
		vec(1.f, 0.f, 2.f ),
		vec(1.f, 0.f, 0.5f),
		vec(3.f, 0.f, 0.5f),
		vec(3.f, 0.f, 2.f ),
		vec(4.f, 0.f, 2.f ),
		vec(4.f, 0.f, 0.f )};

	delau = generateDelaunay(vtsd2, 8);
	delau->computeBSphere();
	delau->mWorldMatrix.translation(0.f, 5.f, -5.f);
	delau->color = vec(0.4f, 1.0f, 0.6f, 0.6f);
	delau->visible = true;
	delau->physic = false;
	delau->updateWorldBSphere();

	//ShowMenu(1);
    
    // test symetrical function
    
    vec_t symetrical;
    symetrical = buildPlan(vec(2.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));
    vec_t sym = vec(6.f, -5.f, 0.f);
    vec_t ressym = symetrical.symetrical(sym);
    printf("%5.2f %5.2f %5.2f\n", ressym.x, ressym.y, ressym.z);

	//
	uint32 *cols = new uint32 [16*16*16];
//	memset(cols, -1, sizeof(uint32) * 16 * 16 * 16);
    
    for (int az = 0;az <16; az ++)
    {
        for (int ay = 0;ay<16; ay ++)
        {
            for (int ax = 0;ax<16;ax++)
            {
                cols[az*16*16 + ay*16 +ax] = 0x1F8FF8FF;//(ax&1)?0xFF00FFFF:0;
            }
        }
    }
    cols[0] = 0;
	mesh_t * ship = generateVoxelMesh(cols, 16, 16, 16);
	ship->computeBSphere();
	ship->mWorldMatrix.translation(8.f, 5.f, -5.f);
	ship->color = vec(1.f, 1.f, 1.f, 0.5f);
	ship->visible = true;
	ship->physic = false;
	ship->updateWorldBSphere();
	delete [] cols;
	/*
	vec_t vts[4] = { vec(-50,10,-50), vec(30, 10, -10), vec(0,10,60), vec(80,10,120) };
	//vec_t vts[2] = { vec(-50,10,-50), vec(80,10,80) };
	BuildFlyingCarRibbon(vts, 4, 0.05f, 1.f);
	
	vec_t vts2[4] = {vec(79.5f,11.5f,120.3f), vec(0.7f,11.3f,60.9f),  vec(30.8f, 11.8f, -9.1f), vec(-52.f,9.2f,-49.f) };
	BuildFlyingCarRibbon(vts2, 4, 0.05f, 1.f);
	*/
#endif

	
#if 0
    //////// sprites
    for (int i=0;i<32;i++)
    {
        mesh_t* trackSpeed = getSprite(i);
        matrix_t tr;
        tr.translation(4.f, 2.f, i*4.f - 16.f);
        trackSpeed->mWorldMatrix.scale(0.2f, 0.2f, 0.2f);
        trackSpeed->mWorldMatrix *= tr;
        trackSpeed->color = vec(1.f, 1.f, 1.f, 1.f);
        trackSpeed->physic = false;
        trackSpeed->updateWorldBSphere();
        trackSpeed->visible = true;
        
        mesh_t *ts2 = trackSpeed->clone();
        tr.translation(-4.f, 2.f, i*4.f - 16.f);
        ts2->mWorldMatrix.scale(0.2f, 0.2f, 0.2f);
        ts2->mWorldMatrix *= tr;
        ts2->color = vec(1.f, 1.f, 1.f, 1.f);
        ts2->physic = false;
        ts2->updateWorldBSphere();
        ts2->visible = true;
        
        //delete trackSpeed;
    }
#endif
	// cylindrical city
	
	//world.buildCylindricalBuilding(vec(-120,-30,0), 10, 1, "I \3 .the rush//");
	/*
	world.buildCylindricalBuilding(vec(-100,0,0), 3, -1);

	world.buildCylindricalBuilding(vec(-80,0,0), 3, -1);
	world.buildCylindricalBuilding(vec(-60,0,0), 4, -1);
	world.buildCylindricalConnection(vec(-80,0,0), vec(-60,0,00), 2);


	for (int i=0;i<5;i++)
	{
		world.buildCylindricalBuilding(vec(-120+i*20,0,-20), 4, i, (i==1)?"I \3 .the rush//":NULL);
	}
	
	
	world.buildCylindricalBuilding(vec(0,0,0), 4, 1, "I \3 .the rush//");
	
	world.buildCylindricalBuilding(vec(20,0,20), 5, 2);

	world.buildCylindricalConnection(vec(0,0,0), vec(20,0,20), 3);
	world.buildCylindricalConnection(vec(0,0,0), vec(20,0,20), 1);

	world.buildCylindricalBuilding(vec(-20,0,20), 4, 3);

	//world.buildCylindricalBuilding(vec(-40, 0, 40), 6, -1);


	world.buildCylindricalConnection(vec(0,0,0), vec(-20,0,20), 2);
	*/
	// track ribbon

	
	/*
	
	vec_t trckRib[NBSEGS];
	for (int i=0;i<NBSEGS;i++)
		trckRib[i] = Tracks[0].segs[i].pt * 0.05f + vec(0.f, 100.f, 0.f);
	ribbon_t *nRib = new ribbon_t;
	nRib->BuildRibbon(trckRib, NBSEGS, 0.125f, 100.f, true);
	GTracksRibbons.push_back(nRib);



	for (int i=0;i<NBSEGS;i++)
		trckRib[i] = Tracks[0].segs[i].point2 * 0.05f + vec(0.f, 100.f, 0.f);

	nRib = new ribbon_t;
	nRib->BuildRibbon(trckRib, NBSEGS, 0.125f, 100.f, true);
	GTracksRibbons.push_back(nRib);
	*/
#if 0
	matrix_t mtt;
	mtt.scale(0.05f, 0.05f, 0.05f );
	mesh_t * merged = track.GenerateTrackForMenus(&Tracks[0], mtt);

	merged->visible = true;
	merged->color = vec(0.f, 0.f, 0.f, 0.5f);
	merged->computeBSphere();
	merged->mWorldMatrix = mtt;
	merged->updateWorldBSphere();

	

#endif
	/*
	world.computeWorldSize();
	world.mSunAmbient = vec(0.3f, 0.3f, 0.4f, 1.f);
	world.mSunColor = vec(0.7f, 0.68f, 0.68f, 1.f);
	*/
#if 0
	Lightning *nLn = new Lightning(32);
	nLn->SetColor(vec(0.8f, 0.8f, 1.f, 1.f));
	nLn->SetPoints(vec(0.f), vec(130.f));
	nLn->SetWidth(20.f);
#endif
    /*
    for (int k = 0;k<10;k++)
    {
	trail = new Trail();
	trail->SetColor(vec(1.f, 0.8f, 0.6f, 1.f));
	trail->SetLossPerSecond(0.f);
	trail->SetWidth(5.f);

    for (int i=0;i< 1337*k;i++)
    {
        trail->SetCurrentPoint( vec( cosf( (float)i * PI /128.f ) * (20.f*r01() + 200.f), 40.f+k*10.f, sinf( (float)i * PI /128.f ) * (20.f*r01() + 200.f) ) );
    }
    }
    */
    /*
	trail = new Trail();
	trail->SetColor(vec(1.f, 0.8f, 0.6f, 1.f));
	trail->SetLossPerSecond(0.f);
	trail->SetWidth(5.f);
    
    trail->SetCurrentPoint( vec( 0.f, 40.f, 0.f ) );
    trail->SetCurrentPoint( vec( 50.f, 40.f, 0.f ) );
    trail->SetCurrentPoint( vec( 50.f, 40.f, 50.f ) );
    trail->SetCurrentPoint( vec( 0.f, 40.f, 50.f ) );
    */
    /*
    vec_t aurPts[]={ vec(0.f, 100.f, 0.f), vec( 100.f, 100.f, 0.f), vec( 100.f, 100.f, 100.f), vec ( 200.f, 100.f, 100.f) };
    mesh_t *aurora = generateAurora( aurPts, 4, 20, 100 );
    aurora->visible = true;
    aurora->color = vec(1.f);
    aurora->mWorldMatrix.identity();
    aurora->updateWorldBSphere();
    */
    BuildAuroraBorealis( vec(0.f, 150.f, 0.f ) , 200.f, 1600, 600.f, 8 );
    BuildAuroraBorealis( vec(0.f, 160.f, 300.f ) , 120.f, 1000, 400.f, 6 );
    BuildAuroraBorealis( vec(0.f, 160.f, -300.f ) , 120.f, 1000, 400.f, 6 );

    /*
    trail->SetCurrentPoint( vec( 0.f, 40.f, 0.f ) );
    trail->SetCurrentPoint( vec( 25.f, 40.f, 0.f ) );
    trail->SetCurrentPoint( vec( 50.f, 40.f, 0.f ) );
    trail->SetCurrentPoint( vec( 50.f, 40.f, 25.f ) );
    trail->SetCurrentPoint( vec( 50.f, 40.f, 50.f ) );
    trail->SetCurrentPoint( vec( 25.f, 40.f, 50.f ) );
    trail->SetCurrentPoint( vec( 0.f, 40.f, 50.f ) );
    */
#if 0
	vec_t acars[]={vec(-100.f, 10.f, -100.f), vec(-10.f, 12.f, -10.f), vec(10.f, 10.f, -10.f), vec(100.f, 15.f, 200.f)};
	FlyingCars *fcars = new FlyingCars(acars, 4, 0.1f);
	fcars->SetWidth(2.f);
	fcars->SetVShift(0.2f);
#endif
	//Game::CreateNewGameSolo(0,3);
	
	/*world.setSunDirection(vec(1.f, 0.8f, 0.38f));
	world.mSunColor =  vec(0.6f, 0.4f, 0.4f);
    */
	/*
	matrix_t mt1,mt2, mt3;
	mt1.translation(0.f, 0.f, 0.f);
	mt2.translation(-1.f, 10.f, -1.f);
	mt3.scale(5.f, 1.f, 5.f);

	pushNewMeshInit();
	pushTrunk( mt3 * mt1, mt3 * mt2);
	mesh_t *mesh = pushNewMesh();
	mesh->visible = true;
	mesh->computeBSphere();
	mesh->mWorldMatrix.identity();
	mesh->updateWorldBSphere();
	mesh->color = vec(1.f, 1.f, 1.f, 0.5f);
	*/

	matrix_t mt;
	//mt.rotationY(PI*0.25f);
	mt.identity();
	/*
	for (float hg = 0.f ; hg< 40.f; hg += 3.f)
	{
		world.BuildPylone(vec(0.f, 0.f, hg * 8.f), vec(0.f, hg+2.f, hg * 8.f), mt, 0);
	}

	for (float hg = 0.f ; hg< 40.f; hg += 3.f)
	{
		world.BuildPylone(vec(20.f, 0.f, hg * 8.f), vec(20.f, hg+2.f, hg * 8.f), mt, 1);
	}

	for (float hg = 0.f ; hg< 40.f; hg += 3.f)
	{
		world.BuildPylone(vec(40.f, 0.f, hg * 8.f), vec(40.f, hg+2.f, hg * 8.f), mt, 2);
	}
	
	omniLight_t * pLight = Renderer::newOmni();
	pLight->mColor = vec(2.f, 2.f, 2.f, 1.f);
	pLight->mPosition = vec(0.f, 5.f, 0.f, 25.f);
	*/
	// start pylon
	/*
	StartupPylon *testPylon1 = new StartupPylon;
	StartupPylon *testPylon2 = new StartupPylon;
	
	matrix_t mtp1, mtp2;
	mtp1.rotationY(1.2f);
	mtp2.translation(10.f, 2.f, 6.f);
	testPylon1->Init(mtp1);
	testPylon2->Init(mtp2);
	*/
	// cone
	/*
	mesh_t *cone = generateLightCone(16);
	cone->visible = true;
	cone->color = vec(1.f, 1.f, 0.f, 0.5f);
	cone->computeBSphere();
	matrix_t coneSize, coneMat;
	coneSize.scale(5.f, 5.f, 10.f);
	coneMat.LookAt(vec(0.f, 5.f, 0.f), vec(5.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));

	cone->mWorldMatrix = coneSize * coneMat;
	cone->updateWorldBSphere();
	*/
#if 0
	// voxel
	PROFILER_START(CanyonTest);
	Height2D h2(64,64);
	h2.Box(0,0,64,64, 10);
	for (int i=0;i<20;i++)
	{
		h2.Circle(32,32,20,160);
		h2.Circle(10,38,4,240);
		h2.Circle(35,40,8,200);
		h2.Circle(38,20,5,210);
		h2.Noise(160);
		
		h2.HorzBlur(5);
		h2.VertBlur(5);
	}
	h2.SmoothStep(0.4f, 0.8f);
	PROFILER_END(); // CanyonTest

	//h2.Save("d:\\temp\\canyon.png");
	
	matrix_t mtp1, mtp2;
	mtp2.translation(0.f, -2.f, 0.f);

	world.InitVoxelTerrain(128,128,128);
	//world.VoxelSphere( vec(8.f, 8.f, 8.f, 7.f) );
	world.AddHeight2D( vec(0.f), vec(64.f, 32.f, 64.f), h2 );
	/*
	world.AddHeight2D( vec(0.f,0.f, 64.f), vec(64.f, 32.f, 64.f), h2 );
	world.AddHeight2D( vec(64.f, 0.f, 0.f), vec(64.f, 32.f, 64.f), h2 );
	world.AddHeight2D( vec(64.f, 0.f, 64.f), vec(64.f, 32.f, 64.f), h2 );
	*/
	mesh_t *terr = world.MakeVoxelMeshAndClean();
	terr->computeBSphere();
	terr->mWorldMatrix = mtp2;
	terr->visible = true;
	terr->color = vec(185.f/255.f, 122.f/255.f, 87.f/255.f, 0.5f);
	terr->mbIsIceGround = true;

	terr->updateWorldBSphere();


	matrix_t sca;
	sca.scale(5.f);

	for (int i= 0;i<10;i++)
	{
		matrix_t tra;
		tra.translation(i*20.f,0,0);


                mesh_t * newship = getCompoundShip( getRandomCompound() )->clone();
		newship->color = vec(r01()*0.6f + 0.4f, r01()*0.6f + 0.4f, r01()*0.6f + 0.4f, 1.f);
			
			//vec(0.9f, 0.75f, 0.75f, 1.f);
		newship->visible = true;
	
		newship->mWorldMatrix = sca * tra;

		newship->updateWorldBSphere();
	}

#endif
/*
	matrix_t mtmini;
		mtmini.identity();
		mesh_t *minitrack = track.GenerateTrackForMenus( &Tracks[0], mtmini );
		
		if (minitrack)
		{
			minitrack->computeBSphere();
			minitrack->visible = true;
			minitrack->color = vec(1.f);
			minitrack->mWorldMatrix.identity();
			minitrack->updateWorldBSphere();
		}

        */
        matrix_t mtp1, mtp2;
        mtp2.scale( 10.f );//.translation(0.f, -2.f, 0.f);
        
	world.InitVoxelTerrain(128,128,128);
    
    for (int z = 0;z<10;z++)
    {
        for (int x=0;x<10;x++)
        {
            world.VoxelPoke( vec(x+4, 1, z+4), true );
        }
    }
    
    world.VoxelPoke( vec(5, 2, 6), true );
    world.VoxelPoke( vec(6, 2, 5), true );

    /*
    world.VoxelPoke( vec(5, 1, 5), true );
    world.VoxelPoke( vec(6, 1, 6), true );
    */

    //void World::VoxelPoke( const vec_t& pos, bool bFill )
	//world.VoxelSphere( vec(8.f, 8.f, 8.f, 7.f) );
	//world.AddHeight2D( vec(0.f), vec(64.f, 32.f, 64.f), h2 );
	/*
	world.AddHeight2D( vec(0.f,0.f, 64.f), vec(64.f, 32.f, 64.f), h2 );
	world.AddHeight2D( vec(64.f, 0.f, 0.f), vec(64.f, 32.f, 64.f), h2 );
	world.AddHeight2D( vec(64.f, 0.f, 64.f), vec(64.f, 32.f, 64.f), h2 );
	*/

    Renderer::setRenderMeshStack(GetMeshStack());

	mesh_t *terr = world.MakeVoxelMesh();
	terr->computeBSphere();
	terr->mWorldMatrix = mtp2;
	terr->visible = true;
	terr->color = vec(1.f, 1.f, 1.f, 0.6f );//vec(185.f/255.f, 122.f/255.f, 87.f/255.f, 0.5f);
	terr->mbIsIceGround = true;

	terr->updateWorldBSphere();
#if 0
#endif
    camera.view(vec(50,30,50), vec(60.f, 10.f, 60.f), vec(0.f, 1.f, 0.f));
    camera.view( vec(0.f, 160.f, 0.f), vec(0.f), vec(0.f, 0.f, -1.f ) );
    //camera.SetCameraCustom();
    /*
    std::vector<buildingPylon_t> buildingPylons;
	mesh_t* city = generateCity( buildingPylons );
    city->computeBSphere();
    //city->mWorldMatrix.scale(50.f);//.identity();
    city->visible = true;
    city->color = vec(1.f, 1.f, 1.f, 0.5f );//vec(185.f/255.f, 122.f/255.f, 87.f/255.f, 0.5f);


    city->updateWorldBSphere();
    
    
    */
    /*
        extern bool GBDrawOcean;
    GBDrawOcean = false;
    
    mesh_t *groundcity = generateEtron( );//getSprite(7)->clone();
    groundcity->computeBSphere();
    //groundcity->mWorldMatrix.identity();
    groundcity->mWorldMatrix.scale(3.f);//
    groundcity->mWorldMatrix.position = vec(0.f, 50.f, 0.f,1.f);
    groundcity->updateWorldBSphere();
    groundcity->visible = true;
    groundcity->color = vec(1.f, 1.f, 1.f, 1.f );

    
    Renderer::setRenderMeshStack(GetMeshStack());
    world.computeWorldSize();
    */
#if 0
    PROFILER_START(GenSpaceBox);
        for (int i=0;i<6;i++)
        {
            PROFILER_START(perlin1);
            Height2D hg( 2048, 2048 );
            hg.PerlinMap( 0,0, 2048, 2048, 127.f, 128.f, 89746, i );
            hg.ContrastBrightness( 0.3f, 0.0f );
            PROFILER_END();

            PROFILER_START(perlin2);
            Height2D hg2( 2048, 2048 );
            hg2.PerlinMap( 0,0, 2048, 2048, 127.f, 128.f, 785, i );
            hg2.SmoothStep(0.55f, 1.f);
            PROFILER_END();

            PROFILER_START(stars);
            Height2D hg3( 2048, 2048 );
            hg3.stars(200.f, 255.f, 5000 );
            hg3.HorzBlur(1);
            hg3.VertBlur(1);
            PROFILER_END();
        

            //


            PROFILER_START(mix);
            u32 *pDest = new u32[2048*2048];

            Height2D::SkyComposite( hg, hg2, vec(173.f/255.f, 14.f/255.f, 2.f/255.f, 1.f)*2.f, vec(6.f/255.f, 18.f/255.f, 100.f/255.f)*1.3f, hg3, pDest, (i==0) );
            /*
            char tmps[512];
            sprintf(tmps, "d:/temp/cube%d.png", i );
            stbi_write_png( tmps, 2048,2048, 4, pDest, 2048*4);
            */
            delete [] pDest;
            PROFILER_END();
        }
        PROFILER_END();
#endif
#if 0
        {
        vec_t pipev[]={vec(0.f), vec(0.f, 0.f, 50.f), vec(0.f, 50.f, 60.f) };
            mesh_t *groundcity = generatePipe( pipev, 3, 5, 0.1f, 2.f, 16, 0x80FFFFFF);//getSprite(7)->clone();

        //mesh_t *groundcity = GetTankerMesh( );//getSprite(7)->clone();
    groundcity->computeBSphere();
    //groundcity->mWorldMatrix.identity();
    groundcity->mWorldMatrix.scale(2.f);//
    groundcity->mWorldMatrix.position = vec(0.f, 50.f, 0.f,1.f);
    groundcity->updateWorldBSphere();
    groundcity->visible = true;
    groundcity->color = vec(1.f, 1.f, 1.f, 1.0f );
        }
        
        CreateTankerOverCity( vec(0.f), 200,400,150,0.1f );
        CreateTankerOverCity( vec(100.f, 200.f, 50.f ), 300,500,250, -0.05f );


        {
            mesh_t *groundcity = GetEnv4Anchor()->clone();
            groundcity->mWorldMatrix.identity();//.position = vec(0.f, 50.f, 0.f,1.f);
            groundcity->updateWorldBSphere();
            groundcity->visible = true;
            groundcity->color = vec(1.f, 1.f, 1.f, 1.0f );
        }
        {
            mesh_t *groundcity = GetEnv4()->clone();
            groundcity->mWorldMatrix.identity();//.position = vec(0.f, 50.f, 0.f,1.f);
            groundcity->updateWorldBSphere();
            groundcity->visible = true;
            groundcity->color = vec(1.f, 1.f, 1.f, 1.0f );
        }

        matrix_t ident;
        ident.identity();
        world.BuildCylindricalBuilding( ident, 10, 1, NULL );

        //exit(0);
        //physicWorld.NewRope();
    GSkyScatParams = &Tracks[1].mSkyScatteringParameters;
#endif
    
    /*
    	mesh_t *pm = generateBox();
	matrix_t tr;
	tr.translation(0.f, -0.5f, 0.f);
	pm->mWorldMatrix.scale( 1200.f, 1.f, 1200.f);
	pm->mWorldMatrix *= tr;
	pm->updateWorldBSphere();
	pm->color = vec(1.f, 1.f, 1.f, 0.5f);
	pm->visible = true;
	pm->physic = false;

    GSkyScatParams->mbDrawOcean = false;


    omniLight_t * pLight = Renderer::newOmni();
	pLight->mColor = vec(2.f, 0.f, 0.f, 1.f);
	pLight->mPosition = vec(100.f, 15.f, 0.f, 60.f);
    */


     world.InitVoxelTerrain(256, 64, 256);

//        int trackIndex = pTrack - GetTrack0();

        Perlin *pinpin = new Perlin( 4, 4, 1.f, 5541  );
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
                    float v = (pinpin->Get( ax*1.2f, fact*0.5f, az*1.2f )*0.5f)+0.5f;
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


       /* AddTunnelLightsAndAdScreens( pTrack, topo, mats1, mats2 );

	    digTunnel( pTrack, trMin, transLen, 6.f, 0.5f );
        */
        // set 

        // tunnel sub


        //LOG("pre voxel mesh\n");


#if 0
        {
            mesh_t *terr = world.MakeVoxelMesh( 2.0f );
            terr->computeBSphere();
            /*
            matrix_t scTerr, trTerr;
            scTerr.scale(1.f/transLen);
            scTerr.m16[5] *= 2.f;
            trTerr.translation(trMin);
            */
            terr->mWorldMatrix.scale( vec(2.f,2.5f,2.f) );// = scTerr * trTerr ; 
            terr->visible = true;
            terr->color = vec(130.0f/255.0f,100.0f/255.0f,14.0f/255.0f, 0.5f);
            terr->shader = vec(0.2f, 0.4f, 0.f, 0.f );

            terr->mbIsIceGround = true;
            terr->updateWorldBSphere();


            FILE *dumpit = fopen("dumpMesh.cpp","wt");
            if (dumpit)
            {
                char tmps[512];
                sprintf( tmps,"int terrainVerticesCount = %d;\n", terr->mVA->GetVertexCount() );
                fputs( tmps, dumpit );
                sprintf( tmps,"int terrainIndicesCount = %d;\n", terr->mIA->GetIndexCount() );
                fputs( tmps, dumpit );

                // vertices
                fputs( " const float terrainVertices[] = {\n", dumpit );
                const char *pv = (const char *)terr->mVA->Lock(VAL_READONLY);
                for (unsigned int i = 0;i<terr->mVA->GetVertexCount();i++)
                {
                    vec_t vt = *(vec_t*)pv;
                    vt.TransformPoint( terr->mWorldMatrix );
                    sprintf(tmps,"%5.4ff,%5.4ff,%5.4ff,\n", vt.x, vt.y, vt.z );
                    fputs( tmps, dumpit );
                    pv += terr->mVA->GetVertexSize();
                }
                terr->mVA->Unlock();
                fputs( " };\n", dumpit );
                // indices
                fputs( " const float terrainIndices[] = {\n", dumpit );
                const int *pi = (const int *)terr->mIA->Lock(VAL_READONLY);
                for (unsigned int i = 0;i<terr->mIA->GetIndexCount();i++)
                {
                   
                    sprintf(tmps,"%d,\n", *pi );
                    fputs( tmps, dumpit );
                    pi++;
                }
                terr->mIA->Unlock();
                fputs( " };\n", dumpit );
                fclose(dumpit);
            }
        }
#endif
        GSkyScatParams->mbDrawOcean = true;
   	//Game::CreateNewGameSolo(0,3);
	LOG("new game solo created\n");
	//track.GoWithTrack(&Tracks[3]);
    camera.SetModeFPS();
 
}
mesh_t* createBoxMarker(const vec_t& pos, const vec_t& col, float sz);


// circuit normal
void testUnit2()
{
	LOG(" start of testunit2\n");
	mesh_t *pm = generateBox();
	matrix_t tr;
	tr.translation(0.f, -0.5f, 0.f);
	pm->mWorldMatrix.scale( 32.f, 0.1f, 32.f);
	pm->mWorldMatrix *= tr;
	pm->updateWorldBSphere();
	pm->color = vec(1.f, 1.f, 1.f, 0.5f);
	pm->visible = true;


	extern float WIDTH , HEIGHT;

	camera.project(90.0f, WIDTH/HEIGHT, 0.1f, 1000.0f);
	camera.view(vec(4,1,4), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));
	
	
	
	Game::CreateNewGameSolo(0,3);
	LOG("new game solo created\n");
	//track.GoWithTrack(&Tracks[3]);
    
    
    int TRACK_WE_USE = 0;
    
	trackSeg_t *segs = Tracks[TRACK_WE_USE].segs;
	camera.view(segs[20].pt+vec(0.f, 1.f, 0.f), segs[5].pt+vec(0.f, 1.f, 0.f), vec(0.f, 1.f, 0.f));
    camera.view(vec(1,600,0), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));	
	camera.view(vec(378,4,278), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));	

    camera.view(segs[10].pt+vec(0.f, 20.f, 0.f), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));	
    
    LOG("track done\n");
#if 0
	world.computeWorldSize();
	vec_t sz = world.mWorldMax-world.mWorldMin;
	world.buildSectors(1024.f/*zmax(sz.x,max(sz.y, sz.z))*0.5f*/, 256.f);
	world.buildSectorsBox();
#endif

	/*
	pm = generateBox();
	//matrix_t tr;
	tr.translation(-512.f, 128.f, -512.f);
	pm->mWorldMatrix.scale( 2048.f, 512.f, 2048.f);
	//pm->mWorldMatrix *= tr;
	pm->updateWorldBSphere();
	pm->color = vec(0.8f, 1.f, 0.8f, 0.25f);
	pm->visible = true;
	pm->physic = true;
	*/

	// static mesh for physics
//	physicWorld.physicMeshesToBulletPhysic();


		//physicWorld.spawn(track.segs[5].pt+vec(0.f, 4.f, 0.f));
		//
    GGame->SetTrack( static_cast<u16>( TRACK_WE_USE ) );
    GGame->SetNumberOfLapsToDo( 3 ); 
	GGame->SetSplit( false ); 
    GGame->SetState( GameStateCountDown );
    GGame->Tick( 0.001f );

	 matrix_t mtc;
	 mtc.translation(segs[1].pt+vec(0.f, 2.f, 0.f));
	 	 matrix_t mtc2;
	 mtc2.translation(segs[1].pt+vec(0.f, 40.f, 0.f));
         GameShip *ps = (GameShip *)GGame->Spawn(GO_Ship, mtc, NULL );
         GGame->SetPlayerShip( ps );
         /*
         GameShip *ps2 = (GameShip *)GGame->Spawn(GO_Ship, mtc2, NULL );
		 ps2->SetBot();
         ps2->mProps.ctrlEnabled = false;
         */
    //camera.lockOnMatrix(&ps->GetPhysics()->mTransform);
	 //camera.SetCameraBehindShip( ps );
	 	camera.project(90.0f, WIDTH/HEIGHT, 0.1f, 1000.0f);
	camera.view(vec(4,1,4), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));

    

//    createBoxMarker( mtc.position, vec(1.f, 0.f, 0.f, 1.f), 8.f );
    

	ps->mProps.ctrlEnabled = false;
    //Bonus::ForceWeaponToAlwaysBe( BT_Mines );

    //FIXME: commenting out unused code because of if(0) 
#if 0
	if ( 0 )
	{
		camera.SetCameraBehindShip( ps );
		ps->mProps.ctrlEnabled = true;
        //camera.SetCameraEndRace(ps2);
        //ps->SetBot();
	}
#endif


    // build screen based on ai point number
    
    world.AddCentralAd( 20 );
    world.AddSideAd( track.getAIPointCount()-5, false, 6 );
    world.AddSideAd( track.getAIPointCount()-5, true, 3 );

	//GGame->Spawn(GO_Ship, track.segs[0].pt+vec(0.f, 4.f, 0.f), "Coleco" );

    
	matrix_t mt, atr, atr2;
    atr.translation(-5, 0, 0);
    atr2.translation(5, 0, 0);    
	track.getAIPoint(0).BuildMatrix(mt, false);
	//mt.translation( track.segs[0].pt );
	//mt.position = track.segs[0].pt;
	//PushTrackBonus(atr * mt, BT_Trackspeed);
	//PushTrackBonus(atr2 * mt, BT_SpeedBooster);//BT_3Missiles);//DestructionShield);
	bonusGenerationParameters_t bonusParams;
	Game::GenerateBonuses( bonusParams, Tracks[0], true, true, false );
    



	Menus::Show(MENU_EMPTY_SOLO);
     

	LOG("menus done\n");

/*	world.setSunDirection(vec(1.f, 0.8f, 0.38f));
	world.mSunColor =  vec(0.6f, 0.4f, 0.4f);
    */



	camera.view(segs[0].pt+vec(0.f, 5.f, 0.f), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));	
        camera.view(vec(400,100,400), vec(0.f, 0.f, 0.f), vec(0.f, 1.f, 0.f));
        Renderer::setRenderMeshStack(GetMeshStack());
	LOG("testunit inited.\n");
}
