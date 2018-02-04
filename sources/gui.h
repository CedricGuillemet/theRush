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

#ifndef GUI_H__
#define GUI_H__


#define GC(x,y) (((y)<<8)+(x))
#define MAX_NB_SELECTORS 256

///////////////////////////////////////////////////////////////////////////////////////////////////

class gui;
class guiElt
{
public:
	virtual ~guiElt() {}
	virtual void applyPrev(float /*aTimeEllapsed*/) {}
	virtual void applyNext(float /*aTimeEllapsed*/) {}
	virtual void applyPrevReleased(float /*aTimeEllapsed*/) {}
	virtual void applyNextReleased(float /*aTimeEllapsed*/) {}

	virtual void draw(gui * /*pg*/ ) {}
	virtual void setFrame(int _x, int _y, int _width, int _height) { x= _x;y=_y;width=_width;height=_height; }
	int x,y, width, height;
};

class guiSlider : public guiElt
{
public:
	virtual ~guiSlider() {}
	virtual void applyPrev(float aTimeEllapsed) { value = Clamp( (value-step*aTimeEllapsed), min, max); }
	virtual void applyNext(float aTimeEllapsed) { value = Clamp( (value+step*aTimeEllapsed), min, max); }
	virtual void draw(gui *pg);
	void set(float _min, float _max, float _value, float _step) { min = _min; max= _max; value=_value; step=_step; }
	float min, max, value, step;
};

class guiCombo : public guiElt
{
public:
	virtual ~guiCombo() {}

	void setItems(const char **szItems, int aCurrentItem, int aNbItems) { mItems = szItems; nbItems = aNbItems; currentItem = aCurrentItem; }
	virtual void draw(gui *pg);
	virtual void applyPrevReleased(float /*aTimeEllapsed*/) 
    { 
        currentItem += (nbItems-1);
        currentItem %= nbItems; 
    }
	virtual void applyNextReleased(float /*aTimeEllapsed*/) 
    { 
        ++currentItem %= nbItems; 
    }
	unsigned int nbItems;
	unsigned int currentItem;
	const char **mItems;
};

class guiProgress : public guiElt
{
public:
	virtual ~guiProgress() {}
	virtual void applyPrev(float aTimeEllapsed) { UNUSED_PARAMETER(aTimeEllapsed); }
	virtual void applyNext(float aTimeEllapsed) { UNUSED_PARAMETER(aTimeEllapsed); }
	virtual void draw(gui *pg);
	void setValue(float val) { value = Clamp(val, 0.f, 1.f); }
	float value;
    static void directDraw( gui *pg, int x, int y, int width, float value );
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class gui
{
public:
	gui(void);
	~gui(void);
	void init();

	void draw( float aTimeEllapsed );
	void tick(float aTimeEllapsed);
	void setResolution(int nbCharsWidth, int nbCharsHeight);

	void putText(int X, int Y, const char *pszText, bool bRevert = false);
	void putChar(int X, int Y, unsigned char achar);
	void clearText( int stx = 0, int sx = 64, int sty = 0, int sy = 36 ); 
	void setBlackBox(int X, int Y, int width, int height);

	// selectors
	void addSelector( int *sels, unsigned int nbSels, int playerId = 0 );
	void selectorNext( int playerId = 0);
	void selectorPrevious( int playerId = 0 );
	int getSelectorIndex( int playerId = 0) const { return mCurrentSelector[playerId]; }
	void setSelectorIndex(int idx, int playerId = 0) 
	{ 
		ASSERT_GAME ( (idx<MAX_NB_SELECTORS ) );
		putText(mSelectors[playerId][mCurrentSelector[playerId]]&0xFF, mSelectors[playerId][mCurrentSelector[playerId]]>>8, " ");
		mCurrentSelector[playerId] = idx; 
		putText(mSelectors[playerId][mCurrentSelector[playerId]]&0xFF, mSelectors[playerId][mCurrentSelector[playerId]]>>8, ">");
	}
	int getSelectorCount( int playerId = 0) const { return mNbSelectors[playerId]; }

	// --- 

	guiSlider* addSlider() { guiSlider* sld = new guiSlider; mElements.push_back(sld); return sld; }

	struct guiBitmap
	{
		guiBitmap( const vec_t & posSize, unsigned int texId ) :
			mTexId( texId), mPosSize( posSize )
		{
		}

		unsigned int mTexId;
		vec_t mPosSize;
	};
	void addBitmap( const guiBitmap& gb ) { mBitmaps.push_back( gb ); }
	void clearBitmaps() { mBitmaps.clear(); }
	void drawBitmaps() const;
	bool BlackBoxIsStable() const { return mbBlackBoxIsStable; }

protected:
    float mLocalTime;
    bool mbBlackBoxIsStable, mbComputeBlackBoxNextFrame;
	std::vector<guiBitmap> mBitmaps;

    //
	unsigned int mTexID;
	int mNbCharsWidth,mNbCharsHeight;
	unsigned char *mTextBuffer;
	vec_t mCurrentBlackBox, mTargetedBlackBox;
	
	// selectors
	int mSelectors[2][MAX_NB_SELECTORS+1];
	unsigned int mNbSelectors[2];
	unsigned int mCurrentSelector[2];	
	// selectors

    u32 *guiPix;
    u8* charsDirty;
	// elements
	std::vector<guiElt*> mElements;

	// sounds
    
    void invalidateLine( int iLine );
};

///////////////////////////////////////////////////////////////////////////////////////////////////

void BlackBorder( u32 * img, int imgw, int imgh, int minPixelX, int maxPixelX, int minPixelY, int maxPixelY );
void RenderText( const char *szText, int txtLen, unsigned int imgw, u32* img );

extern const char * m8x8FontBits;

extern gui ggui;

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
