uniform float velvetAlpha;
uniform vec3 velvetColor;

void main() 
{
	gl_FragColor = vec4(velvetColor, velvetAlpha);
}