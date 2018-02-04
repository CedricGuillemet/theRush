uniform sampler2D TextureSampler;
uniform sampler2D TextureWeapon;
varying vec2 texCoordDiffuse; 
uniform vec2 bonusXY;

void main() 
{ 
	vec2 wpt = texCoordDiffuse.xy * vec2(1.0,0.30) + vec2(-0.75,0.0) + bonusXY;

	vec4 weaponTex = texture2D(TextureWeapon, wpt );
	
	if ( texCoordDiffuse.x < 0.75 )
	{
		weaponTex *= 0.0;
	}

	gl_FragColor = texture2D(TextureSampler, texCoordDiffuse.xy*vec2(2.0,1.0)) + weaponTex;
	gl_FragColor.a *= 0.8;
}