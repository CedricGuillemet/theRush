uniform mat4 modelViewProjectionMatrix;
varying vec3 pos;
varying vec4 texCoordDiffuse; 

void main()
{
	gl_Position = modelViewProjectionMatrix * gl_Vertex;
	texCoordDiffuse = vec4( gl_Vertex.xz - vec2( 0.0, -30.0 ), 0.0, 0.0 ) * 0.03;
	pos = gl_Vertex.xyz;
}