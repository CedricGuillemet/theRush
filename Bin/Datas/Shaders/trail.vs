uniform vec4 vEye;
uniform mat4 modelViewProjectionMatrix; 
varying vec2  uv;
varying vec4 pos;
varying vec4 vertColor;
uniform vec4 trailColor;
uniform mat4 worldMatrix;
uniform float vDecal;
uniform float Width;

void main()
{
	vec4 trPos = worldMatrix * vec4( gl_Vertex.xyz, 1.);
	vec4 trNorm = normalize(worldMatrix * vec4( gl_Normal.xyz, 0.));
	vec3 right = normalize(cross( trNorm.xyz, normalize( trPos.xyz- vEye.xyz) ));
	gl_Position = modelViewProjectionMatrix * (vec4(trPos.xyz,1.0) + vec4(right,0.) * gl_MultiTexCoord0.x * Width ) ;
	uv = gl_MultiTexCoord0.xy;
	uv += vec2(0.5, vDecal);
	vertColor = gl_Color * trailColor *1.3;
	pos = gl_Position;
}