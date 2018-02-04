varying vec4 ProjShadow[4];
uniform mat4 modelViewProjectionMatrix;
uniform mat4 shadowMat[4]; 
varying vec4 trPos; 
varying vec4 vertColor;
uniform mat4 normalMatrix;
varying vec4 normal; 
varying vec3 cameraView;
uniform vec4 clipPlane;
uniform mat4 worldMatrix;
uniform vec3 CameraPosition;
varying float distToClipPlane;
varying vec4  uv;

void main(void)	
{
	ProjShadow[0]		= shadowMat[0] * gl_Vertex;
	ProjShadow[0].z += 0.0 * ProjShadow[0].w;
	ProjShadow[1]		= shadowMat[1] * gl_Vertex;
	ProjShadow[1].z -= 0.0015 * ProjShadow[1].w;
	ProjShadow[2]		= shadowMat[2] * gl_Vertex;
	ProjShadow[2].z -= 0.0015 * ProjShadow[2].w;
	ProjShadow[3]		= shadowMat[3] * gl_Vertex;
	ProjShadow[3].z -= 0.0015 * ProjShadow[3].w;
	gl_Position			= modelViewProjectionMatrix * gl_Vertex;
	vec3 wPos = (worldMatrix * gl_Vertex).xyz;
	distToClipPlane = dot( wPos, clipPlane.xyz ) - clipPlane.w;
	trPos				= modelViewProjectionMatrix * gl_Vertex;
	uv = gl_Position;
	normal = normalMatrix * vec4(gl_Normal.xyz, 0);
	cameraView =  CameraPosition - wPos ;
	vertColor = gl_Color;
}