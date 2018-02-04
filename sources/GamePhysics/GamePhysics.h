// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GAMEPHYSICS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GAMEPHYSICS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GAMEPHYSICS_EXPORTS
#define GAMEPHYSICS_API __declspec(dllexport)
#else
#define GAMEPHYSICS_API __declspec(dllimport)
#endif

class IPhysicShip;
class Game;
class PhysicWorld;
class Track;

GAMEPHYSICS_API IPhysicShip* NewGamePhysics( Game *_GGame, PhysicWorld *_physicWorld, Track *_track );
