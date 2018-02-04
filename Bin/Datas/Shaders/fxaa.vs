varying vec2 Apos;
varying vec4 AposPos;
uniform vec2 screenSize;

void main()
{
	gl_Position = gl_Vertex;
	vec2 uv = (vec2( gl_Position.xy ) + vec2( 1.0 ) ) * 0.5;
	Apos = uv + vec2(0.5/screenSize.x, 0.5/screenSize.y);
	AposPos = vec4(uv.xy, uv.xy + vec2(0.999/screenSize.x, 0.999/screenSize.y));
}