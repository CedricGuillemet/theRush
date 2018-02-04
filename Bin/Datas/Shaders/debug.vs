uniform mat4 modelViewProjectionMatrix;
void main()
{
	gl_Position = modelViewProjectionMatrix * gl_Vertex;
}