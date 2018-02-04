#version 120
uniform vec2 PixelSize;
uniform sampler2D zBuffer, normalBuffer;
uniform sampler2D frameBuffer, bloomBuffer;
varying vec2  uv;
uniform float fogDensity;
uniform vec4 fogColor;


float fogExp2( float d)
{
	//float d = length(position - cameraPosition);
	float l = exp( - pow( d * fogDensity , 2 ) );
	l = clamp( 1 - l, 0.0, 1.0 );
	
	return l;
}

void main()
{
	float r = 0.;
	vec3 sketched = vec3(0.,0.,0.);
	vec3 fmb = texture2D(frameBuffer, uv).rgb;
	float depthValue = texture2D(zBuffer, uv).r;
	vec3 ares = fmb + texture2D(bloomBuffer, uv).rgb*2.5;
	/*float fogFactor = exp( (depthValue - 1000.0) * fogDensity);
	fogFactor = clamp(fogFactor * fogColor.w, 0.0, 1.0);
	*/
	float fogFactor = fogExp2( depthValue );
	gl_FragColor.rgb = mix(ares, fogColor.xyz, fogFactor);
	gl_FragColor.a = (dot(gl_FragColor.rgb, vec3(0.299, 0.587, 0.114)));
}