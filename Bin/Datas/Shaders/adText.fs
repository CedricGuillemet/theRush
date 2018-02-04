varying vec2 uv; 
uniform sampler2D adTextTexture;

void main()
{ 
	vec4 col = texture2D(adTextTexture, uv);
	gl_FragColor = col;

}