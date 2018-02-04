uniform sampler2D texSource;
uniform vec2 PixelSize;
varying vec2  uv;

void main(void) 
{
	float gauss[16];
	gauss[0]=0.1;
	gauss[1]=0.1;
	gauss[2]=0.15;
	gauss[3]=0.25;
	gauss[4]=0.35;
	gauss[5]=0.55;
	gauss[6]=0.8;
	gauss[7]=0.9;
	gauss[8]=1.0;
	gauss[9]=0.9;
	gauss[10]=0.8;
	gauss[11]=0.55;
	gauss[12]=0.35;
	gauss[13]=0.25;
	gauss[14]=0.15;
	gauss[15]=0.1;
	
	
	vec4 blurred = vec4(0.,0.,0.,0.);
	for (int i=-8;i<8;i++)
		blurred += texture2D(texSource, uv + vec2( PixelSize.x* float(i)*4.0,0.0 )) * gauss[i+8];
	blurred *= (1./7.3);
	gl_FragColor = blurred;
}