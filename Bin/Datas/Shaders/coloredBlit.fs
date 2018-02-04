uniform sampler2D tex0;
varying vec2 uv;
uniform vec4 col;

void main()
{
	gl_FragColor = texture2D( tex0, uv ) * col;
}