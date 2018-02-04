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
#include "toolbox.h"

#include "render.h"
#include "game.h"


#if IS_OS_WINDOWS
#include <Mmsystem.h> // for timeGetTime()
#endif

#include <algorithm>    // for sort

#if IS_OS_WINDOWS
#include <io.h>         // for _finddata_t, _findfirst, _findclose
#else
#include <dirent.h>
#endif

typedef std::string tstring;


// DEBUG
//char	gText[ 1024 ];


void GetFilesList(std::vector<std::string>& aList, const char *szPath, const char *szWild, bool bRecurs, bool bDirectoriesInsteadOfFiles, bool bCompletePath)
{
#if IS_OS_WINDOWS

	_finddata_t fileinfo;
	tstring path = szPath;
	tstring wildc = "*";

	long fret;
    std::string findDir = path + wildc;
    intptr_t fndhand = _findfirst( findDir.c_str(), &fileinfo);
	if (fndhand != -1)
	{
		do
		{
			if (strcmp(fileinfo.name,".") && strcmp(fileinfo.name,"..") )
			{
				tstring wildc2;
				wildc2=szPath;
				wildc2+=fileinfo.name;
				if (!(fileinfo.attrib&_A_HIDDEN))
				{
					if (fileinfo.attrib&_A_SUBDIR)
					{
						if (bDirectoriesInsteadOfFiles)
						{
							if (bCompletePath)
							{
								aList.push_back(wildc2);
							}
							else
							{
								aList.push_back(fileinfo.name);
							}
						}

						wildc2+="/";
						if (bRecurs)
							GetFilesList(aList, wildc2.c_str(), szWild, bRecurs, bDirectoriesInsteadOfFiles, bCompletePath);
					}
					else
					{
						if (!bDirectoriesInsteadOfFiles)
						{
							if (strstr(wildc2.c_str(), szWild))
							{
								if (bCompletePath)
								{
									aList.push_back(wildc2);
								}
								else
								{
									aList.push_back(fileinfo.name);
								}
							}
						}
					}
				}
			}
			fret=_findnext(fndhand,&fileinfo);
		}
		while (fret!=-1);
	}
	_findclose(fndhand);

	// BigFile
	//BFDumpList(aList, szPath, szWild);
#else   //  not IS_OS_WINDOWS

	struct dirent *lecture;
	DIR *rep;
	rep = opendir(szPath);

	while ( (rep != NULL) && (lecture = readdir(rep)))
	{
        if ( strcmp(lecture->d_name, ".") && strcmp(lecture->d_name, "..") && (lecture->d_name[0] != '.') )
        {

            tstring wildc2;
            wildc2 = szPath;
            wildc2 += lecture->d_name;
            if (lecture->d_type == DT_DIR)
            {
                if (bDirectoriesInsteadOfFiles)
                {
                    if (bCompletePath)
                    {
                        aList.push_back(wildc2);
                    }
                    else
                    {
                        aList.push_back(lecture->d_name);
                    }
                }

                wildc2+="/";
                if (bRecurs)
                    GetFilesList(aList, wildc2.c_str(), szWild, bRecurs, bDirectoriesInsteadOfFiles, bCompletePath);
            }
            else if (lecture->d_type == DT_REG)
            {
                if (!bDirectoriesInsteadOfFiles)
                    if (strstr(wildc2.c_str(), szWild))
                    {
                        if (bCompletePath)
                        {
                            aList.push_back(wildc2);
                        }
                        else
                        {
                            aList.push_back(lecture->d_name);
                        }
                    }
            }

        }


	}

	closedir(rep);

#endif  //  not IS_OS_WINDOWS
}

unsigned int mNumberOfArrayPools;
unsigned int mTotalBytesInArrayPools;

#define	_NAME_SEPARATOR_			"|"
#define	_THREADID_NAME_SEPARATOR_	"@"

#if IS_OS_WINDOWS
#pragma comment(lib, "Winmm.lib")
#endif

#if USE_PROFILER

using namespace std;

void TimerInit();
//#if defined(WIN32)
double	startHighResolutionTimer( void );
/*
#elif defined(_MAC_)
void	startHighResolutionTimer(unsigned long time[2]);
#else
void	startHighResolutionTimer(unsigned long time[2]);
#endif
*/
//unsigned long	endHighResolutionTimer(unsigned long time[2]);


#if IS_OS_WINDOWS
// Create A Structure For The Timer Information
struct
{
    __int64			frequency;									// Timer Frequency
    float			resolution;									// Timer Resolution
    unsigned long	mm_timer_start;								// Multimedia Timer Start Value
    unsigned long	mm_timer_elapsed;							// Multimedia Timer Elapsed Time
    bool			performance_timer;							// Using The Performance Timer?
    __int64			performance_timer_start;					// Performance Timer Start Value
    __int64			performance_timer_elapsed;					// Performance Timer Elapsed Time
} timer;														// Structure Is Named timer

#endif  //  IS_OS_WINDOWS


typedef struct stGenProfilerData
{
    double			totalTime;
    double			averageTime;
    double			minTime;
    double			maxTime;
    double			lastTime;				// Time of the previous passage
    double			elapsedTime;			// Elapsed Time
    unsigned long	nbCalls;				// Numbers of calls
    char			szBunchCodeName[1024];	// temporary.
} tdstGenProfilerData;

//  Hold the call stack
typedef vector<tdstGenProfilerData> tdCallStackType;

//	Hold the sequence of profiler_starts
map<tstring, tdstGenProfilerData> mapProfilerGraph;

// Hold profiler data vector in function of the thread
map<unsigned long, tdCallStackType> mapCallsByThread;

// Critical section
ZCriticalSection_t	*gProfilerCriticalSection;


// Critical section functions
/*
void NewCriticalSection( void );
void DestroyCriticalSection( void );
void LockCriticalSection( void );
void UnLockCriticalSection( void );
*/

//
// Activate the profiler
//
bool Zprofiler_enable()
{
    // Initialize the timer
    TimerInit();

    // Create the mutex
    gProfilerCriticalSection = NewCriticalSection();

    // Clear maps
    /*
    mapCallsByThread.clear();
    mapProfilerGraph.clear();
    mapCallsByThread.clear();
    */



    return true;
}

//
// Deactivate the profiler
//
void Zprofiler_disable()
{
    // Dump to file
    //Zprofiler_dumpToFile( DUMP_FILENAME );

    // Clear maps
    mapCallsByThread.clear();
    mapProfilerGraph.clear();
    mapCallsByThread.clear();

    // Delete the mutex
    DestroyCriticalSection(gProfilerCriticalSection);
}
#if IS_OS_MACOSX
unsigned long GetCurrentThreadId() { return 0; }
#elif IS_OS_LINUX
unsigned long GetCurrentThreadId() { return 0; }
#endif

//
// Start the profiling of a bunch of code
//
void Zprofiler_start( const char *profile_name )
{
    LockCriticalSection(gProfilerCriticalSection);

    unsigned long ulThreadId = GetCurrentThreadId();

    // Add the profile name in the callstack vector
    tdstGenProfilerData	GenProfilerData;
    memset(&GenProfilerData, 0, sizeof(GenProfilerData));
    GenProfilerData.lastTime	= startHighResolutionTimer();
    GenProfilerData.minTime		= 0xFFFFFFFF;

    // Find or add callstack
    tdCallStackType TmpCallStack;
    map<unsigned long, tdCallStackType>::iterator IterCallsByThreadMap = mapCallsByThread.find(ulThreadId);
    if( IterCallsByThreadMap == mapCallsByThread.end() )
    {
        // Not found. So insert the new pair
        mapCallsByThread.insert(std::make_pair(ulThreadId, TmpCallStack));
        IterCallsByThreadMap = mapCallsByThread.find(ulThreadId);
    }

    // It's the first element of the vector
    if ((*IterCallsByThreadMap).second.empty())
    {
        GenProfilerData.nbCalls	= 1;
        sprintf(GenProfilerData.szBunchCodeName, "%s%d%s%s", _NAME_SEPARATOR_, (int)ulThreadId, _THREADID_NAME_SEPARATOR_, profile_name);
        (*IterCallsByThreadMap).second.push_back( GenProfilerData );
    }
    // It's not the first element of the vector
    else
    {
        // Update the number of calls
        GenProfilerData.nbCalls++;

        // We need to construct the string with the previous value of the
        // profile_start
        char *previousString = (*IterCallsByThreadMap).second[(*IterCallsByThreadMap).second.size()-1].szBunchCodeName;

        // Add the current profile start string
        strcpy(GenProfilerData.szBunchCodeName, previousString);
        strcat(GenProfilerData.szBunchCodeName, _NAME_SEPARATOR_);
        strcat(GenProfilerData.szBunchCodeName, profile_name);

        // Push it
        (*IterCallsByThreadMap).second.push_back( GenProfilerData );
    }

    UnLockCriticalSection(gProfilerCriticalSection);
}

//
// Stop the profiling of a bunch of code
//
void Zprofiler_end( )
{
    unsigned long ulThreadId = GetCurrentThreadId();

    // Retrieve the right entry in function of the threadId
    map<unsigned long, tdCallStackType>::iterator IterCallsByThreadMap = mapCallsByThread.find(ulThreadId);

    // Check if vector is empty
    if( (*IterCallsByThreadMap).second.empty() )
    {
        LOG( "Il y a une erreur dans le vecteur CallStack !!!\n\n");
        return;
    }

    LockCriticalSection(gProfilerCriticalSection);

    // Retrieve the last element from the callstack vector
    tdstGenProfilerData	GenProfilerData;
    GenProfilerData	= (*IterCallsByThreadMap).second[(*IterCallsByThreadMap).second.size()-1];

    // Compute elapsed time
    GenProfilerData.elapsedTime += startHighResolutionTimer()-GenProfilerData.lastTime;
    GenProfilerData.totalTime	+= GenProfilerData.elapsedTime;

    // Find if this entry exists in the map
    map<tstring, tdstGenProfilerData>::iterator	IterMap;
    IterMap	= mapProfilerGraph.find( GenProfilerData.szBunchCodeName );
    if( IterMap!=mapProfilerGraph.end() )
    {
        (*IterMap).second.nbCalls++;

        // Retrieve time information to compute min and max time
        double minTime		= (*IterMap).second.minTime;
        double maxTime		= (*IterMap).second.maxTime;
        //double totalTime	= (*IterMap).second.totalTime;

        if( GenProfilerData.elapsedTime<minTime )
        {
            (*IterMap).second.minTime	= GenProfilerData.elapsedTime;
        }

        if( GenProfilerData.elapsedTime>maxTime )
        {
            (*IterMap).second.maxTime	= GenProfilerData.elapsedTime;
        }

        // Compute Total Time
        (*IterMap).second.totalTime	   += GenProfilerData.elapsedTime;
        // Compute Average Time
        (*IterMap).second.averageTime	= (*IterMap).second.totalTime/(*IterMap).second.nbCalls;

    }
    else
    {
        GenProfilerData.minTime	= GenProfilerData.elapsedTime;
        GenProfilerData.maxTime	= GenProfilerData.elapsedTime;

        // Compute Average Time
        GenProfilerData.averageTime	= GenProfilerData.totalTime/GenProfilerData.nbCalls;

        // Insert this part of the call stack into the Progiler Graph
        mapProfilerGraph.insert( std::make_pair(GenProfilerData.szBunchCodeName, GenProfilerData) );
    }

    // Now, pop back the GenProfilerData from the vector callstack
    (*IterCallsByThreadMap).second.pop_back();

    UnLockCriticalSection(gProfilerCriticalSection);
}

//
// Dump all data in a file
//

bool MyDataSortPredicate(const tdstGenProfilerData un, const tdstGenProfilerData deux)
{
    return un.szBunchCodeName > deux.szBunchCodeName;
}

void LogProfiler()
{

    // Thread Id String
    char szThreadId[16];
    char textLine[1024];
    char *tmpString;

    long i;
    //long nbTreadIds	= 0;
    long size		= 0;

    // Map for calls
    map<tstring, tdstGenProfilerData> mapCalls;
    map<tstring, tdstGenProfilerData>::iterator IterMapCalls;
    mapCalls.clear();

    // Map for Thread Ids
    map<tstring, int> ThreadIdsCount;
    map<tstring, int>::iterator IterThreadIdsCount;
    ThreadIdsCount.clear();

    // Vector for callstack
    vector<tdstGenProfilerData> tmpCallStack;
    vector<tdstGenProfilerData>::iterator IterTmpCallStack;
    tmpCallStack.clear();

    // Copy map data into a vector in the aim to sort it
    map<tstring, tdstGenProfilerData>::iterator	IterMap;
    for(IterMap=mapProfilerGraph.begin(); IterMap!=mapProfilerGraph.end(); ++IterMap)
    {
        strcpy((*IterMap).second.szBunchCodeName, (*IterMap).first.c_str());
        tmpCallStack.push_back( (*IterMap).second );
    }

    // Sort the vector
    std::sort(tmpCallStack.begin(), tmpCallStack.end(), MyDataSortPredicate);

    // Create a map with thread Ids
    for(IterTmpCallStack=tmpCallStack.begin(); IterTmpCallStack!=tmpCallStack.end(); ++IterTmpCallStack)
    {
        //// DEBUG
        //fprintf(DumpFile, "%s\n", (*IterTmpCallStack).szBunchCodeName );
        //// DEBUG

        tmpString	= strstr((*IterTmpCallStack).szBunchCodeName, _THREADID_NAME_SEPARATOR_);
        size		= (long)(tmpString-(*IterTmpCallStack).szBunchCodeName);
        strncpy(szThreadId, (*IterTmpCallStack).szBunchCodeName+1, size-1); szThreadId[size-1] = 0;
        ThreadIdsCount[szThreadId]++;
    }

    // Retrieve the number of thread ids
    //unsigned long nbThreadIds	= mapProfilerGraph.size();

    IterThreadIdsCount = ThreadIdsCount.begin();
    for(unsigned long nbThread=0;nbThread<ThreadIdsCount.size();nbThread++)
    {
        sprintf(szThreadId, "%s", IterThreadIdsCount->first.c_str() );

        LOG("CALLSTACK of Thread %s\n", szThreadId);
        LOG("_______________________________________________________________________________________\n");
        LOG("| Temps total  | Temps Moyen  |   Temps Min  |  Temps Max   | Appels | Nom de l'appel\n");
        LOG("_______________________________________________________________________________________\n");

        long nbSeparator = 0;
        for(IterTmpCallStack=tmpCallStack.begin(); IterTmpCallStack!=tmpCallStack.end(); ++IterTmpCallStack)
        {
            tmpString	= (*IterTmpCallStack).szBunchCodeName+1;

            if( strstr(tmpString, szThreadId) )
            {
                // Count the number of separator in the string
                nbSeparator	= 0;
                while( *tmpString )
                {
                    if( *tmpString++== '|' )
                    {
                        nbSeparator++;
                    }
                }

                // Get times and fill in the dislpay string
                sprintf(textLine, "| %12.4f | %12.4f | %12.4f | %12.4f |%6d  | ",
                    (*IterTmpCallStack).totalTime,
                    (*IterTmpCallStack).averageTime,
                    (*IterTmpCallStack).minTime,
                    (*IterTmpCallStack).maxTime,
                    (int)(*IterTmpCallStack).nbCalls);

                // Get the last start_profile_name in the string
                tmpString = strrchr((*IterTmpCallStack).szBunchCodeName, '|')+1;

                IterMapCalls	= mapCalls.find( tmpString );
                if( IterMapCalls!=mapCalls.end() )
                {
                    double minTime			= (*IterMapCalls).second.minTime;
                    double maxTime			= (*IterMapCalls).second.maxTime;
                    //double totalTime		= (*IterMapCalls).second.totalTime;
                    //double averageTime		= (*IterMapCalls).second.averageTime;
                    //unsigned long nbCalls	= (*IterMapCalls).second.nbCalls;

                    if( (*IterTmpCallStack).minTime<minTime )
                    {
                        (*IterMapCalls).second.minTime	= (*IterTmpCallStack).minTime;
                    }
                    if( (*IterTmpCallStack).maxTime>maxTime )
                    {
                        (*IterMapCalls).second.maxTime	= (*IterTmpCallStack).maxTime;
                    }
                    (*IterMapCalls).second.totalTime		+= (*IterTmpCallStack).totalTime;
                    (*IterMapCalls).second.nbCalls			+= (*IterTmpCallStack).nbCalls;
                    (*IterMapCalls).second.averageTime		= (*IterMapCalls).second.totalTime/(*IterMapCalls).second.nbCalls;
                }

                else
                {
                    tdstGenProfilerData tgt;// = (*IterMap).second;
                    if( strstr(tmpString, szThreadId) )
                    {
                        strcpy( tgt.szBunchCodeName, tmpString );
                    }
                    else
                    {
                        sprintf(tgt.szBunchCodeName, "%s%s%s",
                            szThreadId,
                            _THREADID_NAME_SEPARATOR_,
                            tmpString);
                    }

                    tgt.minTime			= (*IterTmpCallStack).minTime;
                    tgt.maxTime			= (*IterTmpCallStack).maxTime;
                    tgt.totalTime			= (*IterTmpCallStack).totalTime;
                    tgt.averageTime		= (*IterTmpCallStack).averageTime;
                    tgt.elapsedTime		= (*IterTmpCallStack).elapsedTime;
                    tgt.lastTime			= (*IterTmpCallStack).lastTime;
                    tgt.nbCalls			= (*IterTmpCallStack).nbCalls;

                    mapCalls.insert( std::make_pair(tmpString, tgt) );
                }


                // Copy white space in the string to format the display
                // in function of the hierarchy
                for(i=0;i<nbSeparator;i++) strcat(textLine, "  ");

                // Remove the thred if from the string
                if( strstr(tmpString, _THREADID_NAME_SEPARATOR_) )
                {
                    tmpString += strlen(szThreadId)+1;
                }

                // Display the name of the bunch code profiled
                LOG("%s%s\n", textLine, tmpString );
            }
        }
        LOG("_______________________________________________________________________________________\n\n");
        ++IterThreadIdsCount;
    }
    LOG( "\n\n");

    //
    //	DUMP CALLS
    //
    IterThreadIdsCount = ThreadIdsCount.begin();
    for(unsigned long nbThread=0;nbThread<ThreadIdsCount.size();nbThread++)
    {
        sprintf(szThreadId, "%s", IterThreadIdsCount->first.c_str() );

        LOG( "DUMP of Thread %s\n", szThreadId);
        LOG( "_______________________________________________________________________________________\n");
        LOG( "| Temps total  | Temps Moyen  |  Temps Min   |  Temps Max   | Appels | Nom de l'appel\n");
        LOG( "_______________________________________________________________________________________\n");

        for(IterMapCalls=mapCalls.begin(); IterMapCalls!=mapCalls.end(); ++IterMapCalls)
        {
            tmpString = (*IterMapCalls).second.szBunchCodeName;
            if( strstr(tmpString, szThreadId) )
            {
                LOG( "| %12.4f | %12.4f | %12.4f | %12.4f | %6d | %s\n",
                    (*IterMapCalls).second.totalTime,
                    (*IterMapCalls).second.averageTime,
                    (*IterMapCalls).second.minTime,
                    (*IterMapCalls).second.maxTime,
                    (int)(*IterMapCalls).second.nbCalls,
                    (*IterMapCalls).second.szBunchCodeName+strlen(szThreadId)+1);
            }
        }
        LOG( "_______________________________________________________________________________________\n\n");
        ++IterThreadIdsCount;
    }

}

////
////	Gestion des timers
////
#if IS_OS_WINDOWS

// Initialize Our Timer (Get It Ready)
void TimerInit()
{
    memset(&timer, 0, sizeof(timer));

    // Check to see if a performance counter is available
    // If one is available the timer frequency will be updated
    if( !QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency) )
    {
        // No performace counter available
        timer.performance_timer	= false;					// Set performance timer to false
        timer.mm_timer_start	= timeGetTime();			// Use timeGetTime() to get current time
        timer.resolution		= 1.0f/1000.0f;				// Set our timer resolution to .001f
        timer.frequency			= 1000;						// Set our timer frequency to 1000
        timer.mm_timer_elapsed	= timer.mm_timer_start;		// Set the elapsed time to the current time
    }
    else
    {
        // Performance counter is available, use it instead of the multimedia timer
        // Get the current time and store it in performance_timer_start
        QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
        timer.performance_timer			= true;				// Set performance timer to true

        // Calculate the timer resolution using the timer frequency
        timer.resolution				= (float) (((double)1.0f)/((double)timer.frequency));

        // Set the elapsed time to the current time
        timer.performance_timer_elapsed	= timer.performance_timer_start;
    }
}

// platform specific get hires times...
double startHighResolutionTimer()
{
    __int64 time;

    // Are we using the performance timer?
    if( timer.performance_timer )
    {
        // Grab the current performance time
        QueryPerformanceCounter((LARGE_INTEGER *) &time);

        // Return the current time minus the start time multiplied
        // by the resolution and 1000 (To Get MS)
        return ( (double) ( time - timer.performance_timer_start) * timer.resolution)*1000.0f;
    }
    else
    {
        // Return the current time minus the start time multiplied
        // by the resolution and 1000 (To Get MS)
        return( (double) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
    }
}
/*
unsigned long endHighResolutionTimer(unsigned long time[2])
{
unsigned long ticks=0;
//__asm__ __volatile__(
//   "rdtsc\n"
//   "sub  0x4(%%ecx),  %%edx\n"
//   "sbb  (%%ecx),  %%eax\n"
//   : "=a" (ticks) : "c" (time)
//   );
return ticks;
}
*/
#elif IS_OS_MACOSX

// Initialize Our Timer (Get It Ready)
void TimerInit()
{
}

double startHighResolutionTimer()
{
    UnsignedWide t;
    Microseconds(&t);
    /*time[0] = t.lo;
    time[1] = t.hi;
    */
    double ms = double(t.hi*1000LU);
    ms += double(t.lo/1000LU);//*0.001;


    return ms;
}
/*
unsigned long endHighResolutionTimer(unsigned long time[2])
{
UnsignedWide t;
Microseconds(&t);
return t.lo - time[0];
// given that we're returning a 32 bit integer, and this is unsigned subtraction...
// it will just wrap around, we don't need the upper word of the time.
// NOTE: the code assumes that more than 3 hrs will not go by between calls to startHighResolutionTimer() and endHighResolutionTimer().
// I mean... that damn well better not happen anyway.
}
*/
#else

// Initialize Our Timer (Get It Ready)
void TimerInit()
{
}

double startHighResolutionTimer()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts); // Works on Linux


    double ms = double(ts.tv_sec*1000LU);
    ms += double(ts.tv_nsec/1000LU)*0.001;


    return ms;
}
/*
unsigned long endHighResolutionTimer(unsigned long time[2])
{
return 1;
}
*/
#endif

#endif // USE_PROFILER

// Gestion des sections critiques
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////////////
// web

/*
// object 1
typedef struct testObject1 : public serialisableObject_t
{
    virtual ~testObject1() {}

    virtual const char *GetObjectName() const { return mName.c_str(); }
    int value1;
    float value2;
    bool value3;
    std::string mName;
    SERIALIZABLE(testObject1,"Test Object: Checking int and float numbers.")
} testObject1;



serializableField_t testObject1::mSerialisableFields[] = {SE(testObject1,value1),
    SE(testObject1,value2),
    SE(testObject1, mName)

};
NBFIELDS(testObject1);

// object 2
typedef struct shaderObject_t : public serialisableObject_t
{
    virtual ~shaderObject_t() {}

    std::string mVertexShader, mFragmentShader;
    testObject1 mObject0;
    testObject1 *mObject1;
    std::vector<testObject1> objs;
    testObject1 myStaticArray[18];

    SERIALIZABLE(shaderObject_t,"Contains a GLSL Vertex and Fragment Shader. Displays compilation errors.")
} shaderObject_t;

serializableField_t shaderObject_t::mSerialisableFields[] = {SEFD(shaderObject_t,mVertexShader,0,"Vertex Shader Code"),
    SE(shaderObject_t,mFragmentShader),
    SE(shaderObject_t,mObject0),
    SE(shaderObject_t,mObject1),
    SE(shaderObject_t,objs ),
    SE(shaderObject_t, myStaticArray)
};
NBFIELDS(shaderObject_t);
*/



void ShowMembers( serialisableObject_t *pObj )
{
    const serializableField_t *pf = pObj->GetFields();
    for (int i=0;i<pObj->GetNbFields();i++)
    {


        printf(" - %s\n", pf->name);

        if ( pf->type == TYPE_OBJECT)
            ShowMembers( (serialisableObject_t*) (((u8*)pObj)+pf->offset) );

        if ( pf->type == TYPE_OBJECT_PTR)
        {
            serialisableObject_t* newo = *(serialisableObject_t**) (((u8*)pObj)+pf->offset);
            if ( newo )
                ShowMembers( newo );
        }
        if ( pf->container )
        {
            int cnt = pf->container->GetCount( ((u8*)pObj)+pf->offset );
            printf(" container has %d items.\n", cnt );
        }
        pf++;
    }
}

const serializableField_t * serialisableObject_t::GetFieldByName( const char *szMemberName ) const
{
    const serializableField_t *pf = this->GetFields();
    for (int i=0;i<this->GetNbFields();i++, pf++)
    {
        if (! strcmp(szMemberName, pf->name))
            return pf;
    }
    return NULL;
}

u8 * serialisableObject_t::GetFieldPointer( const serializableField_t * pf, int idx) const
{
    if ( pf->container )
    {
        u8* pCont = (((u8*)this)+pf->offset);
        return (u8*)pf->container->GetValue( pCont, idx );
    }
    else
        if ( pf->isPointer )
        {
            serialisableObject_t ** pObj = (serialisableObject_t **)(((u8*)this)+pf->offset);
            return (u8*)*pObj;
        }

        return (((u8*)this)+pf->offset);
}


const char *serialisableObject_t::GetVariableName( serialisableObject_t *pObj ) const
{
    const serializableField_t *pf = this->GetFields();
    for (int i=0;i<this->GetNbFields();i++, pf++)
    {
        if ( (((u8*)this)+pf->offset) == (u8*)pObj)
            return pf->name;
    }
    return NULL;
}


#ifndef RETAIL
#include "mongoose.h"


void(*resetWebFunc)();


void parseHTML( serialisableObject_t *pObj, char const *query_string)
{
    int requestLen = strlen( query_string);
    const serializableField_t *pf = pObj->GetFields();
    for (int i=0;i<pObj->GetNbFields();i++, pf++)
    {
        char buf[2048];
        setlocale(LC_ALL, "C");

        if ( ( pf->type == TYPE_OBJECT ) || ( pf->type == TYPE_OBJECT_PTR ) )
            continue;

        if ( pf->type == TYPE_VECT )
        {
            float *pv = (float*)(((u8*)pObj)+pf->offset);
            static const char *exts[4] = {".x",".y",".z",".w"};
            for (int j = 0 ; j< 4; j++)
            {
                std::string str = pf->name;
                str += exts[j];
                mg_get_var( query_string, requestLen, str.c_str(), buf, 2048);
                if (buf[0])
                    (*pv++) = (float)atof( buf );

            }
            continue;
        }
        mg_get_var( query_string, requestLen, pf->name, buf, 2048);

        if (!buf[0])
        {
            if ( pf->type == TYPE_BOOLEAN)
                *(bool*)(((u8*)pObj)+pf->offset) = false;
            continue;
        }

        switch (pf->type)
        {
        case TYPE_INT:
            *(int*)(((u8*)pObj)+pf->offset) = atoi(buf);
            break;
        case TYPE_FLOAT:
            *(float*)(((u8*)pObj)+pf->offset) = (float)atof(buf);
            break;
        case TYPE_STRING:
            *(std::string*)(((u8*)pObj)+pf->offset) = buf;
            break;
        case TYPE_BOOLEAN:
            *(bool*)(((u8*)pObj)+pf->offset) = (strcmp(buf,"on") == 0);
            break;
        case TYPE_U32:
            sscanf(buf,"%x",(u32*)(((u8*)pObj)+pf->offset) );
            break;
        default:
            break;
        }
    }
}

void generateHTML( struct mg_connection *conn, serialisableObject_t *pObj, const char* szPath )
{
    mg_printf(conn, "<input type=\"text\" name=\"path\" style=\"display:none\" value=\"%s\"/>", szPath );

    std::string pathDotted = szPath;
    pathDotted += szPath[0]?".":"";

    const serializableField_t *pf = pObj->GetFields();
    for ( int i = 0 ; i < pObj->GetNbFields() ; i++, pf++ )
    {
        setlocale(LC_ALL, "C");

        if ( pf->flags&SF_NOEDIT)
            continue;

        if ( pf->container )
        {
            u8* pCont = ((u8*)pObj)+pf->offset;

            mg_printf( conn, "<div class=\"clearfix\"><label for=\"normalSelect\">%s</label><div class=\"input\"><select name=\"normalSelect\" id=\"normalSelect\"onChange=\"location = this.options[this.selectedIndex].value;\">", pf->name );
            mg_printf( conn, "<option value=\"#\" >Select to jump</option>");

            for (int j=0;j<pf->container->GetCount(pCont);j++)
            {
                serialisableObject_t *pSerialObj = (serialisableObject_t *)pf->container->GetValue( pCont, j );
                const char *szObjName = pSerialObj->GetObjectName();
                if ( szObjName && szObjName[0] )
                    mg_printf( conn, "<option value=\"/?path=%s%s[%d]\" >%s</option>", pathDotted.c_str(), pf->name, j, szObjName );
                else
                    mg_printf( conn, "<option value=\"/?path=%s%s[%d]\" >%d</option>", pathDotted.c_str(), pf->name, j, j );
            }

            mg_printf( conn, "</select></div></div>" );
        }
        else if ( pf->type == TYPE_INT)
        {
            mg_printf(conn, "<div class=\"clearfix\"><label for=\"xlInput\">%s</label><div class=\"input\"><input class=\"xlarge\" type=\"text\" name=\"%s\" value=\"%d\"/></div><span class=\"help-block\">%s</span></div>",
                pf->name, pf->name, *(int*) (((u8*)pObj)+pf->offset), pf->description );
        }
        else if ( pf->type == TYPE_U32)
        {
            mg_printf(conn, "<div class=\"clearfix\"><label for=\"xlInput\">%s</label><div class=\"input\"><input class=\"xlarge\" type=\"text\" name=\"%s\" value=\"%x\"/></div><span class=\"help-block\">%s</span></div>",
                pf->name, pf->name, *(u32*) (((u8*)pObj)+pf->offset), pf->description );
        }
        else if ( pf->type == TYPE_FLOAT)
        {
            mg_printf(conn, "<div class=\"clearfix\"><label for=\"xlInput\">%s</label><div class=\"input\"><input class=\"xlarge\" type=\"text\" name=\"%s\" value=\"%5.3f\"/></div><span class=\"help-block\">%s</span></div>",
                pf->name, pf->name, *(float*) (((u8*)pObj)+pf->offset), pf->description );
        }
        else if ( pf->type == TYPE_VECT)
        {
            vec_t* pvt = ((vec_t*) (((u8*)pObj)+pf->offset));
            mg_printf(conn, "<div class=\"clearfix\"><label for=\"xlInput\">%s.x</label><div class=\"input\"><input class=\"xlarge\" type=\"text\" name=\"%s.x\" value=\"%5.3f\"/></div></div>",
                pf->name, pf->name, pvt->x );
            mg_printf(conn, "<div class=\"clearfix\"><label for=\"xlInput\">y</label><div class=\"input\"><input class=\"xlarge\" type=\"text\" name=\"%s.y\" value=\"%5.3f\"/></div></div>",
                pf->name, pvt->y );
            mg_printf(conn, "<div class=\"clearfix\"><label for=\"xlInput\">z</label><div class=\"input\"><input class=\"xlarge\" type=\"text\" name=\"%s.z\" value=\"%5.3f\"/></div></div>",
                pf->name, pvt->z );
            mg_printf(conn, "<div class=\"clearfix\"><label for=\"xlInput\">w</label><div class=\"input\"><input class=\"xlarge\" type=\"text\" name=\"%s.w\" value=\"%5.3f\"/></div><span class=\"help-block\">%s</span></div>",
                pf->name, pvt->w, pf->description );

        }
        else if ( pf->type == TYPE_STRING)
        {
            std::string *ptrString = ((std::string*) (((u8*)pObj)+pf->offset));
            if (pf->flags&SF_ERRORLOG)
            {
                if (ptrString->length())
                    mg_printf(conn, "<div class=\"alert-message error\"><a class=\"close\" href=\"#\">×</a><p>%s</p></div>", ptrString->c_str() );
            }
            else
            {
                mg_printf(conn, "<div class=\"clearfix\"><label for=\"textarea\">%s</label><div class=\"input\"><textarea class=\"xxlarge\" name=\"%s\" id=\"%s\" rows=\"3\">%s</textarea><br/></div><span class=\"help-block\">%s</span></div>",
                    pf->name, pf->name, pf->name, ptrString->c_str(), pf->description );
            }
        }
        else if ( pf->type == TYPE_OBJECT)
        {
            serialisableObject_t *memberObject = (serialisableObject_t*)(((u8*)pObj)+pf->offset);
            mg_printf(conn, "<div class=\"clearfix\"><div class=\"input\"><a href=\"/?path=%s%s\" class=\"btn small\">%s %s</a><br><span class=\"help-block\">%s</span></div></div>", pathDotted.c_str(), pf->name, memberObject->GetTypeName(), pf->name, pf->description );
        }
        else if ( pf->type == TYPE_OBJECT_PTR)
        {
            serialisableObject_t *memberObject = *(serialisableObject_t**)(((u8*)pObj)+pf->offset);
            if (memberObject)
                mg_printf(conn, "<div class=\"clearfix\"><div class=\"input\"><a href=\"/?path=%s%s\" class=\"btn small\">%s %s</a><br></div></div><span class=\"help-block\">%s</span>", pathDotted.c_str(), pf->name, memberObject->GetTypeName(), pf->name, pf->description );
            else
                mg_printf(conn, "<div class=\"clearfix\"><div class=\"input\"><a href=\"#\" class=\"btn small disabled\">serializeObject_t * %s (NULL)</a><br></div><span class=\"help-block\">%s</span></div>", pf->name, pf->description );
        }
        else if ( pf->type == TYPE_BOOLEAN)
        {
            bool boolValue = *(bool*)(((u8*)pObj)+pf->offset);
            mg_printf(conn, "<div class=\"input\"><ul class=\"inputs-list\"><li><label><input type=\"checkbox\" id=\"%s\" name=\"%s\" %s /><span>%s</span></label><span class=\"help-block\">%s</span></div>",
                pf->name, pf->name, boolValue?"checked=on":"", pf->name, pf->description);
        }

    }
}


int FindIndexInMember( std::string& str )
{
    int stbrk = str.find('[');
    if ( stbrk != std::string::npos )
    {
        std::string brk = str.substr( stbrk, str.length() - stbrk );
        if (brk.length())
        {
            int idx;
            sscanf(brk.c_str(), "[%d]",&idx);
            printf("Found Idx %d\n", idx);
            str = str.substr( 0, stbrk );
            return idx;
        }
    }

    return -1;
}
#endif
extern serialisableObject_t *GRootSerialisableObjectPtr;
#ifndef RETAIL
// -----------------------------------------------------
static void *webcallback(enum mg_event event,
struct mg_connection *conn,
    const struct mg_request_info *request_info)
{

    char buf[2048] = {0};;
    char path[2048]={0};//
    serialisableObject_t *parentPointers[64];

    int parentPointersAv = 0;
    serialisableObject_t *pCurrentObject = NULL;//GRootSerialisableObjectPtr;
    if (request_info->query_string)
    {
        mg_get_var(request_info->query_string, strlen(request_info->query_string),
            "path", path, 2048);
    }

    // tokenize parents

    std::string ribbon = "<ul class=\"breadcrumb\">";

    if (path[0])
        ribbon += "<li><a href=\"/\">Root</a><span class=\"divider\">/</span></li>";

    serialisableObject_t* lastObjectInPointerList = parentPointers[parentPointersAv++] = GRootSerialisableObjectPtr;//&GRootObject;

    std::string lastPath = "";
    std::string variableBeingEdited;

    const std::string& s = path;
    std::string::size_type prev_pos = 0, pos = 0;
    while( (pos = s.find('.', pos)) != std::string::npos )
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        // find []

        int idx = FindIndexInMember( substring );
        //

        //sscanf( substring.c_str(), "%p", &parentPointers[parentPointersAv++]);
        const serializableField_t *pf = lastObjectInPointerList->GetFieldByName( substring.c_str() );
        lastObjectInPointerList = (serialisableObject_t*)lastObjectInPointerList->GetFieldPointer( pf, idx );

        parentPointers[parentPointersAv++] = lastObjectInPointerList;

        if (!lastPath.empty())
            lastPath += ".";

        lastPath += substring;
        if ( idx >= 0 )
        {
            sprintf( buf, "[%d]", idx );
            lastPath += buf;
            substring += buf;
        }

        sprintf(buf, "<li><a href=\"/?path=%s\">%s %s</a><span class=\"divider\">/</span></li>", lastPath.c_str(), lastObjectInPointerList->GetTypeName(), substring.c_str() );
        ribbon += buf;

        prev_pos = ++pos;
    }

    std::string substring( s.substr(prev_pos, pos-prev_pos) ); // Last word
    if (!substring.empty())
    {
        int idx = FindIndexInMember( substring );

        const serializableField_t *pf = lastObjectInPointerList->GetFieldByName( substring.c_str() );
        lastObjectInPointerList = (serialisableObject_t*)lastObjectInPointerList->GetFieldPointer( pf, idx );

        if ( idx >= 0 )
        {
            sprintf( buf, "[%d]", idx );
            substring += buf;
        }

        sprintf(buf, "<li>%s %s</li>", lastObjectInPointerList->GetTypeName(), substring.c_str() );
        ribbon += buf;

        parentPointers[parentPointersAv++] = lastObjectInPointerList;
        variableBeingEdited = substring;
    }
    if (!path[0])
    {
        ribbon += "<li>Root</li>";
    }

    pCurrentObject = parentPointers[parentPointersAv-1];

    ribbon+="</ul>";
    // respond to request

    if (event == MG_NEW_REQUEST)
    {
        if (request_info->query_string)
        {


            parseHTML( pCurrentObject, request_info->query_string );
            if ( pCurrentObject )
                pCurrentObject->FieldsHasBeenModified();
        }


        mg_printf(conn, "HTTP/1.0 200 Ok\r\nContent-Type: text/html; charset=\"utf-8\"\r\n\r\n");

        mg_printf(conn, "<html lang=\"en\"><head><meta charset=\"utf-8\"><title>.the rush// Web Editing Tool</title><meta name=\"description\" content=\"\"><meta name=\"author\" content=\"\">");
        mg_printf(conn, "<script src=\"http://html5shim.googlecode.com/svn/trunk/html5.js\"></script>");
        mg_printf(conn, "<link rel=\"stylesheet\" href=\"http://twitter.github.com/bootstrap/1.4.0/bootstrap.min.css\">");
        mg_printf(conn, "<link href=\"http://www.skaven.fr/assets/docs.css\" rel=\"stylesheet\">");
        mg_printf(conn, "<link href=\"http://twitter.github.com/bootstrap/assets/js/google-code-prettify/prettify.css\" rel=\"stylesheet\">");
        mg_printf(conn, "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"http://twitter.github.com/bootstrap/assets/ico/favicon.ico\">");
        mg_printf(conn, "<link rel=\"apple-touch-icon\" href=\"http://twitter.github.com/bootstrap/assets/ico/bootstrap-apple-57x57.png\">");
        mg_printf(conn, "<link rel=\"apple-touch-icon\" sizes=\"72x72\" href=\"http://twitter.github.com/bootstrap/assets/ico/bootstrap-apple-72x72.png\">");
        mg_printf(conn, "<link rel=\"apple-touch-icon\" sizes=\"114x114\" href=\"http://twitter.github.com/bootstrap/assets/ico/bootstrap-apple-114x114.png\">");
        mg_printf(conn, "</head><body><header class=\"jumbotron masthead\" id=\"overview\"><div class=\"inner\"><div ><h3>.the rush// Web Editing</h3>Web Editing allows you to tweak and change most of game content.<br/>You'll see any change you make the moment you press Apply changes.<br/></div></div></header>");


        mg_printf(conn, "%s\n", ribbon.c_str() );
        mg_printf(conn, "<body><br><div class=\"container-fluid\"><div class=\"sidebar\">");
        const char *objDescription = pCurrentObject?pCurrentObject->GetDescription():GRootSerialisableObjectPtr->GetDescription();
        mg_printf(conn, "<h2>%s</h2>%s</div>", variableBeingEdited.c_str(), objDescription );
        mg_printf(conn, "<div class=\"content\"><div class=\"span12\"><form method=\"get\"><fieldset>");

        if ( pCurrentObject )
        {
            int maxTimeout = 50;
            while ( pCurrentObject->ObjectIsDirty() )
            {
#if IS_OS_WINDOWS
                Sleep(2);
#else
                usleep(2000);
#endif
                maxTimeout--;
                if ( !maxTimeout )
                    break;
            }
            generateHTML( conn, pCurrentObject, path );
        }

        mg_printf(conn, "</fieldset></p>\n<p>\n<div class=\"actions\"><input type=\"submit\" class=\"btn primary\" value=\"Apply changes\"></div></p></form></div></div></div></body></html>");

        return (void*)"";  // Mark as processed
    } else {
        return NULL;
    }
}

// B I N A R Y ! ! ! !

void PushBinaryInString( std::string &str, u8* bytes, int bytesCount)
{
    for (int i=0;i<bytesCount;i++)
    {
        str += bytes[i];
    }
}


std::string SimpleTypeToBinary( serialisableObject_t *pObj, const serializableField_t *pf, int idx )
{

    std::string res;
    switch (pf->type)
    {
    case TYPE_FLOAT:
        PushBinaryInString( res, (((u8*)pObj)+pf->offset)+idx, sizeof( float ) );
        break;
    case TYPE_INT:
        PushBinaryInString( res, (((u8*)pObj)+pf->offset)+idx, sizeof( int ) );
        break;
    case TYPE_U32:
        PushBinaryInString( res, (((u8*)pObj)+pf->offset)+idx, sizeof( u32 ) );
        break;
    case TYPE_STRING:
        {
            std::string *pStr = ((std::string*)(((u8*)pObj)+pf->offset)+idx);
            u32 len = pStr->length();
            PushBinaryInString( res, (u8*)&len, sizeof( u32 ) );
            PushBinaryInString( res, (u8*)pStr->c_str(), len );
        }
        break;
    case TYPE_BOOLEAN:
        {
            u8 bl = *(bool*)(((u8*)pObj)+pf->offset)?0xFF:0x00;
            PushBinaryInString( res, &bl, 1 );
        }
        break;
    case TYPE_VECT:
        PushBinaryInString( res, (((u8*)pObj)+pf->offset)+idx, sizeof( float ) * 4 );
        break;
	case TYPE_MATRIX:
		PushBinaryInString( res, (((u8*)pObj)+pf->offset)+idx, sizeof( float ) * 16 );
		break;
    case TYPE_OBJECT:
        {
            serialisableObject_t *memberObject = (serialisableObject_t*)(((u8*)pObj)+pf->offset+idx);
            res += GenerateBinary( memberObject );
        }
        break;
    case TYPE_OBJECT_PTR:
        {
            serialisableObject_t *memberObject = *(serialisableObject_t**)(((u8*)pObj)+pf->offset+idx);
            u32 nullit = 0;
            if ( memberObject )
                res += GenerateBinary( memberObject );
            else
                PushBinaryInString( res, (u8*)&nullit, sizeof( u32 ) );
        }
        break;
    default:
        break;
    }
    return res;
}

std::string GenerateBinary( serialisableObject_t *pObj )
{
    std::string res;

    const serializableField_t *pf = pObj->GetFields();
    for (int i=0;i<pObj->GetNbFields();i++, pf++)
    {
        if ( pf->flags & SF_TRANSIENT )
            continue;

        if (pf->container)
        {
            void *myContainer = (((u8*)pObj)+pf->offset);
            int count = pf->container->GetCount( myContainer );
            for ( int j = 0 ; j < count ; j ++)
            {
                serialisableObject_t *nextObject = (serialisableObject_t *)pf->container->GetValue( myContainer, j );

                if (pf->type == TYPE_UNDEFINED)
                {
                    res += GenerateBinary( nextObject );
                }
                else
                {
                    res += SimpleTypeToBinary( pObj, pf, j );
                }
            }
        }
        else
        {
            res += SimpleTypeToBinary( pObj, pf, 0 );
        }
    }
    return res;
}
#endif
int PopBinaryDatas( const char *pSrc, u8 *pDest, int bytesCount)
{
    memcpy( pDest, pSrc, bytesCount );
    return bytesCount;
}

int ParseBinary( serialisableObject_t *pObj, const serializableField_t *pf, int idx, const char* pBits )
{
    const char *pBitsSvg = pBits;

    switch (pf->type)
    {
    case TYPE_FLOAT:
        pBits += PopBinaryDatas( pBits, (((u8*)pObj)+pf->offset)+idx, sizeof( float ) );
        break;
    case TYPE_INT:
        pBits += PopBinaryDatas( pBits, (((u8*)pObj)+pf->offset)+idx, sizeof( int ) );
        break;
    case TYPE_U32:
        pBits += PopBinaryDatas( pBits, (((u8*)pObj)+pf->offset)+idx, sizeof( u32 ) );
        break;
    case TYPE_STRING:
        {
            std::string *pStr = ((std::string*)(((u8*)pObj)+pf->offset)+idx);
            u32 len;
            pBits += PopBinaryDatas( pBits, (u8*)&len, sizeof( u32 ) );
            pStr->append( pBits, len );
            pBits += len;
        }
        break;
    case TYPE_BOOLEAN:
        {
            u8 bl;// = *(bool*)(((u8*)pObj)+pf->offset)?0xFF:0x00;
            pBits += PopBinaryDatas( pBits, (u8*)&bl, sizeof( u8 ) );
            *(bool*)(((u8*)pObj)+pf->offset) = (bl!=0)?true:false;
        }
        break;
    case TYPE_VECT:
        pBits += PopBinaryDatas( pBits, (((u8*)pObj)+pf->offset)+idx, sizeof( float ) * 4 );
        break;
	case TYPE_MATRIX:
		pBits += PopBinaryDatas( pBits, (((u8*)pObj)+pf->offset)+idx, sizeof( float ) * 16 );
		break;
    case TYPE_OBJECT:
        {
            serialisableObject_t *memberObject = (serialisableObject_t*)(((u8*)pObj)+pf->offset+idx);
            pBits += ParseBinary( memberObject, pBits  );
        }
        break;
    case TYPE_OBJECT_PTR:
        {
            serialisableObject_t *memberObject = *(serialisableObject_t**)(((u8*)pObj)+pf->offset+idx);
            u32 nullit = 0;
            PopBinaryDatas( pBits, (u8*)&nullit, sizeof( u32 ) );
            if ( nullit )
                pBits += ParseBinary( memberObject, pBits  );
            else
            {
                *(serialisableObject_t**)(((u8*)pObj)+pf->offset+idx) = NULL;
                pBits += sizeof( u32 );
            }
        }
        break;
    default:
        break;
    }
    return ( pBits - pBitsSvg );
}

int ParseBinary( serialisableObject_t *pObj, const char *pBits )
{
    const char *pBitsSvg = pBits;
    const serializableField_t *pf = pObj->GetFields();
    for (int i=0;i<pObj->GetNbFields();i++, pf++)
    {
        if ( pf->flags & SF_TRANSIENT )
            continue;

        if (pf->container)
        {
            void *myContainer = (((u8*)pObj)+pf->offset);
            int count = pf->container->GetCount( myContainer );
            for ( int j = 0 ; j < count ; j ++)
            {
                serialisableObject_t *nextObject = (serialisableObject_t *)pf->container->GetValue( myContainer, j );

                if (pf->type == TYPE_UNDEFINED)
                {
                    pBits += ParseBinary( nextObject, pBits );
                }
                else
                {
                    pBits += ParseBinary( pObj, pf, j, pBits );
                }
            }
        }
        else
        {
            pBits += ParseBinary( pObj, pf, 0, pBits );
        }
    }
    return ( pBits - pBitsSvg );
}
#ifndef RETAIL

void startWeb( void(*resetWeb)() )
{
    const char *options[] = {"listening_ports", "8080", NULL};
    resetWebFunc = resetWeb;
    mg_start(&webcallback, NULL, options);
}

#endif



// Removes duplicates vertices based on compareSize
// returns new vertex count
int OptimizeMesh( int compareSize, u8* vertices, int vtCount, int vtSize, unsigned short *indices, int indicesCount )
{
    //PROFILER_START(MeshOptimize);
    char *vtsrc1 = (char*)vertices;

    //unsigned int vtSize = hasColors?28:24;
    // vtReplacement is for each old vertex index, the corresponding new vertex index
    int *vtReplacement = new int [vtCount];
    memset(vtReplacement, -1, sizeof(int) * vtCount);
    /* vtToUse is for rebuilding vertex array. It contains the list of vertices to append in the new array*/
    int *vtToUse = new int [vtCount];

    // number of new vertex in new vertex array
    int toUseIndex = 0;
    for (int i = 0;i<vtCount;i++, vtsrc1+=vtSize )
    {
        if (vtReplacement[i]>=0)
            continue;

        char *vtsrc2 = vtsrc1;
        vtsrc2 += vtSize;
        for (int j=i+1;j<vtCount;j++, vtsrc2+=vtSize)
        {
            if (vtReplacement[j]>=0)
                continue;
            if (!memcmp(vtsrc1, vtsrc2, compareSize))
            {
                vtReplacement[j] = toUseIndex;
            }
        }
        vtToUse[toUseIndex] = i;
        vtReplacement[i] = toUseIndex++;
    }

    // rebuild index array
    for (int i=0;i<indicesCount;i++)
        indices[i] = static_cast<unsigned short>( vtReplacement[indices[i]] );

    // build new VA
    u8 *newVts, *newVts2;
    newVts2 = newVts = new u8 [ toUseIndex * vtSize ];

    for (int i=0;i<toUseIndex;i++, newVts += vtSize)
        memcpy(newVts, &vertices[vtToUse[i]*vtSize], vtSize);

    //delete [] vertices;
    //vertices = (meshVertex_t*)nVts;
    memcpy ( vertices, newVts2, toUseIndex*vtSize );


    // clean shits
    delete [] newVts2;
    delete [] vtReplacement;
    delete [] vtToUse;

    LOG ("Got %d vertices. // toUseIndex = %d.\n", vtCount, toUseIndex);
    return toUseIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define B SAMPLE_SIZE
#define BM (SAMPLE_SIZE-1)

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

#define s_curve(t) ( t * t * (3.0f - 2.0f * t) )
#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
    t = vec[i] + N;\
    b0 = ((int)t) & BM;\
    b1 = (b0+1) & BM;\
    r0 = t - (int)t;\
    r1 = r0 - 1.0f;

float Perlin::noise1(float arg)
{
    int bx0, bx1;
    float rx0, rx1, sx, t, u, v, vec[1];

    vec[0] = arg;

    if (mStart)
    {
        srand(mSeed);
        mStart = false;
        init();
    }

    setup(0, bx0,bx1, rx0,rx1);

    sx = s_curve(rx0);

    u = rx0 * g1[ p[ bx0 ] ];
    v = rx1 * g1[ p[ bx1 ] ];

    return lerp(sx, u, v);
}

float Perlin::noise2(float vec[2])
{
    int bx0, bx1, by0, by1, b00, b10, b01, b11;
    float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
    int i, j;

    if (mStart)
    {
        srand(mSeed);
        mStart = false;
        init();
    }

    setup(0,bx0,bx1,rx0,rx1);
    setup(1,by0,by1,ry0,ry1);

    i = p[bx0];
    j = p[bx1];

    b00 = p[i + by0];
    b10 = p[j + by0];
    b01 = p[i + by1];
    b11 = p[j + by1];

    sx = s_curve(rx0);
    sy = s_curve(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

    q = g2[b00];
    u = at2(rx0,ry0);
    q = g2[b10];
    v = at2(rx1,ry0);
    a = lerp(sx, u, v);

    q = g2[b01];
    u = at2(rx0,ry1);
    q = g2[b11];
    v = at2(rx1,ry1);
    b = lerp(sx, u, v);


    return lerp(sy, a, b);
}

float Perlin::noise3(float vec[3])
{
    int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
    float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
    int i, j;

    if (mStart)
    {
        srand(mSeed);
        mStart = false;
        init();
    }

    setup(0, bx0,bx1, rx0,rx1);
    setup(1, by0,by1, ry0,ry1);
    setup(2, bz0,bz1, rz0,rz1);

    i = p[ bx0 ];
    j = p[ bx1 ];

    b00 = p[ i + by0 ];
    b10 = p[ j + by0 ];
    b01 = p[ i + by1 ];
    b11 = p[ j + by1 ];

    t  = s_curve(rx0);
    sy = s_curve(ry0);
    sz = s_curve(rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

    q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
    q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
    a = lerp(t, u, v);

    q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
    q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
    b = lerp(t, u, v);

    c = lerp(sy, a, b);

    q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
    q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
    a = lerp(t, u, v);

    q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
    q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
    b = lerp(t, u, v);

    d = lerp(sy, a, b);

    return lerp(sz, c, d);
}

void Perlin::normalize2(float v[2])
{
    float s;

    s = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
    s = 1.0f/s;
    v[0] = v[0] * s;
    v[1] = v[1] * s;
}

void Perlin::normalize3(float v[3])
{
    float s;

    s = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    s = 1.0f/s;

    v[0] = v[0] * s;
    v[1] = v[1] * s;
    v[2] = v[2] * s;
}

void Perlin::init(void)
{
    int i, j, k;

    for (i = 0 ; i < B ; i++)
    {
        p[i] = i;
        g1[i] = (float)((rand() % (B + B)) - B) / B;
        for (j = 0 ; j < 2 ; j++)
            g2[i][j] = (float)((rand() % (B + B)) - B) / B;
        normalize2(g2[i]);
        for (j = 0 ; j < 3 ; j++)
            g3[i][j] = (float)((rand() % (B + B)) - B) / B;
        normalize3(g3[i]);
    }

    while (--i)
    {
        k = p[i];
        p[i] = p[j = rand() % B];
        p[j] = k;
    }

    for (i = 0 ; i < B + 2 ; i++)
    {
        p[B + i] = p[i];
        g1[B + i] = g1[i];
        for (j = 0 ; j < 2 ; j++)
            g2[B + i][j] = g2[i][j];
        for (j = 0 ; j < 3 ; j++)
            g3[B + i][j] = g3[i][j];
    }

}


float Perlin::perlin_noise_2D(float vec[2])
{
    int terms    = mOctaves;
    //float freq   = mFrequency;
    float result = 0.0f;
    float amp = mAmplitude;

    vec[0]*=mFrequency;
    vec[1]*=mFrequency;

    for( int i=0; i<terms; i++ )
    {
        result += noise2(vec)*amp;
        vec[0] *= 2.0f;
        vec[1] *= 2.0f;
        amp*=0.5f;
    }


    return result;
}
float Perlin::perlin_noise_3D(float vec[3])
{
    int terms    = mOctaves;
    //float freq   = mFrequency;
    float result = 0.0f;
    float amp = mAmplitude;

    vec[0]*=mFrequency;
    vec[1]*=mFrequency;
    vec[2]*=mFrequency;

    for( int i=0; i<terms; i++ )
    {
        result += noise3(vec)*amp;
        vec[0] *= 2.0f;
        vec[1] *= 2.0f;
        vec[2] *= 2.0f;
        amp*=0.5f;
    }


    return result;
}


Perlin::Perlin(int octaves,float freq,float amp,int seed)
{
    mOctaves = octaves;
    mFrequency = freq;
    mAmplitude = amp;
    mSeed = seed;
    mStart = true;
}



#if IS_OS_MACOSX

#include <CoreServices/CoreServices.h>

// METHODS: MacUserPrefs
// Get Users Preferences folder
//*---------------------------------------------------------------------------*
std::string GetMacUserPrefs()
    //*---------------------------------------------------------------------------*
{
    // ask MacOS to find the users preferences folder (/Users/{username}/Library/Preferences/ on US-english MacOS X)
    // see http://developer.apple.com/samplecode/MoreFilesX/listing2.html
    // and http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html#//apple_ref/c/func/FSFindFolder
    // and http://developer.apple.com/samplecode/FileNotification/listing1.html
    FSRef	fsRef;
    char	path[1024];
    //OSErr	tError =	noErr;

    /*tError =			*/FSFindFolder(kOnAppropriateDisk, kPreferencesFolderType, kDontCreateFolder, &fsRef);
    /*tError =			*/FSRefMakePath(&fsRef, (UInt8*)path, 1024);
    return	std::string(path);
}
std::string GetMacUserHome()
    //*---------------------------------------------------------------------------*
{
    // ask MacOS to find the users preferences folder (/Users/{username}/Library/Preferences/ on US-english MacOS X)
    // see http://developer.apple.com/samplecode/MoreFilesX/listing2.html
    // and http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html#//apple_ref/c/func/FSFindFolder
    // and http://developer.apple.com/samplecode/FileNotification/listing1.html
    FSRef	fsRef;
    char	path[1024];
    //OSErr	tError =	noErr;

    /*tError =			*/FSFindFolder(kOnAppropriateDisk, kCurrentUserFolderType, kDontCreateFolder, &fsRef);
    /*tError =			*/FSRefMakePath(&fsRef, (UInt8*)path, 1024);
    return	std::string(path);
}
#endif

#if IS_OS_LINUX
#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#endif
std::string GetPictureDirectoy()
{
#if IS_OS_LINUX
    passwd *ps = getpwuid(getuid() );
    std::string picdir = ps->pw_dir;
    picdir += "/Pictures";
    return picdir;
#elif IS_OS_MACOSX
    std::string res = GetMacUserHome();
    res += "/Pictures";
    printf("Res picture dir is %s", res.c_str());
    return res;
#elif IS_OS_WINDOWS
    WCHAR szLongPath[_MAX_PATH] = { 0 };
    LPITEMIDLIST pidlDocFiles;
    HRESULT hr = SHGetFolderLocation(NULL,
        CSIDL_MYPICTURES,
        NULL,
        0,
        &pidlDocFiles);

    UNUSED_PARAMETER(hr);

    SHGetPathFromIDListW(pidlDocFiles, szLongPath);
    // Free the ID list allocated by ParseDisplayName
    LPMALLOC pMalloc = NULL;
    SHGetMalloc(&pMalloc);
    pMalloc->Free(pidlDocFiles);
    pMalloc->Release();
    static char imgPath[MAX_PATH];
    sprintf_s(imgPath, MAX_PATH, "%ws",szLongPath);
    return std::string(imgPath);
#endif
}

std::string GetHomeDirectoy()
{
#if IS_OS_LINUX
    passwd *ps = getpwuid(getuid() );
    std::string picdir = ps->pw_dir;
    return picdir;
#elif IS_OS_MACOSX
    return "~/";
#elif IS_OS_WINDOWS
    WCHAR szLongPath[_MAX_PATH] = { 0 };
    LPITEMIDLIST pidlDocFiles;
    HRESULT hr = SHGetFolderLocation(NULL,
        CSIDL_PERSONAL,
        NULL,
        0,
        &pidlDocFiles);

    UNUSED_PARAMETER(hr);

    SHGetPathFromIDListW(pidlDocFiles, szLongPath);
    // Free the ID list allocated by ParseDisplayName
    LPMALLOC pMalloc = NULL;
    SHGetMalloc(&pMalloc);
    pMalloc->Free(pidlDocFiles);
    pMalloc->Release();
    static char imgPath[MAX_PATH];
    sprintf_s(imgPath, MAX_PATH, "%ws",szLongPath);
    strcat(imgPath, "/");
    return std::string(imgPath);
#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//simplex noise

static int grad3[][3] = {{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
    {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
    {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}};
static int grad4[][4]= {{0,1,1,1}, {0,1,1,-1}, {0,1,-1,1}, {0,1,-1,-1},
    {0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
    {1,0,1,1}, {1,0,1,-1}, {1,0,-1,1}, {1,0,-1,-1},
    {-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
    {1,1,0,1}, {1,1,0,-1}, {1,-1,0,1}, {1,-1,0,-1},
    {-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
    {1,1,1,0}, {1,1,-1,0}, {1,-1,1,0}, {1,-1,-1,0},
    {-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}};
static int p[] = {151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};
// To remove the need for index wrapping, float the permutation table length
static int perm[512];// = new int[512];

void initSimplexPerm()
{
    for(int i=0; i<512; i++) perm[i]=p[i & 255];
}
// A lookup table to traverse the simplex around a given point in 4D.
// Details can be found where this table is used, in the 4D noise method.
static int simplex[][4] = {
    {0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
    {0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
    {1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
    {2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0}};
// This method is a *lot* faster than using (int)Math.floor(x)
static int fastfloor(float x) {
    return x>0 ? (int)x : (int)x-1;
}
static float dot(int g[], float x, float y) {
    return g[0]*x + g[1]*y; }
static float dot(int g[], float x, float y, float z) {
    return g[0]*x + g[1]*y + g[2]*z; }
static float dot(int g[], float x, float y, float z, float w) {
    return g[0]*x + g[1]*y + g[2]*z + g[3]*w; }  // 2D simplex noise

static bool simplexInited = false;
float noise(float xin, float yin)
{
    if (!simplexInited)
    {
        initSimplexPerm();
        simplexInited = true;
    }

    float n0, n1, n2; // Noise contributions from the three corners
    // Skew the input space to determine which simplex cell we're in
    float F2 = static_cast<float>(0.5*(sqrt(3.0)-1.0));
    float s = (xin+yin)*F2; // Hairy factor for 2D
    int i = fastfloor(xin+s);
    int j = fastfloor(yin+s);
    float G2 = static_cast<float>((3.0-sqrt(3.0))/6.0);
    float t = (i+j)*G2;
    float X0 = i-t; // Unskew the cell origin back to (x,y) space
    float Y0 = j-t;
    float x0 = xin-X0; // The x,y distances from the cell origin
    float y0 = yin-Y0;
    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else {i1=0; j1=1;}      // upper triangle, YX order: (0,0)->(0,1)->(1,1)
    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    float y1 = y0 - j1 + G2;
    float x2 = static_cast<float>(x0 - 1.0 + 2.0 * G2); // Offsets for last corner in (x,y) unskewed coords
    float y2 = static_cast<float>(y0 - 1.0 + 2.0 * G2);
    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = perm[ii+perm[jj]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1]] % 12;
    int gi2 = perm[ii+1+perm[jj+1]] % 12;
    // Calculate the contribution from the three corners
    float t0 = static_cast<float>(0.5 - x0*x0-y0*y0);
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0);  // (x,y) of grad3 used for 2D gradient
    }
    float t1 = static_cast<float>(0.5 - x1*x1-y1*y1);
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }    float t2 = static_cast<float>(0.5 - x2*x2-y2*y2);
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return static_cast<float>(70.0 * (n0 + n1 + n2));
}
// 3D simplex noise
float noise(float xin, float yin, float zin)
{
    if (!simplexInited)
    {
        initSimplexPerm();
        simplexInited = true;
    }

    float n0, n1, n2, n3; // Noise contributions from the four corners
    // Skew the input space to determine which simplex cell we're in
    float F3 = static_cast<float>(1.0/3.0);
    float s = (xin+yin+zin)*F3; // Very nice and simple skew factor for 3D
    int i = fastfloor(xin+s);
    int j = fastfloor(yin+s);
    int k = fastfloor(zin+s);
    float G3 = static_cast<float>(1.0/6.0); // Very nice and simple unskew factor, too
    float t = (i+j+k)*G3;
    float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j-t;
    float Z0 = k-t;
    float x0 = xin-X0; // The x,y,z distances from the cell origin
    float y0 = yin-Y0;
    float z0 = zin-Z0;
    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
    if(x0>=y0) {
        if(y0>=z0)
        { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
        else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
        else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
    }
    else { // x0<y0
        if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
        else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
        else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
    }
    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = static_cast<float>(x0 - i2 + 2.0*G3); // Offsets for third corner in (x,y,z) coords
    float y2 = static_cast<float>(y0 - j2 + 2.0*G3);
    float z2 = static_cast<float>(z0 - k2 + 2.0*G3);
    float x3 = static_cast<float>(x0 - 1.0 + 3.0*G3); // Offsets for last corner in (x,y,z) coords
    float y3 = static_cast<float>(y0 - 1.0 + 3.0*G3);
    float z3 = static_cast<float>(z0 - 1.0 + 3.0*G3);
    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
    int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
    int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;
    // Calculate the contribution from the four corners
    float t0 = static_cast<float>(0.6 - x0*x0 - y0*y0 - z0*z0);
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }
    float t1 = static_cast<float>(0.6 - x1*x1 - y1*y1 - z1*z1);
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }
    float t2 = static_cast<float>(0.6 - x2*x2 - y2*y2 - z2*z2);
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }
    float t3 = static_cast<float>(0.6 - x3*x3 - y3*y3 - z3*z3);
    if(t3<0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return static_cast<float>(32.0*(n0 + n1 + n2 + n3));
}  // 4D simplex noise
float noise(float x, float y, float z, float w)
{
    if (!simplexInited)
    {
        initSimplexPerm();
        simplexInited = true;
    }


    // The skewing and unskewing factors are hairy again for the 4D case
    float F4 = static_cast<float>((sqrt(5.0)-1.0)/4.0);
    float G4 = static_cast<float>((5.0-sqrt(5.0))/20.0);
    float n0, n1, n2, n3, n4; // Noise contributions from the five corners
    // Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
    float s = (x + y + z + w) * F4; // Factor for 4D skewing
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    int k = fastfloor(z + s);
    int l = fastfloor(w + s);
    float t = (i + j + k + l) * G4; // Factor for 4D unskewing
    float X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
    float Y0 = j - t;
    float Z0 = k - t;
    float W0 = l - t;
    float x0 = x - X0;  // The x,y,z,w distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;
    float w0 = w - W0;
    // For the 4D case, the simplex is a 4D shape I won't even try to describe.
    // To find out which of the 24 possible simplices we're in, we need to
    // determine the magnitude ordering of x0, y0, z0 and w0.
    // The method below is a good way of finding the ordering of x,y,z,w and
    // then find the correct traversal order for the simplex weÕre in.
    // First, six pair-wise comparisons are performed between each possible pair
    // of the four coordinates, and the results are used to add up binary bits
    // for an integer index.
    int c1 = (x0 > y0) ? 32 : 0;
    int c2 = (x0 > z0) ? 16 : 0;
    int c3 = (y0 > z0) ? 8 : 0;
    int c4 = (x0 > w0) ? 4 : 0;
    int c5 = (y0 > w0) ? 2 : 0;
    int c6 = (z0 > w0) ? 1 : 0;
    int c = c1 + c2 + c3 + c4 + c5 + c6;
    int i1, j1, k1, l1; // The integer offsets for the second simplex corner
    int i2, j2, k2, l2; // The integer offsets for the third simplex corner
    int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner
    // simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
    // Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
    // impossible. Only the 24 indices which have non-zero entries make any sense.
    // We use a thresholding to set the coordinates in turn from the largest magnitude.
    // The number 3 in the "simplex" array is at the position of the largest coordinate.
    i1 = simplex[c][0]>=3 ? 1 : 0;
    j1 = simplex[c][1]>=3 ? 1 : 0;
    k1 = simplex[c][2]>=3 ? 1 : 0;
    l1 = simplex[c][3]>=3 ? 1 : 0;
    // The number 2 in the "simplex" array is at the second largest coordinate.
    i2 = simplex[c][0]>=2 ? 1 : 0;
    j2 = simplex[c][1]>=2 ? 1 : 0;    k2 = simplex[c][2]>=2 ? 1 : 0;
    l2 = simplex[c][3]>=2 ? 1 : 0;
    // The number 1 in the "simplex" array is at the second smallest coordinate.
    i3 = simplex[c][0]>=1 ? 1 : 0;
    j3 = simplex[c][1]>=1 ? 1 : 0;
    k3 = simplex[c][2]>=1 ? 1 : 0;
    l3 = simplex[c][3]>=1 ? 1 : 0;
    // The fifth corner has all coordinate offsets = 1, so no need to look that up.
    float x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
    float y1 = y0 - j1 + G4;
    float z1 = z0 - k1 + G4;
    float w1 = w0 - l1 + G4;
    float x2 = static_cast<float>(x0 - i2 + 2.0*G4); // Offsets for third corner in (x,y,z,w) coords
    float y2 = static_cast<float>(y0 - j2 + 2.0*G4);
    float z2 = static_cast<float>(z0 - k2 + 2.0*G4);
    float w2 = static_cast<float>(w0 - l2 + 2.0*G4);
    float x3 = static_cast<float>(x0 - i3 + 3.0*G4); // Offsets for fourth corner in (x,y,z,w) coords
    float y3 = static_cast<float>(y0 - j3 + 3.0*G4);
    float z3 = static_cast<float>(z0 - k3 + 3.0*G4);
    float w3 = static_cast<float>(w0 - l3 + 3.0*G4);
    float x4 = static_cast<float>(x0 - 1.0 + 4.0*G4); // Offsets for last corner in (x,y,z,w) coords
    float y4 = static_cast<float>(y0 - 1.0 + 4.0*G4);
    float z4 = static_cast<float>(z0 - 1.0 + 4.0*G4);
    float w4 = static_cast<float>(w0 - 1.0 + 4.0*G4);
    // Work out the hashed gradient indices of the five simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int ll = l & 255;
    int gi0 = perm[ii+perm[jj+perm[kk+perm[ll]]]] % 32;
    int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1+perm[ll+l1]]]] % 32;
    int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2+perm[ll+l2]]]] % 32;
    int gi3 = perm[ii+i3+perm[jj+j3+perm[kk+k3+perm[ll+l3]]]] % 32;
    int gi4 = perm[ii+1+perm[jj+1+perm[kk+1+perm[ll+1]]]] % 32;
    // Calculate the contribution from the five corners
    float t0 = static_cast<float>(0.6 - x0*x0 - y0*y0 - z0*z0 - w0*w0);
    if(t0<0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad4[gi0], x0, y0, z0, w0);
    }
    float t1 = static_cast<float>(0.6 - x1*x1 - y1*y1 - z1*z1 - w1*w1);
    if(t1<0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad4[gi1], x1, y1, z1, w1);
    }
    float t2 = static_cast<float>(0.6 - x2*x2 - y2*y2 - z2*z2 - w2*w2);
    if(t2<0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad4[gi2], x2, y2, z2, w2);
    }   float t3 = static_cast<float>(0.6 - x3*x3 - y3*y3 - z3*z3 - w3*w3);
    if(t3<0) n3 = 0.0;
    else {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad4[gi3], x3, y3, z3, w3);
    }
    float t4 = static_cast<float>(0.6 - x4*x4 - y4*y4 - z4*z4 - w4*w4);
    if(t4<0) n4 = 0.0;
    else {
        t4 *= t4;
        n4 = t4 * t4 * dot(grad4[gi4], x4, y4, z4, w4);
    }
    // Sum up and scale the result to cover the range [-1,1]
    return static_cast<float>(27.0 * (n0 + n1 + n2 + n3 + n4));
}


