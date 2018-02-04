varying vec2  uv;

void main()
{
	gl_Position = gl_Vertex;
	gl_Position.z = 0.1;
	gl_Position.w = 1.0;
	uv = gl_Vertex.zw;
}