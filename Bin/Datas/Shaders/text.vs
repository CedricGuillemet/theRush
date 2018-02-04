varying vec2 uv; 

void main()
{
	gl_Position = gl_Vertex;
	uv = (gl_Vertex.xy*0.5)+0.5;//gl_MultiTexCoord0.xy; 
}