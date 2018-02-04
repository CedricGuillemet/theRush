uniform mat4 viewProjectionMatrix;
uniform mat4 skin[32];
uniform mat4 shadowMat[4];
uniform mat4 viewMatrix;

varying vec4 normal; 
varying vec4 pos; 
varying vec4 vertColor; 
varying vec4 ProjShadow[4];

void main()
{
int matIndex = int(gl_MultiTexCoord0.x);
	vec4 worldPosition = skin[matIndex] * gl_Vertex;
	pos = viewProjectionMatrix * worldPosition;
	gl_Position = pos;
	vec4 normalWorld = skin[matIndex] * vec4(gl_Normal.xyz, 0);
	normal = viewMatrix * normalWorld;
	vertColor = gl_Color;
	ProjShadow[0]		= shadowMat[0] * worldPosition;
	//"ProjShadow[0].z += 0.004 * ProjShadow[0].w;
	ProjShadow[1]		= shadowMat[1] * worldPosition;
	//"ProjShadow[1].z -= 0.0005 * ProjShadow[1].w;
	ProjShadow[2]		= shadowMat[2] * worldPosition;
	//"ProjShadow[2].z -= 0.0005 * ProjShadow[2].w;
	ProjShadow[3]		= shadowMat[3] * worldPosition;
	//"ProjShadow[3].z -= 0.0005 * ProjShadow[3].w;
}