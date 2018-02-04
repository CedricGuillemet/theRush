uniform vec4 color;
uniform sampler2D ShadowMap;
uniform vec4 shadowNearPlanes;
uniform vec4 shadowFarPlanes;
varying vec4 ProjShadow[4]; 
varying vec4 trPos; 
uniform vec4 sunDirection;
uniform vec4 sunColor;
varying vec4 normal;
varying vec4  uv; 
varying vec4 vertColor; 
varying vec3 cameraView;
uniform samplerCube textureAmbient;
varying float distToClipPlane;
uniform vec4 shader;


float ComputeLightCoverageVariance(sampler2D Sampler, vec2 TexBase, float LightCompare)
{
    vec2 LightValue = texture2D(Sampler, TexBase).xy;
    
    float Diff = LightCompare - LightValue.x;
    if(Diff > 0.0)
    {
		float Variance = LightValue.y - LightValue.x * LightValue.x;
		Variance = max(Variance,0.0001);
		
        return Variance / (Variance + Diff * Diff);
    }
    else
    {
        return 1.0;
    }
}

void main (void)
{
	if (distToClipPlane<0.0) discard;
	vec3 norm = normalize(normal).xyz;
	vec4 albedo = color * vertColor;
	vec4 z = vec4(trPos.z);
	vec4 near = vec4 (z.x >= shadowNearPlanes.x, z.x >= shadowNearPlanes.y,z.x >= shadowNearPlanes.z, z.x >= shadowNearPlanes.w);
	vec4 far = vec4 (z.x < shadowFarPlanes.x, z.x < shadowFarPlanes.y,z.x < shadowFarPlanes.z, z.x < shadowFarPlanes.w);
	vec4 weights = near * far;
	vec4 coord = ProjShadow[0] * weights.x +
	ProjShadow[1] * weights.y +
	ProjShadow[2] * weights.z +
	ProjShadow[3] * weights.w;
	
	//float mip = shadow2DProj(ShadowMap, coord).r;
	vec4 ShadowCoordPostW = coord / coord.w;
	//float sm = chebyshevUpperBound( ShadowCoordPostW, ShadowCoordPostW.z );
	float sm = max(ComputeLightCoverageVariance(ShadowMap, coord.xy , ShadowCoordPostW.z), 0.0 );
	
	vec3 halfAngle = normalize( sunDirection.xyz + normalize( cameraView ) );
	float specular = pow( clamp( dot( norm, halfAngle ), 0.0, 1.0 ), shader.x * 255.0 ) * shader.y;
	float sunContribution = clamp(dot(norm, sunDirection.xyz), 0., 1.);
	vec3 skyAmbientColor = textureCube(textureAmbient, vec3(-norm.x, norm.y, -norm.z)).xyz;
	vec3 ambientComponent = albedo.xyz * skyAmbientColor;
	vec3 diffuseComponent = albedo.xyz * sunColor.xyz * sunContribution * sm;
	vec3 selfIllumComponent = albedo.xyz * clamp((albedo.w-0.5)*4., 0., 1.);
	gl_FragColor = vec4(ambientComponent + diffuseComponent + selfIllumComponent, albedo.w) + vec4(specular);
}