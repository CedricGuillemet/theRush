#version 120
uniform sampler2D depthTex; 
varying vec2 uv;

void main() 
{
	float texelx = 1.0/1280.0;
	float texely = 1.0/720.0;

	float mindepth = 3000.0;
	float maxdepth = 0.0;
	for ( int y = 0;y<8;y++)
	{
		for (int x=0;x<8;x++)
		{
			float depth = texture2D(depthTex,uv+vec2(x*texelx, y*texely) ).r;
			mindepth = min(mindepth, depth);
			maxdepth = max(maxdepth, depth);
		}
	}
	gl_FragColor = vec4(mindepth, maxdepth, 0.1, 0.5);
}