uniform vec4 color; varying vec4 normal; varying vec4 pos; varying vec4 vertColor;
varying vec4 ProjShadow[4];
uniform sampler2D ShadowMap;
uniform vec4 solidShader;
uniform vec4 shadowNearPlanes;
uniform vec4 shadowFarPlanes;

float chebyshevUpperBound( vec4 ShadowCoordPostW, float distance )
{
	vec2 moments = texture2D(ShadowMap,ShadowCoordPostW.xy).rg;
	
	// Surface is fully lit. as the current fragment is before the light occluder
	if (distance <= moments.x)
		return 1.0 ;

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x*moments.x);
	variance = max(variance,0.002);

	float d = distance - moments.x;
	float p_max = variance / (variance + d*d);

	return p_max;
}


void main() 
{
	gl_FragData[0] = color*vertColor;
	vec4 z = vec4(pos.z);
	vec4 near = vec4 (z.x >= shadowNearPlanes.x, z.x >= shadowNearPlanes.y,z.x >= shadowNearPlanes.z, z.x >= shadowNearPlanes.w);
	vec4 far = vec4 (z.x < shadowFarPlanes.x, z.x < shadowFarPlanes.y,z.x < shadowFarPlanes.z, z.x < shadowFarPlanes.w);
	vec4 weights = near * far;
	vec4 coord = ProjShadow[0] * weights.x +
	ProjShadow[1] * weights.y +
	ProjShadow[2] * weights.z +
	ProjShadow[3] * weights.w;
	
	//float sm = shadow2DProj(ShadowMap, coord).r;
		vec4 ShadowCoordPostW = coord / coord.w;
	float sm = chebyshevUpperBound( ShadowCoordPostW, ShadowCoordPostW.z );

	gl_FragData[1] = vec4(sm, normalize(normal.xyz)*0.5 + 0.5);
	gl_FragData[2] = vec4(z.x,1,1,1);
	gl_FragData[3] = solidShader;
}