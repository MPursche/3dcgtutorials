#version 140 compatibilty
uniform mat4 instanceModelViewMatrix[64];

smooth out vec4 color;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * instanceModelMatrix[gl_Instance] * gl_Vertex;
	color = gl_Color;
}