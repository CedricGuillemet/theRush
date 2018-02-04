uniform mat4 MVP;
varying vec2 uv;
uniform float Time;
uniform float Width;
uniform float Offset;
varying vec4 vertColor; 
void main()
{

	uv=gl_MultiTexCoord0.xy; 
	uv.y -= Time;
	vec4 normal = vec4(gl_Normal.xyz, 0);
	vec4 pos = gl_Vertex + ( normal * Offset ) - ( normal * (uv.x-0.5) * Width);
	gl_Position = MVP * pos;
	vertColor = gl_Color;
}