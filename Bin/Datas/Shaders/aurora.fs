varying vec2 tc;
varying vec3 col;
varying vec3 p;
uniform float t;
uniform vec3 worldEyePos;

void main() 
{ 
	//gl_FragColor = vec4(0.1, 0.1, 0.1, 0.0);//0.1,0,0.1,0.01);
	
float u = tc.x; 
float v = tc.y + 0.4;
float r = min ( v*v, 0.11/(v*v));//v*v;
r+=0.6;
r = pow(r, 4);//r*r*r*r*r*r;

float r2 = cos(u*(cos(u*2.33217+t*0.0132371)+0.23147 ) * 98.2214 );
r2+= sin(u*(sin(u*2.3987654+t*0.00512241)+0.17841 ) * 17.3697 );
r2= abs(r2);
r2*= sin(u*(sin(u*1.3+t*0.05)+4 ) )*0.5;


	vec3 eyeToPos = normalize( p - worldEyePos) ;
	float dnl = smoothstep( 0.2, 0.7, abs( dot( vec3(0.0,0.0,1.0), eyeToPos ) ) );
	
float hifreq = abs(cos(tc.x*col.x*(30.0+t*0.0001)));
float decUp = sin(tc.x*6.0+t)*0.6;
hifreq *= clamp(sin(tc.y*3.5* (0.6+decUp) - decUp), 0.0, 1.0);

  float degHeight = clamp( sin( col.y * 3.14159 ) *1.3, 0.0, 1.0 );
  float colh = min(col.y, degHeight);//1/(col.y*40));
  
  vec4 hallows = r * r2 * col.z * colh*colh * vec4(0.3, 0.15, 0.1, 1.0) * 0.5;
  
  gl_FragColor =  hallows + ( dnl * degHeight * colh * col.x) * vec4(0.1, 0.8, 0.3, 0.0)*0.8 ;	
}