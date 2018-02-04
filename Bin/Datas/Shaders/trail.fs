uniform sampler2D texTrail;
varying vec4 vertColor;
varying vec2  uv;
varying vec4 pos;
uniform float fogDensity;
uniform vec3 fogColor;

void main() 
{ 
	vec4 cc = texture2D(texTrail, uv) * vertColor;
	gl_FragColor = cc;
	const float LOG2 = 1.442695;
	float fogFactor = exp2( -fogDensity * fogDensity * pos.z * pos.z * LOG2 );
	fogFactor = 1.-clamp(fogFactor, 0.0, 1.0);
	gl_FragColor.rgb = mix(cc.rgb, fogColor, fogFactor);
}