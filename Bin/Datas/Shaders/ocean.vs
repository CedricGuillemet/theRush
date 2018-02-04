uniform mat4 mViewProj;
uniform mat4 mViewMat;
varying vec4 ProjShadow[4];
uniform mat4 shadowMat[4];
uniform vec4 view_position;
uniform float sun_shininess;
uniform vec3 sun_vec;
uniform vec4 sun_color;
uniform float reflrefr_offset;
uniform vec4 holeDisks[4];
varying vec3 R;
varying vec3 viewNormal;
varying vec3 sunlight;
varying vec2 refl_tc;
varying vec3 Z;
varying float holeColor;

void main()
{
	vec4 hpos = mViewProj* vec4( gl_Vertex.xyz, 1.);
	gl_Position = hpos;
	vec3 normal = normalize(gl_Normal.xyz);
	viewNormal = -normalize( (mViewMat * vec4(normal.xyz, 0.) ).xyz);
	vec3 v = normalize(gl_Vertex.xyz - view_position.xyz/view_position.w);
	R = reflect(v,normal);
	float tsunlight = pow(clamp(dot(R, sun_vec),0., 1.),sun_shininess);
	sunlight = tsunlight*sun_color.xyz;
	vec4 tpos =  mViewProj* vec4( gl_Vertex.x, 0., gl_Vertex.z, 1.);
	tpos.xyz = tpos.xyz/tpos.w;
	tpos.xy = vec2(0.5) + 0.5*tpos.xy*vec2(1.,-1.);
	tpos.z = reflrefr_offset/tpos.z;
	refl_tc = hpos.xy;
	refl_tc /= hpos.w;
	refl_tc += vec2(1., 1.) - normal.xz * 0.05;
	refl_tc *= 0.5;
	refl_tc.y = 1.-refl_tc.y;
	Z = hpos.xyz;
	
	ProjShadow[0]		= shadowMat[0] * gl_Vertex;
	ProjShadow[1]		= shadowMat[1] * gl_Vertex;
	ProjShadow[1].z -= 0.0015 * ProjShadow[1].w;
	ProjShadow[2]		= shadowMat[2] * gl_Vertex;
	ProjShadow[2].z -= 0.0015 * ProjShadow[2].w;
	ProjShadow[3]		= shadowMat[3] * gl_Vertex;
	ProjShadow[3].z -= 0.0015 * ProjShadow[3].w;
	holeColor = 1.;
	
	for ( int i = 0;i<4;i++) 
	{
		vec3 difPos = gl_Vertex.xyz-holeDisks[i].xyz;
		difPos.y = 0.0;
		holeColor *= clamp(dot(difPos,difPos) - holeDisks[i].w + 1.0, 0.0, 1.0);
	}
}