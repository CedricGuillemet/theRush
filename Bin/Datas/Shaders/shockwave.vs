varying vec2 uv; 
uniform mat4 mvp;

void main()
{
	gl_Position =  mvp * gl_Vertex ;
	uv = gl_Vertex.xz;//gl_MultiTexCoord0.xy; 
}