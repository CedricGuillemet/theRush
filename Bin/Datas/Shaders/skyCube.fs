uniform vec2 viewport;
uniform mat4 inv_proj;
uniform mat3 inv_view_rot;
uniform samplerCube source;
vec3 get_world_normal()
{
    vec2 frag_coord = gl_FragCoord.xy/viewport;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((inv_proj * device_normal).xyz);
    vec3 world_normal = normalize(inv_view_rot*eye_normal);
    return world_normal;
}

void main(void)
{
    vec3 normal = get_world_normal();
    vec4 color = textureCube(source, normal);
    gl_FragColor = vec4(color.rgb, dot(color.rgb, vec3(0.299, 0.587, 0.114) ) );
}