uniform sampler2D tExplosion;
uniform float alpha;
//varying vec3 vReflect;
varying vec3 pos;
varying float ao;
varying float d;
float PI = 3.14159265358979323846264;

float random(vec3 scale,float seed){return fract(sin(dot(gl_FragCoord.xyz+seed,scale))*43758.5453+seed);}

void main() {

float u = 1.0 - 1.3 * ao + .01 * random(vec3(12.9898,78.233,151.7182),0.0);
	vec3 color = texture2D( tExplosion, vec2( 0, u ) ).rgb;
	gl_FragColor = vec4( color.rgb, alpha );

}