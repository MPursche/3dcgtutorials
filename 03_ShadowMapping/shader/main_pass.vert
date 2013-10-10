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
	gl_TexCoord[1] = shadowMatrix * gl_Vertex;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	viewSpace_lightDir = gl_NormalMatrix * lightDir;

	shadowDepth =  length((shadowViewMatrix * gl_Vertex).xyz);//-(shadowViewMatrix * gl_Vertex).z;

	// calculate reflected normal
	vec3 eye = -(gl_ModelViewMatrix * gl_Vertex).xyz;
	eye = normalize(eye);
	vec3 reflection = reflect(normal, eye);
	reflection.z += 1.0;
	float m = sqrt(dot(reflection, reflection));
		
	gl_TexCoord[0] = vec4(reflection.x / 2 * m + 0.5, reflection.y / 2 * m + 0.5, 0.0, 0.0);

	// calculate half vector
	halfVector = normalize(viewSpace_lightDir + eye);
}