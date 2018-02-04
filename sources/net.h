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

#ifndef NET_H__
#define NET_H__

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "game.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct server_t
{
	server_t() 
    {
        mTrack = 0;
        mPing = 999;
        mMode = 0;
        mNbLaps = 0;
        mbIsLan = true;
        mNbPlayers = 1;
        mMaxNbPlayers = Game::E_Max_Ship_Count;
        mTimeOut = 0.f;
    }
	server_t( u16 _Track, u16 _Ping, u8 _Mode, u8 _NbLaps, bool _bIsLan, u8 _NbPlayers, u8 _MaxNbPlayers, std::string _URL )
	{
		mTrack = _Track;
		mPing = _Ping;
		mMode = _Mode;
        mNbLaps = _NbLaps;
		mbIsLan = _bIsLan;
		mNbPlayers = _NbPlayers;
        mMaxNbPlayers = _MaxNbPlayers;
		mURL = _URL;
	}

	u16 mTrack;
	u16 mPing;
	u8 mMode;// 0xFF means lobby
    u8 mNbLaps;
	bool mbIsLan; // if not, is Internet
	u8 mNbPlayers;
    u8 mMaxNbPlayers;
	std::string mURL;
	std::string Format() const;
    float mTimeOut;
} server_t;

typedef struct server_advert_data_t
{
    u8 mTrack;
    u8 mMode;
    u8 mNbLaps;
    u8 mNbPlayers;
    u8 mMaxNbPlayers;
}server_advert_data_t;

typedef struct playerNetInfos_t
{
    playerNetInfos_t()
    {
        mShip = NULL;
    }
    std::string mNickName;
    shipIdentifier_t mShipID;
    GameShip *mShip;
    u8 mNetIndex;
    bool mbIsReady;
}playerNetInfos_t;

struct playerTickInfos_t
{
    vec_t position;
    vec_t orientation;
    vec_t linearVelocity;
    vec_t angulraVelocity;
};

class GameObjectNet;

class NetworkInterface
{
public:
    virtual ~NetworkInterface() {}
	virtual void ConnectTo(const std::string& urlToConnectTo) = 0;
	virtual void CreateServer() = 0;
	virtual void Close() = 0;

    virtual bool Active() const = 0;

	virtual void Tick(float aTimeEllapsed) = 0;

    // players
    virtual u8 ClientID() = 0;
    // setplayerinfos must be called before connect
    virtual void SetPlayerInfos( const char *szName, const shipIdentifier_t &mShipID ) = 0;
    virtual int GetConnectedPlayers() const = 0;
    virtual const playerNetInfos_t* GetPlayerInfos( int idx ) const = 0;

    // object
    virtual GameObjectNet* Spawn( u8 objectType, void *pGameObject ) = 0;
    virtual void Destroy( GameObjectNet *pObj ) = 0;
    virtual void *GetObjectByUID( u32 uid ) = 0;

    // game sync
    virtual void informServerImReadyToPlay() = 0;

	virtual void BroadcastStartRace() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

extern NetworkInterface *NetClient;
extern NetworkInterface *NetServer;

// lan discovery

void StartLanDiscovery();
void StopLanDiscovery();
bool IsLanDiscoveryInProgress();
void TickLanDiscovery( float aTimeEllapsed );

// servers available
const std::vector<server_t> & GetActiveServers();

// advertised server info
void UpdateAdvertisedServerInfo( const server_advert_data_t& updatedInfo );

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
