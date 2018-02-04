uniform float time;
uniform sampler2D tex0;
uniform float strength;
varying vec2 uv;

void main() 
{
	vec2 uv2 = uv;
	uv2.x += mod((time + uv.y*time) *2342.121, (uv.y*875.341)/time ) * strength;
	uv2.x += mod((uv.y*time) *64.121, ((uv.x+uv.y)*125.341)/time ) * strength*0.5;
	vec4 oricol = texture2D(tex0,uv);
	vec4 col;
	col = texture2D(tex0,vec2(uv2.x+0.0003,uv2.y));
	col.g = texture2D(tex0,vec2(uv2.x+0.000,uv2.y)).y;
	col.b = texture2D(tex0,vec2(uv2.x-0.0003,uv2.y)).z;
	col = clamp(col*0.5+0.5*col*col*1.2,0.0,1.0);
	col *= 0.5 + 0.5*16.0*uv2.x*uv2.y*(1.0-uv2.x)*(1.0-uv2.y);
	col *= vec4(0.7,1.1,0.9, 1.1);
	col *= 0.9+0.1*sin(10.0*time+uv2.y*1000.0);
	col *= 0.97+0.03*sin(110.0*time);
	float comp = smoothstep( 0.2, 0.7, sin(time) );
	col = mix( oricol, col, clamp(strength*10.0,0.0, 1.0) );
	gl_FragColor = vec4(col);
}