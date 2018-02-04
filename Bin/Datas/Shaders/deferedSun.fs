varying vec2 uv;
varying vec4 vEyeRay;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform sampler2D textureNormals;
uniform sampler2D textureDiffuse;
uniform sampler2D textureSpecular;
uniform samplerCube textureAmbient;

void main()
{
	vec4 normalShadow = texture2D(textureNormals,uv);
	vec3 worldSpaceNormal = normalize(normalShadow.yzw-0.5);
	vec4 albedo = texture2D(textureDiffuse,uv);
	vec4 shader = texture2D(textureSpecular, uv);
	
	float dnl = dot(worldSpaceNormal, sunDir);
	float sunContribution = clamp(dnl, 0., 1.);
	vec3 skyAmbientColor = textureCube(textureAmbient, vec3(-worldSpaceNormal.x, worldSpaceNormal.y, -worldSpaceNormal.z)).xyz;
	vec3 ambientComponent = albedo.xyz * skyAmbientColor;
	vec3 diffuseComponent = albedo.xyz * sunColor * sunContribution * normalShadow.x;
	vec3 selfIllumComponent = albedo.xyz * clamp((albedo.w-0.5)*4., 0., 1.);
	vec3 PixelPos = vEyeRay.xyz;
	vec3 halfAngle = normalize( sunDir + normalize(PixelPos));
	float specular = max(sign(dnl),0.0) * normalShadow.x * pow( clamp( dot( worldSpaceNormal, halfAngle ), 0.0, 1.0 ), shader.x*255.0 ) * shader.y;
	gl_FragColor = vec4(ambientComponent + diffuseComponent + selfIllumComponent + vec3(specular) , albedo.w);
}