uniform mat4 mvp;
uniform float t;

varying vec2 tc;
varying vec3 col;
varying vec3 p;

void main()
{
	//gl_Position = mvp * gl_Vertex;
	
	float freq = 0.02;
    float amp = 30;
    p = gl_Vertex.xyz;
    
    float xfactor = clamp(sin( gl_MultiTexCoord0.x * 3.14159), 0.0, 1.0);
    float amplitude = cos( (t*2.0 + gl_MultiTexCoord0.x) * 0.05 )*amp;
    
    float normFactor =  cos( (p.x) * freq + t)* amplitude * xfactor;
    p += vec3( 0.0, 0.0, normFactor );
    gl_Position = mvp * vec4( p, 1.0);

	col = vec3(xfactor, 1.0-gl_MultiTexCoord0.y, normFactor);
	
	tc = gl_MultiTexCoord0.xy;
}