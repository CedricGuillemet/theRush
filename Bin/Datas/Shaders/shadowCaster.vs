varying vec4 v_position;
uniform mat4 modelViewProjectionMatrix;
void main()
{
	gl_Position = modelViewProjectionMatrix * gl_Vertex;
	v_position = gl_Position;
}