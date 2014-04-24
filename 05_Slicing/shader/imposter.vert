#version 150 compatibility

uniform float nearPlane;
uniform float farPlane;

in vec3 vTangent;

smooth out vec2 texCoord;
smooth out vec3 light;
smooth out vec3 halfVector;
smooth out vec3 normal;
smooth out vec3 eye;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	vec3 tangent = normalize(gl_NormalMatrix * vTangent);
	vec3 bitangent = normalize(cross(normal, tangent));

	vec3 lightWorldSpace = vec3(1.0, 1.0, 0.0);
	light = vec3(dot(lightWorldSpace, tangent),
			     dot(lightWorldSpace, bitangent),
				 dot(lightWorldSpace, normal));

	eye = -(gl_ModelViewMatrix * gl_Vertex).xyz;
	vec3 halfVectorWorldSpace = normalize(lightWorldSpace + eye);
	halfVector = vec3(dot(halfVectorWorldSpace, tangent),
			     dot(halfVectorWorldSpace, bitangent),
				 dot(halfVectorWorldSpace, normal));

	texCoord = gl_MultiTexCoord0.st;
}