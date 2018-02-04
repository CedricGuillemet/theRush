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

#ifndef OCEAN_H__
#define OCEAN_H__

class ZIndexArrayOGL;
class ZVertexArrayOGL;

///////////////////////////////////////////////////////////////////////////////////////////////////

#define XAxis vec(1.f, 0.f, 0.f)
#define YAxis vec(0.f, 1.f, 0.f)
#define ZAxis vec(0.f, 0.f, 1.f)

///////////////////////////////////////////////////////////////////////////////////////////////////

#define n_bits				5
#define n_size				(1<<(n_bits-1))
#define n_size_m1			(n_size - 1)
#define n_size_sq			(n_size*n_size)
#define n_size_sq_m1		(n_size_sq - 1)


#define n_packsize			4

#define np_bits				(n_bits+n_packsize-1)
#define np_size				(1<<(np_bits-1))
#define np_size_m1			(np_size-1)
#define np_size_sq			(np_size*np_size)
#define np_size_sq_m1		(np_size_sq-1)

#define n_dec_bits			12
#define n_dec_magn			4096
#define n_dec_magn_m1		4095

#define max_octaves			32

#define noise_frames		256
#define noise_frames_m1		(noise_frames-1)

#define noise_decimalbits	15
#define noise_magnitude		(1<<(noise_decimalbits-1))

#define scale_decimalbits	15
#define scale_magnitude		(1<<(scale_decimalbits-1))

struct SOFTWARESURFACEVERTEX
{
	float x,y,z;
	float nx,ny,nz;
	float tu,tv;
};

typedef struct oceanParameters_t
{
	oceanParameters_t();

	float mfStrength;
	float mfFalloff;
	float mfScale;
	bool mbSmooth;
	float mbReflRefrStrength;
	uint32 miOctaves;
	float mfLODbias;
	float mfAnimspeed;
	float mfTimemulti;
	bool mbPaused;
	float mfElevation;
	bool mbDisplayTargets;
	float mfSunPosTheta;
	float mfSunPosAlpha;
	float mfSunShininess;
	float mfSunStrength;
	float mfWaterColourR;
	float mfWaterColourG;
	float mfWaterColourB;
	bool mbDisplace;
	bool mbDiffuseRefl;
} oceanParameters_t;

extern oceanParameters_t oceanParameters;
class OceanNoise
{
public:
	OceanNoise() {}
	void init(ZVertexArrayOGL *pVA, int sizeX, int sizeY);
	~OceanNoise() {}
	//void resize(int sizeX, int sizeY);
	bool render_geometry( float aTimeEllapsed, const matrix_t *m );
	vec_t calc_worldpos( const vec_t& uv, const matrix_t *m );
	float get_height_at(int, int, int);
	float get_height_dual(int, int, float strength);
	float get_height_at(float, float);
	//SOFTWARESURFACEVERTEX *vertices;
	ZVertexArrayOGL *mVA;
	void calc_normals( SOFTWARESURFACEVERTEX *vertices );
private:
	void init_noise();
	void calc_noise( float aTimeEllapsed );
	float readtexel_nearest(int u, int v);
	int readtexel_linear(int u, int v, int);
	int readtexel_linear_dual(int u, int v, int);
	int mapsample(int u, int v, int level, int octave);

	int sizeX, sizeY;	// framebuffer size
	float f_sizeX, f_sizeY;
	float *framebuffer;
	int noise[n_size_sq*noise_frames];
	int o_noise[n_size_sq*max_octaves];
	int p_noise[np_size_sq*(max_octaves>>(n_packsize-1))];
	int *r_noise;
	int octaves;
	vec_t e_u, e_v;

	int multitable[max_octaves];
	float last_time;
	float f_multitable[max_octaves];
	double time;

	// remember these
	vec_t t_corners0,t_corners1,t_corners2,t_corners3;
};

inline float OceanNoise::readtexel_nearest(int u, int v)
{
	int lu, lv;
	lu = (u>>n_dec_bits)&n_size_m1;
	lv = (v>>n_dec_bits)&n_size_m1;
	return (float)noise[lv*n_size + lu];
}


inline int OceanNoise::readtexel_linear_dual(int u, int v, int o)
{
    UNUSED_PARAMETER(o);

	int iu, iup, iv, ivp, fu, fv;
	iu = (u>>n_dec_bits)&np_size_m1;
	iv = ((v>>n_dec_bits)&np_size_m1)*np_size;

	iup = ((u>>n_dec_bits) + 1)&np_size_m1;
	ivp = (((v>>n_dec_bits) + 1)&np_size_m1)*np_size;

	fu = u & n_dec_magn_m1;
	fv = v & n_dec_magn_m1;

	int ut01 = ((n_dec_magn-fu)*r_noise[iv + iu] + fu*r_noise[iv + iup])>>n_dec_bits;
	int ut23 = ((n_dec_magn-fu)*r_noise[ivp + iu] + fu*r_noise[ivp + iup])>>n_dec_bits;
	int ut = ((n_dec_magn-fv)*ut01 + fv*ut23) >> n_dec_bits;
	return ut;
}

inline float OceanNoise::get_height_dual(int u, int v, float strength)
{
	int value=0;
	r_noise = p_noise;	// pointer to the current noise source octave
	int hoct = octaves / n_packsize;
	for(int i=0; i<hoct; i++)
	{
		value += readtexel_linear_dual(u,v,0);
		u = u << n_packsize;
		v = v << n_packsize;
		r_noise += np_size_sq;
	}
	return value*strength/*prm->mfStrength*//noise_magnitude;
}

class OceanCamera
{
public:
	OceanCamera();
	OceanCamera(const OceanCamera *src);
	OceanCamera(const vec_t& pos, float rotx, float roty, float rotz, float fov, float aspect, float nearz, float farz);
	virtual ~OceanCamera();
	void update();
	void update(const matrix_t &aview, const matrix_t &aproj,
	const matrix_t &ainvView, const matrix_t &ainvProj);

	void update_lookat();
	
protected:

	float fov, aspect,znear, zfar, rotx, roty, rotz;
	
	
public:
	matrix_t view, invview, proj, invproj, viewproj, invviewproj;
	vec_t forward,up,right;
	vec_t position;
};
extern OceanCamera oceanCamera;

class Ocean
{
public:
	Ocean(int gridsize_x, int gridsize_y, OceanCamera *tesscamera);
	virtual ~Ocean();
	bool PrepareVisibily(const OceanCamera*);

	//void calc_efficiency();
	
	void set_displacement_amplitude(float amplitude);
	float get_height_at(float x, float z) { return software_brus.get_height_at(x,z); }
	//void UploadGeometry();
	void ComputeMesh( float aTimeEllapsed );
	void Draw();
	matrix_t range;
	vec_t	plane, upper_bound, lower_bound;

	OceanCamera	*rendering_camera;

public:

	bool initbuffers();
	bool getMinMax(matrix_t *range);
	bool within_frustum(const vec_t *pos);

public:
	vec_t	normal, u, v, pos;
	float		min_height,max_height;
	int			gridsize_x, gridsize_y;
	bool		plane_within_frustum;

	OceanNoise software_brus;
	
	ZIndexArrayOGL *mIA;
	ZVertexArrayOGL *mVA;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif