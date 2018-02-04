varying vec2 uv; 
uniform float alpha;
void main() 
{ 
float dist = dot(uv, uv);
	float col = clamp( (1.0-clamp(dist,0.0,1.0) ) * 10.0, 0.0, 1.0);
	col -= (1.0-clamp(dist*1.16,0.0,1.0) ) * 1.3;
	col = smoothstep(0.25, 0.75, col);
	gl_FragColor = vec4( 1.0, 1.0, 1.0, alpha*col );
}