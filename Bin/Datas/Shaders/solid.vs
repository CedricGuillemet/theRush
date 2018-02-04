uniform mat4 normalViewMatrix;
uniform mat4 modelViewProjectionMatrix; 
varying vec4 normal; 
varying vec4 pos; 
varying vec4 vertColor; 
varying vec4 ProjShadow[4];
uniform mat4 shadowMat[4];


void main()
{
	gl_Position = modelViewProjectionMatrix * gl_Vertex;
	pos = gl_Position;
	normal = normalViewMatrix * vec4(gl_Normal.xyz, 0);
	vertColor = gl_Color;
	ProjShadow[0]		= shadowMat[0] * gl_Vertex;
	ProjShadow[1]		= shadowMat[1] * gl_Vertex;
	ProjShadow[2]		= shadowMat[2] * gl_Vertex;
	ProjShadow[3]		= shadowMat[3] * gl_Vertex;
}