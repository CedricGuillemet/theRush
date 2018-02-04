uniform mat4 modelViewProjectionMatrix; 
varying vec4 pos; 
varying vec2 uv;

void main()
{
	gl_Position = modelViewProjectionMatrix * gl_Vertex; 
	pos=gl_Position; 
	uv=gl_MultiTexCoord0.xy; 
}