uniform vec2 viewport;
uniform mat4 inv_proj;
uniform mat3 inv_view_rot;
uniform samplerCube source;

vec3 get_world_normal(vec2 pos, vec2 dims)
{
	vec2 frag_coord = pos/dims;
	frag_coord = (frag_coord-0.5)*2.0;
	vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
	vec3 eye_normal = normalize((inv_proj * device_normal).xyz);
	vec3 world_normal = normalize(inv_view_rot*eye_normal);
	return world_normal;
}

vec4 sample(float xoff, float yoff){
	vec2 off = gl_FragCoord.xy*2.0+vec2(xoff, yoff);
	vec3 normal = get_world_normal(off, viewport*2.0);
	return textureCube(source, normal);
}

void main(void){
	vec4 color = (
		sample(-10.95, -10.95) +
		sample(-10.95, +10.95) +
		sample(+10.95, -10.95) +
		sample(+10.95, +10.95)
	) * 0.25;
	gl_FragColor = vec4(color.rgb, 1.0);
}