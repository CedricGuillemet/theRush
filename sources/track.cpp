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
#include "track.h"

#include "mesh.h"
#include "camera.h"
#include "physics.h"
#include "content.h"
#include "render.h"

Track track;
extern shapePattern_t shapePatterns[];

extern int g_seed;
// trackSeg_t


serializableField_t trackSeg_t::mSerialisableFields[] = {
    SE( trackSeg_t, pt ),
    SE( trackSeg_t, mbHasFourche ),
    SE( trackSeg_t, point2 ),
    SE( trackSeg_t, up ),
    SE( trackSeg_t, width),
    SE( trackSeg_t, border)

};
NBFIELDS(trackSeg_t);

// track

serializableField_t track_t::mSerialisableFields[] = {
        SE(track_t,mName),
        SED(track_t,trackLength,"Total length, in meters."),
        SED(track_t,mbOnlyDefaultBrick,"Only default brick."),
        SED(track_t,nbHoles, "Diving board count."),
        SE(track_t,holes),
        SE(track_t,mBonusGenerationParameters),
        SE(track_t,mSkyScatteringParameters),
        SED(track_t, color1,"Track tone color 1"),
        SED(track_t, color2,"Track tone color 2"),
        SED(track_t, color3,"Track tone color 3"),
        SED(track_t, color4,"Track tone color 4"),
        SED(track_t, color5,"Track tone color 5"),
        SED(track_t, mEnvironmentIndex, "Env Index. 0 = ice floes 1 = city"),
        SED(track_t, mRockGradiant, "rock gradiant 0-7"),
		SED(track_t, mPrefabs, "prefabs"),
        SEF(track_t, segs, SF_NOEDIT)
};


NBFIELDS(track_t);

// hole_t

serializableField_t hole_t::mSerialisableFields[] = {
        SED(hole_t,holeStartDistance,"Distance from the track start to the hole."),
        SED(hole_t,holeLength,"Hole length, in meters.")
};

NBFIELDS(hole_t);

// prefab
serializableField_t prefabInstance_t::mSerialisableFields[] = {
        SE(prefabInstance_t,mPrefabFile),
        SE(prefabInstance_t,mPrefabMatrix)
};

NBFIELDS(prefabInstance_t);

// skyScatParams_t

serializableField_t skyScatParams_t::mSerialisableFields[] = {SE(skyScatParams_t,mKr),
    	                                                        SE(skyScatParams_t,mSunDirection),
	                                                            SE(skyScatParams_t,mSunColor),
                                                               SE(skyScatParams_t,mRayleighBrightness),
                                                               SE(skyScatParams_t,mMieBrightness),
                                                               SE(skyScatParams_t,mScatterStrength),
                                                               SE(skyScatParams_t,mSpotBrightness),
                                                               SE(skyScatParams_t,mRayleighStrength),
                                                               SE(skyScatParams_t,mMieStrength),
                                                               SE(skyScatParams_t,mRayleighCollectionPower),
                                                               SE(skyScatParams_t,mMieCollectionPower),
                                                               SE(skyScatParams_t,mMieDistribution),
                                                                  SE(skyScatParams_t, mFogDensity),
                                                                SE(skyScatParams_t, mFogColor),
                                                                SE(skyScatParams_t, mFarPlane),
                                                                SE(skyScatParams_t,mbDrawOcean)

                                                              };
NBFIELDS(skyScatParams_t);

// ai points
AIPoint_t::AIPoint_t(const vec_t& aIPos, const vec_t& aIRight, const vec_t& aIUp, const vec_t& aIDir, float width )
{
    topology[0] = topology[1] = TrackTopology::TOPO_USED;
    mAIPos[0] = aIPos;
    mAIRight[0] = aIRight;
    mAIUp[0] = aIUp;
    mAIDir[0] = aIDir;

    vec_t railLeft = aIPos + ( aIRight * width );
    vec_t railRight = aIPos + ( aIRight * (-width) );


    mRailLeft[0] = railLeft;
    mRailRight[0] = railRight;

    mAIPos[1] = aIPos;
    mAIRight[1] = aIRight;
    mAIUp[1] = aIUp;
    mAIDir[1] = aIDir;
    mRailLeft[1] = railLeft;
    mRailRight[1] = railRight;



    mAIwidth[0] = mAIwidth[1] = width;

    hasFourche = false;
}
AIPoint_t::AIPoint_t(const vec_t& aIPos, const vec_t& aIRight, const vec_t& aIUp, const vec_t& aIDir, const vec_t& railLeft, const vec_t& railRight, float width,
          const vec_t& aIPos2, const vec_t& aIRight2, const vec_t& aIUp2, const vec_t& aIDir2, const vec_t& railLeft2, const vec_t& railRight2, float width2)
{
    topology[0] = topology[1] = TrackTopology::TOPO_USED;

    mAIPos[0] = aIPos;
    mAIRight[0] = aIRight;
    mAIUp[0] = aIUp;
    mAIDir[0] = aIDir;
    mRailLeft[0] = railLeft;
    mRailRight[0] = railRight;

    hasFourche = true;

    mAIPos[1] = aIPos2;
    mAIRight[1] = aIRight2;
    mAIUp[1] = aIUp2;
    mAIDir[1] = aIDir2;
    mRailLeft[1] = railLeft2;
    mRailRight[1] = railRight2;

    mAIwidth[0] = width;
    mAIwidth[1] = width2;
}



// Track Line --------------------------------------------------------------------------------------

track_t::track_t()
{
    mEnvironmentIndex = 0;
    nbHoles = 0;
    for (int i = 0 ; i < (sizeof(holes)/sizeof(hole_t)) ; i++ )
    {
        holes[i].holeStartDistance = holes[i].holeLength = 0.f;
    }
    nbFourches = 0;
    trackLength = 3000.f;
    mDefaultShapePattern = &shapePatterns[3/*20*/];

    color1 = 0xFFF7D99D;
    color2 = 0xFFB9A986;
    color3 = 0xFFA07DA3;
    color4 = 0xFFFBE4B6;
    color5 = 0xFFFBEAC9;

    mbIsDirty = false;
    mbOnlyDefaultBrick = false;
    mRockGradiant = 0;
}

void track_t::BuildT0(float length)
{
	float radius = length / (2.f*PI);

	for (int i=0;i<NBSEGS;i++)
	{
		float ng = ((2*PI)/(float)(NBSEGS)) * float(i);

		segs[i].pt = vec( cosf(ng) * radius, 0.f, sinf(ng) * radius);
		segs[i].force = vec_t::zero;

		segs[i].force2 = vec_t::zero;
		segs[i].point2 = segs[i].pt;
		segs[i].mbHasFourche = false;

        segs[i].up[0] = vec( 0.f, 1.f, 0.f );
        segs[i].up[1] = vec( 0.f, 1.f, 0.f );

        segs[i].width[0] = segs[i].width[1] = 12.f;
	}

    //FIXME: hardcoded array size for 'fourches' should be replaced with const / enum
    ASSERT_GAME( 0 <= nbFourches && nbFourches <= 5 );

	for (int i = 0 ; i < nbFourches ; ++i)
	{
		for (int j = fourches[i].start ; j < fourches[i].end ; ++j)
        {
            ASSERT_GAME( 0 <= j  && j < NBSEGS );
			segs[j].mbHasFourche = true;
        }
	}
}

vec_t NoiseFactor = vec(1.f, 0.04f, 1.0f);
vec_t randVect()
{
	vec_t res = vec( r01(), r01(), r01() );
	res *= vec( 1.f/255.f, 1.f/255.f, 1.f/255.f);
	res -=vec(0.5f, 0.5f, 0.5f);
	res.normalize();
	//res *= NoiseFactor;

	return res;
}

void track_t::ApplyPressures(int segSize, int decal, float strength)
{
	int segPos = (segSize>>1) + decal;
	if (segPos < NBSEGS)
	{

		vec_t press = randVect()* strength;

		segs[segPos].pressure = press;


		vec_t press2 = randVect()* strength;
		segs[segPos].pressure2 = press2;

	}
	//printf("I %d \n", (segSize>>1) + decal );

	if (segSize == 1)
		return;
	ApplyPressures(segSize>>1, decal, strength*0.35f);
	ApplyPressures(segSize>>1, decal+(segSize>>1)-1, strength*0.35f);


}

void track_t::ApplyPressureModifier()
{
	for(int i = 0 ;i < NBSEGS ; i++)
	{
		segs[i].force += (segs[i].pressure*NoiseFactor);
		segs[i].force2 += (segs[i].pressure2*NoiseFactor);
	}
}


void track_t::ApplyMaxCurveModifier()
{
	for(int i = 0 ;i < NBSEGS ; i++)
	{
		int idx = i;
		int idxPrev = (i-1+NBSEGS)%NBSEGS;
		int idxNext = (i+1)%NBSEGS;

		vec_t vecNext = segs[idxNext].pt - segs[idx].pt;
		vec_t vecPrev = segs[idx].pt - segs[idxPrev].pt;
		vecNext.normalize();
		vecPrev.normalize();

		float dot = vecPrev.dot(vecNext);
		if (dot< 0.3f)
		{
			vec_t forcefc = ( (segs[idxNext].pt + segs[idxPrev].pt)*0.5f ) - segs[idx].pt;
			segs[i].force += forcefc * 0.2f;
		}
	}
}

void track_t::ApplyMaxSlopeModifier(float aSlope)
{
    UNUSED_PARAMETER(aSlope);

	for(int i = 0 ;i < NBSEGS ; i++)
	{
		int idx1 = i;
		int idx2 = (i+1)%NBSEGS;

		vec_t dif = segs[idx2].pt-segs[idx1].pt;
		dif.normalize();
		/*
		 if (dif.y<-aSlope)
		 segs[i].force += vec(0,-dif.y-aSlope,0)*5.f;
		 else if (dif.y>aSlope)
		 segs[i].force -= vec(0,dif.y-aSlope,0)*5.f;
		 */
	}
}

void recalDistByForce(const vec_t &pos1, const vec_t &pos2, float idealDist, vec_t &force1, vec_t &force2)
{
	vec_t dif = pos2-pos1;
	float len = dif.length();
	if (len> FLOAT_EPSILON)
	{
		dif *= (1.f/len);
		vec_t force = dif * (idealDist-len) * 0.5f;
		force2 += force;
		force1 -= force;
	}
}

void recalDistByForce2(const vec_t &pos1, const vec_t &pos2, float idealDist, vec_t &force1, vec_t &force2)
{
    UNUSED_PARAMETER(force1);

	vec_t dif = pos2-pos1;
	float len = dif.length();
	if (len> FLOAT_EPSILON)
	{
		dif *= (1.f/len);
		vec_t force = dif * (idealDist-len);
		force2 += force;
		//		force1 -= force;
	}
}

void recalDistByForce3(const vec_t &pos1, const vec_t &pos2, float idealDist, vec_t &force1, vec_t &force2)
{
    UNUSED_PARAMETER(force2);

	vec_t dif = pos2-pos1;
	float len = dif.length();
	if (len> FLOAT_EPSILON)
	{
		dif *= (1.f/len);
		vec_t force = dif * (idealDist-len);
		//force2 += force;
		force1 -= force;
	}
}
void track_t::ApplyDistModifier(float dist)
{
	for(int i = 0 ;i < NBSEGS ; i++)
	{
		int idx1 = i;
		int idx2 = (i+1)%NBSEGS;


		recalDistByForce(segs[idx1].pt, segs[idx2].pt, dist, segs[idx1].force, segs[idx2].force);


		if ( (!segs[idx1].mbHasFourche)&& (segs[idx2].mbHasFourche))
			recalDistByForce2(segs[idx1].pt, segs[idx2].point2, dist, segs[idx1].force, segs[idx2].force2);

		recalDistByForce(segs[idx1].point2, segs[idx2].point2, dist, segs[idx1].force2, segs[idx2].force2);

		if ( (segs[idx1].mbHasFourche)&& (!segs[idx2].mbHasFourche))
			recalDistByForce3(segs[idx1].point2, segs[idx2].pt, dist, segs[idx1].force2, segs[idx2].force);


	}
}

void track_t::limiteEcart(int idx, vec_t symetricalPlan, vec_t segRight)
{
    float ecart = symetricalPlan.signedDistanceTo(segs[idx].pt);
    static const float minEcart = 20.f;
    static const float maxEcart = 35.f;

    if (ecart>0.f)
    {
        if (ecart>maxEcart)
        {
            segs[idx].pt -= segRight;// * (ecart +maxEcart)*0.1f;
            /*
            float newecart = symetricalPlan.signedDistanceTo(segs[idx].pt);
            printf("1prev %5.2f new %5.2f\n", ecart, newecart);
             */
        }
        else if (ecart<minEcart)
        {
            segs[idx].pt += segRight;// * (ecart -maxEcart)*0.1f;
            /*
            float newecart = symetricalPlan.signedDistanceTo(segs[idx].pt);
            printf("2prev %5.2f new %5.2f\n", ecart, newecart);
             */
        }
    }
    else
    {

        if (ecart<-maxEcart)
        {
            segs[idx].pt += segRight;// * (ecart +maxEcart)*0.1f;
            /*
            float newecart = symetricalPlan.signedDistanceTo(segs[idx].pt);
            printf("3prev %5.2f new %5.2f\n", ecart, newecart);
             */
        }
        else if (ecart>-minEcart)
        {
            segs[idx].pt -= segRight;// * (ecart -maxEcart)*0.1f;
            /*
            float newecart = symetricalPlan.signedDistanceTo(segs[idx].pt);
            printf("4prev %5.2f new %5.2f\n", ecart, newecart);
             */
        }
    }


}
void track_t::ApplyForces()
{
	for(int i=0;i<NBSEGS;i++)
	{
		int idx = i;
		int idxp1 = (i+1)%NBSEGS;
		int idxp2 = (i+2)%NBSEGS;
		int idxm1 = (i-1+NBSEGS)%NBSEGS;
		int idxm2 = (i-2+NBSEGS)%NBSEGS;


		segs[i].pt += segs[i].force;
		segs[i].force = vec_t::zero;
		if ((!segs[i].mbHasFourche)/*||(!segs[idxp2].mbHasFourche) */)
			segs[i].point2 = segs[i].pt;
		else
        {
            if (!segs[idxm1].mbHasFourche)
            {
                vec_t segDir = segs[idxm1].pt - segs[idxm2].pt;
                segDir.normalize();
                vec_t segRight;
                segRight.cross(segDir, vec(0.f, 1.f, 0.f));
                segRight.normalize();


                vec_t symetricalPlan = buildPlan(segs[idxm1].pt, segRight);

                limiteEcart(idx, symetricalPlan, segRight);

                segs[idx].point2 = symetricalPlan.symetrical(segs[idx].pt);
                //printf("Sym A\n");
            }

            else if (!segs[idxp1].mbHasFourche)
            {

                vec_t segDir = segs[idxp2].pt - segs[idxp1].pt;
                segDir.normalize();
                vec_t segRight;
                segRight.cross(segDir, vec(0.f, 1.f, 0.f));
                segRight.normalize();


                vec_t symetricalPlan = buildPlan(segs[idxp1].pt, segRight);
                limiteEcart(idx, symetricalPlan, segRight);

                segs[idx].point2 = symetricalPlan.symetrical(segs[idx].pt);
            }
            else
            {
                segs[i].point2 += segs[i].force2;
            }

        }


		segs[i].force2 = vec_t::zero;

	}
}


void track_t::SmoothInclinaison(const vec_t &vm1, const vec_t& v, vec_t& vp1, float threshold, float redresseRatio)
{

	vec_t vectprev = v-vm1;
	if (vectprev.length() > FLOAT_EPSILON)
	{
		vectprev.normalize();

		vec_t curVect1 = vp1-v;
		float curLength1 = curVect1.length();
		if (curLength1 > FLOAT_EPSILON)
		{
			curVect1 *= 1.f/curLength1;

			float dt = vectprev.dot(curVect1);
			if (dt < threshold)
			{
				vec_t rec = (curVect1 *curLength1*(1.f-redresseRatio)) + (vectprev*curLength1*redresseRatio);
				rec.normalize();
				rec *= curLength1;
				printf("L%5.2f dt %5.2f %5.2f %5.2f %5.2f\n", curLength1, dt, rec.x, rec.y, rec.z);
				vp1 = v + rec;
			}
		}
	}
}



inline void AntiCover(const vec_t& pt, const vec_t& currentPoint, vec_t *pForce, float distFromRoot0, float distFromRoot1)
{

	static const float minDist = 40.f;

	vec_t dif = pt - currentPoint;
	dif.w = 0.f;
	float d = dif.length();
	if ((d <minDist)&&(d>FLOAT_EPSILON))
	{
		dif *= (1.f/d);
		vec_t aforce = dif * (minDist -d);
        //aforce += vec(0.f, 10.f, 0.f);
		aforce.y = (aforce.y<0.f)?0.f:aforce.y;
        if (distFromRoot0 > distFromRoot1)
            aforce.y += (minDist -d) * 0.5f;



		(*pForce) -= aforce * 0.001f;
        /*
        createBoxMarker(currentPoint, vec(0.f, 1.0f, 0.f, 1.0f), 10.f );
        createBoxMarker(pt, vec(1.f, 0.0f, 0.f, 1.0f), 10.f );
        */
	}
}

void track_t::ApplyAntiCover(float trackLength, float segDist, float minimalDist)
{
    UNUSED_PARAMETER(minimalDist);

    float boutLen = (trackLength/(float)NBSEGS);
	/*
	int segTrackNum = 0;
	float segTrackAv = 0.f;
	*/


	for (int i=3;i<12;i++)
	{
        vec_t sourcesPt[2] = {segs[i].pt, segs[i].point2};
		vec_t *forcesPt[2] = {&segs[i].force, &segs[i].force2};

        int idx = i;
        int idxp1 = ((i+1)%NBSEGS);
        int idxm1 = ((i-1+NBSEGS)%NBSEGS);


        int idxOthers[] = {idx, idxp1, idxm1};

        bool hasFourche = (segs[i].mbHasFourche);

        if (hasFourche && (!segs[idxp1].mbHasFourche))
            continue;
        for (int k=0;k<(hasFourche?2:1);k++)
		{

            const vec_t& currentPoint = sourcesPt[k];

            // pass 1
            // any other segment

            for (int j=0;j<NBSEGS;j++)
			{
				if ((segs[j].pt-currentPoint).lengthSq() > (boutLen*5.f)*(boutLen*5.f))
					continue;

                if ( (j != idxp1) &&
                 (j !=  idxm1) &&
                    (i != j))
                {
                    for (float ft = 0.f; ft<boutLen; ft += segDist)
                    {
                        vec_t pt = getPointOnTrack(j, ft/boutLen, false);
                        AntiCover(pt, currentPoint, forcesPt[k], i*boutLen, j*boutLen + ft);
                    }
                    if (segs[j].mbHasFourche)
                        for (float ft = 0.f; ft<boutLen; ft += segDist)
                        {
                            vec_t pt = getPointOnTrack(j, ft/boutLen, true);
                            AntiCover(pt, currentPoint, forcesPt[k], i*boutLen, j*boutLen + ft);
                        }

                }
            }

            // pass 2
            // segment previous, current and next but with a different fourche index
            for (int aj=0;aj<3;aj++)
            {
                int j = idxOthers[aj];
                if (segs[j].mbHasFourche)
                {
                    for (float ft = 0.f; ft<boutLen; ft += segDist)
                    {
                        vec_t pt = getPointOnTrack(j, ft/boutLen, (k==0));
                        AntiCover(pt, currentPoint, forcesPt[k], i*boutLen, j*boutLen + ft);
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Pieces Deform --------------------------------------------------------------------------------------

extern meshVertex_t mVts[MAXTMPMESHBUFSIZE];
extern int vtav;
extern unsigned short tris[MAXTMPMESHBUFSIZE];
extern int triav;

vec_t track_t::getPointOnTrack(int segTrackNum, float interp, bool bFourche)
{
	//int idxm2 = (segTrackNum-2+NBSEGS)%NBSEGS;
	int idxm1 = (segTrackNum-1+NBSEGS)%NBSEGS;
	int idx = segTrackNum%NBSEGS;
	int idxp1 = (segTrackNum+1)%NBSEGS;
	int idxp2 = (segTrackNum+2)%NBSEGS;


	bool bSwap = (segs[idx].mbHasFourche && (!segs[idxp1].mbHasFourche));
	vec_t currentKey, nextKey, nextKeyP1, prevKey;

	if (bSwap)
	{
		// fourche sortante
		currentKey = (bFourche&&segs[idxp1].mbHasFourche)?segs[idxp1].point2:segs[idxp1].pt;
		nextKey = (bFourche&&segs[idx].mbHasFourche)?segs[idx].point2:segs[idx].pt;
		nextKeyP1 = (bFourche&&segs[idxm1].mbHasFourche)?segs[idxm1].point2:segs[idxm1].pt;
		prevKey = (bFourche&&segs[idxp2].mbHasFourche)?segs[idxp2].point2:segs[idxp2].pt;
		interp = 1.f - interp;
	}
	else

	{
		// fouche entrante
		currentKey = (bFourche&&segs[idx].mbHasFourche)?segs[idx].point2:segs[idx].pt;
		nextKey = (bFourche&&segs[idxp1].mbHasFourche)?segs[idxp1].point2:segs[idxp1].pt;
		nextKeyP1 = (bFourche&&segs[idxp2].mbHasFourche)?segs[idxp2].point2:segs[idxp2].pt;
		prevKey = (bFourche&&segs[idxm1].mbHasFourche)?segs[idxm1].point2:segs[idxm1].pt;
		if ( (!segs[idx].mbHasFourche) && segs[idxm1].mbHasFourche )
		{
			prevKey = (segs[idxm1].point2 + segs[idxm1].pt) * 0.5f;
		}
	}

	vec_t splinePos = currentKey.interpolateHermite(nextKey, nextKeyP1, prevKey, interp);


    if (
        (segs[idxp1].mbHasFourche && (! segs[idx].mbHasFourche) )
		)
    {
        vec_t nsplinePos = LERP(currentKey, nextKey, interp);//interpolateHermite(nextKey, nextKeyP1, prevKey, interp);
        if (interp > 0.5f)
            splinePos = LERP(nsplinePos, splinePos, Clamp( (interp-0.75f) * 4.f, 0.f, 1.f ) );
        else
            splinePos = nsplinePos;//LERP( splinePos, nsplinePos, Clamp( interp* 6.f, 0.f, 1.f ) );


    }
	else if (segs[idx].mbHasFourche && (!segs[idxp1].mbHasFourche) )
	{
        vec_t nsplinePos = LERP(currentKey, nextKey, interp);//interpolateHermite(nextKey, nextKeyP1, prevKey, interp);
		/*
        if (interp > 0.5f)
            splinePos = LERP( splinePos, nsplinePos, Clamp( (interp-0.75f) * 4.f, 0.f, 1.f ) );
        else
		*/
            splinePos = nsplinePos;//LERP( splinePos, nsplinePos, Clamp( interp* 6.f, 0.f, 1.f ) );
	}
	return splinePos;
}

float track_t::getWidthOnTrack( int segTrackNum, float interp, bool bFourche )
{
    int idx = segTrackNum%NBSEGS;
	int idxp1 = (segTrackNum+1)%NBSEGS;

    float smoothedInterp = smootherstep( 0.25f, 0.75f, interp );
    float aWidth = LERP( segs[idx].width[bFourche?1:0], segs[idxp1].width[bFourche?1:0], smoothedInterp );
    return aWidth;
}

float track_t::getBorderForcedOnTrack( int segTrackNum, float interp, bool bFourche )
{
    int idx = segTrackNum%NBSEGS;
	int idxp1 = (segTrackNum+1)%NBSEGS;

    float v1 = segs[idx].border[bFourche?1:0]?1.1f:0.0f;
    float v2 = segs[idxp1].border[bFourche?1:0]?1.1f:0.0f;
    float aBorder = LERP( v1, v2, interp );
    return aBorder;
}

float track_t::getGroundLockOnTrack( int segTrackNum, float interp, bool bFourche )
{
    int idx = segTrackNum%NBSEGS;
	int idxp1 = (segTrackNum+1)%NBSEGS;

    float v1 = segs[idx].groundLock[bFourche?1:0]?1.1f:0.0f;
    float v2 = segs[idxp1].groundLock[bFourche?1:0]?1.1f:0.0f;
    float aBorder = LERP( v1, v2, interp );
    return aBorder;
}

/*
float interf(vec_t pt1, vec_t pt2, vec_t plan)
{

	float dt1 = fabsf(plan.signedDistanceTo(pt1));
	float dt2 = fabsf(plan.signedDistanceTo(pt2));

	return (dt1)/(dt1+dt2);
}

void Track::computeDeformMatrix(const vec_t& newPos, const vec_t& prevPos, matrix_t &destMatrix)
{
	vec_t direction = newPos-prevPos;
	direction.w = 0.f;
	direction.normalize();

	vec_t right;
	right.cross(vec(0.f,1.f,0.f), direction);
	right.normalize();
	vec_t up;
	up.cross(direction, right);
	up.normalize();


	destMatrix.set( vec(right.x, right.y, right.z, 0.f),
		   vec(up.x, up.y, up.z, 0.f),
		   vec(direction.x, direction.y, direction.z, 0.f),
		   vec(newPos.x, newPos.y, newPos.z, 1.f));

}
*/
void Track::setDeformedMeshProperties(mesh_t *pm, float distance)
{
    UNUSED_PARAMETER(distance);

	pm->mBaseMatrix.identity();
	pm->computeBSphere();

	//mBuildProgress.push_back( buildProgress_t( distance, pm, pm->bSphere ) );
	/*
	matrix_t localScale, localTrans1, localTrans2, local;
	localScale.scale(0.3f);
	localTrans1.translation( -pm->bSphere );
	localTrans2.translation( pm->bSphere );
	local = localTrans1 * localScale * localTrans2;
	pm->mWorldMatrix = local;
	*/
	pm->updateWorldBSphere();
	pm->color = vec(1.f, 1.f, 1.f, 1.f);// vec(0.5f, 0.5f, 0.6f, 0.5f);
	pm->visible = true;
	pm->physic = false;
}

mesh_t* Track::clipMesh(mesh_t *pm, const vec_t& pt1, const vec_t& pt2, int slice, const vec_t&branchPoint)
{
    UNUSED_PARAMETER(slice);

	vec_t segDir, segPt = pt1;

	segDir = pt2-pt1;
	segDir.w = 0.f;

    segDir.normalize();
    vec_t segRight;
    segRight.cross(segDir, vec(0.f, 1.f, 0.f));
    segRight.normalize();


    vec_t symetricalPlan = buildPlan(segPt, segRight);

    float distToClipPlan = symetricalPlan.signedDistanceTo(branchPoint);
    //printf(" dist to clip plane : %5.2f\n", distToClipPlan);
	if ( distToClipPlan < 0.f) // pas bon ca
    {
        symetricalPlan = -symetricalPlan;
    }

    // clip it

    mesh_t *newpm = pm->clipPlane( symetricalPlan );
	if (!newpm)
		return pm;

    delete pm;

	return newpm;

	//return pm;
}


typedef struct fourche_t
{
	fourche_t()
	{
        Clear();
	}

    ~fourche_t()
    {
        ASSERT_GAME( mesh[0] == NULL );
        ASSERT_GAME( mesh[1] == NULL );
        ASSERT_GAME( mWallPhysicMeshes[0] == NULL );
        ASSERT_GAME( mWallPhysicMeshes[1] == NULL );
        ASSERT_GAME( mGroundPhysicMeshes[0] == NULL );
        ASSERT_GAME( mGroundPhysicMeshes[1] == NULL );
    }

    void Clear()
    {
        mesh[0] = mesh[1] = NULL;
        mWallPhysicMeshes[0] = mWallPhysicMeshes[1] = NULL;
        mGroundPhysicMeshes[0] = mGroundPhysicMeshes[1] = NULL;
    }

	mesh_t *mesh[2];
    mesh_t *mGroundPhysicMeshes[2];
    mesh_t *mWallPhysicMeshes[2];
    matrix_t mat[2];
	float distance;
} fourche_t;



mesh_t* createBoxMarker(const vec_t& pos, const vec_t& col, float sz)
{
	mesh_t *pm = generateBox();
	matrix_t tr;
	tr.translation(pos);
	pm->mWorldMatrix.scale( sz,sz,sz);
	pm->mWorldMatrix *= tr;
	pm->updateWorldBSphere();
	pm->color = col;
	pm->visible = true;
	pm->physic = false;
    return pm;
	//world.computeWorldSize();
}

//std::vector<hole_t> holes;
int track_t::isInHole(float aPos, float aSegLen)
{
    //std::vector<hole_t>::const_iterator iter = holes.begin();
    for (int i = 0;i<nbHoles; i ++)
    {
        const hole_t & hole = holes[i];
        if ( (aPos >= hole.holeStartDistance) &&
            ( (aPos+aSegLen) <= (hole.holeStartDistance+hole.holeLength)) )
            return (i+1);
    }
    return 0;
}

bool track_t::holeOnTheWay( float aPos, float aCheckLength)
{
    for (int i = 0;i<nbHoles; i ++)
    {
        const hole_t & hole = holes[i];
        if ( ( (aPos+aCheckLength) >= hole.holeStartDistance) &&
            ( (aPos <= (hole.holeStartDistance+hole.holeLength)) ) )
            return true;
    }
    return false;
}


// returns distance. 99999999 if no holes
float track_t::findNearestHole(float currentDistance, float &holeLength)
{
    if (!nbHoles)
        return 99999999.f;
    float shortest = 999999.f;

    for (int i = 0;i<nbHoles; i ++)
    {
        const hole_t & hole = holes[i];
        if ( (currentDistance <= (hole.holeStartDistance+hole.holeLength*0.5f) )  &&
            ( hole.holeStartDistance<shortest  ) )
        {
             shortest = hole.holeStartDistance;
             holeLength = hole.holeLength;
        }
    }
    return shortest - currentDistance;
}

float track_t::shortestDistanceToHoleForthAndBack(float currentDistance)
{
    if (!nbHoles)
        return 99999999.f;
    float shortest = 999999.f;

    for (int i = 0;i<nbHoles; i ++)
    {
        const hole_t & hole = holes[i];

        if ( (currentDistance > hole.holeStartDistance )  &&
			(currentDistance <= (hole.holeStartDistance+hole.holeLength) ) )
			return 0;

		float distprev = hole.holeStartDistance-currentDistance;
		if (distprev >0.f && distprev<shortest)
			shortest = distprev;

		float distnext = currentDistance- (hole.holeStartDistance+hole.holeLength);
		if (distnext >0.f && distnext<shortest)
			shortest = distnext;
    }
    return shortest;
}

void Track::deformPieces( track_t *pTrack, float trackLength, float segDist, const TrackTopology &topo, matrix_t *deformMatrices[2], bool bMiniTrack)
{
    track_t *mCurrentTrack = pTrack;
	int segTrackNum = 0;
	float segTrackAv = 0.f;
	std::list<mesh_t*> wallPhysicMeshes;
	std::list<mesh_t*> groundPhysicMeshes;
    std::list<mesh_t*> solidMeshes;

	std::vector<std::vector<fourche_t> > fourches;
	fourches.resize(4);
	int fourcheAv = -1;


    mAIPoints.clear();

	// previous pos
	float boutLen = (trackLength/(float)NBSEGS);
	vec_t prevPos[2];
	prevPos[1] = prevPos[0] = mCurrentTrack->getPointOnTrack(NBSEGS-1, 0.999f, false);

	// previous matrix
	matrix_t previousMt[2];
	vec_t newPos[2];
	newPos[1] = newPos[0] = mCurrentTrack->getPointOnTrack(segTrackNum, segTrackAv/boutLen, false);

	previousMt[0] = deformMatrices[0][0];
	previousMt[1] = deformMatrices[1][0];

    float previousRoadWidth = topo.width( false, 0 );
	int matNum = 0;
	matrix_t matrix0 = previousMt[0];



	// loop
	int patternIndex = 0;
	for (float ft = 0.f; ft<trackLength; ft += segDist, patternIndex++)
	{

		// next slice
		bool hasChangedSlice = false;
		segTrackAv += segDist;
		if (segTrackAv > boutLen)
		{
			segTrackAv -= boutLen;
			// next seg
			segTrackNum++;
			hasChangedSlice = true;
		}




		prevPos[0] = newPos[0];
		prevPos[1] = newPos[1];

		trackSeg_t *segs = mCurrentTrack->segs;

		bool hasFourcheNext = segs[(segTrackNum+1)%NBSEGS].mbHasFourche;
		bool hasFourcheCurrent = segs[segTrackNum%NBSEGS].mbHasFourche;
		bool hasFourchePrevious = segs[(segTrackNum-1+NBSEGS)%NBSEGS].mbHasFourche;
		bool hasFourche = (hasFourcheCurrent||hasFourcheNext);




		if (hasChangedSlice)
		{
			if (hasFourcheCurrent&&hasFourchePrevious&&(!hasFourcheNext))
				fourcheAv ++;
			if (hasFourcheNext && (!hasFourcheCurrent))
				fourcheAv ++;
			hasChangedSlice = false;
		}

		// new matrix
		matrix_t newMt[2];





		for (int k=0;k<2;k++)
		{
			newPos[k] = mCurrentTrack->getPointOnTrack(segTrackNum, segTrackAv/boutLen, (k==1));

			float holeLength;
			float distToHole = mCurrentTrack->findNearestHole( ft, holeLength );
			if (distToHole<= holeLength)
			{
				newPos[k].y += mCurrentTrack->computeDiveHeight( distToHole, holeLength, 0.15f);
			}
		}






		if (segTrackNum >= NBSEGS)
		{
            //printf("Skipping\n");
			newMt[1] = newMt[0] = deformMatrices[0][0];
/*            newMt[1].position.y += 10.f;
            newMt[0].position.y += 10.f;
*/
		}
		else
		{
			//computeDeformMatrix(newPos[0], prevPos[0], newMt[0]);
			newMt[0] = deformMatrices[0][matNum];

			if (hasFourche)
				//computeDeformMatrix(newPos[1], prevPos[1], newMt[1]);
				newMt[1] = deformMatrices[1][matNum];

			else if (!hasFourchePrevious)
			{
				newPos[1] = newPos[0];
				newMt[1] = newMt[0];
			}


		}

        if  ( (hasFourcheCurrent) && (!hasFourcheNext) )
		{
			if ( ((segTrackAv + segDist) / boutLen ) >1.f)
			{
				matrix_t tmpMatrix;
				tmpMatrix.lerp(newMt[0],  newMt[1], 0.5f);
				tmpMatrix.orthoNormalize();
				newMt[0] = newMt[1] = tmpMatrix;
			}
		}


		// holes
		for (int chkHoles = 0; chkHoles < (hasFourche?2:1); chkHoles ++)
		{
			vec_t hlPosPrev = previousMt[chkHoles].position + previousMt[chkHoles].up * 10.f;
			vec_t hlPosNew  = newMt[chkHoles].position + newMt[chkHoles].up * 10.f;

			if ( (hlPosNew.y > 0.f && hlPosPrev.y < 0.f) ||
				(hlPosNew.y < 0.f && hlPosPrev.y > 0.f) )
			{
				vec_t dif = (hlPosNew - hlPosPrev);
				dif *= 1.f/(hlPosNew.y - hlPosPrev.y);
				dif *= hlPosNew.y;
				vec_t holePos = hlPosNew - dif;

				if (mTrackHolesAv<MAX_TRACK_OCEAN_HOLES)
				{
					mTrackHoles[mTrackHolesAv++] = vec( holePos.x, holePos.y, holePos.z, 50.f * 50.f );
				}
			}
		}


        float newRoadWidth = topo.width( false, patternIndex );

        // AI
		if (hasFourche)
		{
			mAIPoints.push_back(AIPoint_t( newPos[0], newMt[0].right, newMt[0].up, newMt[0].dir, newPos[0] + (newMt[0].right*newRoadWidth), newPos[0] + (newMt[0].right*(-newRoadWidth)) , newRoadWidth,
				newPos[1], newMt[1].right, newMt[1].up, newMt[1].dir, newPos[1] + (newMt[1].right*12.f), newPos[1] + (newMt[1].right*(-newRoadWidth)), newRoadWidth
				));
		}
		else
		{
			//const shapePattern_t *pCurrentPattern = topo.pattern( false, patternIndex );

			mAIPoints.push_back(AIPoint_t( newPos[0], newMt[0].right, newMt[0].up, newMt[0].dir,
				newRoadWidth/*pCurrentPattern->shapeNear->width*/ ));
		}

		// create geometry slice
		fourche_t curFourche;

		int bInHole = mCurrentTrack->isInHole(ft, segDist);

		int aiPtPos = mAIPoints.size()-1;
		mAIPoints[aiPtPos].bFourcheStarting = ( (!hasFourcheCurrent) && hasFourcheNext);
		mAIPoints[aiPtPos].bFourcheEnding = (hasFourcheCurrent && (!hasFourcheNext));
		mAIPoints[aiPtPos].bInsideHole = (bInHole!=0);


        if ( !bInHole )
        {
            bool hasHoleNext = ( mCurrentTrack->isInHole(ft+segDist, segDist) != 0 );
            bool hasHolePrev = ( mCurrentTrack->isInHole(ft-segDist, segDist) != 0 );
            int triangulation = hasHoleNext?2:(hasHolePrev?1:0);

			bool bPushFourche = ( (hasFourcheNext&&(!hasFourcheCurrent)) ||
						(hasFourcheCurrent&&hasFourchePrevious&&(!hasFourcheNext)) );

			bPushFourche = false;
            // Geometry



            for (int sl = 0;sl<(hasFourche?2:1);sl++)
            {
				const shapePattern_t *pCurrentPattern = topo.pattern( (sl == 1), patternIndex );
                bool bTurningLeft = topo.turningLeft( (sl == 1), patternIndex );
                bool bTurningRight = topo.turningRight( (sl == 1), patternIndex );

                int leftDarkStart, leftDarkCount;
                int rightDarkStart, rightDarkCount;

                const vec_t&        colLight = topo.colorLight( (sl == 1), patternIndex );
                const vec_t&        colDark = topo.colorDark( (sl == 1), patternIndex );
                vec_t colMid = ( colLight + colDark ) * 0.5f;

                if (bTurningLeft || bTurningRight)
                {
                    if (bTurningLeft)
                    {
                        leftDarkStart = 0; leftDarkCount = 0xFF;
                        rightDarkStart = 0; rightDarkCount = 0x0;
                    }
                    else
                    {
                        rightDarkStart = 0; rightDarkCount = 0xFF;
                        leftDarkStart = 0; leftDarkCount = 0x0;
                    }
                }
                else
                {
                    /*
                    static int decalMotifStart =0;
                    //if (patternIndex&1)
                        --decalMotifStart &= 7;
                        */
                    leftDarkCount = fastrand()&7;
                    rightDarkCount = fastrand()&7;
                    leftDarkStart = fastrand()&7;
                    rightDarkStart = fastrand()&7;
                }

                float wallColorRedAlpha = topo.wallColorRedAlpha( (sl == 1), patternIndex )*255.f;
                u32 wallColorRedAlphau32 = (((int)wallColorRedAlpha) );// << 24 ) + 0xFF;




//                wallColorRedAlpha
				triav = 0;
				int nbVt = pCurrentPattern->GenerateMesh( (u8*)mVts, VAF_XYZ|VAF_NORMAL|VAF_COLOR, tris, triav,
					previousMt[sl], newMt[sl],
					triangulation, 0,
                    previousRoadWidth/12.f, newRoadWidth/12.f,
                    ((patternIndex&1)?colMid:colLight).toUInt32(), ((patternIndex&1)?colDark:(colMid*0.9f)).toUInt32(),
                    leftDarkStart, leftDarkCount, rightDarkStart, rightDarkCount, wallColorRedAlphau32
                    );
				vtav = nbVt;

                ASSERT_GAME( curFourche.mesh[sl] == NULL );
                curFourche.mesh[sl] = pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR );
				curFourche.distance = ft;
                curFourche.mat[sl] = previousMt[sl];
				if ( (!bPushFourche) && (!bMiniTrack) )
					setDeformedMeshProperties( curFourche.mesh[sl], ft );
                solidMeshes.push_back( curFourche.mesh[sl] );

                //curFourche.mesh[sl]->color = vec(1.f);//topo.color( (sl == 1), patternIndex );
            }

			if (!bMiniTrack)
			{ // no transparent, no fourches, no physics

            // transparency
			for (int sl = 0;sl<(hasFourche?2:1);sl++)
            {
				const shapePattern_t *pCurrentPattern = topo.pattern( (sl == 1), patternIndex );

				triav = 0;
				int nbVt = pCurrentPattern->GenerateMesh( (u8*)mVts, VAF_XYZ|VAF_NORMAL|VAF_COLOR, tris, triav,
					previousMt[sl], newMt[sl],
					triangulation, 1, previousRoadWidth/12.f, newRoadWidth/12.f);
				vtav = nbVt;
                if (nbVt)
				{
                    //FIXME: curFourche.mesh[sl] != NULL
                    //ASSERT_GAME( curFourche.mesh[sl] == NULL );
					curFourche.mesh[sl] = pushNewMesh( VAF_XYZ|VAF_NORMAL|VAF_COLOR );
					curFourche.distance = ft;
					curFourche.mat[sl] = previousMt[sl];

					if (!bPushFourche)
						setDeformedMeshProperties( curFourche.mesh[sl], ft );

					curFourche.mesh[sl]->mbTransparent = true;
					curFourche.mesh[sl]->mbCastShadows = false; // artefacts with VSM
				}
            }
            // create physic wall

            for (int sl = 0;sl<(hasFourche?2:1);sl++)
            {
                const shapePattern_t *pCurrentPattern = topo.pattern( (sl == 1), patternIndex );

				triav = 0;
				int nbVt = pCurrentPattern->GenerateMesh( (u8*)mVts, VAF_XYZ, tris, triav,
					previousMt[sl], newMt[sl],
					0, 2,
                    previousRoadWidth/12.f, newRoadWidth/12.f);
				vtav = nbVt;

                if ( nbVt )//buildPhysicSlice(defaultPieceSlice, defaultPieceSlice, -1, -1, previousMt[sl], newMt[sl], true, triangulation))
				{

					mesh_t *pm = pushNewMesh( VAF_XYZ );
	//                if (bMakeClip) pm = clipMesh(pm, newPos, prevPos, sl);

                    ASSERT_GAME( curFourche.mWallPhysicMeshes[sl] == NULL );
					curFourche.mWallPhysicMeshes[sl] = pm;//.push_back(pm);
					wallPhysicMeshes.push_back(pm);
				}

                //setDeformedMeshProperties(pm);
            }

            // create physic ground
            for (int sl = 0;sl<(hasFourche?2:1);sl++)
            {
				const shapePattern_t *pCurrentPattern = topo.pattern( (sl == 1), patternIndex );

               				triav = 0;
				int nbVt = pCurrentPattern->GenerateMesh( (u8*)mVts, VAF_XYZ, tris, triav,
					previousMt[sl], newMt[sl],
					0, 3,
                    previousRoadWidth/12.f, newRoadWidth/12.f);
				vtav = nbVt;
                if ( nbVt )//buildPhysicSlice(defaultPieceSlice, defaultPieceSlice, -1, -1, previousMt[sl], newMt[sl], false, triangulation))
				{

					mesh_t *pm = pushNewMesh(VAF_XYZ);
					pm->visible = false;
	//                if (bMakeClip) pm = clipMesh(pm, newPos, prevPos, sl);

                    ASSERT_GAME( curFourche.mGroundPhysicMeshes[sl] == NULL );
					curFourche.mGroundPhysicMeshes[sl] = pm;//.push_back(pm);
					groundPhysicMeshes.push_back(pm);
				}
                //setDeformedMeshProperties(pm);
            }
			}

            // fourche and clip check
            if ( bPushFourche)
			{
				fourches[fourcheAv].push_back(curFourche);
			}
        }

		previousMt[0] = newMt[0];
		previousMt[1] = newMt[1];
        previousRoadWidth = newRoadWidth;
		matNum ++;

        curFourche.Clear();
	}
	// clip meshes // find contact point

	for (int l = 0;l<4;l++)
	{
		if (!fourches[l].empty())
		{

			float nearestDist = 999999.f;
			vec_t nearestPoint(vec_t::zero);
			unsigned int brickCountLeft = 0;
			unsigned int brickCountRight = 0;
			for (unsigned int i = 0;i<fourches[l].size();i++)
			{
				vec_t colors[4] = {vec(1.f, 0.f, 0.f, 0.5f),
					vec(0.f, 1.f, 0.f, 0.5f),
					vec(0.f, 0.f, 1.f, 0.5f),
					vec(0.f, 1.f, 1.f, 0.5f) };

				if (i&1)
				{
				fourches[l][i].mesh[0]->color = colors[0];
				fourches[l][i].mesh[1]->color = colors[0];
				}
				static const float meshHalfWidth = 12.f;

				for (unsigned int j = 0;j<fourches[l].size();j++)
				{
					for (int k = 0;k<4;k++)
					{
						vec_t seg1A, seg1B, seg2A, seg2B, res;

						seg1A = vec((k&1)?-meshHalfWidth:meshHalfWidth, 0.f, 0.f);
						seg1B = vec((k&1)?-meshHalfWidth:meshHalfWidth, 0.f, segDist);
						seg2A = vec((k&2)?-meshHalfWidth:meshHalfWidth, 0.f, 0.f);
						seg2B = vec((k&2)?-meshHalfWidth:meshHalfWidth, 0.f, segDist);
						const matrix_t& t0 = fourches[l][i].mat[0];
						const matrix_t& t1 = fourches[l][j].mat[1];
						seg1A.TransformPoint( t0 );
						seg1B.TransformPoint( t0 );
						seg2A.TransformPoint( t1 );
						seg2B.TransformPoint( t1 );

						if ( segment2segmentXZ(seg1A, seg1B, seg2A, seg2B, res) )
						{

							float newDist = (res-fourches[l][0].mat[0].position).length();
							if (newDist < nearestDist)
							{
								nearestDist = newDist;
								nearestPoint = res;
								brickCountLeft = i;
								brickCountRight = j;
							}
						}
					}
				}
			}

            // clip meshes
            vec_t extremeCutPoint;
            extremeCutPoint = (l&1)?fourches[l][fourches[l].size()-1].mat[1].position : fourches[l][0].mat[0].position;
            int sliceId = 1-(l&1);

            const vec_t& cutPoint1 = (l&1)?nearestPoint:extremeCutPoint;
            const vec_t& cutPoint2 = (l&1)?extremeCutPoint:nearestPoint;

            for (int j = 0 ; j < 2 ; j++)
            {
                vec_t branchPoint;
                branchPoint = (l&1)?fourches[l][0].mat[j].position : fourches[l][fourches[l].size()-2].mat[j].position;


                unsigned int iStart = (l&1)?(j?brickCountRight:brickCountLeft):0;
                unsigned int iEnd = (l&1)?fourches[l].size(): ( (j?(brickCountRight+1):(brickCountLeft+1)) );

                for (unsigned int i = iStart;i<iEnd;i++)
                {
                    // geom
                    mesh_t *srcm = fourches[l][i].mesh[j];
                    //FIXME: Should it be removed from solidMeshes?

                    mesh_t *pm = clipMesh(srcm, cutPoint1, cutPoint2, sliceId, branchPoint);
                    //FIXME: what holds the clipped mesh ref? Should it be SolidMeshes?

                    fourches[l][i].mesh[j] = NULL;

					setDeformedMeshProperties( pm, 0 );//fourches[l][i].distance );

                    // wall
                    srcm = fourches[l][i].mWallPhysicMeshes[j];
					if (srcm)
					{
                    wallPhysicMeshes.remove(srcm);
                    pm = clipMesh(srcm, cutPoint1, cutPoint2, sliceId, branchPoint);
                    wallPhysicMeshes.push_back(pm);
                    pm->visible = false;

                    fourches[l][i].mWallPhysicMeshes[j] = NULL;
					}
                    //setDeformedMeshProperties(pm);

                    // ground
                    srcm = fourches[l][i].mGroundPhysicMeshes[j];
					if (srcm)
					{
                    groundPhysicMeshes.remove(srcm);
                    pm = clipMesh(srcm, cutPoint1, cutPoint2, sliceId, branchPoint);
                    groundPhysicMeshes.push_back(pm);
                    pm->visible = false;

                    fourches[l][i].mGroundPhysicMeshes[j] = NULL;
					}
                    //setDeformedMeshProperties(pm);
                }
			}
		}
	}

	// merge physic shits
    mesh_t * trackMeshGroundPhysic = NULL;
    mesh_t * trackMeshWallPhysic = NULL;

    if (!groundPhysicMeshes.empty())
    {
        trackMeshGroundPhysic = merge(groundPhysicMeshes);
        trackMeshGroundPhysic->visible = false;

        deleteMeshes( groundPhysicMeshes );
        ASSERT_GAME( groundPhysicMeshes.empty() );
    }
    if (!wallPhysicMeshes.empty())
    {
        trackMeshWallPhysic = merge(wallPhysicMeshes);
        trackMeshWallPhysic->visible = false;

        deleteMeshes( wallPhysicMeshes );
        ASSERT_GAME( wallPhysicMeshes.empty() );
    }

    if (!solidMeshes.empty())
    {
        mesh_t *pMergedSolidMesh =merge(solidMeshes);
        pMergedSolidMesh->visible = true;
        pMergedSolidMesh->color = vec(1.f);
        pMergedSolidMesh->computeBSphere();
        pMergedSolidMesh->mWorldMatrix.identity();
        pMergedSolidMesh->updateWorldBSphere();

        deleteMeshes( solidMeshes );
        ASSERT_GAME( solidMeshes.empty() );
    }

    physicWorld.setTrackMeshes(trackMeshGroundPhysic, trackMeshWallPhysic);
}

void track_t::ComputeSegments()
{
	PROFILER_START(ComputeSegments);
	// seed

	g_seed = seed;
	// building stating segs
	BuildT0(trackLength);
	ApplyPressures( 64, 0, 20.f );

	// test

    float decalUp = 17.f/2000.f;
    UpdateSegments( decalUp );
    ApplyPressures( 64, 0, 0.f );

    PROFILER_END(); // ComputeSegments
}

void track_t::UpdateSegments( float decalUp )
{
    PROFILER_START(UpdateSegments);
	for (int i=0;i<2000;i++)
	{
		std::vector<vec_t> lockedPositions;
		lockedPositions.resize(NBSEGS);

		for (int j= 0;j<NBSEGS;j++)
		{
			if ( segs[j].positionLock[0] )
			{
				lockedPositions[j] = segs[j].pt;
			}
		}
		for (int j= 0;j<NBSEGS;j++)
		{
			segs[j].pt.y += decalUp;
			segs[j].point2.y += decalUp;
		}

		segs[1].pt.y = segs[0].pt.y; // always starts flat


		ApplyPressureModifier();
		ApplyDistModifier( trackLength/(float)(NBSEGS));
		//ApplyFourcheModifiers();
        //
		/*ApplyMaxCurveModifier();
		ApplyMaxSlopeModifier(0.8f);
         */



            //ApplyAntiCover(trackLength, (trackLength/(float)(NBSEGS)) * 0.125f, trackLength/(float)(NBSEGS));
		ApplyForces();


                // force go in/go out of water
                if (i>1000)
                {
                    for (int j = 0;j<NBSEGS;j++)
                    {
                        float *h1, *h2;
                        for (int k=0;k<2;k++)
                        {
                            if (!k) { h1 = &segs[j].pt.y; h2 = &segs[(j+1)%NBSEGS].pt.y; }
                                else { h1 = &segs[j].point2.y; h2 = &segs[(j+1)%NBSEGS].point2.y; }
                            float dif = fabsf(*h2-*h1);
                            if (dif< 30.f)
                            {
                                if ( ( *h1<0 ) && (*h2>0) )
                                {
                                    *h1-= 0.05f;
                                    *h2+= 0.05f;
                                }
                                else if ( ( *h2<0 ) && (*h1>0) )
                                {
                                    *h2-= 0.05f;
                                    *h1+= 0.05f;
                                }
                            }
                            if ((*h1<0)&&(*h1>-38))
                            {
                                *h1 = -38.f;
                            }

                            if ((*h1>0)&&(*h1<10))
                            {
                                *h1 = 10.f;
                            }

                        }

                    }
                }

                // anti crossing lines

                for (int j = 0;j<NBSEGS;j++)
                {
                    vec_t res;
                    int idx = j;
                    int idxp1 = (j+1)%NBSEGS;
                    if ( segs[idx].mbHasFourche && segs[idxp1].mbHasFourche)
                    {
                        if (segment2segmentXZ(segs[j].pt, segs[idxp1].pt,
                                              segs[j].point2, segs[idxp1].point2, res))
                        {
                            static const float forceChgCrossing = 0.05f;
segs[j].force.y += forceChgCrossing;
//segs[idxp1].force.y += forceChgCrossing;

segs[j].force2.y += forceChgCrossing;
//segs[idxp1].force2.y -= forceChgCrossing;

                        }
                    }
                }




                /*
                // decal up pour etre 1 peu au dessus de l'eau
                for (int j = 0;j<NBSEGS;j++)
                {
                    segs[i].pt.y += decalUp;
                    segs[i].point2.y += decalUp;
                }
                */

		for (int j= 0;j<NBSEGS;j++)
		{
			if ( segs[j].positionLock[0] )
			{
				segs[j].pt = lockedPositions[j];
			}
		}
	}

	// holes generation
#if 0
	this->nbHoles = 0;
	float holeDist = 0.f;
	float boutLen = (trackLength/(float)NBSEGS);
	for (int j = 0;j<NBSEGS;j++, holeDist += boutLen )
	{
		int idxp1 = (j+1)%NBSEGS;
		if (segs[idxp1].mbHasFourche)
			continue;

		int idxp2 = (j+2)%NBSEGS;
		int idxm1 = (j-1+NBSEGS)%NBSEGS;

		vec_t cur = segs[idxp1].pt-segs[j].pt;
		cur.normalize();
		vec_t nex = segs[idxp2].pt-segs[idxp1].pt;
		nex.normalize();
		vec_t pre = segs[j].pt - segs[idxm1].pt;
		pre.normalize();

		if ( (pre.dot(cur)>0.99f)&&(cur.dot(nex)> 0.99f)&&(this->nbHoles<8)&&(r01()>0.3f) )
		{
			holes[nbHoles].holeStartDistance = holeDist;
			holes[nbHoles].holeLength = boutLen*r01()*0.8f;
			nbHoles++;

		}

	}
#endif

	// segs up

	for (int k=0;k<2;k++)
	{
		for (int j = 0;j<NBSEGS;j++)
		{
			int idx = j;
			int idxp1 = (j+1)%NBSEGS;
			int idxm1 = (j-1 + NBSEGS)%NBSEGS;

			vec_t s1, s2;
			if (k)
			{
				s1 = (segs[idxp1].point2 - segs[idx].point2);
				s2 = (segs[idx].point2 - segs[idxm1].point2);
			}
			else
			{
				s1 = (segs[idxp1].pt - segs[idx].pt);
				s2 = (segs[idx].pt - segs[idxm1].pt);
			}

			s1.normalize();
			s2.normalize();



            segs[idx].right[k].cross(s2, segs[idx].up[k] );
            segs[idx].right[k].normalize();
#if 0
			if (s1.dot(s2)>=0.9f)
			{
				segs[idx].up[k] = vec(0.f, 1.f, 0.f);
				//continue;
			}

			vec_t upit;
			upit.cross( s1, s2 );
			vec_t right;
			right.cross(s1, upit);
			right.normalize();



			segs[idx].up[k].cross( right, s1);

			segs[idx].up[k].normalize();
			if (segs[idx].up[k].y<0.f)
				segs[idx].up[k] = -segs[idx].up[k];

			// attenuate
			static const float upAttenuateFactor = 1.5f;
			//segs[idx].up[k] += vec(0.f, 1.f, 0.f) * upAttenuateFactor;
            //segs[idx].up[k] = vec( 0.f, 1.f, 0.f );
			segs[idx].up[k].w = 0.f;
			segs[idx].up[k].normalize();

			// proper right
			segs[idx].right[k] = right;//.cross( segs[idx].up[k], s2 );
			//segs[idx].right[k].normalize();
#endif

		}
	}
	// up for fourches
	for (int j = 0;j<NBSEGS;j++)
	{
		int idx = j;
		int idxp1 = (j+1)%NBSEGS;
		int idxm1 = (j-1 + NBSEGS)%NBSEGS;
		if ( ( !segs[idx].mbHasFourche && segs[idxm1].mbHasFourche) ||
			 (segs[idxp1].mbHasFourche && (!segs[idx].mbHasFourche)) )
		{
			segs[idx].up[0] += segs[idx].up[1];
			segs[idx].up[0].normalize();
			segs[idx].up[1] = segs[idx].up[0];
			//segs[idx].up[0] *= 10.f;
		}

	}

	// minimal curve

	// build minimal T0



    PROFILER_END(); // UpdateSegments
}

void Track::ComputeMinimalCurve(  track_t& tr )
{
    for (int k = 0;k<2;k++)
    {
// iterate T0

        PerSliceInfo_t tmpcurveWidth[NBSEGS * 4];
        tr.DumpMatrices( curve[k], (k==1), (tr.trackLength/(float)(NBSEGS_MINIMALCURVE)), NBSEGS_MINIMALCURVE, tmpcurveWidth );

        for (int i=0;i<(NBSEGS * 4);i++)
        {
            curveWidth[k][i] = tmpcurveWidth[i].width;
        }

        /*
        for (int i=0;i<NBSEGS_MINIMALCURVE;i++)
            curve[k][i].position.y += 17.f;
            */
        // iterate T0

        float lateralShift[NBSEGS*4];
        memset(lateralShift, 0, sizeof(float)* NBSEGS*4);

        memcpy (minimalCurve[k], curve[k], sizeof(matrix_t) * NBSEGS*4 );

        vec_t *miniNewPos = new vec_t [NBSEGS*4];

        for (int i=0;i<100;i++)
        {

                for (int j=0;j<NBSEGS*4;j++)
                {
                        int idxp1   = (j+1)%(NBSEGS*4);
                        int idx     = (j);
                        int idxm1   = (j-1+NBSEGS*4)%(NBSEGS*4);

                        const vec_t& vp1 = minimalCurve[k][idxp1].position;
                        const vec_t& v   = curve[k][idx  ].position;
                        const vec_t& vm1 = curve[k][idxm1].position;

                        vec_t difav = vp1-v;
                        difav.normalize();
                        vec_t divpr = v-vm1;
                        divpr.normalize();

                        vec_t crossup;
                        crossup.cross(difav, divpr);

                        float dt = crossup.dot(minimalCurve[k][idx].up);

                        float strength = dt * 0.25f;
                        float MaxDecalageLateral = ( ( curveWidth[k][idx] / 2.f ) - 1.f );
                        if ( fabsf(lateralShift[idx] + strength) < MaxDecalageLateral )
                        {
                                miniNewPos[idx] = minimalCurve[k][idx].position - minimalCurve[k][idx].right * strength;
                                lateralShift[idx] += strength;
                        }
                }
                // re-apply
                for (int j=0;j<NBSEGS*4;j++)
                {
                        minimalCurve[k][j].position = miniNewPos[j];
                }
        }

        delete [] miniNewPos;
    }
}

void 	track_t::ComputeBounds( vec_t & boundMin, vec_t & boundMax )
{

    boundMin = boundMax = segs[0].pt;
    for (int i=0;i<NBSEGS;i++)
    {
        segs[i].pt.y += this->YTranslation;
        segs[i].point2.y += this->YTranslation;

        boundMin.isMinOf(segs[i].pt);
        boundMin.isMinOf(segs[i].point2);
        boundMax.isMaxOf(segs[i].pt);
        boundMax.isMaxOf(segs[i].point2);
    }
}

void track_t::ComputeUpForSpeed()
{
    for (int k=0;k<2;k++)
    {
        for (int j = 0;j<NBSEGS;j++)
        {
            int idx = j;
            int idxp1 = (j+1)%NBSEGS;
            int idxm1 = (j-1 + NBSEGS)%NBSEGS;

            vec_t s1, s2;
            if (k)
            {
                s1 = (segs[idxp1].point2 - segs[idx].point2);
                s2 = (segs[idx].point2 - segs[idxm1].point2);
            }
            else
            {
                s1 = (segs[idxp1].pt - segs[idx].pt);
                s2 = (segs[idx].pt - segs[idxm1].pt);
            }

            s1.normalize();
            s2.normalize();



            segs[idx].right[k].cross(s2, segs[idx].up[k] );
            segs[idx].right[k].normalize();

            float dotDir = s1.dot(s2);
            if (dotDir>=0.9999f)
            {
                segs[idx].up[k] = vec(0.f, 1.f, 0.f);
                continue;
            }

            vec_t upit;
            upit.cross( s1, s2 );
            vec_t right;
            right.cross(s1, upit);
            right.normalize();



            segs[idx].up[k].cross( right, s1);




            segs[idx].up[k].normalize();
            if (segs[idx].up[k].y<0.f)
                segs[idx].up[k] = -segs[idx].up[k];

            if ( idx && (segs[idx-1].right[k].dot(s1) > 0.f) )
                segs[idx].up[k] += right * 0.5f;
            else
                segs[idx].up[k] -= right * 0.5f;

            // attenuate
            //static const float upAttenuateFactor = 1.5f;
            //segs[idx].up[k] += vec(0.f, 1.f, 0.f) * upAttenuateFactor;
            //segs[idx].up[k] = vec( 0.f, 1.f, 0.f );
            segs[idx].up[k].w = 0.f;
            segs[idx].up[k].normalize();

            // proper right
            segs[idx].right[k] = right;//.cross( segs[idx].up[k], s2 );
            //segs[idx].right[k].normalize();


        }
    }
}

void track_t::DumpMatrices( matrix_t *pMats, bool bFourche, float distance, int nbMatsMax, PerSliceInfo_t *psi )
{
	float boutLen = (trackLength/(float)NBSEGS);
	//int curve1av = 0;
	int segTrackNum = 0;
	float segTrackAv = 0.f;
	int k = bFourche?1:0;
	int nbMats = 0;
    PerSliceInfo_t *pDestWidths = psi;
	for (float ft = 0.f; ft<trackLength; ft += distance)
	{

		segTrackAv += distance;
		if (segTrackAv > boutLen)
		{
			segTrackAv -= boutLen;
			// next seg
			segTrackNum++;
			//hasChangedSlice = true;
		}
		int idxp1 = (segTrackNum+1)%NBSEGS;
		int i = segTrackNum;



		//for (float lp = 0.f; lp<1.f; lp +=0.25f,currentDist+= distance )

		{
			float lp = segTrackAv/boutLen;
			float holeLength;
			float distToHole = findNearestHole( ft, holeLength );

            pDestWidths->width = getWidthOnTrack( i, lp, bFourche );
            pDestWidths->borderForced = getBorderForcedOnTrack( i, lp, bFourche );
            pDestWidths++;

			vec_t ps = getPointOnTrack(i, lp, bFourche);
			vec_t ps2 = getPointOnTrack(i, lp+(distance/boutLen), bFourche);
			ps.w = ps2.w = 0.f;
			if (distToHole<= holeLength)
			{
				ps.y += computeDiveHeight( distToHole, holeLength, 0.15f);
			}


			distToHole = findNearestHole( ft+ (distance), holeLength );
			if (distToHole<= holeLength)
			{
				ps2.y += computeDiveHeight( distToHole, holeLength, 0.15f);
			}


			// up
			vec_t daup = LERP(segs[i].up[k], segs[idxp1].up[k], lp);
			daup.normalize();
			// rights

			vec_t daright, dadir;
			dadir = ps2-ps;
			dadir.normalize();
			daright.cross(daup, dadir);
			daright.normalize();

            daup.cross(dadir, daright );
            daup.normalize();

			/*curve1[curve1av++].*/ pMats->set( daright, daup, dadir, ps);



            if (bFourche&&(! (segs[i].mbHasFourche || segs[(i+1)%NBSEGS].mbHasFourche)))
				pMats->m16[15] = 0.f;
			else
				pMats->m16[15] = 1.f;
			pMats++;
			nbMats ++;
			if (nbMats >= nbMatsMax)
				return;
		}
	}
}

void initTracks()
{
	PROFILER_START(initTracks);

    for (unsigned int i = 0;i<NBTracks;i++)
	{
		Tracks[i].ComputeSegments();
	}


    PROFILER_END(); // initTracks
}

void initTracksMiniMesh()
{
    PROFILER_START(initTracksMiniMesh);

    // mini mesh

    matrix_t mtmini;
    mtmini.identity();
    for ( int i = 0;i<GameSettings.TracksCountUsedInGame;i++)
	{
        mesh_t *minitrack = track.GenerateTrackForMenus( &Tracks[i], mtmini );

		if (minitrack)
		{
			minitrack->computeBSphere();
            matrix_t recenter;
            recenter.translation( -minitrack->bSphere );
            transformMesh( minitrack, recenter);
            minitrack->computeBSphere();

			minitrack->visible = true;
			minitrack->color = vec(1.f);
			minitrack->mWorldMatrix.identity();
			minitrack->updateWorldBSphere();
		}
        Tracks[i].mMiniMesh = minitrack;
    }

	PROFILER_END(); // initTracksMiniMesh
}


void Track::GoWithTrack( track_t *pTrack, bool bForceComputation )
{
    UNUSED_PARAMETER(bForceComputation);

	/*if ((mCurrentTrack == pTrack)&&(!bForceComputation))
		return;
*/
    mCurrentTrack = pTrack;

	PROFILER_START(TrackCompute);

//	mBuildProgress.clear();

	mCurrentTrack = pTrack;

	mTrackHolesAv = 0;

	TrackTopology topo;

	// generate matrices
	float segDist = 6.5f;
	int mbSteps = (int)(mCurrentTrack->trackLength/segDist) + 1;
	matrix_t *deformMatrices[2];
	deformMatrices[0] = new matrix_t[ mbSteps ];
	deformMatrices[1] = new matrix_t[ mbSteps ];

    PerSliceInfo_t *psi[2];
	psi[0] = new PerSliceInfo_t[ mbSteps ];
	psi[1] = new PerSliceInfo_t[ mbSteps ];

	mCurrentTrack->DumpMatrices( deformMatrices[0], false, segDist, mbSteps, psi[0] );
	mCurrentTrack->DumpMatrices( deformMatrices[1], true, segDist, mbSteps, psi[1] );
	LOG("Build Topology\n");
	//topo.FillTopologyWithDefault( pTrack->mDefaultShapePattern, mbSteps );
	topo.InitWithDefault(pTrack->mDefaultShapePattern, false, mbSteps, psi[0] );
	topo.InitWithDefault(pTrack->mDefaultShapePattern, true, mbSteps, psi[1] );
	topo.BuildTopology( false, deformMatrices[0], mbSteps, psi[0], *mCurrentTrack );
	topo.BuildTopology( true, deformMatrices[1], mbSteps, psi[1], *mCurrentTrack );

	LOG("Done\n");
	// func to set it up
	// placed here to create voxel so we can find it in topology
    /*
    if (mCurrentTrack->TrackSetupFunction)
         mCurrentTrack->TrackSetupFunction( topo, deformMatrices[0], deformMatrices[1] );
         */
    TrackSetupGuenin( pTrack, topo, deformMatrices[0], deformMatrices[1], psi[0], psi[1] );
    GSkyScatParams = &pTrack->mSkyScatteringParameters;
    GSkyScatParams->mbIsDirty = true;

	LOG("Deform Pieces\n");
	PROFILER_START(MeshDeform);
	deformPieces( mCurrentTrack, mCurrentTrack->trackLength, segDist, topo, deformMatrices );

	PROFILER_END(); // MeshDeform

	LOG("Done\n");

    // build spawn points

	matrix_t mt1, mt2, atr, atr2;
	atr.translation(5, 0, 0);
	atr2.translation(-5, 0, 0);

	track.getAIPoint(0).BuildMatrix(mt1, false);
        track.getAIPoint(4).BuildMatrix(mt2, false);

	for (int i=0;i<8;i++)
	{
		matrix_t mtr;
        mtr.lerp(mt1, mt2, (float)(i>>1) * 0.33f + 0.25f);
        mtr.orthoNormalize();
		spawnPoints[i] = ((i&1)?atr:atr2) * mtr;
	}
	LOG("SetOceanHoles\n");


    pTrack->mbIsDirty = false;

    LOG("Compute Minimal curve");

    ComputeMinimalCurve( *pTrack );
    pTrack->ComputeBounds( boundMin, boundMax );

    TrackSetupWorld( pTrack, topo, deformMatrices[0], deformMatrices[1], psi[0], psi[1] );

    // set topology to aipoints
    for ( unsigned int i = 0; i < mAIPoints.size() ; i ++ )
    {
        mAIPoints[i].topology[0] = topo.mCurrentToopology[0][i].trackSliceProperties;
        mAIPoints[i].topology[1] = topo.mCurrentToopology[1][i].trackSliceProperties;
    }

	delete [] deformMatrices[0];
	delete [] deformMatrices[1];
    delete [] psi[0];
    delete [] psi[1];

	// generate prefabs

	for (unsigned int i = 0;i<pTrack->mPrefabs.size();i++)
	{
		prefabInstance_t &prefabInstance = pTrack->mPrefabs[i];

		std::map<std::string, mesh_t*>::iterator iter = prefabs.find(prefabInstance.mPrefabFile);
		if ( iter != prefabs.end())
		{
			// add and instanciate prefab
			mesh_t *pm = (*iter).second->clone();

			matrix_t tr;
			pm->mWorldMatrix = prefabInstance.mPrefabMatrix;
			pm->updateWorldBSphere();
			pm->color = vec(1.f);
			pm->visible = true;
			pm->physic = false;

			prefabInstance.mMesh = pm;
		}
		else
			prefabInstance.mMesh = NULL;
	}

	LOG("Done\n");

	PROFILER_END(); // TrackCompute
}

mesh_t* Track::GenerateTrackForMenus( track_t *pTrack, matrix_t &mt )
{
    UNUSED_PARAMETER(mt);

	PROFILER_START(GenerateTrackForMenus);

	//mCurrentTrack = pTrack;

	mesh_t *last = GMeshes[GetMeshStack()].last;

	TrackTopology topo;

	float segDist = 10.f;
	int mbSteps = (int)(pTrack->trackLength/segDist) + 1;
	matrix_t *deformMatrices[2];
	deformMatrices[0] = new matrix_t[ mbSteps ];
	deformMatrices[1] = new matrix_t[ mbSteps ];

    PerSliceInfo_t *psi[2];
	psi[0] = new PerSliceInfo_t[ mbSteps ];
	psi[1] = new PerSliceInfo_t[ mbSteps ];

	pTrack->DumpMatrices( deformMatrices[0], false, segDist, mbSteps, psi[0] );
	pTrack->DumpMatrices( deformMatrices[1], true, segDist, mbSteps, psi[1] );



	topo.InitWithDefault( pTrack->mDefaultShapePattern, false, mbSteps, psi[0] );
	topo.InitWithDefault( pTrack->mDefaultShapePattern, true, mbSteps, psi[1] );

	deformPieces( pTrack, pTrack->trackLength, segDist, topo, deformMatrices, true );

	delete [] deformMatrices[0];
	delete [] deformMatrices[1];
    delete [] psi[0];
    delete [] psi[1];
    deformMatrices[0] = NULL;
    deformMatrices[1] = NULL;
    psi[0] = NULL;
    psi[1] = NULL;

	// merge
	std::list<mesh_t*> meshestm;
    if ( last )
	    last = last->mNext;
    else
        last = GMeshes[GetMeshStack()].first;

	while (last)
	{
		meshestm.push_back( last );
		last = last->mNext;
	}
     mesh_t * merged = NULL;
    if ( meshestm.size() > 1)
    {
	    merged = merge(meshestm);
	    deleteMeshes(meshestm);
    }
    else
    {
        merged = *meshestm.begin();
    }


	// back to normal
	//mCurrentTrack = NULL; // so you can generate the same track for game
    pTrack->mbIsDirty = false;
	PROFILER_END(); // GenerateTrackForMenus

	return merged;
}

vec_t mProjX, mProjY;
inline vec_t projectWithGrav(const vec_t& vt)
{
    return vec(vt.dot(mProjX), 0.f, vt.dot(mProjY));
}

inline float SquaredDistance(const vec_t& vt1, const vec_t& vt2)
{
	return ( (vt1.x-vt2.x)*(vt1.x-vt2.x) + (vt1.y-vt2.y)*(vt1.y-vt2.y) + (vt1.z-vt2.z)*(vt1.z-vt2.z) );
}

inline float Distance(const vec_t& vt1, const vec_t& vt2)
{
	return sqrtf( (vt1.x-vt2.x)*(vt1.x-vt2.x) + (vt1.y-vt2.y)*(vt1.y-vt2.y) + (vt1.z-vt2.z)*(vt1.z-vt2.z) );
}

typedef unsigned int uint;
bool Track::getClosestSampledPoint(const vec_t& mobilePoint,
                                   matrix_t& trackMatrix,
                                   vec_t& trackMiddle,
                                   int &aaCurIdx,
                                   float &rightLeftFactor,
                                   const vec_t& lastKnownGravity,
                                   float &upDownFactor,
                                   float& distanceToBorder,
                                   int &aaCurFourche,
								   bool bHoleDetected)
{

    PROFILER_START(GetClosestSampledPoint);

	// point 0
	if (lastKnownGravity.dot(vec(1.f, 0.f, 0.f))>0.9f)
	{
		mProjX.cross(lastKnownGravity, vec(0.f, 0.f, 1.f));
		mProjX.normalize();
		mProjY.cross(mProjX, lastKnownGravity);
		mProjY.normalize();
	}
	else
	{
		mProjX.cross(lastKnownGravity, vec(1.f, 0.f, 0.f));
		mProjX.normalize();
		mProjY.cross(mProjX, lastKnownGravity);
		mProjY.normalize();
	}

	uint loopSC = mAIPoints.size();//uint(mBricks.size()*SPLITCOUNT);
	int loopthru = loopSC;

	vec_t ptCol;
	uint i;
	uint localIdx;

	//uint halfss = 10;
    int oocurpt;
    vec_t amobilePoint;
    float oocurdist;


    const uint curIdx = aaCurIdx;
    if ( curIdx > loopSC)
    {
        loopthru = loopSC;
        localIdx =0;
    }
    else
    {
        loopthru = bHoleDetected?100:50 ;
        localIdx = ((curIdx-(loopthru>>1))+loopSC)%loopSC;
    }


    oocurdist = 99999999.f;
    oocurpt = -1;
    float railFourcheDist = 99999999.f;


    amobilePoint = projectWithGrav(mobilePoint);


    for (int tanj = 0;tanj<1;tanj++) // BEWARE FOURCHE
    {
        int j = (tanj+aaCurFourche)&1;

        for (i=0;i<(uint)loopthru;i++)
        {
            int lpidx = (localIdx+i)%loopSC;


            vec_t tv1, tv2;
            tv1 = projectWithGrav(mAIPoints[lpidx].mRailRight[j]);
            tv2 = projectWithGrav(mAIPoints[lpidx].mRailLeft[j]);


            vec_t tmpptCol;
            /*
             printf("dist = %5.2f - %d \n", (tv1-amobilePoint).length(), lpidx );
             if ( lpidx == 43)
             {
             printf("Projected : %5.4f %5.4f %5.4f\n", amobilePoint.x, amobilePoint.y, amobilePoint.z );
             printf("p1        : %5.4f %5.4f %5.4f\n", tv1.x, tv1.y, tv1.z );
             printf("p2        : %5.4f %5.4f %5.4f\n", tv2.x, tv2.y, tv2.z );

             }
             */
            if (CollisionClosestPointOnSegment( amobilePoint, tv1, tv2, tmpptCol))
            {
                float nDist = SquaredDistance(tmpptCol, amobilePoint);
                const vec_t& aip = mAIPoints[lpidx].mAIPos[j];
                float nDistRail = SquaredDistance(aip, mobilePoint);
                if ( (nDist<oocurdist) && (nDistRail < railFourcheDist) )
                {
                    if ( nDist < 400.f) // spatial coherency can't be 20units further than track point
                    {
                    railFourcheDist = nDistRail;
                    oocurdist = nDist;
                    oocurpt = lpidx;
                    ptCol = tmpptCol;
                    aaCurFourche = j;
                    }
                }
            }
        }
    }
	if (oocurpt == -1)
	{
		// pb pour sly N°1
		PROFILER_END(); // GetClosestSampledPoint
		return false;
		//DebugBreak();
	}



    //LOG("curf : %d\n", aaCurFourche);
	aaCurIdx = oocurpt;
	localIdx = oocurpt;
	//i = 0;

	float nDistp1=9999999999.f, nDistm1=9999999999.f;
	vec_t ptColp1 = vec_t::zero, ptColm1 = vec_t::zero;
	// get neighbour segs
	bool nneigfound = false;
	int neidDecal = 1;

    //int hookFoundOnFourche = 0;
	while ((!nneigfound)&&(neidDecal<20))
	{
        int lpidx1 = (localIdx+neidDecal)%loopSC;
        int lpidx2 = (localIdx-neidDecal+loopSC)%loopSC;
        //int count1 = mAIPoints[lpidx1].hasFourche?2:1;
        //int count2 = mAIPoints[lpidx2].hasFourche?2:1;
        int j = aaCurFourche;
        // passe 1
        //for (int j = 0 ; j <count1 ; j++)
        {
            vec_t tv1 = projectWithGrav(mAIPoints[lpidx1].mRailRight[j]);
            vec_t tv2 = projectWithGrav(mAIPoints[lpidx1].mRailLeft[j]);


            if (CollisionClosestPointOnSegment( amobilePoint, tv1, tv2, ptColp1 ))
            {
                nDistp1 = SquaredDistance(ptColp1, amobilePoint);
                nneigfound = true;
                //hookFoundOnFourche = (j>0)?1:0;
            }
        }
        // passe 2
        //for (int j = 0 ; j < count2 ; j++)
        {

            vec_t tv1 = projectWithGrav(mAIPoints[lpidx2].mRailRight[j]);
            vec_t tv2 = projectWithGrav(mAIPoints[lpidx2].mRailLeft[j]);


            if (CollisionClosestPointOnSegment( amobilePoint, tv1, tv2, ptColm1 ))
            {
                nDistm1 = SquaredDistance(ptColm1, amobilePoint);
                nneigfound = true;
                //hookFoundOnFourche = (j>0)?1:0;
            }
        }
		if (!nneigfound)
			neidDecal++;

	}

	if (!nneigfound)
	{
		// pb pour sly N°2
		//DebugBreak();
		PROFILER_END(); // GetClosestSampledPoint
		return false;
	}


	int idx2;
	float ndistrt;
	vec_t ptcolision;
	//tvector4 gqt, gqt2;
	bool bDirInv = false;
	if (nDistp1<nDistm1)
	{
		idx2 = ((localIdx+neidDecal)%loopSC);
		ndistrt = sqrtf(nDistp1);
		ptcolision = ptColp1;

	}
	else
	{
		idx2 = ((localIdx-neidDecal+loopSC)%loopSC);
		ndistrt = sqrtf(nDistm1);
		ptcolision = ptColm1;
		bDirInv  =true;
	}
	//gqt = samplesRot[oocurpt];
	//gqt2 = samplesRot[idx2];
	// lerp1
	float lerp1= 0.f;//Distance(ptcolision, ptCol);
	lerp1 = sqrtf(oocurdist)/Distance(ptcolision, ptCol);

	// --

    int hookFoundOnFourche = aaCurFourche;
	// lerp2
    int ausedIdx = (localIdx)%loopSC;
	vec_t vtlong = projectWithGrav(mAIPoints[ausedIdx].mRailLeft[hookFoundOnFourche] - mAIPoints[ausedIdx].mRailRight[hookFoundOnFourche]);
	vec_t vtcourt = ptCol- projectWithGrav(mAIPoints[ausedIdx].mRailRight[hookFoundOnFourche]);

	float lerp2 = vtcourt.length()/ vtlong.length();
	rightLeftFactor = lerp2;
	//LOG("%5.4f\r\n",rightLeftFactor);
	/*
     if ((lerp1<0)||(lerp1>1)||(lerp2<0)||(lerp2>1))
     {
     //DebugBreak();
     return false;
     }
     */
	// quaternion avec le lerp1
	// position avec bilerp
	// le up avec lerp1 entre les mAIUp
	// dir =vecteur entre les 2
	// point on track = lerp1

	//tvector4 goodQt;
	vec_t goodUp, goodDir, goodPointOnTrack, goodProjectedPosition;
	//goodQt.Lerp(gqt, gqt2, lerp1);
	goodUp.lerp(mAIPoints[oocurpt].mAIUp[hookFoundOnFourche], mAIPoints[idx2].mAIUp[hookFoundOnFourche], lerp1);

    vec_t d1,d2;
	if (bDirInv)
	{
		goodDir = mAIPoints[oocurpt].mAIPos[hookFoundOnFourche] - mAIPoints[idx2].mAIPos[hookFoundOnFourche];
        d2 = mAIPoints[oocurpt].mAIPos[hookFoundOnFourche];
        d1 = mAIPoints[idx2].mAIPos[hookFoundOnFourche];

	}
	else
	{
		goodDir = mAIPoints[idx2].mAIPos[hookFoundOnFourche] - mAIPoints[oocurpt].mAIPos[hookFoundOnFourche];
        d2 = mAIPoints[idx2].mAIPos[hookFoundOnFourche];
        d1 = mAIPoints[oocurpt].mAIPos[hookFoundOnFourche];
	}
	goodDir.normalize();

	goodPointOnTrack.lerp(mAIPoints[oocurpt].mAIPos[hookFoundOnFourche],
                          mAIPoints[idx2].mAIPos[hookFoundOnFourche],
                          lerp1);

	goodProjectedPosition.lerp( LERP(mAIPoints[oocurpt].mRailRight[hookFoundOnFourche],
                                     mAIPoints[oocurpt].mRailLeft[hookFoundOnFourche],
                                     lerp2),
                               LERP(mAIPoints[idx2].mRailRight[hookFoundOnFourche],
                                    mAIPoints[idx2].mRailLeft[hookFoundOnFourche],
                                    lerp2),
                               lerp1);

	upDownFactor = lerp1;

	vec_t goodRight;
	/*
     goodRight.Cross(goodDir, goodUp);
     goodRight.Normalize();
     */
	goodRight = LERP((mAIPoints[oocurpt].mRailRight[hookFoundOnFourche]- mAIPoints[oocurpt].mRailLeft[hookFoundOnFourche]),
                     (mAIPoints[idx2].mRailRight[hookFoundOnFourche]- mAIPoints[idx2].mRailLeft[hookFoundOnFourche]),
                     lerp1);
	goodRight.normalize();

	trackMatrix.dir = goodDir;//LERP( d2, d1, upDownFactor);
    //trackMatrix.dir.normalize();
	trackMatrix.right = goodRight;
	/*trackMatrix.up.cross( trackMatrix.right, trackMatrix.dir );//  = goodUp;
    trackMatrix.up.w = 0.f;
    trackMatrix.up.normalize();
    */
    trackMatrix.up = goodUp;
	trackMatrix.position = goodProjectedPosition;
	trackMatrix.m16[15] = 1;
    trackMatrix.orthoNormalize();

	// RAYCAST

	trackMiddle = goodPointOnTrack;

	float trackWith = LERP( (mAIPoints[oocurpt].mRailRight[hookFoundOnFourche]-mAIPoints[oocurpt].mRailLeft[hookFoundOnFourche]).length(),

                           (mAIPoints[idx2].mRailRight[hookFoundOnFourche]-mAIPoints[idx2].mRailLeft[hookFoundOnFourche]).length(), lerp1);

	static const float tolerance = 0.03f;
	if (rightLeftFactor>(1.f-tolerance))
	{
		distanceToBorder = trackWith*(tolerance-(1.f-(rightLeftFactor)));
	}
	else if (rightLeftFactor<tolerance)
	{
		distanceToBorder = trackWith*fabsf(rightLeftFactor-tolerance);
	}

	PROFILER_END(); // GetClosestSampledPoint


	//LOG("TTak = %5.5f \n", gTimer.ForceGetTime() - tlagStart);

	return true;
}



void TrackTopology::FillTopologyWithSequence( bool bFourche, shapeSequence_t *pSeq, int start, int count )
{
	std::vector<trackTopology_t>& vv = mCurrentToopology[bFourche?1:0];

    int nbNoRepeat = 0;
    int nbRepeat = 0;
    for (int i=0;i<pSeq->nbEntries;i++)
    {
        if (!pSeq->entries[i].bRepeatable)
            nbNoRepeat++;
        if (i && pSeq->entries[i].bRepeatable && (!pSeq->entries[i-1].bRepeatable) )
            nbRepeat++;

    }

    int nbPerRepeat, closeRepeat;

	if (nbRepeat)
	{
		nbPerRepeat = (count-nbNoRepeat)/nbRepeat;
		closeRepeat = (count-nbNoRepeat)%nbRepeat;
	}
	else
	{
		nbPerRepeat = 0;
		closeRepeat = 0;
	}
    int av = 0;
    int previousRepeatBegin = 0;
    int countRepeat = 1;
    for (int i=0;i<count;)
    {
        if (av && pSeq->entries[av].bRepeatable && (! pSeq->entries[av-1].bRepeatable))
        {
            previousRepeatBegin = av;
            countRepeat = i+nbPerRepeat+closeRepeat;
            closeRepeat = 0;
        }
        else
            countRepeat = i+1;

        int tmpav = av;
        for (;i<countRepeat;i++)
        {
            vv[i+start].pattern = pSeq->entries[tmpav].pattern;
            vv[i+start].trackSliceProperties |= (pSeq->isTransparent?TOPO_TRANSPARENT:0);
            if (!pSeq->entries[++tmpav].bRepeatable)
                tmpav = av;
        }
        if (pSeq->entries[av].bRepeatable)
            while (pSeq->entries[++av].bRepeatable);
        else av++;
    }
}


void TrackTopology::BuildTopology( bool bFourche, const matrix_t *pMats, int nbMats, PerSliceInfo_t *psi, track_t& tr)
{
	std::vector<trackTopology_t>& vv = mCurrentToopology[bFourche?1:0];
	/*
	vv.resize( nbMats );
	memset ( &vv[0], 0, sizeof(trackTopology_t) * nbMats );
	*/
	/*
			TOPO_INGROUND = 1 << 0,
		TOPO_HOLE = 1 << 1,
		TOPO_HIGHSLOPE = 1 << 2,
		TOPO_FOURCHE = 1 << 3,
		TOPO_STRAIGHT = 1 << 4,
		TOPO_STARTUP = 1 << 5,
		*/
    //u32 randomColors[] = { 0xFFF7D99D,	0xFFB9A986,	0xFFA07DA3,	0xFFFBE4B6,	0xFFFBEAC9};//0xFFFF8E00,	0xFFBF8030,	0xFFA65C00,	0xFFFFAA40,	0xFFFFC173 };
    u32 randomColors[] = { tr.color1, tr.color2, tr.color3, tr.color4, tr.color5 };
	float distance = 0.f;
	float distAv = tr.trackLength / (float)nbMats;
	for (int i=0;i<nbMats;i++)
	{
        int prevIdx = (i-1+nbMats)%nbMats;

		if (pMats[i].position.y <0.f)
			vv[i].trackSliceProperties |= TOPO_INGROUND;
		if (bFourche && (pMats[i].position.w < FLOAT_EPSILON))
			vv[i].trackSliceProperties |= TOPO_FOURCHE;

		float distToHole = tr.shortestDistanceToHoleForthAndBack(distance);
		if (distToHole <40.f)
		{
			vv[i].trackSliceProperties |= TOPO_HOLEINOUT;
			if (distToHole < FLOAT_EPSILON)
				vv[i].trackSliceProperties |= TOPO_HOLE;
		}


        vv[i].leftRight = pMats[prevIdx].dir.dot( pMats[i].right );//, upDown


        float straightValue = pMats[i].dir.dot(pMats[(i+1)%nbMats].dir);
		if ( straightValue >0.99f)
			vv[i].trackSliceProperties |= TOPO_STRAIGHT;


        float strRaw = ((1.f-straightValue) * 500.f);
        float strClamped = Clamp( strRaw, 0.f, 1.f );
        float strStepped = smootherstep(0.25, 0.75, strClamped );
        //printf("Straight : %5.4f / %5.4f / %5.4f \n", strRaw, strClamped, strStepped );
        vv[i].wallColorRedAlpha = strStepped*0.66f + 0.33f;

		if (i<16)
			vv[i].trackSliceProperties |= TOPO_STARTUP;
		distance += distAv;

        if ( vv[i].pattern != vv[prevIdx].pattern )
        {
            vv[i].colorLight = vec( randomColors[fastrand()%5] ).swapedRB();
        }

        // width
        vv[i].width = psi[i].width;
        vv[i].borderForced = psi[i].borderForced;

        if (vv[i].borderForced > 0.1f)
            vv[i].trackSliceProperties |= TOPO_BORDERFORCED;
	}

    // colors
    bool bPreviouslyTurning = false;
    int avRandom = 0;
    for (int i=0;i<nbMats;i++)
    {
        bool isturning = (fabsf(vv[i].leftRight)<0.05f);
        if (!isturning)
        {
            vv[i].trackSliceProperties |= (vv[i].leftRight<0)?TOPO_TURNING_LEFT:TOPO_TURNING_RIGHT;
            /*
        TOPO_GETTING_UP = 1<<11,
        TOPO_GETTING_DOWN = 1<<12;
        */

        }
        if ( bPreviouslyTurning != isturning )
        {
            vv[i].colorLight = vec( randomColors[avRandom] ).swapedRB();
            ++avRandom %= 5;
        }

        bPreviouslyTurning = isturning;
        //vv[i].color = vec( randomColors[0] ).swapedRB();//(?vec(0.2f, 0.2f, 0.2f, 0.5f):vec(0.8f, 0.8f, 0.8f, 0.5f);
    }
    // holes and start
    for ( int i = 0 ; i < nbMats ; i++)
    {
        if (vv[i].trackSliceProperties&TOPO_HOLE)
        {
            vv[i].colorLight = vec(1.f, 0.f, 0.f, 1.f);
        }
        if (vv[i].trackSliceProperties&TOPO_STARTUP)
        {
            vv[i].colorLight = vec(1.f, 0.9f, 0.9f, 1.f);
        }
    }


    for ( int i = 0 ; i < nbMats ; i++)
    {
        if (vv[i].colorLight.w>0.5f)
        {
            vec_t col1 = vv[i].colorLight;
            int j;
            for ( j = i + 1 ; j < nbMats ; j++)
            {
                if ( vv[j].colorLight.w > 0.5f )
                {
                    vec_t col2 = vv[j].colorLight;

                    float len = 1.f/(float)(j-i);
                    float av = 0.f;
                    for (int k = i ; k < j ; k++, av += len )
                    {
                        vv[k].colorLight.lerp( col1, col2, av );
                    }
                    i = j-1;
                    j = nbMats;
                }
            }
        }
    }
    // apply color toggle
    for ( int i = 0 ; i < nbMats ; i++)
    {
        float fact = 1.f;
        bool isturning = (fabsf(vv[i].leftRight)<0.05f);

        //if (i&1)
        {
            fact = isturning?0.5f:0.25f;
        }

        vv[i].colorLight.w = 1.f;
        vv[i].colorDark = vv[i].colorLight;
        vv[i].colorDark.x *= fact;
        vv[i].colorDark.y *= fact;
        vv[i].colorDark.z *= fact;

    }
}
#if 0
void Track::progressMesh( float distance, float length)
{
    return;
	for (unsigned int i = 0;i< mBuildProgress.size(); i++)
	{
		const buildProgress_t& prg = mBuildProgress[i];

		float factor;
        if ( length < 0.f)
        {
            factor = 1.f;
        }
        else
        {
            factor = ( (mBuildProgress[i].distance - distance) / length );
		    factor = 1.f - Clamp( factor, 0.f, 1.f );
        }

		if ( prg.bUseWorldMat )
		{
			matrix_t localScale;
			localScale.scale( factor );
			prg.pMesh->mWorldMatrix = localScale * prg.mWorldMat;
		}
		else
		{
			matrix_t localScale, localTrans1, localTrans2, local;
			localScale.scale( factor );
			localTrans1.translation( -prg.pivot );
			localTrans2.translation( prg.pivot );
			local = localTrans1 * localScale * localTrans2;
			prg.pMesh->mWorldMatrix = local;
		}
		prg.pMesh->updateWorldBSphere();
	}
}
#endif



mesh_t* Track::generatePortalMesh( int brickStart, int brickCount, bool forward )
{
	mesh_t *pm = new mesh_t;

	int nbIdx = brickCount * 6 ;
	int nbVts = (brickCount + 1) * 2 ;

	pm->mIA->Init( nbIdx, VAU_STATIC );
	pm->mVA->Init( VAF_XYZ|VAF_NORMAL|VAF_TEX0|VAF_COLOR, nbVts, true, VAU_STATIC );

	pm->triCount = nbIdx;

	meshColorUVVertex_t *pv = (meshColorUVVertex_t *)pm->mVA->Lock(VAL_WRITE);

	// vertices
	int k = 0;
	for (int i=brickStart;i<=(brickStart+brickCount);i++,k++)
	{
		const AIPoint_t& pt = getAIPoint(i);
		float v = (float)k/(float)brickCount;
		float zeroIt = v * ((float)(brickCount-k)/(float)brickCount) * 10.f;
		zeroIt = zmin(zeroIt, 0.5f);
		static const float minHeight = 0.3f;
		static const float maxHeight = 1.3f;
		float mulit = zeroIt;//(zeroIt?0.5f:(j?1.f:0.f));
		vec_t pos = pt.mAIPos[0] + pt.mAIUp[0] * ( minHeight + (maxHeight-minHeight) * (0.5f + mulit ) );

		float texv = (float)k*0.5f;
		if (!forward) 
			texv =-texv;
		pv->set( pos/*+pt.mAIRight[0]*10.f*/, pt.mAIRight[0], 0xFFFFFFFF, vec( 0.f, texv ) );
		pv ++;
		pv->set( pos, pt.mAIRight[0], 0xFFFFFFFF, vec( 1.f, texv ) );
		pv ++;

	}
	
	// indices
	unsigned short *pi = (unsigned short*)pm->mIA->Lock(VAL_WRITE);
	unsigned short av = 0;

	for (int i=0;i<brickCount;i++)
	{
		*pi++ = 0 + av;
		*pi++ = 1 + av;
		*pi++ = 3 + av;

		*pi++ = 0 + av;
		*pi++ = 3 + av;
		*pi++ = 2 + av;
		av += 2;
	}


	pm->mIA->Unlock();
	pm->mVA->Unlock();

	pm->visible = false;
	pm->color = forward?vec( 0.f, 0.f, 1.f, 1.0f ):vec( 1.f, 0.f, 1.f, 1.0f );
	pm->computeBSphere();
	pm->mWorldMatrix.identity();
	pm->updateWorldBSphere();

	return pm;
}