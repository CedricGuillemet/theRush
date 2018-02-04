uniform vec4 dproj; 
varying vec2 uv; 
varying vec4 vEyeRay; 
void main()
{
	gl_Position = gl_Vertex; 
	uv = (vec2( gl_Position.xy ) + vec2( 1.0 ) ) * 0.5; 
	vEyeRay = vec4(gl_Vertex.xy*dproj.xy , 1,0);
}