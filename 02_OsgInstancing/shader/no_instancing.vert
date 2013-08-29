#version 150 compatibility

uniform vec3 lightDirection;

smooth out vec2 texCoord;
smooth out vec3 normal;
smooth out vec3 lightDir;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix *gl_Vertex;
	texCoord = gl_MultiTexCoord0.xy;
	normal   = gl_NormalMatrix * gl_Normal;
	lightDir = lightDirection;
}