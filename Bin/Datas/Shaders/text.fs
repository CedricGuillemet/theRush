uniform sampler2D tex0;
varying vec2 uv; 

void main() 
{ 
	gl_FragColor = texture2D(tex0, uv);
}