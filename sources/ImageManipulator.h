#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "maths.h"

typedef unsigned char u8;
typedef uint32_t u32;
class ImageManipulator
{
public:
    ImageManipulator();
	ImageManipulator(int aWidth, int aHeight, uint32 fillWith = 0);
    ImageManipulator( const char *szFilename );
    ~ImageManipulator();

	void Box(int x, int y, int width, int height, u8 value);
	void Circle(int x, int y, int radius, u8 value);
	void Noise(int strength);
	void Stars(float min, float max, unsigned int count );
	void SmoothStep(float edge0, float edge1);

    // 0 to 1
    void ContrastBrightness( float contrast, float brightness);

    // blur
	void HorzBlur(int strength);

	void VertBlur(int strength);

	//factors are <<8. 256 means 1.f
	void ApplyKernel(int *factors, int kernelWidth);

	void Save(const char *szName);
    void Load(const char *szName);

	void FloeDisk( int x, int y, int radius, float height, float amp, int seed, float thresholdAdd, float thresholdFactor);
    void PerlinMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace = 0, float freq = 4.f );
    void SimplexMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace = 0, float freq = 4.f );
    
    void ComputeNormalMap( u32* nrmBits, float bumpHeightScale );

    void MaxxedBy( ImageManipulator &hg2 );
	void SmoothCliff();
    void Erode(float smoothness);

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }
	u32* GetBits() const { return mBits; }

    void ChamferInAlpha();
    void BlendLayer( ImageManipulator& img );
    void WhiteBlackTheshold( unsigned char threshold, uint32_t borderColor, uint32_t fillColor  );
    void BlendLayerRectangle( ImageManipulator& img, int px, int py, int sx, int sy, int dx, int dy );
    void GetRandomPosWithAlpha( unsigned char alpha, int &x, int &y );

    void Init( int aWidth, int aHeight, uint32 fillWith );
    unsigned int UploadToGL();
protected:
	int mWidth, mHeight;
	u32 *mBits;


};
