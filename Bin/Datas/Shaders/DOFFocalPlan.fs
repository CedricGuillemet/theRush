uniform sampler2D bgl_DepthTexture;
varying vec2  uv;
uniform float focus;
uniform float marker;

void main()
{
	float depth   = texture2D(bgl_DepthTexture,uv ).r;
	gl_FragColor = vec4(1., 0.,0., (1. - clamp(abs(depth- focus), 0., 1.)) * marker );
}