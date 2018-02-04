uniform mat4 modelViewProjectionMatrix;
varying vec2 texCoordDiffuse; 

void main()
{
	gl_Position = modelViewProjectionMatrix * gl_Vertex;
	texCoordDiffuse = vec2(1.0) - vec2( gl_Vertex.x*0.015+0.5, gl_Vertex.y*0.08 );
}