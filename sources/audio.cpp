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

#include "audio.h"
#include "content.h"

#include "include_OpenAL.h"


///////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{
    int stb_vorbis_decode_memory( unsigned char *mem, int len, int *channels, short **output);
    int stb_vorbis_decode_memory2(uint8 *mem, int len, int *channels, short **output, float *duration);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// sounds

void Sound::Init( int nbChannels, short *soundData, int size, unsigned int freq )
{
    alGenBuffers( 1, &sampleIdx );
    alBufferData( sampleIdx, (nbChannels==1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16, soundData, size, freq );
}

Sound::~Sound()
{
    alDeleteBuffers(1, &sampleIdx);
}


std::map<uint32, Sound> AllSounds;

void Audio::DecodeOgg( uint32 hash, const std::string& oggFile)
{
    int nbChannels = 0;
    short *output;
    float duration;
    int resDecode = stb_vorbis_decode_memory2( (unsigned char*)oggFile.c_str(), oggFile.length(), &nbChannels, &output, &duration );
    unsigned int freq = static_cast<unsigned int>(static_cast<float>(resDecode)/duration);
    printf("OGG freq = %d\n", freq );
    AllSounds.insert( std::make_pair( hash, Sound() ) );
    AllSounds[hash].Init( nbChannels, output, (resDecode<<1), freq );
}

const Sound* Audio::GetSound( uint32 hash )
{
    std::map<uint32, Sound>::const_iterator iter = AllSounds.find( hash );

    if ( iter != AllSounds.end() )
        return &(*iter).second;

    const std::string& oggFile = getFileFromMemory( hash );
    if ( oggFile.empty() )
    {
        LOG("Audio file hash %d file not found", hash );
        return NULL;
    }

    DecodeOgg( hash, oggFile );

    return &AllSounds[hash];
}

const Sound* Audio::GetSound( const char *szFilename )
{
    uint32 hash = superFastHash( szFilename, strlen( szFilename ) );
    std::map<uint32, Sound>::const_iterator iter = AllSounds.find( hash );

    if ( iter != AllSounds.end() )
        return &(*iter).second;

    const std::string& oggFile = getFileFromMemory( hash );
    if ( oggFile.empty() )
    {
        LOG("Audio file %s file not found", szFilename );
        return NULL;
    }

    DecodeOgg( hash, oggFile );

    return &AllSounds[hash];

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Channel

Channel *Channel::AllocateChannel( const Sound *psnd, AUDIOGROUP grp )
{
    Channel *pchan = NULL;

    if ( mGChannels.Size() < MAX_CHANNEL)
    {
        pchan = new Channel;

        alGenSources( 1, &pchan->channelIdx );
        //NOTE: it can fail if it couldn't load dynamically the library
        //or because of some audio driver issue

        if ( pchan->channelIdx != Invalid_Channel_Index )
        {
            alSourceQueueBuffers( pchan->channelIdx, 1, psnd->GetSampleIndex() );

            // constructor
            pchan->mbDestroyOnStop = true;
            pchan->mGrp = grp;
            pchan->mbPaused = true;
            pchan->mEffectIndex = 0;
            pchan->mVolume = 1.f;

            *mGChannels.Push() = pchan;

            const bool bUseRelativeCoordinates = ( grp != AUDIO_GROUP_GAME3D );
            pchan->SetRelative( bUseRelativeCoordinates );
        }
        else
        {
            delete pchan;
            pchan = NULL;
        }
    }

    //NOTE: pchan can be NULL

    return pchan;
}

void Channel::DestroyChannel( Channel *pChan )
{
    ASSERT_GAME( pChan != NULL );
    ASSERT_GAME( pChan->channelIdx != Invalid_Channel_Index );

    ASSERT_GAME( mGChannels.Contains( pChan ) == true );

    if (pChan->IsPlaying() )
        alSourceStop( pChan->channelIdx );

    alDeleteSources(1, &pChan->channelIdx );

    delete pChan;

    ASSERT_GAME( mGChannels.Contains( pChan ) == true );
    mGChannels.Erase( pChan );
    ASSERT_GAME( mGChannels.Contains( pChan ) == false );
}

void Channel::DestroyAllChannels()
{
    //HACK: DestroyChannel() modifies mGChannels[], it copies/moves the last element to the element which has just been destroyed and decrease the number of elements
    // In this consideration, it makes more sense to Destroy the last element to avoid unnecessary copy
    while ( !mGChannels.Empty() )
    {
        Channel *pLastChan = *mGChannels.GetLastToModify();
        DestroyChannel( pLastChan );
    }

}

void Channel::DestroyAllChannelsByGroup( AUDIOGROUP aGrp )
{
    //HACK: DestroyChannel() modifies mGChannels[], it copies/moves the last element to the element which has just been destroyed and decrease the number of elements
    // In this consideration, parsing mGChannels[] from last to first makes more sense to simplify parsing
    for ( int i = mGChannels.Size()-1 ; i >= 0; --i )
    {
        Channel* pChan = mGChannels[i];
        if ( pChan->GetGroup() == aGrp )
            DestroyChannel( pChan );
    }
}

Channel::Channel() :
    channelIdx(Invalid_Channel_Index),
    mbDestroyOnStop(false),
    mbPaused(false),
    mGrp(AUDIO_GROUP_NONE),
    mEffectIndex(-1),
    mVolume(-1.f)
{

}

Channel::~Channel()
{
    ASSERT_GAME( channelIdx == Invalid_Channel_Index || mGChannels.Contains(this) );
}

void Channel::SetRelative( bool bRelative )
{
    ASSERT_GAME( mGChannels.Contains(this) );
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSourcei( channelIdx, AL_SOURCE_RELATIVE, bRelative?AL_TRUE:AL_FALSE );
}

void Channel::SetLoop( bool bLoop )
{
    ASSERT_GAME( mGChannels.Contains(this) );
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSourcei( channelIdx, AL_LOOPING, bLoop?AL_TRUE:AL_FALSE );
}

void Channel::SetDestroyOnStop( bool bDestroy )
{
    ASSERT_GAME( mGChannels.Contains(this) );
    mbDestroyOnStop = bDestroy;
}

void Channel::Play()
{
    ASSERT_GAME( mGChannels.Contains(this) );
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSourcePlay( channelIdx );
    mbPaused = false;
}

void Channel::Pause()
{
    ASSERT_GAME( mGChannels.Contains(this) );
    ASSERT_GAME( mbPaused == false );
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSourcePause( channelIdx );
    mbPaused = true;
}

void Channel::Stop()
{
    ASSERT_GAME( mGChannels.Contains(this) );
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSourceStop( channelIdx );
    mbPaused = false;
}

void Channel::WaitForDestroy()
{
    ASSERT_GAME( mGChannels.Contains(this) );
    ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    ASSERT_GAME( mbDestroyOnStop == false );
    ASSERT_GAME( mbPaused == false );
    mbPaused = true;
}

bool Channel::IsPlaying() const
{
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );

    ALint state;

    /* Get relevant source info */
    alGetSourcei( channelIdx, AL_SOURCE_STATE, &state);

    //alSource( AL_PLAYING );
    return ( state == AL_PLAYING);
}

void Channel::Tick( float aTimeEllapsed )
{
    UNUSED_PARAMETER(aTimeEllapsed);

    //HACK: DestroyChannel() modifies mGChannels[], it copies/moves the last element to the element which has just been destroyed and decrease the number of elements
    // In this consideration, parsing mGChannels[] from last to first makes more sense to simplify parsing
    for ( int i = mGChannels.Size()-1 ; i >= 0; --i )
    {
        Channel* pChan = mGChannels[i];
        ASSERT_GAME( pChan->channelIdx != Invalid_Channel_Index );

        if ( !pChan->IsPaused() )
        {
            if ( pChan->IsPlaying() )
            {
                alSourcef( pChan->channelIdx, AL_GAIN, Channel::mGroupVolumes[pChan->mGrp] * pChan->mVolume );
            }
            else if( pChan->mbDestroyOnStop )
            {
                Channel::DestroyChannel( pChan );
                pChan = NULL;
            }
            else
            {
                //HACK: marking Channel has paused to avoid being ticked while waiting for being destroyed
                pChan->WaitForDestroy();
            }
        }
    }
}

void Channel::SetVolume( float volume )
{
    //alSourcef( channelIdx, AL_GAIN, volume );
    mVolume = volume;
}

void Channel::SetPitch( float pitch )
{
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSourcef( channelIdx, AL_PITCH, pitch );
}

void Channel::SetPosition( const vec_t& pos )
{
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSource3f( channelIdx, AL_POSITION, pos.x, pos.y, pos.z);
}

void Channel::SetVelocity( const vec_t& vel )
{
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );
    alSource3f( channelIdx, AL_VELOCITY, vel.x, vel.y, vel.z );
}


// effects

ALuint mEffectSlots[1];
ALuint mEffects[1];

void Channel::SetEffect( int effectIndex )
{
	ASSERT_GAME( channelIdx != Invalid_Channel_Index );

    ASSERT_GAME( effectIndex == 0 || effectIndex == 1 );
    mEffectIndex = effectIndex;

    // Enable (non-filtered) Send from Source to Auxiliary Effect Slot
    const ALuint effectSlot = ( effectIndex == 0 )? AL_EFFECTSLOT_NULL : mEffectSlots[0];
    alSource3i( channelIdx, AL_AUXILIARY_SEND_FILTER, effectSlot, 0, AL_FILTER_NULL );
}

void Channel::SetEffect( int effectIndex, AUDIOGROUP grp )
{
    for ( int i = mGChannels.Size()-1 ; i >= 0; --i )
    {
        Channel& channel = *mGChannels[i];

        if ( channel.GetGroup() == grp )
            channel.SetEffect( effectIndex );
    }
}

FastArray<Channel*, MAX_CHANNEL> Channel::mGChannels;
float Channel::mGroupVolumes[AUDIO_GROUP_COUNT] = { 1.f, 1.f, 1.f, 1.f };

///////////////////////////////////////////////////////////////////////////////////////////////////

void Audio::ClearSounds()
{
    AllSounds.clear();
}

void Audio::SetGroupVolume( AUDIOGROUP group, float volume )
{
    Channel::mGroupVolumes[ group ] = volume;
}

void Audio::PlaySound( const Sound *psound, AUDIOGROUP grp )
{
    Channel *pChan = Channel::AllocateChannel( psound, grp );
    if ( pChan != NULL )
        pChan->Play();
}

void Audio::PlayPauseGroup( AUDIOGROUP group, bool bPlay )
{
    for ( int i = Channel::mGChannels.Size()-1 ; i >= 0; --i )
    {
        Channel& channel = *Channel::mGChannels[i];

        if ( channel.GetGroup() == group )
        {
            if ( bPlay )
            {
                channel.Play();
            }
            else if ( !channel.IsPaused() )
            {
                channel.Pause();
            }
        }
    }
}

ALboolean setReverbProperties(ALuint _effect, EFXEAXREVERBPROPERTIES *reverbProp)
{
	ALboolean bReturn = AL_FALSE;

	if (reverbProp)
	{
		// Clear AL Error code
		alGetError();

		alEffectf(_effect, AL_EAXREVERB_DENSITY, reverbProp->flDensity);
		alEffectf(_effect, AL_EAXREVERB_DIFFUSION, reverbProp->flDiffusion);

		alEffectf(_effect, AL_EAXREVERB_GAIN, reverbProp->flGain);
		alEffectf(_effect, AL_EAXREVERB_GAINHF, reverbProp->flGainHF);
		alEffectf(_effect, AL_EAXREVERB_GAINLF, reverbProp->flGainLF);

		alEffectf(_effect, AL_EAXREVERB_DECAY_TIME, reverbProp->flDecayTime);
		alEffectf(_effect, AL_EAXREVERB_DECAY_HFRATIO, reverbProp->flDecayHFRatio);
		alEffectf(_effect, AL_EAXREVERB_DECAY_LFRATIO, reverbProp->flDecayLFRatio);
/*
		if(_useEarlyReflection && isRoomValid())
		{
			// set all early reflections to zero.
			// we wil simulate early reflections.
			alEffectf(_effect, AL_EAXREVERB_REFLECTIONS_GAIN, 0);
			alEffectf(_effect, AL_EAXREVERB_REFLECTIONS_DELAY, 0);
			alEffectfv(_effect, AL_EAXREVERB_REFLECTIONS_PAN, reverbProp->flReflectionsPan);
		}
		else
        */
		{
			alEffectf(_effect, AL_EAXREVERB_REFLECTIONS_GAIN, reverbProp->flReflectionsGain);
			alEffectf(_effect, AL_EAXREVERB_REFLECTIONS_DELAY, reverbProp->flReflectionsDelay);
			alEffectfv(_effect, AL_EAXREVERB_REFLECTIONS_PAN, reverbProp->flReflectionsPan);
		}

		alEffectf(_effect, AL_EAXREVERB_LATE_REVERB_GAIN, reverbProp->flLateReverbGain);
		alEffectf(_effect, AL_EAXREVERB_LATE_REVERB_DELAY, reverbProp->flLateReverbDelay);
//		alEffectf(_effect, AL_EAXREVERB_LATE_REVERB_DELAY, _lateReverbDelay);
		alEffectfv(_effect, AL_EAXREVERB_LATE_REVERB_PAN, reverbProp->flLateReverbPan);

		alEffectf(_effect, AL_EAXREVERB_ECHO_TIME, reverbProp->flEchoTime);
		alEffectf(_effect, AL_EAXREVERB_ECHO_DEPTH, reverbProp->flEchoDepth);

		alEffectf(_effect, AL_EAXREVERB_MODULATION_TIME, reverbProp->flModulationTime);
		alEffectf(_effect, AL_EAXREVERB_MODULATION_DEPTH, reverbProp->flModulationDepth);

		alEffectf(_effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverbProp->flAirAbsorptionGainHF);

		alEffectf(_effect, AL_EAXREVERB_HFREFERENCE, reverbProp->flHFReference);
		alEffectf(_effect, AL_EAXREVERB_LFREFERENCE, reverbProp->flLFReference);

		alEffectf(_effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverbProp->flRoomRolloffFactor);

		alEffecti(_effect, AL_EAXREVERB_DECAY_HFLIMIT, reverbProp->iDecayHFLimit);

		if (alGetError() == AL_NO_ERROR)
		{
			bReturn = AL_TRUE;
		}
	}

	return bReturn;
}

bool mbAudioInited = false;

void Audio::Init()
{
    ALCdevice *device;
    ALCcontext *ctx;

    /* Open and initialize a device with default settings */
    device = alcOpenDevice(NULL);
    if(!device)
    {
        LOG("OpenAL: Could not open a device!\n");
        return;
    }

    ctx = alcCreateContext(device, NULL);
    if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if(ctx != NULL)
            alcDestroyContext(ctx);
        alcCloseDevice(device);
        LOG( "OpenAL: Could not set a context!\n");
        return;

    }

    // Generate an Auxiliary Effect Slot
    alGenAuxiliaryEffectSlots(1, mEffectSlots);

    alGenEffects(1, mEffects);
    if (alGetError() == AL_NO_ERROR)
    {
        // Set the Effect Type
        alEffecti( mEffects[0], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB );
        if (alGetError() != AL_NO_ERROR)
        {
            alDeleteEffects(1, &mEffects[0]);
        }
    }
    /*
    EFXEAXREVERBPROPERTIES efxReverb;
    setReverbParameters(_roomType, &efxReverb);

    // Set the Effect parameters
    if (!setReverbProperties(&efxReverb))
    {
        LOG("Failed to set Reverb Parameters\n");
    }
    */


    EFXEAXREVERBPROPERTIES effectsToLoad[]={ EFX_REVERB_PRESET_OUTDOORS_DEEPCANYON/*EFX_REVERB_PRESET_DRIVING_TUNNEL*/ };
    setReverbProperties( mEffects[0], &effectsToLoad[0] );
    // Load Effect into Auxiliary Effect Slot
    alAuxiliaryEffectSloti(mEffectSlots[0], AL_EFFECTSLOT_EFFECT, mEffects[0]);

    mbAudioInited = true;
    LOG("Audio : Channels uses %d bytes.\n", (int)Channel::GetUsedMemory() );
}

void Audio::Tick( float aTimeEllapsed )
{
    if (!mbAudioInited)
        return;

    Channel::Tick( aTimeEllapsed );
}

void Audio::Uninit()
{
    if (!mbAudioInited)
        return;

    Audio::ClearSounds();
    ALCdevice *device;
    ALCcontext *ctx;

    alAuxiliaryEffectSloti(mEffectSlots[0], AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
    alDeleteEffects( 1, mEffects );
	alDeleteAuxiliaryEffectSlots( 1, mEffectSlots );


    /* Close the device belonging to the current context, and destroy the
     * context. */
    ctx = alcGetCurrentContext();
    if(ctx == NULL)
        return;

    device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(device);

}

static bool CameraPositionValid = false;
static vec_t lastCameraPosition;
void Audio::InvalidatePosition()
{
    CameraPositionValid = false;
}

void Audio::SetListener( const matrix_t& mat, float aTimeEllapsed )
{
    if ( CameraPositionValid )
    {
        vec_t diff = mat.position - lastCameraPosition;
        diff *= (1.f / aTimeEllapsed );
        diff *= 0.003f;
        alListener3f( AL_VELOCITY, diff.x, diff.y, diff.z );
    }
    else
    {
        alListener3f(AL_VELOCITY, 0, 0, 0);
    }

    float listenDirUp[]={ -mat.dir.x, -mat.dir.y, -mat.dir.z,
        mat.up.x, mat.up.y, mat.up.z };
    alListenerfv( AL_ORIENTATION, listenDirUp );
    alListener3f( AL_POSITION, mat.position.x, mat.position.y, mat.position.z );


    lastCameraPosition = mat.position;
    CameraPositionValid = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "fmod.hpp"
#include "fmod_errors.h"

#ifdef RETAIL
bool GBEditorInited = false;
#else
extern bool GBEditorInited;
#endif

namespace AudioMusic
{

FMOD::System     *system;
FMOD::Sound      *sound = NULL;
FMOD::Channel    *channel = 0;
float musicVolume = 1.f;
void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}


/*
    TIPS:

    1. use F_CALLBACK.  Do NOT force cast your own function to fmod's callback type.
    2. return FMOD_ERR_FILE_NOTFOUND in open as required.
    3. return number of bytes read in read callback.  Do not get the size and count 
       around the wrong way in fread for example, this would return 1 instead of the number of bytes read.

    QUESTIONS:

    1. Why does fmod seek to the end and read?  Because it is looking for ID3V1 tags.  
       Use FMOD_IGNORETAGS in System::createSound / System::createStream if you don't like this behaviour.

*/

FMOD_RESULT F_CALLBACK myopen(const char *name, int /*unicode*/, unsigned int *filesize, void **handle, void ** userdata)
{
    if (name)
    {
        FILE *fp;

        fp = fopen(name, "rb");
        if (!fp)
        {
            return FMOD_ERR_FILE_NOTFOUND;
        }

        fseek(fp, 0, SEEK_END);
        *filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *userdata = (void *)0x12345678;
        *handle = fp;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK myclose(void *handle, void * /*userdata*/)
{
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    fclose((FILE *)handle);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK myread(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void * /*userdata*/)
{
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (bytesread)
    {
        *bytesread = (int)fread(buffer, 1, sizebytes, (FILE *)handle);
    
        if (*bytesread < sizebytes)
        {
            return FMOD_ERR_FILE_EOF;
        }
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK myseek(void *handle, unsigned int pos, void * /*userdata*/)
{
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    fseek((FILE *)handle, pos, SEEK_SET);

    return FMOD_OK;
}


int InitMusic()
{
    FMOD_RESULT       result;
    unsigned int      version;

    /*
        Create a System object and initialize.
    */
    result = FMOD::System_Create(&system);
    ERRCHECK(result);

    result = system->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }

    result = system->init(1, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    result = system->setFileSystem(myopen, myclose, myread, myseek, 0, 0, 2048);
    ERRCHECK(result);
	return 1;
}

void UninitMusic()
{
	FMOD_RESULT       result;
	if (channel)
		channel->stop();
	if (sound)
	{
		result = sound->release();
		ERRCHECK(result);
	}
    result = system->close();
    ERRCHECK(result);
    result = system->release();
    ERRCHECK(result);
}

bool StreamMusic( const char *szFileName )
{
	FMOD_RESULT       result;

	if (sound)
		sound->release();

    result = system->createStream( szFileName, FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound );
    //ERRCHECK(result);
	if (result != FMOD_OK)
	{
		return false;
	}
    result = system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
    ERRCHECK(result);
	channel->setVolume( musicVolume );

	return true;
}


void PlayPauseMusic()
{
	if (channel)
	{
		bool paused;
		channel->getPaused(&paused);
		channel->setPaused(!paused);
	}
}



void TickMusic()
{
	if (GBEditorInited)
		return;

	if (channel)
	{
		bool paused;
		channel->getPaused(&paused);
		if (!paused)
		{
			bool playing;
			channel->isPlaying(&playing);
			if (!playing)
				PlayRandomSong();
		}
	}
}

void SetMusicVolume( float volume )
{
	musicVolume = volume;
	if (channel)
	{
		channel->setVolume( volume );
	}
}

int lastSongPlayedIndex = -1;
std::vector<std::string> SongsList;


void PlayRandomSong()
{
	if (GBEditorInited)
		return;

	if (SongsList.size() < 2 )
		return;

	int newRandom = fastrand()%SongsList.size();
	while ( newRandom == lastSongPlayedIndex)
		newRandom = fastrand()%SongsList.size();
	
	lastSongPlayedIndex = newRandom;
	if (!StreamMusic( SongsList[newRandom].c_str() ))
		PlayRandomSong();
}

void BuildSongsList()
{
    const char *szMusicExts[] = {".mp3",".ogg"};

    for (int i = 0;i<sizeof(szMusicExts)/sizeof(const char*);i++)
    {
        GetFilesList( SongsList, "Musics/", szMusicExts[i], false, false, true );
    }
}

}