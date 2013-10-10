#version 150 compatibility

uniform mat4 shadowMatrix;
uniform mat4 shadowViewMatrix;
uniform vec3 lightDir;

smooth out vec3 normal;
smooth out vec3 halfVector;
smooth out vec3 viewSpace_lightDir;
smooth out float shadowDepth;

void main()
{
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = shadowMatrix * gl_Vertex;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	viewSpace_lightDir = normalize(gl_NormalMatrix * lightDir);
	shadowDepth = length((shadowViewMatrix * gl_Vertex).xyz);//-(shadowViewMatrix * gl_Vertex).z;

	// calculate eye vector
	vec3 eye = -(gl_ModelViewMatrix * gl_Vertex).xyz;
	eye = normalize(eye);
	
	// calculate half vector
	halfVector = normalize(viewSpace_lightDir + eye);
}