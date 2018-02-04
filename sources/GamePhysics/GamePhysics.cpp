// GamePhysics.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "GamePhysics.h"
#include "../ZShipPhysics.h"


// This is an example of an exported function.
GAMEPHYSICS_API IPhysicShip* fnGamePhysics( Game *_GGame, IPhysicWorld *_physicWorld, ITrack *_track )
{
	return new ZShipPhysics( _GGame, _physicWorld, _track);
}
