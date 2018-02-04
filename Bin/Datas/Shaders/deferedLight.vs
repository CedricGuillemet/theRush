uniform mat4 worldViewProj;
uniform vec2 deprojection;
varying vec4 eyeRay;
varying vec4 uv;

void main() 
{
	vec4 pos = worldViewProj * vec4( gl_Vertex.xyz, 1.);
	gl_Position = pos;
	uv = pos;
	uv += pos.w;
	uv *= 0.5;
	eyeRay = vec4(pos.xy * deprojection, pos.zw);
}