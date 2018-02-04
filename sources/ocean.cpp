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
#include "ocean.h"
#include "render.h"

#include "include_GL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#define QUAD_PIPE FALSE
#define PACKED_NOISE TRUE

oceanParameters_t::oceanParameters_t()
{
	mfStrength=	11.3f;
	mbDisplace=	true ;
	miOctaves=		8 ;
	mfScale=		0.38f ;
	mfFalloff=		0.56f;
	mfAnimspeed=	1.4f ;
	mfTimemulti=	1.27f ;
	mbPaused=		false ;
	mfLODbias=		0.0f;
	mbDisplayTargets= false ;
	mfElevation=	7.0f ;
    
	mfSunPosAlpha=		2.38f ;
	mfSunPosTheta=		0.256f ;
	mfSunShininess=	150.0f ;
	mfSunStrength=		10.85f ;
    
	mbSmooth=			false ;
	mbReflRefrStrength = 0.1f ;
	mfWaterColourR=	0.35f;
	mfWaterColourG=	0.54f;
	mfWaterColourB=	0.52f;
	mbDiffuseRefl=		false ;
}

oceanParameters_t oceanParameters;

OceanCamera oceanCamera;

OceanCamera::OceanCamera(){
	position = vec(0,0,0);
	fov = PI*0.5f;
	aspect = 1.0f;
	znear = 0.1f;
	zfar = 1000.f;
	rotx = 0;
	roty = 0;
	rotz = 0.5;
	this->update();
}

OceanCamera::OceanCamera(const vec_t& pos, float rotx, float roty, float rotz, float fov, float aspect, float nearz, float farz){
	this->position = pos;
	this->fov = fov;
	this->aspect = aspect;
	this->znear = nearz;
	this->zfar = farz;
	this->rotx = rotx;
	this->roty = roty;
	this->rotz = rotz;
	this->update();
}

OceanCamera::OceanCamera(const OceanCamera *src){
	this->position	= src->position;
	this->fov		= src->fov;
	this->aspect	= src->aspect;
	this->znear		= src->znear;
	this->zfar		= src->zfar;
	this->rotx		= src->rotx;
	this->roty		= src->roty;
	this->rotz		= src->rotz;
	this->update();
}

OceanCamera::~OceanCamera()
{
	// nothing funny here
}

void OceanCamera::update()
{
    
	matrix_t rotatex,rotatey,rotatez,translation;
	// perspective matrix
	proj.PerspectiveFovLH2(fov, aspect, znear, zfar);
	// view matrix
	rotatex.rotationX(rotx);
	rotatey.rotationX(roty);
	rotatez.rotationX(rotz);
	//D3DXMatrixTranslation(&translation,-position.x,-position.y,-position.z);
	translation.translation(-position.x,-position.y,-position.z);
	view = translation * rotatey*rotatex*rotatez;
	// and finally the combined viewproj
	viewproj = view*proj;
	// and all the inverses
	invproj.inverse(proj);
	invview.inverse(view);
	invviewproj.inverse(viewproj);
	// and the direction vectors
	//D3DXVec3TransformNormal(&forward,&D3DXVECTOR3(0,0,1),&invview);
	//D3DXVec3TransformNormal(&up,&D3DXVECTOR3(0,1,0),&invview);
	//D3DXVec3TransformNormal(&right,&D3DXVECTOR3(1,0,0),&invview);
	forward.TransformVector(ZAxis, invview);
	up.TransformVector(YAxis, invview);
	right.TransformVector(XAxis, invview);
    
}

void OceanCamera::update( const matrix_t &aview, const matrix_t &aproj,
                         const matrix_t &ainvView, const matrix_t &ainvProj )
{
    UNUSED_PARAMETER(ainvView);
    UNUSED_PARAMETER(ainvProj);

	view = aview;
	proj = aproj;
	// and finally the combined viewproj
	invview.inverse(view);
	/*
     D3DXVec3TransformCoord(&position,&D3DXVECTOR3(0,0,0),&invview);
     D3DXVec3TransformNormal(&forward,&D3DXVECTOR3(0,0,1),&invview);
     D3DXVec3TransformNormal(&up,&D3DXVECTOR3(0,1,0),&invview);
     D3DXVec3TransformNormal(&right,&D3DXVECTOR3(1,0,0),&invview);
     D3DXMatrixLookAtLH( &view, &position,&(position+forward), &D3DXVECTOR3(0,1,0));
     */
	position.TransformPoint(vec(0.f, 0.f,0.f, 1.f), invview);
	position.w = 0.f;
	forward.TransformVector(ZAxis, invview);
	up.TransformVector(YAxis, invview);
	right.TransformVector(XAxis, invview);
	view.lookAtLH(position, position+forward, YAxis);
    
    
    
	viewproj = view*proj;
	// and all the inverses
	/*
     D3DXMatrixInverse(&invproj,NULL,&proj);
     D3DXMatrixInverse(&invview,NULL,&view);
     D3DXMatrixInverse(&invviewproj,NULL,&viewproj);
     */
	invproj.inverse(proj);
	invview.inverse(view);
	invviewproj.inverse(viewproj);
	// and the direction vectors
	/*
     D3DXVec3TransformCoord(&position,&D3DXVECTOR3(0,0,0),&invview);
     D3DXVec3TransformNormal(&forward,&D3DXVECTOR3(0,0,1),&invview);
     D3DXVec3TransformNormal(&up,&D3DXVECTOR3(0,1,0),&invview);
     D3DXVec3TransformNormal(&right,&D3DXVECTOR3(1,0,0),&invview);
     */
	position.TransformPoint(vec(0.f, 0.f,0.f, 1.f), invview);
	forward.TransformVector(ZAxis, invview);
	up.TransformVector(YAxis, invview);
	right.TransformVector(XAxis, invview);
	position.w = 0.f;

	return;

    //FIXME: warning C4702: unreachable code
#if 0
    matrix_t view, invview, proj, invproj, viewproj, invviewproj;
	
    view = aview;
    invview = ainvView;
    proj = aproj;
    invproj = ainvProj;
    
    viewproj = view * proj;
    invviewproj = invview * invproj;
    
    forward = ainvView.dir;
	up = ainvView.up;
	right = ainvView.right;
	position = ainvView.position;
    
    
	forward.w = 0.f;
	up.w = 0.f;
	right.w = 0.f;
	position.w = 0.f;
    
	return;
#endif

    //FIXME: warning C4702: unreachable code
#if 0
	/*
     D3DXMATRIXA16 rotatex,rotatey,rotatez,translation;
     // perspective matrix
     D3DXMatrixPerspectiveFovLH(&proj, fov, aspect, znear, zfar);
     // view matrix
     D3DXMatrixRotationX(&rotatex,rotx);
     D3DXMatrixRotationY(&rotatey,roty);
     D3DXMatrixRotationZ(&rotatez,rotz);
     D3DXMatrixTranslation(&translation,-position.x,-position.y,-position.z);
     view = translation * rotatey*rotatex*rotatez;
     */
    //	RenderView = aview;
    //	RenderViewProj = aview * aproj;
	view = aview;
	proj = aproj;
	// and finally the combined viewproj
	invview.inverse(view);
    
	viewproj = view*proj;
	invproj.inverse(proj);
	//invview.inverse(view);
	invviewproj.inverse(viewproj);
    
    
	position = invview.position;
	forward = invview.dir;
	up = invview.up;
	right = invview.right;
#endif
}

// set all parameters assuming position, forward & all the perspective shit are correct
void OceanCamera::update_lookat()
{
    
	view.lookAtLH(position, position+forward, up);
    
	viewproj = view*proj;
    
	invproj.inverse(proj);
	invview.inverse(view);
	invviewproj.inverse(viewproj);
    
	right.TransformVector(XAxis, invview);
    
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void OceanNoise::init(ZVertexArrayOGL *pVA, int sX, int sY)
{
	mVA = pVA;
	this->sizeX = sX;
	this->sizeY = sY;
    
	time = 0.0;
	last_time = 0;//GetTimer()->GetTime(); //timeGetTime();
	octaves = 0;	// don't want to have the noise accessed before it's calculated
    
	f_sizeX = (float) sizeX;
	f_sizeY = (float) sizeY;
	
	// reset normals
	mVA->Init( VAF_XYZ|VAF_NORMAL|VAF_TEX0, sizeY*sizeX, true, VAU_DYNAMIC );
	SOFTWARESURFACEVERTEX *vertices	= (SOFTWARESURFACEVERTEX*)mVA->Lock( VAL_WRITE );
	for(int v=0; v<sizeY; v++)
	{
		for(int u=0; u<sizeX; u++)
		{
			vertices[v*sizeX + u].nx =	0.0f;
			vertices[v*sizeX + u].ny =	1.0f;
			vertices[v*sizeX + u].nz =	0.0f;
			vertices[v*sizeX + u].tu = (float)u/(sizeX-1);
			vertices[v*sizeX + u].tv = (float)v/(sizeY-1);
		}
	}
	mVA->Unlock();
	this->init_noise();
}

void OceanNoise::init_noise()
{
	// create noise (uniform)
	float* tempnoise = new float [n_size_sq*noise_frames];
	for(int i=0; i<(n_size_sq*noise_frames); i++)
	{
		//this->noise[i] = rand()&0x0000FFFF;
		float temp = (float) rand()/RAND_MAX;
		tempnoise[i] = 4*(temp - 0.5f);
	}
    
	for(int frame=0; frame<noise_frames; frame++)
	{
		for(int v=0; v<n_size; v++)
		{
			for(int u=0; u<n_size; u++)
			{
				/*float temp = 0.25f * (tempnoise[frame*n_size_sq + v*n_size + u] +
                 tempnoise[frame*n_size_sq + v*n_size + ((u+1)&n_size_m1)] +
                 tempnoise[frame*n_size_sq + ((v+1)&n_size_m1)*n_size + u] +
                 tempnoise[frame*n_size_sq + ((v+1)&n_size_m1)*n_size + ((u+1)&n_size_m1)]);*/
				int v0 = ((v-1)&n_size_m1)*n_size,
                v1 = v*n_size,
                v2 = ((v+1)&n_size_m1)*n_size,
                u0 = ((u-1)&n_size_m1),
                u1 = u,
                u2 = ((u+1)&n_size_m1),
                f  = frame*n_size_sq;
				float temp = (1.0f/14.0f) * (	tempnoise[f + v0 + u0] + tempnoise[f + v0 + u1] + tempnoise[f + v0 + u2] +
                                             tempnoise[f + v1 + u0] + 6.0f*tempnoise[f + v1 + u1] + tempnoise[f + v1 + u2] +
                                             tempnoise[f + v2 + u0] + tempnoise[f + v2 + u1] + tempnoise[f + v2 + u2]);
                
				this->noise[frame*n_size_sq + v*n_size + u] = int(noise_magnitude*temp);
			}
		}
	}
    
    delete[] tempnoise;
    tempnoise = NULL;
}

void OceanNoise::calc_noise( float aTimeEllapsed )
{
	octaves = zmin(oceanParameters.miOctaves, max_octaves);
    
	// calculate the strength of each octave
	float sum=0.0f;
	for(int i=0; i<octaves; i++)
	{
		f_multitable[i] = powf(oceanParameters.mfFalloff,1.0f*i);
		sum += f_multitable[i];
	}
    
	{
		for(int i=0; i<octaves; i++)
		{
			f_multitable[i] /= sum;
		}}
    
	{
		for(int i=0; i<octaves; i++)
		{
			multitable[i] = int(scale_magnitude*f_multitable[i]);
		}}
    
    
	//float this_time = GetTimer()->GetTime();//timeGetTime();
	double itime = aTimeEllapsed * 1000.f;//16.6f;//this_time - last_time;
	static double lp_itime=0.0;
	//last_time = this_time;
	itime *= 0.001 * oceanParameters.mfAnimspeed ;
	lp_itime = 0.99*lp_itime + 0.01 * itime;
	if ( !oceanParameters.mbPaused )
		time += lp_itime;
    
    
	double	r_timemulti = 1.0;
    
	for(int o=0; o<octaves; o++)
	{
		unsigned int image[3];
		int amount[3];
		double dImage, fraction = modf(time*r_timemulti,&dImage);
		int iImage = (int)dImage;
		amount[0] = int(scale_magnitude*f_multitable[o]*(pow(sin((fraction+2)*PI/3),2)/1.5));
		amount[1] = int(scale_magnitude*f_multitable[o]*(pow(sin((fraction+1)*PI/3),2)/1.5));
		amount[2] = int(scale_magnitude*f_multitable[o]*(pow(sin((fraction)*PI/3),2)/1.5));
		image[0] = (iImage) & noise_frames_m1;
		image[1] = (iImage+1) & noise_frames_m1;
		image[2] = (iImage+2) & noise_frames_m1;
		{
			for(int i=0; i<n_size_sq; i++)
			{
				o_noise[i + n_size_sq*o] =	(	((amount[0] * noise[i + n_size_sq * image[0]])>>scale_decimalbits) +
                                             ((amount[1] * noise[i + n_size_sq * image[1]])>>scale_decimalbits) +
                                             ((amount[2] * noise[i + n_size_sq * image[2]])>>scale_decimalbits));
			}
		}
        
		r_timemulti *= oceanParameters.mfTimemulti ;
	}
    
#if	PACKED_NOISE
	{
		int octavepack = 0;
		for(int o=0; o<octaves; o+=n_packsize)
		{
			for(int v=0; v<np_size; v++)
				for(int u=0; u<np_size; u++)
				{
					p_noise[v*np_size+u+octavepack*np_size_sq] = o_noise[(o+3)*n_size_sq + (v&n_size_m1)*n_size + (u&n_size_m1)];
					p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 3, o);
					p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 2, o+1);
					p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 1, o+2);
				}
            octavepack++;
            
            /*for(int v=0; v<20; v++)
             for(int u=0; u<20; u++)
             p_noise[v*np_size+u] = 1000;*/
            // debug box
            
		}
	}
#endif  //  PACKED_NOISE
}

inline int OceanNoise::mapsample(int u, int v, int upsamplepower, int octave)
{
	int magnitude = 1<<upsamplepower;
	int pu = u >> upsamplepower;
	int pv = v >> upsamplepower;
	int fu = u & (magnitude-1);
	int fv = v & (magnitude-1);
	int fu_m = magnitude - fu;
	int fv_m = magnitude - fv;
    
	int o = fu_m*fv_m*o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu)&n_size_m1)] +
    fu*fv_m*o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu+1)&n_size_m1)] +
    fu_m*fv*o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu)&n_size_m1)] +
    fu*fv*o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu+1)&n_size_m1)];
    
	return o >> (upsamplepower+upsamplepower);
}



bool OceanNoise::render_geometry( float aTimeEllapsed, const matrix_t *m)
{
	SOFTWARESURFACEVERTEX *vertices	= (SOFTWARESURFACEVERTEX*)mVA->Lock( VAL_WRITE );
    
    
	this->calc_noise( aTimeEllapsed );
    
	float magnitude = n_dec_magn * oceanParameters.mfScale;
	//float inv_magnitude_sq = 1.0f/(oceanParameters.mfScale*oceanParameters.mfScale);
    
	matrix_t m_inv;
	//D3DXMatrixInverse( &m_inv, NULL, m );
	m_inv.inverse(*m);
	e_u.TransformVector(XAxis, *m);
	e_v.TransformVector(YAxis, *m);
	//D3DXVec3TransformNormal( &e_u, &D3DXVECTOR3(1,0,0), m);
	//D3DXVec3TransformNormal( &e_v, &D3DXVECTOR3(0,1,0), m);
	//D3DXVec3Normalize( &e_u, &e_u );
	//D3DXVec3Normalize( &e_v, &e_v );
	e_u.normalize();
	e_v.normalize();
    
    
	t_corners0 = this->calc_worldpos(vec(0.0f,0.0f),m);
	t_corners1 = this->calc_worldpos(vec(+1.0f,0.0f),m);
	t_corners2 = this->calc_worldpos(vec(0.0f,+1.0f),m);
	t_corners3 = this->calc_worldpos(vec(+1.0f,+1.0f),m);
    
	matrix_t surface_to_world;
    
    
	float	du = 1.0f/float(sizeX-1),
    dv = 1.0f/float(sizeY-1),
    u,v=0.0f;
	vec_t result;
	int i=0;
	for(int iv=0; iv<sizeY; iv++)
	{
		u = 0.0f;
		for(int iu=0; iu<sizeX; iu++)
		{
            
			//result = (1.0f-v)*( (1.0f-u)*t_corners0 + u*t_corners1 ) + v*( (1.0f-u)*t_corners2 + u*t_corners3 );
			result.x = (1.0f-v)*( (1.0f-u)*t_corners0.x + u*t_corners1.x ) + v*( (1.0f-u)*t_corners2.x + u*t_corners3.x );
			result.z = (1.0f-v)*( (1.0f-u)*t_corners0.z + u*t_corners1.z ) + v*( (1.0f-u)*t_corners2.z + u*t_corners3.z );
			result.w = (1.0f-v)*( (1.0f-u)*t_corners0.w + u*t_corners1.w ) + v*( (1.0f-u)*t_corners2.w + u*t_corners3.w );
            
			float divide = 1.0f/result.w;
			result.x *= divide;
			result.z *= divide;
            
			vertices[i].x = result.x;
			vertices[i].z = result.z;
			//vertices[i].y = get_height_at(magnitude*result.x, magnitude*result.z, octaves);
			vertices[i].y = get_height_dual((int)(magnitude*result.x), int(magnitude*result.z), oceanParameters.mfStrength);
            
			i++;
			u += du;
		}
		v += dv;
	}
    
	// smooth the heightdata
	if(oceanParameters.mbSmooth)
	{
		//for(int n=0; n<3; n++)
		for(int iv=1; iv<(sizeY-1); iv++)
		{
			for(int iu=1; iu<(sizeX-1); iu++)
			{
				vertices[iv*sizeX + iu].y =	0.2f * (vertices[iv*sizeX + iu].y +
                                                    vertices[iv*sizeX + (iu+1)].y +
                                                    vertices[iv*sizeX + (iu-1)].y +
                                                    vertices[(iv+1)*sizeX + iu].y +
                                                    vertices[(iv-1)*sizeX + iu].y);
			}
		}
	}
    
	if(!oceanParameters.mbDisplace)
	{
		// reset height to 0
		for(int iu=0; iu<(sizeX*sizeY); iu++)
		{
			vertices[iu].y = 0;
		}
        
	}
    
	PROFILER_START(CalcNormals);
	calc_normals( vertices );
	PROFILER_END(); // CalcNormals
    
	mVA->Unlock();
	return true;
    
}



// check the point of intersection with the plane (0,1,0,0) and return the position in homogenous coordinates
vec_t OceanNoise::calc_worldpos(const vec_t& uv, const matrix_t *m)
{
	// this is hacky.. this does take care of the homogenous coordinates in a correct way,
	// but only when the plane lies at y=0
	vec_t	origin = vec(uv.x,uv.y,-1.f,1.f);
	vec_t	direction = vec(uv.x,uv.y,1.f,1.f);
    
	//D3DXVec4Transform( &origin, &origin, m );
	origin.transform(*m);
	//D3DXVec4Transform( &direction, &direction, m );
	direction.transform(*m);
	direction -= origin;
    
	float	l = -origin.y / direction.y;	// assumes the plane is y=0
    
	vec_t worldPos = origin;
	worldPos += (direction*l);
	return worldPos;
}

void OceanNoise::calc_normals( SOFTWARESURFACEVERTEX *vertices )
{
	for(int v=1; v<(sizeY-1); v++)
	{
		for(int u=1; u<(sizeX-1); u++)
		{
			vec_t vec1 = vec(	vertices[v*sizeX + u + 1].x-vertices[v*sizeX + u - 1].x,
                             vertices[v*sizeX + u + 1].y-vertices[v*sizeX + u - 1].y,
                             vertices[v*sizeX + u + 1].z-vertices[v*sizeX + u - 1].z);
            
			vec_t vec2 = vec(	vertices[(v+1)*sizeX + u].x - vertices[(v-1)*sizeX + u].x,
                             vertices[(v+1)*sizeX + u].y - vertices[(v-1)*sizeX + u].y,
                             vertices[(v+1)*sizeX + u].z - vertices[(v-1)*sizeX + u].z);
			vec_t normal;
			//D3DXVec3Cross( &normal, &vec2, &vec1 );
			normal.cross(vec2, vec1);
			vertices[v*sizeX + u].nx = normal.x;
			vertices[v*sizeX + u].ny = normal.y;
			vertices[v*sizeX + u].nz = normal.z;
            
		}
	}
}

inline int OceanNoise::readtexel_linear(int u, int v, int offset)
{
    
	int iu, iup, iv, ivp, fu, fv;
	iu = (u>>n_dec_bits)&n_size_m1;
	iv = ((v>>n_dec_bits)&n_size_m1)*n_size;
    
	iup = (((u>>n_dec_bits) + 1)&n_size_m1);
	ivp = (((v>>n_dec_bits) + 1)&n_size_m1)*n_size;
    
	fu = u & n_dec_magn_m1;
	fv = v & n_dec_magn_m1;
	/*float f_fu = (float) fu / n_dec_magn;
     float f_fv = (float) fv / n_dec_magn;*/
	/*float ut01 = ((n_dec_magn_m1-fu)*o_noise[iv + iu + offset] + fu*o_noise[iv + iup + offset]);
     float ut23 = ((n_dec_magn_m1-fu)*o_noise[ivp + iu + offset] + fu*o_noise[ivp + iup + offset]);*/
    
	int ut01 = ((n_dec_magn-fu)*o_noise[offset + iv + iu] + fu*o_noise[offset + iv + iup]) >> n_dec_bits;
	int ut23 = ((n_dec_magn-fu)*o_noise[offset + ivp + iu] + fu*o_noise[offset + ivp + iup]) >> n_dec_bits ;
	int ut = ((n_dec_magn-fv)*ut01 + fv*ut23) >> n_dec_bits;
	return ut;
}


inline float OceanNoise::get_height_at(int u, int v, int octaves)
{
	int value=0;
	//r_noise = o_noise;	// pointer to the current noise source octave
	for(int i=0; i<octaves; i++)
	{
		value += readtexel_linear(u,v,i*n_size_sq);
		u = u << 1;
		v = v << 1;
		//r_noise += n_size_sq;
	}
	return (float)(value)*oceanParameters.mfStrength / noise_magnitude;
}

float OceanNoise::get_height_at(float u, float v)
{
	float magnitude = n_dec_magn * oceanParameters.mfScale;
	//return get_height_at(magnitude*u, magnitude*v, octaves);
	return get_height_dual(int(magnitude*u), int(magnitude*v), oceanParameters.mfStrength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Ocean::Ocean(int size_x, int size_y, OceanCamera *arenderingcamera ) 
{
	mIA = new ZIndexArrayOGL;
	mVA = new ZVertexArrayOGL;
	
	software_brus.init(mVA,size_x+1,size_y+1);
    
	const vec_t _pos = vec(0.f, 0.f, 0.f);
	const vec_t _n = vec(0.f, 1.f, 0.f);
    
	plane = buildPlan(_pos, _n);
    
	normal = _n;
	normal.normalize();
	
	// calculate the u and v-vectors
	// take one of two vectors (the one further away from the normal) and force it into the plane
	vec_t x;
	
	if(fabs( XAxis.dot(normal)) < fabs(ZAxis.dot(normal)))
	{
		x = XAxis;
	} else {
		x = ZAxis;
	}
	u = x - normal*normal.dot(x);
	
	u.normalize();
	
	v.cross(u, normal);
	
	//	this->prm = prm;
	this->pos = _pos;
	this->gridsize_x = size_x+1;
	this->gridsize_y = size_y+1;
	this->rendering_camera = arenderingcamera;
    
    
	set_displacement_amplitude(0.0f);
    
	initbuffers();
    
	/*
     //camera cam(D3DXVECTOR3(0.f, 10.f, 0.f), -20.f, 0.f, 0.f, 90.f, 1.0f, 0.1f, 1000.f);
     //this->rendering_camera = cam;
     matrix_t bidon;
     this->rendering_camera->update(bidon, bidon, bidon, bidon);
     matrix_t range;
     this->getMinMax(&range);
     */
}

bool Ocean::initbuffers()
{
	int nbIdx = (gridsize_x-1)*(gridsize_y*2);
	mIA->Init( nbIdx, VAU_STATIC );
    
	unsigned short *pInd = (unsigned short*)mIA->Lock( VAL_WRITE );
	unsigned int idx = 0;
	for ( unsigned int x = 0 ; x < (unsigned int)(gridsize_x-1) ; x ++)
	{
		if (x&1)
		{
			// Descending
			for (unsigned int y = 0 ; y < (unsigned int)(gridsize_y) ; y ++)
			{
				*pInd++ = static_cast<unsigned short>( idx+1 );
				*pInd++ = static_cast<unsigned short>( idx );
				idx -= gridsize_x;
			}
			idx += (gridsize_x+1);
		}
		else
		{
			// Ascending
			for (unsigned int y = 0 ; y < (unsigned int)(gridsize_y) ; y ++)
			{
				*pInd++ = static_cast<unsigned short>( idx );
				*pInd++ = static_cast<unsigned short>( idx+1 );
				idx += gridsize_x;
			}
			idx -= (gridsize_x-1);
		}
	}
	mIA->Unlock();
    
	return true;
}
void Ocean::Draw()
{
	mIA->Bind();
	mVA->Bind();
    
	int nbIdx = (gridsize_x-1)*(gridsize_y*2);
	glDrawElements(	GL_TRIANGLE_STRIP, nbIdx, GL_UNSIGNED_SHORT, 0);
    
}

bool Ocean::within_frustum(const vec_t *pos)
{
	vec_t test;
	test.transform(vec(pos->x, pos->y, pos->z, 1.f), rendering_camera->viewproj);
	test.x /= test.w;
	test.y /= test.w;
	test.z /= test.w;
	//D3DXVec3TransformCoord(&test, pos, &(rendering_camera->viewproj));
	if((fabs(test.x) < 1.00001f)&&(fabs(test.y) < 1.00001f)&&(fabs(test.z) < 1.00001f))
		return true;
    
	return false;
}

float dispmulti(float dist){
	return zmax(0, zmin(1, dist-1));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool Ocean::PrepareVisibily(const OceanCamera *cam)
{
	//delete rendering_camera;
	rendering_camera = (OceanCamera *)cam;//new OceanCamera(cam);
    
	plane_within_frustum = this->getMinMax(&range);
	return true;
}

void Ocean::ComputeMesh( float aTimeEllapsed )
{
	/*
     if (!mVisible)
     {
     return ;
     }
     */
    
	PROFILER_START(OceanComputeMesh);
	if (plane_within_frustum)
	{
		software_brus.render_geometry( aTimeEllapsed, &range);
	}
	PROFILER_END(); // OceanComputeMesh
    
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// get the matrix that defines the minimum rectangle in which the frustum is located

void PlaneIntersectLine(vec_t * pout,
                        const vec_t * pp,
                        const vec_t * pv1,
                        const vec_t * pv2)
{
	vec_t direction, normal;
    float dot, temp;
    
    normal.x = pp->x;
    normal.y = pp->y;
    normal.z = pp->z;
	normal.w = 0.f;
    direction.x = pv2->x - pv1->x;
    direction.y = pv2->y - pv1->y;
    direction.z = pv2->z - pv1->z;
	direction.w = 0.f;
    
	dot = normal.dot(direction);
    if ( !dot ) 
		return ;
	temp = ( pp->w + Dot( normal, *pv1) ) / dot;
    pout->x = pv1->x - temp * direction.x;
    pout->y = pv1->y - temp * direction.y;
    pout->z = pv1->z - temp * direction.z;
	pout->w = 0.f;
}

float PlaneDotNormal(
                     const vec_t * pP,
                     const vec_t * pV
                     )
{
	return pP->x*pV->x + pP->y*pV->y + pP->z*pV->z;
}

float PlaneDotCoord(
                    const vec_t * pP,
                    const vec_t * pV
                    )
{
	return pP->x*pV->x + pP->y*pV->y + pP->z*pV->z + pP->w;
}


bool Ocean::getMinMax(matrix_t *range)
{
    
	set_displacement_amplitude(oceanParameters.mfStrength);
	
	vec_t frustum[8],proj_points[24];		// frustum to check the camera against
    
	int n_points=0;
	int cube[] = {	0,1,	0,2,	2,3,	1,3,
		0,4,	2,6,	3,7,	1,5,
		4,6,	4,5,	5,7,	6,7};	// which frustum points are connected together?
    
	// transform frustum points to worldspace (should be done to the rendering_camera because it's the interesting one)
	frustum[0].transform(vec(-1,-1,-1,1), rendering_camera->invviewproj);
	frustum[1].transform(vec(+1,-1,-1,1), rendering_camera->invviewproj);
	frustum[2].transform(vec(-1,+1,-1,1), rendering_camera->invviewproj);
	frustum[3].transform(vec(+1,+1,-1,1), rendering_camera->invviewproj);
	frustum[4].transform(vec(-1,-1,+1,1), rendering_camera->invviewproj);
	frustum[5].transform(vec(+1,-1,+1,1), rendering_camera->invviewproj);
	frustum[6].transform(vec(-1,+1,+1,1), rendering_camera->invviewproj);
	frustum[7].transform(vec(+1,+1,+1,1), rendering_camera->invviewproj);
    
	for (int i=0;i<8;i++)
	{
		frustum[i].x /= frustum[i].w;
		frustum[i].y /= frustum[i].w;
		frustum[i].z /= frustum[i].w;
		frustum[i].w = 0.f;
	}
    
	// check intersections with upper_bound and lower_bound
	for(int i=0; i<12; i++){
		int src=cube[i*2], dst=cube[i*2+1];
		if ((upper_bound.x*frustum[src].x + upper_bound.y*frustum[src].y + upper_bound.z*frustum[src].z + upper_bound.w*1)/(upper_bound.x*frustum[dst].x + upper_bound.y*frustum[dst].y + upper_bound.z*frustum[dst].z + upper_bound.w*1)<0)
		{
			PlaneIntersectLine( &proj_points[n_points++], &upper_bound, &frustum[src], &frustum[dst]);
		}
		if ((lower_bound.x*frustum[src].x + lower_bound.y*frustum[src].y + lower_bound.z*frustum[src].z + lower_bound.w*1)/(lower_bound.x*frustum[dst].x + lower_bound.y*frustum[dst].y + lower_bound.z*frustum[dst].z + lower_bound.w*1)<0)
		{
			PlaneIntersectLine( &proj_points[n_points++], &lower_bound, &frustum[src], &frustum[dst]);
		}
	}
	// check if any of the frustums vertices lie between the upper_bound and lower_bound planes
	{
		for(int i=0; i<8; i++){
			if ((upper_bound.x*frustum[i].x + upper_bound.y*frustum[i].y + upper_bound.z*frustum[i].z + upper_bound.w*1)/(lower_bound.x*frustum[i].x + lower_bound.y*frustum[i].y + lower_bound.z*frustum[i].z + lower_bound.w*1)<0){
				proj_points[n_points++] = frustum[i];
			}
		}
	}
    for (int i=0;i<24;i++)
		proj_points[i].w = 0.f;
	//
	// create the camera the grid will be projected from
	//
	//delete rendering_camera;
	rendering_camera = rendering_camera;//new OceanCamera(rendering_camera);
	// make sure the camera isn't too close to the plane
	float height_in_plane = (lower_bound.x*rendering_camera->position.x +
                             lower_bound.y*rendering_camera->position.y +
                             lower_bound.z*rendering_camera->position.z);
    
	bool keep_it_simple = true;
	bool underwater=false;
    
	if (height_in_plane < 0.0f) underwater = true;
    
	//vec_t svgForward = 
	if(keep_it_simple)
	{
		rendering_camera->forward = rendering_camera->forward;
		if (rendering_camera->forward.dot(vec(0.f, 1.f, 0.f)) < 0.2f)
		{
			rendering_camera->forward.y = 0.2f;//-rendering_camera->forward.y;
			rendering_camera->forward.normalize();
		}
		
        
		rendering_camera->update_lookat();
	}
	else
	{
		// this code doesn't work
		vec_t aimpoint, aimpoint2;
		
		if (height_in_plane < (oceanParameters.mfStrength+oceanParameters.mfElevation))
		{
			if(underwater)
				rendering_camera->position += vec(lower_bound.x,lower_bound.y,lower_bound.z)*(oceanParameters.mfStrength + oceanParameters.mfElevation - 2*height_in_plane);
			else
				rendering_camera->position += vec(lower_bound.x,lower_bound.y,lower_bound.z)*(oceanParameters.mfStrength + oceanParameters.mfElevation - height_in_plane);
		}
		
		// aim the projector at the point where the camera view-vector intersects the plane
		// if the camera is aimed away from the plane, mirror it's view-vector against the plane
		if( (PlaneDotNormal(&plane, &(rendering_camera->forward)) < 0.0f) ^ (PlaneDotCoord(&plane, &(rendering_camera->position)) < 0.0f ) )
		{
            vec_t tempp = rendering_camera->position + rendering_camera->forward;
			PlaneIntersectLine( &aimpoint, &plane, &(rendering_camera->position), &tempp );
			//D3DXPlaneIntersectLine( (D3DXVECTOR3 *)&aimpoint, (const D3DXPLANE *)&plane, (const D3DXVECTOR3 *)&(rendering_camera->position), (const D3DXVECTOR3 *)&(rendering_camera->position + rendering_camera->forward) );			
		}
		else
		{
			vec_t flipped;
			flipped = rendering_camera->forward - normal* 2* rendering_camera->forward.dot(normal);//D3DXVec3Dot(&(rendering_camera->forward),&normal);
            vec_t tempp = (rendering_camera->position + flipped);
			PlaneIntersectLine( &aimpoint, &plane, &(rendering_camera->position), &tempp );
		}
        
		// force the point the camera is looking at in a plane, and have the projector look at it
		// works well against horizon, even when camera is looking upwards
		// doesn't work straight down/up
		float af = fabs(PlaneDotNormal(&plane, &(rendering_camera->forward)));
		//af = 1 - (1-af)*(1-af)*(1-af)*(1-af)*(1-af);
		//aimpoint2 = (rendering_camera->position + rendering_camera->zfar * rendering_camera->forward);
		aimpoint2 = (rendering_camera->position + rendering_camera->forward * 10.0f );
		aimpoint2 = aimpoint2 - normal*aimpoint2.dot(normal);//D3DXVec3Dot(&aimpoint2,&normal);
        
		// fade between aimpoint & aimpoint2 depending on view angle
        
		aimpoint = aimpoint*af + aimpoint2*(1.0f-af);
		//aimpoint = aimpoint2;
        
		rendering_camera->forward = aimpoint-rendering_camera->position;
		rendering_camera->update_lookat();
	}
    
    
    
	//sprintf( debugdata, "n_points %i\n",n_points);
	{
		for(int i=0; i<n_points; i++){
			// project the point onto the Ocean plane
			proj_points[i] = proj_points[i] - normal*proj_points[i].dot(normal);//D3DXVec3Dot(&proj_points[i],&normal);
		}
	}
    
	{
		for(int i=0; i<n_points; i++)
		{
			proj_points[i].w = 1.f;
			//D3DXVec3TransformCoord( &proj_points[i], &proj_points[i], &(rendering_camera->view));
			proj_points[i].transform(rendering_camera->view);
			proj_points[i].x /= proj_points[i].w;
			proj_points[i].y /= proj_points[i].w;
			proj_points[i].z /= proj_points[i].w;
			proj_points[i].w = 1.f;
			//D3DXVec3TransformCoord( &proj_points[i], &proj_points[i], &(rendering_camera->proj));
			proj_points[i].transform(rendering_camera->proj);
			proj_points[i].x /= proj_points[i].w;
			proj_points[i].y /= proj_points[i].w;
			proj_points[i].z /= proj_points[i].w;
            
		}
	}
    
	// debughonk
    
	/*	for(int i=0; i<n_points; i++){
     sprintf( debugdata, "%s%f  %f  %f\n",debugdata,proj_points[i].x,proj_points[i].y,proj_points[i].z);
     }*/
    
	// get max/min x & y-values to determine how big the "projection window" must be
	if (n_points > 0)
	{
		float x_min,y_min,x_max,y_max;
		
		x_min = proj_points[0].x;
		x_max = proj_points[0].x;
		y_min = proj_points[0].y;
		y_max = proj_points[0].y;
		
		for(int i=1; i<n_points; i++)
		{
			if (proj_points[i].x > x_max) x_max = proj_points[i].x;
			if (proj_points[i].x < x_min) x_min = proj_points[i].x;
			if (proj_points[i].y > y_max) y_max = proj_points[i].y;
			if (proj_points[i].y < y_min) y_min = proj_points[i].y;
		}
        
		/*
         sprintf( debugdata, "%sx = [%f..%f] y = [%f..%f]\n",debugdata,x_min,x_max,y_min,y_max);
         
         sprintf( debugdata, "%sheight_in_plane: %f\n",debugdata,height_in_plane);
         */
		//sprintf( debugdata,	"%slimit_y_upper = %f\n",debugdata,limit_y_upper);
		//		sprintf( debugdata, "%sy1 = [%f] y2 = [%f]\n",debugdata,y1,y2);
        
		// build the packing matrix that spreads the grid across the "projection window"
		matrix_t pack;
		pack.set(vec(	x_max-x_min,	0,				0,		x_min),
                 vec(0,				y_max-y_min,	0,		y_min),
                 vec(0,				0,				1,		0),
                 vec(0,				0,				0,		1));
		//D3DXMatrixTranspose(&pack,&pack);
		pack.transpose();
		*range = pack*rendering_camera->invviewproj;
        
		return true;
	}
    
	return false;
#if 0
    
	oceanParameters_t *prm = &oceanParameters;
	
    
	set_displacement_amplitude(oceanParameters.mfStrength);
    
	float		x_min,y_min,x_max,y_max;
	D3DXVECTOR3 frustum[8],proj_points[24];		// frustum to check the camera against
    
	int n_points=0;
	int cube[] = {	0,1,	0,2,	2,3,	1,3,
		0,4,	2,6,	3,7,	1,5,
		4,6,	4,5,	5,7,	6,7};	// which frustum points are connected together?
    
	// transform frustum points to worldspace (should be done to the rendering_camera because it's the interesting one)
	D3DXVec3TransformCoord(&frustum[0], &D3DXVECTOR3(-1,-1,-1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));
	D3DXVec3TransformCoord(&frustum[1], &D3DXVECTOR3(+1,-1,-1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));
	D3DXVec3TransformCoord(&frustum[2], &D3DXVECTOR3(-1,+1,-1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));
	D3DXVec3TransformCoord(&frustum[3], &D3DXVECTOR3(+1,+1,-1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));
	D3DXVec3TransformCoord(&frustum[4], &D3DXVECTOR3(-1,-1,+1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));
	D3DXVec3TransformCoord(&frustum[5], &D3DXVECTOR3(+1,-1,+1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));
	D3DXVec3TransformCoord(&frustum[6], &D3DXVECTOR3(-1,+1,+1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));
	D3DXVec3TransformCoord(&frustum[7], &D3DXVECTOR3(+1,+1,+1), (const D3DXMATRIX *)&(rendering_camera->invviewproj));	
    
    
	D3DXPLANE _upper_bound(upper_bound.x, upper_bound.y, upper_bound.z, upper_bound.w);
	D3DXPLANE _lower_bound(lower_bound.x, lower_bound.y, lower_bound.z, lower_bound.w);
	// check intersections with upper_bound and lower_bound	
	for(int i=0; i<12; i++){
		int src=cube[i*2], dst=cube[i*2+1];
		if ((_upper_bound.a*frustum[src].x + _upper_bound.b*frustum[src].y + _upper_bound.c*frustum[src].z + _upper_bound.d*1)/(_upper_bound.a*frustum[dst].x + _upper_bound.b*frustum[dst].y + _upper_bound.c*frustum[dst].z + _upper_bound.d*1)<0){			
			D3DXPlaneIntersectLine( &proj_points[n_points++], (const D3DXPLANE*)&upper_bound, &frustum[src], &frustum[dst]);			
		}
		if ((_lower_bound.a*frustum[src].x + _lower_bound.b*frustum[src].y + _lower_bound.c*frustum[src].z + _lower_bound.d*1)/(_lower_bound.a*frustum[dst].x + _lower_bound.b*frustum[dst].y + _lower_bound.c*frustum[dst].z + _lower_bound.d*1)<0){			
			D3DXPlaneIntersectLine( &proj_points[n_points++], (const D3DXPLANE*)&_lower_bound, &frustum[src], &frustum[dst]);			
		}
	}
	// check if any of the frustums vertices lie between the upper_bound and lower_bound planes
	{
		for(int i=0; i<8; i++){		
			if ((_upper_bound.a*frustum[i].x + _upper_bound.b*frustum[i].y + _upper_bound.c*frustum[i].z + _upper_bound.d*1)/(_lower_bound.a*frustum[i].x + _lower_bound.b*frustum[i].y + _lower_bound.c*frustum[i].z + _lower_bound.d*1)<0){			
				proj_points[n_points++] = frustum[i];
			}		
		}	
	}
    
	//
	// create the camera the grid will be projected from
	//
	delete projecting_camera;
	projecting_camera = new camera(rendering_camera);
	// make sure the camera isn't too close to the plane
	float height_in_plane = (_lower_bound.a*projecting_camera->position.x +
                             _lower_bound.b*projecting_camera->position.y +
                             _lower_bound.c*projecting_camera->position.z);
    
	bool keep_it_simple = false;
	bool underwater=false;
    
	if (height_in_plane < 0.0f) underwater = true;
    
	if(keep_it_simple)
	{
		projecting_camera->forward = rendering_camera->forward;
		projecting_camera->update_lookat();
	}
	else
	{
		D3DXVECTOR3 aimpoint, aimpoint2;		
        
		if (height_in_plane < (prm->params[p_fStrength].fData+prm->get_float(p_fElevation)))
		{					
			if(underwater)
				projecting_camera->position += D3DXVECTOR3(_lower_bound.a,_lower_bound.b,_lower_bound.c)*(prm->params[p_fStrength].fData + prm->get_float(p_fElevation) - 2*height_in_plane);															
			else
				projecting_camera->position += D3DXVECTOR3(_lower_bound.a,_lower_bound.b,_lower_bound.c)*(prm->params[p_fStrength].fData + prm->get_float(p_fElevation) - height_in_plane);
		} 
		
		// aim the projector at the point where the camera view-vector intersects the plane
		// if the camera is aimed away from the plane, mirror it's view-vector against the plane
		if( (D3DXPlaneDotNormal((D3DXPLANE*)&plane, &(rendering_camera->forward)) < 0.0f) log_xor (D3DXPlaneDotCoord(&plane, &(rendering_camera->position)) < 0.0f ) )
		{				
			D3DXPlaneIntersectLine( &aimpoint, (const D3DXPLANE*)&plane, &(rendering_camera->position), &(rendering_camera->position + rendering_camera->forward) );			
		}
		else
		{
			D3DXVECTOR3 flipped;
			flipped = rendering_camera->forward - 2*normal*D3DXVec3Dot((const D3DXVECTOR3*)&(rendering_camera->forward),&normal);
			D3DXPlaneIntersectLine( &aimpoint, (const D3DXPLANE*)&plane, (const D3DXVECTOR3*)&(rendering_camera->position), &(rendering_camera->position + flipped) );			
		}
        
        // force the point the camera is looking at in a plane, and have the projector look at it
        // works well against horizon, even when camera is looking upwards
        // doesn't work straight down/up
        float af = fabs(D3DXPlaneDotNormal((const D3DXPLANE*)&plane, (const D3DXPLANE*)&(rendering_camera->forward)));
        //af = 1 - (1-af)*(1-af)*(1-af)*(1-af)*(1-af);
        //aimpoint2 = (rendering_camera->position + rendering_camera->zfar * rendering_camera->forward);
        aimpoint2 = (rendering_camera->position + 10.0f * rendering_camera->forward);
        aimpoint2 = aimpoint2 - normal*D3DXVec3Dot(&aimpoint2,(D3DXVECTOR3*)&normal);
		
        // fade between aimpoint & aimpoint2 depending on view angle
        
        aimpoint = aimpoint*af + aimpoint2*(1.0f-af);
        //aimpoint = aimpoint2;
        
        projecting_camera->forward = aimpoint-projecting_camera->position;
        projecting_camera->update_lookat();
	}
    
	// get max/min x & y-values to determine how big the "projection window" must be
	if (n_points > 0){
		x_min = proj_points[0].x;
		x_max = proj_points[0].x;
		y_min = proj_points[0].y;
		y_max = proj_points[0].y;
		for(int i=1; i<n_points; i++){
			if (proj_points[i].x > x_max) x_max = proj_points[i].x;
			if (proj_points[i].x < x_min) x_min = proj_points[i].x;
			if (proj_points[i].y > y_max) y_max = proj_points[i].y;
			if (proj_points[i].y < y_min) y_min = proj_points[i].y;
		}		
		
		/*
         sprintf( debugdata, "%sx = [%f..%f] y = [%f..%f]\n",debugdata,x_min,x_max,y_min,y_max);
         
         sprintf( debugdata, "%sheight_in_plane: %f\n",debugdata,height_in_plane);
         */
		//sprintf( debugdata,	"%slimit_y_upper = %f\n",debugdata,limit_y_upper);
		//		sprintf( debugdata, "%sy1 = [%f] y2 = [%f]\n",debugdata,y1,y2);
        
		// build the packing matrix that spreads the grid across the "projection window"
		D3DXMATRIXA16 pack(	x_max-x_min,	0,				0,		x_min,
                           0,				y_max-y_min,	0,		y_min,
                           0,				0,				1,		0,	
                           0,				0,				0,		1);
		D3DXMatrixTranspose(&pack,&pack);
		*range = pack*projecting_camera->invviewproj;
        
		return true;
	}
	return false;
#endif
}

Ocean::~Ocean()
{
	delete mVA;
	delete mIA;
}

void Ocean::set_displacement_amplitude(float amplitude)
{
	upper_bound = buildPlan((this->pos +  this->normal * amplitude ), (this->normal));
	upper_bound.w = -upper_bound.w;
	lower_bound = buildPlan((this->pos -  this->normal * amplitude ), (this->normal));
	lower_bound.w = -lower_bound.w;
}
