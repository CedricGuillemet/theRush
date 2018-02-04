varying vec4 eyeRay;
varying vec4 uv;
uniform vec4 lightViewPos;
uniform vec4 lightViewColor;
uniform mat3 invView;
uniform sampler2D albedoSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalSampler;
uniform sampler2D textureSpecular;

void main()
{
	vec3 norm = normalize(texture2DProj(normalSampler, uv).yzw-0.5);
	vec4 shader = texture2DProj(textureSpecular, uv);
	vec4 albedo = texture2DProj(albedoSampler, uv);
	float depth = texture2DProj(depthSampler, uv).x;
	vec3 eyeDir = vec3(eyeRay.xy/eyeRay.w, 1.);
	vec3 PixelPos = eyeDir * depth;
	vec3 lightvect = lightViewPos.xyz - PixelPos ;
	vec3 diffAmbColor = vec3(1.-(dot(lightvect, lightvect)*(lightViewPos.w)));
	//vec3 halfAngle = normalize( normalize(lightvect) + normalize(eyeDir));
	
	diffAmbColor *= lightViewColor.xyz;
	//diffAmbColor = max(dot( normalize(lightvect.xyz), (invView * vec4(norm.x, norm.y, norm.z,0.0)).xyz ), 0.);
	vec3 tt = invView * norm;
	vec3 ttn = vec3(tt.x,tt.y,-tt.z);
	//float specular = pow( max( dot(  ttn, halfAngle ), 0.0 ), shader.x*255.0 ) * shader.y;
	diffAmbColor *= max(dot( normalize(lightvect.xyz), ttn ), 0.);
	gl_FragColor = vec4( albedo.xyz * diffAmbColor /*+ vec3(specular)*/, 0.);
}