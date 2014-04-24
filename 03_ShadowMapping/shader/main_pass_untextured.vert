#version 150 compatibility

uniform mat4 shadow_viewProjectionMatrix;
uniform mat4 shadow_viewMatrix;
uniform vec4 lightPosition;

smooth out vec3 normal;
smooth out vec3 halfVector;
smooth out vec3 viewSpace_lightDir;
smooth out float shadowDepth;

void main()
{
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = shadow_viewProjectionMatrix * gl_Vertex;
	shadowDepth = -(shadow_viewMatrix * gl_Vertex).z;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	viewSpace_lightDir = normalize(gl_NormalMatrix * lightPosition.xyz);

	// calculate eye vector
	vec3 eye = -(gl_ModelViewMatrix * gl_Vertex).xyz;
	eye = normalize(eye);
	
	// calculate half vector
	halfVector = normalize(viewSpace_lightDir + eye);
}