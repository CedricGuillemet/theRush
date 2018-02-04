#include "stdafx.h"
#include "ImageManipulator.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STBI_NO_WRITE
#define STBI_HEADER_FILE_ONLY
#include "stb_image.h"
#include "toolbox.h"

#include "include_GL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

ImageManipulator::ImageManipulator(int aWidth, int aHeight, uint32 fillWith )
{
    Init( aWidth, aHeight, fillWith );
}

ImageManipulator::ImageManipulator( const char *szFilename )
{
    Load( szFilename );
}

void ImageManipulator::Init( int aWidth, int aHeight, uint32 fillWith )
{
    	mWidth = aWidth;
	mHeight = aHeight;
	mBits = new u32 [mWidth*mHeight];
	//memset(mBits, 0, sizeof(u32) * mWidth*mHeight );
    for (int i = 0;i<mWidth*mHeight;i++)
        mBits[i] = fillWith;
}

ImageManipulator::ImageManipulator( )
{
	mWidth = 0;
	mHeight = 0;
	mBits = NULL;
}

ImageManipulator::~ImageManipulator()
{
    delete[] mBits;
    mBits = NULL;
}
#if 0
// x + : z,y
// x - : -z, y
// y + : x,z
// y - : -x, z
// z + : x,y
// z - : -x,y
void ImageManipulator::PerlinMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace, float freq )
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

void ImageManipulator::SimplexMap( int x, int y, int width, int height, float amp, float base, int seed, int cubeFace, float freq)
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
#endif
#if 0
void ImageManipulator::ComputeNormalMap( u32* nrmBits, float bumpHeightScale )
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
#endif
#if 0
void ImageManipulator::FloeDisk( int x, int y, int radius, float height, float amp, int seed, float thresholdAdd, float thresholdFactor)
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
#endif
void ImageManipulator::SmoothCliff()
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

void ImageManipulator::Erode(float smoothness)
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





void ImageManipulator::Box(int x, int y, int width, int height, u8 value)
{
	for (int iy = y; iy < y+height; iy++)
		for (int ix = x; ix < x+width; ix++)
			mBits[iy * mWidth + ix] = value;
}

void ImageManipulator::Circle(int x, int y, int radius, u8 value)
{
	for (int iy = y-radius; iy < y+radius; iy++)
		for (int ix = x-radius; ix < x+radius; ix++)
			if ( ((ix-x)*(ix-x) + (iy-y)*(iy-y)) <= (radius*radius) )
				mBits[iy * mWidth + ix] = value;
}

void ImageManipulator::Noise(int strength)
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

void ImageManipulator::Stars(float min, float max, unsigned int count )
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

void ImageManipulator::SmoothStep(float edge0, float edge1)
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
void ImageManipulator::ContrastBrightness( float contrast, float brightness)
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


// blur
void ImageManipulator::HorzBlur(int strength)
{
    if (!strength)
        return;
    
    int nbp = (strength<<1) +1 ;    
    
	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			vec_t val = vec( 0.f );

			for (int ax = x-strength; ax<=x+strength; ax++)
			{
				if ((ax<0)||(ax>=mWidth))
					continue;
                vec_t n;
                n.fromUInt32( mBits[y * mWidth + ax] );
				val += n;
			}
			val *= 1.f/(float)nbp;
			mBits[y * mWidth + x] = val.toUInt32();
		}
	}
}

void ImageManipulator::VertBlur(int strength)
{
    if (!strength)
        return;
    
    int nbp = (strength<<1) +1 ;      
    
	for (int y = 0;y<mHeight; y++)
	{
		for (int x = 0;x<mWidth; x++)
		{
			vec_t val = vec( 0.f );
			for (int ay = y-strength; ay<=y+strength; ay++)
			{
				if ((ay<0)||(ay>=mHeight))
					continue;
                vec_t n;
                n.fromUInt32( mBits[ay * mWidth + x] );
				val += n;
			}
			val *= 1.f/(float)nbp;
			mBits[y * mWidth + x] = val.toUInt32();
		}
	}
}

//factors are <<8. 256 means 1.f
void ImageManipulator::ApplyKernel(int *factors, int kernelWidth)
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

void ImageManipulator::Save(const char *szName)
{
	stbi_write_png(szName, mWidth, mHeight, 4, mBits, mWidth*4);
}

void ImageManipulator::Load(const char *szName)
{
    int x,y,comp;
    stbi_uc * uc = stbi_png_load(szName,&x, &y, &comp, 4 );

    Init( x,y, 0 );
    memcpy( mBits, uc, 4*x*y);
    stbi_image_free(uc);
}

void ImageManipulator::ChamferInAlpha()
{
	/*
    unsigned char *src = new u8 [mWidth * mHeight ];
    unsigned char *dst = new u8 [mWidth * mHeight ];
    for (int i = 0;i<(mWidth*mHeight);i++)
    {
        if (mBits[i]&0xFFFFFF)
            src[i] = 0xFF;
        else
            src[i] = 0x0;
    }
    ComputeChamfer( src, mWidth, mHeight, dst );

    for (int i = 0;i<(mWidth*mHeight);i++)
    {
        mBits[i] &= 0xFFFFFF;
        mBits[i] += dst[i]<<24;
    }

    delete [] src;
    delete [] dst;
	*/
}

void ImageManipulator::BlendLayer( ImageManipulator& img )
{
    for (int i = 0;i<(mWidth*mHeight);i++)
    {
        vec_t v,v2;
        v.fromUInt32(mBits[i] );
        v2.fromUInt32(img.mBits[i] );

        float l = (float)((img.mBits[i]&0xFF000000)>>24) / 255.f;
        v.lerp(v2, l );
        mBits[i] = v.toUInt32();
    }

}

void ImageManipulator::WhiteBlackTheshold( unsigned char threshold, uint32_t borderColor, uint32_t fillColor )
{
    for (int i = 0;i<(mWidth*mHeight);i++)
    {
        if ((mBits[i]&0xFFFFFF) != 0xFFFFFF)
            continue;

        u8 alpha = ((mBits[i]&0xFF000000)>>24);
        if ( alpha <= threshold )
        {
            mBits[i]&=0xFF000000;
            mBits[i]+=borderColor;
        }
        else
        {
            mBits[i]&=0xFF000000;
            mBits[i]+=fillColor;
        }
    }
}

void ImageManipulator::BlendLayerRectangle( ImageManipulator& img, int px, int py, int sx, int sy, int dx, int dy )
{
    if ( dx<0) { px -= dx; dx = 0; }
    if ( dy<0) { py -= dy; dy = 0; }

    int ly = ((dy + sy)>=mHeight)?mHeight:(dy + sy);
    int lx = ((dx + sx)>=mWidth)?mWidth:(dx + sx);

    
    int spy = py;
    for (int y = dy; y<ly;y++,spy++)
    {
        int spx = px;
        for (int x = dx;x<lx;x++, spx++)
        {
            int destpos = y * mWidth + x;
            int srcpos = spy * img.mWidth + spx;

            vec_t v,v2;
            v.fromUInt32(mBits[destpos] );
            v2.fromUInt32(img.mBits[srcpos] );
            /*
            float l = (float)((img.mBits[srcpos]&0xFF000000)>>24) / 255.f;
            float l2 = (float)((mBits[destpos]&0xFF000000)>>24) / 255.f;
            */
            v.lerp(v2, v.x * v2.w ); // v.x because we don't want damages on non existant flesh
            mBits[destpos] = v.toUInt32();
        }
    }
}

void ImageManipulator::GetRandomPosWithAlpha( unsigned char alpha, int &x, int &y )
{
    int nx = int(r01() * mWidth);
    int ny = int(r01() * mHeight);

    int pos = ny * mWidth + nx;
    bool bFound = false;
    while (!bFound)
    {
        u8 v = ((mBits[pos]&0xFF000000)>>24);
        if ( abs(v - alpha) <= 4 )
        {
            y = pos/mWidth;
            x = pos%mWidth;
            return;
        }
        pos ++;
        if (pos >= (mWidth * mHeight ) )
            pos = 0;
    }
}

unsigned int ImageManipulator::UploadToGL()
{
    unsigned int res;

    glGenTextures(1, &res);

    glBindTexture(GL_TEXTURE_2D, res);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mBits );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glGenerateMipmap(GL_TEXTURE_2D);

    return res;
}
/*
void ImageManipulator::MaxxedBy( ImageManipulator &hg2 )
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
*/

///////////////////////////////////////////////////////////////////////////////////////////////////