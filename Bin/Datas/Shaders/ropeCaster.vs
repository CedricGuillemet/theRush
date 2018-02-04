uniform mat4 viewProjectionMatrix;
uniform mat4 skin[32];
varying vec4 v_position;
void main()
{
	int matIndex = int(gl_MultiTexCoord0.x);
	vec4 worldPosition = skin[matIndex] * gl_Vertex;
	gl_Position = viewProjectionMatrix * worldPosition;
	v_position = gl_Position;
}