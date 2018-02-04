uniform mat4 mvp; 
varying vec2 uv;

void main()
{
	gl_Position = mvp * gl_Vertex; 
	uv=gl_MultiTexCoord0.xy; 
}