uniform vec4 color; 
varying vec4 normal; 
varying vec4 pos; 
varying vec4 vertColor;
varying vec4 ProjShadow[4];
uniform sampler2D ShadowMap;
uniform sampler2D rotRandomMap;
uniform vec4 solidShader;
uniform vec4 shadowNearPlanes;
uniform vec4 shadowFarPlanes;
varying vec3 worldSpaceNormal;
varying vec3 worldSpacePosition;

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

	float ComputeLightCoverageVariance(sampler2D Sampler, vec2 TexBase, float LightCompare)
{
    vec2 LightValue = texture2D(Sampler, TexBase).xy;
    
    float Diff = LightCompare - LightValue.x;
    if(Diff > 0.0)
    {
		float Variance = LightValue.y - LightValue.x * LightValue.x;
		Variance = max(Variance,0.0005);
		
        return Variance / (Variance + Diff * Diff);
    }
    else
    {
        return 1.0;
    }
}

void main() 
{ 
vec2 poisson[16];
	poisson[0] = vec2(0.866604,0.543992)-vec2(0.5);
	poisson[1] = vec2(0.194617,0.724509)-vec2(0.5);
	poisson[2] = vec2(0.328318,0.0294504)-vec2(0.5);
	poisson[3] = vec2(0.948668,0.0382397)-vec2(0.5);
	poisson[4] = vec2(0.463851,0.693258)-vec2(0.5);
	poisson[5] = vec2(0.939146,0.955901)-vec2(0.5);
	poisson[6] = vec2(0.698691,0.881741)-vec2(0.5);
	poisson[7] = vec2(0.635487,0.430982)-vec2(0.5);
	poisson[8] = vec2(0.633595,0.0625935)-vec2(0.5);
	poisson[9] = vec2(0.121189,0.253945)-vec2(0.5);
	poisson[10] = vec2(0.00234993,0.548387)-vec2(0.5);
	poisson[11] = vec2(0.439283,0.27485)-vec2(0.5);
	poisson[12] = vec2(0.285379,0.489456)-vec2(0.5);
	poisson[13] = vec2(0.03296,0.964721)-vec2(0.5);
	poisson[14] = vec2(0.378704,0.998108)-vec2(0.5);
	poisson[15] = vec2(0.984985,0.286935)-vec2(0.5);
	
	
	vec3 icecol = mix(
	vec3(62.0/255.0, 200.0/255.0, 217.0/255.0),
	vec3(41.0/255.0, 151.0/255.0, 168.0/255.0),

	clamp( dot( normal.xyz, vec3(0.0, 0.0, 1.0) ), 0.0, 1.0) );
	float snowfactor = clamp( dot(  normalize(worldSpaceNormal), vec3(0.0, 1.0, 0.0) ), 0.7, 1.0)-0.7;
	vec4 rotRandom = texture2D(rotRandomMap, gl_FragCoord.xy * (1./64.) );
	
	vec4 snowEffect = texture2D(rotRandomMap, worldSpacePosition.xz ) + (texture2D(rotRandomMap, worldSpacePosition.xz )-vec4(0.5) );
	
	vec3 icecolsnow = mix(icecol, vec3(0.79+rotRandom.x*0.21), snowfactor *40.0);
	gl_FragData[0] = vec4(icecolsnow, 0.55 + snowfactor* 0.03);
	vec4 z = vec4(pos.z);
	vec4 near = vec4 (z.x >= shadowNearPlanes.x, z.x >= shadowNearPlanes.y,z.x >= shadowNearPlanes.z, z.x >= shadowNearPlanes.w);
	vec4 far = vec4 (z.x < shadowFarPlanes.x, z.x < shadowFarPlanes.y,z.x < shadowFarPlanes.z, z.x < shadowFarPlanes.w);
	vec4 weights = near * far;
	vec4 coord = ProjShadow[0] * weights.x +
	ProjShadow[1] * weights.y +
	ProjShadow[2] * weights.z +
	ProjShadow[3] * weights.w;
	/*
	vec4 ShadowCoordPostW = coord / coord.w;
	//float sm = shadow2DProj(ShadowMap, coord);
	float sm = chebyshevUpperBound( ShadowCoordPostW, ShadowCoordPostW.z );
	*/
	
	//vec4 rotRandom = texture2D(rotRandomMap, gl_FragCoord.xy * (1./64.) );
	float sm = 0.0;//shadow2DProj(ShadowMap, coord).r;vec2 decal;
	float shadowSmoother = max ( (z.x-shadowFarPlanes.z)* 0.0033, 0.0 );
	vec4 ShadowCoordPostW = coord / coord.w;
	for (int i=0;i<1;i++)
	{
		vec2 decal = vec2(poisson[i].x*rotRandom.x - poisson[i].y*rotRandom.y, poisson[i].x*rotRandom.y + poisson[i].y*rotRandom.x) * vec2(1./4096., 1./1024.) * 4.0;
		
		//sm += chebyshevUpperBound( vec4(coord.xy + decal, coord.zw), ShadowCoordPostW.z );//shadow2DProj(ShadowMap, vec4(coord.xy + decal, coord.zw) ).r;
		sm += max(ComputeLightCoverageVariance(ShadowMap, coord.xy + decal, ShadowCoordPostW.z), 0.0 );
	}
	sm *= (1./4.);
	sm += shadowSmoother;
	
	
	gl_FragData[1] = vec4(sm, normalize(normal.xyz + vec3(snowEffect.xy*1.5, 0.0)*snowfactor )*0.5 + 0.5);
	gl_FragData[2] = vec4(z.x,1,1,1);
	gl_FragData[3] = vec4(snowfactor*100.+0.1, 0.5 + snowfactor*0.5, 0.0, 0.0);
}