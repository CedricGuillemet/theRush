varying vec2 uv; 
void main()
{
	gl_Position = gl_Vertex; 
	uv = (vec2( gl_Position.xy ) + vec2( 1.0 ) ) * 0.5; 
}