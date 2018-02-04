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

#if EDITOR_ENABLE

#include "camera.h"
#include "therush.h"
#include "track.h"
#include "render.h"
#include "game.h"
#include "bonus.h"
#include "gui.h"
#include "audio.h"
#include "JSON_serializer.h"
#include "physics.h"
#include "fx.h"


#include "include_libGizmo.h"
#include "include_GL.h"
#include "include_SDL.h"


#include "imgui.h"
#include "imguiRenderGL.h"
#include "world.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

void PutBannerText();
void StartGameSolo( unsigned int gameMode );

bool GBEditorInited = false;
bool GBEditorTestingMap = false;

IGizmo *gizmo, *gizmoMove, *gizmoRotate, *gizmoScale;
matrix_t anchorsMat[NBSEGS];
bool anchorBorder[NBSEGS];
bool groundLock[NBSEGS];
bool positionLock[NBSEGS];

int mAnchorEditIndex = 0;
int mTrackEditIndex = 0;

bool mbUsingGizmo = false;
bool mbIsPlaying = false;
int miShowHelp = 0;
prefabInstance_t *currentPrefab = NULL;
unsigned int CurrentGameMode = 1;
GameShip *mpShip;
bool showPrefabs = false;
bool playModeRecord = false;
bool GizmoWorldMode = true;

typedef struct undoRedo_t
{
    undoRedo_t( trackSeg_t *dest )
    {
        pDest = dest;
    }
    trackSeg_t *pDest;


    trackSeg_t originalSegs[NBSEGS];
    trackSeg_t modifiedSegs[NBSEGS];

    void saveUnDo() { memcpy( originalSegs, pDest, sizeof(trackSeg_t) * NBSEGS ); }
    void saveRedo() { memcpy( modifiedSegs, pDest, sizeof(trackSeg_t) * NBSEGS ); }
    void doIt() { memcpy( pDest, modifiedSegs, sizeof(trackSeg_t) * NBSEGS ); }
    void undoIt() { memcpy( pDest, originalSegs, sizeof(trackSeg_t) * NBSEGS ); }
}undoRedo_t;

typedef struct undoRedoTrack_t
{
    std::vector<undoRedo_t> undos;
    std::vector<undoRedo_t> redos;

    void PreDo()
    {
        undos.push_back( undoRedo_t( Tracks[mTrackEditIndex].segs ) );
        undos[undos.size() -1].saveUnDo();
        redos.clear();
    }

    void PostDo()
    {
        undos[undos.size() -1].saveRedo();
    }

    void ReDo()
    {
        if ( redos.empty() )
            return;
        redos[redos.size() -1].doIt();
        undos.push_back( redos[redos.size() -1] );
        redos.pop_back();

        Tracks[mTrackEditIndex].mbIsDirty = true;
    }

    void UnDo()
    {
        if ( undos.empty() )
            return;
        undos[undos.size() -1].undoIt();
        redos.push_back( undos[undos.size() -1] );
        undos.pop_back();

        Tracks[mTrackEditIndex].mbIsDirty = true;
    }

} undoRedoTrack_t;


undoRedoTrack_t undoRedos[MAX_NB_TRACKS];

void BuildAnchorsMatrices()
{
    matrix_t mirrorx;
    mirrorx.scale( -1.f, 1.f, 1.f );
    for (int k=0;k<1;k++)
    {
        bool bFourche = (k==1);
        for (int i=0;i<NBSEGS;i++)
        {
            anchorsMat[i] = mirrorx*Tracks[mTrackEditIndex].segs[i].GetMat( bFourche );
            anchorBorder[i] = Tracks[mTrackEditIndex].segs[i].border[ bFourche ];
			groundLock[i] = Tracks[mTrackEditIndex].segs[i].groundLock[ bFourche ];
			positionLock[i] = Tracks[mTrackEditIndex].segs[i].positionLock[ bFourche ];
        }
    }
}

void RecomputeTrack()
{
    trackBonusInstance_t::ClearBonuses();
    ClearMeshStack();
    Renderer::clearAllLights();
	Renderer::portals.clear();
    ClearFlashLights();
    track.GoWithTrack( &Tracks[mTrackEditIndex], true );

    const gameType_t& gt = GameTypes[CurrentGameMode];

    Game::GenerateBonuses( Tracks[mTrackEditIndex].mBonusGenerationParameters, Tracks[mTrackEditIndex], 
        gt.bSpeedTrackBonus, 
        gt.bWeaponTrackBonus,
        gt.bColorTrackBonus );

    BuildAnchorsMatrices();


}



void DrawAnchorList()
{
    glColor4f(1,1,1,1);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);


    for (int k=0;k<2;k++)
    {
        //FIXME: unused local variable, should it be removed or is it used as a reminder?
        //bool bFourche = (k==1);
        for (int i=0;i<NBSEGS;i++)
        {
            glPushMatrix();

            glMultMatrixf( anchorsMat[i].m16 );

            static const float anchorScale = 8.f;

			float bordered = anchorBorder[i];
			float red = bordered?1.f:0.5f;

			if (positionLock[i])
			{
				glPushMatrix();
				glScalef(anchorScale,anchorScale,anchorScale);
				glBegin(GL_QUADS);
				// Front Face
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
				//
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
				//
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
				//
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				//
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
				//
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
				glColor4f(1.f,0.f,1.f,1.f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
				glEnd();
				glPopMatrix();
			}
			else
			{
				glBegin(GL_TRIANGLES);
				glColor4f(red,0.6f,0.6f,1.f);glVertex3f(-1.0f*anchorScale, -0.5f*anchorScale,  -1.0f*anchorScale);
				glColor4f(red,0.6f,0.6f,1.f);glVertex3f( 0.0f*anchorScale, -0.5f*anchorScale,   2.0f*anchorScale);
				glColor4f(red,0.6f,0.6f,1.f);glVertex3f( 1.0f*anchorScale, -0.5f*anchorScale,  -1.0f*anchorScale);

				glColor4f(red,0.5f,0.5f,1.f);glVertex3f(-1.0f*anchorScale, -0.5f*anchorScale,  -1.0f*anchorScale);
				glColor4f(red,0.5f,0.5f,1.f);glVertex3f( 0.0f*anchorScale, -0.5f*anchorScale,   2.0f*anchorScale);
				glColor4f(red,0.5f,0.5f,1.f);glVertex3f( 0.0f*anchorScale,  0.5f*anchorScale,   0.0f*anchorScale);

				glColor4f(red,0.7f,0.7f,1.f);glVertex3f(-1.0f*anchorScale, -0.5f*anchorScale,  -1.0f*anchorScale);
				glColor4f(red,0.7f,0.7f,1.f);glVertex3f( 0.0f*anchorScale,  0.5f*anchorScale,   0.0f*anchorScale);
				glColor4f(red,0.7f,0.7f,1.f);glVertex3f( 1.0f*anchorScale, -0.5f*anchorScale,  -1.0f*anchorScale);

				glColor4f(red,0.4f,0.4f,1.f);glVertex3f( 0.0f*anchorScale,  0.5f*anchorScale,   0.0f*anchorScale);
				glColor4f(red,0.4f,0.4f,1.f);glVertex3f( 0.0f*anchorScale, -0.5f*anchorScale,   2.0f*anchorScale);
				glColor4f(red,0.4f,0.4f,1.f);glVertex3f( 1.0f*anchorScale, -0.5f*anchorScale,  -1.0f*anchorScale);
				if (groundLock[i])
				{
					glColor4f(red,0.6f,0.6f,1.f);glVertex3f(-1.0f*anchorScale,  1.5f*anchorScale,  -1.0f*anchorScale);
					glColor4f(red,0.6f,0.6f,1.f);glVertex3f( 0.0f*anchorScale,  1.5f*anchorScale,   2.0f*anchorScale);
					glColor4f(red,0.6f,0.6f,1.f);glVertex3f( 1.0f*anchorScale,  1.5f*anchorScale,  -1.0f*anchorScale);

					glColor4f(red,0.5f,0.5f,1.f);glVertex3f(-1.0f*anchorScale,  1.5f*anchorScale,  -1.0f*anchorScale);
					glColor4f(red,0.5f,0.5f,1.f);glVertex3f( 0.0f*anchorScale,  1.5f*anchorScale,   2.0f*anchorScale);
					glColor4f(red,0.5f,0.5f,1.f);glVertex3f( 0.0f*anchorScale,  0.5f*anchorScale,   0.0f*anchorScale);

					glColor4f(red,0.7f,0.7f,1.f);glVertex3f(-1.0f*anchorScale,  1.5f*anchorScale,  -1.0f*anchorScale);
					glColor4f(red,0.7f,0.7f,1.f);glVertex3f( 0.0f*anchorScale,  0.5f*anchorScale,   0.0f*anchorScale);
					glColor4f(red,0.7f,0.7f,1.f);glVertex3f( 1.0f*anchorScale,  1.5f*anchorScale,  -1.0f*anchorScale);

					glColor4f(red,0.4f,0.4f,1.f);glVertex3f( 0.0f*anchorScale,  0.5f*anchorScale,   0.0f*anchorScale);
					glColor4f(red,0.4f,0.4f,1.f);glVertex3f( 0.0f*anchorScale,  1.5f*anchorScale,   2.0f*anchorScale);
					glColor4f(red,0.4f,0.4f,1.f);glVertex3f( 1.0f*anchorScale,  1.5f*anchorScale,  -1.0f*anchorScale);
				}
				glEnd();
			}
            glPopMatrix();
        }
    }
}

void ChangeRoadHeight( bool bEverySegs, bool moveUp )
{
track_t& ET =  Tracks[mTrackEditIndex];
    undoRedos[mTrackEditIndex].PreDo();
        
    int st = bEverySegs?0:mAnchorEditIndex;
    int nd = bEverySegs?NBSEGS:(mAnchorEditIndex+1);
        
    for (int i = st ; i < nd ; i ++)
    {
        if ( moveUp )//KeyCtrlDown() )
        {
            ET.segs[i].pt.y += 1.f;
            ET.segs[i].point2.y += 1.f;
        }
        else
		{
            ET.segs[i].pt.y -= 1.f;
            ET.segs[i].point2.y -= 1.f;
        }
    }

    ET.UpdateSegments();
    undoRedos[mTrackEditIndex].PostDo();                
    ET.mbIsDirty = true;
}


void ChangeRoadWidth( bool bEverySegs, bool Incr )
{
    track_t& ET =  Tracks[mTrackEditIndex];
    undoRedos[mTrackEditIndex].PreDo();
        
    int st = bEverySegs?0:mAnchorEditIndex;
    int nd = bEverySegs?NBSEGS:(mAnchorEditIndex+1);
        
    for (int i = st ; i < nd ; i ++)
    {
        if ( Incr && ET.segs[i].width[0] < 25.2f)
        {
            ET.segs[i].width[0] += 1.2f;
            ET.segs[i].width[1] += 1.2f;
        }

        if ( (!Incr) && ET.segs[i].width[0] > 6.f)
        {
            ET.segs[i].width[0] -= 1.2f;
            ET.segs[i].width[1] -= 1.2f;
        }
        
   }

    ET.UpdateSegments();
    undoRedos[mTrackEditIndex].PostDo();                
    ET.mbIsDirty = true;
}

void ResetAxis( bool bEverySegs )
{
	track_t& ET =  Tracks[mTrackEditIndex];

    undoRedos[mTrackEditIndex].PreDo();

	int st = bEverySegs?0:mAnchorEditIndex;
    int nd = bEverySegs?NBSEGS:(mAnchorEditIndex+1);
        
    for (int i = st ; i < nd ; i ++)
    {
		ET.segs[i].up[0] = vec( 0.f, 1.f, 0.f );
		ET.segs[i].up[1] = vec( 0.f, 1.f, 0.f );
		ET.segs[i].width[0] = 12.f;
		ET.segs[i].width[1] = 12.f;
	}
    Tracks[mTrackEditIndex].UpdateSegments();
    undoRedos[mTrackEditIndex].PostDo();                
    Tracks[mTrackEditIndex].mbIsDirty = true;
}


void InitEditor()
{
    GBEditorInited = true;
    extern float WIDTH , HEIGHT;

    camera.project(90.0f, WIDTH/HEIGHT, 0.1f, 1000.0f);
    camera.view( vec(100.f), vec(0.f), vec(0.f, 1.f, 0.f) );
    camera.SetCameraCustom();
	
    camera2.project(90.0f, WIDTH/HEIGHT, 0.1f, 1000.0f);
    camera2.view( vec(100.f), vec(0.f), vec(0.f, 1.f, 0.f) );
    camera2.SetCameraCustom();


    Game::CreateNewGameSolo( 0, 9999 );

    //track.GoWithTrack(&Tracks[mTrackEditIndex]);
    RecomputeTrack();

    //objectMatrix.translation(Tracks[mTrackEditIndex].segs[0].pt);//.identity();
    //Tracks[mTrackEditIndex].segs[0].pt;

    gizmoRotate = CreateRotateGizmo();
    gizmoMove = CreateMoveGizmo();
    gizmoScale = CreateScaleGizmo();
    gizmo = gizmoMove;

    gizmoRotate->SetDisplayScale( 1.5f );
    gizmoMove->SetDisplayScale( 3.f );
    gizmoScale->SetDisplayScale( 3.f );

    BuildAnchorsMatrices();

    Renderer::setRenderMeshStack( GetMeshStack() );


	imguiRenderGLInit( FileToString( "Datas/fonts/imagine.ttf" ).c_str() );
/*    
	extern bool splitScreenEnabled;
	splitScreenEnabled = true;
	*/
}

void UninitEditor()
{
    if (!GBEditorInited)
        return;

	imguiRenderGLDestroy();
}

void StopGameTest()
{
    camera.SetCameraCustom();
    //GGame->Destroy( mpShip );
    GGame->DestroyGameObjects();

    GGame->ResetNetIndexAllocator();

    physicWorld.ClearCollisions();

    Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME2D );
    Channel::DestroyAllChannelsByGroup( AUDIO_GROUP_GAME3D );

    Menus::Show( MENU_NONE );
    GBEditorTestingMap = false;
    RecomputeTrack();
}

void TickEditor( float aTimeEllapsed )
{
    if (!GBEditorInited)
        return;

    if ( (!mbIsPlaying) || playModeRecord )
    {
        if (MouseRButDown())
        {
            camera.SetModeFPS();
        }
        else
        {
            camera.SetCameraCustom();
            camera.updateFPSMoves();
        }
    }

    // gizmo
    extern float WIDTH, HEIGHT;

    gizmo->SetScreenDimension( (int)WIDTH, (int)HEIGHT );
    //gizmo->SetCameraMatrix( camera.mView.m16, camera.mProjection.m16 );

	if ( currentPrefab )
	{
		gizmo->SetEditMatrix( currentPrefab->mPrefabMatrix );
		currentPrefab->mMesh->mWorldMatrix = currentPrefab->mPrefabMatrix;
		currentPrefab->mMesh->updateWorldBSphere();

	}
	else
		gizmo->SetEditMatrix( anchorsMat[mAnchorEditIndex].m16 );

    extern int MouseX, MouseY;
	if ( currentPrefab && keysReleased(SDLK_DELETE))
	{
		delete currentPrefab->mMesh;
		track.GetCurrentTrack()->mPrefabs.erase( track.GetCurrentTrack()->mPrefabs.begin() + ( currentPrefab - &track.GetCurrentTrack()->mPrefabs[0]) );
		currentPrefab = NULL;
	}

    if (MouseLButPressed())
    {
        mbUsingGizmo = gizmo->OnMouseDown( MouseX, MouseY );
        if (!mbUsingGizmo)
        {
            // pick
            vec_t rayOrigin, rayDir;

            camera.BuildRay( MouseX, MouseY- (((int)SCREENHEIGHT-(int)HEIGHT)>>1), (int)WIDTH, (int)HEIGHT, rayOrigin, rayDir );
			bool bFound = false;
            for (int i=0;i<NBSEGS;i++)
            {
                vec_t sph = anchorsMat[i].position;
                sph.w = 10.f;
                float rs = IntersectRaySphere( rayOrigin, rayDir, sph );
                if ( rs > 0.f)
                {
                    mAnchorEditIndex = i;
					currentPrefab = NULL;
					bFound = true;
                }
            }
			// search for meshes
			if (!bFound)
			{
				for (unsigned int i = 0;i<track.GetCurrentTrack()->mPrefabs.size();i++)
				{
					prefabInstance_t &prefabInstance = track.GetCurrentTrack()->mPrefabs[i];

					if (!prefabInstance.mMesh)
						continue;

					float rs = IntersectRaySphere( rayOrigin, rayDir, prefabInstance.mMesh->worldBSphere );
					if ( rs > 0.f)
					{
						currentPrefab = &prefabInstance;
						break;
					}
				}
			}
        }
		else
		{
			if (KeyShiftDown() && currentPrefab)
			{
				track.GetCurrentTrack()->mPrefabs.push_back( prefabInstance_t( currentPrefab->mMesh->clone(), currentPrefab->mPrefabFile, currentPrefab->mPrefabMatrix ) );
				currentPrefab = &track.GetCurrentTrack()->mPrefabs[track.GetCurrentTrack()->mPrefabs.size() -1 ];
			}
		}
    }
    else
        if (MouseLButReleased())
        {
            if (mbUsingGizmo)
            {
                mbUsingGizmo = false;
                gizmo->OnMouseUp( MouseX, MouseY );   

                undoRedos[mTrackEditIndex].PreDo();
                float roadWidth = Tracks[mTrackEditIndex].segs[mAnchorEditIndex].width[0];
                /*
                if ( gizmo == gizmoScale )
                    roadWidth = 6.f+anchorsMat[mAnchorEditIndex].right.length()*3.f;
                    */
                for (int i = 0;i<5;i++)
                {
                Tracks[mTrackEditIndex].segs[mAnchorEditIndex].pt = anchorsMat[mAnchorEditIndex].position;
                Tracks[mTrackEditIndex].segs[mAnchorEditIndex].up[0] = normalized( anchorsMat[mAnchorEditIndex].up );
                Tracks[mTrackEditIndex].segs[mAnchorEditIndex].up[1] = normalized( anchorsMat[mAnchorEditIndex].up );
                Tracks[mTrackEditIndex].segs[mAnchorEditIndex].width[0] = roadWidth;
                Tracks[mTrackEditIndex].segs[mAnchorEditIndex].width[1] = roadWidth;
                
                Tracks[mTrackEditIndex].UpdateSegments();
                }
                anchorsMat[mAnchorEditIndex].orthoNormalize();

                undoRedos[mTrackEditIndex].PostDo();
                Tracks[mTrackEditIndex].mbIsDirty = true;
                
            }

        }
        else
        {
            /*char tmps[512];
            sprintf(tmps," %d %d\n", MouseX, MouseY);
            OutputDebugString( tmps );
            */
            gizmo->OnMouseMove( MouseX, MouseY );
        }

        // rotate
        if (keysReleased(SDLK_1) ) // move
        {
            gizmo = gizmoMove;
			if (currentPrefab)
				gizmo->SetLocation( GizmoWorldMode?IGizmo::LOCATE_WORLD:IGizmo::LOCATE_LOCAL );
			else
				gizmo->SetLocation( IGizmo::LOCATE_WORLD );
        }

        if (keysReleased(SDLK_2) ) // rotate
        {
            gizmo = gizmoRotate;
            if (currentPrefab)
				gizmo->SetLocation( GizmoWorldMode?IGizmo::LOCATE_WORLD:IGizmo::LOCATE_LOCAL );
			else
				gizmo->SetLocation( IGizmo::LOCATE_LOCAL );
        }
        
        if (keysReleased(SDLK_3) ) // scale
        {
            gizmo = gizmoScale;
            gizmo->SetLocation( IGizmo::LOCATE_LOCAL );
        }
        
    if ( keysReleased(SDLK_a) )
    {
        GBEditorTestingMap = !GBEditorTestingMap;
         RecomputeTrack();
    }

	if ( keysReleased(SDLK_n) )
    {
        // free ride
        if (!mbIsPlaying)
        {
            GBEditorTestingMap = true;
            RecomputeTrack();

            matrix_t mtc;
			GGame->Restart( CurrentGameMode );

			world.SetGameSpeed( 1.f );
			setDOFBlur( 0.f );

			// go game in solo
			const int shipCountSolo = Game::E_Max_Ship_Count;
			for ( int i = 0 ; i < shipCountSolo ; ++i )
			{
				matrix_t mt;
				mt = track.GetSpawnMatrix( i );
				mt.position += mt.up * 1.5f;
				mt.position.w = 1.f;

				GameShip *ps = (GameShip *)GGame->Spawn(GO_Ship, mt, NULL );

				ps->SetBot( 0.9f + static_cast<float>(i)*0.1f );
				shipIdentifier_t shipID;
				shipID.SetRandom();
				ps->SetShipIdentifier( shipID );
				ps->mProps.ctrlEnabled = false;
			}
			playModeRecord = true;
        }
        else
        {
            StopGameTest();
			extern bool splitScreenEnabled;
			splitScreenEnabled = false;
			playModeRecord = false;
        }
        mbIsPlaying = !mbIsPlaying;
    }
	if (playModeRecord)
	{
		if ( keysReleased(SDLK_SPACE))
		{
			float worldSpeed = world.GetGameSpeed();
			if (worldSpeed <= 0.1f)
				world.SetGameSpeed( 1.f );
			else 
				world.SetGameSpeed( 0.f );
		}
		if ( keysReleased(SDLK_s))
		{
			const int shipCountSolo = Game::E_Max_Ship_Count;
			for ( int i = 0 ; i < shipCountSolo ; ++i )
			{
				GameShip *ps = (GameShip *)GGame->GetShip(i);
				ps->mProps.ctrlEnabled = true;
			}
		}
		extern float GInputGlobalSpeed;
		config_t config = GetEngineConfig();
		camera.mSensibility = config.mouseSensibility * GInputGlobalSpeed;
	}
	else
	{
		config_t config = GetEngineConfig();
		camera.mSensibility = config.mouseSensibility;
	}

    if ( keysReleased(SDLK_f) )
    {
        // free ride
        if (!mbIsPlaying)
        {
            GBEditorTestingMap = true;
            RecomputeTrack();

            matrix_t mtc;


            mtc.translation(Tracks[mTrackEditIndex].segs[0].pt+vec(0.f, 24.f, 0.f));
            //mtc = track.GetSpawnMatrix( 1 );

            GGame->SetType( static_cast<u8>(CurrentGameMode) );
            GGame->SetNumberOfLapsToDo( 99 );
			GGame->SetSplit( false );
            GGame->ClearFlagPropsDirtyNetwork();
            GGame->ClearFlagPropsDirtyWorld();
#if IS_OS_WINDOWS
			GGame->LoadPhysicExtension();
#endif
            mpShip = (GameShip *)GGame->Spawn(GO_Ship, mtc, NULL );
            mpShip->mProps.ctrlEnabled = true;


            GGame->SetPlayerShip( mpShip );
            camera.SetCameraBehindShip( mpShip );
            
            // another one bites the dust
            
            // mtc.translation(Tracks[mTrackEditIndex].segs[1].pt+vec(0.f, 24.f, 0.f));
			
			// --- SECOND PLAYER ---
            mtc = track.GetSpawnMatrix( 2 );
            GameShip * mpShip2 = (GameShip *)GGame->Spawn(GO_Ship, mtc, NULL );
            /*
            Menus::Show(MENU_EMPTY_SOLO);
            GGame->SetState(GameStatePlaying);
			*/
			/*
			extern bool splitScreenEnabled;
			splitScreenEnabled = true;
			camera2.SetCameraBehindShip( mpShip2 );
			*/
            /*
            // check mine
            mtc.translation(Tracks[mTrackEditIndex].segs[2].pt+vec(0.f, 24.f, 0.f));
            GGame->Spawn(GO_Mine, mtc, NULL );
            */
            
        }
        else
        {
            StopGameTest();
			extern bool splitScreenEnabled;
			splitScreenEnabled = false;
        }
        mbIsPlaying = !mbIsPlaying;
    }
    if ( keysReleased(SDLK_g) )
    {
        // go for a game
        if (!mbIsPlaying)
        {
            GBEditorTestingMap = true;
            RecomputeTrack();
            /*
            matrix_t mtc;
            mtc.translation(Tracks[mTrackEditIndex].segs[0].pt+vec(0.f, 24.f, 0.f));
            mpShip = (GameShip *)GGame->Spawn(GO_Ship, mtc, NULL );
            mpShip->mProps.ctrlEnabled = true;
            GGame->SetPlayerShip( mpShip );
            camera.SetCameraBehindShip( mpShip );
            Menus::Show(MENU_EMPTY_SOLO);
            */
            StartGameSolo( CurrentGameMode );
            
        }
        else
        {
            StopGameTest();
        }
        mbIsPlaying = !mbIsPlaying;
    }
    if ( keysReleased(SDLK_d) )
    {
        ++CurrentGameMode %= NBGameTypes;
        RecomputeTrack();
    }

    


    if ( keysReleased(SDLK_u) )
    {
		ResetAxis( false );
    }

    if ( keysReleased(SDLK_KP_PLUS) || keysReleased(SDLK_KP_MINUS) )
    {
		bool bEverySegs = KeyShiftDown()||KeyCtrlDown();

		ChangeRoadWidth( bEverySegs, KeyCtrlDown() );

    }

    if ( keysReleased(SDLK_j) )
    {
        mTrackEditIndex--;
        if (mTrackEditIndex<0)
            mTrackEditIndex = MAX_NB_TRACKS-1;
        Tracks[mTrackEditIndex].mbIsDirty = true;
    }

    if ( keysReleased(SDLK_k) )
    {
        mTrackEditIndex ++;
        if (mTrackEditIndex >= MAX_NB_TRACKS )
            mTrackEditIndex = 0;

        Tracks[mTrackEditIndex].mbIsDirty = true;
    }
    if ( keysReleased(SDLK_b) )
    {
        trackBonusInstance_t::bAllBonusActive = !trackBonusInstance_t::bAllBonusActive;
    }

    if ( keysReleased(SDLK_h) )
    {
        // texte en haut + anchors
        // texte haut + help + anchors
        // rien
        // rien + banner
        ++miShowHelp %= 4;
        ggui.clearText();
    }



    if ( keysReleased(SDLK_r) )
    {
        undoRedos[mTrackEditIndex].PreDo();
        trackSeg_t *psegs = Tracks[mTrackEditIndex].segs;
        float trRadius = Tracks[mTrackEditIndex].trackLength /( 2.f * PI );

        // flat
        /*
        for ( int i = 0 ; i < NBSEGS ; i++ )
        {
            float ng = ( ( 2.f * PI ) / (float)NBSEGS ) * (float)i;
            vec_t trpos = vec( cosf(ng) * trRadius, 10.f, sinf(ng) * trRadius );

            psegs[i].point2 = psegs[i].pt = trpos;
            psegs[i].up[0] = vec( 0.f, 1.f, 0.f );
            psegs[i].up[1] = vec( 0.f, 1.f, 0.f );

        }
        */
        /* vers le haut
        for ( int i = 0 ; i < NBSEGS ; i++ )
        {
            float ng = ( ( 2.f * PI ) / (float)NBSEGS ) * (float)i;
            vec_t trpos = vec( 0.f, 10.f + sinf(ng+ 1.5f*PI) * trRadius + trRadius, sinf(ng) * trRadius );

            psegs[i].point2 = psegs[i].pt = trpos;
            psegs[i].up[0] = vec( 0.f, 1.f, 0.f );
            psegs[i].up[1] = vec( 0.f, 1.f, 0.f );

        }
        */
        // coté
        
        for ( int i = 0 ; i < NBSEGS ; i++ )
        {
            float ng = ( ( 2.f * PI ) / (float)NBSEGS ) * (float)i;
            vec_t trpos = vec( cosf(ng) * trRadius, 20.f, sinf(ng) * trRadius );

            psegs[i].point2 = psegs[i].pt = trpos;
            psegs[i].up[1] = psegs[i].up[0] = -normalized(trpos);
            psegs[i].width[0] = psegs[i].width[1] = 12.f;

        }
         psegs[0].up[1] = psegs[0].up[0] = normalized(vec(-1.f, 0.5f, 0.f));
         
        // lemiscate
        /*
        float aConst = 1.0f;
        for ( int i = 0 ; i < NBSEGS ; i++ )
        {
            float ng = ( ( 2.f * PI ) / (float)NBSEGS ) * (float)i;
            
            //vec_t trpos = vec( sqrtf( 2 * trRadius * trRadius *cosf(2*ng) * cos(ng*ng) ), 20.f, sqrtf( 2 * trRadius * trRadius * cosf( 2 * ng )*sin(ng*ng)) );
            vec_t trpos = vec( (aConst *cosf(ng)) / (1.f+sinf(ng)*sinf(ng) ), (sinf(ng)*0.5f + 0.5f)*5.f + 20.f, (aConst * sinf(ng)*cosf(ng)) / (1.f+sinf(ng)*sinf(ng)));
            trpos.x *= trRadius;
            trpos.z *= trRadius;
            
            LOG(" %5.2f %5.2f %5.2f \n", trpos.x, trpos.y, trpos.z);
            psegs[i].up[1] = psegs[i].up[0] = vec(0.f, 1.f, 0.f);
            psegs[i].point2 = psegs[i].pt = trpos;
        }
        Tracks[mTrackEditIndex].ComputeUpForSpeed();
        */
        Tracks[mTrackEditIndex].UpdateSegments();
        undoRedos[mTrackEditIndex].PostDo();                
        Tracks[mTrackEditIndex].mbIsDirty = true;
    }

    if ( KeyCtrlDown() && keysReleased(SDLK_z) ) 
    {
        undoRedos[mTrackEditIndex].UnDo();
    }

    if ( KeyCtrlDown() && keysReleased(SDLK_y) ) 
    {
        undoRedos[mTrackEditIndex].ReDo();
    }

    if ( keysReleased( SDLK_e ) )
    {
        extern bool GBDrawBeacon;
        GBDrawBeacon = !GBDrawBeacon;
    }

    static float timeBeforeSaveInfoHidden = -1.f;

    if ( keysReleased(SDLK_F5) )
    {
        extern serialisableObject_t *GRootSerialisableObjectPtr;
        JSONSerializer jsonser;
        StringToFile( "rushContent.json", jsonser.GenerateJSON( GRootSerialisableObjectPtr ) );
        timeBeforeSaveInfoHidden = 3.f;
    }
	extern bool GDebugDrawPhysic;
	if ( keysReleased(SDLK_F1) )
		GDebugDrawPhysic = !GDebugDrawPhysic;

    timeBeforeSaveInfoHidden -= aTimeEllapsed;

    if ( Tracks[mTrackEditIndex].mbIsDirty )
        RecomputeTrack();

    // infos
    if (miShowHelp < 2)
    {
        char tmps[512];
        sprintf( tmps, "Track #%d//%s//%s//Width %5.2f//H For Help              ", mTrackEditIndex, Tracks[mTrackEditIndex].mName.c_str(), trackBonusInstance_t::bAllBonusActive?"Bonus ON":"Bonus OFF",
            (Tracks[mTrackEditIndex].segs[mAnchorEditIndex].width[0]/12.f)*100.f);
        ggui.putText(0, 35, tmps );

        sprintf( tmps, "Game Mode: %s           ",GameTypes[CurrentGameMode].szName );
        ggui.putText(0, 34, tmps );
    }
    if ( timeBeforeSaveInfoHidden > 0.f)
        ggui.putText(0, 33, " Datas saved! " );    
    else
        ggui.putText(0, 33, "              " );
    
    if (miShowHelp == 1)
    {
        ggui.putText(2, 30, "U : Y up/axe reset");
        ggui.putText(2, 29, "J : Previous Track");
        ggui.putText(2, 28, "K : Next Track");
        ggui.putText(2, 27, "P : Take screenshot" );

        ggui.putText(2, 25, "F : Free ride on/off");
        ggui.putText(2, 24, "G : Start game on/off");
        ggui.putText(2, 23, "D : Change game mode");
        ggui.putText(2, 22, "B : Bonus active on/off");
        ggui.putText(2, 21, "E : Beacon rendered on/off");


        ggui.putText(2, 17, "1 : Gizmo Move");
        ggui.putText(2, 16, "2 : Gizmo Rotate");
        ggui.putText(2, 15, "+/- Increase/dec road width");
        ggui.putText(2, 14, "A : toggle env (for arctic,desert,space)");


        ggui.putText(2, 12, "F5 : Save");
    }

    if ( miShowHelp == 3)
    {
        PutBannerText();
    }

}


void DrawEditor( float aTimeEllapsed )
{
    UNUSED_PARAMETER( aTimeEllapsed );

    if (!GBEditorInited)
        return;
    if (mbIsPlaying)
        return;

    glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
    glLoadIdentity();									// Reset The Projection Matrix
    glMultMatrixf( camera.mProjection.m16 );
    
    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
    glLoadIdentity();		
    glMultMatrixf( camera.mView.m16 );



	//glPopAttrib();

    
    gizmo->SetCameraMatrix( camera.mView.m16, camera.mProjection.m16 );

	
    glDisable(GL_ALPHA_TEST);

    glDisableClientState(GL_COLOR_ARRAY);
	glDisable( GL_LIGHTING );
	glColor4f(1,1,1,1);
    glBindTexture( GL_TEXTURE_2D, 0 );
	
	glColorMask(1,1,1,1);
    glDisable(GL_CULL_FACE);

    glDisable(GL_TEXTURE_2D);

    if ( miShowHelp < 2 )
    {
        DrawAnchorList();

        glPushMatrix();
        gizmo->Draw();
        glPopMatrix();
    }

    //glColorMask(1,1,1,1);
    glColor4f(1,1,1,1);
	/*
	glPushAttrib( GL_ALL_ATTRIB_BITS  );



	// GUI
	glPopAttrib();
	*/
	glActiveTexture( GL_TEXTURE0 );

		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, WIDTH, 0, HEIGHT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	extern int MouseX, MouseY;
	glViewport(0, 0, (int)SCREENWIDTH, (int)SCREENHEIGHT);

	imguiBeginFrame( MouseX, (int)(HEIGHT-MouseY-1), MouseLButDown(), 0 );

	if (showPrefabs)
	{
		static int prefabScrollpf = 0;

		imguiBeginScrollArea("", int(WIDTH-300), 0, 300, (int)(HEIGHT), &prefabScrollpf);

		imguiLabel("Select To Instanciate");


		std::map<std::string, mesh_t*>::const_iterator iter = prefabs.begin();
		for (; iter != prefabs.end(); ++ iter)
		{
			char fileName[512];
			_splitpath( (*iter).first.c_str(), NULL,NULL, fileName, NULL);
			if (imguiItem(fileName))
			{
				// add and instanciate prefab
				mesh_t *pm = (*iter).second->clone();

				matrix_t tr;
				tr.translation( camera.mViewInverse.position - camera.mViewInverse.dir * 50.f );
				pm->mWorldMatrix = tr;
				pm->updateWorldBSphere();
				pm->color = vec(1.f);
				pm->visible = true;
				pm->physic = false;


				// add to track
				track.GetCurrentTrack()->mPrefabs.push_back( prefabInstance_t( pm, (*iter).first, tr ) );
				currentPrefab = &track.GetCurrentTrack()->mPrefabs[track.GetCurrentTrack()->mPrefabs.size()-1];
				showPrefabs = false;
			}
		}
		
		if (imguiButton("Cancel"))
			showPrefabs = !showPrefabs;

		imguiEndScrollArea();

	}
	else
	{
		static int propScroll = 0;
		
		imguiBeginScrollArea("", (int)(SCREENWIDTH-300), 0, 300, (int)(SCREENHEIGHT), &propScroll);
		if (!currentPrefab)
		{
			trackSeg_t& tst = Tracks[mTrackEditIndex].segs[mAnchorEditIndex];

			if (imguiCheck("Has wall", tst.border[0]))
			{
				undoRedos[mTrackEditIndex].PreDo();
				tst.border[0] = tst.border[1] = !tst.border[0];
				Tracks[mTrackEditIndex].UpdateSegments();
				undoRedos[mTrackEditIndex].PostDo();                
				Tracks[mTrackEditIndex].mbIsDirty = true;
			}
			if (imguiCheck("Ground lock", tst.groundLock[0]))
			{
				undoRedos[mTrackEditIndex].PreDo();
				tst.groundLock[0] = tst.groundLock[1] = !tst.groundLock[0];
				Tracks[mTrackEditIndex].UpdateSegments();
				undoRedos[mTrackEditIndex].PostDo();                
				Tracks[mTrackEditIndex].mbIsDirty = true;
			}
			if (imguiCheck("Position Locked", tst.positionLock[0]))
			{
				undoRedos[mTrackEditIndex].PreDo();
				tst.positionLock[0] = tst.positionLock[1] = !tst.positionLock[0];
				Tracks[mTrackEditIndex].UpdateSegments();
				undoRedos[mTrackEditIndex].PostDo();                
				Tracks[mTrackEditIndex].mbIsDirty = true;
			}

		}

		if (imguiButton("Show Prefabs"))
			showPrefabs = !showPrefabs;
		if (imguiButton("Reload prefabs"))
		{
			LoadPrefabs();
			Tracks[mTrackEditIndex].mbIsDirty = true;
		}
		imguiSeparator();
		imguiLabel("Transform");
		// rotate
        if (imguiButton("Move"))//keysReleased(SDLK_1) ) // move
        {
            gizmo = gizmoMove;
			if (currentPrefab)
				gizmo->SetLocation( GizmoWorldMode?IGizmo::LOCATE_WORLD:IGizmo::LOCATE_LOCAL );
			else
				gizmo->SetLocation( IGizmo::LOCATE_WORLD );
        }

        if (imguiButton("Rotate"))//keysReleased(SDLK_2) ) // rotate
        {
            gizmo = gizmoRotate;
			if (currentPrefab)
				gizmo->SetLocation( GizmoWorldMode?IGizmo::LOCATE_WORLD:IGizmo::LOCATE_LOCAL );
			else
				gizmo->SetLocation( IGizmo::LOCATE_LOCAL );
        }
        
        if (imguiButton("Scale"))//keysReleased(SDLK_3) ) // scale
        {
            gizmo = gizmoScale;
			gizmo->SetLocation( IGizmo::LOCATE_LOCAL );
        }
		if (imguiButton(GizmoWorldMode?"World Mode":"Local Mode")) //"Scale")) // gizmo mode
        {
			GizmoWorldMode = !GizmoWorldMode;
			if (currentPrefab)
				gizmo->SetLocation( GizmoWorldMode?IGizmo::LOCATE_WORLD:IGizmo::LOCATE_LOCAL );
			else
			{
				if (gizmo == gizmoMove )
				{
					gizmo->SetLocation( IGizmo::LOCATE_WORLD );
				}
				else 
				{
					gizmo->SetLocation( IGizmo::LOCATE_LOCAL );
				}
			}
		}

		imguiLabel("Track");
		if (imguiButton("Enlarge Point"))
			ChangeRoadWidth( false, true );
		if (imguiButton("Reduce Point"))
			ChangeRoadWidth( false, false );
		if (imguiButton("Enlarge Track"))
			ChangeRoadWidth( true, true );
		if (imguiButton("Reduce Track"))
			ChangeRoadWidth( true, false );
		if (imguiButton("Move Point Up"))
			ChangeRoadHeight( false, true );
		if (imguiButton("Move Point Down"))
			ChangeRoadHeight( false, false );
		if (imguiButton("Move Track Up"))
			ChangeRoadHeight( true, true );
		if (imguiButton("Move Track Down"))
			ChangeRoadHeight( true, false );
		if (imguiButton("Rest Point Axis"))
			ResetAxis( false );
		if (imguiButton("Reset Track Axis"))
			ResetAxis( true );
		/*
		if (imguiButton("Looping"))
		{
			int st0 = (mAnchorEditIndex-1+NBSEGS)%NBSEGS;
			int st1 = mAnchorEditIndex;
			int st2 = (mAnchorEditIndex+1)%NBSEGS;
			int st3 = (mAnchorEditIndex+2)%NBSEGS;
			int st4 = (mAnchorEditIndex+3)%NBSEGS;
			int st5 = (mAnchorEditIndex+4)%NBSEGS;

			track_t& ET =  Tracks[mTrackEditIndex];

			vec_t pos1 = ET.segs[st1].pt;
			vec_t difDir = ET.segs[st1].pt - ET.segs[st0].pt;

			vec_t roadRight;
			roadRight.cross( normalized(difDir), ET.segs[st0].up[0]);
			roadRight.normalize();

			float lenh = difDir.length();// * cosf(ZPI * 0.5f);
			vec_t loopDir = roadRight * lenh;
			vec_t midLooping = pos1 + loopDir;//vec(0.f, lenh , 0.f);//* 0.5f;
			float rightLen = 5.f;
			ET.segs[st1].positionLock[0] = true;
			ET.segs[st2].positionLock[0] = true;
			ET.segs[st3].positionLock[0] = true;
			ET.segs[st4].positionLock[0] = true;
			ET.segs[st2].point2 = ET.segs[st2].pt = roadRight * 1.f * rightLen + pos1 + (difDir+loopDir );// * 0.5f; 
			ET.segs[st3].point2 = ET.segs[st3].pt = roadRight * 2.f * rightLen + pos1 + (loopDir ) * 2.f;
			ET.segs[st4].point2 = ET.segs[st4].pt = roadRight * 3.f * rightLen + pos1 + (-difDir+loopDir );// * 0.5f;
			ET.segs[st5].point2 = ET.segs[st5].pt = roadRight * 4.f * rightLen + pos1 ;

			ET.segs[st2].up[0] = (midLooping - ET.segs[st2].point2).normalize();
			ET.segs[st3].up[0] = (midLooping - ET.segs[st3].point2).normalize();
			ET.segs[st4].up[0] = (midLooping - ET.segs[st4].point2).normalize();
			ET.segs[st5].up[0] = (midLooping - ET.segs[st5].point2).normalize();


			Tracks[mTrackEditIndex].mbIsDirty = true;
			ET.UpdateSegments();
			ET.segs[st1].positionLock[0] = false;
			ET.segs[st2].positionLock[0] = false;
			ET.segs[st3].positionLock[0] = false;
			ET.segs[st4].positionLock[0] = false;
		}
		*/
		imguiLabel("Config");
		
		config_t config = GetEngineConfig();
		camera.mSensibility = config.mouseSensibility;
		if (imguiSlider("Mouse sensibility",&camera.mSensibility, 0.01f, 1.f, 0.01f, true))
		{
			config.mouseSensibility = camera.mSensibility;
			SetEngineConfig( config );
		}
		
		
		
		

		imguiEndScrollArea();
	}

	imguiEndFrame();
	glDisable(GL_DEPTH_TEST);
	glDepthMask(0);
	imguiRenderGLDraw();
	glDepthMask(1);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glColor4f(1,1,1,1);
	//glPushAttrib( GL_ALL_ATTRIB_BITS  );
}

#endif  //  EDITOR_ENABLE
