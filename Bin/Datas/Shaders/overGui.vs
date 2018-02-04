varying vec4 col;
uniform mat4 modelViewProjectionMatrix;

void main()
{
	gl_Position = modelViewProjectionMatrix * gl_Vertex;
	col = vec4(clamp( dot(gl_Normal.xyz, vec3(1.0) ), 0.0, 1.0 ) * 0.7 + 0.3);
}