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

#include "net.h"

#include "maths.h"
#include "toolbox.h"
#include "track.h"
#include "menus.h"
#include "camera.h"
#include "world.h"
#include "bonus.h"
#include "gui.h"

#ifdef WIN32
//#include "WindowsIncludes.h" // Sleep
#else
#include <unistd.h> // usleep
#include <cstdio>
#include "Getche.h"
#endif

#include "include_RakNet.h"

using namespace RakNet;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define SERVER_PORT 21177

///////////////////////////////////////////////////////////////////////////////////////////////////

void setDOFFocus(float aFocus);
void setDOFBlur(float aBlur);



bool GBInsideClientNetTick = false;
MENU_IDS menuToShow = MENU_NONE;

enum ZNETMESSAGE
{
    ZNMSG_NICKNAME  = ID_USER_PACKET_ENUM,
    ZNMSG_NICKNAMESLIST,
    ZNMSG_SHIPCONTROLS,
    ZNMSG_USERREADY, 
    ZNMSG_STARTRACE,
    ZNMSG_NETINDEX,

    ZNMSG_REPLICA_CREATE,
    ZNMSG_REPLICA_MODIFY,
    ZNMSG_REPLICA_DELETE,
};


class NetworkIDManager;
class RakPeerInterface;
class GameNet;

///////////////////////////////////////////////////////////////////////////////////////////////////
// servers
std::vector<server_t> GServers;
const std::vector<server_t> & GetActiveServers() { return GServers; }

server_advert_data_t GAdvertisedServerInfo;
bool GBAdvertisedServerChanged = false;

void UpdateAdvertisedServerInfo( const server_advert_data_t& updatedInfo )
{
    const bool bTrackChanged = (GAdvertisedServerInfo.mTrack != updatedInfo.mTrack);
    const bool bModeChanged = (GAdvertisedServerInfo.mMode != updatedInfo.mMode);
    const bool bNbLapsChanged = (GAdvertisedServerInfo.mNbLaps != updatedInfo.mNbLaps);
    const bool bNbPlayersChanged = (GAdvertisedServerInfo.mNbPlayers != updatedInfo.mNbPlayers);
    const bool bMaxNbPlayersChanged = (GAdvertisedServerInfo.mMaxNbPlayers != updatedInfo.mMaxNbPlayers);

    GAdvertisedServerInfo.mTrack = updatedInfo.mTrack;
    GAdvertisedServerInfo.mMode = updatedInfo.mMode;
    GAdvertisedServerInfo.mNbLaps = updatedInfo.mNbLaps;
    GAdvertisedServerInfo.mNbPlayers = updatedInfo.mNbPlayers;
    GAdvertisedServerInfo.mMaxNbPlayers = updatedInfo.mMaxNbPlayers;

    GBAdvertisedServerChanged = bTrackChanged || bModeChanged || bNbLapsChanged || bNbPlayersChanged || bMaxNbPlayersChanged;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// LAN discovery

RakNet::RakPeerInterface *lanDiscoveryClient = NULL;
float lanDiscoveryTimeBeforeBroadcast;

void StartLanDiscovery()
{
    lanDiscoveryClient=RakNet::RakPeerInterface::GetInstance();

    RakNet::SocketDescriptor socketDescriptor(0,0);
    socketDescriptor.socketFamily=AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
    lanDiscoveryClient->Startup(1, &socketDescriptor, 1);
    lanDiscoveryTimeBeforeBroadcast = 0.f;
}

void StopLanDiscovery()
{
    if (lanDiscoveryClient)
		RakNet::RakPeerInterface::DestroyInstance(lanDiscoveryClient);
    lanDiscoveryClient = NULL;
}

bool IsLanDiscoveryInProgress()
{
    return (lanDiscoveryClient != NULL);
}

std::map<SystemAddress, int> GAvailableLANServers;

void TickLanDiscovery( float aTimeEllapsed )
{
    if (!lanDiscoveryClient)
        return;

    RakNet::Packet* p = lanDiscoveryClient->Receive();
    
    while (p)
    {
        int msgid = p->data[0];
        LOG(" Lan Discovery: Received Message # %d\n", msgid);

        switch (msgid)
        {
        case ID_UNCONNECTED_PONG:
            {
                RakNet::BitStream bsIn(p->data, p->length, false);
                bsIn.IgnoreBytes(1);
                RakNet::TimeMS time;
                bsIn.Read(time);
                //printf("Got pong from %s with time %i\n", p->systemAddress.ToString(), RakNet::GetTimeMS() - time);

                u32 dataLength = p->length - sizeof(u8) - sizeof(RakNet::TimeMS);
                u8* pData = p->data + sizeof(u8) + sizeof(RakNet::TimeMS);
                ASSERT_GAME( dataLength == sizeof(u8) + sizeof(server_advert_data_t) );

                BitStream bsServerData(pData, dataLength, false);
                bsServerData.IgnoreBytes(1);
                server_advert_data_t serverInfo;
                bsServerData.Read( serverInfo.mTrack );
                bsServerData.Read( serverInfo.mMode );
                bsServerData.Read( serverInfo.mNbLaps );
                bsServerData.Read( serverInfo.mNbPlayers );
                bsServerData.Read( serverInfo.mMaxNbPlayers );

                //GAvailableLANServers[p->systemAddress.ToString()] = 3.f;
                std::map<SystemAddress, int>::iterator iter = GAvailableLANServers.find(p->systemAddress);
                int idx;
                if ( iter == GAvailableLANServers.end() )
                {
                    // new one
                    GServers.push_back( server_t() );
                    idx = GServers.size()-1;
                    GAvailableLANServers.insert( std::make_pair( p->systemAddress, idx) );
                }
                else
                {
                    idx = (*iter).second;
                }
            
                server_t& server = GServers[idx];
                server.mPing = static_cast<u16>( RakNet::GetTimeMS() - time );
                server.mbIsLan = true;
                server.mTimeOut = 3.f;
                server.mURL = p->systemAddress.ToString();

                server.mTrack = serverInfo.mTrack;
                server.mMode = serverInfo.mMode;
                server.mNbLaps = serverInfo.mNbLaps;
                server.mNbPlayers = serverInfo.mNbPlayers;
                server.mMaxNbPlayers = serverInfo.mMaxNbPlayers;
            }
            break;
        }
        lanDiscoveryClient->DeallocatePacket(p);
        p = lanDiscoveryClient->Receive();
    }
    
    lanDiscoveryTimeBeforeBroadcast -= aTimeEllapsed;
    if ( lanDiscoveryTimeBeforeBroadcast <= 0.f )
    {
        lanDiscoveryClient->Ping("255.255.255.255", SERVER_PORT, true);
        lanDiscoveryTimeBeforeBroadcast = 1.f;
    }

    // tick previously found servers
    std::map<SystemAddress, int>::iterator iter = GAvailableLANServers.begin();
    std::map<SystemAddress, int>::iterator iter2 = GAvailableLANServers.begin();    
    for (; iter != GAvailableLANServers.end();)
    {
        iter2 = iter;
        ++iter2;
        
        int idx = (*iter).second; 
        bool bSuppress = false;
        
        GServers[idx].mTimeOut -= aTimeEllapsed;
        if ( GServers[idx].mTimeOut <= 0.f )
            bSuppress = true;
        
        if ( bSuppress )
        {
            GAvailableLANServers.erase( iter ); // FIXME
            GServers.erase( GServers.begin()+idx );
        }
        iter = iter2;
    }
                                                                                     
                                                                                     
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//

class GameObjectNet 
{
public:
    void Init( u8 type, void *pObject)
    {
        mUID = ++ReplicaUID;
        mbReliable = false;
        mLocalTime = 0.f;
        mFreq = 1.f/10.f;

        mType = type;
        mObject = pObject;
    }

    void ConstructionSerialization(RakNet::BitStream & stream );
    void ConstructionDeserialization(RakNet::BitStream & stream );
    bool SerializeIt( RakNet::BitStream & stream );
    void DeserializeIt( RakNet::BitStream & stream );

    //GameObject *mpGameObject;
    static u32 ReplicaUID;
    u32 mUID;
    bool mbReliable;
    float mLocalTime, mFreq;
    u8 mType;
    void *mObject;
};


class Network : public NetworkInterface
{
public:
    Network( bool isServer );
    virtual ~Network();

    void ConnectTo(const std::string& urlToConnectTo);
    void CreateServer();
    void Close();

    void Tick(float aTimeEllapsed);

    bool Active() const { return ( peer != NULL ); }

    //const std::vector<server_t> & GetActiveServers() const { return GServers; }

    // players infos
    void SetPlayerInfos( const char *szName, const shipIdentifier_t &aShipID ) { mPlayerName = szName; mShipID = aShipID; }
    int GetConnectedPlayers() const { return mNetPlayer.size(); }
    const playerNetInfos_t* GetPlayerInfos( int idx ) const 
    { 
        ASSERT_GAME( idx < static_cast<int>(mNetPlayer.size()) ); 

        std::map<RakNet::SystemAddress, playerNetInfos_t >::const_iterator iter = mNetPlayer.begin();
        for (int i=0;i<idx;i++)
            ++iter;
        return &(*iter).second; 
    }

    // objects management
    virtual GameObjectNet* Spawn( u8 objectType, void *pGameObject );
    virtual void Destroy( GameObjectNet *pObj );
    
    // wait
    void postWaitForBuddies() 
    { 
        mbNeedWaitForBuddies = true; 
    }

    // sync
    void informServerImReadyToPlay( ) 
    { 
		//MessageBox(NULL,"I'm ready", "ready", MB_OK);
        ASSERT_GAME( !mbIsServer );
        if (!peer)
            return;
        informServerImReadyToPlay( false, UNASSIGNED_SYSTEM_ADDRESS ); 
    }
    void informServerImReadyToPlay( bool bBroadcast, const SystemAddress& from );

    void SetPlayersNotReady();

    virtual u8 ClientID()
    {
        ASSERT_GAME( !mbIsServer );
        return mNetIndex;
    }
    void* GetObjectByUID( u32 uid )
    {
        ASSERT_GAME( mbIsServer );
        // local server not started->return null and client will create its local instance
        if (!bServerStarted)
            return NULL;
        return FindObjectByUID( uid )->mObject;
    }

	virtual void BroadcastStartRace();
private:
    // replica
    ArrayPool<GameObjectNet, 1024> mNetGameObjectsPool;
    std::map<u32, GameObjectNet*> mReplicas;

    GameObjectNet* FindObjectByUID( u32 uid )
    {
        std::map<u32, GameObjectNet*>::iterator iter = mReplicas.find( uid );
        if ( iter == mReplicas.end() ) 
            return NULL;
        return (*iter).second;
    }

    void ReplicaCheck( float aTimeEllapsed )
    {
        std::map<u32, GameObjectNet*>::iterator iter = mReplicas.begin();
        for (; iter != mReplicas.end();iter++)
        {
            GameObjectNet* pObj = (GameObjectNet*)(*iter).second;
            pObj->mLocalTime += aTimeEllapsed;
            if ( pObj->mLocalTime >= pObj->mFreq )
            {
                pObj->mLocalTime -= pObj->mFreq;
                RakNet::BitStream stream;
                u8 msg = ZNMSG_REPLICA_MODIFY;
                stream.Write( msg );
                stream.Write( pObj->mUID );
                if ( pObj->SerializeIt( stream ) )
                {
                    peer->Send( &stream, HIGH_PRIORITY, pObj->mbReliable?RELIABLE:UNRELIABLE_SEQUENCED/*RELIABLE*/, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                }
            }
        }
    }

    void ReplicateEveryObjectsFor ( const SystemAddress &addr )
    {
        std::map<u32, GameObjectNet*>::iterator iter = mReplicas.begin();
        for (; iter != mReplicas.end();iter++)
        {
            GameObjectNet* pObj = (GameObjectNet*)(*iter).second;
            RakNet::BitStream stream;
            u8 msg = ZNMSG_REPLICA_CREATE;
            stream.Write( msg );
            pObj->ConstructionSerialization( stream );
            peer->Send( &stream, HIGH_PRIORITY, RELIABLE, 0, addr, false );
        }
    }
    // replica



    bool mbNeedWaitForBuddies;
    
    void InitReplicationSystem();

    void UpdateAdvertisedServerInfo();

    void BroadcastNickList();

    float mTimeBeforeResendControls;

    RakNet::RakPeerInterface *peer;
    RakNet::TimeMS nextSendTime;
    //bool isConnected;
    bool bServerStarted;
    unsigned int ConnectionCount();
    std::map<RakNet::SystemAddress, playerNetInfos_t > mNetPlayer;
    SystemAddress mySystemAddress;


    // ReplicaManager3 requires NetworkIDManager to lookup pointers from numbers.
    RakNet::NetworkIDManager *GNetworkIdManager;
    // The system that performs most of our functionality for this demo
    //ReplicaManager3rush *GReplicaManager;
    /*friend class GameNet;
    
    GameNet *mGame;
    */
    bool mbIsServer;
	bool mBroadcastStartupDone;
    // player infos
    shipIdentifier_t mShipID;
    std::string mPlayerName;
    u8 mNetIndex;
};

//extern Network GNetwork;
Network GNetworkClient( false );
Network GNetworkServer( true );

NetworkInterface *NetClient = &GNetworkClient;
NetworkInterface *NetServer = &GNetworkServer;


static const unsigned int NUM_CLIENTS=8;

typedef struct GameSerialisationFlags_t
{
    u8 SerGameProps : 1;
    u8 SerBonusInvalidated : 1;
    u8 SerBonusValidated : 1;
	u8 SerEndString : 1;
} GameSerialisationFlags_t;

void GameObjectNet::ConstructionSerialization(RakNet::BitStream & stream )
{
    stream.Write( mType );
    stream.Write( mUID );
    switch (mType)
    {
    case GO_Game:// nothing
        break;
    case GO_Ship:
        {
            GameShip *ps = (GameShip*)mObject;
            stream.Write( ps->GetMatrix() );
            stream.Write( ps->mProps );
            stream.Write( ps->GetShipIndentifier() );  
            //LOG(" Ser %d\n", ps->GetShipIndentifier().mShipColor );
        }
        break;
    case GO_Mine:
        {
        GameMine *gm = (GameMine*)mObject;
        stream.Write( gm->GetMatrix() );
        }
        break;
    case GO_Missile:
        {
        GameMissile *gm = (GameMissile*)mObject;
        stream.Write( gm->GetMatrix() );
        }
        break;
    case GO_Explosion:
        {
        GameExplosion *ge = (GameExplosion*)mObject;
        stream.Write( ge->GetMatrix() );
        }
        break;
    default:
        //ASSERT(0);
        break;
    }
}

void GameObjectNet::ConstructionDeserialization(RakNet::BitStream & stream )
{
    
    switch (mType)
    {
    case GO_Game:// nothing
        break;
    case GO_Ship:
        {
            LOG("New ship!\n");
            GameShip *ps = (GameShip*)mObject;
            matrix_t mat;
            shipIdentifier_t shipId;
            GameShip::shipProps_t shpProps;
            stream.Read( mat );
            stream.Read( shpProps );
            stream.Read( shipId );        

//            LOG(" Deser %d\n", shipId.mShipColor );

            ps->NetworkConstruct( mat, shipId );

            ps->mProps = shpProps;

            // for local player?
            const int ShipNetIndex = ps->GetOwnerNetIndex();
            ASSERT_GAME( Game::IsValidObjectNetIndex(ShipNetIndex) );

            const int ClientNetIndex = NetClient->ClientID();
            ASSERT_GAME( Game::IsValidObjectNetIndex(ClientNetIndex) );

            if ( ShipNetIndex == ClientNetIndex )
            {
                camera.SetCameraBehindShip( ps );
                ps->mProps.ctrlEnabled = false;
                menuToShow = MENU_EMPTY_NETWORK;
                LOG("MENU_EMPTY_NETWORK\n");
				GGame->SetPlayerShip( ps );
            }
        }
        break;
    case GO_Mine:
        {
            GameMine *gm = (GameMine*)mObject;
            matrix_t mat;
            stream.Read( mat );
            gm->SetMatrix( mat );
        }
        break;
    case GO_Missile:
        {
            GameMissile *gm = (GameMissile*)mObject;
            matrix_t mat;
            stream.Read( mat );
            gm->SetMatrix( mat );
        }
        break;
    case GO_Explosion:
        {
            GameExplosion *ge = (GameExplosion*)mObject;
            matrix_t mat;
            stream.Read( mat );
            ge->SetMatrix( mat );
        }
        break;
    default:
        //ASSERT(0);
        break;
    }
}

bool GameObjectNet::SerializeIt( RakNet::BitStream & stream )
{
    switch (mType)
    {
    case GO_Game:// nothing
        {
            GameSerialisationFlags_t gsf;
            gsf.SerBonusInvalidated = (!GGame->GetInvalidatedBonuses().empty() );
            gsf.SerBonusValidated = (!GGame->GetValidatedBonuses().empty() );
			gsf.SerGameProps = GGame->GetGameProps().mbDirtyForNetwork;//&&GGame->GetGameProps().mbDirtyForWorld;
			gsf.SerEndString = GGame->EndStringDirty();
            stream.Write( gsf );
            if ( gsf.SerGameProps )
            {
				ASSERT_GAME(NetServer);
				((Network*)NetServer)->SetPlayersNotReady();
                LOG("Server asked for serialisation!\n");
                stream.Write( GGame->GetGameProps() );

			}
			if ( gsf.SerEndString)
			{
                u8 nbStrs = static_cast<u8>( GGame->GetEndResultsNbStrings() );
                stream.Write( nbStrs );

                for ( int i = 0 ; i < nbStrs ; i++ )
                    stream.Write( RakString( GGame->GetEndResultString( i ).c_str() ) );

			}
			

			
            const std::vector<u16>* bnLists[] = { &GGame->GetInvalidatedBonuses(), &GGame->GetValidatedBonuses() };
            for (int j = 0;j<2;j++)
            {
                if ( !bnLists[j]->empty() )
                {

                    u8 nbBonus = static_cast<u8>( bnLists[j]->size() );
                    stream.Write( nbBonus );
                    for ( unsigned int i = 0 ; i < nbBonus ; i++ )
                        stream.Write( (*bnLists[j])[i] );
                }
            }

            
            GGame->ClearValidatedAndInvalidatedBonusesList();

			if ( gsf.SerGameProps||gsf.SerEndString||gsf.SerBonusInvalidated||gsf.SerBonusValidated )
			{
                GGame->ClearFlagPropsDirtyNetwork();
                return true;
            }
        }
        break;
    case GO_Ship:
        {
            GameShip *ps = (GameShip*)mObject;
            matrix_t mat;
            stream.Write( ps->GetCurrentControls() );

            ps->mProps.physicEnabled = ps->GetPhysics()->PhysicEnabled();
            stream.Write( ps->mProps );
            //stream.Write( ps->GetPhysics()->GetTransform() );

            ServerShipSync_t syncValues = ps->GetPhysics()->BuildResyncValues();
            stream.Write( syncValues );

			return true;
        }
        break;
    case GO_Missile:
        break;
    default:
        //ASSERT(0);
        break;
    }
    return false;
}

void GameObjectNet::DeserializeIt( RakNet::BitStream & stream )
{
    switch (mType)
    {
    case GO_Game:// nothing
        if (!GGame->IsAuth())
        {
			GameSerialisationFlags_t gsf;
			stream.Read( gsf );

			gameProperties_t gp;

			if ( gsf.SerGameProps )
			{
				stream.Read( gp );
			}

			if ( gsf.SerEndString)
			{
				u8 nbStrs;
				stream.Read( nbStrs );

				// TODO: only for client
				GGame->SetEndResultsNbStrings( nbStrs );
				for (int i = 0 ; i < 8 ; i++)
				{
					RakString rakStr;
					stream.Read( rakStr );
					GGame->SetEndResultString( i, rakStr.C_String() );
				}

			}
			if ( gsf.SerGameProps )
			{
				ggui.clearBitmaps();
				camera.SetCameraCustom();
				GGame->Restart();
				Menus::Show(MENU_EMPTY_NETWORK);
				GGame->SetNumberOfLapsToDo( gp.mNumberOfLapsToDo );
				GGame->SetSplit( false );
				GGame->SetTrack( gp.mTrackIndex );
				GGame->SetType( gp.mType );
			}
		
            
            bool changeBonusOps[] = { gsf.SerBonusInvalidated, gsf.SerBonusValidated };
            for ( int j = 0;j<2;j++)
            {
                if ( changeBonusOps[j] )
                {
                    std::vector<trackBonusInstance_t>& BonusList = trackBonusInstance_t::GetTrackBonuses();

                    u8 nbBonus;
                    stream.Read( nbBonus );
                    for ( unsigned int i = 0 ; i < nbBonus ; i++ )
                    {
                        u16 bnToChange;
                        stream.Read( bnToChange );
						if (bnToChange<BonusList.size())
						{
							if (j)
								BonusList[ bnToChange ].GetActive();
							else
								BonusList[ bnToChange ].GetInactive();
						}
                    }
                }
            }
        }
        // chargement intervient ici. on a deja envoyé le message et on init les players a "pas ready" après
        GNetworkClient.postWaitForBuddies();
        break;
    case GO_Ship:
        {
            if ( !GGame->IsAuth() )
            {
                GameShip *ps = (GameShip*)mObject;
            
                ServerShipSync_t syncValues;
                shipControls_t controls;


				stream.Read( controls );
                stream.Read( ps->mProps );
                ps->GetPhysics()->EnablePhysics( ps->mProps.physicEnabled );

                stream.Read( syncValues );

                ps->GetPhysics()->SetServerResync( syncValues );
				if ( ps != GGame->GetPlayerShip() )
	                ps->SetCurrentControls( controls );
            }
        }
        break;
    default:
        //ASSERT(0);
        break;
    }
    
}


u32 GameObjectNet::ReplicaUID = 0;


std::string server_t::Format() const
{
    std::string res;

    char tmps[512];

    res = mbIsLan?"LAN":"NET";
    res += "   ";
    if (mMode != 0xFF)
    {

        sprintf(tmps, "[%s]%s                     ", GameTypes[mMode].szName, Tracks[mTrack].mName.c_str() );
        res += std::string(tmps).substr(0, 23);
    }
    else
    {
        res += std::string("Lobby                           ").substr(0,23);
    }
    sprintf(tmps, "%3d", mPing);
    res += tmps;
    sprintf(tmps, "     %d/%d", mNbPlayers, mMaxNbPlayers);
    res += tmps;
    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int Network::ConnectionCount()
{
    unsigned int i,count;
    for (i=0,count=0; i < NUM_CLIENTS;i++)
        if (peer->GetSystemAddressFromIndex(i)!=RakNet::UNASSIGNED_SYSTEM_ADDRESS)
            count++;
    return count;
}

Network::Network( bool isServer )
{
    mbNeedWaitForBuddies = false;
    bServerStarted = false;
    peer = NULL;
    GNetworkIdManager = NULL;

    mTimeBeforeResendControls = 0.f;

    mbIsServer = isServer;
	mBroadcastStartupDone = false;
}

Network::~Network()
{
    LOG("UninitNetwork \n");

    Close();
    mNetGameObjectsPool.ForceClear();
    LOG("Done\n");
}

void Network::InitReplicationSystem()
{
    GNetworkIdManager = new RakNet::NetworkIDManager;
    LOG("\nMy GUID is %s\n", peer->GetMyGUID().ToString());
}

void Network::ConnectTo(const std::string& urlToConnectTo)
{
    Close();

    peer = RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor socketDescriptor;
    socketDescriptor.port=0;
    nextSendTime=0;

    const RakNet::StartupResult startResult = peer->Startup(1,&socketDescriptor,1);
    RakAssert( startResult == RakNet::RAKNET_STARTED );
    UNUSED_PARAMETER(startResult);  //HACK: needed when RakAssert() is void, to prevent warning 4189 : local variable is initialized but not referenced

    //isConnected=false;

    InitReplicationSystem();

    if ( peer->Connect(urlToConnectTo.c_str(), (unsigned short) SERVER_PORT, 0, 0, 0) != RakNet::CONNECTION_ATTEMPT_STARTED )
    {
        LOG("Client connect call failed!\n");
    }

    // le Game reste authoritative si on a deja 1 server de lancé
    if ( !GNetworkServer.bServerStarted )
        GGame->DisableAuthoritative();
}

void Network::CreateServer()
{
    Close();

    peer = RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor socketDescriptor;
    socketDescriptor.socketFamily=AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
    socketDescriptor.port=(unsigned short) SERVER_PORT;

    const unsigned short maxConnections = static_cast<unsigned short>(NUM_CLIENTS);
    const RakNet::StartupResult startResult = peer->Startup(maxConnections,&socketDescriptor,1);
    RakAssert( startResult == RakNet::RAKNET_STARTED );
    UNUSED_PARAMETER(startResult);  //HACK: needed when RakAssert() is void, to prevent warning 4189 : local variable is initialized but not referenced

    peer->SetMaximumIncomingConnections(NUM_CLIENTS);

    InitReplicationSystem();

    mySystemAddress = peer->GetSystemAddressFromIndex(0);

    
    bServerStarted = true;

    Spawn( GO_Game, GGame );
}

void Network::Close()
{
    if (peer)
    {
        peer->Shutdown(100,0);
        RakNet::RakPeerInterface::DestroyInstance(peer);
    }

    if (GNetworkIdManager)
    {
        delete GNetworkIdManager;
    }

    GNetworkIdManager = NULL;
    peer = NULL;
    mNetPlayer.clear();
    mTimeBeforeResendControls = 0.f;
    bServerStarted = false;

    if ( GGame )
        GGame->EnableAuthoritative();

	mReplicas.clear();
}

void Network::informServerImReadyToPlay( bool bBroadcast, const SystemAddress& from )
{
    BitStream bitStream;
    unsigned char msgid = ZNMSG_USERREADY;
    bitStream.Write(msgid);
    if ( bBroadcast )
    {
        // sent by server
        bitStream.Write( from );
        peer->Send(&bitStream, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
    }
    else
    {
        // to the server
        bitStream.Write( UNASSIGNED_SYSTEM_ADDRESS );
        peer->Send(&bitStream, HIGH_PRIORITY, RELIABLE, 0, peer->GetSystemAddressFromIndex(0), false);
    }
    
}

void Network::UpdateAdvertisedServerInfo()
{
    const u16 track = GAdvertisedServerInfo.mTrack;
    const u8 mode = GAdvertisedServerInfo.mMode;
    const u8 nbLaps = GAdvertisedServerInfo.mNbLaps;
    const u8 nbPlayers = GAdvertisedServerInfo.mNbPlayers;
    const u8 maxNbPlayers = GAdvertisedServerInfo.mMaxNbPlayers;

    LOG(" Sending advertised server info: track (%d), mode(%d), laps(%d), players(%d/%d)\n", track, mode, nbLaps, nbPlayers, maxNbPlayers);

    BitStream bitStream;
    bitStream.Write( track );
    bitStream.Write( mode );
    bitStream.Write( nbLaps );
    bitStream.Write( nbPlayers );
    bitStream.Write( maxNbPlayers );

    const char* pData = reinterpret_cast<const char*>(bitStream.GetData());
    const u32 dataLengthInBits = bitStream.GetNumberOfBitsUsed();
    ASSERT_GAME( dataLengthInBits % 8 == 0);
    const u32 dataLength = dataLengthInBits / 8;
    ASSERT_GAME( dataLength == sizeof(u8) + sizeof(server_advert_data_t) );

    peer->SetOfflinePingResponse(pData, dataLength);
}

void Network::BroadcastNickList()
{
    // broadcast list
    BitStream bitStream;
    unsigned char msgid = ZNMSG_NICKNAMESLIST;
    bitStream.Write(msgid);
    u8 nbPlayers = static_cast<u8>( mNetPlayer.size() );
    LOG(" Sending %d nicks : \n", static_cast<int>( mNetPlayer.size() ) );
    bitStream.Write( nbPlayers );
    std::map<RakNet::SystemAddress, playerNetInfos_t >::const_iterator iter = mNetPlayer.begin();
    for ( ; iter != mNetPlayer.end() ; ++ iter)
    {
        bitStream.Write( (*iter).first );
        bitStream.Write(RakString( (*iter).second.mNickName.c_str() ));
        bitStream.Write( (*iter).second.mShipID );
        
        LOG("   - %s\n", (*iter).second.mNickName.c_str() );
    }
    peer->Send(&bitStream, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
}


GameObjectNet* Network::Spawn( u8 objectType, void *pGameObject )
{
    if ( (!mbIsServer) || GBInsideClientNetTick )
        return NULL;
    if (! bServerStarted )
        return NULL;
    RakNet::BitStream stream;
    u8 msg = ZNMSG_REPLICA_CREATE;
    stream.Write( msg );

    GameObjectNet *nObject = mNetGameObjectsPool.New();
    nObject->Init( objectType, pGameObject );
    nObject->ConstructionSerialization( stream );
    mReplicas.insert( std::make_pair( nObject->mUID, nObject ) );
    peer->Send( &stream, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true );
    return nObject;
}

void Network::Destroy( GameObjectNet* pObj )
{
    if ( (!pObj) || (!bServerStarted) )
        return;

    RakNet::BitStream stream;
    u8 msg = ZNMSG_REPLICA_DELETE;
    stream.Write( msg );
    stream.Write( pObj->mUID );
    peer->Send( &stream, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true );

    mReplicas.erase( pObj->mUID );
    mNetGameObjectsPool.Delete( pObj );
}

void Network::SetPlayersNotReady()
{
    std::map<RakNet::SystemAddress, playerNetInfos_t >::iterator iter = mNetPlayer.begin();
    for (; iter != mNetPlayer.end() ; ++iter)
    {
        (*iter).second.mbIsReady = false;
    }
	mBroadcastStartupDone = false;
}
void Network::BroadcastStartRace()
{
	if (!mbIsServer)
		return;
	if (!bServerStarted)
		return;
	if (mBroadcastStartupDone)
		return;

	mBroadcastStartupDone = true;
	ASSERT_GAME(mbIsServer);

	BitStream bitStream;
	unsigned char msgname = ZNMSG_STARTRACE;
	bitStream.Write(msgname);
	peer->Send(&bitStream, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	// set back not ready flag
	SetPlayersNotReady();
	//

#if IS_OS_WINDOWS
	GGame->LoadPhysicExtension();
#endif

	int idx = 0;
	std::map<RakNet::SystemAddress, playerNetInfos_t >::iterator iter = mNetPlayer.begin();
	for ( ; iter != mNetPlayer.end() ; ++iter, idx++)
	{
		playerNetInfos_t & playerNfos = (*iter).second;
		matrix_t mt;
		mt = track.GetSpawnMatrix( idx);
		mt.position += mt.up * 1.5f;//ShipParameters.mEquilibriumHeight;//... 1.f;
		mt.position.w = 1.f;

		//                            LOG(" Server spawns %d\n", playerNfos.mShipID.mShipColor );
		GameShip *ps = (GameShip *)GGame->SpawnShip( mt, playerNfos.mShipID, playerNfos.mNetIndex );
		playerNfos.mShip = ps;
		ps->SetName( playerNfos.mNickName.c_str() );

		/*
		ps->mProps.mNetIndex = (*iter).second.mNetIndex;
		ps->SetShipIdentifier(  );
		*/
	}
}

void Network::Tick(float aTimeEllapsed)
{
    if (!peer)
        return;

    if (!mbIsServer)
        GBInsideClientNetTick = true;

    menuToShow = MENU_NONE;

    Packet *p = peer->Receive();
    while (p)
    {
        int msgid = p->data[0];
        LOG(" Received Message # %d\n", msgid);
        switch (msgid)
        {
            /*
        case ID_CONNECTION_REQUEST:
            // connection attempt on server
            if ( mbIsServer )
            {
                peer->CancelConnectionAttempt(p->systemAddress ) ;
            }
            break;
            */
        case ID_CONNECTION_ATTEMPT_FAILED:
            LOG("ID_CONNECTION_ATTEMPT_FAILED\n");
            menuToShow = MENU_LOBBY_JOINERROR;
            break;
        case ID_NO_FREE_INCOMING_CONNECTIONS:
            LOG("ID_NO_FREE_INCOMING_CONNECTIONS\n");
            break;
        case ID_CONNECTION_REQUEST_ACCEPTED:
            LOG("ID_CONNECTION_REQUEST_ACCEPTED\n");
            //
            {
                //peer->CloseConnection( p->systemAddress, true );

#if IS_OS_WINDOWS
					GGame->LoadPhysicExtension();
#endif
                // user send its login
                BitStream bitStream;
                unsigned char msgname = ZNMSG_NICKNAME;
                bitStream.Write(msgname);
                RakString nickStr = mPlayerName.c_str();
                LOG("Sent nick %s\n",mPlayerName.c_str());
                bitStream.WriteCompressed(nickStr);
                bitStream.Write( mShipID );

                peer->Send(&bitStream, HIGH_PRIORITY, RELIABLE, 0, peer->GetSystemAddressFromIndex(0),false/*UNASSIGNED_SYSTEM_ADDRESS, true*/);
                if ( !NetServer->Active() )
                    menuToShow = MENU_CONNECTED_WAITING;
            }
            break;
        case ID_NEW_INCOMING_CONNECTION:
            LOG("ID_NEW_INCOMING_CONNECTION from %s\n", p->systemAddress.ToString());
            LOG("Connections = %i\n", ConnectionCount());
            if (mbIsServer)
            {
                ReplicateEveryObjectsFor ( p->systemAddress );
            }
            break;
        case ID_DISCONNECTION_NOTIFICATION:
            {
            LOG("ID_DISCONNECTION_NOTIFICATION\n");
            LOG("Connections = %i\n", ConnectionCount());

            playerNetInfos_t &curPlayerInfos = mNetPlayer[p->systemAddress];
            u8 playerNetIndex = curPlayerInfos.mNetIndex;
            mNetPlayer.erase( p->systemAddress);

            if ( mbIsServer )
            {
                BroadcastNickList();
                // Got ship?
               
                GameShip *pShip = GGame->GetShipByNetIndex( playerNetIndex );
                GGame->Destroy( pShip );
            }

            if (!mbIsServer)
            {
                menuToShow = MENU_CONNECTION_LOST;
            }
            }
            break;
        case ID_CONNECTION_LOST:
            LOG("ID_CONNECTION_LOST\n");
            LOG("Connections = %i\n", ConnectionCount());
            mNetPlayer.erase( p->systemAddress);

            if ( mbIsServer )
                BroadcastNickList();

            if (!mbIsServer)
            {
                menuToShow = MENU_CONNECTION_LOST;
            }

            break;

            // custom messages
            /*
            ZNMSG_NICKNAME
            Server receievs a nick name, update its list and broadcast whole list to other players
            */
        case ZNMSG_STARTRACE:
            {
                SetPlayersNotReady();
                world.SetGameSpeed( 1.f );
                setDOFBlur( 0.f );
                GGame->SetState( GameStateCountDown );
            }
            break;
        case ZNMSG_USERREADY:
            {
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                SystemAddress sysaddr;
                zrakpack.Read( sysaddr );
                if ( mbIsServer )
                {
                    informServerImReadyToPlay( true, p->systemAddress );
                    sysaddr = p->systemAddress;
                }

                mNetPlayer[sysaddr].mbIsReady = true;
                

                // the server counts ready guys, if every body is ok, start the race
                if ( mbIsServer )
                {
                    bool everybodyok = true;
                    std::map<RakNet::SystemAddress, playerNetInfos_t >::iterator iter = mNetPlayer.begin();
                    for ( ; iter != mNetPlayer.end() ; ++iter)
                    {
                        if ( !(*iter).second.mbIsReady )
                        {
                            everybodyok = false;
                            break;
                        }
                    }
                    if ( everybodyok )
                    {
						BroadcastStartRace();
                    }
                    
                }
            }
            break;
        case ZNMSG_NETINDEX:
            {
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                zrakpack.Read( mNetIndex );
            }
            break;
        case ZNMSG_NICKNAME:
            {
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                RakString nickStr;
                zrakpack.ReadCompressed(nickStr);
                shipIdentifier_t shipID;
                zrakpack.Read( shipID );
                const char *szNickn =  nickStr.C_String();
                playerNetInfos_t &curPlayerInfos = mNetPlayer[p->systemAddress];
                curPlayerInfos.mNickName = szNickn;
                curPlayerInfos.mbIsReady = false;
                curPlayerInfos.mShipID = shipID;
                LOG(" Received nick %s\n", szNickn );
                
                // nick accepted, send netindex
                
                BitStream bitStream;
                unsigned char msgname = ZNMSG_NETINDEX;
                bitStream.Write(msgname);
                //FIXME: probably should refactor this NetIndex management, check for failure/overflow?
                static u8 localGNetIndex = 0;
                ++localGNetIndex%=15;
                u8 resLocalGNetIndex = localGNetIndex+1;
                bitStream.Write( resLocalGNetIndex );
                peer->Send(&bitStream, HIGH_PRIORITY, RELIABLE, 0, p->systemAddress, false );
                curPlayerInfos.mNetIndex = resLocalGNetIndex;
                
                // broadcast new nick list
                BroadcastNickList();
            }
            break;
        case ZNMSG_NICKNAMESLIST:
            {
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                u8 nbNicks;
                zrakpack.Read(nbNicks);
                mNetPlayer.clear();
                for (int i=0;i<nbNicks;i++)
                {
                    SystemAddress addr;
                    zrakpack.Read(addr);

                    RakString nickStr;
                    zrakpack.Read(nickStr);
                    shipIdentifier_t shipID;
                    zrakpack.Read(shipID);
                    const char *szNickn =  nickStr.C_String();
                    mNetPlayer[addr].mNickName = nickStr.C_String();
                    mNetPlayer[addr].mbIsReady = false;
                    mNetPlayer[addr].mShipID = shipID;
                    LOG(" Received nick List %d - %s (%s)\n", i, szNickn, addr.ToString() );
                }
            }
            break;
        case ZNMSG_SHIPCONTROLS:
            {
                // from client to server.
                // server to other clients is replicated thru GameShip ship replication
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                shipControls_t shpCtrl;
                zrakpack.Read(shpCtrl);
                 playerTickInfos_t playerTickInfo;
                 zrakpack.Read(playerTickInfo);
                std::map<RakNet::SystemAddress, playerNetInfos_t >::iterator iter = mNetPlayer.find( p->systemAddress );
                if ( iter != mNetPlayer.end() )
                {
                    GameShip * ps = (*iter).second.mShip;
                    if ( ps && GGame->ShipExists(ps) )
                    {
                        ps->SetCurrentControls( shpCtrl );
                        ps->GetPhysics()->SetServerResync(ServerShipSync_t( playerTickInfo.position, playerTickInfo.orientation ) );
                    }
                }
                else
                {
                    ASSERT_GAME( 0 );
                }
            }
            break;
        // REPLICATION$
        case ZNMSG_REPLICA_CREATE:
            {
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                u8 objType;
                u32 uid;
                zrakpack.Read( objType );
                zrakpack.Read( uid );
                GameObjectNet *nObject = mNetGameObjectsPool.New();
                if ( objType == GO_Game )
                    nObject->Init( objType, GGame );
                else
                {
                    // if we also have a server in the procress, we have to find back 
                    void *objectPresentInLocalServer = NetServer->GetObjectByUID( uid ); 
                    if (!objectPresentInLocalServer)
                        nObject->Init( objType, GGame->Spawn( objType, matrix_t::Identity ) );
                    else
                        nObject->Init( objType, objectPresentInLocalServer );
                }
                nObject->mUID = uid;
                nObject->ConstructionDeserialization( zrakpack );
                
                mReplicas.insert( std::make_pair( nObject->mUID, nObject ) );
            }
            break;
        case ZNMSG_REPLICA_MODIFY:
            {
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                u32 objUID;
                zrakpack.Read( objUID );
                GameObjectNet *obj = FindObjectByUID( objUID );
                ASSERT_GAME( obj );
                obj->DeserializeIt( zrakpack );
            }
            break;
        case ZNMSG_REPLICA_DELETE:
            {
                RakNet::BitStream zrakpack(p->data+1, p->bitSize, false);
                u32 objUID;
                zrakpack.Read( objUID );
                GameObjectNet *obj = FindObjectByUID( objUID );
                ASSERT_GAME( obj );
                // if we have a local server, the object is already deleted, no need to destroy it twice
                if ( !NetServer->Active() )
                    GGame->Destroy( (GameObject*)obj->mObject );
                mNetGameObjectsPool.Delete( obj );
                mReplicas.erase( objUID );
            }
            break;

            /*
            case ID_USER_PACKET_ENUM:
            {
            if (memcmp(p->data, randomData, RANDOM_DATA_SIZE)!=0)
            {
            printf("Bad data on server!\n");
            }
            break;
            }
            */
        }
        peer->DeallocatePacket(p);
        p = peer->Receive();
   }

   if ( mbNeedWaitForBuddies )
   {
       // assume all players aren't ready. mbIsReady is set to false when we receive nick list of just after a race has started
        mbNeedWaitForBuddies = false;
        if ( GGame->GetState() == GameStateWaiting )
        {
            menuToShow = MENU_WAITING_BUDDIES;
            LOG("MENU_WAITING_BUDDIES\n");
        }
    }

    if (menuToShow != MENU_NONE)
    {
        ASSERT_GAME( !mbIsServer );
        Menus::Show( menuToShow );
    }
    
    // send controls to server
    mTimeBeforeResendControls -= aTimeEllapsed; 
    if ( mTimeBeforeResendControls <= 0.f)
    {
        mTimeBeforeResendControls = 0.1f; //10 send per second

        if (GGame && GGame->GetPlayerShip() )
        {
            if (GGame->IsAuth())
            {
                // we are the machine acting as server
                //mNetPlayer[mySystemAddress].mShip = GGame->GetPlayerShip();
            }
            else
            {
                const IPhysicShip* phys = GGame->GetPlayerShip()->GetPhysics();
                // send controls to server
                BitStream bitStream;
                unsigned char msgid = ZNMSG_SHIPCONTROLS;
                bitStream.Write( msgid );
                bitStream.Write( GGame->GetPlayerShip()->GetCurrentControls() );
                
                

                playerTickInfos_t playerTickInfo;
                playerTickInfo.position = phys->GetCurrentPosition();
                playerTickInfo.orientation = phys->GetCurrentOrientation();

                bitStream.Write( playerTickInfo );


                peer->Send( &bitStream, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, peer->GetSystemAddressFromIndex(0), false );
            }
        }
    }
    
    if ( mbIsServer )
    {
        ReplicaCheck( aTimeEllapsed );

        if ( GBAdvertisedServerChanged )
        {
            UpdateAdvertisedServerInfo();
        }
    }
    GBInsideClientNetTick = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// called by authoritative 

// CrÃ©ation:
// server crÃ©e -> game::spawn -> si gameisauth-> si netserver connected -> netserver:spawn(retourne le gameobjectnet) 
// client recoit (AllocReplica) -> si GGame is auth -> fin
// client recoit (AllocReplica) -> si GGame pas auth -> Netclient::spawn(Game::spawn)

// deletion
// game::destroy -> si netobject* -> netserver::destroy(netobjct*)
// destructeur du netobject * -> si GGame is auth -> fin
// destructeur du netobject * -> si GGame pas auth -> game::destroy( netobject::mGameObject );

