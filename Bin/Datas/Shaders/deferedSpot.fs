varying vec4 eyeRay;
varying vec4 uv;
uniform vec4 lightViewPos;
uniform vec4 lightViewColor;
uniform float light_atten_begin;
uniform float light_atten_end;
uniform vec2 cos_light_angle_atten;
uniform vec3 light_direction;
uniform sampler2D albedoSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalSampler;

void main() 
{
	gl_FragColor = lightViewColor;
	vec3 norm = normalize(texture2DProj(normalSampler, uv).yzw-0.5);
	vec4 albedo = texture2DProj(albedoSampler, uv);
	float depth = texture2DProj(depthSampler, uv).x;
	vec3 PixelPos = vec3(eyeRay.xy/eyeRay.w, 1.)* depth;
	vec3 dir_to_light = lightViewPos.xyz - PixelPos ;
	vec3 light_contrib;
	float dist_to_light;
	float n_dot_l;
	{
		dist_to_light = length(dir_to_light);
		float atten_amount =
			clamp((dist_to_light   - light_atten_begin) /
			  (light_atten_end - light_atten_begin),
				   0.0, 1.0);
		dir_to_light = normalize(dir_to_light);
		vec2 cos_angle_atten = cos_light_angle_atten;
		float  cos_angle = dot(-dir_to_light, light_direction);
		float angle_atten_amount = 
			clamp((cos_angle         - cos_angle_atten.x) /
				  (cos_angle_atten.y - cos_angle_atten.x),
				   0.0, 1.0);
		light_contrib = (1.0 - atten_amount) * (1.0 - angle_atten_amount) * lightViewColor.xyz;
		n_dot_l       = dot(norm, dir_to_light);
	}
	vec3 diffAmbColor = light_contrib * n_dot_l;
	gl_FragColor = vec4(albedo.xyz * diffAmbColor, 1.);
}