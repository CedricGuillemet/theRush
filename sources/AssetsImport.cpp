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

#include "define_platform.h"

#if IS_OS_LINUX

#include <string.h>

#elif IS_OS_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     1   // Exclude rarely-used stuff from Windows headers
#endif

#include <windows.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <list>
#include <vector>
#include <map>
#include <string>

#include "define_macros.h"
#include "define_types.h"
#include "debug_common.h"
#include "debug_assert.h"

#include "maths.h"
#include "toolbox.h"

#include "include_GL.h"


typedef struct shipsMeshVertex_t
{
    float x,y,z;
    float nx, ny, nz;
    u32 col;
    void normalize()
    {
        vec_t n = vec(nx, ny, nz, 0.f);
        n.normalize();
        nx = n.x; ny = n.y; nz = n.z;
    }
	void transform(const matrix_t& mt)
	{
		vec_t v = vec(x,y,z,0.f);
		v.TransformPoint( mt );
		x = v.x; y = v.y; z = v.z;

		vec_t n = vec(nx, ny, nz, 0.f);


        n.TransformVector( mt);
        n.normalize();
        nx = n.x; ny = n.y; nz = n.z;


	}

    void write(FILE *fp)
    {
        fwrite(&x, sizeof(float), 1, fp );
        fwrite(&y, sizeof(float), 1, fp );
        fwrite(&z, sizeof(float), 1, fp );

        short tempnorm;
        tempnorm = static_cast<short>(nx * 32767.f);
        fwrite( &tempnorm, sizeof(short), 1, fp);
        tempnorm = static_cast<short>(ny * 32767.f);
        fwrite( &tempnorm, sizeof(short), 1, fp);
        tempnorm = static_cast<short>(nz * 32767.f);
        fwrite( &tempnorm, sizeof(short), 1, fp);
        /*
        vec_t colf;
        colf.fromUInt32(col);
        short col5551 = colf.toUInt5551();
        fwrite(&col5551, sizeof(unsigned short), 1, fp);
        */
        fwrite(&col, sizeof(u32), 1, fp);
    }
} shipsMeshVertex_t;

/*
std::vector<shipsMeshVertex_t> importVertices;
std::vector<unsigned short> importIndices;
std::vector<vec_t> importTmpVertices;
*/

std::string dumpToHexaBytes(u8 *ptr, int nb)
{
    std::string res = "";
    for (int i = 0; i< ((nb&0xFFFFF8)+1); i++)
    {
        if (i<nb)
        {
            char tmp[128];
            sprintf( tmp, "0x%02x", ptr[i] );
            res += tmp;
            if ( (i+1) < nb)
                res += ",";
            if ((i&7) == 7)
                res += "\n";
        }
    }
    return res;
}
serialisableObject_t * GRootSerialisableObjectPtr;


int GetUse( std::vector<u16>& idx, u16 us[3] )
{
    bool bUsed[] = {false, false, false};

    int iLowestIndex = 999;
    int iHighest = -1;
    int nbUsed = 0;
    const int idxCount = idx.size();

    for (int i=0;i<3;i++)
    {
        for (int j = 0; j < idxCount; ++j)
        {
            if ( idx[j] == us[i])
            {
                if ( j<iLowestIndex)
                    iLowestIndex = j;
                if (j>iHighest)
                    iHighest = j;

                nbUsed ++;
                bUsed[i] = true;
                break;
            }
        }
    }
    // count used ones
    switch( nbUsed)
    {
    case 0:
        return 0; // no match
    case 1:
        return 1; // only one, need another pass
    case 2:
        // match, insert inside vector
        {

            for (int i= 0;i<3;i++)
            {
                if ( !bUsed[i])
                {
                    int middleMan = us[i];
                    if (iLowestIndex == 0 && iHighest == (idx.size()-1) )
                        idx.insert( idx.begin()+ iHighest+1, middleMan );
                    else
                        idx.insert( idx.begin()+ iLowestIndex+1, middleMan );
                    return 2;

                }
            }
        }
        ASSERT_TOOL(0);// should have return before
    case 3:
        return 0;
        break;
        //ASSERT(0);
        // should not be possible
    }
    ASSERT_TOOL(0);
    return 0;
}



void importShipsV2ASE(const char *szASEName, const char *szBINName)
{
    setlocale(LC_ALL, "C");


			matrix_t mtRot;
			mtRot.rotationX(-PI*0.5f);

std::vector<shipsMeshVertex_t> importVertices;
std::vector<u32> importIndices;
std::vector<vec_t> importTmpVertices;
std::vector<vec_t> importTmpNorm;
std::vector<vec_t> animsRot, animsPos;
std::vector<u8> animRotFrame, animsPosFrame;
std::vector<u32> importColor;
std::vector<int> importIndexConv;

vec_t lastParsedPosition, lastParsedDirection;

// reacteur & valve

float maxRadius = 0.f;
vec_t animTrans;



    FILE *fp = fopen(szASEName,"rt");
    if (!fp)
            return;

    char curline[1024];
    std::string meshname="";
    int faceAv = 0;
    int nrmFaceAv = 0;
    int meshAv = 0;
                int faceuvnb = 0;
                int cfaceav = 0;


	FILE *fpo = fopen(szBINName,"wb");

	/*
    fputs("#include \"maths.h\"\ntypedef struct ASEMesh_t\n", fpo);
    fputs("{\n", fpo);
    fputs("int mNbVertices;\n", fpo);
    fputs("u8 *mVertices;\n", fpo);
    fputs("int mNbIndices;\n", fpo);
    fputs("u8 *mIndices;\n", fpo);
    fputs("} ASEMesh_t;\n\n", fpo);
	*/

    //fputs("ASEMesh_t ASEShips[]={\n", fpo);
    float tx = 0.f, ty=0.f, tz=0.f;
    while ( 1 )
    {
        char *getsStatus = fgets(curline, 1024, fp);
        std::string str = curline;
        size_t found;


        bool bWriteDown = (str.find("*GEOMOBJECT")!=-1)||(!getsStatus);
        //if ( (found = str.find("*PROP_RECVSHADOW")) != -1)
        if (bWriteDown)
        {

            lastParsedPosition.TransformPoint( mtRot );
            lastParsedDirection.TransformVector( mtRot );

            maxRadius = 0.f;



            if (importIndices.empty())
                continue;

			for (unsigned int k = 0;k< importVertices.size() ; k++)
				importVertices[k].transform(mtRot);

            u32 nbv;

            {
                if ( importIndices.size() == 6)
                {
                    int makeabreak = 1;
                }

                for (int ipv = 0;ipv< importVertices.size(); ipv++)
                {
                    //importVertices[ipv].col = importColor[ipv];
                }

			    // optimize
                /*
			    int newNbVts = OptimizeMesh( sizeof( shipsMeshVertex_t ),
				    (u8*)&importVertices[0],
				    importVertices.size(),
				    sizeof( shipsMeshVertex_t ),
				    &importIndices[0],
				    importIndices.size() );
                    */
                 int newNbVts = importVertices.size();
                printf(" before : %d after : %d -> ", importVertices.size(), newNbVts);
			    /// write

			    fwrite( meshname.c_str(), 1, meshname.length() +1, fpo );
			    nbv = newNbVts;//importVertices.size();
			    fwrite( &nbv, sizeof(u32), 1, fpo );
			    //fwrite(&importVertices[0], sizeof( shipsMeshVertex_t ), nbv, fpo );

                for (int ipv = 0;ipv<nbv; ipv++)
                {
                    importVertices[ipv].nx = importTmpNorm[importIndexConv[ipv]].x;
				    importVertices[ipv].ny = importTmpNorm[importIndexConv[ipv]].y;
				    importVertices[ipv].nz = importTmpNorm[importIndexConv[ipv]].z;


                    importVertices[ipv].normalize();
                    //importVertices[ipv].col = 0x80808080;
                    importVertices[ipv].write( fpo );
                }



            }


            if (nbv< 256)
                printf(" u8 possible\n");
            else
                 if (nbv< 65536)
                    printf(" u16 possible\n");
                 else
                     printf(" u32!\n");

			u32 nbi = importIndices.size();
			fwrite( &nbi, sizeof(u32), 1, fpo );
            if (nbv< 256)
            {
                for (int win = 0; win<importIndices.size();win++)
                {
                    u8 idx2w = importIndices[win];
                    fwrite(&idx2w, sizeof(u8), 1, fpo);
                }
            }
            else
            {
                if (nbv< 65536)
                {
                    for (int win = 0; win<importIndices.size();win++)
                    {
                        u16 idx2w = importIndices[win];
                        fwrite(&idx2w, sizeof(u16), 1, fpo);
                    }
                }
                else
                {
                    fwrite(&importIndices[0], sizeof( u32 ), importIndices.size(), fpo );
                }
            }



            u16 padding = 0;
            fwrite( &padding, sizeof(u16), 1, fpo );

            meshAv++;
            /*
            mesh_t *nmesh = pushNewMesh(&importIndices[0], importIndices.size(), &importVertices[0], VAF_XYZ|VAF_COLOR|VAF_NORMAL, importVertices.size());
            nmesh->computeBSphere();
            nmesh->mWorldMatrix.identity();
            nmesh->updateWorldBSphere();

            nmesh->visible = true;
            nmesh->color = vec(1.f);
            */


            if (!getsStatus)
            {
                goto finishedImporting;
            }
        }
        else
        if ( (found = str.find("*NODE_NAME \"")) != -1)
        {
                meshname = str.substr(13, str.size()-13-2);
                if (meshname[0] == '\"')
                        meshname = meshname.substr(1, 100);

                //printf("// %s\n", meshname.c_str() );
                faceAv = 0;
                faceuvnb = 0;
                nrmFaceAv = 0;
                cfaceav = 0;
                tx = 0.f;
                ty = 0.f;
                tz = 0.f;
                importTmpVertices.clear();
                importTmpNorm.clear();

				printf("mesh : %s\n", meshname.c_str() );



        }
        else if ((found = str.find("*MESH_CFACELIST")) != -1)
        {
            // do not remove!
        }
        else if ((found = str.find("*MESH_CFACE")) != -1)
        {
            int nbVt;
            int x,y,z;
            const char *pparse = str.c_str()+14;
            int nbread = sscanf(pparse, "%*[ \t]%d%*[ \t]%d%*[ \t]%d%*[ \t]%d", &nbVt, &x, &y, &z);

            if (x < importColor.size() )
            {
            importVertices[cfaceav++].col = importColor[x];
            importVertices[cfaceav++].col = importColor[x];
            importVertices[cfaceav++].col = importColor[x];
            }

        }
        else if ((found = str.find("*MESH_FACENORMAL")) != -1)
        {
        }
        else if ((found = str.find("*MESH_VERTEX_LIST")) != -1)
        {
        }
        else if ( (found = str.find("*MESH_FACE_LIST")) != -1)
        {
        }
        else if ( (found = str.find("*MESH_FACE")) != -1)
        {
            int ii[3];
            const char *pparse = str.c_str()+20;
            int nbread = sscanf( pparse, "%*[ \t]A:%*[ \t]%d%*[ \t]B:%*[ \t]%d%*[ \t]C:%*[ \t]%d%*[ \t]%d", &ii[0], &ii[1], &ii[2] );
            //printf("// i %d %d %d\n", ii[0], ii[1], ii[2] );

            if ( faceAv >3000)
            {
                int a = 1;
            }
            for (int i=0;i<3;i++)
            {
                //if (faceAv<importVertices.size())
                {
				    importVertices[faceAv].x = importTmpVertices[ii[i]].x-tx;// + tx;
				    importVertices[faceAv].y = importTmpVertices[ii[i]].y-ty;// + ty;
				    importVertices[faceAv].z = importTmpVertices[ii[i]].z-tz;// + tz;

                    importIndexConv.push_back(ii[i]);
                    /*
				    importVertices[faceAv].nx = importTmpNorm[ii[i]].x;
				    importVertices[faceAv].ny = importTmpNorm[ii[i]].y;
				    importVertices[faceAv].nz = importTmpNorm[ii[i]].z;
                    */
                    faceAv++;
                }
            }
        }
        else if ( (found = str.find("*MESH_NUMFACES")) != -1)
        {
            int nbFaces;
            const char *pparse = str.c_str()+16;
            sscanf( pparse, "%d", &nbFaces );
            //printf("// %d faces\n", nbFaces);

            importVertices.resize( nbFaces*3 );
            importIndices.resize( nbFaces*3 );
            importColor.resize( nbFaces*3 );
            importTmpNorm.resize( nbFaces*3 );
            importIndexConv.clear();
            for (int i=0;i<nbFaces*3;i++)
            {
                importIndices[i] = i;
                importVertices[i].nx = importVertices[i].ny = importVertices[i].nz = 0.f;
                importTmpNorm[i] = vec(0.f);
            }
        }
        else
        if ( (found = str.find("*MESH_VERTEXNORMAL")) != -1)
        {
            int nbVt;
            float nx, ny, nz;
            const char *pparse = str.c_str()+22;
            int nbread = sscanf( pparse, "%*[ \t]%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &nx, &ny, &nz);
            //printf("// n %5.4f %5.4f %5.4f\n", nx, ny, nz );
            /*
            if (nrmFaceAv >= importVertices.size())
            {
                int a = 1;
            }
            */

            importTmpNorm[nbVt] += vec(nx,ny,nz);


            //importTmpNorm.push_back( vec(nx,ny,nz) );
            //nrmFaceAv++;
        }
        else
        if ( (found = str.find("*MESH_VERTEX")) != -1)
        {
            int nbVt;
            float x,y,z;
            const char *pparse = str.c_str()+15;
            int nbread = sscanf(pparse, "%*[ \t]%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &x, &y, &z);
            vec_t meshVt = vec(x,y,z, 0.f);
            float nRadius = (meshVt-lastParsedPosition).length();
            maxRadius = (maxRadius>nRadius)?maxRadius:nRadius;
            //printf("// vt %5.4f %5.4f %5.4f\n", x,y,z);
            /*
            importTmpVertices[nbVt].x = x;
            importTmpVertices[nbVt].y = y;
            importTmpVertices[nbVt].z = z;
            */
            importTmpVertices.push_back( meshVt );
        }
        else
        if ( (found = str.find("*MESH_VERTCOL")) != -1)
        {
            int nbVt;
            float r,g,b;
            const char *pparse = str.c_str()+17;
            int nbread = sscanf( pparse, "%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &r, &g, &b);
            //printf("// rgb %5.4f %5.4f %5.4f\n", r,g,b );
            bool bFullWhite = ((r>0.95f)&&(g>0.95f)&&(b>0.95f));

			float alpha = bFullWhite?1.f:0.5f;
			u32 col = vec(r,g,b,alpha).toUInt32();
			if (nbVt < importVertices.size())
				importColor[nbVt] = col;


        }
        else
        if ( (found = str.find("*TM_ROW3")) != -1)
        {

            const char *pparse = str.c_str()+10;
            int nbread = sscanf( pparse, "%*[ \t]%f%*[ \t]%f%*[ \t]%f", &tx, &ty, &tz );
            lastParsedPosition = vec( tx, ty, tz );



            //printf("// tr %5.4f %5.4f %5.4f\n", tx, ty, tz );
        }
        else if ( (found = str.find("*TM_ROW2")) != -1)
        {
            const char *pparse = str.c_str()+10;
            int nbread = sscanf( pparse, "%*[ \t]%f%*[ \t]%f%*[ \t]%f", &tx, &ty, &tz );

            ///lastParsedDirection = vec( -tx, tz, ty);

        }
    }
    fclose(fp);
finishedImporting:	;



    //fputs("", fpo);
	fclose(fpo);

}

// build batiments
struct batiment
{
    batiment()
    {
    }
    batiment(const batiment& bat)
    {
        height = bat.height;
        const int batIdxCount = bat.idx.size();
        for (int i = 0; i < batIdxCount; ++i)
            idx.push_back( bat.idx[i] );
        int a = 1;
    }
    float height;
    std::vector<unsigned short> idx;
};

void importASECity( const char *szASEName )
{
    setlocale(LC_ALL, "C");

    std::vector<vec_t> importVertices;
    std::vector<unsigned short> importIndices;



    FILE *fp = fopen(szASEName,"rt");
    if (!fp)
            return;

    char curline[1024];
    std::string meshname="";
    int faceAv = 0;
    int nrmFaceAv = 0;
    int meshAv = 0;
    int faceuvnb = 0;


	FILE *fpo = fopen("Bin/Datas/Meshes/city.bin","wb");

    float tx = 0.f, ty=0.f, tz=0.f;
    while ( fgets(curline, 1024, fp) )
    {
        std::string str = curline;
        size_t found;

        if ( (found = str.find("*PROP_RECVSHADOW")) != -1)
        {
			matrix_t mtRot;
			mtRot.rotationX(-PI*0.5f);

			for (unsigned int k = 0;k< importVertices.size() ; k++)
				importVertices[k].transform(mtRot);

            std::vector<batiment> batiments;

            const int importIndexCount = importIndices.size();
            ASSERT_TOOL_MSG( importIndexCount%3 == 0, "Imported Index count (%d) should be a multiple of 3.", importIndexCount );

            for (int i = 0; i < importIndexCount; i+=3)
            {
                u16 toImport[]  ={ importIndices[i], importIndices[i+1], importIndices[i+2] };

                bool bAssociated = false;

                const int batimentCount = batiments.size();
                for (int j = 0; j < batimentCount; ++j)
                {
                    std::vector<unsigned short>& v =  batiments[j].idx;

                    int ret = GetUse( v, toImport );
                    if (ret == 1)
                    {
                        /*importIndices.push_back( toImport[0] );
                        importIndices.push_back( toImport[1] );
                        importIndices.push_back( toImport[2] );
                        */
                        bAssociated = true;
                        break;

                    }
                    else if (ret == 2)
                    {
                        bAssociated = true;
                        break;
                    }

                }

searchEnd:;
                if ( !bAssociated )
                {
                    // new batiment

                    batiment bat;
                    bat.idx.push_back( toImport[0] );
                    bat.idx.push_back( toImport[1] );
                    bat.idx.push_back( toImport[2] );

                    bat.height = importVertices[toImport[0]].y;
                    batiments.push_back( bat );
                }

            }
            // dump- check

            int acount[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int totalSize = 2; // nbBats
            unsigned short nbBats = batiments.size();
            fwrite( &nbBats, sizeof(unsigned short), 1, fpo );

            const int batimentCount = batiments.size();
            for (int  i = 0; i < batimentCount; ++i)
            {
                totalSize += 5;// nbsegs(u8) + height;
                fwrite( &batiments[i].height, sizeof(float), 1, fpo );
                u8 nbpt = batiments[i].idx.size();
                fwrite( &nbpt, sizeof(u8), 1, fpo);

                vec_t dif1 = importVertices[batiments[i].idx[1]]-importVertices[batiments[i].idx[0]];
                vec_t dif2 = importVertices[batiments[i].idx[2]]-importVertices[batiments[i].idx[1]];
                dif1.normalize();
                dif2.normalize();
                vec_t crs;

                 bool invDir = false;

                  crs.cross( dif1, dif2 );

                if (crs.y < 0.f)
                    invDir = true;


                for (int j = 0;j<nbpt;j++)
                {
                    int ii0 = batiments[i].idx[invDir?(nbpt-1-j) : j];
                    const vec_t& vtx = importVertices[ii0];
                    /*fwrite( &vtx.x, sizeof(float), 1, fpo);
                    fwrite( &vtx.z, sizeof(float), 1, fpo);
                    */
                    fixed816_t fixedx(vtx.x);
                    fwrite( &fixedx, sizeof(fixed816_t), 1, fpo);

                    fixed816_t fixedz(vtx.z);
                    fwrite( &fixedz, sizeof(fixed816_t), 1, fpo);

                }
                acount[nbpt] ++;
                totalSize += nbpt * 8;
            }

            int a = 1;

        }
        else if ((found = str.find("*MESH_FACENORMAL")) != -1)
        {
        }
        else if ((found = str.find("*MESH_VERTEX_LIST")) != -1)
        {
        }
        else if ( (found = str.find("*MESH_FACE_LIST")) != -1)
        {
        }
        else if ( (found = str.find("*MESH_FACE")) != -1)
        {
            int ii[3];
            const char *pparse = str.c_str()+20;
            int nbread = sscanf( pparse, "%*[ \t]A:%*[ \t]%d%*[ \t]B:%*[ \t]%d%*[ \t]C:%*[ \t]%d%*[ \t]%d", &ii[0], &ii[1], &ii[2] );
            importIndices.push_back( ii[0] );
            importIndices.push_back( ii[1] );
            importIndices.push_back( ii[2] );
            //printf(" %d %d %d\n", ii[0], ii[1], ii[2]);
        }
        else if ( (found = str.find("*MESH_NUMFACES")) != -1)
        {
            int nbFaces;
            const char *pparse = str.c_str()+16;
            sscanf( pparse, "%d", &nbFaces );
        }
        else
        if ( (found = str.find("*MESH_VERTEX")) != -1)
        {
            int nbVt;
            float x,y,z;
            const char *pparse = str.c_str()+16;
            int nbread;
            if (importVertices.size() >=999)
                nbread = sscanf(pparse, "%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &x, &y, &z);
            else
                nbread = sscanf(pparse, "%*[ \t]%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &x, &y, &z);
            importVertices.push_back( vec(x,y,z, 0.f) );
            //printf(" %f %f %f\n", x,y,z);
        }
        else
        if ( (found = str.find("*TM_ROW3")) != -1)
        {

            const char *pparse = str.c_str()+10;
            int nbread = sscanf( pparse, "%*[ \t]%f%*[ \t]%f%*[ \t]%f", &tx, &ty, &tz );
			//if (shipsColorManagement)
			{
				tx=ty=tz = 0.f;
			}
        }
    }
    fclose(fp);


    fputs("};", fpo);
	fclose(fpo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct placeHolder
{
    placeHolder( const vec_t& p, const vec_t& d, float r)
    {
        position = p;
        direction = d;
        radius = r;
    }
    vec_t position;
    vec_t direction;
    float radius;
};

void writePlaceHolders( const std::vector<placeHolder>& plc, FILE *fpo)
{
    u8 nbReactors = plc.size();
    fwrite ( &nbReactors, sizeof(u8), 1, fpo);

    const int plcCount = plc.size();
    for (int i = 0; i < plcCount; ++i)
    {
        fwrite (& plc[i].position.x, sizeof(float), 3, fpo);
        fwrite (& plc[i].direction.x, sizeof(float), 3, fpo);
        fwrite (& plc[i].radius, sizeof(float), 1, fpo);
    }
}

void writePipes( const std::vector<std::vector< vec_t > >& plc, FILE *fpo)
{
    u8 nbPipes = plc.size()>>1;
    fwrite ( &nbPipes, sizeof(u8), 1, fpo);

    const int plcCount = plc.size();
    for (int i = 0; i< plcCount; ++i)
    {
        u8 nbPipesVts = plc[i].size();
        if (!nbPipesVts)
            continue;

        fwrite ( &nbPipesVts, sizeof(u8), 1, fpo);
        for (int j = 0;j<nbPipesVts;j++)
        {
            fwrite (& plc[i][j].x, sizeof(float), 3, fpo);
        }
    }
}

struct pubVertex
{
    float x,y,z;
    float u,v;

    void write( FILE *fp )
    {
        fwrite(&x, sizeof(float)*3, 1, fp);

        short nu = static_cast<short>(u * 32767.f );
        short nv = static_cast<short>(v * 32767.f );

        fwrite(&nu, sizeof(short), 1, fp );
        fwrite(&nv, sizeof(short), 1, fp );
    }
};

void importASE(const char *szASEName, const char *szBINName, bool shipsColorManagement, bool usePubVertex = false)
{
    setlocale(LC_ALL, "C");


			matrix_t mtRot;
			mtRot.rotationX(-PI*0.5f);

std::vector<shipsMeshVertex_t> importVertices;
std::vector<unsigned short> importIndices;
std::vector<vec_t> importTmpVertices;
std::vector<vec_t> importTmpUV;
std::vector<vec_t> importUV;

std::vector<vec_t> animsRot, animsPos;
std::vector<u8> animRotFrame, animsPosFrame;

vec_t lastParsedPosition, lastParsedDirection;

// reacteur & valve


std::vector<placeHolder> reactors;
std::vector<placeHolder> valves;

std::vector< std::vector<vec_t> > pipes;

float maxRadius = 0.f;
vec_t animTrans;

    std::vector<pubVertex> importPubVertices;


    FILE *fp = fopen(szASEName,"rt");
    if (!fp)
            return;

    char curline[1024];
    std::string meshname="";
    int faceAv = 0;
    int nrmFaceAv = 0;
    int meshAv = 0;
                int faceuvnb = 0;



	FILE *fpo = fopen(szBINName,"wb");

	/*
    fputs("#include \"maths.h\"\ntypedef struct ASEMesh_t\n", fpo);
    fputs("{\n", fpo);
    fputs("int mNbVertices;\n", fpo);
    fputs("u8 *mVertices;\n", fpo);
    fputs("int mNbIndices;\n", fpo);
    fputs("u8 *mIndices;\n", fpo);
    fputs("} ASEMesh_t;\n\n", fpo);
	*/

    //fputs("ASEMesh_t ASEShips[]={\n", fpo);
    float tx = 0.f, ty=0.f, tz=0.f;
    while ( 1 )
    {
        char *getsStatus = fgets(curline, 1024, fp);
        std::string str = curline;
        size_t found;


        bool bWriteDown = (str.find("*GEOMOBJECT")!=-1)||(!getsStatus);
        //if ( (found = str.find("*PROP_RECVSHADOW")) != -1)
        if (bWriteDown)
        {




            lastParsedPosition.TransformPoint( mtRot );
            lastParsedDirection.TransformVector( mtRot );

            bool isReactor = ( strstr(meshname.c_str(), "shipreactor" ) != NULL );
            bool isValve = ( strstr(meshname.c_str(), "valve" ) != NULL );
            bool isPipe = ( strstr(meshname.c_str(), "pipe" ) != NULL );

            if (isReactor)
                reactors.push_back( placeHolder( lastParsedPosition, lastParsedDirection, maxRadius ) );
            if (isValve)
                valves.push_back( placeHolder( lastParsedPosition, lastParsedDirection, maxRadius ) );

            maxRadius = 0.f;

            if ( isReactor || isValve || isPipe)
            {
                if (!getsStatus)
                {
                    goto finishedImporting;
                }
                continue;
            }

            if (importIndices.empty())
                continue;

			for (unsigned int k = 0;k< importVertices.size() ; k++)
				importVertices[k].transform(mtRot);

            u32 nbv;
            if (!usePubVertex)
            {
                if ( importIndices.size() == 6)
                {
                    int makeabreak = 1;
                }

			    // optimize
			    int newNbVts = OptimizeMesh( sizeof( shipsMeshVertex_t ),
				    (u8*)&importVertices[0],
				    importVertices.size(),
				    sizeof( shipsMeshVertex_t ),
				    &importIndices[0],
				    importIndices.size() );
                printf(" before : %d after : %d -> ", importVertices.size(), newNbVts);
			    /// write

			    fwrite( meshname.c_str(), 1, meshname.length() +1, fpo );
			    nbv = newNbVts;//importVertices.size();
			    fwrite( &nbv, sizeof(u32), 1, fpo );
			    //fwrite(&importVertices[0], sizeof( shipsMeshVertex_t ), nbv, fpo );

                const int vertexCount = nbv;
                for (int ipv = 0; ipv < vertexCount; ++ipv)
                {
                    importVertices[ipv].write( fpo );
                }



            }
            else
            {
                const int importVertexCount = importVertices.size();
                for (int pi = 0; pi < importVertexCount; ++pi)
                {
                    importPubVertices[pi].x = importVertices[pi].x;
                    importPubVertices[pi].y = importVertices[pi].y;
                    importPubVertices[pi].z = importVertices[pi].z;
                    importPubVertices[pi].u = importUV[pi].x;
                    importPubVertices[pi].v = importUV[pi].y;
                }
			    // optimize    std::vector<pubVertex> importPubVertices;
			    int newNbVts = OptimizeMesh( 12/*sizeof( pubVertex )*/,
				    (u8*)&importPubVertices[0],
				    importPubVertices.size(),
				    sizeof( pubVertex ),
				    &importIndices[0],
				    importIndices.size() );
                printf(" before : %d after : %d -> ", importVertices.size(), newNbVts);
			    /// write

			    //fwrite( meshname.c_str(), 1, meshname.length() +1, fpo );
			    nbv = newNbVts;//importVertices.size();
			    fwrite( &nbv, sizeof(u32), 1, fpo );
			    //fwrite(&importPubVertices[0], sizeof( shipsMeshVertex_t ), nbv, fpo );

                const int vertexCount = nbv;
                for (int ipv = 0; ipv < vertexCount; ++ipv)
                    importPubVertices[ipv].write(fpo);


            }


            if (nbv < U8_MAX)
                printf(" u8 possible\n");
            else
                printf(" u16 needed\n");

			u32 nbi = importIndices.size();
			fwrite( &nbi, sizeof(u32), 1, fpo );

            if (nbv < U8_MAX)
            {
                const int ImportIndexCount = importIndices.size();
                for (int win = 0; win < ImportIndexCount; ++win)
                {
                    const unsigned short idx = importIndices[win];
                    ASSERT_TOOL_MSG( idx < U8_MAX, "Imported index (%d) should be smaller than %d in order to be stored as a u8 (one byte).", idx, U8_MAX );
                    u8 idx2w = static_cast<u8>(idx);
                    fwrite(&idx2w, sizeof(u8), 1, fpo);
                }
            }
            else
            {
                fwrite(&importIndices[0], sizeof( unsigned short ), importIndices.size(), fpo );
            }

            // anims
            u8 nbanims = animsPos.size();
            fwrite( &nbanims, sizeof(u8), 1, fpo );

            // local matrix
            if ( usePubVertex && animsPos.empty() )
            {
                animTrans.TransformPoint( mtRot );
                fwrite( &animTrans.x, sizeof(float)*3, 1, fpo);
            }

            const int animPosCount = animsPos.size();
            for (int pa = 0; pa < animPosCount; ++pa)
            {
                fwrite(&animsPosFrame[pa], sizeof(u8), 1, fpo );
                //animsPos[pa] += animTrans;
                animsPos[pa].w = 0.f;
                animsPos[pa].TransformPoint( mtRot );
                fwrite(&animsPos[pa], sizeof( float )*3, 1, fpo );
            }
            nbanims = animsRot.size();
            fwrite( &nbanims, sizeof(u8), 1, fpo );

            const int animRotCount = animsRot.size();
            for (int pa = 0; pa < animRotCount; ++pa)
            {
                fwrite(&animRotFrame[pa], sizeof(u8), 1, fpo );
                vec_t rotAxe = vec(animsRot[pa].x, animsRot[pa].y, animsRot[pa].z, 0.f);

                rotAxe.TransformVector( mtRot );

                fwrite(&rotAxe.x, sizeof( float )*3, 1, fpo );
                fwrite(&animsRot[pa].w, sizeof( float ), 1, fpo );
            }

            meshAv++;

            if (!getsStatus)
            {
                goto finishedImporting;
            }
        }
        else
        if ( (found = str.find("*NODE_NAME \"")) != -1)
        {
                meshname = str.substr(13, str.size()-13-2);
                if (meshname[0] == '\"')
                        meshname = meshname.substr(1, 100);

                //printf("// %s\n", meshname.c_str() );
                faceAv = 0;
                faceuvnb = 0;
                nrmFaceAv = 0;
                tx = 0.f;
                ty = 0.f;
                tz = 0.f;
                importTmpVertices.clear();
                animsRot.clear();
                animsPos.clear();
                animRotFrame.clear();
                animsPosFrame.clear();
                importTmpUV.clear();
				printf("mesh : %s\n", meshname.c_str() );

				if (strstr(meshname.c_str(), "pipe"))
				{
                    pipes.resize(pipes.size()+1);
				}
        }
        else if ((found = str.find("*MESH_FACENORMAL")) != -1)
        {
        }
        else if ((found = str.find("*MESH_VERTEX_LIST")) != -1)
        {
        }
        else if ( (found = str.find("*MESH_FACE_LIST")) != -1)
        {
        }
        else if ( (found = str.find("*SHAPE_VERTEX_KNOT")) != -1)
        {
            int nbVt;
            float x,y,z;
            const char *pparse = str.c_str()+20;
            int nbread = sscanf(pparse, "%*[ \t]%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &x, &y, &z);
            vec_t meshVt = vec(x,y,z, 0.f);
            meshVt.TransformPoint( mtRot );
            pipes[pipes.size()-1].push_back( meshVt );
        }
        else if ( (found = str.find("*MESH_FACE")) != -1)
        {
            int ii[3];
            const char *pparse = str.c_str()+20;
            int nbread = sscanf( pparse, "%*[ \t]A:%*[ \t]%d%*[ \t]B:%*[ \t]%d%*[ \t]C:%*[ \t]%d%*[ \t]%d", &ii[0], &ii[1], &ii[2] );
            //printf("// i %d %d %d\n", ii[0], ii[1], ii[2] );
            for (int i=0;i<3;i++)
            {
				importVertices[faceAv].x = importTmpVertices[ii[i]].x + tx;
				importVertices[faceAv].y = importTmpVertices[ii[i]].y + ty;
				importVertices[faceAv].z = importTmpVertices[ii[i]].z + tz;
                faceAv++;
            }
        }
        else if ( (found = str.find("*MESH_TFACE")) != -1)
        {
            int ii[3];
            int tmpface;
            const char *pparse = str.c_str()+15;
            int nbread = sscanf( pparse, "%d%*[ \t]%d%*[ \t]%d%*[ \t]%d", &tmpface, &ii[0], &ii[1], &ii[2] );
            //printf("// i %d %d %d\n", ii[0], ii[1], ii[2] );
            if (nbread > 0)
            for (int i=0;i<3;i++)
            {
				importUV[faceuvnb].x = importTmpUV[ii[i]].x;
				importUV[faceuvnb].y = importTmpUV[ii[i]].y;
                faceuvnb++;
            }
        }
        else if ( (found = str.find("*CONTROL_ROT_SAMPLE")) != -1)
        {
            int ipos;
            float rx,ry,rz,rw;
            const char *pparse = str.c_str()+23;
            int nbread = sscanf( pparse, "%d%*[ \t]%f%*[ \t]%f%*[ \t]%f%*[ \t]%f", &ipos, &rx, &ry, &rz, &rw );
            //printf("// i %d %d %d\n", ii[0], ii[1], ii[2] );
            //printf(" Rot %d : %f %f %f %f\n", ipos, rx,ry,rz,rw);
            animsRot.push_back( vec( rx,ry,rz,rw ) );
            animRotFrame.push_back( ipos/160 );
        }
        else if ( (found = str.find("*CONTROL_POS_SAMPLE")) != -1)
        {
            int ipos;
            float tx,ty,tz;
            const char *pparse = str.c_str()+23;
            int nbread = sscanf( pparse, "%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &ipos, &tx, &ty, &tz );
            //printf("// i %d %d %d\n", ii[0], ii[1], ii[2] );
            //printf(" Trans %d : %f %f %f\n", ipos, tx, ty, tz );
            animsPos.push_back( vec( tx, ty, tz ) );
            animsPosFrame.push_back( ipos/160 );
        }

        else if ( (found = str.find("*MESH_NUMFACES")) != -1)
        {
            int nbFaces;
            const char *pparse = str.c_str()+16;
            sscanf( pparse, "%d", &nbFaces );
            //printf("// %d faces\n", nbFaces);

            importVertices.resize( nbFaces*3 );
            importIndices.resize( nbFaces*3 );
            importUV.resize( nbFaces*3 );
            importPubVertices.resize( nbFaces*3 );

            for (int i=0;i<nbFaces*3;i++)
            {
                importIndices[i] = i;
            }
        }
        else
        if ( (found = str.find("*MESH_VERTEXNORMAL")) != -1)
        {
            int nbVt;
            float nx, ny, nz;
            const char *pparse = str.c_str()+22;
            int nbread = sscanf( pparse, "%*[ \t]%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &nx, &ny, &nz);
            //printf("// n %5.4f %5.4f %5.4f\n", nx, ny, nz );
            /*
            if (nrmFaceAv >= importVertices.size())
            {
                int a = 1;
            }
            */
            importVertices[nrmFaceAv].nx = nx;
            importVertices[nrmFaceAv].ny = ny;
            importVertices[nrmFaceAv].nz = nz;

            nrmFaceAv++;
        }
        else
        if ( (found = str.find("*MESH_VERTEX")) != -1)
        {
            int nbVt;
            float x,y,z;
            const char *pparse = str.c_str()+16;
            int nbread = sscanf(pparse, "%*[ \t]%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &x, &y, &z);
            vec_t meshVt = vec(x,y,z, 0.f);
            float nRadius = (meshVt-lastParsedPosition).length();
            maxRadius = (maxRadius>nRadius)?maxRadius:nRadius;
            //printf("// vt %5.4f %5.4f %5.4f\n", x,y,z);
            /*
            importVertices[nbVt].x = x;
            importVertices[nbVt].y = y;
            importVertices[nbVt].z = z;
            */
            importTmpVertices.push_back( meshVt );
        }
        else
        if ( (found = str.find("*MESH_TVERT")) != -1 )
        {
 int nbVt;
            float u,v;
            const char *pparse = str.c_str()+14;
            int nbread = sscanf(pparse, "%*[ \t]%d%*[ \t]%f%*[ \t]%f", &nbVt, &u, &v );
            //printf("// vt %5.4f %5.4f %5.4f\n", x,y,z);
            /*
            importVertices[nbVt].x = x;
            importVertices[nbVt].y = y;
            importVertices[nbVt].z = z;
            */
            if ( nbread )
                importTmpUV.push_back( vec( u, v, 0.f, 0.f ) );
        }
        else
        if ( (found = str.find("*MESH_VERTCOL")) != -1)
        {
            int nbVt;
            float r,g,b;
            const char *pparse = str.c_str()+17;
            int nbread = sscanf( pparse, "%d%*[ \t]%f%*[ \t]%f%*[ \t]%f", &nbVt, &r, &g, &b);
            //printf("// rgb %5.4f %5.4f %5.4f\n", r,g,b );
			if (shipsColorManagement)
			{
				float alpha = 0.5f;//(r<0.5f)?0.5f:r;
				u32 col = vec(r,g,b,alpha).toUInt32();

                const int importVertexCount = importVertices.size();
				if (nbVt < importVertexCount)
					importVertices[nbVt].col = col;
			}
			else
			{
				float alpha = zmax(r,zmax(g,b)); //(r<0.5f)?0.5f:r;
				u32 col = vec(r,g,b,alpha).toUInt32();

                const int importVertexCount = importVertices.size();
                if (nbVt < importVertexCount)
					importVertices[nbVt].col = col;
			}
        }
        else
        if ( (found = str.find("*TM_ROW3")) != -1)
        {

            const char *pparse = str.c_str()+10;
            int nbread = sscanf( pparse, "%*[ \t]%f%*[ \t]%f%*[ \t]%f", &tx, &ty, &tz );
            lastParsedPosition = vec( tx, ty, tz );
			if (shipsColorManagement)

			{
				tx=ty=tz = 0.f;
			}

            if (usePubVertex)
            {
                //tx=ty=tz = 0.f;
                tx=-tx;
                ty=-ty;
                tz = -tz;
                animTrans = vec(tx, ty, tz, 0.f);

            }


            //printf("// tr %5.4f %5.4f %5.4f\n", tx, ty, tz );
        }
        else if ( (found = str.find("*TM_ROW2")) != -1)
        {
            const char *pparse = str.c_str()+10;
            int nbread = sscanf( pparse, "%*[ \t]%f%*[ \t]%f%*[ \t]%f", &tx, &ty, &tz );

            lastParsedDirection = vec( tx, ty, tz);

        }
    }
    fclose(fp);
finishedImporting:	;
    // write place holders
    writePlaceHolders( reactors, fpo );
    writePlaceHolders( valves, fpo );
    writePipes( pipes, fpo );


    //fputs("", fpo);
	fclose(fpo);

}


typedef struct IOshapeSegment_t
{
	vec_t v1, v2;
	u32 color;
} IOshapeSegment_t;

typedef struct IOshape_t
{
	IOshape_t()
	{
		groundMinimal = 9999.f;
		groundLargest = 0.f;
	}

	std::vector<IOshapeSegment_t> mSegments;
	std::vector<IOshapeSegment_t> mWall, mGround;
	float groundMinimal, groundLargest;
} IOshape_t;

std::map< std::string, IOshape_t> mIOAllShapes;

void ImportSlices()
{
	//
	FILE *fp = fopen("Assets/slices.ase","rt");
	if (!fp)
		return;

	char curline[1024];
	bool mbShapeClosed = false;
	std::string shapename;
	int shapeType; // 0 = normal, 1 = ground, 2 = wall
	u32 shapeColor;
	int pointIndex;
	int nbVts;
	bool bNewBatch = false;
	while ( fgets(curline, 1024, fp) )
	{
		std::string str = curline;
		size_t found;

		if ( (found = str.find("*SHAPEOBJECT {")) != -1)
		{
			// new shape
		}
		if ( (found = str.find("*NODE_NAME \"")) != -1)
		{
			shapename = str.substr(13, str.size()-13-2);
			if (shapename[0] == '\"')
				shapename = shapename.substr(1, 100);

			mbShapeClosed = false;
			// type
			found = shapename.find("_");
			std::string shpType = shapename.substr(found+1,100);
			shapename = shapename.substr(0, found);
			if ( shpType == "ground")
			{
				shapeType = 1;
				pointIndex = 0;
			}
			else if (shpType == "wall")
			{
				shapeType = 2;
				pointIndex = 0;
			}
			else
			{
				shapeType = 0;
				//shapeColor = atoi(shpType.c_str() );
				//shapeColor = (shapeColor>=0x80)?1:0
				sscanf(shpType.c_str(), "%x", &shapeColor);
				pointIndex = mIOAllShapes[shapename].mSegments.size()<<1;
			}
			bNewBatch = true;

		}
		if ( (found = str.find("*SHAPE_CLOSED")) != -1)
		{
			mbShapeClosed = true;


		}
		if ( (found = str.find("*SHAPE_VERTEXCOUNT")) != -1)
		{
			std::string strtp = str.substr(21, 100);
			nbVts = atoi(strtp.c_str());
			int toAlloc = (nbVts + (mbShapeClosed?1:0) ) - 1;
			std::vector<IOshapeSegment_t> * vecseg;
			switch (shapeType)
			{
			case 0:
				vecseg = &mIOAllShapes[shapename].mSegments;
				break;
			case 1:
				vecseg = &mIOAllShapes[shapename].mGround;
				break;
			case 2:
				vecseg = &mIOAllShapes[shapename].mWall;
				break;
			}
			vecseg->resize( vecseg->size() + toAlloc);
			bNewBatch = true;
		}
		if ( (found = str.find("*SHAPE_VERTEX_KNOT")) != -1)
		{
			std::string strtp = str.substr(23, 100);
			float tmpfloat;

			IOshapeSegment_t *pSeg;

			int segiCount = ((bNewBatch)?1:2);

			for (int twopt = 0; twopt <segiCount; twopt ++)
			{
				switch (shapeType)
				{
				case 0:
					if ( (pointIndex>>1)>=static_cast<int>(mIOAllShapes[shapename].mSegments.size()))
						continue;
					pSeg = &mIOAllShapes[shapename].mSegments[pointIndex>>1];
					break;
				case 1:
					if ( (pointIndex>>1)>=static_cast<int>(mIOAllShapes[shapename].mGround.size()))
						continue;
					pSeg = &mIOAllShapes[shapename].mGround[pointIndex>>1];
					break;
				case 2:
					if ( (pointIndex>>1)>=static_cast<int>(mIOAllShapes[shapename].mWall.size()))
						continue;
					pSeg = &mIOAllShapes[shapename].mWall[pointIndex>>1];
					break;
				}

				pSeg->color = shapeColor;
				if ( !(pointIndex&1) )
				{
					sscanf(strtp.c_str(), "%f\t%f\t%f", &pSeg->v1.x, &tmpfloat, &pSeg->v1.y);
					pSeg->v1.z = 0.f;
					if ( bNewBatch && mbShapeClosed)
					{
						// only non physic shapes can be closed
						mIOAllShapes[shapename].mSegments[mIOAllShapes[shapename].mSegments.size()-1].v2 = pSeg->v1;
					}
					if ( ( shapeType == 1) && (pSeg->v1.y <= mIOAllShapes[shapename].groundMinimal ) )
					{
						mIOAllShapes[shapename].groundMinimal = pSeg->v1.y;
						if ( fabsf(pSeg->v1.x) >= mIOAllShapes[shapename].groundLargest )
							mIOAllShapes[shapename].groundLargest = fabsf(pSeg->v1.x);
					}
				}
				else
				{
					sscanf(strtp.c_str(), "%f\t%f\t%f", &pSeg->v2.x, &tmpfloat, &pSeg->v2.y);
					pSeg->v2.z = 0.f;
					if ( ( shapeType == 1) && (pSeg->v2.y <= mIOAllShapes[shapename].groundMinimal ) )
					{
						mIOAllShapes[shapename].groundMinimal = pSeg->v2.y;
						if ( fabsf(pSeg->v2.x) >= mIOAllShapes[shapename].groundLargest )
							mIOAllShapes[shapename].groundLargest = fabsf(pSeg->v2.x);
					}
				}
				pointIndex ++;
			}
			bNewBatch = false;
		}
	}


	fclose(fp);


	// code generation
	FILE *fpo = fopen("Assets/slices.cpp","wt");
	char tmps[1024];
	int idx = 0;
	std::map< std::string, IOshape_t>::const_iterator iter = mIOAllShapes.begin();
	for (; iter != mIOAllShapes.end() ; ++ iter)
	{
		sprintf(tmps, "#define %s %d\n", (*iter).first.c_str(), idx ++);
		fputs(tmps, fpo);
	}
	std::string shapetout="";
	std::string shapetout1 = "";
	std::string shapetout2 = "";

	iter = mIOAllShapes.begin();
	for (idx = 0; iter != mIOAllShapes.end() ; ++ iter, idx ++)
	{
		shapetout+="{";
		sprintf( tmps, "%5.2ff", (*iter).second.groundLargest );
		shapetout += tmps;
		shapetout += ", ";

		shapetout1="{";
		shapetout2="{";
		//shapeSegment_t shape0Segs[] = { {0.f, 0.f, 0.f, 0.f, 0xFFFFFFFF}, {0.f, 0.f, 0.f, 0.f, 0xFFFFFFFF} };
		bool bFirstSeg;
		std::vector<IOshapeSegment_t>::const_iterator ii;
		std::vector<IOshapeSegment_t>::const_iterator iiend;
		int segsQty[4] = {0,0,0,0};
		const char *arrayName[4] = {"shapeSegment_t %sSegs[] = {\n",
			"shapeSegment_t %sSegsTransparent[] = {\n",
			"shapeSegment_t %sSegsWall[] = {\n",
			"shapeSegment_t %sSegsGround[] = {\n" };
		// solid seg
		for (int si = 0;si<4;si++)
		{
			std::string listeOutput;
			if (si<2)
			{
				ii = (*iter).second.mSegments.begin();
				iiend = (*iter).second.mSegments.end();
			}
			else if (si == 2)
			{
				if ((*iter).second.mWall.empty())
					goto endofline;
				ii = (*iter).second.mWall.begin();
				iiend = (*iter).second.mWall.end();
			}
			else
			{
				if ((*iter).second.mGround.empty())
					goto endofline;

				ii = (*iter).second.mGround.begin();
				iiend = (*iter).second.mGround.end();
			}

			sprintf( tmps, arrayName[si], (*iter).first.c_str() );
			//fputs(tmps, fpo);
			listeOutput = tmps;
			bFirstSeg = true;
			for (; ii != iiend ; ++ii)
			{
				if ( ((si == 0) && ( (*ii).color < 0x80)) ||
					((si == 1) && ( (*ii).color >= 0x80)) )
					continue;

				sprintf(tmps, "%s{%5.5ff, %5.5ff, %5.5ff, %5.5ff, 0x%x}\n", bFirstSeg?"":",",
					(*ii).v1.x,
					(*ii).v1.y -  (*iter).second.groundMinimal,
					(*ii).v2.x,
					(*ii).v2.y -  (*iter).second.groundMinimal,
					(*ii).color );
				//fputs(tmps, fpo);
				listeOutput += tmps;
				bFirstSeg = false;
				segsQty[si] ++;
			}
			//fputs("};\n\n", fpo);
			listeOutput += "};\n\n";
endofline :
			if ( segsQty[si] )
			{
				fputs( listeOutput.c_str(), fpo);
				char tmpsw[512];
				const char *arrName[4] = {"%sSegs",
					"%sSegsTransparent",
					"%sSegsWall",
					"%sSegsGround" };

				sprintf(tmpsw, arrName[si],  (*iter).first.c_str());
				shapetout1 += tmpsw;
				//shapetout1 += ",";
				sprintf(tmpsw, "%d", segsQty[si]);
				shapetout2 += tmpsw;
			}
			else
			{
				//shapetout += "NULL, 0";
				shapetout2 += "0";
				shapetout1 += "NULL";
			}
			if ( si != 3)
			{
				shapetout2 += ",";
				shapetout1 += ",";
			}
		}

		shapetout += shapetout1;
		shapetout += "}, ";
		shapetout += shapetout2;
		shapetout += "} ";
		if (idx != (mIOAllShapes.size()-1))
			shapetout += "},\n";
		else
			shapetout += "}\n";


	}
	// output shapef
	fputs("\n\n\n", fpo);
	fputs("shape_t Shapes[]={\n", fpo);
	fputs(shapetout.c_str(), fpo);
	fputs("};\n\n\n", fpo);

	fclose(fpo);
}


int main(int argc, char* argv[])
{
#if 0
    typedef struct testStruct_t
    {
        int a,b;
    } testStruct_t;

    ArrayPool<testStruct_t, 50> poolObstacles;

    testStruct_t *obs1 = poolObstacles.New();
    poolObstacles.Delete( obs1 );

    testStruct_t *obs2,*obs3, *obs4;
    /*
    testStruct_t *obs2 = poolObstacles.New();
    testStruct_t *obs3 = poolObstacles.New();
    testStruct_t *obs4;
    for (int i=0;i<50;i++)
    {
        obs4 = poolObstacles.New();
    }
    */
    for ( ArrayPool<testStruct_t, 50>::poolElt *iter = poolObstacles.GetFirst(); iter; iter=iter->GetNext() )
    {
        printf(" Ptr 0x%p \n", iter );
    }
    obs3 = poolObstacles.New();

    //poolObstacles.Delete( obs3 );
    //poolObstacles.Delete( obs1 );
    printf("--------\n");
    for ( ArrayPool<testStruct_t, 50>::poolElt *iter = poolObstacles.GetFirst(); iter; iter=iter->GetNext() )
    {
        printf(" Ptr 0x%p \n", iter );
    }
    obs4 = poolObstacles.New();
    obs2 = poolObstacles.New();

    //poolObstacles.Delete( obs4 );
    //poolObstacles.Delete( obs2 );
    printf("--------\n");
    for ( ArrayPool<testStruct_t, 50>::poolElt *iter = poolObstacles.GetFirst(); iter; iter=iter->GetNext() )
    {
        printf(" Ptr 0x%p \n", iter );
    }

    poolObstacles.Delete( obs4 );
    poolObstacles.Delete( obs2 );
    poolObstacles.Delete( obs3 );
    printf("-------- all cleared\n");
        for ( ArrayPool<testStruct_t, 50>::poolElt *iter = poolObstacles.GetFirst(); iter; iter=iter->GetNext() )
    {
        printf(" Ptr 0x%p \n", iter );
    }
        printf("-------- all cleared done\n");
        /*
    for (int i=0;i<50;i++)
    {
        obs4 = poolObstacles.New();
    }
        for ( ArrayPool<testStruct_t, 50>::poolElt *iter = poolObstacles.GetFirst(); iter; iter=iter->GetNext() )
    {
        printf(" Ptr 0x%p \n", iter );
    }
    */

        for (int j = 0 ; j < 1000 ; j++)
        {
            int nbIter = fastrand()&0xF;

            if ( (nbIter + poolObstacles.GetUsedItemsCount() ) > 50 )
                nbIter = 50-poolObstacles.GetUsedItemsCount();

            for (int i = 0 ; i < nbIter ; i++)
            {
                obs4 = poolObstacles.New();
            }

            nbIter = fastrand()&0xF;

            int nbc = (poolObstacles.GetUsedItemsCount()-nbIter );
            if ( nbc < 0 )
                nbIter = poolObstacles.GetUsedItemsCount();

            for (int i = 0 ; i < nbIter ; i++)
            {
                poolObstacles.Delete( poolObstacles.GetFirst() );
            }

        }

        printf("--------  multi done\n");

    ArrayPool<testStruct_t, 500> poolObstacles2;


    printf("*** %d ArrayPools active with a total of %d bytes\n",
        GetNumberOfArrayPools(),
        GetNumberOfBytesInArrayPools() );


    return 0;
#endif
	ImportSlices();
	//importShipsASE();
	importASE( "Assets/objects3D.ASE", "Bin/Datas/Meshes/objects3D.bin", false );
    importASE( "Assets/texts.ASE", "Bin/Datas/Meshes/adTexts.bin", false, true );
	importShipsV2ASE( "Assets/ships.ASE", "Bin/Datas/Meshes/ships.bin" );
    importASE( "Assets/tanker.ASE", "Bin/Datas/Meshes/tanker.bin", true );
    importShipsV2ASE( "Assets/env4.ASE", "Bin/Datas/Meshes/env4.bin" );
	importASECity( "Assets/city.ASE" );
    /*
	system("bin2c ships.bin");
	system("bin2c objects3D.bin");
    system("bin2c city.bin");
    */
	return 0;
}

