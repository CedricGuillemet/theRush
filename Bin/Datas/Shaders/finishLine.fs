varying vec2 uv; 
varying vec4 pos; 
uniform sampler2D texScreen;

void main()
{ 
	vec4 col = texture2D(texScreen, uv);
	vec4 z = vec4(pos.z);
	gl_FragData[0] = vec4(col.xyz, col.w * 0.45);
	gl_FragData[1] = vec4(1.0, 0.5, 1.0,0.5);
	gl_FragData[2] = vec4(z.x,1,1,1);
	gl_FragData[3] = vec4(0.0);
}