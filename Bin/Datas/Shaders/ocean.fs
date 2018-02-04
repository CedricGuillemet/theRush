varying vec3 R;
varying vec4 ProjShadow[4];
varying vec3 sunlight;
uniform sampler2DShadow ShadowMap;
uniform vec4 shadowNearPlanes;
uniform vec4 shadowFarPlanes;
//"varying vec2 refr_tc;
varying vec2 refl_tc;
varying vec3 Z;
varying vec3 viewNormal;
varying float holeColor;
uniform samplerCube sky;
uniform sampler2D reflmap;
uniform float sun_strength;

void main()
{
	if (holeColor < 0.1) discard;
	vec4 z = vec4(Z.z);
	vec4 near = vec4 (z.x >= shadowNearPlanes.x, z.x >= shadowNearPlanes.y,z.x >= shadowNearPlanes.z, z.x >= shadowNearPlanes.w);
	vec4 far = vec4 (z.x < shadowFarPlanes.x, z.x < shadowFarPlanes.y,z.x < shadowFarPlanes.z, z.x < shadowFarPlanes.w);
	vec4 weights = near * far;
	vec4 coord = ProjShadow[0] * weights.x + ProjShadow[1] * weights.y + ProjShadow[2] * weights.z + ProjShadow[3] * weights.w;
	//float mip = shadow2DProj(ShadowMap, coord).r;
	//mip *= 0.5;
	//mip += 0.5;
	vec3 global_refl = textureCube(sky,R).rgb  + sunlight.rgb*sun_strength;
	vec4 local_refl = texture2D(reflmap, refl_tc);
	local_refl.xyz *= 0.5;
	vec3 refl = mix( global_refl , local_refl.rgb, local_refl.a);
	
	gl_FragData[0] = vec4(refl.rgb, 0.5+dot(refl.rgb, vec3(0.299, 0.587, 0.114) ) * 0.5 );
	gl_FragData[1] = vec4(1.0, 0.5, 1.0, 0.5);//normalize(viewNormal.xyz)*0.5 + 0.5);
	gl_FragData[2] = vec4(Z.z,1,1,1);
	gl_FragData[3] = vec4(0.0);
}