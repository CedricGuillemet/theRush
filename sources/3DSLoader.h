
#ifndef LOADER_3DS
#define LOADER_3DS

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "3ds.h"

struct Triangle3DS
{
    unsigned int v0;
    unsigned int v1;
    unsigned int v2;
};

struct Vertex3DS
{
    float x, y, z;
    float nx, ny, nz;
    float r, g, b;
};

struct Mesh3DS
{
    unsigned int    mNumTriangles;
    Triangle3DS*    mTriangles;
    unsigned int    mNumVertices;
    Vertex3DS*      mVertices;
    float o[4];
    float x[4];
    float y[4];
    float z[4];
};


class Loader3DS
{
public:
	Loader3DS();
	~Loader3DS();
    
    
	bool loadFile(std::string filename);
    
    unsigned int numMeshes;
    Mesh3DS* meshes;
    
    
};


#endif // LOADER_3DS

