uniform sampler2D texSource;
varying vec2  uv;

void main(void) 
{
	vec4 val = texture2D(texSource, uv );
	vec3 ColorOut;
	ColorOut = val.xyz* clamp( (val.w-0.55), 0.0, 1.0);
	gl_FragColor = vec4(ColorOut, 1.);
}