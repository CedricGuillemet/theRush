uniform sampler2D TextureSampler;
varying vec3 pos;
varying vec4 texCoordDiffuse; 

uniform float shininess;
uniform float shininess2;
uniform vec2 bonusXY;

float saturate( float value )
{
	return clamp( value, 0.0, 1.0 );
}
float getCircle( vec3 uv, float radius, float thickness, float angle)
{
	float angled = (angle*1.6)-0.8;
	float ng = saturate(saturate(saturate( dot(uv, vec3(cos(angled),sin(angled),0)))*30.0 )+0.1);
	float circle1 = saturate(saturate(sqrt( dot( uv, uv) )-radius) * 100.0 );
	float circle2 = saturate(saturate(sqrt( dot( uv, uv) )-(radius+thickness)) * 100.0 );
	float circ1 = saturate(circle1-circle2);
	return circ1 * ng;
}

void main() 
{ 
	float hilit = abs(pos.y) * 2;

	vec3 uv = texCoordDiffuse.xyz*0.5;
	
	float circ1 = getCircle( uv, 0.84, 0.15, shininess);
	float circ2 = getCircle( uv, 0.65, 0.1, shininess2);

/*
	float carre = saturate((saturate( dot(abs(uv), vec3(0.45,0.0,0.0 ) )  -0.1)) * 200.0);
	carre += saturate((saturate( dot(abs(uv+vec3(0.0, -0.2, 0.0)), vec3(0.0,0.41,0.0) )  -0.1)) * 200.0);
	
	float bonusMask = saturate(1-carre);
 

	 //vec2 bonusXY = vec2(0.25 *2.0 , 0.25 * 2.0 );
	 float texBonus  = 1.3-texture2D(TextureSampler, (uv.xy-vec2(-0.225, -0.05) )*vec2(0.55,0.5)+ bonusXY ).r;
	 float bonusVal = (texBonus * bonusMask)*0.5;
	*/ 
	vec4 degat = mix(vec4(0.3, 0.9, 0.3, 1.0), vec4(10.0, 0.0,0.0,1.0), (1.0-saturate(shininess2*2.2)) );
	
	vec4 result = hilit * (vec4(0.5, 0.5, 0.8, 1.0) + circ1*vec4(1.9, 0.4, 1.9, 1.0) + circ2*degat);// + bonusVal;
	result.a *= 0.8;
    
	gl_FragColor = result;
}