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

#ifndef MATH_H___
#define MATH_H___

///////////////////////////////////////////////////////////////////////////////////////////////////

struct matrix_t;

inline void FPU_MatrixF_x_MatrixF(const float *a, const float *b, float *r)
{
	r[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
	r[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
	r[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
	r[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

	r[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
	r[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
	r[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
	r[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

	r[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
	r[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
	r[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
	r[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

	r[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
	r[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
	r[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
	r[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];
}

const float PI    =  3.14159265358979323846f;

const float PI_MUL_2 =  6.28318530717958647692f;
const float PI_DIV_2 =  1.57079632679489655800f;
const float PI_DIV_4 =  0.78539816339744827900f;
const float INV_PI   =  0.31830988618379069122f;
const float DEGTORAD =  0.01745329251994329547f;
const float RADTODEG = 57.29577951308232286465f;
const float SQRT2    =  1.41421356237309504880f;
const float SQRT3    =  1.73205080756887729352f;
#define    FLOAT_EPSILON    float(1.192092896e-07)
#define    NEARLY_EQUAL_FLOAT_EPSILON   1.e-06F

#define LERP(x,y,z) (x+(y-x)*z)
#define zmax(x,y) ((x>y)?x:y)
#define zmin(x,y) ((x<y)?x:y)
#define Clamp(x,y,z) ( (x<y)?y:((x>z)?z:x) )
#define DegreeToRadian(fDegrees) ((3.14159265f/180.0f)*fDegrees)
#define FREQ60Hz (1.f/60.f)
#define FREQ100Hz (1.f/100.f)
#define FREQ120Hz (1.f/120.f)

inline bool IsNearlyEqual(float _fA, float _fB, float _fEpsilon = NEARLY_EQUAL_FLOAT_EPSILON )
{
    const float fDiff = _fA - _fB;
    return ((-_fEpsilon < fDiff) && (fDiff < _fEpsilon));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct vec_t
{
public:

	float x,y,z,w;

    //vec_t() {}
    //vec_t(const vec_t& _v) : x(_v.x), y(_v.y), z(_v.z), w(_v.w) {}
    //vec_t(float _fV): x(_fV), y(_fV), z(_fV), w(_fV) {}

	void lerp(const vec_t& v, float t)
	{
		x += (v.x-x) * t;
		y += (v.y-y) * t;
		z += (v.z-z) * t;
		w += (v.w-w) * t;
	}

    void lerp(const vec_t& v, const vec_t& v2,float t)
	{
        *this = v;
        lerp(v2, t);
	}

	inline void set(float v) { x = y = z = w = v; }
	inline void set(float _x, float _y, float _z, float _w)	{ x = _x; y = _y; z = _z; w = _w; }

	inline vec_t& operator -= ( const vec_t& v ) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	inline vec_t& operator += ( const vec_t& v ) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	inline vec_t& operator *= ( const vec_t& v ) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	inline vec_t& operator *= ( float v ) { x *= v;	y *= v;	z *= v;	w *= v;	return *this; }

	inline vec_t operator * ( float f ) const;
	inline vec_t operator - () const;
	inline vec_t operator - ( const vec_t& v ) const;
	inline vec_t operator + ( const vec_t& v ) const;
	inline vec_t operator * ( const vec_t& v ) const;

	inline const vec_t& operator + () const { return (*this); }
	inline float length() const { return sqrtf(x*x +y*y +z*z +w*w); };
	inline float lengthSq() const { return (x*x +y*y +z*z +w*w); };
	inline vec_t normalize() { (*this) *= (1.f/length()); return (*this); }
	inline vec_t normalize(const vec_t& v) { this->set(v.x, v.y, v.z, v.w); this->normalize(); return (*this); }

    bool isNormalized(float _fEpsilon = NEARLY_EQUAL_FLOAT_EPSILON) const
    {
        const float lenSq = lengthSq();
        return IsNearlyEqual( lenSq, 1.f, _fEpsilon );
    }

	inline void cross(const vec_t& v)
	{
		vec_t res;
		res.x = y * v.z - z * v.y;
		res.y = z * v.x - x * v.z;
		res.z = x * v.y - y * v.x;

		x = res.x;
		y = res.y;
		z = res.z;
		w = 0.f;
	}
	inline void cross(const vec_t& v1, const vec_t& v2)
	{
		x = v1.y * v2.z - v1.z * v2.y;
		y = v1.z * v2.x - v1.x * v2.z;
		z = v1.x * v2.y - v1.y * v2.x;
		w = 0.f;
	}
	inline float dot( const vec_t &v) const
	{
		return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);
	}

	void isMaxOf(const vec_t& v)
	{
		x = (v.x>x)?v.x:x;
		y = (v.y>y)?v.y:y;
		z = (v.z>z)?v.z:z;
		w = (v.w>w)?v.z:w;
	}
	void isMinOf(const vec_t& v)
	{
		x = (v.x>x)?x:v.x;
		y = (v.y>y)?y:v.y;
		z = (v.z>z)?z:v.z;
		w = (v.w>w)?z:v.w;
	}

    vec_t symetrical(const vec_t& v) const
    {
        vec_t res;
        float dist = signedDistanceTo(v);
        res = v;
        res -= (*this)*dist*2.f;

        return res;
    }
	void transform(const matrix_t& matrix );
	void transform(const vec_t & s, const matrix_t& matrix );

	void TransformVector(const matrix_t& matrix );
	void TransformPoint(const matrix_t& matrix );
	void TransformVector(const vec_t& v, const matrix_t& matrix ) { (*this) = v; this->TransformVector(matrix); }
	void TransformPoint(const vec_t& v, const matrix_t& matrix ) { (*this) = v; this->TransformPoint(matrix); }

    // quaternion slerp
    //void slerp(const vec_t &q1, const vec_t &q2, float t );

	inline float signedDistanceTo(const vec_t& point) const;
	vec_t interpolateHermite(const vec_t &nextKey, const vec_t &nextKeyP1, const vec_t &prevKey, float ratio) const;
    static float d(const vec_t& v1, const vec_t& v2) { return (v1-v2).length(); }
    static float d2(const vec_t& v1, const vec_t& v2) { return (v1-v2).lengthSq(); }

	static vec_t zero;

    uint16 toUInt5551() const { return static_cast<uint16>( ((int)(w*1.f)<< 15) + ((int)(z*31.f)<< 10) + ((int)(y*31.f)<< 5) + ((int)(x*31.f)) ); }
    void fromUInt5551(unsigned short v) { w = (float)( (v&0x8000) >> 15) ; z = (float)( (v&0x7C00) >> 10) * (1.f/31.f);
	y = (float)( (v&0x3E0) >> 5) * (1.f/31.f); x = (float)( (v&0x1F)) * (1.f/31.f); }

	uint32 toUInt32() const { return ((int)(w*255.f)<< 24) + ((int)(z*255.f)<< 16) + ((int)(y*255.f)<< 8) + ((int)(x*255.f)); }
	void fromUInt32(uint32 v) { w = (float)( (v&0xFF000000) >> 24) * (1.f/255.f); z = (float)( (v&0xFF0000) >> 16) * (1.f/255.f);
	y = (float)( (v&0xFF00) >> 8) * (1.f/255.f); x = (float)( (v&0xFF)) * (1.f/255.f); }

    vec_t swapedRB() const;
} vec_t;

inline vec_t vec(int _x, int _y, int _z) { vec_t res; res.set((float)_x, (float)_y, (float)_z, 0.f); return res;}
inline vec_t vec(float _x, float _y) { vec_t res; res.set(_x, _y, 0.f, 0.f); return res;}
inline vec_t vec(float _x, float _y, float _z) { vec_t res; res.set(_x, _y, _z, 0.f); return res;}
inline vec_t vec(float _x, float _y, float _z, float _w) { vec_t res; res.set(_x, _y, _z, _w); return res;}
inline vec_t vec(float _v) { return vec(_v, _v, _v, _v); }
inline vec_t vec( u32 col ) { vec_t res; res.fromUInt32(col); return res; }


inline vec_t vec_t::operator * ( float f ) const { return vec(x * f, y * f, z * f, w *f); }
inline vec_t vec_t::operator - () const { return vec(-x, -y, -z, -w); }
inline vec_t vec_t::operator - ( const vec_t& v ) const { return vec(x - v.x, y - v.y, z - v.z, w - v.w); }
inline vec_t vec_t::operator + ( const vec_t& v ) const { return vec(x + v.x, y + v.y, z + v.z, w + v.w); }
inline vec_t vec_t::operator * ( const vec_t& v ) const { return vec(x * v.x, y * v.y, z * v.z, w * v.w); }
inline float vec_t::signedDistanceTo(const vec_t& point) const	{ return (point.dot(vec(x,y,z))) - w; }

inline vec_t normalized(const vec_t& v) { vec_t res; res = v; res.normalize(); return res; }
inline vec_t cross(const vec_t& v1, const vec_t& v2)
{
    vec_t res;
    res.x = v1.y * v2.z - v1.z * v2.y;
    res.y = v1.z * v2.x - v1.x * v2.z;
    res.z = v1.x * v2.y - v1.y * v2.x;
    res.w = 0.f;
    return res;
}

inline float Dot( const vec_t &v1, const vec_t &v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

inline float distance(const vec_t& v1, const vec_t& v2) { return vec_t::d(v1, v2); }

inline vec_t MakeNormal(const vec_t & point1, const vec_t & point2, const vec_t & point3)
{
	vec_t nrm;
	vec_t tmp1 = point1 - point3;
	vec_t tmp2 = point2 - point3;
	nrm.cross(tmp1, tmp2);
	return nrm;
}

inline float vecByIndex(const vec_t& v, int idx)
{
	switch( idx)
	{
	case 0: return v.x;
	case 1: return v.y;
	case 2: return v.z;
	default: return v.w;
	}
}

inline vec_t vecMul(const vec_t& v1, const vec_t& v2)
{
	return vec(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z , v1.w * v2.w);
}

inline vec_t vecMin(const vec_t& v1, const vec_t& v2)
{
	vec_t res = v1;
	res.isMinOf(v2);

	return res;
}
inline vec_t vecMax(const vec_t& v1, const vec_t& v2)
{
	vec_t res = v1;
	res.isMaxOf(v2);
	return res;
}

inline vec_t vecFloor(const vec_t& v)
{
	return vec(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w) );
}

inline vec_t splatZ(const vec_t& v) { return vec(v.z); }
inline vec_t splatW(const vec_t& v) { return vec(v.w); }

inline vec_t vecReciprocal(const vec_t& v) { return vec(1.f/v.x, 1.f/v.y, 1.f/v.z, 1.f/v.w); }

inline vec_t buildPlan(const vec_t & p_point1, const vec_t & p_normal)
{
	vec_t normal, res;
	normal.normalize(p_normal);
	res.w = normal.dot(p_point1);
	res.x = normal.x;
	res.y = normal.y;
	res.z = normal.z;

	return res;
}
inline vec_t vec_t::swapedRB() const { return vec(z,y,x,w); }

inline float smootherstep(float edge0, float edge1, float x)
{
    // Scale, and clamp x to 0..1 range
    x = Clamp((x - edge0)/(edge1 - edge0), 0, 1);
    // Evaluate polynomial
    return x*x*x*(x*(x*6 - 15) + 10);
}

inline vec_t *slerp(vec_t *pout, const vec_t* pq1, const vec_t* pq2, float t)
{
    float dot, epsilon;

    epsilon = 1.0f;
    dot = pq1->dot( *pq2 );
    if ( dot < 0.0f ) epsilon = -1.0f;
    pout->x = (1.0f - t) * pq1->x + epsilon * t * pq2->x;
    pout->y = (1.0f - t) * pq1->y + epsilon * t * pq2->y;
    pout->z = (1.0f - t) * pq1->z + epsilon * t * pq2->z;
    pout->w = (1.0f - t) * pq1->w + epsilon * t * pq2->w;
    return pout;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct vec3
{
    float x,y,z;

    vec3() {}
    vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    void set( float v)
    {
        x = y = z = v;
    }
    float length() const
    {
        return sqrtf( x * x + y * y + z * z );
    }
    void lerp( float v, float t)
    {
        x = LERP( x, v, t);
        y = LERP( y, v, t);
        z = LERP( z, v, t);
    }
    vec3& operator = (const vec_t& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    vec3& operator * (const float v)
    {
        vec3 ret;
        ret.x = x * v;
        ret.y = y * v;
        ret.z = z * v;
        return *this;
    }

    vec3& operator += (const vec_t& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    vec3& operator += (const vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    vec3& operator -= (const vec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    vec_t getVec() const
    {
        return vec( x, y, z, 0.f );
    }
} vec3;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct matrix_t
{
public:
	union
	{
		float m[4][4];
		float m16[16];
		struct
		{
			vec_t right, up, dir, position;
		} ;
	};

    matrix_t() {}

    matrix_t(const matrix_t& _m) : right(_m.right), up(_m.up), dir(_m.dir), position(_m.position) {}

    matrix_t(float _fV) { right.set(_fV); up.set(_fV); dir.set(_fV); position.set(_fV); }

	matrix_t(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15, float v16)
    {
		m16[0] = v1;
		m16[1] = v2;
		m16[2] = v3;
		m16[3] = v4;
		m16[4] = v5;
		m16[5] = v6;
		m16[6] = v7;
		m16[7] = v8;
		m16[8] = v9;
		m16[9] = v10;
		m16[10] = v11;
		m16[11] = v12;
		m16[12] = v13;
		m16[13] = v14;
		m16[14] = v15;
		m16[15] = v16;
    }
	matrix_t(const vec_t & r, const vec_t &u, const vec_t& d, const vec_t& p) { set(r, u, d, p); }

	void set(const vec_t & r, const vec_t &u, const vec_t& d, const vec_t& p) { right=r; up=u; dir=d; position=p; }
	void set(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15, float v16)
	{
		m16[0] = v1;
		m16[1] = v2;
		m16[2] = v3;
		m16[3] = v4;
		m16[4] = v5;
		m16[5] = v6;
		m16[6] = v7;
		m16[7] = v8;
		m16[8] = v9;
		m16[9] = v10;
		m16[10] = v11;
		m16[11] = v12;
		m16[12] = v13;
		m16[13] = v14;
		m16[14] = v15;
		m16[15] = v16;
	}

	operator float * () { return m16; }
	operator const float* () const { return m16; }
	void translation(float _x, float _y, float _z) { this->translation( vec(_x, _y, _z) ); }

	void translation(const vec_t& vt)
	{
		right.set(1.f, 0.f, 0.f, 0.f);
		up.set(0.f, 1.f, 0.f, 0.f);
		dir.set(0.f, 0.f, 1.f, 0.f);
		position.set(vt.x, vt.y, vt.z, 1.f);
	}

	inline void rotationY(const float angle )
	{
		float c = cosf(angle);
		float s = sinf(angle);

		right = vec(c, 0.f, -s, 0.f);
		up = vec(0.f, 1.f, 0.f , 0.f);
		dir = vec(s, 0.f, c , 0.f);
		position = vec(0.f, 0.f, 0.f , 1.f);
	}

	inline void rotationX(const float angle )
	{
		float c = cosf(angle);
		float s = sinf(angle);

		right.set(1.f, 0.f , 0.f, 0.f);
		up.set(0.f, c , s, 0.f);
		dir.set(0.f, -s, c, 0.f);
		position.set(0.f, 0.f , 0.f, 1.f);
	}

	inline void rotationZ(const float angle )
	{
		float c = cosf(angle);
		float s = sinf(angle);

		right.set(c , s, 0.f, 0.f);
		up.set(-s, c, 0.f, 0.f);
		dir.set(0.f , 0.f, 1.f, 0.f);
		position.set(0.f , 0.f, 0, 1.f);
	}
	inline void scale(float _s)
	{
		right.set(_s, 0.f, 0.f, 0.f);
		up.set(0.f, _s, 0.f, 0.f);
		dir.set(0.f, 0.f, _s, 0.f);
		position.set(0.f, 0.f, 0.f, 1.f);
	}
	inline void scale(float _x, float _y, float _z)
	{
		right.set(_x, 0.f, 0.f, 0.f);
		up.set(0.f, _y, 0.f, 0.f);
		dir.set(0.f, 0.f, _z, 0.f);
		position.set(0.f, 0.f, 0.f, 1.f);
	}
	inline void scale(const vec_t& s) { scale(s.x, s.y, s.z); }

	inline matrix_t& operator *= ( const matrix_t& mat )
	{
		matrix_t tmpMat;
		tmpMat = *this;
		tmpMat.Multiply(mat);
		*this = tmpMat;
		return *this;
	}
	inline matrix_t operator * ( const matrix_t& mat ) const
	{
		matrix_t matT;
		matT.Multiply(*this, mat);
		return matT;
	}

	inline void Multiply( const matrix_t &matrix)
	{
		matrix_t tmp;
		tmp = *this;

		FPU_MatrixF_x_MatrixF( (float*)&tmp, (float*)&matrix, (float*)this);
	}

	inline void Multiply( const matrix_t &m1, const matrix_t &m2 )
	{
		FPU_MatrixF_x_MatrixF( (float*)&m1, (float*)&m2, (float*)this);
	}

	void glhPerspectivef2(float fovyInDegrees, float aspectRatio, float znear, float zfar);
	void glhFrustumf2(float left, float right, float bottom, float top,	float znear, float zfar);
	void PerspectiveFovLH2(const float fovy, const float aspect, const float zn, const float zf );
	void OrthoOffCenterLH(const float l, float r, float b, const float t, float zn, const float zf );
	void lookAtRH(const vec_t &eye, const vec_t &at, const vec_t &up );
	void lookAtLH(const vec_t &eye, const vec_t &at, const vec_t &up );
	void LookAt(const vec_t &eye, const vec_t &at, const vec_t &up );
    void rotationQuaternion( const vec_t &q );

	inline float GetDeterminant() const
	{
		return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] +    m[0][2] * m[1][0] * m[2][1] -
			m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] -    m[0][0] * m[1][2] * m[2][1];
	}

	float inverse(const matrix_t &srcMatrix, bool affine = false );
	float inverse(bool affine=false);
	void identity() {
		right.set(1.f, 0.f, 0.f, 0.f);
		up.set(0.f, 1.f, 0.f, 0.f);
		dir.set(0.f, 0.f, 1.f, 0.f);
		position.set(0.f, 0.f, 0.f, 1.f);
	}
	inline void transpose()
	{
		matrix_t tmpm;
		for (int l = 0; l < 4; l++)
		{
			for (int c = 0; c < 4; c++)
			{
				tmpm.m[l][c] = m[c][l];
			}
		}
		(*this) = tmpm;
	}
	void rotationAxis(const vec_t & axis, float angle );
	void lerp(const matrix_t& r, const matrix_t& t, float s)
	{
		right = LERP(r.right, t.right, s);
		up = LERP(r.up, t.up, s);
		dir = LERP(r.dir, t.dir, s);
		position = LERP(r.position, t.position, s);
	}
	void rotationYawPitchRoll(const float yaw, const float pitch, const float roll );

	inline void orthoNormalize()
	{
		right.normalize();
		up.normalize();
		dir.normalize();
	}

    bool isOrthoNormalized(float _fEpsilon = NEARLY_EQUAL_FLOAT_EPSILON) const
    {
        return ( right.isNormalized(_fEpsilon) && up.isNormalized(_fEpsilon) && dir.isNormalized(_fEpsilon) );
    }

    inline vec_t transformVector(const vec_t& _v) const
    {
        vec_t out;

        out.x = _v.x * m[0][0] + _v.y * m[1][0] + _v.z * m[2][0] ;
        out.y = _v.x * m[0][1] + _v.y * m[1][1] + _v.z * m[2][1] ;
        out.z = _v.x * m[0][2] + _v.y * m[1][2] + _v.z * m[2][2] ;
        out.w = _v.x * m[0][3] + _v.y * m[1][3] + _v.z * m[2][3] ;

        return out;
    }

    inline vec_t transformPoint(const vec_t& _v) const
    {
        vec_t out;

        out.x = _v.x * m[0][0] + _v.y * m[1][0] + _v.z * m[2][0] + m[3][0] ;
        out.y = _v.x * m[0][1] + _v.y * m[1][1] + _v.z * m[2][1] + m[3][1] ;
        out.z = _v.x * m[0][2] + _v.y * m[1][2] + _v.z * m[2][2] + m[3][2] ;
        out.w = _v.x * m[0][3] + _v.y * m[1][3] + _v.z * m[2][3] + m[3][3] ;

        return out;
    }

    static matrix_t Identity;
    static matrix_t Zero;

} matrix_t;

#if IS_OS_MACOSX
#include <xmmintrin.h> //declares _mm_* intrinsics
#endif
//#include <intrin.h>

#if IS_OS_WINDOWS
inline int FloatToInt_SSE(float x)
{
    return _mm_cvt_ss2si( _mm_load_ss(&x) );
}
#endif

extern int g_seed;
inline int fastrand()
{
	g_seed = (214013*g_seed+2531011);
	return (g_seed>>16)&0x7FFF;
}

inline float r01()
{
	return ((float)fastrand())*(1.f/32767.f);
}



inline bool CollisionClosestPointOnSegment( const vec_t & point, const vec_t & vertPos1, const vec_t & vertPos2, vec_t& res )
{

    vec_t c = point - vertPos1;
    vec_t V;

    V.normalize(vertPos2 - vertPos1);
    float d = (vertPos2 - vertPos1).length();
    float t = V.dot(c);

    if (t < 0.f)
    {
        return false;//vertPos1;
    }

    if (t > d)
    {
        return false;//vertPos2;
    }

    res = vertPos1 + V * t;
    return true;
}

inline vec_t CollisionClosestPointOnSegment( const vec_t & point, const vec_t & vertPos1, const vec_t & vertPos2 )
{

    vec_t c = point - vertPos1;
    vec_t V;

    V.normalize(vertPos2 - vertPos1);
    float d = (vertPos2 - vertPos1).length();
    float t = V.dot(c);

    if (t < 0.f)
    {
        return vertPos1;
    }

    if (t > d)
    {
        return vertPos2;
    }

    return vertPos1 + V * t;
}



inline float DistanceCollisionClosestPointOnSegment( const vec_t & point, const vec_t & vertPos1, const vec_t & vertPos2 )
{

    vec_t c = point - vertPos1;
    vec_t V;

    V.normalize(vertPos2 - vertPos1);
    float d = (vertPos2 - vertPos1).length();
    float t = V.dot(c);

    if (t < 0.f)
    {
        return (vertPos1-point).length();
    }

    if (t > d)
    {
        return (vertPos2-point).length();
    }

    vec_t r = vertPos1 + V * t;
    return ( r - point ).length();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T> struct PID
{
        inline PID()
        {
                Ki = 1;
                Kp = 1;
                Kd = 1;
                error = previouserror = I = 0.f;
        }
        inline PID(float _Ki, float _Kp, float _Kd)
        {
                Ki = _Ki;
                Kp = _Kp;
                Kd = _Kd;
                error = previouserror = I = 0.f;
        }
        inline void SetIPD(float _Ki, float _Kp, float _Kd)
        {
                Ki = _Ki;
                Kp = _Kp;
                Kd = _Kd;
        }
        /*
        start:
        previous_error = error or 0 if undefined
        error = setpoint - actual_position
        P = Kp * error
        I = I + Ki * error * dt
        D = Kd * (error - previous_error) / dt
        output = P + I + D
        wait(dt)
        goto start
        */
        inline T Compute(const T& desiredPos, const T& currentPos, float dt)
        {
                previouserror = error;
                error = desiredPos - currentPos;
                T P = Kp * error;
                I += Ki * error * dt;
                T D = Kd * (error - previouserror) / dt;
                T output = P + I + D;
                return output;
        }

        T error, previouserror, I;
        float Ki, Kp, Kd;
};

typedef PID<float> PIDf;

#pragma pack(push)
#pragma pack(1)
struct fixed816_t
{
    char intValue;
    short floatValue;

    float toFloat()
    {
        return static_cast<float>(intValue) + static_cast<float>(floatValue)/32767.f;
    }

    fixed816_t( float v)
    {
        intValue = static_cast<char>(v);
        floatValue = static_cast<unsigned short>(fmodf(v, 1.f) * 32767.f);
    }
};

#pragma pack(pop)



struct ZFrustum
{
    void    Update(const matrix_t &view, const matrix_t& projection);

    bool    PointInFrustum( const vec_t & vt ) const
    {
        // If you remember the plane equation (A*x + B*y + C*z + D = 0), then the rest
        // of this code should be quite obvious and easy to figure out yourself.
        // In case don't know the plane equation, it might be a good idea to look
        // at our Plane Collision tutorial at www.GameTutorials.com in OpenGL Tutorials.
        // I will briefly go over it here.  (A,B,C) is the (X,Y,Z) of the normal to the plane.
        // They are the same thing... but just called ABC because you don't want to say:
        // (x*x + y*y + z*z + d = 0).  That would be wrong, so they substitute them.
        // the (x, y, z) in the equation is the point that you are testing.  The D is
        // The distance the plane is from the origin.  The equation ends with "= 0" because
        // that is true when the point (x, y, z) is ON the plane.  When the point is NOT on
        // the plane, it is either a negative number (the point is behind the plane) or a
        // positive number (the point is in front of the plane).  We want to check if the point
        // is in front of the plane, so all we have to do is go through each point and make
        // sure the plane equation goes out to a positive number on each side of the frustum.
        // The result (be it positive or negative) is the distance the point is front the plane.

        // Go through all the sides of the frustum
        for(int i = 0; i < 6; i++ )
        {
            // Calculate the plane equation and check if the point is behind a side of the frustum
            if(m_Frustum[i][A] * vt.x + m_Frustum[i][B] * vt.y + m_Frustum[i][C] * vt.z + m_Frustum[i][D] <= 0)
            {
                // The point was behind a side, so it ISN'T in the frustum
                return false;
            }
        }

        // The point was inside of the frustum (In front of ALL the sides of the frustum)
        return true;
    }

    bool    SphereInFrustum( const vec_t & vt) const
    {
        for(int i = 0; i < 6; i++ )
        {
            // If the center of the sphere is farther away from the plane than the radius
            if( m_Frustum[i][A] * vt.x + m_Frustum[i][B] * vt.y + m_Frustum[i][C] * vt.z + m_Frustum[i][D] <= -vt.w )
            {
                // The distance was greater than the radius so the sphere is outside of the frustum
                return false;
            }
        }

        // The sphere was inside of the frustum!
        return true;
    }

	int SphereInFrustumVis(const vec_t& v) const
	{

		float distance;
		int result = 2;

		for(int i=0; i < 6; i++) {
			distance = m_Frustum[i][A] * v.x + m_Frustum[i][B] * v.y + m_Frustum[i][C] * v.z + m_Frustum[i][D];//pl[i].distance(p);
			if (distance < -v.w)
				return 0;
			else if (distance < v.w)
				result =  1;
		}
		return(result);
	}

    bool    BoxInFrustum( const vec_t & vt, const vec_t & size ) const
    {
        for(int i = 0; i < 6; i++ )
        {
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;

            // If we get here, it isn't in the frustum
            return false;
        }

        return true;
    }

	// matrix is an orthonormalized matrix. only orientation is used.
	bool OBBInFrustum( const matrix_t &mt, const vec_t &pos, const vec_t& size) const;

private:

    float m_Frustum[6][4];
    void NormalizePlane(float frustum[6][4], int side);

    enum FrustumSide
    {
        RIGHT    = 0,        // The RIGHT side of the frustum
        LEFT    = 1,        // The LEFT     side of the frustum
        BOTTOM    = 2,        // The BOTTOM side of the frustum
        TOP        = 3,        // The TOP side of the frustum
        BACK    = 4,        // The BACK    side of the frustum
        FRONT    = 5            // The FRONT side of the frustum
    };

    // Like above, instead of saying a number for the ABC and D of the plane, we
    // want to be more descriptive.
    enum PlaneData
    {
        A = 0,                // The X value of the plane's normal
        B = 1,                // The Y value of the plane's normal
        C = 2,                // The Z value of the plane's normal
        D = 3                // The distance the plane is from the origin
    };


};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
