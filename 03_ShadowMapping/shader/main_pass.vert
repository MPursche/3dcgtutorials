#version 150 compatibility

uniform mat4 shadow_viewProjectionMatrix;
uniform mat4 shadow_viewMatrix;
uniform mat4 osg_modelMatrix;
uniform vec4 lightPosition;

smooth out vec3 normal;
smooth out vec3 halfVector;
smooth out vec3 viewSpace_lightDir;
smooth out float shadowDepth;

void main()
{
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[1] = shadow_viewProjectionMatrix * osg_modelMatrix * gl_Vertex;
	shadowDepth = -(shadow_viewMatrix * osg_modelMatrix * gl_Vertex).z;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	viewSpace_lightDir = normalize(gl_NormalMatrix * lightPosition.xyz);

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