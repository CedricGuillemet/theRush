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

#ifndef AUDIO_H__
#define AUDIO_H__

///////////////////////////////////////////////////////////////////////////////////////////////////

class Sound
{
public:
    Sound( )
    {
        sampleIdx = 0;
    }
    void Init( int nbChannels, short *soundData, int size, unsigned int freq );
    ~Sound();
    const unsigned int* GetSampleIndex() const { return &sampleIdx; }
protected:
    unsigned int sampleIdx;    
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_CHANNEL 1024

enum AUDIOGROUP
{
    AUDIO_GROUP_NONE = -1,
    AUDIO_GROUP_GAME3D = 0,
    AUDIO_GROUP_GAME2D = 1,
    AUDIO_GROUP_GUI = 2,
    AUDIO_GROUP_MUSIC = 3,
    AUDIO_GROUP_COUNT = 4
};

struct Channel
{
    //
    static Channel *AllocateChannel( const Sound *psnd, AUDIOGROUP grp );
    static void DestroyChannel( Channel *pchan );
    static void Tick( float aTimeEllapsed );
    static void DestroyAllChannels();
    static void DestroyAllChannelsByGroup( AUDIOGROUP aGrp );

    //
    Channel();
    ~Channel();

    // 
    void Play();
    void Pause();
    void Stop();
    void WaitForDestroy();
    bool IsPlaying() const ;
    bool IsPaused() const { return mbPaused; }
    void SetVolume( float volume );
    void SetPitch( float pitch );
    void SetRelative( bool bRelative );

    // props
    void SetLoop( bool bLoop );
    void SetDestroyOnStop( bool bDestroy );
    AUDIOGROUP GetGroup() const { return mGrp; }
    
    // effects
    void SetEffect( int effectIndex );
    static void SetEffect( int effectIndex, AUDIOGROUP grp );

    // 3D
    void SetPosition( const vec_t& pos );
    void SetVelocity( const vec_t& vel );

    static u32 GetUsedMemory() { return sizeof(mGChannels) + mGChannels.Size() * sizeof(Channel); }

private:
    
    friend void initAudio();
    friend class Audio;
    
    //
    enum { Invalid_Channel_Index = U16_MAX };
    unsigned int channelIdx;

    // props
    bool mbDestroyOnStop;
    bool mbPaused;
    AUDIOGROUP mGrp;
    int mEffectIndex;
    float mVolume;

    static FastArray<Channel*, MAX_CHANNEL> mGChannels;
    static float mGroupVolumes[AUDIO_GROUP_COUNT];
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Audio
{
public:
    // engine life
    static void Init();
    static void Tick( float aTimeEllapsed );
    static void Uninit();
 
    // sound
    static const Sound* GetSound( const char *szFilename );
    static const Sound* GetSound( uint32 hash );    
    static void ClearSounds();
    
    // groups
    static void SetGroupVolume( AUDIOGROUP group, float volume );
    static void PlayPauseGroup( AUDIOGROUP group, bool bPlay );    
    
    // helper
    static void PlaySound( const Sound *psound, AUDIOGROUP grp );
    
    // position
    static void InvalidatePosition();
    static void SetListener( const matrix_t& mat, float aTimeEllapsed );

	
protected:
    static void DecodeOgg( uint32 hash, const std::string& oggFile);
};

#define SoundGui(x) Audio::PlaySound( Audio::GetSound( "Datas/Sounds/menu/"x".ogg"), AUDIO_GROUP_GUI ) ;
#define SoundDroid(x) Audio::PlaySound( Audio::GetSound( "Datas/Sounds/droid/"x".ogg"), AUDIO_GROUP_GAME2D ) ;

// music
namespace AudioMusic
{
int InitMusic();
void UninitMusic();
void TickMusic();
bool StreamMusic( const char *szFileName);
void PlayPauseMusic();
void SetMusicVolume( float volume );
void PlayRandomSong();
void BuildSongsList();
}
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif

