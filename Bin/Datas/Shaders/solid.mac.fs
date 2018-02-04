uniform vec4 color; varying vec4 normal; varying vec4 pos; varying vec4 vertColor;
varying vec4 ProjShadow[4];
uniform sampler2DShadow ShadowMap;
uniform sampler2D rotRandomMap;
uniform vec4 solidShader;
uniform vec4 shadowNearPlanes;
uniform vec4 shadowFarPlanes;

void main() 
{
	vec2 poisson[16];
	poisson[0] = vec2(0.866604,0.543992)-vec2(0.5);
	poisson[1] = vec2(0.194617,0.724509)-vec2(0.5);
	poisson[2] = vec2(0.328318,0.0294504)-vec2(0.5);
	poisson[3] = vec2(0.948668,0.0382397)-vec2(0.5);
	poisson[4] = vec2(0.463851,0.693258)-vec2(0.5);
	poisson[5] = vec2(0.939146,0.955901)-vec2(0.5);
	poisson[6] = vec2(0.698691,0.881741)-vec2(0.5);
	poisson[7] = vec2(0.635487,0.430982)-vec2(0.5);
	poisson[8] = vec2(0.633595,0.0625935)-vec2(0.5);
	poisson[9] = vec2(0.121189,0.253945)-vec2(0.5);
	poisson[10] = vec2(0.00234993,0.548387)-vec2(0.5);
	poisson[11] = vec2(0.439283,0.27485)-vec2(0.5);
	poisson[12] = vec2(0.285379,0.489456)-vec2(0.5);
	poisson[13] = vec2(0.03296,0.964721)-vec2(0.5);
	poisson[14] = vec2(0.378704,0.998108)-vec2(0.5);
	poisson[15] = vec2(0.984985,0.286935)-vec2(0.5);
	
	gl_FragData[0] = color*vertColor;
	vec4 z = vec4(pos.z);
	vec4 near = vec4 (z.x >= shadowNearPlanes.x, z.x >= shadowNearPlanes.y,z.x >= shadowNearPlanes.z, z.x >= shadowNearPlanes.w);
	vec4 far = vec4 (z.x < shadowFarPlanes.x, z.x < shadowFarPlanes.y,z.x < shadowFarPlanes.z, z.x < shadowFarPlanes.w);
	vec4 weights = near * far;
	vec4 coord = ProjShadow[0] * weights.x +
	ProjShadow[1] * weights.y +
	ProjShadow[2] * weights.z +
	ProjShadow[3] * weights.w;
	vec4 rotRandom = texture2D(rotRandomMap, gl_FragCoord.xy * (1./64.) );
	/*
	float sm = shadow2DProj(ShadowMap, coord).r;vec2 decal;
	for (int i=0;i<8;i++)
	{
		decal = vec2(poisson[i].x*rotRandom.x - poisson[i].y*rotRandom.y, poisson[i].x*rotRandom.y + poisson[i].y*rotRandom.x) * vec2(1./4096., 1./1024.) * 4.0;
		sm += shadow2DProj(ShadowMap, vec4(coord.xy + decal, coord.zw) ).r;
	}
	
	sm *= (1./8.);
	*/
	float sm = 1.0;
	

	gl_FragData[1] = vec4(sm, normalize(normal.xyz)*0.5 + 0.5);
	gl_FragData[2] = vec4(z.x,1,1,1);
	gl_FragData[3] = solidShader;
}