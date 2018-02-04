uniform sampler2D tex0;
varying vec2 uv;
uniform vec4 Color;
varying vec4 vertColor; 
void main() 
{ 
	gl_FragColor = texture2D( tex0, uv ) * Color * vertColor;
}