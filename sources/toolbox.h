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

#ifndef TOOLBOX_H__
#define TOOLBOX_H__

#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define OFFSETOF(s,m)                       (size_t(&(((s*)1)->m))-1)
// LOGGED User name

#if IS_OS_WINDOWS
inline std::string GetUserName()
{
	char acUserName[100];
    DWORD nUserName = sizeof(acUserName);
    if (GetUserName(acUserName, &nUserName))
	{
        return acUserName;
    }
	return "Player";
}
#elif IS_OS_LINUX
inline std::string GetUserName()
{
	return getlogin();
}
#elif IS_OS_MACOSX
inline std::string GetUserName()
{
	return getlogin();
}
#endif

std::string GetHomeDirectoy();
std::string GetPictureDirectoy();
// file IO


void GetFilesList(std::vector<std::string>& aList, const char *szPath, const char *szWild, bool bRecurs, bool bDirectoriesInsteadOfFiles, bool bCompletePath);

inline void StringToFile( const char *szFileName, const std::string& str )
{
    FILE *fp = fopen( szFileName, "wb");
    if (fp)
    {
        fwrite( str.c_str(), str.length() , 1, fp );
        fclose( fp );
    }
}

inline std::string FileToString( const char *szFileName )
{
    std::string str;
    FILE *fp = fopen( szFileName, "rb");
    if (fp)
    {
        //disabling warning C4127: conditional expression is constant
        #pragma warning(push)
        #pragma warning(disable:4127)
        while(1)
        #pragma warning(pop)
        {
            int c = getc(fp);
            if ( c == EOF )
                break;
            str+= (char)c;
        }
        str+=(char)0;
        fclose( fp );
    }
    return str;
}


// PROFILE/LOG

#if LOG_ENABLE && USE_LOG_TO_FILE && !IS_OS_MACOSX

static const char *GetLogFileName()
{
	static std::string LogFileName = "";

	if ( LogFileName.empty() )
	{
		LogFileName = GetHomeDirectoy() + "/therush/therushlog.txt";
	}
	return LogFileName.c_str();
}

inline void ClearlogFile()
{
	if (FILE *fp = fopen(GetLogFileName(), "wt") )
		fclose( fp );
}

inline void LOG(const char *format,...)
{
    va_list ptr_arg;
    va_start( ptr_arg, format );

    static char tmps[262144];
    vnsprintf( tmps, sizeof(tmps), format, ptr_arg );

	FILE* fp = fopen(GetLogFileName(),"a+t");
	if (fp)
	{
		fwrite( tmps, strlen(tmps), 1, fp);
		fclose(fp);
	}

    va_end(ptr_arg);
}
#elif LOG_ENABLE

#define LOG debug_printf

#else

#define LOG(fmt, ...)	\
    DECLARE_MACRO_BEGIN \
    (void)sizeof(fmt); \
    DECLARE_MACRO_END

#endif

// Fast Hash

#if !defined (get16bits)
#define get16bits(d) ((((uint32)(((const uint8 *)(d))[1])) << 8)\
+(uint32)(((const uint8 *)(d))[0]) )
#endif
//
// super hash function by Paul Hsieh
//
inline uint32 superFastHash (const char * data, int len) {
	uint32 hash = len, tmp;
	int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
			hash ^= hash << 16;
			hash ^= data[sizeof (uint16)] << 18;
			hash += hash >> 11;
			break;
        case 2: hash += get16bits (data);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
        case 1: hash += *data;
			hash ^= hash << 10;
			hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

#define hashFast(x) superFastHash(x, (sizeof(x)-1) )

#if USE_PROFILER

// Critical Section
#if IS_OS_WINDOWS
typedef CRITICAL_SECTION ZCriticalSection_t;
inline char* ZGetCurrentDirectory(int bufLength, char *pszDest)
{
    return (char*)GetCurrentDirectoryA(bufLength, pszDest);
}

#elif IS_OS_LINUX
#include <pthread.h>
typedef pthread_mutex_t ZCriticalSection_t;
inline char* ZGetCurrentDirectory(int bufLength, char *pszDest)
{
    return getcwd(pszDest, bufLength);
}

#elif IS_OS_MACOSX
#import <CoreServices/CoreServices.h>
typedef MPCriticalRegionID ZCriticalSection_t;
inline char* ZGetCurrentDirectory(int bufLength, char *pszDest)
{
    return getcwd(pszDest, bufLength);
}
#endif


__inline ZCriticalSection_t* NewCriticalSection()
{
#if IS_OS_LINUX
	ZCriticalSection_t *cs = new pthread_mutex_t;
	//(*cs) = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init (cs, NULL);
	return cs;
#elif IS_OS_MACOSX
	MPCriticalRegionID* criticalRegion = new MPCriticalRegionID;
	OSStatus err = MPCreateCriticalRegion (criticalRegion);
	if (err != 0)
	{
		delete criticalRegion;
		criticalRegion = NULL;
	}

	return criticalRegion;
#elif IS_OS_WINDOWS
	CRITICAL_SECTION *cs = new CRITICAL_SECTION;
	InitializeCriticalSection(cs);
	return cs;
#endif
}

__inline void DestroyCriticalSection(ZCriticalSection_t *cs)
{
#if IS_OS_LINUX
	delete cs;
#elif IS_OS_MACOSX
	MPDeleteCriticalRegion(*cs);
#elif IS_OS_WINDOWS
	DeleteCriticalSection(cs);
	delete cs;
#endif
}

__inline void LockCriticalSection(ZCriticalSection_t *cs)
{
#if IS_OS_LINUX
	pthread_mutex_lock( cs );
#elif IS_OS_MACOSX
	MPEnterCriticalRegion(*cs, kDurationForever);
#elif IS_OS_WINDOWS
	EnterCriticalSection(cs);
#endif
}

__inline void UnLockCriticalSection(ZCriticalSection_t *cs)
{
#if IS_OS_LINUX
	pthread_mutex_unlock( cs );
#elif IS_OS_MACOSX
	MPExitCriticalRegion(*cs);
#elif IS_OS_WINDOWS
	LeaveCriticalSection(cs);
#endif
}


bool Zprofiler_enable();
void Zprofiler_disable();
void Zprofiler_start( const char *profile_name );
void Zprofiler_end( );
void LogProfiler();

//defines

#define PROFILER_ENABLE Zprofiler_enable()
#define PROFILER_DISABLE Zprofiler_disable()
#define PROFILER_START(x) Zprofiler_start(QUOTE(x))
#define PROFILER_END() Zprofiler_end()

#else

#define LogProfiler()

#define PROFILER_ENABLE
#define PROFILER_DISABLE
#define PROFILER_START(x)
#define PROFILER_END()

#endif  //  USE_PROFILER

typedef int TriangleList[3];
int Delaunay(int N, vec_t *vts, int& numTriangles, TriangleList*& triangle);

inline bool seg2seg(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Dx, float Dy, vec_t& res)
{
#define THRES 0.0001
#define INF(x,y) ((x<y)&&(fabsf(x-y)>THRES))
#define SUP(x,y) ((x>y)&&(fabsf(x-y)>THRES))

	float Sx;
    float Sy;

    if(fabsf(Ax - Bx) < THRES)
    {
        if(fabsf(Cx - Dx) < THRES) return false;
        else
        {
            float pCD = (Cy-Dy)/(Cx-Dx);
            Sx = Ax;
            Sy = pCD*(Ax-Cx)+Cy;
        }
    }
    else
    {
        if(fabsf(Cx - Dx) < THRES)
        {
            float pAB = (Ay-By)/(Ax-Bx);
            Sx = Cx;
            Sy = pAB*(Cx-Ax)+Ay;
        }
        else
        {
            float pCD = (Cy-Dy)/(Cx-Dx);
            float pAB = (Ay-By)/(Ax-Bx);
            float oCD = Cy-pCD*Cx;
            float oAB = Ay-pAB*Ax;
            Sx = (oAB-oCD)/(pCD-pAB);
            Sy = pCD*Sx+oCD;
        }
    }
    /*
	float v1 = Sy - Ay;
	float v2 = Sy - By;
	bool b1 = INF(Sy, Ay);
	bool b2 = INF(Sy, By);
	bool b3 = SUP(Sy, Ay);
	bool b4 = SUP(Sy, By);
     */
    if((INF(Sx, Ax) && INF(Sx, Bx))||(SUP(Sx, Ax) && SUP(Sx, Bx)) || (INF(Sx, Cx) && INF(Sx, Dx))||(SUP(Sx, Cx) && SUP(Sx, Dx))
       || (INF(Sy, Ay) && INF(Sy, By))||(SUP(Sy, Ay) && SUP(Sy, By)) || (INF(Sy, Cy) && INF(Sy, Dy))||(SUP(Sy, Cy) && SUP(Sy, Dy))) return false;

    res = vec(Sx, Sy, 0.f, 0.f);
    return true; //or :     return new Point2D.Float((float)Sx,(float)Sy)
}

inline bool segment2segmentXZ(vec_t A,vec_t B,vec_t C,vec_t D, vec_t &res)
{
    float Ax = A.x;
    float Ay = A.z;
    float Bx = B.x;
    float By = B.z;
    float Cx = C.x;
    float Cy = C.z;
    float Dx = D.x;
    float Dy = D.z;

    bool ret = seg2seg( Ax,  Ay,  Bx,  By,  Cx,  Cy,  Dx,  Dy,  res);
	res = vec(res.x, 0.f, res.y, 0.f);
	return ret;
}

inline bool segment2segmentXY(vec_t A,vec_t B,vec_t C,vec_t D, vec_t &res)
{
    float Ax = A.x;
    float Ay = A.y;
    float Bx = B.x;
    float By = B.y;
    float Cx = C.x;
    float Cy = C.y;
    float Dx = D.x;
    float Dy = D.y;

    return seg2seg( Ax,  Ay,  Bx,  By,  Cx,  Cy,  Dx,  Dy,  res);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------
// Name  : intersectRaySphere()
// Input : rO - origin of ray in world space
//         rV - vector describing direction of ray in world space
//         sO - Origin of sphere
//         sR - radius of sphere
// Notes : Normalized directional vectors expected
// Return: distance to sphere in world units, -1 if no intersection.
// -----------------------------------------------------------------------

inline float IntersectRaySphere(const vec_t & rO, const vec_t & rV, const vec_t & sO, float sR)
{

	vec_t Q = sO-rO;

	//float c = Q.Length();
	float v = Q.dot(rV);
	float d = sR*sR - (Q.lengthSq() - v*v);

	// If there was no intersection, return -1
	if (d < 0.0f)
	{
		return (-1.0f);
	}

	// Return the distance to the [first] intersecting point
	return (v - sqrtf(d));
}


inline float IntersectRaySphere(const vec_t & rO, const vec_t & rV, const vec_t & sphere)
{

	vec_t Q = sphere-rO;

	//float c = Q.Length();
	float v = Q.dot(rV);
	float d = sphere.w*sphere.w - (Q.lengthSq() - v*v);

	// If there was no intersection, return -1
	if (d < 0.0f)
	{
		return (-1.0f);
	}

	// Return the distance to the [first] intersecting point
	return (v - sqrtf(d));
}

inline std::string formatTime(float timeInSeconds)
{
	char tmps[512];
	int racingMinutes = (int)floorf( timeInSeconds/60.f );
	int racingSeconds = (int)floorf( fmodf( timeInSeconds, 60.f ) );
	int racingFrac = (int)(fmodf( timeInSeconds, 1.f ) * 100.f );

	sprintf(tmps, "%2d\"%02d\'%02d", racingMinutes, racingSeconds, racingFrac);
	return tmps;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
inline float IntersectRayPlane(const tvector3 & rOrigin, const tvector3& rVector, const tvector3& pOrigin, const tvector3 & pNormal)
{

  float d = - pNormal.Dot(pOrigin);

  float numer = pNormal.Dot(rOrigin) + d;
  float denom = pNormal.Dot(rVector);

  if (denom == 0)  // normal is orthogonal to vector, cant intersect
  {
      return (-1.0f);
  }

  return -(numer / denom);
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
// rorigin.w and rVector.w must be 0
inline float IntersectRayPlane(const vec_t & rOrigin, const vec_t& rVector, const vec_t& plan )
{

//  float d = - pNormal.Dot(pOrigin);

  float numer = plan.dot(rOrigin) - plan.w;
  float denom = plan.dot(rVector);

  if (denom == 0)  // normal is orthogonal to vector, cant intersect
  {
      return (-1.0f);
  }

  return -(numer / denom);
}

int OptimizeMesh( int compareSize, u8* vertices, int vtCount, int vtSize, unsigned short *indices, int indicesCount );

///////////////////////////////////////////////////////////////////////////////////////////////////
// simplex

float noise( float x, float y, float z, float w );
float noise( float x, float y, float z );
float noise( float x, float y );

///////////////////////////////////////////////////////////////////////////////////////////////////
// web

#ifndef RETAIL
#include <map>

void startWeb( void(*resetWeb)() );

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#define EPSILON 0.000001
#define CROSS(dest,v1,v2) \
    dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
    dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
    dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
    dest[0]=v1[0]-v2[0]; \
    dest[1]=v1[1]-v2[1]; \
    dest[2]=v1[2]-v2[2];

inline int intersect_triangle(float orig[3], float dir[3],
                   float vert0[3], float vert1[3], float vert2[3],
                   float *t, float *u, float *v)
{
    float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
    float det,inv_det;

    /* find vectors for two edges sharing vert0 */
    SUB(edge1, vert1, vert0);
    SUB(edge2, vert2, vert0);

    /* begin calculating determinant - also used to calculate U parameter */
    CROSS(pvec, dir, edge2);

    /* if determinant is near zero, ray lies in plane of triangle */
    det = DOT(edge1, pvec);

#ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
    if (det < EPSILON)
        return 0;

    /* calculate distance from vert0 to ray origin */
    SUB(tvec, orig, vert0);

    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec);
    if (*u < 0.0 || *u > det)
        return 0;

    /* prepare to test V parameter */
    CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec);
    if (*v < 0.0 || *u + *v > det)
        return 0;

    /* calculate t, scale parameters, ray intersects triangle */
    *t = DOT(edge2, qvec);
    inv_det = 1.0 / det;
    *t *= inv_det;
    *u *= inv_det;
    *v *= inv_det;
#else                    /* the non-culling branch */
    if (det > -EPSILON && det < EPSILON)
        return 0;
    inv_det = 1.0f / det;

    /* calculate distance from vert0 to ray origin */
    SUB(tvec, orig, vert0);

    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec) * inv_det;
    if (*u < 0.0 || *u > 1.0)
        return 0;

    /* prepare to test V parameter */
    CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec) * inv_det;
    if (*v < 0.0 || *u + *v > 1.0)
        return 0;

    /* calculate t, ray intersects triangle */
    *t = DOT(edge2, qvec) * inv_det;
#endif
    return 1;
}



#define SAMPLE_SIZE 1024

class Perlin
{
public:

  Perlin(int octaves,float freq,float amp,int seed);


  float Get(float x,float y)
  {
    float vec[2];
    vec[0] = x;
    vec[1] = y;
    return perlin_noise_2D(vec);
  };

  float Get(float x,float y, float z)
  {
    float vec[3];
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    return perlin_noise_3D(vec);
  };
private:
  void init_perlin(int n,float p);
  float perlin_noise_2D(float vec[2]);
  float perlin_noise_3D(float vec[3]);

  float noise1(float arg);
  float noise2(float vec[2]);
  float noise3(float vec[3]);
  void normalize2(float v[2]);
  void normalize3(float v[3]);
  void init(void);

  int   mOctaves;
  float mFrequency;
  float mAmplitude;
  int   mSeed;

  int p[SAMPLE_SIZE + SAMPLE_SIZE + 2];
  float g3[SAMPLE_SIZE + SAMPLE_SIZE + 2][3];
  float g2[SAMPLE_SIZE + SAMPLE_SIZE + 2][2];
  float g1[SAMPLE_SIZE + SAMPLE_SIZE + 2];
  bool  mStart;

};


template<typename T, int reservedElements> struct FastArray
{
    FastArray()
    {
        Clear();
    }
    void Clear()
    {
        mNbElts = 0;

        COMPILE_TIME_ASSERT( sizeof( elements) == sizeof(T) * reservedElements );
        memset( (void*)&elements[0], 0, sizeof( elements) );
    }
    T* Push() { ASSERT_GAME( mNbElts < reservedElements ); return &elements[mNbElts++]; }
    T* Pop() { ASSERT_GAME( mNbElts > 0 ); return &elements[(--mNbElts)]; }
    T* PushCapped() { if ( mNbElts >= reservedElements ) return NULL; return &elements[mNbElts++]; }
    const T* GetFirst() const { return &elements[0]; }
    T* GetFirstToModify() { return &elements[0]; }
    const T* GetLast() const { return &elements[mNbElts-1]; }
    T* GetLastToModify() { return &elements[mNbElts-1]; }
    const T* GetArrayEnd() const { return &elements[reservedElements]; }
    const T* GetEndUsed() const { return &elements[mNbElts]; }

    bool Contains( const T& item  ) const
    {
        const unsigned int itemCount = mNbElts;
        for (unsigned int i = 0 ; i < itemCount; ++i )
        {
            if ( elements[i] == item )
            {
                return true;
            }
        }

        return false;
    }

    void Erase( T* item )
    {
        if (item != &elements[mNbElts-1])
            *item = elements[mNbElts-1];
        mNbElts--;
    }

    void Erase( const T& item )
    {
        for (unsigned int i = 0 ; i < mNbElts ; i++ )
        {
            if ( elements[i] == item)
            {
                Erase( i );
                return;
            }
        }

    }

    void Erase( unsigned int idx )
    {
        if ( idx != (mNbElts-1) )
            elements[idx] = elements[mNbElts-1];
        mNbElts--;
    }

    bool Empty() const { return (mNbElts == 0); }

    unsigned int Size() { return mNbElts; }
    T& operator [](const int idx) { return elements[idx]; }
protected:
    unsigned int mNbElts;
    T elements[reservedElements];
};


template<typename T, int reservedElements> struct ArrayPool
{
    ArrayPool()
    {
        COMPILE_TIME_ASSERT( reservedElements > 2 );
        //printf("Size of pool is %d\n", sizeof(*this));
        ComputeLinks();

        extern unsigned int mNumberOfArrayPools, mTotalBytesInArrayPools;
        mNumberOfArrayPools ++;
        mTotalBytesInArrayPools += sizeof(*this);
        mNbUsedItems = 0;
    }
    ~ArrayPool()
    {
        //ASSERT( (!firstUsed) );
        extern unsigned int mNumberOfArrayPools, mTotalBytesInArrayPools;
        mNumberOfArrayPools --;
        mTotalBytesInArrayPools -= sizeof(*this);
    }

    void ForceClear()
    {
        while (poolElt* elt = GetFirst())
            Delete( elt );
    }

    struct poolElt : public T
    {

        poolElt* GetNext() const
        {
#ifdef _DEBUG
            ASSERT_GAME ( (mThis == this) );
#endif
            return mNext;
        }

        poolElt* operator ++ ()
        {
            return mNext;
        }

protected:
#ifdef _DEBUG
        poolElt* mThis;
        bool isFree;
#endif
        poolElt* mNext;
        poolElt* mPrevious;
        friend struct ArrayPool;
    };

    poolElt* GetFirst() const { return firstUsed; }
    poolElt* GetFirstFree() const { return firstFree; }

    T* New()
    {
        ASSERT_GAME( firstFree );
#ifdef _DEBUG
        ASSERT_GAME ( (firstFree->isFree) );
#endif
        poolElt *iter = firstFree;

        if ( iter->mNext )
            iter->mNext->mPrevious = NULL;

        firstFree = iter->mNext;

        iter->mNext = firstUsed;

        if ( firstUsed )
            firstUsed->mPrevious = iter;

        firstUsed = iter;

        mNbUsedItems ++;
#ifdef _DEBUG
        iter->isFree = false;
        CheckConsistence();
#endif
        return iter;
    }

    bool empty() { return (firstUsed == NULL); }

    poolElt* Delete( T* pElement )
    {
        poolElt *iter = (poolElt*)pElement;
        poolElt *res = iter->GetNext();
#ifdef _DEBUG
        ASSERT_GAME ( (iter->mThis == iter) );
        ASSERT_GAME ( (!iter->isFree) );
#endif
        if (iter->mPrevious)
            iter->mPrevious->mNext = iter->mNext;
        if (iter->mNext)
            iter->mNext->mPrevious = iter->mPrevious;

        if ( iter == firstUsed )
            firstUsed = iter->mNext;

        iter->mPrevious = NULL;
        iter->mNext = firstFree;
        if ( firstFree )
            firstFree->mPrevious = iter;

        firstFree = iter;

        mNbUsedItems --;

#ifdef _DEBUG
        iter->isFree = true;
        CheckConsistence();
#endif
        return res;

    }
    unsigned int GetUsedItemsCount() const { return mNbUsedItems; }
    bool hasFreeElements() const { return (firstFree != NULL); }
protected:
    void ComputeLinks()
    {
        firstUsed = NULL;
        firstFree = elements;
        firstFree[0].mPrevious = NULL;
        firstFree[0].mNext = &firstFree[1];
#ifdef _DEBUG
        firstFree[0].mThis = &firstFree[0];
        firstFree[0].isFree = true;
#endif
        for (int i=1;i<reservedElements-1;i++)
        {
            firstFree[i].mPrevious = &firstFree[i-1];
            firstFree[i].mNext     = &firstFree[i+1];
#ifdef _DEBUG
            firstFree[i].mThis = &firstFree[i];
            firstFree[i].isFree = true;
#endif

        }
        firstFree[reservedElements-1].mPrevious = &firstFree[reservedElements-2];
        firstFree[reservedElements-1].mNext = NULL;
#ifdef _DEBUG
        firstFree[reservedElements-1].mThis = &firstFree[reservedElements-1];
        firstFree[reservedElements-1].isFree = true;
#endif

    }
#ifdef _DEBUG
    void CheckConsistence()
    {
        int nbElt = 0;
        for ( poolElt *iter = firstUsed; iter; iter=iter->GetNext() )
            nbElt ++;

        ASSERT_GAME( (nbElt == static_cast<int>(GetUsedItemsCount())) );

        for ( poolElt *iter = firstFree; iter; iter=iter->GetNext() )
            nbElt ++;

        ASSERT_GAME( (nbElt == reservedElements) );
    }
#endif

    poolElt *firstUsed;
    poolElt *firstFree;

    poolElt elements[reservedElements];
    unsigned int mNbUsedItems;

};
// debug infos
inline unsigned int GetNumberOfArrayPools() { extern unsigned int mNumberOfArrayPools; return mNumberOfArrayPools; }
inline unsigned int GetNumberOfBytesInArrayPools() { extern unsigned int mTotalBytesInArrayPools; return mTotalBytesInArrayPools; }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Serialization

// types enum
enum TypeID
{
    TYPE_UNDEFINED,
    TYPE_FLOAT,
    TYPE_INT,
    TYPE_STRING,
    TYPE_BIGSTRING,
    TYPE_BOOLEAN,
    TYPE_OBJECT,
    TYPE_OBJECT_PTR,
    TYPE_VECT,
    TYPE_U32,
	TYPE_MATRIX
};

enum SERILIZATION_FLAGS
{
    SF_NORMAL = 1,
    SF_TRANSIENT = 1<<1, // can be edited, is not serialized
    SF_NOEDIT = 1<<2, // serialized, not editable
    SF_EXPAND = 1<<3, // object is expanded (no HT link) in the editing process. works for in-array elements
    SF_ERRORLOG = 1<<4
};

// base type
template <typename TYPE> inline TypeID GetTypeID() { return TYPE_UNDEFINED; }

struct serialisableObject_t;
struct serializableField_t;

// Specialisation examples
template <> inline TypeID GetTypeID<int>() { return TYPE_INT; }
template <> inline TypeID GetTypeID<float>() { return TYPE_FLOAT; }
template <> inline TypeID GetTypeID<std::string>() { return TYPE_STRING; }
template <> inline TypeID GetTypeID<bool>() { return TYPE_BOOLEAN; }
template <> inline TypeID GetTypeID<vec_t>() { return TYPE_VECT; }
template <> inline TypeID GetTypeID<u32>() { return TYPE_U32; }
template <> inline TypeID GetTypeID<matrix_t>() { return TYPE_MATRIX; }

// container base
struct serializableContainer_t
{
    virtual int GetCount(void* container) const = 0;
    virtual void* GetValue(void* container, int index) = 0;
    virtual void* Push( void* container, int idx ) = 0;
};

// container specific
template <typename TYPE> struct serializableContainerVector : public serializableContainer_t
{
    int GetCount( void* container ) const { return ((std::vector<TYPE>*)container)->size(); }
    void* GetValue(void* container, int index) { return &((std::vector<TYPE>*)container)->at(index); }
    void *Push( void* container, int idx )
    {
        if ( idx < this->GetCount( container ) )
            return this->GetValue( container, idx ); // return pointeur to value if we already have the space for

        ((std::vector<TYPE>*)container)->push_back(TYPE());
        return this->GetValue( container, this->GetCount( container )-1 );
    }
};

template <typename TYPE, int itemsCount> struct serializableContainerStaticArray : public serializableContainer_t
{
    int GetCount( void* container ) const { UNUSED_PARAMETER(container); return itemsCount; }
    void* GetValue(void* container, int index) { return &((TYPE*)container)[index]; }
    void *Push( void* container, int idx ) { return &((TYPE*)container)[idx]; }
};


// base class
typedef struct serialisableObject_t
{
    virtual const serializableField_t * GetFields() const = 0;
    virtual int GetNbFields() const = 0;
    virtual const char *GetTypeName() const = 0;
    virtual void FieldsHasBeenModified() {} // flag your object as dirty/need update
    virtual bool ObjectIsDirty() const { return false; } // object is still dirty/needs update
    const serializableField_t * GetFieldByName( const char *szMemberName ) const ;
    const char *GetVariableName( serialisableObject_t *pObj ) const;
    virtual const char *GetDescription() const = 0;
    u8 * GetFieldPointer( const serializableField_t * pf, int idx) const;
    virtual const char *GetObjectName() const { return NULL; }
} serialisableObject_t;

// check if type is a pointer
template<typename T> struct is_pointer2 { static const bool value = false; };
template<typename T> struct is_pointer2<T*> { static const bool value = true; };

// type is an array ... or not
template<typename T> struct isArray { static const int cnt = 1; };
template<typename T, size_t N> struct isArray<T[N]> { static const int cnt = N; };

// is sub class
template<typename BaseT, typename DerivedT>
struct IsRelated
{
    static DerivedT derived();

    static char test(const BaseT&); // sizeof(test()) == sizeof(char)
    static char (&test(...))[2];    // sizeof(test()) == sizeof(char[2])
    enum { exists = (sizeof(test(derived())) == sizeof(char)) };
};



// serializable field
struct serializableField_t
{
    // default
    template <typename OBJECT_TYPE, typename FIELD_TYPE> serializableField_t(const char*_name, FIELD_TYPE (OBJECT_TYPE::*field), size_t ofs, u32 _flags=0, const char * szDescription="") :
        name(_name), offset(ofs), container(NULL), description(szDescription), flags(_flags), isPointer(is_pointer2<FIELD_TYPE>::value)
    {
        UNUSED_PARAMETER(field);

        type = isPointer?TYPE_OBJECT_PTR:IsRelated<serialisableObject_t,FIELD_TYPE>::exists?TYPE_OBJECT:GetTypeID<FIELD_TYPE>(); // ouch!

        //disabling warning C4127: conditional expression is constant
        //disabling warning C6326: potential comparison of a constant with another constant
        #pragma warning(push)
        #pragma warning(disable:4127)
        #pragma warning(disable:6326)
        if ( isArray<FIELD_TYPE>::cnt > 1 )
        #pragma warning(pop)
        {
            container = new serializableContainerStaticArray<FIELD_TYPE, isArray<FIELD_TYPE>::cnt>;
        }
    }

    // for contained types
    template <typename OBJECT_TYPE, typename FIELD_TYPE> serializableField_t(const char*_name, std::vector<FIELD_TYPE> (OBJECT_TYPE::*field), size_t ofs, u32 _flags=0, const char * szDescription="" ) :
        type(GetTypeID<FIELD_TYPE>() ), name(_name), offset(ofs), container( new serializableContainerVector<FIELD_TYPE>() ), description(szDescription), flags(_flags) { UNUSED_PARAMETER(field); }

    template <typename OBJECT_TYPE, typename FIELD_TYPE, size_t cnt> serializableField_t(const char*_name, FIELD_TYPE (OBJECT_TYPE::*field)[cnt], size_t ofs, u32 _flags=0, const char * szDescription="" ) :
        type(GetTypeID<FIELD_TYPE>() ), name(_name), offset(ofs), container( new serializableContainerStaticArray<FIELD_TYPE, cnt>() ), description(szDescription), flags(_flags) { UNUSED_PARAMETER(field); }



    TypeID type;
    const char* name;
    size_t offset;
    serializableContainer_t *container;
    const char *description;
    u32 flags;
    bool isPointer;
} ;



// objects

#define SERIALIZABLE(x,y) virtual const serializableField_t * GetFields() const { return mSerialisableFields; }\
                        virtual int GetNbFields() const { return mNbFields; } \
                        virtual const char *GetTypeName() const { return QUOTE(x); } \
                        virtual const char *GetDescription() const { return y; } \
                        static serializableField_t mSerialisableFields[]; \
                        static int mNbFields;

#define NBFIELDS(x) int x::mNbFields = sizeof(x::mSerialisableFields)/sizeof(serializableField_t);
#define SE(cls,mbr) serializableField_t(QUOTE(mbr),&cls::mbr, offsetof(cls,mbr))
#define SEF(cls,mbr,flag) serializableField_t(QUOTE(mbr),&cls::mbr, offsetof(cls,mbr), flag)
#define SEFD(cls,mbr,flag, desc) serializableField_t(QUOTE(mbr),&cls::mbr, offsetof(cls,mbr), flag, desc)
#define SED(cls,mbr, desc) serializableField_t(QUOTE(mbr),&cls::mbr, offsetof(cls,mbr), 0, desc)

// read/write
#ifndef RETAIL
std::string GenerateBinary( serialisableObject_t *pObj );
#endif

int ParseBinary( serialisableObject_t *pObj, const char *pBits );

#endif
