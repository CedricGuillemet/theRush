
#include "3DSLoader.h"


#include <stdlib.h>

#include "math.h"

using namespace std;



#if 0
#define LOGDEBUG(x) std::cout<<x;
#else
#define LOGDEBUG(x) 
#endif


Loader3DS::Loader3DS()
{
    numMeshes = 0;
    meshes = NULL;
}

Loader3DS::~Loader3DS()
{
    // TODO!!
    
    if(meshes)
        delete [] meshes;
    numMeshes = 0;
}


bool Loader3DS::loadFile(std::string filename)
{
    LOGDEBUG("loading "<<filename<<endl);
    
    CLoad3DS load3DS;
    t3DModel models3DS;
    models3DS.numOfObjects = 0;
    models3DS.numOfMaterials = 0;
    bool loadResult = load3DS.Import3DS(&models3DS, filename.c_str());
    
    if(loadResult)
    {
        LOGDEBUG("num objects " << models3DS.numOfObjects<< endl);
        LOGDEBUG("num materials " << models3DS.numOfMaterials<< endl);
    
        // Debug
        for (int m=0; m<models3DS.numOfObjects; ++m) 
        {
            t3DObject* object = &models3DS.pObject.at(m);
            LOGDEBUG( "NAME "      << object->strName << endl);
            LOGDEBUG( "    numOfVerts   " << object->numOfVerts << endl);
            LOGDEBUG( "    numOfFaces   " << object->numOfFaces << endl);
            LOGDEBUG( "    numTexVertex " << object->numTexVertex << endl);
            LOGDEBUG( "    material     " << object->materialID << endl);
            LOGDEBUG( "    bHasTexture  " << object->bHasTexture << endl);
        }
        
        // Convert to our format
        if(meshes)
            delete [] meshes;
        numMeshes = models3DS.numOfObjects;
        meshes = new Mesh3DS[numMeshes];
        
        // Convert each of the meshes
        for (unsigned int m=0; m<numMeshes; ++m) 
        {
            Mesh3DS* mesh = &meshes[m];
            t3DObject* object = &models3DS.pObject.at(m);
            
            memcpy(mesh->o, object->o, 4*sizeof(float));
            memcpy(mesh->x, object->x, 4*sizeof(float));
            memcpy(mesh->y, object->y, 4*sizeof(float));
            memcpy(mesh->z, object->z, 4*sizeof(float));
            
            // We simply read all triangles and dupicate all vertices as we want per face color (unique per vertex color, later uv, etc)
            // Later, we can run a pass to reduce the amount of vertices
            mesh->mNumTriangles = object->numOfFaces;
            mesh->mTriangles = new Triangle3DS[mesh->mNumTriangles];
            mesh->mNumVertices = mesh->mNumTriangles*3;
            mesh->mVertices = new Vertex3DS[mesh->mNumVertices];
            unsigned int* smoothGroup = new unsigned int[mesh->mNumVertices];
            unsigned int currentIndexId = 0;
            unsigned int currentVertexId = 0;
            for(unsigned int t=0; t<mesh->mNumTriangles; ++t)
            {
                unsigned int vIds[3];
                vIds[0] = object->pFaces[t].vertIndex[0];
                vIds[1] = object->pFaces[t].vertIndex[1];
                vIds[2] = object->pFaces[t].vertIndex[2];
                tMaterialInfo* mat = object->pFaces[t].mat;
                
                float r = 1.0f;
                float g = 0.0f;
                float b = 1.0f;
                if(mat)
                {
                    r = float(mat->color[0])/255.0f;
                    g = float(mat->color[1])/255.0f;
                    b = float(mat->color[2])/255.0f;
                }

                // Store vertex indices
                mesh->mTriangles[t].v0 = currentIndexId;
                mesh->mTriangles[t].v1 = currentIndexId+1;
                mesh->mTriangles[t].v2 = currentIndexId+2;
                currentIndexId+=3;
                
                // Store vertex data
                for(int v=0; v<3; ++v)
                {
                    Vertex3DS& vertexOut = mesh->mVertices[currentVertexId];
                    unsigned int vIdIn = vIds[v];
                    
                    vertexOut.x  = object->pVerts[vIdIn].x;// - object->o[0];
                    vertexOut.y  = object->pVerts[vIdIn].y;// - object->o[1];
                    vertexOut.z  = object->pVerts[vIdIn].z;// - object->o[2];
                    vertexOut.nx = object->pNormals[vIdIn].x;
                    vertexOut.ny = object->pNormals[vIdIn].y;
                    vertexOut.nz = object->pNormals[vIdIn].z;
                    
                    smoothGroup[currentVertexId] = 0;
                    
                    float L = vertexOut.ny*0.5f+0.5f;
                    vertexOut.r  = powf(r*L, 1.0f/2.2f); // Colors are stored in linear space, remove pow and do convertion in vertex shader
                    vertexOut.g  = powf(g*L, 1.0f/2.2f);
                    vertexOut.b  = powf(b*L, 1.0f/2.2f);
                    currentVertexId++;
                }
            }

            delete [] smoothGroup;
            
        
        }
    }
        
    return loadResult;
}




