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
#include "gui.h"
#include "content.h"
#include "render.h"
#include "audio.h"

#include "include_GL.h"
#define STBI_NO_WRITE
//#define STBI_HEADER_FILE_ONLY
#include "stb_image.h"


gui ggui;
const char * m8x8FontBits;

gui::gui(void)
{
	mTextBuffer = NULL;
	mCurrentBlackBox.set(0.f);
	mTargetedBlackBox.set(0.f);
    mLocalTime = 0.f;
    mbBlackBoxIsStable = false;
}

gui::~gui(void)
{
	if (mTextBuffer)
	{
		delete [] mTextBuffer;
        delete [] charsDirty;
        delete [] guiPix;
	}
}


void gui::init()
{
    //ASSERT_GAME( m8x8FontBits );
    
    int x,y,comp;
    stbi_uc * uc = stbi_png_load("Datas/Textures/A8x8font.png",&x, &y, &comp, 1 );

	m8x8FontBits = (const char*)uc;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &mTexID);	

	glBindTexture(GL_TEXTURE_2D, mTexID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

    
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 128, 128, 0, GL_ALPHA, GL_UNSIGNED_BYTE, m8x8FontBits );

	//stbi_image_free(uc); // not freed
}

void gui::clearText( int stx, int sx, int sty, int sy )
{
    ASSERT_GAME(mTextBuffer);
    ASSERT_GAME(charsDirty);
	/*
	memset( mTextBuffer, ' ', mNbCharsWidth * mNbCharsHeight );
    memset( charsDirty, -1, (mNbCharsWidth * mNbCharsHeight)>>3 );
	*/
	for (int y = sty; y<(sty+sy);y++)
	{
		for (int x = stx; x<(stx+sx);x++)
		{
			mTextBuffer[y*64+x] = ' ';
			charsDirty[(y*64+x)>>3] = 0xFF;
		}
	}
}

void gui::setResolution(int nbCharsWidth, int nbCharsHeight)
{
	if (mTextBuffer)
	{
        ASSERT_GAME(guiPix);
        ASSERT_GAME(charsDirty);
        
		delete [] mTextBuffer;
        delete [] guiPix;
        delete [] charsDirty;
	}

	mNbCharsWidth = nbCharsWidth;
	mNbCharsHeight = nbCharsHeight;

	mTextBuffer = new unsigned char [mNbCharsWidth * mNbCharsHeight];
    
    unsigned int guiPixSize = nbCharsWidth*8*nbCharsHeight*8;
    guiPix = new u32 [guiPixSize];
    memset ( guiPix, 0, guiPixSize * sizeof(u32) );
    
    unsigned int charsDirtySize = (mNbCharsWidth*nbCharsHeight)>>3;
    charsDirty = new u8 [charsDirtySize];
    memset ( charsDirty, 0, charsDirtySize * sizeof(u8) );
    
    clearText();

}

void gui::tick(float aTimeEllapsed)
{
    if (!mbBlackBoxIsStable)
    {
        float minboxx = zmin( mCurrentBlackBox.x, mTargetedBlackBox.x);
        float minboxy = zmin( mCurrentBlackBox.y, mTargetedBlackBox.y);    
        
        float maxboxx = zmax( mCurrentBlackBox.x+mCurrentBlackBox.z, mTargetedBlackBox.x+mTargetedBlackBox.z);
        float maxboxy = zmax( mCurrentBlackBox.y+mCurrentBlackBox.w, mTargetedBlackBox.y+mTargetedBlackBox.w);    
        
        int mincharx = (((int)minboxx-1));
        int minchary = (((int)minboxy-1));
        int maxcharx = (((int)maxboxx+1));
        int maxchary = (((int)maxboxy+1));        
        
        mincharx = zmax(mincharx, 0);
        minchary = zmax(minchary, 0);        
        
        maxcharx = zmin(maxcharx, 64);
        maxchary = zmin(maxchary, 36);
        
        for (int y = minchary ; y<maxchary ; y++)
        {
            for (int x = mincharx ; x<maxcharx ; x++)
            {
                unsigned int charPos = y*mNbCharsWidth + x;
                charsDirty[ charPos>>3 ] |= 1<<(charPos&7);    
                
            }
            
        }
        
    }
    
    
	mCurrentBlackBox.lerp(mTargetedBlackBox, aTimeEllapsed*10.f);
    
    
    mbComputeBlackBoxNextFrame = false;
    mbBlackBoxIsStable = true;
    if ( fabsf( mCurrentBlackBox.x - mTargetedBlackBox.x ) <= 0.1f )
    {
        mbComputeBlackBoxNextFrame = true;
        mCurrentBlackBox.x = mTargetedBlackBox.x;
    }
    else
        mbBlackBoxIsStable = false;
    
    if ( fabsf( mCurrentBlackBox.y - mTargetedBlackBox.y ) <= 0.1f )
    {
        mbComputeBlackBoxNextFrame = true;
        mCurrentBlackBox.y = mTargetedBlackBox.y;
    }
    else
        mbBlackBoxIsStable = false;
    
    if ( fabsf( mCurrentBlackBox.z - mTargetedBlackBox.z ) <= 0.1f )
    {
        mbComputeBlackBoxNextFrame = true;
        mCurrentBlackBox.z = mTargetedBlackBox.z;
    }
    else
        mbBlackBoxIsStable = false;
    
    if ( fabsf( mCurrentBlackBox.w - mTargetedBlackBox.w ) <= 0.1f )
    {
        mbComputeBlackBoxNextFrame = true;
        mCurrentBlackBox.w = mTargetedBlackBox.w;
    }
    else
        mbBlackBoxIsStable = false;

    
    mbComputeBlackBoxNextFrame |= (!mbBlackBoxIsStable);
    //if (!mbBlackBoxIsStable)
}

void gui::setBlackBox(int X, int Y, int width, int height)
{
    mTargetedBlackBox.set( (float)X, (float)Y, (float)width, (float)height );
	mbBlackBoxIsStable = false;
}

void gui::putText(int X, int Y, const char *pszText, bool bRevert)
{
	if (Y>= mNbCharsHeight)
		return;
	if (Y <0)
		return;

	int Ydec = Y*mNbCharsWidth;
	int sz = strlen(pszText);
	if (X+sz<0) 
		return;
	for (int i=0;i<sz;i++)
	{
		if (X+i >= mNbCharsWidth)
			return;
		if (X+i<0)
			continue;

        unsigned int charPos = (Ydec + X+i);
        if (mTextBuffer[ charPos ] != pszText[bRevert?((sz-1)-i):i] )
        {
            mTextBuffer[ charPos ] = pszText[bRevert?((sz-1)-i):i];
            //printf("Prev  : %x ( %x )->", charsDirty[ charPos>>3 ], charPos);
            charsDirty[ charPos>>3 ] |= 1<<(charPos&7);
            //printf(" %x \n", charsDirty[ charPos>>3 ] );
            //printf(" char %d (x:%d y:%d) \n", charPos, charPos&63, charPos>>6);
        }
	}
}

void gui::invalidateLine( int iLine )
{

    for (int i=0;i<8;i++)
    {
        charsDirty[ (iLine<<3) + i] = 0xFF;
    }
}

void gui::putChar(int X, int Y, unsigned char achar)
{
	if (Y>= mNbCharsHeight)
		return;
	if (Y <0)
		return;
	if (X>= mNbCharsWidth)
		return;
	if (X<0) 
		return;
    
    unsigned int charPos = Y*mNbCharsWidth + X;
    if ( mTextBuffer[ charPos ] != achar)
    {
        mTextBuffer[ charPos ] = achar;
        charsDirty[ charPos>>3 ] |= 1<<(charPos&7);    
    }
}

GLuint mGuiTexture = 0;

// rendering
void gui::draw( float aTimeEllapsed )
{
    
    mLocalTime += aTimeEllapsed;
    /*
    if (mLocalTime < (1.f/30.f) )
    {
        glBindTexture( GL_TEXTURE_2D, mGuiTexture );
        return;
    }
     */
    glBindTexture( GL_TEXTURE_2D, mGuiTexture );
      
    mLocalTime = 0.f;
        
	PROFILER_START(gui::draw);

    if (!mGuiTexture)
    {
        glGenTextures( 1, &mGuiTexture );
        glBindTexture( GL_TEXTURE_2D, mGuiTexture );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 512, 288, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

    int minPixelX(10000), maxPixelX(0);
    int minPixelY(10000), maxPixelY(0);
    
    // black box

    int blackMinX,blackMaxX, blackMinY, blackMaxY;
    

    int aminPixelX = (int)(mCurrentBlackBox.x * 8.f);
    int aminPixelY = (int)(mCurrentBlackBox.y * 8.f);
    int amaxPixelX = (int)((mCurrentBlackBox.x+mCurrentBlackBox.z) * 8.f);
    int amaxPixelY = (int)((mCurrentBlackBox.y+mCurrentBlackBox.w) * 8.f);        
    
    blackMinX = aminPixelX;
    blackMaxX = amaxPixelX;
    blackMinY = aminPixelY;
    blackMaxY = amaxPixelY;
    
    if (mbComputeBlackBoxNextFrame)        
    {
        minPixelX = aminPixelX;
        minPixelY = aminPixelY;
        maxPixelX = amaxPixelX;
        maxPixelY = amaxPixelY;            
    }
    
    // text
	unsigned char *txtBufAv = mTextBuffer;
	for (int y = 0;y< mNbCharsHeight;y++)
	{
		for (int x = 0;x< mNbCharsWidth;x++,             txtBufAv++)
		{ 
            unsigned int charPos = (y*mNbCharsWidth+x);
            if ( ( charsDirty[ charPos>>3 ] & (1<<(charPos&7)) ) )
            {
                //printf("detected : x:%d y:%d \n", x, y);
                minPixelX = zmin( minPixelX, (x<<3) );
                minPixelY = zmin( minPixelY, (y<<3) );
                maxPixelX = zmax( maxPixelX, ((x+1)<<3) );
                maxPixelY = zmax( maxPixelY, ((y+1)<<3) );
                

                unsigned char c = (*txtBufAv);
                //if (c && c != ' ')
                {
                    u32 *pgpp = guiPix + ( x * 8 ) + ( y * 512 * 8 );
                    
                    for ( int cy = 0 ; cy < 8 ; cy ++ )
                    {
                        const u8 *pfs = (const u8*)m8x8FontBits;
                        pfs += (c&0xF)*8 + ( ( c >> 4 ) )*8*128 + ( 7-cy )*128;
                        
                        for (int cx = 0; cx < 8 ; cx ++ )
                        {
                            int pixposx = (x<<3) + cx;
                            int pixposy = (y<<3) + cy;
                            
                            u32 transparentPixel = 0x00000000;
                            
                            if  ( pixposx>= blackMinX && pixposx<blackMaxX &&
                                pixposy>= blackMinY && pixposy<blackMaxY )
                            {
                                if ( mNbSelectors && ( ( y == mSelectors[0][mCurrentSelector[0]] ) || ( y == mSelectors[1][mCurrentSelector[1]] )) )
                                    transparentPixel = 0xA0000000;
                                else
                                    transparentPixel = 0x80000000;
                            
                            }
                            *pgpp++ = ( *pfs++ )?0xFFFFFFFF:transparentPixel;
                        }
                        
                        pgpp+= (512-8);
                    }
                }
            }
        }
    }
    memset( charsDirty, 0, (mNbCharsWidth * mNbCharsHeight)>>3 );
        
    minPixelX = zmax( ( minPixelX - 1 ), 0 );
    minPixelY = zmax( ( minPixelY - 1 ), 0 );    
    
    maxPixelX = zmin( ( maxPixelX + 1 ), 512);
    maxPixelY = zmin( ( maxPixelY + 1 ), 288);    
    
    
    //if (minPixelY<maxPixelY)
    //printf(" black pass (%d %d) (%d %d)\n", minPixelX, minPixelY, maxPixelX, maxPixelY);
    // black pass
//    BlackBorder( guiPix, 512, 288, minPixelX, maxPixelX, minPixelY, maxPixelY );
    

    for ( int y = minPixelY ; y < maxPixelY ; y++ )
    {
        for ( int x = minPixelX ; x < maxPixelX ; x++ )
        {

            if ( guiPix[ (y<<9)+x] == 0xFFFFFFFF )
                continue;

            int sty = ((y-1)>=0)?(y-1):0;
            int stx = ((x-1)>=0)?(x-1):0;
            int ndy = ((y+2)<=288)?(y+2):288;
            int ndx = ((x+2)<=512)?(x+2):512;
            
            for (int my = sty; my <ndy; my++)
            {
                for (int mx = stx; mx < ndx; mx ++)
                {
                    if ( guiPix[ (my<<9) +mx] == 0xFFFFFFFF ) 
                    {
                        guiPix[ (y<<9)+x] = 0xFF000000; 
                        goto hasBlack;
                    }
                }
            }
            //if ( guiPix[ (y<<9)+x] == 0xFF000000 )
            {
                u32 transparentPixel = 0x00000000;
                int pixposx = x;
                int pixposy = y;
                
                if  ( pixposx>= blackMinX && pixposx<blackMaxX &&
                     pixposy>= blackMinY && pixposy<blackMaxY )
                {
                    int chary = (y>>3);
                    int selectory = (mSelectors[0][mCurrentSelector[0]]>>8);
                    if ( mNbSelectors && ( chary == selectory ) )
                        transparentPixel = 0xA0000000;
                    else
                        transparentPixel = 0x80000000;
                    
                }
                guiPix[ (y<<9)+x] = transparentPixel;
                
            }

hasBlack:; 
        }
    }


    glBindTexture( GL_TEXTURE_2D, mGuiTexture );
	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    
    if ( minPixelY < maxPixelY)
    {
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0 ,minPixelY, 512, maxPixelY-minPixelY, GL_RGBA, GL_UNSIGNED_BYTE, guiPix+512*minPixelY );
        //glTexSubImage2D( GL_TEXTURE_2D, 0, 0 ,0, 512, 288, GL_RGBA, GL_UNSIGNED_BYTE, guiPix );
        //printf("%d // %d\n", minPixelY, maxPixelY );
    }
    /*
    else
    {
        printf("tex : %d %d\n", minPixelY, maxPixelY);
    }
     */

	/*
		struct guiBitmap
	{
		guiBitmap( const vec_t & posSize, unsigned int texId ) :
			mTexId( texId), mPosSize( posSize )
		{
		}

		unsigned int mTexId;
		vec_t mPosSize;
	};
	*/
	


    PROFILER_END(); // gui::draw
}


void gui::drawBitmaps() const
{

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 64.f, 0, 36.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glDepthMask(0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    
	for ( unsigned int i =0;i<mBitmaps.size();i++)
	{
		const guiBitmap &gb = mBitmaps[i];
		const vec_t gv = gb.mPosSize;
		glBindTexture(GL_TEXTURE_2D, gb.mTexId );

		glBegin(GL_QUADS);
		glTexCoord2f(0,1);
		glVertex2f( gv.x, gv.y );
		glTexCoord2f(1,1);
		glVertex2f( gv.x + gv.z, gv.y );
		glTexCoord2f(1,0);
		glVertex2f( gv.x + gv.z, gv.y + gv.w );
		glTexCoord2f(0,0);
		glVertex2f( gv.x	   , gv.y + gv.w );
		glEnd();
	}
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(1);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void gui::addSelector( int *sels, unsigned int nbSels, int playerId )
{
	ASSERT_GAME ( (nbSels < MAX_NB_SELECTORS ) );

	if ((!sels) || (!nbSels))
	{
		mNbSelectors[playerId] = 0;
		mCurrentSelector[playerId] = 0;
		return;
	}
	memcpy(mSelectors[playerId], sels, sizeof(int)*nbSels);
	mNbSelectors[playerId] = nbSels;
	mCurrentSelector[playerId] = 0;
	putText(mSelectors[playerId][mCurrentSelector[playerId]]&0xFF, mSelectors[playerId][mCurrentSelector[playerId]]>>8, ">");
    invalidateLine( mSelectors[playerId][mCurrentSelector[playerId]]>> 8 );
}

void gui::selectorNext( int playerId )
{
	if (!mNbSelectors[playerId])
		return;

	putText(mSelectors[playerId][mCurrentSelector[playerId]]&0xFF, mSelectors[playerId][mCurrentSelector[playerId]]>>8, " ");
    invalidateLine( mSelectors[playerId][mCurrentSelector[playerId]]>> 8 );    
	mCurrentSelector[playerId] ++;
	mCurrentSelector[playerId] %= mNbSelectors[playerId];
	putText(mSelectors[playerId][mCurrentSelector[playerId]]&0xFF, mSelectors[playerId][mCurrentSelector[playerId]]>>8, ">");
    invalidateLine( mSelectors[playerId][mCurrentSelector[playerId]]>> 8 );    

	//sndPrevNext->Play();
    //Audio::Push2DAudioEvent(1);
    SoundGui("ButtonNextPrev");

}

void gui::selectorPrevious( int playerId )
{
	if (!mNbSelectors[playerId])
		return;

	putText(mSelectors[playerId][mCurrentSelector[playerId]]&0xFF, mSelectors[playerId][mCurrentSelector[playerId]]>>8, " ");
    invalidateLine( mSelectors[playerId][mCurrentSelector[playerId]]>> 8 );    
	if (mCurrentSelector[playerId])
	{
		mCurrentSelector[playerId] --;
		mCurrentSelector[playerId] %= mNbSelectors[playerId];
	}
	else
		mCurrentSelector[playerId] = mNbSelectors[playerId]-1;

    ASSERT_GAME( mCurrentSelector[playerId] < MAX_NB_SELECTORS );
	putText(mSelectors[playerId][mCurrentSelector[playerId]]&0xFF, mSelectors[playerId][mCurrentSelector[playerId]]>>8, ">");
    invalidateLine( mSelectors[playerId][mCurrentSelector[playerId]]>> 8 );    

	//sndPrevNext->Play();
    SoundGui("ButtonNextPrev");
}

// -- elements

void guiSlider::draw(gui *pg)
{
	pg->putText(x, y, "\21");
	pg->putText(x+width-1, y, "\20");

	for (int i=x+1;i<(x+width-1);i++)
		pg->putText(i, y, "\260");

	int aPos = (int)floorf(LERP( float(x+1), float(x+width-2), (value-min)/(max-min)));

	pg->putText(aPos, y, "\333");

}
void guiCombo::draw(gui *pg)
{
	pg->putText(x, y, "\21");
	pg->putText(x+width-1, y, "\20");

	const char *text = mItems[currentItem];
	int len = strlen(text);
	if (len>(width-2))
	{
		int ind = 0;
		for (int i=x+1;i<(x+width-1);i++)
		{
			pg->putChar(i, y, text[ind++]);
		}
	}
	else
	{
		for (int i=x+1;i<(x+width-1);i++)
		{
			pg->putChar(i, y, ' ');
		}
		int decal = (width-2-len)>>1;
		int ind = 0;
		for (int i=0;i<len;i++)
		{
			pg->putChar(x+1+decal+i, y, text[ind++]);
		}
	}
}

void guiProgress::directDraw( gui *pg, int x, int y, int width, float value )
{
	pg->putText(x, y, "[");
	pg->putText(x+width-1, y, "]");
    
    
	int aPos = (int)floorf(LERP( float(x+1), float(x+width-2), value));
	for (int i=x+1;i<(x+width-1);i++)
		pg->putChar(i, y, i<aPos?'\333':'\260');
    
}

void guiProgress::draw(gui *pg)
{
    directDraw( pg, x, y, width, value );
}

void BlackBorder( u32 * img, int imgw, int imgh, int minPixelX, int maxPixelX, int minPixelY, int maxPixelY )
{
    for ( int y = minPixelY ; y < maxPixelY ; y++ )
    {
        for ( int x = minPixelX ; x < maxPixelX ; x++ )
        {
            
            if ( img[ (y*imgw)+x] == 0xFFFFFFFF )
                continue;
            
            int sty = ((y-1)>=0)?(y-1):0;
            int stx = ((x-1)>=0)?(x-1):0;
            int ndy = ((y+2)<=imgh)?(y+2):imgh;
            int ndx = ((x+2)<=imgw)?(x+2):imgw;
            
            for (int my = sty; my <ndy; my++)
            {
                for (int mx = stx; mx < ndx; mx ++)
                {
                    if ( img[ (my*imgw) +mx] == 0xFFFFFFFF ) 
                    {
                        img[ (y*imgw)+x] = 0xFF000000; 
                        goto hasBlack;
                    }
                }
            }
            
        hasBlack:; 
        }
    }
}

void RenderText( const char *szText, int txtLen, unsigned int imgw, u32* img )
{
    int strCharAv = 0;
    
    for ( unsigned int x = 1 ; (x < imgw)&&(strCharAv<txtLen) ; x+=8, strCharAv++ )
    {
        unsigned char c = szText[strCharAv];
        if (c && c != ' ')
        {
            u32 *pgpp = img + ( x ) + ( imgw );
            
            for ( int cy = 0 ; cy < 8 ; cy ++ )
            {
                const u8 *pfs = (const u8*)m8x8FontBits;
                pfs += (c&0xF)*8 + ( ( c >> 4 ) )*8*128 + ( cy )*128;
                
                for (int cx = 0; cx < 8 ; cx ++ )
                {
                    *pgpp++ = ( *pfs++ )?0xFFFFFFFF:0;
                }
                
                pgpp+= (imgw-8);
            }
        }
    }
}


