#version 150 compatibility

uniform float nearPlane;
uniform float farPlane;

smooth out vec4 color;
smooth out vec3 normal;
smooth out vec2 texCoord;
smooth out float linearDepth;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 position = gl_ModelViewMatrix * gl_Vertex;
	linearDepth = (-position.z - nearPlane) / (farPlane - nearPlane);
	normal = normalize(gl_NormalMatrix * gl_Normal);
	color = gl_Color;
	texCoord = gl_MultiTexCoord0.st;
}