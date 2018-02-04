uniform sampler2D depthTex; 
varying vec2 uv; 
varying vec4 vEyeRay; 
uniform mat4 lightMat;
uniform vec2 zPlanes;
void main() 
{ 
    vec4 minmax = vec4(1000.0, 0.0, 0.0, 0.0);

    float depth = texture2D(depthTex,uv).r;
    if ( ( depth > zPlanes.x ) && ( depth < zPlanes.y ) )
    {
		vec4 viewPos = vEyeRay * vec4(depth);
		vec4 lightSpacePos = lightMat * viewPos;
		gl_FragColor = vec4(lightSpacePos.x, lightSpacePos.x, lightSpacePos.y, lightSpacePos.y);
    }
	else
	{
		gl_FragColor = vec4(10000.0,-10000.0,10000.0,-10000.0);
    }
}